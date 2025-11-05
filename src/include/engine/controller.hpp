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

#ifndef ENGINE_CONTROLLER_HPP
#define ENGINE_CONTROLLER_HPP

#include <boost/asio/awaitable.hpp>
#include <boost/beast/http/message_generator.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/string_body.hpp>
#include <memory>

namespace engine {
class state;
using controller_callback_type = std::function<boost::asio::awaitable<
    boost::beast::http::response<boost::beast::http::string_body>>(
    const std::shared_ptr<state>&,
    const boost::beast::http::request<boost::beast::http::string_body>&)>;

class controller : public std::enable_shared_from_this<controller> {
  controller_callback_type callback_;

 public:
  explicit controller(controller_callback_type callback);
  controller_callback_type& callback() noexcept;
};
}  // namespace engine

#endif  // ENGINE_CONTROLLER_HPP
