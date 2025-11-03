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

#ifndef QUEUE_JOB_HPP
#define QUEUE_JOB_HPP

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>

namespace queue {
using handler_type = std::function<void()>;

class job : public std::enable_shared_from_this<job> {
  boost::uuids::uuid id_ = boost::uuids::random_generator()();
  handler_type handler_;
  std::atomic<bool> started_{false};
  std::atomic<bool> finished_{false};
  std::chrono::system_clock::time_point started_at_;
  std::chrono::system_clock::time_point finished_at_;

 public:
  explicit job(handler_type handler);
  const boost::uuids::uuid& id() const noexcept;
  bool started() const noexcept;
  bool finished() const noexcept;
  std::chrono::system_clock::time_point started_at() const noexcept;
  std::chrono::system_clock::time_point finished_at() const noexcept;
  void run() noexcept;
};

using shared_job = std::shared_ptr<job>;
}  // namespace queue

#endif  // QUEUE_JOB_HPP
