#pragma once
#include "queue.h"
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <iostream>
#include <ctime>

namespace roro_lib
{
      class thread_mgr
      {
        public:
            struct counters
            {
                  std::size_t number_thread = 0;
                  std::size_t count_block = 0;
                  std::size_t count_all_cmds = 0;
            };

            template <typename F,
                      typename std::enable_if_t<std::is_invocable_v<F, std::mutex&, std::condition_variable &,
                                                                    queue<std::vector<std::string>, std::time_t> &, bool &,
                                                                    thread_mgr::counters &>>* Facke = nullptr,
                      typename... Args
            >
            thread_mgr(std::size_t count_threads,
                       queue<Args...>& q,
                       F fn)
            {
                  thread_mgr_internal(count_threads, q, fn);
            }

            template <typename T, typename MF,
                      typename std::enable_if_t<std::is_member_function_pointer_v<MF>>* Facke = nullptr,
                      typename... Args
            >
            thread_mgr(std::size_t count_threads,
                       queue<Args...>& q,
                       T&& obj, MF mfn)
            {
                  using namespace std::placeholders;
                  thread_mgr_internal(count_threads, q, bind(mfn, std::forward<T>(obj), _1, _2, _3, _4, _5));
            }


            void notify_one()
            {
                  cv.notify_one();
            }

            void finalize_all()
            {
                  exit = true;
                  cv.notify_all();

                  for (auto& thr : list_threads)
                  {
                        if (thr.joinable())
                              thr.join();
                  }
            }

            ~thread_mgr()
            {
                  finalize_all();
            };

            auto get_list_counters()
            {
                  return list_counters;
            }

        private:
            std::mutex cv_m;
            std::condition_variable cv;
            static inline bool exit = false;
            std::list<std::thread> list_threads;
            std::list<struct counters> list_counters;

            template <typename T, typename... Args>
            void thread_mgr_internal(std::size_t count_threads,
                queue<Args...>& q,
                T fn)
            {
                  for (size_t i = 0; i < count_threads; i++)
                  {
                        counters counter_thread;
                        counter_thread.number_thread = i;
                        list_counters.push_back(counter_thread);

                        list_threads.push_back(std::thread(fn, std::ref(cv_m), std::ref(cv), std::ref(q), std::ref(exit), std::ref(list_counters.back())));
                  }
            }
      };

      std::ostream& operator<<(std::ostream& out, const thread_mgr::counters& counters)
      {
            out << counters.count_block << " bloks; " << counters.count_all_cmds << " cmds";
            return out;
      }
}
