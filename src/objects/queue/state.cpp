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

#include <boost/core/ignore_unused.hpp>
#include <queue/state.hpp>

namespace queue {
state::~state() {
  std::scoped_lock _lock(queues_mutex_);
  queues_.clear();
}

queue_container& state::queues() noexcept {
  return queues_;
}

shared_queue state::add_queue(const std::string& name) noexcept {
  std::scoped_lock _lock(queues_mutex_);
  auto [_it, _ignored] =
      queues_.try_emplace(name, std::make_shared<queue>(make_strand(ioc_)));
  boost::ignore_unused(_ignored);
  return _it->second;
}

shared_queue state::get_queue(const std::string& name) noexcept {
  if (queue_exists(name)) {
    return queues_[name];
  }
  return add_queue(name);
}

bool state::remove_queue(const std::string& name) noexcept {
  std::scoped_lock _lock(queues_mutex_);
  return queues_.erase(name) == 1;
}

bool state::queue_exists(const std::string& name) noexcept {
  std::scoped_lock _lock(queues_mutex_);
  return queues_.contains(name);
}
}  // namespace queue
