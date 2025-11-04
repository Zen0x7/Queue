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

#include <engine/errors/job_cancelled.hpp>
#include <engine/job.hpp>
#include <engine/task.hpp>

namespace engine {
job::job(const std::shared_ptr<task>& task, boost::json::object data) : task_(task), data_(std::move(data)) {}

const boost::uuids::uuid& job::id() const noexcept {
  return id_;
}

bool job::started() const noexcept {
  return started_.load(std::memory_order_acquire);
}

bool job::failed() const noexcept {
  return failed_.load(std::memory_order_acquire);
}

bool job::finished() const noexcept {
  return finished_.load(std::memory_order_acquire);
}

bool job::cancelled() const noexcept {
  return cancelled_.load(std::memory_order_acquire);
}

std::exception_ptr job::exception() const noexcept {
  return exception_;
}

std::chrono::system_clock::time_point job::started_at() const noexcept {
  return started_at_;
}

std::chrono::system_clock::time_point job::finished_at() const noexcept {
  return finished_at_;
}

boost::asio::awaitable<void> job::run() noexcept {
  started_.store(true, std::memory_order_release);
  started_at_ = std::chrono::system_clock::now();

  try {
    if (cancelled_.load(std::memory_order_acquire)) {
      throw errors::job_cancelled();
    }
    co_await (*task_->callback())(cancelled_, data_);
  } catch (errors::job_cancelled&) {
    cancelled_.store(true, std::memory_order_release);
    cancelled_at_ = std::chrono::system_clock::now();
  } catch (...) {
    failed_.store(true, std::memory_order_release);
    exception_ = std::current_exception();
  }

  finished_.store(true, std::memory_order_release);
  finished_at_ = std::chrono::system_clock::now();
  co_return;
}

void job::cancel() noexcept {
  cancelled_.store(true, std::memory_order_release);
}
}  // namespace engine
