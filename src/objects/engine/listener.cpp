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

#include <boost/asio/co_spawn.hpp>
#include <engine/errors/session_error.hpp>
#include <engine/listener.hpp>
#include <engine/session.hpp>
#include <engine/state.hpp>
#include <iostream>

namespace engine {
boost::asio::awaitable<void> listener(std::shared_ptr<state> state,
                                      boost::asio::ip::tcp::endpoint endpoint) {
  const auto _executor = co_await boost::asio::this_coro::executor;
  auto _acceptor = boost::asio::ip::tcp::acceptor{_executor, endpoint};

  state->port_.store(_acceptor.local_endpoint().port(),
                     std::memory_order_release);
  state->running_.store(true, std::memory_order_release);

  for (;;) {
    co_spawn(
        _executor,
        session(state,
                boost::beast::tcp_stream{co_await _acceptor.async_accept()}),
        [](const std::exception_ptr &throwable) {
          if (throwable) {
            try {
              std::rethrow_exception(throwable);
            } catch (const boost::system::system_error &exception) {
              std::cerr << "[Listener] Boost error: " << exception.what()
                        << std::endl;
            } catch (...) {
              std::cerr << "[Listener] Unknown exception thrown." << std::endl;
            }
          }
        });
  }
}
}  // namespace engine
