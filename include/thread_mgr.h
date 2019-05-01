#pragma once
#include "queue.h"
#include "counters.h"
#include <functional>
#include <thread>
#include <list>
#include <iostream>
#include <ctime>


namespace roro_lib
{
      template<typename... Args>
      class thread_mgr
      {
            queue<Args...>& queue_thread;

        public:
            template <typename F,
                      typename std::enable_if_t<std::is_invocable_v<F, queue<Args...>&, counters_thread_mgr&>>* Facke = nullptr                      
            >
            thread_mgr(std::size_t count_threads,
                       queue<Args...>& q,
                       F fn) : queue_thread(q)
            {
                  thread_mgr_internal(count_threads, q, fn);
            }

            template <typename T, typename MF,
                      typename std::enable_if_t<std::is_member_function_pointer_v<MF>>* Facke = nullptr
            >
            thread_mgr(std::size_t count_threads,
                       queue<Args...>& q,
                       T&& obj, MF mfn) : queue_thread(q)
            {
                  using namespace std::placeholders;
                  thread_mgr_internal(count_threads, q, bind(mfn, std::forward<T>(obj), _1, _2));
            }

            void finalize_threads()
            {
                  queue_thread.push_exit();

                  for (auto& thr : list_threads)
                  {
                        if (thr.joinable())
                              thr.join();
                  }
            }

            ~thread_mgr()
            {
                  finalize_threads();
            };

            auto get_list_counters()
            {
                  return list_counters;
            }

        private:
            std::list<std::thread> list_threads;
            std::list<counters_thread_mgr> list_counters;

            template <typename T>
            void thread_mgr_internal(std::size_t count_threads,
                                     queue<Args...>& q,
                                     T fn)
            {
                  for (size_t i = 0; i < count_threads; i++)
                  {
                        counters_thread_mgr counter_thread;
                        counter_thread.number_thread = i;
                        list_counters.push_back(counter_thread);

                        list_threads.push_back(std::thread(fn, std::ref(q), std::ref(list_counters.back())));
                  }
            }
      };

      std::ostream& operator<<(std::ostream& out, const counters_thread_mgr& counters)
      {
            out << counters.count_block << " bloks; " << counters.count_all_cmds << " cmds";
            return out;
      }
}
