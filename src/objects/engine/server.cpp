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
#include <boost/asio/detached.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <engine/controllers/status_controller.hpp>
#include <engine/listener.hpp>
#include <engine/router.hpp>
#include <engine/server.hpp>
#include <engine/state.hpp>

namespace engine {

server::server() : state_(std::make_shared<state>()) {}

void server::start(const unsigned short int port) const {
  auto const _address = boost::asio::ip::make_address("0.0.0.0");

  const auto _router = state_->get_router();

  _router->add(std::make_shared<route>(controllers::status_controller::verbs(),
                                       "/status",
                                       controllers::status_controller::make()));

  co_spawn(state_->ioc(),
           listener(state_, boost::asio::ip::tcp::endpoint{_address, port}),
           boost::asio::detached);

  state_->run();
}

std::shared_ptr<state> server::get_state() const { return state_; }
}  // namespace engine
