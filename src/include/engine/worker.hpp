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

#ifndef ENGINE_WORKER_HPP
#define ENGINE_WORKER_HPP

#include <memory>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>

namespace engine {
class job;

class worker : public std::enable_shared_from_this<worker> {
  boost::uuids::uuid id_ = boost::uuids::random_generator()();
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
  std::atomic<std::uint64_t> number_of_tasks_;

 public:
  explicit worker(
      boost::asio::strand<boost::asio::io_context::executor_type> strand);

  const boost::uuids::uuid& id() const noexcept;

  std::uint64_t number_of_tasks() const noexcept;

  template <typename Handler>
  std::shared_ptr<job> dispatch(Handler&& handler) {
    auto _job = std::make_shared<job>(std::forward<Handler>(handler));
    number_of_tasks_.fetch_add(1, std::memory_order_release);
    boost::asio::co_spawn(
        strand_,
        [_job]() -> boost::asio::awaitable<void> { co_await _job->run(); },
        boost::asio::detached);
    return _job;
  }
};
}  // namespace engine

#endif  // ENGINE_WORKER_HPP
