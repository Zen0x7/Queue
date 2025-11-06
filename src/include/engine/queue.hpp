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

#include <atomic>
#include <boost/asio/strand.hpp>
#include <boost/json/object.hpp>
#include <boost/uuid/uuid.hpp>
#include <engine/support.hpp>
#include <engine/task.hpp>
#include <engine/worker.hpp>
#include <map>
#include <memory>

namespace engine {
class queue : public std::enable_shared_from_this<queue> {
  strand_of<boost::asio::io_context::executor_type> strand_;

  map_of<uuid, shared_worker> workers_;
  std::mutex workers_mutex_;

  map_of<uuid, shared_job> jobs_;
  std::mutex jobs_mutex_;

  map_hash_of<std::string, shared_task, std::less<>> tasks_;
  std::mutex tasks_mutex_;

 public:
  explicit queue(strand_of<boost::asio::io_context::executor_type> strand);
  std::size_t number_of_workers() const;
  std::size_t number_of_jobs() const;
  shared_job dispatch(std::string const& name, object data = {});
  void add_task(std::string name, handler_type handler);
  void set_workers_to(std::size_t no_of_workers);
  void cancel();

 private:
  void prepare();
  void upscale(std::size_t to = 1);
  void downscale(std::size_t to = 1);
  void dispatch(const shared_job& job);
  shared_worker get_worker();
  shared_task get_task(const std::string& name);
};
}  // namespace engine

#endif  // ENGINE_QUEUE_HPP
