#pragma once
#include "publisher.h"
#include "counters.h"
#include <vector>
#include <string>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <exception>

namespace roro_lib
{
      class command_reader : public publisher_mixin<void(const std::vector<std::string>&, std::time_t)>
      {
        public:
            command_reader(size_t size_bulk) : size_bulk(size_bulk){};

            void read()
            {
                  size_t count_cmd_bulk = 0;
                  std::time_t time_first_cmd = 0;
                  std::vector<std::string> command_list;

                  for (std::string line; getline(std::cin, line);) // For exit by EOF. (Console Linux Ctrl+D. Console Windows Ctrl+Z)
                  {
                        reader_counters.count_string++;

                        if (count_cmd_bulk == 0)
                              time_first_cmd = get_time();

                        if (line == "{")
                        {
                              notify_subscribers(command_list, time_first_cmd);
                              brackets_read(reader_counters.count_all_cmds);
                              count_cmd_bulk = 0;
                              continue;
                        }

                        if (line == "}")
                        {
                              throw std::runtime_error("found not a pair bracket");
                        }

                        command_list.push_back(line);
                        reader_counters.count_all_cmds++;

                        if (count_cmd_bulk == size_bulk - 1)
                        {
                              notify_subscribers(command_list, time_first_cmd);
                              count_cmd_bulk = 0;
                              continue;
                        }
                        count_cmd_bulk++;
                  }

                  notify_subscribers(command_list, time_first_cmd);
            };

            counters_command_reader get_counters()
            {
                  return reader_counters;
            }

        private:
            counters_command_reader reader_counters;

            size_t size_bulk;

            std::time_t get_time()
            {
                  return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            }

            void notify_subscribers(std::vector<std::string>& command_list, std::time_t time_first_cmd)
            {
                  if (!command_list.empty())
                  {
                        reader_counters.count_block++;
                        notify(command_list, time_first_cmd);
                        command_list.clear();
                  }
            }

            void brackets_read(std::size_t& count_all_cmds)
            {
                  size_t count_bracket = 1;
                  std::time_t time_first_cmd = 0;
                  std::vector<std::string> command_list;

                  for (std::string line; count_bracket != 0;)
                  {
                        reader_counters.count_string++;

                        if (!getline(std::cin, line))
                              exit(EXIT_SUCCESS); // Program exit  by EOF. (Console Linux Ctrl+D. Console Windows Ctrl+Z)

                        if (time_first_cmd == 0)
                              time_first_cmd = get_time();


                        if (line == "{")
                        {
                              ++count_bracket;
                              continue;
                        }

                        if (line == "}")
                        {
                              --count_bracket;
                              continue;
                        }

                        command_list.push_back(line);
                        count_all_cmds++;
                  }

                  notify_subscribers(command_list, time_first_cmd);
            }
      };

      std::ostream& operator<<(std::ostream& out, const counters_command_reader& counters)
      {
            out << counters.count_string << " strings; " << counters.count_block << " bloks; " << counters.count_all_cmds << " cmds";
            return out;
      }
}
