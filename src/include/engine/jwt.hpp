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

#ifndef ENGINE_JWT_HPP
#define ENGINE_JWT_HPP

#include <boost/json/object.hpp>
#include <boost/uuid/uuid.hpp>
#include <memory>

namespace engine {
class jwt : public std::enable_shared_from_this<jwt> {
  boost::uuids::uuid id_;
  boost::uuids::uuid sub_;
  std::string header_;
  boost::json::object payload_;
  std::string signature_;

 public:
  jwt(boost::uuids::uuid id, boost::uuids::uuid sub, std::string header,
      boost::json::object payload, std::string signature);

  std::string as_string() const;

  boost::uuids::uuid get_id() const;
  boost::uuids::uuid get_sub() const;
  boost::json::object get_payload() const;
  std::string get_signature() const;

  static std::shared_ptr<jwt> make(boost::uuids::uuid id,
                                   const std::string &key);

  static std::shared_ptr<jwt> from(const std::string_view &bearer,
                                   const std::string &key);
};
}  // namespace engine

#endif  // ENGINE_JWT_HPP
