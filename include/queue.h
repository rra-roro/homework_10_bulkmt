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
        public:
            enum cmd_type
            {
                  data,
                  exit
            };

            bool empty()
            {
                  std::lock_guard<std::mutex> queue_guard(queue_mutex);
                  return raw_queue.empty();
            }

            template <typename... Args_push>
            void push(Args_push... args)
            {
                  std::lock_guard<std::mutex> queue_guard(queue_mutex);
                  raw_queue.emplace_front(data, args...);
                  queue_cv.notify_one();
            }

            void push_exit()
            {
                  std::lock_guard<std::mutex> queue_guard(queue_mutex);
                  raw_queue.emplace_front(exit, Args()...);
                  queue_cv.notify_all();
            }

            std::tuple<cmd_type, Args...> pop()
            {
                  std::unique_lock<std::mutex> queue_uguard(queue_mutex);
                  queue_cv.wait(queue_uguard, [&] { return !raw_queue.empty(); });

                  std::tuple<cmd_type, Args...> item(raw_queue.back());

                  if (std::get<0>(item) != exit)
                        raw_queue.pop_back();

                  queue_uguard.unlock();

                  return item;
            }

        private:
            std::mutex queue_mutex;
            std::condition_variable queue_cv;
            std::list<std::tuple<cmd_type, Args...>> raw_queue;

      };

}
