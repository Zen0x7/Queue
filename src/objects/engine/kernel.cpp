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

#include <engine/auth.hpp>
#include <engine/controller.hpp>
#include <engine/errors/not_found_error.hpp>
#include <engine/jwt.hpp>
#include <engine/kernel.hpp>
#include <engine/route.hpp>
#include <engine/router.hpp>
#include <engine/state.hpp>
#include <engine/validator.hpp>

namespace engine {
async_of<message> kernel(shared_state state, request_type request) {
  using enum http_field;

  if (request.method() == http_verb::options) {
    auto _verbs = state->get_router()->methods_of(request.target());
    const auto _methods = boost::join(_verbs, ",");
    response_empty_type _response{http_status::no_content, request.version()};
    _response.set(access_control_allow_methods, _methods.empty() ? "" : _methods);
    _response.set(access_control_allow_headers, "Accept,Authorization,Content-Type");
    _response.set(access_control_allow_origin, "*");
    co_return _response;
  }

  try {
    auto [_params, _route] = state->get_router()->find(request.method(), request.target());
    auto _controller = _route->get_controller();
    auto _auth = std::make_unique<auth>();
    if (_controller->config().authenticated_) {
      if (request[authorization].empty()) {
        response_empty_type _response{http_status::unauthorized, request.version()};
        _response.set(access_control_allow_origin, "*");
        _response.prepare_payload();
        co_return _response;
      }
      try {
        std::string _bearer{request[authorization]};
        _auth->set_jwt(jwt::from(_bearer, state->get_key()));
      } catch (...) {
        response_empty_type _response{http_status::unauthorized, request.version()};
        _response.set(access_control_allow_origin, "*");
        _response.prepare_payload();
        co_return _response;
      }
    }
    auto _response = co_await _controller->callback()(state, std::move(request), std::move(_params), std::move(_auth));
    _response.set(access_control_allow_origin, "*");
    co_return _response;
  } catch (const errors::not_found_error &) {
    response_empty_type _response{http_status::not_found, request.version()};
    _response.set(access_control_allow_origin, "*");
    _response.prepare_payload();
    co_return _response;
  } catch (...) {
    response_empty_type _response{http_status::internal_server_error, request.version()};
    _response.set(access_control_allow_origin, "*");
    _response.prepare_payload();
    co_return _response;
  }
}
}  // namespace engine
