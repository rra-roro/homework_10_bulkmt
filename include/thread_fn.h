#pragma once
#include "queue.h"
#include "counters.h"
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <ctime>
#include <tuple>

namespace roro_lib
{
      void output_to_console(queue<std::vector<std::string>, std::time_t>& que,
          counters_thread_mgr& counters_thread)
      {
            try
            {
                  using queue_thread_t = std::remove_reference_t<decltype(que)>;

                  while (true)
                  {
                        auto item = que.pop();
                        if (std::get<0>(item) == queue_thread_t::exit)
                              return;

                        counters_thread.count_block++;


                        std::string bulk = "bulk:";
                        for (auto cmd : std::get<1>(item))
                        {
                              bulk += " " + cmd + ",";
                              counters_thread.count_all_cmds++;
                        };

                        bulk.back() = '\n';

                        std::cout << bulk;
                  }
            }
            catch (...)
            {

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

            void save(queue<std::vector<std::string>, std::time_t>& que,
                      counters_thread_mgr& counters_thread)
            {
                  try
                  {
                        using queue_thread_t = std::remove_reference_t<decltype(que)>;

                        while (true)
                        {
                              auto item = que.pop();
                              if (std::get<0>(item) == queue_thread_t::exit)
                                    return;

                              counters_thread.count_block++;

                              std::fstream fout(get_filename(std::get<2>(item), counters_thread.number_thread), std::fstream::out);
                              for (auto cmd : std::get<1>(item))
                              {
                                    fout << cmd << "\n";
                                    counters_thread.count_all_cmds++;
                              }
                              fout.close();
                        }
                  }
                  catch (...)
                  {

                  }
            }
      };
}
