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
#include <map>
#include <memory>

#include <boost/json/object.hpp>

#include <boost/asio/strand.hpp>
#include <boost/core/ignore_unused.hpp>

#include <boost/uuid/uuid.hpp>

#include <engine/worker.hpp>

#include <engine/errors/task_not_found.hpp>

namespace engine {
class worker;

class job;

using task_handler =
    std::function<boost::asio::awaitable<void>(std::atomic<bool>&,
                                               boost::json::object const&)>;

using tasks_container = std::map<std::string, task_handler>;

class queue : public std::enable_shared_from_this<queue> {
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
  std::map<boost::uuids::uuid, std::shared_ptr<worker>> workers_;
  std::map<boost::uuids::uuid, std::shared_ptr<job>> jobs_;
  tasks_container tasks_;

  std::mutex jobs_mutex_;

 public:
  explicit queue(
      boost::asio::strand<boost::asio::io_context::executor_type> strand);

  std::size_t number_of_workers() const;
  std::size_t number_of_jobs() const;

  std::shared_ptr<job> dispatch(std::string const& name,
                                boost::json::object data) {
    const auto it = tasks_.find(name);
    if (it == tasks_.end())
      throw errors::task_not_found();
    auto handler = it->second;
    auto _comparator = [](const auto& a, const auto& b) {
      return a.second->number_of_tasks() < b.second->number_of_tasks();
    };
    auto _worker_iterator = std::ranges::min_element(workers_, _comparator);
    auto& [_, _worker] = *_worker_iterator;
    boost::ignore_unused(_);
    auto _job = _worker->dispatch(handler, std::move(data));
    {
      std::scoped_lock _lock(jobs_mutex_);
      jobs_.try_emplace(_job->id(), _job);
    }
    return _job;
  }

  template <typename Handler>
  void add_task(std::string name, Handler handler) {
    tasks_[std::move(name)] = task_handler(std::move(handler));
  }

  void set_workers_to(std::size_t number_of_workers);

  void cancel();
};
}  // namespace engine

#endif  // ENGINE_QUEUE_HPP
