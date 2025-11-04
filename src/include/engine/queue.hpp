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

#ifndef ENGINE_QUEUE_HPP
#define ENGINE_QUEUE_HPP

#include <algorithm>
#include <atomic>
#include <boost/asio/strand.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/json/object.hpp>
#include <boost/uuid/uuid.hpp>
#include <engine/errors/task_not_found.hpp>
#include <engine/worker.hpp>
#include <map>
#include <memory>

#include <engine/task.hpp>

namespace engine {
class worker;
class job;

class queue : public std::enable_shared_from_this<queue> {
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
  std::map<boost::uuids::uuid, std::shared_ptr<worker>> workers_;
  std::map<boost::uuids::uuid, std::shared_ptr<job>> jobs_;
  std::mutex jobs_mutex_;
  std::map<std::string, std::shared_ptr<task>, std::less<>> tasks_;

 public:
  explicit queue(boost::asio::strand<boost::asio::io_context::executor_type> strand);
  std::size_t number_of_workers() const;
  std::size_t number_of_jobs() const;
  std::shared_ptr<job> dispatch(std::string const& name, boost::json::object data);
  void add_task(std::string name, handler_type handler);
  void set_workers_to(std::size_t number_of_workers);
  void cancel();
};
}  // namespace engine

#endif  // ENGINE_QUEUE_HPP
