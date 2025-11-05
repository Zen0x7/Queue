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

#include <engine/errors/task_not_found.hpp>
#include <engine/job.hpp>
#include <engine/queue.hpp>
#include <engine/worker.hpp>
#include <ranges>

namespace engine {
queue::queue(boost::asio::strand<boost::asio::io_context::executor_type> strand)
    : strand_(std::move(strand)) {
  prepare();
}

std::size_t queue::number_of_workers() const { return workers_.size(); }

std::size_t queue::number_of_jobs() const { return jobs_.size(); }

std::shared_ptr<job> queue::dispatch(std::string const& name,
                                     boost::json::object data) {
  auto _job = get_worker()->dispatch(get_task(name), std::move(data));
  dispatch(_job);
  return _job;
}

void queue::add_task(std::string name, handler_type handler) {
  std::scoped_lock _lock(tasks_mutex_);
  tasks_[std::move(name)] = std::make_shared<task>(std::move(handler));
}

void queue::set_workers_to(const std::size_t no_of_workers) {
  if (number_of_workers() < no_of_workers) {
    upscale(no_of_workers);
  } else {
    downscale(no_of_workers);
  }
}

void queue::cancel() {
  std::scoped_lock _lock(jobs_mutex_);
  for (const std::shared_ptr<job>& _job : jobs_ | std::views::values) {
    _job->cancel();
  }
}

void queue::prepare() { upscale(); }

void queue::upscale(const std::size_t to) {
  std::scoped_lock _lock(workers_mutex_);
  while (number_of_workers() != to) {
    auto _worker =
        std::make_shared<worker>(make_strand(strand_.get_inner_executor()));
    workers_.try_emplace(_worker->id(), _worker);
  }
}

void queue::downscale(const std::size_t to) {
  std::scoped_lock _lock(workers_mutex_);
  auto _iterator = workers_.begin();
  while (number_of_workers() != to) {
    _iterator = workers_.erase(_iterator);
  }
}

void queue::dispatch(const std::shared_ptr<job>& job) {
  std::scoped_lock _lock(jobs_mutex_);
  jobs_.try_emplace(job->id(), job);
}

std::shared_ptr<worker> queue::get_worker() {
  std::scoped_lock _lock(workers_mutex_);
  const auto _iterator = std::ranges::min_element(
      workers_ | std::views::values,
      [](const std::shared_ptr<worker>& a, const std::shared_ptr<worker>& b) {
        return a->number_of_tasks() < b->number_of_tasks();
      });
  return *_iterator;
}

std::shared_ptr<task> queue::get_task(const std::string& name) {
  std::scoped_lock _lock(tasks_mutex_);
  const auto it = tasks_.find(name);
  if (it == tasks_.end()) throw errors::task_not_found();
  return it->second;
}
}  // namespace engine
