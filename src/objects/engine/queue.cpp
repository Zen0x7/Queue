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

#include <engine/queue.hpp>
#include <engine/worker.hpp>

namespace engine {
queue::queue(boost::asio::strand<boost::asio::io_context::executor_type> strand)
    : strand_(std::move(strand)) {
  auto _worker =
      std::make_shared<worker>(make_strand(strand_.get_inner_executor()));
  workers_.try_emplace(_worker->id(), _worker);
}

void queue::scale_to(const std::size_t number_of_workers) {
  while (workers_.size() != number_of_workers) {
    auto _worker =
        std::make_shared<worker>(make_strand(strand_.get_inner_executor()));
    workers_.try_emplace(_worker->id(), _worker);
  }
}
}  // namespace engine
