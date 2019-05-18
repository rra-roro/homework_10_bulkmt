#include "gtest/gtest.h"
#include "gtest/gtest_prod.h"

#include <string_view>
#include <filesystem>

#include "lib_version.h"

#define PRIVATE_TEST 1
#include "publisher.h"
#include "bulk_reader.h"
#include "queue.h"
#include "thread_fn.h"
#include "thread_mgr.h"

TEST(version, test1)
{
      ASSERT_TRUE(version() > 0);
}

namespace roro_lib
{
      struct publisher : public publisher_mixin<void(void)>
      {
            void run()
            {
                  notify();
            };
      };


      void subscriber_fn() {}

      struct subscriber_functor
      {
            void test(){};
            void operator()(){};
      };


      class PublisherMixinTest : public testing::Test
      {
        public:
            void SetUp() {}
            void TearDown() {}
      };

      TEST_F(PublisherMixinTest, UniqueAddSubscribers1)
      {
            publisher pbl;

            pbl.subscribe(subscriber_fn);
            ASSERT_TRUE(pbl.subscribers.size() == 1);

            pbl.subscribe(subscriber_functor()); // rvalue - добавляем
            ASSERT_TRUE(pbl.subscribers.size() == 2);

            subscriber_functor sf;

            pbl.subscribe(sf);
            ASSERT_TRUE(pbl.subscribers.size() == 3);

            pbl.subscribe(sf, &subscriber_functor::test);
            ASSERT_TRUE(pbl.subscribers.size() == 4);

            std::function<void(void)> fn1 = subscriber_fn;
            pbl.subscribe(fn1);
            ASSERT_TRUE(pbl.subscribers.size() == 5);

            pbl.subscribe(std::function<void(void)>(subscriber_fn)); //  rvalue - добавляем
            ASSERT_TRUE(pbl.subscribers.size() == 6);


            // ----
            pbl.subscribe(subscriber_functor()); //  rvalue - добавляем повторно
            ASSERT_TRUE(pbl.subscribers.size() == 7);

            pbl.subscribe(std::function<void(void)>(subscriber_fn)); //   rvalue - добавляем повторно
            ASSERT_TRUE(pbl.subscribers.size() == 8);

            // ----
            pbl.subscribe(subscriber_fn);                 // Уже есть в подписчиках
            pbl.subscribe(sf);                            // Уже есть в подписчиках
            pbl.subscribe(sf, &subscriber_functor::test); // Уже есть в подписчиках

            std::function<void(void)> fn2;
            pbl.subscribe(fn1); // function == nullptr  - игнорируем
            pbl.subscribe(fn2); // Уже есть в подписчиках

            // ----
            auto l1 = []() { int i = 1; i++; };

            pbl.subscribe(l1);
            ASSERT_TRUE(pbl.subscribers.size() == 9);

            pbl.subscribe(l1);
            ASSERT_TRUE(pbl.subscribers.size() == 9); // Уже есть в подписчиках

            pbl.subscribe([]() { int i = 1; i++; }); //  rvalue
            ASSERT_TRUE(pbl.subscribers.size() == 10);

            pbl.subscribe([]() { int i = 1; i++; }); //  rvalue - добавляем повторно
            ASSERT_TRUE(pbl.subscribers.size() == 11);

            ASSERT_TRUE(pbl.subscribers.size() == 11);
      }


      struct subscriber_base0
      {
            int m[52];
            void test0(){};
      };

      struct subscriber_base1
      {
            int m[30];
            void test0(){};
            void test1(){};
            void test2(){};
      };

      struct subscriber_base2 : subscriber_base1
      {
            int m[256];
            void test2(){};
      };

      struct subscriber_d : subscriber_base2
      {
            void test0(){};
      };

      struct subscriber_md : subscriber_base0, subscriber_base1
      {
            void test0(){};
      };

      TEST_F(PublisherMixinTest, UniqueAddSubscribers2)
      {
            publisher pbl;
            subscriber_d sd;
            subscriber_md md;

            pbl.subscribe(sd, &subscriber_d::test0);
            ASSERT_TRUE(pbl.subscribers.size() == 1);
            pbl.subscribe(sd, &subscriber_d::test1);
            ASSERT_TRUE(pbl.subscribers.size() == 2);
            pbl.subscribe(sd, &subscriber_d::test2);
            ASSERT_TRUE(pbl.subscribers.size() == 3);

            pbl.subscribe(sd, &subscriber_d::subscriber_base1::test0);
            ASSERT_TRUE(pbl.subscribers.size() == 4);
            pbl.subscribe(sd, &subscriber_d::subscriber_base1::test2);
            ASSERT_TRUE(pbl.subscribers.size() == 5);

            pbl.subscribe(sd, &subscriber_d::test0);                   // Уже есть в подписчиках
            pbl.subscribe(sd, &subscriber_d::subscriber_base1::test2); // Уже есть в подписчиках

            ASSERT_TRUE(pbl.subscribers.size() == 5);

            // pbl.subscribe(md, &subscriber_md::test0);                // Подписчик напрямую не может быть типом с
            //                                                                  Множественным и/или виртуальным наследованием

            pbl.subscribe(std::bind(&subscriber_md::test0, md)); // Но через bind может
            ASSERT_TRUE(pbl.subscribers.size() == 6);

            pbl.subscribe(std::bind(&subscriber_md::test0, md)); // через bind подписываем многократно
            ASSERT_TRUE(pbl.subscribers.size() == 7);
      }


