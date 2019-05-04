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
#include <exception>

namespace roro_lib
{
      void output_to_console(queue<std::vector<std::string>, std::time_t>& que,
          counters_thread_mgr& counters_thread,
          std::exception_ptr& ex_ptr)
      {
            try
            {
                  try
                  {
                        using queue_thread_t = std::remove_reference_t<decltype(que)>;

                        while (true)
                        {
                              auto item = que.pop_wait();
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
                  catch (std::bad_alloc&)
                  {
                        std::throw_with_nested(std::runtime_error("output_to_console() failed."));
                  }
                  catch (...)
                  {
                        std::stringstream ss;
                        ss << std::this_thread::get_id();
                        std::throw_with_nested(std::runtime_error("output_to_console() failed. Thread ID:" + ss.str()));
                  }
            }
            catch (...)
            {
                  ex_ptr = std::current_exception();
            }
      }


      struct save_log_file
      {
            std::string get_filename(std::time_t time_first_cmd, std::size_t number_thread)
            try
            {
                  std::stringstream sstr;
                  sstr << "./bulk" << time_first_cmd << "_tid" << number_thread << ".log";
                  return sstr.str();
            }
            catch (...)
            {
                  std::throw_with_nested(std::runtime_error("save_log_file::get_filename() failed."));
            }


            void save(queue<std::vector<std::string>, std::time_t>& que,
                counters_thread_mgr& counters_thread,
                std::exception_ptr& ex_ptr)
            {
                  try
                  {
                        try
                        {
                              using queue_thread_t = std::remove_reference_t<decltype(que)>;

                              while (true)
                              {
                                    auto item = que.pop_wait();
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
                        catch (std::bad_alloc&)
                        {
                              std::throw_with_nested(std::runtime_error("save_log_file::save() failed."));
                        }
                        catch (...)
                        {
                              std::stringstream ss;
                              ss << std::this_thread::get_id();
                              std::throw_with_nested(std::runtime_error("save_log_file::save() failed. Thread ID:" + ss.str()));
                        }
                  }
                  catch (...)
                  {
                        ex_ptr = std::current_exception();
                  }
            }
      };
}
