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

#include <dotenv.h>

#include <engine/encoding.hpp>
#include <engine/queue.hpp>
#include <engine/router.hpp>
#include <engine/state.hpp>

namespace engine {
state::state() : router_(std::make_shared<router>()) {
  key_ = base64url_decode(dotenv::getenv("APP_KEY", "-66WcolkZd8-oHejFFj1EUhxg3-8UWErNkgMqCwLDEI"));
}

state::~state() {
  std::scoped_lock _lock(queues_mutex_);
  for (const auto& _queue : queues_ | std::views::values) {
    _queue->cancel();
  }
  queues_.clear();
}

bool state::get_running() const { return running_.load(std::memory_order_acquire); }

std::string state::get_key() const { return key_; }

unsigned short int state::get_port() const { return port_.load(std::memory_order_acquire); }

void state::set_port(const unsigned short int port) { port_.store(port, std::memory_order_release); }

void state::set_running(const bool running) { running_.store(running, std::memory_order_release); }

map_hash_of<std::string, shared_queue, std::less<>>& state::queues() noexcept { return queues_; }

shared_router state::get_router() const noexcept { return router_; }

shared_queue state::get_queue(const std::string& name) noexcept {
  std::scoped_lock _lock(queues_mutex_);
  auto [_it, _ignored] = queues_.try_emplace(name, std::make_shared<queue>(make_strand(ioc_)));
  boost::ignore_unused(_ignored);
  return _it->second;
}

bool state::remove_queue(const std::string& name) noexcept {
  std::scoped_lock _lock(queues_mutex_);
  return queues_.erase(name) == 1;
}

bool state::queue_exists(const std::string& name) noexcept {
  std::scoped_lock _lock(queues_mutex_);
  return queues_.contains(name);
}

void state::run() noexcept {
  boost::asio::signal_set _signals(this->ioc_, SIGINT, SIGTERM);
  _signals.async_wait(
      [&](boost::system::error_code const&, int)
      {
          this->ioc_.stop();
      });

  vector_of<std::jthread> _threads_container;
  const auto _threads = std::thread::hardware_concurrency();
  _threads_container.reserve(_threads);
  for (auto _i = _threads - 1; _i > 0; --_i) _threads_container.emplace_back([this] { this->ioc_.run(); });
  ioc_.run();
  for (auto& _thread : _threads_container) _thread.join();
}

boost::asio::io_context& state::ioc() noexcept { return ioc_; }
}  // namespace engine
