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

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/json/object.hpp>
#include <engine/job.hpp>
#include <engine/task.hpp>
#include <thread>

TEST(job, can_run) {
  boost::asio::io_context _ioc;
  std::atomic _executed{false};
  const auto _task = std::make_shared<engine::task>(
      [&_executed](auto& cancelled,
                   auto& data) -> boost::asio::awaitable<void> {
        boost::ignore_unused(cancelled, data);
        _executed.store(true, std::memory_order_release);
        co_return;
      });
  const auto _job = std::make_shared<engine::job>(_task, boost::json::object{});
  auto fut = co_spawn(_ioc, _job->run(), boost::asio::use_future);
  _ioc.run();
  fut.get();
  ASSERT_TRUE(_executed.load(std::memory_order_acquire));
}

TEST(job, run_is_promise) {
  boost::asio::io_context _ioc;
  std::atomic _executed{false};
  const auto _task = std::make_shared<engine::task>(
      [&_executed](auto& cancelled,
                   auto& data) -> boost::asio::awaitable<void> {
        boost::ignore_unused(cancelled, data);
        _executed.store(true, std::memory_order_release);
        co_return;
      });
  const auto _job = std::make_shared<engine::job>(_task, boost::json::object{});
  const auto _promise = _job->run();
  ASSERT_FALSE(_executed.load(std::memory_order_acquire));
  std::atomic<bool> _cancelled;
  auto _result = (*_task->callback())(_cancelled, boost::json::object{});
  ASSERT_FALSE(_executed.load(std::memory_order_acquire));

  const auto _running_job = [&_job]() -> boost::asio::awaitable<void> {
    co_await _job->run();
    co_return;
  };

  auto fut = co_spawn(_ioc, _running_job, boost::asio::use_future);
  _ioc.run();
  fut.get();

  ASSERT_TRUE(_job->finished());
}
