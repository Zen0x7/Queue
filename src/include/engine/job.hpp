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

#ifndef ENGINE_JOB_HPP
#define ENGINE_JOB_HPP

#include <atomic>
#include <boost/asio/awaitable.hpp>
#include <boost/json/object.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <chrono>
#include <memory>

namespace engine {
class task;

class job : public std::enable_shared_from_this<job> {
  boost::uuids::uuid id_ = boost::uuids::random_generator()();
  std::shared_ptr<task> task_;
  std::atomic<bool> started_{false};
  std::atomic<bool> cancelled_{false};
  std::atomic<bool> finished_{false};
  std::atomic<bool> failed_{false};
  std::exception_ptr exception_;
  std::chrono::system_clock::time_point started_at_;
  std::chrono::system_clock::time_point cancelled_at_;
  std::chrono::system_clock::time_point finished_at_;
  boost::json::object data_;

 public:
  explicit job(const std::shared_ptr<task>& task, boost::json::object data);
  const boost::uuids::uuid& id() const noexcept;
  bool started() const noexcept;
  bool failed() const noexcept;
  bool finished() const noexcept;
  bool cancelled() const noexcept;
  std::exception_ptr exception() const noexcept;
  std::chrono::system_clock::time_point started_at() const noexcept;
  std::chrono::system_clock::time_point finished_at() const noexcept;
  boost::asio::awaitable<void> run() noexcept;
  void cancel() noexcept;
};
}  // namespace engine

#endif  // ENGINE_JOB_HPP
