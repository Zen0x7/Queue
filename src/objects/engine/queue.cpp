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

#include <boost/core/ignore_unused.hpp>

namespace engine {
queue::queue(boost::asio::strand<boost::asio::io_context::executor_type> strand) : strand_(std::move(strand)) {
  prepare();
}

std::size_t queue::number_of_workers() const {
  return workers_.size();
}

std::size_t queue::number_of_jobs() const {
  return jobs_.size();
}

std::shared_ptr<job> queue::dispatch(std::string const& name, boost::json::object data) {
  const auto _task = get_task(name);
  const auto _worker = get_worker();
  auto _job = _worker->dispatch(_task, std::move(data));
  push_job(_job);
  return _job;
}

void queue::add_task(std::string name, handler_type handler) {
  tasks_[std::move(name)] = std::make_shared<task>(std::move(handler));
}

void queue::set_workers_to(const std::size_t number_of_workers) {
  if (workers_.size() < number_of_workers) {
    upscale(number_of_workers);
  } else if (workers_.size() > number_of_workers) {
    downscale(number_of_workers);
  }
}

void queue::cancel() {
  std::scoped_lock _lock(jobs_mutex_);
  std::ranges::for_each(jobs_, [&](auto& job) { job.second->cancel(); });
}

void queue::prepare() {
  auto _worker = std::make_shared<worker>(make_strand(strand_.get_inner_executor()));
  workers_.try_emplace(_worker->id(), _worker);
}

void queue::upscale(const std::size_t to) {
  const std::size_t _needed = to - workers_.size();
  for (std::size_t _index = 0; _index < _needed; _index++) {
    auto _worker = std::make_shared<worker>(make_strand(strand_.get_inner_executor()));
    workers_.try_emplace(_worker->id(), _worker);
  }
}

void queue::downscale(const std::size_t to) {
  const std::size_t _remove = workers_.size() - to;
  auto _iterator = workers_.begin();
  for (std::size_t _index = 0; _index < _remove; ++_index) {
    _iterator = workers_.erase(_iterator);
  }
}

void queue::push_job(const std::shared_ptr<job>& job) {
  std::scoped_lock _lock(jobs_mutex_);
  jobs_.try_emplace(job->id(), job);
}

std::shared_ptr<worker> queue::get_worker() {
  auto _comparator = [](const auto& a, const auto& b) { return a.second->number_of_tasks() < b.second->number_of_tasks(); };
  auto _worker_iterator = std::ranges::min_element(workers_, _comparator);
  auto const& [_, _worker] = *_worker_iterator;
  boost::ignore_unused(_);
  return _worker;
}

std::shared_ptr<task> queue::get_task(const std::string& name) {
  const auto it = tasks_.find(name);
  if (it == tasks_.end())
    throw errors::task_not_found();
  return it->second;
}
}  // namespace engine
