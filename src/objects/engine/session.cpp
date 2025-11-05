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

#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/message_generator.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/write.hpp>
#include <engine/kernel.hpp>
#include <engine/session.hpp>

namespace engine {
    boost::asio::awaitable<void> session(std::shared_ptr<state> state, boost::beast::tcp_stream stream) {
        boost::beast::flat_buffer _buffer;

        for (;;) {
            stream.expires_after(std::chrono::seconds(5));

            boost::beast::http::request<boost::beast::http::string_body> _request;
            co_await boost::beast::http::async_read(stream, _buffer, _request);

            boost::beast::http::message_generator _message =
                co_await kernel(state, std::move(_request));

            const bool keep_alive = _message.keep_alive();
            co_await boost::beast::async_write(stream, std::move(_message));

            if (!keep_alive) {
                break;
            }
        }

        stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send);
    }
}  // namespace engine