      struct i_subscriber
      {
            virtual void test0() = 0;
            virtual void test1() = 0;
      };

      struct subscriber_base : i_subscriber
      {
            void test0(){};
            void test1(){};
            virtual void test2(){};
      };

      struct subscriber_d2 : subscriber_base
      {
            void test2(){};
      };

      TEST_F(PublisherMixinTest, UniqueAddSubscribers3)
      {
            publisher pbl;

            subscriber_d2 sd1;
            subscriber_d2 sd2;


            //i_subscriber& is = sd2;
            subscriber_base& sb = sd2;

            pbl.subscribe(sd1, &subscriber_d2::test0);
            ASSERT_TRUE(pbl.subscribers.size() == 1);
            pbl.subscribe(sd1, &subscriber_d2::test1);
            ASSERT_TRUE(pbl.subscribers.size() == 2);
            pbl.subscribe(sd1, &subscriber_d2::test2);
            ASSERT_TRUE(pbl.subscribers.size() == 3);

            //pbl.subscribe(is, &i_subscriber::test0);   // is  - нельзя инстанцировать
            //pbl.subscribe(is, &i_subscriber::test1);
            pbl.subscribe(sb, &subscriber_d2::test0);
            ASSERT_TRUE(pbl.subscribers.size() == 4);
            pbl.subscribe(sb, &subscriber_d2::test1);
            ASSERT_TRUE(pbl.subscribers.size() == 5);
            //pbl.subscribe(sb, &subscriber_d2::test2);  // &subscriber_d2::test2 - обращение к ф-ии что нет в subscriber_base
      }


      struct subscriber_state
      {
            int st = 0;
            void test() { st += 1; };
            void operator()() { st += 2; };
      };

      TEST_F(PublisherMixinTest, StoreSubscribersByRef)
      {
            publisher pbl;

            subscriber_state sscr;

            pbl.subscribe(sscr);
            pbl.subscribe(sscr, &subscriber_state::test);
            pbl.run();

            ASSERT_TRUE(sscr.st == 3);
      }

      TEST_F(PublisherMixinTest, StoreSubscribersByValue)
      {
            publisher pbl;

            subscriber_state sscr;

            pbl.subscribe(std::move(sscr));
            pbl.subscribe(std::move(sscr), &subscriber_state::test);

            pbl.subscribe(subscriber_state());
            pbl.subscribe(subscriber_state(), &subscriber_state::test);

            pbl.run();

            ASSERT_TRUE(sscr.st == 0);
      }

      TEST_F(PublisherMixinTest, copy)
      {
            std::function<void(void)> fn1 = subscriber_fn;
            subscriber_functor sf;

            publisher pbl1;
            pbl1.subscribe(subscriber_fn);
            pbl1.subscribe(subscriber_functor());
            pbl1.subscribe(sf);
            pbl1.subscribe(sf, &subscriber_functor::test);
            pbl1.subscribe(fn1);
            pbl1.subscribe(std::function<void(void)>(subscriber_fn));

            ASSERT_TRUE(pbl1.subscribers.size() == 6);

            publisher pbl2(pbl1);
            ASSERT_TRUE(pbl2.subscribers.size() == 6);

            publisher pbl3;
            pbl3 = pbl1;
            ASSERT_TRUE(pbl3.subscribers.size() == 6);
      }

      TEST_F(PublisherMixinTest, move)
      {
            std::function<void(void)> fn1 = subscriber_fn;
            subscriber_functor sf;

            publisher pbl1;
            pbl1.subscribe(subscriber_fn);
            pbl1.subscribe(subscriber_functor());
            pbl1.subscribe(sf);
            pbl1.subscribe(sf, &subscriber_functor::test);
            pbl1.subscribe(fn1);
            pbl1.subscribe(std::function<void(void)>(subscriber_fn));

            ASSERT_TRUE(pbl1.subscribers.size() == 6);

            publisher pbl2(std::move(pbl1));
            ASSERT_TRUE(pbl2.subscribers.size() == 6);

            publisher pbl3;
            pbl3 = std::move(pbl2);
            ASSERT_TRUE(pbl3.subscribers.size() == 6);
      }

