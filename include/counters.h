#pragma once
#include <cstddef>

namespace roro_lib
{
      struct counters_thread_mgr
      {
            std::size_t number_thread = 0;
            std::size_t count_block = 0;
            std::size_t count_all_cmds = 0;
      };

      struct counters_command_reader
      {
            std::size_t count_string = 0;
            std::size_t count_block = 0;
            std::size_t count_all_cmds = 0;
      };
}
