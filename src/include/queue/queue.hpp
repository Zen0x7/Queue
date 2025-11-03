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

#ifndef QUEUE_QUEUE_HPP
#define QUEUE_QUEUE_HPP

#include <map>
#include <memory>

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

#include <queue/job.hpp>

namespace queue {
class queue : public std::enable_shared_from_this<queue> {
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;

 public:
  explicit queue(
      boost::asio::strand<boost::asio::io_context::executor_type> strand);

  template <typename Handler>
  shared_job push(Handler&& handler) {
    auto _job = std::make_shared<job>(std::forward<Handler>(handler));
    boost::asio::post(strand_, [_job] { _job->run(); });
    return _job;
  }
};

using shared_queue = std::shared_ptr<queue>;

using queue_container = std::map<std::string, shared_queue, std::less<>>;
}  // namespace queue

#endif  // QUEUE_QUEUE_HPP
