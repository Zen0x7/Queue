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

#ifndef ENGINE_QUEUE_HPP
#define ENGINE_QUEUE_HPP

#include <algorithm>
#include <map>
#include <memory>

#include <boost/asio/strand.hpp>
#include <boost/core/ignore_unused.hpp>

#include <boost/uuid/uuid.hpp>

#include <engine/worker.hpp>

namespace engine {
class worker;

class job;

class queue : public std::enable_shared_from_this<queue> {
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
  std::map<boost::uuids::uuid, std::shared_ptr<worker>> workers_;

 public:
  explicit queue(
      boost::asio::strand<boost::asio::io_context::executor_type> strand);

  std::size_t number_of_workers() const;

  template <typename Handler>
  std::shared_ptr<job> push(Handler&& handler) {
    auto _comparator = [](const auto& a, const auto& b) {
      return a.second->number_of_tasks() < b.second->number_of_tasks();
    };
    auto _worker_iterator = std::ranges::min_element(workers_, _comparator);
    auto& [_, _worker] = *_worker_iterator;
    boost::ignore_unused(_);
    auto _job = _worker->push(std::forward<Handler>(handler));
    return _job;
  }

  void set_workers_to(std::size_t number_of_workers);
};
}  // namespace engine

#endif  // ENGINE_QUEUE_HPP
