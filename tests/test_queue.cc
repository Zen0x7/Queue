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

#include <queue/state.hpp>

TEST(queue, can_handle_jobs) {
  const auto _state = std::make_shared<queue::state>();
  const auto _queue = _state->add_queue("notifications");

  std::atomic _task_executed{false};

  const auto _job = _queue->push([&_task_executed] {
    _task_executed.store(true, std::memory_order_release);
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
  const auto _state = std::make_shared<queue::state>();
  const auto _queue = _state->add_queue("notifications");

  std::atomic _first_task_executed{false};
  std::atomic _second_task_executed{false};
  std::atomic _third_task_executed{false};

  const auto _first_job = _queue->push([&_first_task_executed] {
    _first_task_executed.store(true, std::memory_order_release);
  });
  const auto _second_job = _queue->push([&_second_task_executed] {
    _second_task_executed.store(true, std::memory_order_release);
  });
  const auto _third_job = _queue->push([&_third_task_executed] {
    _third_task_executed.store(true, std::memory_order_release);
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