      struct subscriber_exeption
      {
            void test1() { throw std::runtime_error("Ex test1()"); };
            void test2() { throw std::runtime_error("Ex test2()"); };
            void test3() { throw std::runtime_error("Ex test3()"); };
      };

      TEST_F(PublisherMixinTest, exeption)
      {
            publisher pbl;
            subscriber_exeption sbscrex;

            pbl.subscribe(sbscrex, &subscriber_exeption::test1);
            pbl.subscribe(sbscrex, &subscriber_exeption::test2);
            pbl.subscribe(sbscrex, &subscriber_exeption::test3);
            ASSERT_TRUE(pbl.subscribers.size() == 3);

            ASSERT_TRUE(pbl.get_last_notify_exception().size() == 0);

            pbl.run();

            ASSERT_TRUE(pbl.get_last_notify_exception().size() == 3);
      }


      struct subscriber_taged
      {
            int i = 0;
            subscriber_taged(int arg) : i(arg){};
            void test1(){};
            void test2(){};
            void test3(){};
      };


      TEST_F(PublisherMixinTest, DelSubscribers)
      {
            publisher pbl;
            publisher::subscriber_handle handle1 = pbl.subscribe(subscriber_taged(1), &subscriber_taged::test1);
            publisher::subscriber_handle handle2 = pbl.subscribe(subscriber_taged(2), &subscriber_taged::test2);
            publisher::subscriber_handle handle3 = pbl.subscribe(subscriber_taged(3), &subscriber_taged::test3);
            publisher::subscriber_handle handle4 = pbl.subscribe(subscriber_taged(4), &subscriber_taged::test1);
            publisher::subscriber_handle handle5 = pbl.subscribe(subscriber_taged(5), &subscriber_taged::test2);
            publisher::subscriber_handle handle6 = pbl.subscribe(subscriber_taged(6), &subscriber_taged::test3);
            ASSERT_TRUE(pbl.subscribers.size() == 6);

            pbl.unsubscribe(handle3);
            pbl.unsubscribe(handle4);

            ASSERT_TRUE(pbl.subscribers.size() == 4);

            pbl.unsubscribe(handle3);
            pbl.unsubscribe(handle4);

            ASSERT_TRUE(pbl.subscribers.size() == 4);

            pbl.unsubscribe_all();
            ASSERT_TRUE(pbl.subscribers.size() == 0);
      }


      template <typename Base = publisher_mixin<int(int, int)>>
      struct publisher_not_void : public Base
      {
            using Base::notify;
            using Base::get_fn_by_handle;
            using Base::get_last_notify_exception;

            using notify_ret_t = std::invoke_result_t<decltype(&publisher_not_void::notify), Base, int, int>;

        public:
            notify_ret_t retvs;

            int run()
            {
                  int sum = 0;
                  retvs = notify(1, 1);

                  for (auto& retv : retvs)
                  {
                        sum += retv.first;
                  }
                  return sum;
            };
      };

      int fn1(int a1, int a2)
      {
            return a1 + a2;
      }

      int fn2(int a1, int a2)
      {
            return a1 + a2 + 1;
      }

      int fn3(int a1, int a2)
      {
            return a1 + a2 + 3;
      }

      TEST_F(PublisherMixinTest, NotifiRetValues)
      {
            publisher_not_void pbl;
            pbl.subscribe(fn1);
            pbl.subscribe(fn2);
            pbl.subscribe(fn3);
            ASSERT_TRUE(pbl.subscribers.size() == 3);

            pbl.subscribe(fn1);
            pbl.subscribe(fn2);
            pbl.subscribe(fn3);
            ASSERT_TRUE(pbl.subscribers.size() == 3);

            int sum = pbl.run();

            ASSERT_TRUE(pbl.retvs.size() == 3);
            ASSERT_TRUE(sum == 10);
      }

