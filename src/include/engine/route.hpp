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

#ifndef ENGINE_ROUTE_HPP
#define ENGINE_ROUTE_HPP

#include <engine/controller.hpp>
#include <engine/string_hasher.hpp>

#include <memory>
#include <unordered_map>
#include <vector>
#include <regex>

#include <boost/beast/http/verb.hpp>

namespace engine {
class route : public std::enable_shared_from_this<route> {
  std::string signature_;
  std::shared_ptr<std::regex> expression_;
  std::shared_ptr<controller> controller_;
  std::vector<boost::beast::http::verb> verbs_;
  std::vector<std::string> parameters_;

public:
  explicit route(std::vector<boost::beast::http::verb> verbs, std::string signature, const std::shared_ptr<controller> &controller);
  void compile();
  std::shared_ptr<std::regex> &get_expression();
  std::shared_ptr<controller> &get_controller();
  std::vector<boost::beast::http::verb> &get_verbs();
  std::vector<std::string> &get_parameters();
  std::pair<bool, std::unordered_map<std::string, std::string, string_hasher, std::equal_to<>>> match(const std::string &input);
};
}  // namespace engine

#endif  // ENGINE_ROUTE_HPP
