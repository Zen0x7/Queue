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

#ifndef ENGINE_SUPPORT_HPP
#define ENGINE_SUPPORT_HPP

#include <array>
#include <atomic>
#include <bcrypt/BCrypt.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/message_generator.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/json/object.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <chrono>
#include <engine/string_hasher.hpp>
#include <functional>
#include <map>
#include <memory>
#include <regex>
#include <string>
#include <thread>
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

class queue;
using shared_queue = std::shared_ptr<queue>;

class jwt;
using shared_jwt = std::shared_ptr<jwt>;

struct controller_config;

class controller;
using shared_controller = std::shared_ptr<controller>;

using http_verb = boost::beast::http::verb;
using http_status = boost::beast::http::status;
using http_field = boost::beast::http::field;
using message = boost::beast::http::message_generator;
using response_type = boost::beast::http::response<boost::beast::http::string_body>;
using response_empty_type = boost::beast::http::response<boost::beast::http::empty_body>;

using request_type = boost::beast::http::request<boost::beast::http::string_body>;
using route_params_type = std::unordered_map<std::string, std::string, string_hasher, std::equal_to<>>;

using controller_callback_type =
    std::function<boost::asio::awaitable<response_type>(const shared_state&, const request_type&, route_params_type)>;

template <typename T>
using shared_of = std::shared_ptr<T>;

template <typename T>
using atomic_of = std::atomic<T>;

template <typename T>
using vector_of = std::vector<T>;

template <typename T>
using optional_of = std::optional<T>;

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

using endpoint = boost::asio::ip::tcp::endpoint;
using acceptor = boost::asio::ip::tcp::acceptor;
using resolver = boost::asio::ip::tcp::resolver;
using tcp_stream = boost::beast::tcp_stream;
using flat_buffer = boost::beast::flat_buffer;
using system_error = boost::system::system_error;
using socket = boost::asio::ip::tcp::socket;
}  // namespace engine

#endif  // ENGINE_SUPPORT_HPP
