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

#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/write.hpp>
#include <engine/server.hpp>
#include <engine/state.hpp>
#include <thread>

class server : public testing::Test {
 public:
  std::shared_ptr<engine::server> server_;
  std::shared_ptr<std::jthread> thread_;

 protected:
  void SetUp() override {
    server_ = std::make_shared<engine::server>();

    thread_ = std::make_shared<std::jthread>([this]() { server_->start(); });

    thread_->detach();

    while (server_->get_state()->running_.load() == false) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  void TearDown() override { server_->get_state()->ioc().stop(); }
};

TEST_F(server, can_handle_http_request) {
  boost::asio::io_context _client_ioc;
  boost::asio::ip::tcp::resolver _resolver(_client_ioc);
  const std::string _host = "127.0.0.1";
  const unsigned short int _port = server_->get_state()->port_.load();
  auto const _tcp_resolver_results =
      _resolver.resolve(_host, std::to_string(_port));
  boost::beast::tcp_stream _stream(_client_ioc);
  _stream.connect(_tcp_resolver_results);

  boost::beast::http::request<boost::beast::http::string_body> _request{
      boost::beast::http::verb::get, "/status", 11};
  _request.set(boost::beast::http::field::host, _host);
  _request.set(boost::beast::http::field::user_agent, "Client");
  _request.prepare_payload();

  boost::beast::http::write(_stream, _request);
  boost::beast::flat_buffer _buffer;

  boost::beast::http::response<boost::beast::http::string_body> _response;
  boost::beast::http::read(_stream, _buffer, _response);

  ASSERT_EQ(_response.body().size(), 0);
  ASSERT_EQ(_response.result_int(), 200);

  boost::beast::error_code _ec;
  _stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, _ec);
  ASSERT_EQ(_ec, boost::beast::errc::success);
}

TEST_F(server, can_timeout_http_sessions) {
  boost::asio::io_context _client_ioc;
  boost::asio::ip::tcp::resolver _resolver(_client_ioc);
  const std::string _host = "127.0.0.1";
  const unsigned short int _port = server_->get_state()->port_.load();
  auto const _tcp_resolver_results =
      _resolver.resolve(_host, std::to_string(_port));
  boost::beast::tcp_stream _stream(_client_ioc);
  _stream.connect(_tcp_resolver_results);

  boost::beast::http::request<boost::beast::http::string_body> _request{
      boost::beast::http::verb::get, "/status", 11};
  _request.set(boost::beast::http::field::host, _host);
  _request.set(boost::beast::http::field::user_agent, "Client");
  _request.prepare_payload();

  boost::beast::http::write(_stream, _request);
  boost::beast::flat_buffer _buffer;

  boost::beast::http::response<boost::beast::http::string_body> _response;
  boost::beast::http::read(_stream, _buffer, _response);

  ASSERT_EQ(_response.body().size(), 0);
  ASSERT_EQ(_response.result_int(), 200);

  std::this_thread::sleep_for(std::chrono::seconds(6));

  boost::beast::error_code _write_ec;
  boost::beast::http::write(_stream, _request, _write_ec);
  ASSERT_EQ(_write_ec, boost::beast::errc::success);

  boost::beast::error_code _disconnect_ec;
  _stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both,
                            _disconnect_ec);

  ASSERT_EQ(_disconnect_ec, boost::beast::errc::not_connected);
}
