#pragma once
#include "queue.h"
#include "thread_mgr.h"
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <ctime>

#include <tuple>
#include <mutex>
#include <condition_variable>

namespace roro_lib
{
      void output_to_console(std::mutex& cv_m,
                             std::condition_variable& cv,
                             queue<std::vector<std::string>, std::time_t>& que,
                             bool& exit,
                             thread_mgr::counters& counters_thread)
      {
            while (true)
            {
                  std::unique_lock<std::mutex> lock(cv_m);
                  cv.wait(lock, [&] { return !que.empty() || exit; });
                  if (exit)
                        return;

                  auto item = que.pop();

                  if (item)
                  {
                        counters_thread.count_block++;

                        std::string bulk = "bulk:";
                        for (auto cmd : std::get<0>(*item))
                        {
                              bulk += " " + cmd + ",";
                              counters_thread.count_all_cmds++;
                        };

                        bulk.back() = '\n';

                        std::cout << bulk;
                  }
            }
      }


      struct save_log_file
      {
            std::string get_filename(std::time_t time_first_cmd, std::size_t number_thread)
            {
                  std::stringstream sstr;
                  sstr << "./bulk" << time_first_cmd << "_tid" << number_thread << ".log";

                  return sstr.str();
            }

            void save(std::mutex& cv_m,
                      std::condition_variable& cv,
                      queue<std::vector<std::string>, std::time_t>& que,
                      bool& exit,
                      thread_mgr::counters& counters_thread)
            {
                  while (true)
                  {
                        std::unique_lock<std::mutex> lock(cv_m);

                        cv.wait(lock, [&] { return !que.empty() || exit; });                       

                        if (exit)
                              return;

                        auto item = que.pop();
                        
                        lock.unlock();

                        if (item)
                        {
                              counters_thread.count_block++;

                              std::fstream fout(get_filename(std::get<1>(*item), counters_thread.number_thread), std::fstream::out);
                              for (auto cmd : std::get<0>(*item))
                              {
                                    fout << cmd << "\n";
                                    counters_thread.count_all_cmds++;
                              }
                              fout.close();
                        }
                  }
            }
      };
}
