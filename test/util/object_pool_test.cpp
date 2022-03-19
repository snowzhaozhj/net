#include <net/util/object_pool.hpp>

#include "net_test.hpp"

#include <thread>

class ObjectPoolTest : public testing::Test {};

struct A {
  int a;
  bool b;
  char c;
};

TEST_F(ObjectPoolTest, NewShared) {
  {
    auto object = net::object_pool::NewShared<A>();
    object->a = 3;
    object->b = true;
    object->c = 'a';
  }
  auto object = net::object_pool::NewShared<A>();
  EXPECT_EQ(object->a, 3);
  EXPECT_EQ(object->b, true);
  EXPECT_EQ(object->c, 'a');
}

TEST_F(ObjectPoolTest, NewUnique) {
  {
    auto object = net::object_pool::NewUnique<A>();
    object->a = 5;
    object->b = true;
    object->c = 'c';
  }
  auto object = net::object_pool::NewUnique<A>();
  EXPECT_EQ(object->a, 5);
  EXPECT_EQ(object->b, true);
  EXPECT_EQ(object->c, 'c');
}

TEST_F(ObjectPoolTest, New) {
  auto object = net::object_pool::New<A>();
  object->a = 7;
  object->b = false;
  object->c = 'd';
  net::object_pool::Delete(object);
  object = net::object_pool::New<A>();
  EXPECT_EQ(object->a, 7);
  EXPECT_EQ(object->b, false);
  EXPECT_EQ(object->c, 'd');
}

TEST_F(ObjectPoolTest, TLS) {
  std::thread t1([] {
    {
      auto object = net::object_pool::NewShared<A>();
      object->a = 7;
      object->b = false;
      object->c = 'd';
    }
    auto object = net::object_pool::NewShared<A>();
    EXPECT_EQ(object->a, 7);
    EXPECT_EQ(object->b, false);
    EXPECT_EQ(object->c, 'd');
  });
  std::thread t2([] {
    {
      auto object = net::object_pool::NewShared<A>();
      object->a = 5;
      object->b = true;
      object->c = 'm';
    }
    auto object = net::object_pool::NewShared<A>();
    EXPECT_EQ(object->a, 5);
    EXPECT_EQ(object->b, true);
    EXPECT_EQ(object->c, 'm');
  });
  t1.join();
  t2.join();
}
