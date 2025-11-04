#pragma once

#ifndef ENGINE_TASK_HPP
#define ENGINE_TASK_HPP

#include <atomic>
#include <functional>
#include <memory>

#include <boost/asio/awaitable.hpp>
#include <boost/json/object.hpp>

namespace engine {
using handler_signature_type = boost::asio::awaitable<void>(std::atomic<bool>&, boost::json::object const&);
using handler_type = std::function<handler_signature_type>;

class task : public std::enable_shared_from_this<task> {
  std::shared_ptr<handler_type> callback_;

 public:
  explicit task(handler_type callback);

  std::shared_ptr<handler_type> callback() const noexcept;
};
}  // namespace engine

#endif  // ENGINE_TASK_HPP
