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

#include <queue/state.hpp>

namespace queue {
queue_container& state::queues() {
  return _queues;
}

shared_queue state::add_queue(std::string name) noexcept {
  std::scoped_lock _lock(_queues_mutex);

  if (_queues.contains(name)) {
    return _queues[name];
  }

  auto _queue = std::make_shared<queue>();
  _queues.emplace(name, _queue);
  return _queue;
}

shared_queue state::get_queue(const std::string& name) noexcept {
  if (queue_exists(name)) {
    return _queues[name];
  }
  return add_queue(name);
}

bool state::remove_queue(const std::string& name) noexcept {
  std::scoped_lock _lock(_queues_mutex);
  return _queues.erase(name) == 1;
}

bool state::queue_exists(const std::string& name) noexcept {
  std::scoped_lock _lock(_queues_mutex);
  return _queues.contains(name);
}
}  // namespace queue
