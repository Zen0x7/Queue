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

#pragma once

#ifndef ENGINE_LISTENER_HPP
#define ENGINE_LISTENER_HPP

#include <boost/asio/awaitable.hpp>
#include <boost/beast/core/tcp_stream.hpp>

namespace engine {
class state;

boost::asio::awaitable<void> listener(const std::shared_ptr<state> &state,
                                      boost::asio::ip::tcp::endpoint endpoint);
}  // namespace engine

#endif  // ENGINE_LISTENER_HPP
