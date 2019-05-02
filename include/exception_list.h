#pragma once
#include <exception>
#include <list>
#include <iostream>
#include <string>

namespace roro_lib
{
      void print_exception(const std::exception& e, int level = 0)
      {
            std::cerr << std::string(level, ' ') << "exception: " << e.what() << '\n';
            try
            {
                  rethrow_if_nested(e);
            }
            catch (const std::exception& e)
            {
                  print_exception(e, level + 1);
            }
            catch (...)
            {
            }
      }

      class exception_ptr_list
      {
        private:
            std::list<std::exception_ptr> list_exception_ptr;           

        public:
            void add_back_exception_ptr()
            {
                  list_exception_ptr.emplace_back();
            }

            auto& back()
            {
                  return list_exception_ptr.back();
            }

            auto& back() const
            {
                  return list_exception_ptr.back();
            }

            void rethrow_if_exist()
            {
                  list_exception_ptr.remove(false);
                  if (!list_exception_ptr.empty()) throw *this;
            }

            exception_ptr_list& operator+=(exception_ptr_list rhs)
            {
                  this->list_exception_ptr.splice(this->list_exception_ptr.end(), rhs.list_exception_ptr);
                  return *this; 
            }

            friend exception_ptr_list operator+(exception_ptr_list lhs, 
                                                const exception_ptr_list& rhs)
            {
                  lhs += rhs;
                  return lhs;
            }           

            static void print(const exception_ptr_list& ex_list)
            {
                  for (auto& ex_ptr : ex_list.list_exception_ptr)
                  {
                        try
                        {
                              if (ex_ptr)
                              {
                                    std::rethrow_exception(ex_ptr);
                              }
                        }
                        catch (const std::exception& e)
                        {
                              print_exception(e);
                        }
                  }
            }
      };

}
