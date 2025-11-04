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

#include <engine/worker.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>

namespace engine {
worker::worker(boost::asio::strand<boost::asio::io_context::executor_type> strand) : strand_(std::move(strand)) {}

const boost::uuids::uuid& worker::id() const noexcept {
  return id_;
}

std::uint64_t worker::number_of_tasks() const noexcept {
  return number_of_tasks_.load(std::memory_order_acquire);
}

std::shared_ptr<job> worker::dispatch(const std::shared_ptr<task>& task, boost::json::object data) {
  auto _job = std::make_shared<job>(task, std::move(data));
  number_of_tasks_.fetch_add(1, std::memory_order_release);
  co_spawn(strand_, [_job]() -> boost::asio::awaitable<void> { co_await _job->run(); }, boost::asio::detached);
  return _job;
}
}  // namespace engine
