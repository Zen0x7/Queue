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

#ifndef ENGINE_CONTROLLERS_QUEUES_WORKERS_CONTROLLER_HPP
#define ENGINE_CONTROLLERS_QUEUES_WORKERS_CONTROLLER_HPP

#include <engine/support.hpp>

namespace engine::controllers::queues {
class workers_controller {
 public:
  static vector_of<http_verb> verbs();
  static shared_controller make();
};

}  // namespace engine::controllers::queues

#endif  // ENGINE_CONTROLLERS_QUEUES_WORKERS_CONTROLLER_HPP
