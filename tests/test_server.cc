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
#include <engine/task_group.hpp>

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
        std::make_shared<controller>([](const shared_state &state, const request_type &request, const params_type &params,
                                        const shared_auth &auth) -> async_of<response_type> {
          response_empty_type _response{http_status::ok, request.version()};
          _response.prepare_payload();
          throw std::system_error();
          co_return _response;
        })));

    thread_ = std::make_shared<std::jthread>([this]() {
      server_->start();
      server_->get_state()->set_running(false);
    });

    thread_->detach();

    while (server_->get_state()->get_running() == false) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

  void TearDown() override {
    server_->get_task_group()->emit(boost::asio::cancellation_type::total);
    while (server_->get_state()->get_running() == true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    server_->get_state()->ioc().stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
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

TEST_F(test_server, can_handle_post_auth_attempt_request) {
  boost::asio::io_context _client_ioc;
  resolver _resolver(_client_ioc);
  const std::string _host = "127.0.0.1";
  const unsigned short int _port = server_->get_state()->get_port();
  auto const _tcp_resolver_results = _resolver.resolve(_host, std::to_string(_port));
  tcp_stream _stream(_client_ioc);
  _stream.connect(_tcp_resolver_results);

  request_type _request{http_verb::post, "/api/auth/attempt", 11};
  _request.set(http_field::host, _host);
  _request.set(http_field::user_agent, "Client");
  _request.body() = serialize(boost::json::object({
      {"email", "contact@zendev.cl"},
      {"password", "super_secret_password"},
  }));
  _request.prepare_payload();

  write(_stream, _request);
  flat_buffer _buffer;

  response_type _response;
  read(_stream, _buffer, _response);

  ASSERT_GT(_response.body().size(), 0);
  ASSERT_EQ(_response.result_int(), 200);

  boost::system::error_code _parse_ec;
  auto _result = boost::json::parse(_response.body(), _parse_ec);

  ASSERT_EQ(_parse_ec, boost::beast::errc::success);
  ASSERT_TRUE(_result.is_object());
  ASSERT_TRUE(_result.as_object().contains("data"));
  ASSERT_TRUE(_result.as_object().at("data").is_object());
  ASSERT_TRUE(_result.as_object().at("data").as_object().contains("token"));
  ASSERT_TRUE(_result.as_object().at("data").as_object().at("token").is_string());
  std::string _auth_id{_result.as_object().at("data").as_object().at("token").as_string()};

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

TEST_F(test_server, can_throw_unprocessable_entity_on_invalid_body) {
  boost::asio::io_context _client_ioc;
  resolver _resolver(_client_ioc);
  const std::string _host = "127.0.0.1";
  const unsigned short int _port = server_->get_state()->get_port();
  auto const _tcp_resolver_results = _resolver.resolve(_host, std::to_string(_port));
  tcp_stream _stream(_client_ioc);
  _stream.connect(_tcp_resolver_results);

  request_type _request{http_verb::post, "/api/auth/attempt", 11};
  _request.set(http_field::host, _host);
  _request.set(http_field::user_agent, "Client");
  _request.body() = "";
  _request.prepare_payload();

  write(_stream, _request);
  flat_buffer _buffer;

  response_type _response;
  read(_stream, _buffer, _response);

  ASSERT_GT(_response.body().size(), 0);
  ASSERT_EQ(_response.result_int(), 422);

  boost::system::error_code _parse_ec;
  auto _result = boost::json::parse(_response.body(), _parse_ec);

  ASSERT_EQ(_parse_ec, boost::beast::errc::success);
  ASSERT_TRUE(_result.is_object());
  ASSERT_TRUE(_result.as_object().contains("message"));
  ASSERT_TRUE(_result.as_object().at("message").is_string());
  ASSERT_EQ(_result.as_object().at("message").as_string(), "The given data was invalid.");
  ASSERT_TRUE(_result.as_object().contains("errors"));
  ASSERT_TRUE(_result.as_object().at("errors").is_object());
  ASSERT_TRUE(_result.as_object().at("errors").as_object().contains("*"));
  ASSERT_TRUE(_result.as_object().at("errors").as_object().at("*").is_array());
  ASSERT_EQ(_result.as_object().at("errors").as_object().at("*").as_array().size(), 1);
  ASSERT_TRUE(_result.as_object().at("errors").as_object().at("*").as_array().at(0).is_string());
  ASSERT_EQ(_result.as_object().at("errors").as_object().at("*").as_array().at(0).as_string(), "The payload must be a valid json value.");

  boost::beast::error_code _ec;
  _stream.socket().shutdown(socket::shutdown_both, _ec);
  ASSERT_EQ(_ec, boost::beast::errc::success);
}

TEST_F(test_server, can_throw_unprocessable_entity_on_invalid_payload) {
  boost::asio::io_context _client_ioc;
  resolver _resolver(_client_ioc);
  const std::string _host = "127.0.0.1";
  const unsigned short int _port = server_->get_state()->get_port();
  auto const _tcp_resolver_results = _resolver.resolve(_host, std::to_string(_port));
  tcp_stream _stream(_client_ioc);
  _stream.connect(_tcp_resolver_results);

  request_type _request{http_verb::post, "/api/auth/attempt", 11};
  _request.set(http_field::host, _host);
  _request.set(http_field::user_agent, "Client");
  _request.body() = "{}";
  _request.prepare_payload();

  write(_stream, _request);
  flat_buffer _buffer;

  response_type _response;
  read(_stream, _buffer, _response);

  ASSERT_GT(_response.body().size(), 0);
  ASSERT_EQ(_response.result_int(), 422);

  boost::system::error_code _parse_ec;
  auto _result = boost::json::parse(_response.body(), _parse_ec);

  ASSERT_EQ(_parse_ec, boost::beast::errc::success);
  ASSERT_TRUE(_result.is_object());
  ASSERT_TRUE(_result.as_object().contains("message"));
  ASSERT_TRUE(_result.as_object().at("message").is_string());
  ASSERT_EQ(_result.as_object().at("message").as_string(), "The given data was invalid.");
  ASSERT_TRUE(_result.as_object().contains("errors"));
  ASSERT_TRUE(_result.as_object().at("errors").is_object());
  ASSERT_TRUE(_result.as_object().at("errors").as_object().contains("email"));
  ASSERT_TRUE(_result.as_object().at("errors").as_object().at("email").is_array());
  ASSERT_EQ(_result.as_object().at("errors").as_object().at("email").as_array().size(), 1);
  ASSERT_TRUE(_result.as_object().at("errors").as_object().at("email").as_array().at(0).is_string());
  ASSERT_EQ(_result.as_object().at("errors").as_object().at("email").as_array().at(0).as_string(), "Attribute email is required.");
  ASSERT_TRUE(_result.as_object().at("errors").as_object().contains("password"));
  ASSERT_TRUE(_result.as_object().at("errors").as_object().at("password").is_array());
  ASSERT_EQ(_result.as_object().at("errors").as_object().at("password").as_array().size(), 1);
  ASSERT_TRUE(_result.as_object().at("errors").as_object().at("password").as_array().at(0).is_string());
  ASSERT_EQ(_result.as_object().at("errors").as_object().at("password").as_array().at(0).as_string(), "Attribute password is required.");

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