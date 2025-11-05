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

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/message_generator.hpp>
#include <boost/beast/http/string_body.hpp>
#include <engine/controller.hpp>
#include <engine/errors/parse_error.hpp>
#include <engine/route.hpp>
#include <engine/state.hpp>
#include <functional>

TEST(route, can_be_instanced) {
  auto _controller = std::make_shared<engine::controller>(
      [](const std::shared_ptr<engine::state> &state,
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

  engine::route _route(
      {
          boost::beast::http::verb::get,
          boost::beast::http::verb::post,
      },
      "/hello-world",
      std::make_shared<engine::controller>(
          [](const std::shared_ptr<engine::state> &state,
             const boost::beast::http::request<boost::beast::http::string_body>
                 request,
             std::unordered_map<std::string, std::string, string_hasher,
                                std::equal_to<>>
                 params)
              -> boost::asio::awaitable<boost::beast::http::response<
                  boost::beast::http::string_body>> {
            boost::beast::http::response<boost::beast::http::empty_body>
                _response{boost::beast::http::status::ok, request.version()};
            _response.prepare_payload();
            co_return _response;
          }));

  ASSERT_EQ(_route.get_parameters().size(), 0);
  ASSERT_EQ(_route.get_verbs().size(), 2);
}

TEST(route, can_be_compiled) {
  auto _controller = std::make_shared<engine::controller>(
      [](const std::shared_ptr<engine::state> &state,
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

  engine::route _route(
      {
          boost::beast::http::verb::get,
          boost::beast::http::verb::post,
      },
      "/api/users/{user_id}/configurations",
      std::make_shared<engine::controller>(
          [](const std::shared_ptr<engine::state> &state,
             const boost::beast::http::request<boost::beast::http::string_body>
                 request,
             std::unordered_map<std::string, std::string, string_hasher,
                                std::equal_to<>>
                 params)
              -> boost::asio::awaitable<boost::beast::http::response<
                  boost::beast::http::string_body>> {
            boost::beast::http::response<boost::beast::http::empty_body>
                _response{boost::beast::http::status::ok, request.version()};
            _response.prepare_payload();
            co_return _response;
          }));

  ASSERT_EQ(_route.get_parameters().size(), 1);
  ASSERT_EQ(_route.get_verbs().size(), 2);
}

TEST(route, can_be_invoked) {
  boost::asio::io_context _ioc;

  auto _controller = std::make_shared<engine::controller>(
      [](const std::shared_ptr<engine::state> &state,
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

  auto _executed = std::make_shared<std::atomic<bool>>(false);

  engine::route _route(
      {
          boost::beast::http::verb::get,
          boost::beast::http::verb::post,
      },
      "/endpoint",
      std::make_shared<engine::controller>(
          [&_executed](
              const std::shared_ptr<engine::state> &state,
              const boost::beast::http::request<boost::beast::http::string_body>
                  request,
              std::unordered_map<std::string, std::string, string_hasher,
                                 std::equal_to<>>
                  params)
              -> boost::asio::awaitable<boost::beast::http::response<
                  boost::beast::http::string_body>> {
            boost::beast::http::response<boost::beast::http::empty_body>
                _response{boost::beast::http::status::ok, request.version()};
            std::cout << "Inside of controller ..." << std::endl;
            _response.prepare_payload();
            _executed->store(true, std::memory_order_release);
            co_return _response;
          }));

  auto _callback = _route.get_controller()->callback();

  boost::beast::http::request<boost::beast::http::string_body> _request{
      boost::beast::http::verb::get, "/status", 11};
  _request.set(boost::beast::http::field::host, "127.0.0.1");
  _request.set(boost::beast::http::field::user_agent, "Client");
  _request.prepare_payload();

  const auto _state = std::make_shared<engine::state>();
  std::unordered_map<std::string, std::string, string_hasher, std::equal_to<>>
      _params;
  co_spawn(_ioc, (_callback)(_state, _request, _params), boost::asio::detached);
  _ioc.run();
  ASSERT_TRUE(_executed->load(std::memory_order_acquire));
}

TEST(route, can_be_matched) {
  auto _controller = std::make_shared<engine::controller>(
      [](const std::shared_ptr<engine::state> &state,
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

  auto _executed = std::make_shared<std::atomic<bool>>(false);

  engine::route _route(
      {
          boost::beast::http::verb::get,
          boost::beast::http::verb::post,
      },
      "/parameters/{1}/{2}/{3}",
      std::make_shared<engine::controller>(
          [&_executed](
              const std::shared_ptr<engine::state> &state,
              const boost::beast::http::request<boost::beast::http::string_body>
                  request,
              std::unordered_map<std::string, std::string, string_hasher,
                                 std::equal_to<>>
                  params)
              -> boost::asio::awaitable<boost::beast::http::response<
                  boost::beast::http::string_body>> {
            boost::beast::http::response<boost::beast::http::empty_body>
                _response{boost::beast::http::status::ok, request.version()};
            std::cout << "Inside of controller ..." << std::endl;
            _response.prepare_payload();
            _executed->store(true, std::memory_order_release);
            co_return _response;
          }));

  auto [_matched, _parameters] = _route.match("/parameters/4/5/6");
  ASSERT_TRUE(_matched);
  ASSERT_TRUE(_parameters.contains("1"));
  ASSERT_TRUE(_parameters.contains("2"));
  ASSERT_TRUE(_parameters.contains("3"));
  ASSERT_FALSE(_parameters.contains("4"));
  ASSERT_FALSE(_parameters.contains("5"));
  ASSERT_FALSE(_parameters.contains("6"));
  ASSERT_EQ(_parameters.at("1"), "4");
  ASSERT_EQ(_parameters.at("2"), "5");
  ASSERT_EQ(_parameters.at("3"), "6");

  auto [_not_matches, _] = _route.match("/parameters/7/8");
  ASSERT_FALSE(_not_matches);
}

TEST(route, throw_error_on_duplicated_parameters) {
  auto _controller = std::make_shared<engine::controller>(
      [](const std::shared_ptr<engine::state> &state,
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

  auto _executed = std::make_shared<std::atomic<bool>>(false);

  bool throws = false;

  try {
    engine::route _route(
        {
            boost::beast::http::verb::get,
            boost::beast::http::verb::post,
        },
        "/parameters/{1}/{1}/{1}",
        std::make_shared<engine::controller>(
            [&_executed](const std::shared_ptr<engine::state> &state,
                         const boost::beast::http::request<
                             boost::beast::http::string_body>
                             request,
                         std::unordered_map<std::string, std::string,
                                            string_hasher, std::equal_to<>>
                             params)
                -> boost::asio::awaitable<boost::beast::http::response<
                    boost::beast::http::string_body>> {
              boost::beast::http::response<boost::beast::http::empty_body>
                  _response{boost::beast::http::status::ok, request.version()};
              std::cout << "Inside of controller ..." << std::endl;
              _response.prepare_payload();
              _executed->store(true, std::memory_order_release);
              co_return _response;
            }));
  } catch (const engine::errors::parse_error &e) {
    std::cout << e.what() << std::endl;
    throws = true;
  }

  ASSERT_TRUE(throws);
}
