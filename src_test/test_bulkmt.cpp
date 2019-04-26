#include "gtest/gtest.h"
#include "gtest/gtest_prod.h"

#include "lib_version.h"

#define PRIVATE_TEST 1
#include "publisher.h"

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


      void subscriber_fn(){}

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

            pbl.add_subscriber(subscriber_fn); 
            ASSERT_TRUE(pbl.subscribers.size() == 1);

            pbl.add_subscriber(subscriber_functor());                    // rvalue - добавляем
            ASSERT_TRUE(pbl.subscribers.size() == 2);

            subscriber_functor sf;

            pbl.add_subscriber(sf); 
            ASSERT_TRUE(pbl.subscribers.size() == 3);

            pbl.add_subscriber(sf, &subscriber_functor::test); 
            ASSERT_TRUE(pbl.subscribers.size() == 4);

            std::function<void(void)> fn1 = subscriber_fn;
            pbl.add_subscriber(fn1);
            ASSERT_TRUE(pbl.subscribers.size() == 5); 

            pbl.add_subscriber(std::function<void(void)>(subscriber_fn));  //  rvalue - добавляем
            ASSERT_TRUE(pbl.subscribers.size() == 6);


            // ----
            pbl.add_subscriber(subscriber_functor());                      //  rvalue - добавляем повторно
            ASSERT_TRUE(pbl.subscribers.size() == 7);

            pbl.add_subscriber(std::function<void(void)>(subscriber_fn)); //   rvalue - добавляем повторно
            ASSERT_TRUE(pbl.subscribers.size() == 8);

            // ----
            pbl.add_subscriber(subscriber_fn);                            // Уже есть в подписчиках
            pbl.add_subscriber(sf);                                       // Уже есть в подписчиках
            pbl.add_subscriber(sf, &subscriber_functor::test);            // Уже есть в подписчиках

            std::function<void(void)> fn2;
            pbl.add_subscriber(fn1);                 // function == nullptr  - игнорируем
            pbl.add_subscriber(fn2);                 // Уже есть в подписчиках


            ASSERT_TRUE(pbl.subscribers.size() == 8);
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

            pbl.add_subscriber(sd, &subscriber_d::test0);
            ASSERT_TRUE(pbl.subscribers.size() == 1);
            pbl.add_subscriber(sd, &subscriber_d::test1);
            ASSERT_TRUE(pbl.subscribers.size() == 2);
            pbl.add_subscriber(sd, &subscriber_d::test2);
            ASSERT_TRUE(pbl.subscribers.size() == 3);

            pbl.add_subscriber(sd, &subscriber_d::subscriber_base1::test0);
            ASSERT_TRUE(pbl.subscribers.size() == 4);
            pbl.add_subscriber(sd, &subscriber_d::subscriber_base1::test2);
            ASSERT_TRUE(pbl.subscribers.size() == 5);

            pbl.add_subscriber(sd, &subscriber_d::test0);                   // Уже есть в подписчиках
            pbl.add_subscriber(sd, &subscriber_d::subscriber_base1::test2); // Уже есть в подписчиках

            ASSERT_TRUE(pbl.subscribers.size() == 5);

            // pbl.add_subscriber(md, &subscriber_md::test0);                // Подписчик напрямую не может быть типом с
            //                                                                  Множественным и/или виртуальным наследованием

            pbl.add_subscriber(std::bind(&subscriber_md::test0, md)); // Но через bind может
            ASSERT_TRUE(pbl.subscribers.size() == 6);

            pbl.add_subscriber(std::bind(&subscriber_md::test0, md)); // через bind подписываем многократно
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

            pbl.add_subscriber(sd1, &subscriber_d2::test0);
            ASSERT_TRUE(pbl.subscribers.size() == 1);
            pbl.add_subscriber(sd1, &subscriber_d2::test1);
            ASSERT_TRUE(pbl.subscribers.size() == 2);
            pbl.add_subscriber(sd1, &subscriber_d2::test2);
            ASSERT_TRUE(pbl.subscribers.size() == 3);

            //pbl.add_subscriber(is, &i_subscriber::test0);   // is  - нельзя инстанцировать
            //pbl.add_subscriber(is, &i_subscriber::test1);
            pbl.add_subscriber(sb, &subscriber_d2::test0);
            ASSERT_TRUE(pbl.subscribers.size() == 4);
            pbl.add_subscriber(sb, &subscriber_d2::test1);
            ASSERT_TRUE(pbl.subscribers.size() == 5);
            //pbl.add_subscriber(sb, &subscriber_d2::test2);  // &subscriber_d2::test2 - обращение к ф-ии что нет в subscriber_base
      }


      struct subscriber_state
      {
            int st = 0;
            void test() { ++st; };
            void operator()() { ++st; };
      };

      TEST_F(PublisherMixinTest, StoreSubscribersByRef)
      {
            publisher pbl;

            subscriber_state sscr;

            pbl.add_subscriber(sscr);
            pbl.add_subscriber(sscr, &subscriber_state::test);
            pbl.run();

            ASSERT_TRUE(sscr.st == 2);
      }

      TEST_F(PublisherMixinTest, StoreSubscribersByValue)
      {
            publisher pbl;

            subscriber_state sscr;

            pbl.add_subscriber(std::move(sscr));
            pbl.add_subscriber(std::move(sscr), &subscriber_state::test);

            pbl.add_subscriber(subscriber_state());
            pbl.add_subscriber(subscriber_state(), &subscriber_state::test);

            pbl.run();

            ASSERT_TRUE(sscr.st == 0);
      }

      TEST_F(PublisherMixinTest, copy)
      {
            std::function<void(void)> fn1 = subscriber_fn;
            subscriber_functor sf;

            publisher pbl1;
            pbl1.add_subscriber(subscriber_fn);
            pbl1.add_subscriber(subscriber_functor());
            pbl1.add_subscriber(sf);
            pbl1.add_subscriber(sf, &subscriber_functor::test);
            pbl1.add_subscriber(fn1);
            pbl1.add_subscriber(std::function<void(void)>(subscriber_fn));

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
            pbl1.add_subscriber(subscriber_fn);
            pbl1.add_subscriber(subscriber_functor());
            pbl1.add_subscriber(sf);
            pbl1.add_subscriber(sf, &subscriber_functor::test);
            pbl1.add_subscriber(fn1);
            pbl1.add_subscriber(std::function<void(void)>(subscriber_fn));

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

            pbl.add_subscriber(sbscrex, &subscriber_exeption::test1);
            pbl.add_subscriber(sbscrex, &subscriber_exeption::test2);
            pbl.add_subscriber(sbscrex, &subscriber_exeption::test3);
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
            publisher::subscriber_handle handle1 = pbl.add_subscriber(subscriber_taged(1), &subscriber_taged::test1);
            publisher::subscriber_handle handle2 = pbl.add_subscriber(subscriber_taged(2), &subscriber_taged::test2);
            publisher::subscriber_handle handle3 = pbl.add_subscriber(subscriber_taged(3), &subscriber_taged::test3);
            publisher::subscriber_handle handle4 = pbl.add_subscriber(subscriber_taged(4), &subscriber_taged::test1);
            publisher::subscriber_handle handle5 = pbl.add_subscriber(subscriber_taged(5), &subscriber_taged::test2);
            publisher::subscriber_handle handle6 = pbl.add_subscriber(subscriber_taged(6), &subscriber_taged::test3);
            ASSERT_TRUE(pbl.subscribers.size() == 6);

            pbl.del_subscriber(handle3);
            pbl.del_subscriber(handle4);

            ASSERT_TRUE(pbl.subscribers.size() == 4);

            pbl.del_subscriber(handle3);
            pbl.del_subscriber(handle4);

            ASSERT_TRUE(pbl.subscribers.size() == 4);

            pbl.del_all_subscribers();
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
            return a1 + a2 +1;
      }

      int fn3(int a1, int a2)
      {
            return a1 + a2 + 3;
      }

      TEST_F(PublisherMixinTest, NotifiRetValues)
      {
            publisher_not_void pbl;
            pbl.add_subscriber(fn1);
            pbl.add_subscriber(fn2);
            pbl.add_subscriber(fn3);
            ASSERT_TRUE(pbl.subscribers.size() == 3);

            pbl.add_subscriber(fn1);
            pbl.add_subscriber(fn2);
            pbl.add_subscriber(fn3);
            ASSERT_TRUE(pbl.subscribers.size() == 3);

            int sum = pbl.run();

            ASSERT_TRUE(pbl.retvs.size() == 3);
            ASSERT_TRUE(sum == 10);
      }

}
 
