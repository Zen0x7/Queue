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

#ifndef ENGINE_ROUTER_HPP
#define ENGINE_ROUTER_HPP

#include <engine/route.hpp>
#include <memory>
#include <regex>
#include <vector>

namespace engine {
class router : public std::enable_shared_from_this<router> {
  std::vector<std::shared_ptr<route>> routes_;

 public:
  std::vector<std::shared_ptr<route>> get_routes();

  std::shared_ptr<router> add(std::shared_ptr<route> route);

  std::tuple<std::unordered_map<std::string, std::string, string_hasher,
                                std::equal_to<>>,
             std::shared_ptr<route>>
  find(boost::beast::http::verb verb, const std::string &path);
};
}  // namespace engine

#endif  // ENGINE_ROUTER_HPP
