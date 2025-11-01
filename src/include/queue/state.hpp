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

#ifndef QUEUE_STATE_HPP
#define QUEUE_STATE_HPP

#include <boost/asio/io_context.hpp>
#include <memory>
#include <mutex>

#include <queue/queue.hpp>

namespace queue {
class state : public std::enable_shared_from_this<state> {
  queue_container queues_;
  std::mutex queues_mutex_;
  boost::asio::io_context ioc_;

 public:
  ~state();
  queue_container& queues() noexcept;
  shared_queue add_queue(const std::string& name) noexcept;
  shared_queue get_queue(const std::string& name) noexcept;
  bool remove_queue(const std::string& name) noexcept;
  bool queue_exists(const std::string& name) noexcept;
};

using shared_state = std::shared_ptr<state>;
}  // namespace queue

#endif  // QUEUE_STATE_HPP
