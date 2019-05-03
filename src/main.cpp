#include "queue.h"
#include "thread_mgr.h"
#include "thread_fn.h"
#include "exception_list.h"

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
 This demo is my bulkmt.

    bulkmt  [-version | -? | -N <size> -T <count>]
       Options:  
       -version      -get version of program
       -?            -about program (this info)
       -N <size>     - [required] count of command in the block. Should be > 1
       -T <count>    - [optional] count of file thread. Should be > 0. Default 2
                                
)" << endl;
}

void version_bulk()
{
      cout << "Version bulkmt: " << version() << endl;
}

#ifndef _TEST


int main(int argc, char* argv[])
{
      try
      {
            size_t size_bulk = 0;
            size_t count_thread = 2;

            ParserCommandLine PCL;
            PCL.AddFormatOfArg("?", no_argument, '?');
            PCL.AddFormatOfArg("help", no_argument, '?');
            PCL.AddFormatOfArg("version", no_argument, 'v');
            PCL.AddFormatOfArg("N", required_argument, 'n');
            PCL.AddFormatOfArg("T", required_argument, 't');

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

            if (PCL.Option['t'])
            {
                  const size_t size_param = PCL.Option['t'].ParamOption[0].size();
                  const char* const ptr_str = PCL.Option['t'].ParamOption[0].data();
                  from_chars(ptr_str, ptr_str + size_param, count_thread);
            }

            using queue_tread_t = queue<std::vector<std::string>, std::time_t>;

            command_reader cmdr(size_bulk);

            queue_tread_t console_queue;
            thread_mgr console_tmgr(1, console_queue, output_to_console);
            cmdr.add_subscriber([&](const auto& vec, auto t)
            {
                  try
                  {
                        console_queue.push(vec, t);
                  }
                  catch (...)
                  {
                        std::throw_with_nested(std::runtime_error("'console' subscriber failed."));
                  }
            });
                        

            queue_tread_t file_queue;
            thread_mgr file_tmgr(count_thread, file_queue, save_log_file(), &save_log_file::save);
            cmdr.add_subscriber([&](const auto& vec, auto t)
            {
                  try
                  {
                        file_queue.push(vec, t);
                  }
                  catch (...)
                  {
                        std::throw_with_nested(std::runtime_error("'file' subscriber failed."));
                  }
            });

            cmdr.read();

            console_tmgr.finalize_threads();
            file_tmgr.finalize_threads();

            // выводим счетчики на консоль

            cout << "\nmain thread - " << cmdr.get_counters() << std::endl;
            cout << "log thread - " << console_tmgr.get_list_counters().back() << std::endl;

            for (auto file_counters : file_tmgr.get_list_counters())
            {
                  cout << "file" << file_counters.number_thread << " thread - " << file_counters << std::endl;
            }

            cout << std::endl;

            // повторно бросаем исключения потоков

            exception_ptr_list all_threads_exceptions = console_tmgr.get_threads_exceptions() +
                                                        file_tmgr.get_threads_exceptions();
            all_threads_exceptions.rethrow_if_exist();
      }
      catch (const exception_ptr_list& ex_list)
      {
            exception_ptr_list::print(ex_list);
            return EXIT_FAILURE;
      }
      catch (const exception& ex)
      {
            print_nested_exception(ex);
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
