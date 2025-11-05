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

#include <gtest/gtest.h>

#include <boost/beast/http/empty_body.hpp>
#include <engine/errors/not_found_error.hpp>
#include <engine/route.hpp>
#include <engine/router.hpp>
#include <engine/state.hpp>
#include <functional>

TEST(router, can_resolve_requests) {
  const auto _router = std::make_shared<engine::router>();

  _router
      ->add(std::make_shared<engine::route>(
          std::vector{
              boost::beast::http::verb::post,
          },
          "/transactions/{id}",
          std::make_shared<engine::controller>(
              [](const std::shared_ptr<engine::state> &state,
                 const boost::beast::http::request<
                     boost::beast::http::string_body>
                     request,
                 std::unordered_map<std::string, std::string, string_hasher,
                                    std::equal_to<>>
                     params)
                  -> boost::asio::awaitable<boost::beast::http::response<
                      boost::beast::http::string_body>> {
                boost::beast::http::response<boost::beast::http::empty_body>
                    _response{boost::beast::http::status::ok,
                              request.version()};
                _response.prepare_payload();
                co_return _response;
              })))
      ->add(std::make_shared<engine::route>(
          std::vector{
              boost::beast::http::verb::delete_,
          },
          "/users/{id}",
          std::make_shared<engine::controller>(
              [](const std::shared_ptr<engine::state> &state,
                 const boost::beast::http::request<
                     boost::beast::http::string_body>
                     request,
                 std::unordered_map<std::string, std::string, string_hasher,
                                    std::equal_to<>>
                     params)
                  -> boost::asio::awaitable<boost::beast::http::response<
                      boost::beast::http::string_body>> {
                boost::beast::http::response<boost::beast::http::empty_body>
                    _response{boost::beast::http::status::ok,
                              request.version()};
                _response.prepare_payload();
                co_return _response;
              })));

  {
    auto [_params, _route] =
        _router->find(boost::beast::http::verb::delete_, "/users/5");
    ASSERT_EQ(_params.size(), 1);
    ASSERT_EQ(_params.at("id"), "5");
  }

  {
    bool throws = false;
    try {
      auto [_params, _route] =
          _router->find(boost::beast::http::verb::post, "/users/5");
    } catch (const engine::errors::not_found_error &) {
      throws = true;
    }
    ASSERT_TRUE(throws);
  }

  {
    auto [_params, _route] =
        _router->find(boost::beast::http::verb::post, "/transactions/7");
    ASSERT_EQ(_params.size(), 1);
    ASSERT_EQ(_params.at("id"), "7");
  }

  {
    bool throws = false;
    try {
      auto [_params, _route] =
          _router->find(boost::beast::http::verb::delete_, "/transactions/5");
    } catch (const engine::errors::not_found_error &) {
      throws = true;
    }
    ASSERT_TRUE(throws);
  }

  {
    bool throws = false;
    try {
      auto [_params, _route] =
          _router->find(boost::beast::http::verb::post, "/not_found");
    } catch (const engine::errors::not_found_error &) {
      throws = true;
    }
    ASSERT_TRUE(throws);
  }
}