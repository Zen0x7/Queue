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

#include <engine/errors/not_found_error.hpp>
#include <engine/router.hpp>

namespace engine {
std::vector<std::shared_ptr<route>> router::get_routes() { return routes_; }

std::shared_ptr<router> router::add(std::shared_ptr<route> route) {
  routes_.push_back(std::move(route));
  return shared_from_this();
}

std::tuple<std::unordered_map<std::string, std::string, string_hasher,
                              std::equal_to<>>,
           std::shared_ptr<route>>
router::find(const boost::beast::http::verb verb, const std::string &path) {
  for (auto const &_route : get_routes()) {
    if (auto [_matched, _params] = _route->match(path);
        std::ranges::find(_route->get_verbs(), verb) !=
            _route->get_verbs().end() &&
        _matched) {
      return std::make_tuple(_params, _route);
    }
  }
  throw errors::not_found_error();
}
}  // namespace engine
