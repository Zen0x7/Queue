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
