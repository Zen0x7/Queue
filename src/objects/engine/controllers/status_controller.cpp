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

#include <boost/beast/http/empty_body.hpp>
#include <engine/controllers/status_controller.hpp>

namespace engine::controllers {
std::vector<boost::beast::http::verb> status_controller::verbs() {
  return std::vector{
      boost::beast::http::verb::get,
  };
}

std::shared_ptr<controller> status_controller::make() {
  return std::make_shared<controller>(
      [](const std::shared_ptr<state> &state,
         const boost::beast::http::request<boost::beast::http::string_body>
             request,
         std::unordered_map<std::string, std::string, string_hasher,
                            std::equal_to<>>
             params)
          -> boost::asio::awaitable<
              boost::beast::http::response<boost::beast::http::string_body>> {
        boost::beast::http::response<boost::beast::http::empty_body> _response{
            boost::beast::http::status::ok, request.version()};
        _response.prepare_payload();
        co_return _response;
      });
}
}  // namespace engine::controllers
