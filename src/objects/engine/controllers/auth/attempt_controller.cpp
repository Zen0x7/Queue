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

#include <engine/auth.hpp>
#include <engine/controller.hpp>
#include <engine/controllers/auth/attempt_controller.hpp>
#include <engine/jwt.hpp>
#include <engine/state.hpp>

namespace engine::controllers::auth {
vector_of<http_verb> attempt_controller::verbs() {
  return vector_of{
      http_verb::post,
  };
}

shared_controller attempt_controller::make() {
  return std::make_shared<controller>(
      [](const shared_state &state, const request_type &request, const params_type &params,
         const shared_auth &auth) -> async_of<response_type> {
        const auto _id = boost::uuids::random_generator()();
        const auto _jwt = jwt::make(_id, state->get_key());
        response_type _response{http_status::ok, request.version()};
        const object _data = {{"data", {{"token", _jwt->as_string()}}}};
        _response.body() = serialize(_data);
        _response.prepare_payload();
        co_return _response;
      },
      controller_config{.validated_ = true,
                        .validation_rules_ = {
                            {"*", "is_object"},
                            {"email", "is_string"},
                            {"password", "is_string"},
                        }});
}
}  // namespace engine::controllers::auth
