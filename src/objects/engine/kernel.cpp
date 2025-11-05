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

#include <boost/algorithm/string/join.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <engine/controller.hpp>
#include <engine/errors/not_found_error.hpp>
#include <engine/kernel.hpp>
#include <engine/route.hpp>
#include <engine/router.hpp>
#include <engine/state.hpp>

namespace engine {
boost::asio::awaitable<boost::beast::http::message_generator> kernel(
    std::shared_ptr<state> state,
    boost::beast::http::request<boost::beast::http::string_body> request) {
  if (request.method() == boost::beast::http::verb::options) {
    auto _verbs = state->get_router()->methods_of(request.target());
    const auto _methods = boost::join(_verbs, ",");
    boost::beast::http::response<boost::beast::http::empty_body> _response{
        boost::beast::http::status::no_content, request.version()};
    _response.set(boost::beast::http::field::access_control_allow_methods,
                  _methods.empty() ? "" : _methods);
    _response.set(boost::beast::http::field::access_control_allow_headers,
                  "Accept,Authorization,Content-Type");
    _response.set(boost::beast::http::field::access_control_allow_origin, "*");
    co_return _response;
  }

  try {
    auto [_params, _route] =
        state->get_router()->find(request.method(), request.target());
    auto _response =
        co_await _route->get_controller()->callback()(state, request);
    _response.set(boost::beast::http::field::access_control_allow_origin, "*");
    co_return _response;
  } catch (const errors::not_found_error &) {
    boost::beast::http::response<boost::beast::http::empty_body> _response{
        boost::beast::http::status::not_found, request.version()};
    _response.set(boost::beast::http::field::access_control_allow_origin, "*");
    _response.prepare_payload();
    co_return _response;
  }
}
}  // namespace engine
