#pragma once
#include <list>
#include <tuple>
#include <mutex>
#include <optional>

namespace roro_lib
{
      template <typename... Args>
      class queue
      {
            std::mutex queue_mutex;
            std::list<std::tuple<Args...>> raw_queue;

        public:
            bool empty()
            {
                  std::lock_guard<std::mutex> queue_guard(queue_mutex);
                  return raw_queue.empty();
            }

            template <typename... Args_push>
            void push(Args_push... args)
            {
                  std::lock_guard<std::mutex> queue_guard(queue_mutex);
                  raw_queue.emplace_front(args...);
            }

            std::optional<std::tuple<Args...>> pop()
            {
                  std::unique_lock<std::mutex> queue_uguard(queue_mutex);

                  if (!raw_queue.empty())
                  {
                        std::tuple<Args...> item(raw_queue.back());
                        raw_queue.pop_back();
                        queue_uguard.unlock();
                        return item;
                  }
                  queue_uguard.unlock();
                  return {};
            }
      };

}
