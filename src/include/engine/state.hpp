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

#ifndef ENGINE_STATE_HPP
#define ENGINE_STATE_HPP

#include <boost/asio/io_context.hpp>
#include <map>
#include <memory>
#include <mutex>

namespace engine {
class queue;

class state : public std::enable_shared_from_this<state> {
  std::map<std::string, std::shared_ptr<queue>, std::less<>> queues_;
  std::mutex queues_mutex_;
  boost::asio::io_context ioc_;

 public:
  std::atomic<bool> running_{false};
  std::atomic<unsigned short int> port_{0};

  ~state();
  std::map<std::string, std::shared_ptr<queue>, std::less<>>& queues() noexcept;
  std::shared_ptr<queue> get_queue(const std::string& name) noexcept;
  bool remove_queue(const std::string& name) noexcept;
  bool queue_exists(const std::string& name) noexcept;
  void run() noexcept;
  boost::asio::io_context& ioc() noexcept;
};
}  // namespace engine

#endif  // ENGINE_STATE_HPP
