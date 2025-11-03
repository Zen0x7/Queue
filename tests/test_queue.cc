// Copyright (C) 2025 Ian Torres <iantorres@outlook.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <gtest/gtest.h>

#include <engine/job.hpp>
#include <engine/queue.hpp>
#include <engine/state.hpp>
#include <engine/worker.hpp>

#include <thread>

TEST(queue, can_handle_jobs) {
  const auto _state = std::make_shared<engine::state>();
  const auto _queue = _state->add_queue("notifications");

  std::atomic _task_executed{false};

  const auto _job = _queue->dispatch(
      [&_task_executed](auto& cancelled) -> boost::asio::awaitable<void> {
        boost::ignore_unused(cancelled);
        _task_executed.store(true, std::memory_order_release);
        co_return;
      });

  ASSERT_FALSE(_job->started());
  ASSERT_FALSE(_job->finished());
  ASSERT_FALSE(_task_executed.load(std::memory_order_acquire));
  ASSERT_FALSE(_job->started_at() != std::chrono::system_clock::time_point{});
  ASSERT_FALSE(_job->finished_at() != std::chrono::system_clock::time_point{});
  ASSERT_FALSE(_job->id().is_nil());

  _state->run();

  ASSERT_TRUE(_job->started());
  ASSERT_TRUE(_job->finished());
  ASSERT_TRUE(_task_executed.load(std::memory_order_acquire));
  ASSERT_TRUE(_job->started_at() != std::chrono::system_clock::time_point{});
  ASSERT_TRUE(_job->finished_at() != std::chrono::system_clock::time_point{});
  const auto _duration = _job->finished_at() - _job->started_at();
  ASSERT_GE(_duration.count(), 0);
}

TEST(queue, can_handle_multiple_jobs) {
  const auto _state = std::make_shared<engine::state>();
  const auto _queue = _state->add_queue("notifications");

  std::atomic _first_task_executed{false};
  std::atomic _second_task_executed{false};
  std::atomic _third_task_executed{false};

  const auto _first_job = _queue->dispatch(
      [&_first_task_executed](auto& cancelled) -> boost::asio::awaitable<void> {
        boost::ignore_unused(cancelled);
        _first_task_executed.store(true, std::memory_order_release);
        co_return;
      });
  const auto _second_job =
      _queue->dispatch([&_second_task_executed](
                           auto& cancelled) -> boost::asio::awaitable<void> {
        boost::ignore_unused(cancelled);
        _second_task_executed.store(true, std::memory_order_release);
        co_return;
      });
  const auto _third_job = _queue->dispatch(
      [&_third_task_executed](auto& cancelled) -> boost::asio::awaitable<void> {
        boost::ignore_unused(cancelled);
        _third_task_executed.store(true, std::memory_order_release);
        co_return;
      });

  _state->run();

  ASSERT_TRUE(_first_job->finished());
  ASSERT_TRUE(_first_task_executed.load(std::memory_order_acquire));
  ASSERT_TRUE(_second_job->finished());
  ASSERT_TRUE(_second_task_executed.load(std::memory_order_acquire));
  ASSERT_TRUE(_third_job->finished());
  ASSERT_TRUE(_third_task_executed.load(std::memory_order_acquire));

  ASSERT_TRUE(_first_job->finished_at() <= _second_job->finished_at());
  ASSERT_TRUE(_second_job->finished_at() <= _third_job->finished_at());
}

TEST(queue, can_handle_multiple_jobs_on_multiple_workers) {
  const auto _state = std::make_shared<engine::state>();
  const auto _queue = _state->add_queue("notifications");
  _queue->set_workers_to(16);

  std::atomic<std::uint64_t> _tasks_executed{0};

  for (std::uint32_t i = 0; i < 2048; ++i) {
    _queue->dispatch(
        [&_tasks_executed](auto& cancelled) -> boost::asio::awaitable<void> {
          boost::ignore_unused(cancelled);
          _tasks_executed.fetch_add(1, std::memory_order_relaxed);
          co_return;
        });
  }

  _state->run();
  ASSERT_EQ(_tasks_executed.load(), 2048);
}

TEST(queue, can_upscale_and_downscale_workers) {
  const auto _state = std::make_shared<engine::state>();
  const auto _queue = _state->add_queue("notifications");
  _queue->set_workers_to(16);
  ASSERT_EQ(16, _queue->number_of_workers());
  _queue->set_workers_to(32);
  ASSERT_EQ(32, _queue->number_of_workers());
  _queue->set_workers_to(64);
  ASSERT_EQ(64, _queue->number_of_workers());
  _queue->set_workers_to(32);
  ASSERT_EQ(32, _queue->number_of_workers());
  _queue->set_workers_to(16);
  ASSERT_EQ(16, _queue->number_of_workers());
  _queue->set_workers_to(16);
  ASSERT_EQ(16, _queue->number_of_workers());
}

TEST(queue, can_be_cancelled) {
  const auto _state = std::make_shared<engine::state>();
  const auto _queue = _state->add_queue("notifications");
  _queue->set_workers_to(4);

  std::atomic<std::size_t> _jobs_executed;

  _queue->dispatch(
      [&_queue](std::atomic<bool>& cancelled) -> boost::asio::awaitable<void> {
        boost::ignore_unused(cancelled);
        _queue->cancel();
        co_return;
      });

  for (std::uint32_t i = 0; i < 256; ++i) {
    _queue->dispatch([&_jobs_executed](std::atomic<bool> const& cancelled)
                         -> boost::asio::awaitable<void> {
      boost::ignore_unused(cancelled);
      _jobs_executed.fetch_add(1, std::memory_order_relaxed);
      co_return;
    });
  }

  _state->run();
  ASSERT_GE(_jobs_executed.load(std::memory_order_relaxed), 0);
  ASSERT_LT(_jobs_executed.load(std::memory_order_relaxed), 256);
  ASSERT_EQ(_queue->number_of_jobs(), 256 + 1);

  std::cout << _jobs_executed.load(std::memory_order_relaxed)
            << " jobs has been processed before cancellation" << std::endl;
}
