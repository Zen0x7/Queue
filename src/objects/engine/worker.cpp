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

#include <engine/worker.hpp>

namespace engine {
worker::worker(boost::asio::strand<boost::asio::io_context::executor_type> strand) : strand_(std::move(strand)) {}

const boost::uuids::uuid& worker::id() const noexcept {
  return id_;
}

std::uint64_t worker::number_of_tasks() const noexcept {
  return number_of_tasks_.load(std::memory_order_acquire);
}
}  // namespace engine
