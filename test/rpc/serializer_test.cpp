#include <net/rpc/serializer.hpp>

#include "net_test.hpp"

namespace net {

class SerializerTest : public testing::Test {
 public:
  SerializerTest() {
    EXPECT_EQ(s.ReadableBytes(), 0);
  }

  ~SerializerTest() override {
    EXPECT_EQ(s.ReadableBytes(), 0);
  }

  Serializer s;
};

TEST_F(SerializerTest, Base) {
  s << (int8_t) 1 << (int16_t) 2 << (int32_t) 3 << (int64_t) 4
    << (uint8_t) 5 << (uint16_t) 6 << (uint32_t) 7 << (uint64_t) 8
    << 'a';
  EXPECT_EQ(Get<int8_t>(s), (int8_t) 1);
  EXPECT_EQ(Get<int16_t>(s), (int16_t) 2);
  EXPECT_EQ(Get<int32_t>(s), (int32_t) 3);
  EXPECT_EQ(Get<int64_t>(s), (int64_t) 4);
  EXPECT_EQ(Get<uint8_t>(s), (uint8_t) 5);
  EXPECT_EQ(Get<uint16_t>(s), (uint16_t) 6);
  EXPECT_EQ(Get<uint32_t>(s), (uint32_t) 7);
  EXPECT_EQ(Get<uint64_t>(s), (uint64_t) 8);
  EXPECT_EQ(Get<char>(s), 'a');
}

TEST_F(SerializerTest, Float) {
  float a1 = 12.15894;
  float a2 = 14123.2341;
  s << a1 << a2;
  auto ra1 = Get<float>(s);
  auto ra2 = Get<float>(s);
  EXPECT_FLOAT_EQ(a1, ra1);
  EXPECT_FLOAT_EQ(a2, ra2);

  double b1 = 12341234.12341234;
  double b2 = 132134.2143123;
  s << b1 << b2;
  auto rb1 = Get<double>(s);
  auto rb2 = Get<double>(s);
  EXPECT_DOUBLE_EQ(b1, rb1);
  EXPECT_DOUBLE_EQ(b2, rb2);
}

TEST_F(SerializerTest, String) {
  s << "Hello, world";
  auto rstr = Get<std::string>(s);
  EXPECT_EQ(rstr, "Hello, world");
  s << "Hello" << ", world";
  auto rstr1 = Get<std::string>(s);
  EXPECT_EQ(rstr1, "Hello");
  auto rstr2 = Get<std::string>(s);
  EXPECT_EQ(rstr2, ", world");
}

TEST_F(SerializerTest, Pair) {
  std::pair<int, char> p{1, 'a'};
  s << p;
  auto rp = Get<std::pair<int, char>>(s);
  EXPECT_EQ(rp.first, 1);
  EXPECT_EQ(rp.second, 'a');
}

TEST_F(SerializerTest, Tuple) {
  std::tuple<int, char, std::string, float> t{2, 'b', "Hello", 1.34};
  s << t;
  auto rp = Get<decltype(t)>(s);
  EXPECT_EQ(std::get<0>(t), std::get<0>(rp));
  EXPECT_EQ(std::get<1>(t), std::get<1>(rp));
  EXPECT_EQ(std::get<2>(t), std::get<2>(rp));
  EXPECT_FLOAT_EQ(std::get<3>(t), std::get<3>(rp));
}

namespace {

template<template<typename T> typename Container>
void TestBaseContainer(Serializer &s) {
  Container<int> ci{4, -3, 999, INT_MAX, INT_MIN};
  s << ci;
  EXPECT_THAT(Get<Container<int>>(s), testing::ElementsAre(4, -3, 999, INT_MAX, INT_MIN));
  EXPECT_EQ(s.ReadableBytes(), 0);

  Container<uint32_t> cui{0, 1, 3, UINT32_MAX};
  s << cui;
  EXPECT_THAT(Get<Container<uint32_t>>(s), testing::ElementsAre(0, 1, 3, UINT32_MAX));
  EXPECT_EQ(s.ReadableBytes(), 0);

  Container<std::string> cs{"str1", "str23", "str345"};
  s << cs;
  EXPECT_THAT(Get<Container<std::string>>(s), testing::ElementsAre("str1", "str23", "str345"));
  EXPECT_EQ(s.ReadableBytes(), 0);
}

template<template<typename Key> typename Set>
void TestSet(Serializer &s) {
  Set<std::string> str_set;
  str_set.emplace("Hello");
  str_set.emplace("Hello");
  str_set.emplace("Hello");
  str_set.emplace("World");
  s << str_set;
  auto rstr_set = Get<decltype(str_set)>(s);
  EXPECT_EQ(rstr_set.size(), 2);
  EXPECT_EQ(rstr_set.count("Hello"), 1);
  EXPECT_EQ(rstr_set.count("World"), 1);
  EXPECT_EQ(s.ReadableBytes(), 0);

  Set<int> int_set;
  int_set.emplace(1);
  int_set.emplace(2);
  int_set.emplace(1);
  int_set.emplace(2);
  s << int_set;
  auto rint_set = Get<decltype(int_set)>(s);
  EXPECT_EQ(rint_set.size(), 2);
  EXPECT_EQ(rint_set.count(1), 1);
  EXPECT_EQ(rint_set.count(2), 1);
  EXPECT_EQ(s.ReadableBytes(), 0);
}

template<template<typename Key> typename MultiSet>
void TestMultiSet(Serializer &s) {
  MultiSet<std::string> str_set;
  str_set.emplace("Hello");
  str_set.emplace("Hello");
  str_set.emplace("Hello");
  str_set.emplace("World");
  s << str_set;
  auto rstr_set = Get<decltype(str_set)>(s);
  EXPECT_EQ(rstr_set.size(), 4);
  EXPECT_EQ(rstr_set.count("Hello"), 3);
  EXPECT_EQ(rstr_set.count("World"), 1);
  EXPECT_EQ(s.ReadableBytes(), 0);

  MultiSet<int> int_set;
  int_set.emplace(1);
  int_set.emplace(2);
  int_set.emplace(1);
  int_set.emplace(2);
  s << int_set;
  auto rint_set = Get<decltype(int_set)>(s);
  EXPECT_EQ(rint_set.size(), 4);
  EXPECT_EQ(rint_set.count(1), 2);
  EXPECT_EQ(rint_set.count(2), 2);
  EXPECT_EQ(s.ReadableBytes(), 0);
}

template<template<typename Key, typename Value> typename Map>
void TestMap(Serializer &s) {
  Map<std::string, int> stoi_map;
  stoi_map["Hello"] = 4;
  stoi_map["World"] = 5;
  s << stoi_map;
  auto rstoi_map = Get<decltype(stoi_map)>(s);
  EXPECT_EQ(rstoi_map.size(), 2);
  EXPECT_TRUE(rstoi_map.find("Hello") != rstoi_map.end());
  EXPECT_EQ(rstoi_map["Hello"], 4);
  EXPECT_TRUE(rstoi_map.find("World") != rstoi_map.end());
  EXPECT_EQ(rstoi_map["World"], 5);
  EXPECT_EQ(s.ReadableBytes(), 0);
}

template<template<typename Key, typename Value> typename MultiMap>
void TestMultiMap(Serializer &s) {
  MultiMap<std::string, int> stoi_mmap;
  stoi_mmap.emplace("Hello", 4);
  stoi_mmap.emplace("World", 7);
  stoi_mmap.emplace("Hello", 5);
  s << stoi_mmap;
  auto rstoi_mmap = Get<decltype(stoi_mmap)>(s);
  EXPECT_EQ(rstoi_mmap.size(), 3);
  EXPECT_EQ(rstoi_mmap.count("Hello"), 2);
  EXPECT_EQ(rstoi_mmap.count("World"), 1);
  EXPECT_EQ(rstoi_mmap.find("World")->second, 7);
  auto [hb, he] = rstoi_mmap.equal_range("Hello");
  int sum = 0;
  while (hb != he) {
    sum += hb->second;
    ++hb;
  }
  EXPECT_EQ(sum, 9);
}

}

TEST_F(SerializerTest, BaseContainer) {
  TestBaseContainer<std::vector>(s);
  TestBaseContainer<std::deque>(s);
  TestBaseContainer<std::list>(s);
}

TEST_F(SerializerTest, Set) {
  TestSet<std::set>(s);
  TestSet<std::unordered_set>(s);
  TestMultiSet<std::multiset>(s);
  TestMultiSet<std::unordered_multiset>(s);
}

TEST_F(SerializerTest, Map) {
  TestMap<std::map>(s);
  TestMap<std::unordered_map>(s);
  TestMultiMap<std::multimap>(s);
  TestMultiMap<std::unordered_multimap>(s);
}

}
