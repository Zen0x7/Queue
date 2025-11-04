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

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/json.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <engine/job.hpp>
#include <memory>

namespace engine {
class job;
class task;

class worker : public std::enable_shared_from_this<worker> {
  boost::uuids::uuid id_ = boost::uuids::random_generator()();
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
  std::atomic<std::uint64_t> number_of_tasks_;

  static boost::asio::awaitable<void> run(std::shared_ptr<job> job);

 public:
  explicit worker(boost::asio::strand<boost::asio::io_context::executor_type> strand);
  const boost::uuids::uuid& id() const noexcept;
  std::uint64_t number_of_tasks() const noexcept;
  std::shared_ptr<job> dispatch(const std::shared_ptr<task>& task, boost::json::object data);
};
}  // namespace engine

#endif  // ENGINE_WORKER_HPP
