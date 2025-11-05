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

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ip/address.hpp>
#include <engine/listener.hpp>
#include <engine/server.hpp>
#include <engine/state.hpp>
#include <iostream>

namespace engine {

server::server() : state_(std::make_shared<state>()) {}

void server::start(const unsigned short int port) const {
  auto const _address = boost::asio::ip::make_address("0.0.0.0");

  co_spawn(state_->ioc(),
           listener(state_, boost::asio::ip::tcp::endpoint{_address, port}),
           [](const std::exception_ptr &throwable) {
                  if (throwable) {
                    try {
                      std::rethrow_exception(throwable);
                    } catch (const std::system_error &exception) {
                        std::cerr << "[Server] System error: " << exception.what() << std::endl;
                    } catch (const boost::system::system_error & exception) {
                        std::cerr << "[Server] Boost error: " << exception.what() << std::endl;
                    }catch (...) {
                      std::cerr << "[Server] Unknown exception thrown." << std::endl;
                    }
                  }
                });

  state_->run();
}

std::shared_ptr<state> server::get_state() const { return state_; }
}  // namespace engine