      TEST(Command_Reader, ParserTest)
      {
            command_reader cmdr(3);

            auto read = [&](std::string_view sv) {
                  std::stringstream ss;
                  ss << sv;
                  cmdr.read(ss);
            };

            size_t index_calls = 0;

            cmdr.subscribe([&](const std::vector<std::string>& command_list, std::time_t t) {
                  switch (index_calls)
                  {
                  case 0:
                        ASSERT_TRUE(command_list.size() == 3);
                        ASSERT_TRUE(command_list[0] == "cmd1");
                        ASSERT_TRUE(command_list[1] == "cmd2");
                        ASSERT_TRUE(command_list[2] == "cmd3");
                        break;
                  case 1:
                        ASSERT_TRUE(command_list.size() == 2);
                        ASSERT_TRUE(command_list[0] == "cmd4");
                        ASSERT_TRUE(command_list[1] == "cmd5");
                        break;
                  case 2:
                        ASSERT_TRUE(command_list.size() == 4);
                        ASSERT_TRUE(command_list[0] == "cmdA");
                        ASSERT_TRUE(command_list[1] == "cmdB");
                        ASSERT_TRUE(command_list[2] == "cmdC");
                        ASSERT_TRUE(command_list[3] == "cmdD");
                        break;
                  case 3:
                        ASSERT_TRUE(command_list.size() == 1);
                        ASSERT_TRUE(command_list[0] == "cmd6");
                        break;
                  case 4:
                        ASSERT_TRUE(command_list.size() == 6);
                        ASSERT_TRUE(command_list[0] == "cmdA");
                        ASSERT_TRUE(command_list[1] == "cmdB");
                        ASSERT_TRUE(command_list[2] == "cmdAA");
                        ASSERT_TRUE(command_list[3] == "cmdBB");
                        ASSERT_TRUE(command_list[4] == "cmdC");
                        ASSERT_TRUE(command_list[5] == "cmdD");
                        break;
                  case 5:
                        ASSERT_TRUE(command_list.size() == 1);
                        ASSERT_TRUE(command_list[0] == "cmd7");
                        break;
                  case 6:
                        ASSERT_TRUE(command_list.size() == 3);
                        ASSERT_TRUE(command_list[0] == "cmd11");
                        ASSERT_TRUE(command_list[1] == "cmd22");
                        ASSERT_TRUE(command_list[2] == "cmd33");
                        break;
                  case 7:
                        ASSERT_FALSE(command_list.size() == 2);
                        ASSERT_FALSE(command_list[0] == "cmdAN");
                        ASSERT_FALSE(command_list[1] == "cmdBN");
                        break;
                  }
                  index_calls++;
            });

            read("cmd1\ncmd2\ncmd3\n"
                 "cmd4\ncmd5\n"
                 "{\n"
                 "cmdA\ncmdB\ncmdC\ncmdD\n"
                 "}\n"
                 "cmd6\n"
                 "{\n"
                 "cmdA\ncmdB\n{\ncmdAA\ncmdBB\n}\ncmdC\ncmdD\n"
                 "}\n"
                 "cmd7\n");

            read("cmd11\ncmd22\ncmd33\n"
                 "{\n"
                 "cmdAN\ncmdBN\n");

            bool is_exception = false;
            try
            {
                  read("}\ncmdA\ncmdB\n");
            }
            catch (...)
            {
                  is_exception = true;
            }
            ASSERT_TRUE(is_exception);
            ASSERT_TRUE(index_calls == 7);
      }

      TEST(Command_Reader, BulkFileSaveTest)
      {
            namespace fs = std::filesystem;

            for (auto& p : fs::directory_iterator(fs::current_path()))
            {
                  if (p.is_regular_file() && p.path().extension() == ".log")
                        fs::remove(p);
            }
            
            using queue_tread_t = queue<std::vector<std::string>, std::time_t>;

            command_reader cmdr(3);

            auto read = [&](std::string_view sv) {
                  std::stringstream ss;
                  ss << sv;
                  cmdr.read(ss);
            };

            queue_tread_t file_queue;
            thread_mgr file_tmgr(2, file_queue, save_log_file(), &save_log_file::save);
            cmdr.subscribe([&](const auto& vec, auto t) {
                  try
                  {
                        file_queue.push_wait(vec, t);
                  }
                  catch (...)
                  {
                        std::throw_with_nested(std::runtime_error("'file' subscriber failed."));
                  }
            });

            read("cmd1\ncmd2\ncmd3\n"
                 "cmd4\ncmd5\n"
                 "{\n"
                 "cmdA\ncmdB\ncmdC\ncmdD\n"
                 "}\n"
                 "cmd6\n"
                 "{\n"
                 "cmdA\ncmdB\n{\ncmdAA\ncmdBB\n}\ncmdC\ncmdD\n"
                 "}\n"
                 "cmd7\n");

            file_tmgr.finalize_threads();

            size_t count_log_files = 0;

            for (auto& p : fs::directory_iterator(fs::current_path()))
            {
                  if (p.is_regular_file() && p.path().extension() == ".log")
                  {
                        count_log_files++;
                        fs::remove(p);
                  }                  
            }

            ASSERT_TRUE(count_log_files == 6);
      }

}
