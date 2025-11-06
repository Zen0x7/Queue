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

#ifndef ENGINE_TYPES_HPP
#define ENGINE_TYPES_HPP

#include <boost/asio/awaitable.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/json/object.hpp>
#include <boost/uuid/uuid.hpp>
#include <engine/string_hasher.hpp>
#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace engine {
class state;
using shared_state = std::shared_ptr<state>;

class route;
using shared_route = std::shared_ptr<route>;

class router;
using shared_router = std::shared_ptr<router>;

class task;
using shared_task = std::shared_ptr<task>;

class job;
using shared_job = std::shared_ptr<job>;

class worker;
using shared_worker = std::shared_ptr<worker>;

class controller;
using shared_controller = std::shared_ptr<controller>;

using http_verb = boost::beast::http::verb;
using response_type =
    boost::beast::http::response<boost::beast::http::string_body>;
using request_type =
    boost::beast::http::request<boost::beast::http::string_body>;
using route_params_type = std::unordered_map<std::string, std::string,
                                             string_hasher, std::equal_to<>>;

using controller_callback_type =
    std::function<boost::asio::awaitable<response_type>(
        const shared_state&, const request_type&, route_params_type)>;

template <typename T>
using shared_of = std::shared_ptr<T>;

template <typename T>
using atomic_of = std::atomic<T>;

template <typename T>
using vector_of = std::vector<T>;

template <typename T, typename S>
using tuple_of = std::tuple<T, S>;

template <typename T, typename S>
using pair_of = std::pair<T, S>;

template <typename T, typename S>
using map_of = std::map<T, S>;

template <typename T, typename S, typename R>
using map_hash_of = std::map<T, S, R>;

template <typename T, typename S>
using map_of = std::map<T, S>;

template <typename T>
using async_of = boost::asio::awaitable<T>;

template <typename T>
using strand_of = boost::asio::strand<T>;

using time_point = std::chrono::system_clock::time_point;

using uuid = boost::uuids::uuid;

using object = boost::json::object;

using handler_signature_type = async_of<void>(atomic_of<bool>&, object const&);
using handler_type = std::function<handler_signature_type>;
}  // namespace engine

#endif  // ENGINE_TYPES_HPP
