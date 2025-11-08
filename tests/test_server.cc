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

#include <engine/controller.hpp>
#include <engine/jwt.hpp>
#include <engine/route.hpp>
#include <engine/router.hpp>
#include <engine/server.hpp>
#include <engine/state.hpp>
#include <engine/support.hpp>

using namespace engine;

class test_server : public testing::Test {
 public:
  shared_of<server> server_;
  shared_of<std::jthread> thread_;

 protected:
  void SetUp() override {
    server_ = std::make_shared<server>();

    auto _router = server_->get_state()->get_router();

    _router->add(std::make_shared<route>(
        vector_of{
            http_verb::get,
        },
        "/system_error",
        std::make_shared<controller>([](const shared_state &state, const request_type request, params_type params,
                                        shared_auth auth) -> async_of<response_type> {
          response_empty_type _response{http_status::ok, request.version()};
          _response.prepare_payload();
          throw std::system_error();
          co_return _response;
        })));

    thread_ = std::make_shared<std::jthread>([this]() { server_->start(); });

    thread_->detach();

    while (server_->get_state()->get_running() == false) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  void TearDown() override { server_->get_state()->ioc().stop(); }
};

TEST_F(test_server, can_handle_http_request) {
  boost::asio::io_context _client_ioc;
  resolver _resolver(_client_ioc);
  const std::string _host = "127.0.0.1";
  const unsigned short int _port = server_->get_state()->get_port();
  auto const _tcp_resolver_results = _resolver.resolve(_host, std::to_string(_port));
  tcp_stream _stream(_client_ioc);
  _stream.connect(_tcp_resolver_results);

  request_type _request{http_verb::get, "/api/status", 11};
  _request.set(http_field::host, _host);
  _request.set(http_field::user_agent, "Client");
  _request.prepare_payload();

  write(_stream, _request);
  flat_buffer _buffer;

  response_type _response;
  read(_stream, _buffer, _response);

  ASSERT_EQ(_response.body().size(), 0);
  ASSERT_EQ(_response.result_int(), 200);

  boost::beast::error_code _ec;
  _stream.socket().shutdown(socket::shutdown_both, _ec);
  ASSERT_EQ(_ec, boost::beast::errc::success);
}

TEST_F(test_server, can_handle_unauthorized_requests) {
  boost::asio::io_context _client_ioc;
  resolver _resolver(_client_ioc);
  const std::string _host = "127.0.0.1";
  const unsigned short int _port = server_->get_state()->get_port();
  auto const _tcp_resolver_results = _resolver.resolve(_host, std::to_string(_port));
  tcp_stream _stream(_client_ioc);
  _stream.connect(_tcp_resolver_results);

  request_type _request{http_verb::get, "/api/user", 11};
  _request.set(http_field::host, _host);
  _request.set(http_field::user_agent, "Client");
  _request.prepare_payload();

  write(_stream, _request);
  flat_buffer _buffer;

  response_type _response;
  read(_stream, _buffer, _response);

  ASSERT_EQ(_response.body().size(), 0);
  ASSERT_EQ(_response.result_int(), 401);

  boost::beast::error_code _ec;
  _stream.socket().shutdown(socket::shutdown_both, _ec);
  ASSERT_EQ(_ec, boost::beast::errc::success);
}

TEST_F(test_server, can_handle_get_user_request) {
  boost::asio::io_context _client_ioc;
  resolver _resolver(_client_ioc);
  const std::string _host = "127.0.0.1";
  const unsigned short int _port = server_->get_state()->get_port();
  auto const _tcp_resolver_results = _resolver.resolve(_host, std::to_string(_port));
  tcp_stream _stream(_client_ioc);
  _stream.connect(_tcp_resolver_results);

  auto _id = boost::uuids::random_generator()();
  auto _jwt = jwt::make(_id, server_->get_state()->get_key());
  request_type _request{http_verb::get, "/api/user", 11};
  _request.set(http_field::host, _host);
  _request.set(http_field::user_agent, "Client");
  _request.set(http_field::authorization, _jwt->as_string());
  _request.prepare_payload();

  write(_stream, _request);
  flat_buffer _buffer;

  response_type _response;
  read(_stream, _buffer, _response);

  ASSERT_EQ(_response.body().size(), 54);
  ASSERT_EQ(_response.result_int(), 200);

  boost::system::error_code _parse_ec;
  auto _result = boost::json::parse(_response.body(), _parse_ec);

  ASSERT_EQ(_parse_ec, boost::beast::errc::success);
  ASSERT_TRUE(_result.is_object());
  ASSERT_TRUE(_result.as_object().contains("data"));
  ASSERT_TRUE(_result.as_object().at("data").is_object());
  ASSERT_TRUE(_result.as_object().at("data").as_object().contains("id"));
  ASSERT_TRUE(_result.as_object().at("data").as_object().at("id").is_string());
  std::string _auth_id{_result.as_object().at("data").as_object().at("id").as_string()};
  ASSERT_EQ(_auth_id, to_string(_id));

  boost::beast::error_code _ec;
  _stream.socket().shutdown(socket::shutdown_both, _ec);
  ASSERT_EQ(_ec, boost::beast::errc::success);
}

TEST_F(test_server, can_throw_unauthorized_on_invalid_tokens) {
  boost::asio::io_context _client_ioc;
  resolver _resolver(_client_ioc);
  const std::string _host = "127.0.0.1";
  const unsigned short int _port = server_->get_state()->get_port();
  auto const _tcp_resolver_results = _resolver.resolve(_host, std::to_string(_port));
  tcp_stream _stream(_client_ioc);
  _stream.connect(_tcp_resolver_results);

  request_type _request{http_verb::get, "/api/user", 11};
  _request.set(http_field::host, _host);
  _request.set(http_field::user_agent, "Client");
  _request.set(http_field::authorization, "Bearer ...");
  _request.prepare_payload();

  write(_stream, _request);
  flat_buffer _buffer;

  response_type _response;
  read(_stream, _buffer, _response);

  ASSERT_EQ(_response.body().size(), 0);
  ASSERT_EQ(_response.result_int(), 401);

  boost::beast::error_code _ec;
  _stream.socket().shutdown(socket::shutdown_both, _ec);
  ASSERT_EQ(_ec, boost::beast::errc::success);
}

TEST_F(test_server, can_timeout_http_sessions) {
  boost::asio::io_context _client_ioc;
  resolver _resolver(_client_ioc);
  const std::string _host = "127.0.0.1";
  const unsigned short int _port = server_->get_state()->get_port();
  auto const _tcp_resolver_results = _resolver.resolve(_host, std::to_string(_port));
  tcp_stream _stream(_client_ioc);
  _stream.connect(_tcp_resolver_results);

  request_type _request{http_verb::get, "/api/status", 11};
  _request.set(http_field::host, _host);
  _request.set(http_field::user_agent, "Client");
  _request.prepare_payload();

  write(_stream, _request);
  flat_buffer _buffer;

  response_type _response;
  read(_stream, _buffer, _response);

  ASSERT_EQ(_response.body().size(), 0);
  ASSERT_EQ(_response.result_int(), 200);

  std::this_thread::sleep_for(std::chrono::seconds(6));

  boost::beast::error_code _write_ec;
  write(_stream, _request, _write_ec);
  ASSERT_EQ(_write_ec, boost::beast::errc::success);

  boost::beast::error_code _disconnect_ec;
  _stream.socket().shutdown(socket::shutdown_both, _disconnect_ec);

  ASSERT_EQ(_disconnect_ec, boost::beast::errc::not_connected);
}

TEST_F(test_server, can_handle_http_cors_request) {
  boost::asio::io_context _client_ioc;
  resolver _resolver(_client_ioc);
  const std::string _host = "127.0.0.1";
  const unsigned short int _port = server_->get_state()->get_port();
  auto const _tcp_resolver_results = _resolver.resolve(_host, std::to_string(_port));
  tcp_stream _stream(_client_ioc);
  _stream.connect(_tcp_resolver_results);

  request_type _request{http_verb::options, "/api/status", 11};
  _request.set(http_field::host, _host);
  _request.set(http_field::user_agent, "Client");
  _request.prepare_payload();

  write(_stream, _request);
  flat_buffer _buffer;

  response_type _response;
  read(_stream, _buffer, _response);

  ASSERT_EQ(_response.body().size(), 0);
  ASSERT_EQ(_response.result_int(), 204);
  ASSERT_EQ(_response[http_field::access_control_allow_methods], "GET");

  request_type _another_request{http_verb::options, "/not-found", 11};
  _another_request.set(http_field::host, _host);
  _another_request.set(http_field::user_agent, "Client");
  _another_request.prepare_payload();

  write(_stream, _another_request);
  flat_buffer _another_buffer;

  response_type _another_response;
  read(_stream, _another_buffer, _another_response);

  ASSERT_EQ(_another_response.body().size(), 0);
  ASSERT_EQ(_another_response.result_int(), 204);
  ASSERT_EQ(_another_response[http_field::access_control_allow_methods], "");

  boost::beast::error_code _ec;
  _stream.socket().shutdown(socket::shutdown_both, _ec);
  ASSERT_EQ(_ec, boost::beast::errc::success);
}

TEST_F(test_server, can_handle_exceptions) {
  boost::asio::io_context _client_ioc;
  resolver _resolver(_client_ioc);
  const std::string _host = "127.0.0.1";
  const unsigned short int _port = server_->get_state()->get_port();
  auto const _tcp_resolver_results = _resolver.resolve(_host, std::to_string(_port));
  tcp_stream _stream(_client_ioc);
  _stream.connect(_tcp_resolver_results);

  request_type _request{http_verb::get, "/system_error", 11};
  _request.set(http_field::host, _host);
  _request.set(http_field::user_agent, "Client");
  _request.prepare_payload();

  write(_stream, _request);
  flat_buffer _buffer;

  response_type _response;
  read(_stream, _buffer, _response);

  ASSERT_EQ(_response.body().size(), 0);
  ASSERT_EQ(_response.result_int(), 500);

  boost::beast::error_code _ec;
  _stream.socket().shutdown(socket::shutdown_both, _ec);
  ASSERT_EQ(_ec, boost::beast::errc::success);
}