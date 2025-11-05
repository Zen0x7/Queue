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

#ifndef ENGINE_KERNEL_HPP
#define ENGINE_KERNEL_HPP

#include <boost/asio/awaitable.hpp>
#include <boost/beast/http/message_generator.hpp>
#include <boost/beast/http/string_body.hpp>

namespace engine {
class state;

boost::asio::awaitable<boost::beast::http::message_generator> kernel(
    const std::shared_ptr<state> &state,
    const boost::beast::http::request<boost::beast::http::string_body>
        &request);
}  // namespace engine

#endif  // ENGINE_KERNEL_HPP
