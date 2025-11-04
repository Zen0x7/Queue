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

#include <engine/task.hpp>
#include <engine/worker.hpp>
#include <thread>

#include <boost/core/ignore_unused.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/json/object.hpp>

TEST(worker, can_run) {
  boost::asio::io_context _ioc;
  std::atomic _executed{false};
  const auto _task = std::make_shared<engine::task>([&_executed](auto& cancelled, auto& data) -> boost::asio::awaitable<void> {
    boost::ignore_unused(cancelled, data);
    _executed.store(true, std::memory_order_release);
    co_return;
  });
  const auto _worker = std::make_shared<engine::worker>(make_strand(_ioc));
  auto fut = co_spawn(
      _ioc,
      [&_worker, &_task]() -> boost::asio::awaitable<void> {
        _worker->dispatch(_task, boost::json::object{});
        co_return;
      },
      boost::asio::use_future);
  _ioc.run();
  fut.get();
  ASSERT_TRUE(_executed.load(std::memory_order_acquire));
}

TEST(worker, can_be_instanced) {
  boost::asio::io_context _ioc;
  const engine::worker _worker(make_strand(_ioc));
  ASSERT_EQ(_worker.number_of_tasks(), 0);
}
