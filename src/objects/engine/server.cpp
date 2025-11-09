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

#include <engine/controllers/auth/attempt_controller.hpp>
#include <engine/controllers/status_controller.hpp>
#include <engine/controllers/user_controller.hpp>
#include <engine/listener.hpp>
#include <engine/route.hpp>
#include <engine/router.hpp>
#include <engine/server.hpp>
#include <engine/signal_handler.hpp>
#include <engine/state.hpp>
#include <engine/task_group.hpp>

namespace engine {

server::server() : state_(std::make_shared<state>()), task_group_(std::make_shared<task_group>(state_->ioc().get_executor())) {}

void server::start(const unsigned short int port) const {
  auto const _address = boost::asio::ip::make_address("0.0.0.0");

  const auto _router = state_->get_router();

  _router->add(std::make_shared<route>(controllers::status_controller::verbs(), "/api/status", controllers::status_controller::make()))
      ->add(std::make_shared<route>(controllers::user_controller::verbs(), "/api/user", controllers::user_controller::make()))
      ->add(std::make_shared<route>(controllers::auth::attempt_controller::verbs(), "/api/auth/attempt",
                                    controllers::auth::attempt_controller::make()));
  ;

  co_spawn(make_strand(state_->ioc()), listener(*task_group_, state_, endpoint{_address, port}),
           task_group_->adapt([](const std::exception_ptr& throwable) {
             if (throwable) {
               try {
                 std::rethrow_exception(throwable);
               } catch (std::exception& exception) {
                 std::cerr << "Error in listener: " << exception.what() << "\n";
               }
             }
           }));

  co_spawn(make_strand(state_->ioc()), signal_handler(state_->ioc(), *task_group_), boost::asio::detached);

  state_->run();
}

shared_state server::get_state() const { return state_; }

shared_of<task_group> server::get_task_group() { return task_group_; }
}  // namespace engine
