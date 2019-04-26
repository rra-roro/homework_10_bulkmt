#pragma once
#include <vector>
#include <string>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <exception>
#include "publisher.h"

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
                        if (count_cmd_bulk == 0)
                              time_first_cmd = get_time();

                        if (line == "{")
                        {
                              notify_subscribers(command_list, time_first_cmd);
                              brackets_read();
                              count_cmd_bulk = 0;
                              continue;
                        }

                        if (line == "}")
                        {
                              throw std::runtime_error("found not a pair bracket");
                        }

                        command_list.push_back(line);

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

        private:

            size_t size_bulk;

            std::time_t get_time()
            {
                  return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            }

            void notify_subscribers(std::vector<std::string>& command_list, std::time_t time_first_cmd)
            {
                  if (!command_list.empty())
                  {
                        notify(command_list, time_first_cmd);
                        command_list.clear();
                  }
            }

            void brackets_read()
            {
                  size_t count_bracket = 1;
                  std::time_t time_first_cmd = 0;
                  std::vector<std::string> command_list;

                  for (std::string line; count_bracket != 0;)
                  {
                        if (!getline(std::cin, line))  
                              exit(EXIT_SUCCESS);        // Program exit  by EOF. (Console Linux Ctrl+D. Console Windows Ctrl+Z)

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
                  }

                  notify_subscribers(command_list, time_first_cmd);
            }
      };
}
