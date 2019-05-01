#include "queue.h"
#include "thread_mgr.h"
#include "thread_fn.h"

#include "bulk_reader.h"
#include <iostream>
#include <exception>
#include <charconv>

#include "lib_version.h"
#include "CLParser.h"


using namespace std;
using namespace roro_lib;

void help()
{
      cout << R"(
 This demo is my bulk.

    bulk  [-version | -? | -N <size>]
       Options:  
       -version      -get version of program
       -?            -about program (this info)
       -N <size>     -count of command in the block. Should be > 1
                                
)" << endl;
}

void version_bulk()
{
      cout << "Version bulk: " << version() << endl;
}

#ifndef _TEST


int main(int argc, char* argv[])
{
      try
      {
            size_t size_bulk = 0;
            ParserCommandLine PCL;
            PCL.AddFormatOfArg("?", no_argument, '?');
            PCL.AddFormatOfArg("help", no_argument, '?');
            PCL.AddFormatOfArg("version", no_argument, 'v');
            PCL.AddFormatOfArg("N", required_argument, 'n');

            PCL.SetShowError(false);
            PCL.Parser(argc, argv);

            if (PCL.Option['?'])
            {
                  help();
                  return 0;
            }
            if (PCL.Option['v'])
            {
                  version_bulk();
                  return 0;
            }

            if (!PCL.Option['n'])
            {
                  help();
                  return 0;
            }
            else
            {
                  const size_t size_param = PCL.Option['n'].ParamOption[0].size();
                  const char* const ptr_str = PCL.Option['n'].ParamOption[0].data();
                  from_chars(ptr_str, ptr_str + size_param, size_bulk);

                  if (size_bulk == 0)
                  {
                        help();
                        return 0;
                  }
            }

            using queue_tread_t = queue<std::vector<std::string>, std::time_t>;

            command_reader cmdr(size_bulk);

            queue_tread_t file_queue;
            thread_mgr file_tmgr(2, file_queue, save_log_file(), &save_log_file::save);
            cmdr.add_subscriber([&](auto vec, auto t) { file_queue.push(vec, t); });

            queue_tread_t console_queue;
            thread_mgr console_tmgr(1, console_queue, output_to_console);
            cmdr.add_subscriber([&](auto vec, auto t){ console_queue.push(vec, t); } );
            

            cmdr.read();

            console_tmgr.finalize_threads();
            file_tmgr.finalize_threads();


            cout << "\nmain thread - " << cmdr.get_counters() << std::endl;

            auto console_counters = console_tmgr.get_list_counters();
            cout << "log thread - " << console_counters.back() << std::endl;

            auto files_counters = file_tmgr.get_list_counters();
            for (auto file_counters : files_counters)
            {
                  cout << "file" << file_counters.number_thread << " thread - " << file_counters << std::endl;
            }

      }
      catch (const exception& ex)
      {
            cerr << "Error: " << ex.what() << endl;
            return EXIT_FAILURE;
      }
      catch (...)
      {
            cerr << "Error: unknown exception" << endl;
            return EXIT_FAILURE;
      }

      return 0;
}

#endif
