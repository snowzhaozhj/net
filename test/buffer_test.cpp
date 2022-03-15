#include <net/buffer.hpp>

#include "net_test.hpp"

class BufferTest : public testing::Test {};

TEST_F(BufferTest, Append) {
  net::Buffer buffer;
  buffer.Append("hello");
  EXPECT_EQ(buffer.ReadableBytes(), 5);
  EXPECT_EQ(buffer.RetriveAll(), "hello");
  EXPECT_EQ(buffer.ReadableBytes(), 0);

  std::string s("world");
  buffer.Append(s.data(), s.size());
  EXPECT_EQ(buffer.ReadableBytes(), 5);
  EXPECT_EQ(buffer.RetriveAll(), "world");
  EXPECT_EQ(buffer.ReadableBytes(), 0);

  char ca[14] = "hello, world!";
  buffer.Append(ca, 13);
  EXPECT_EQ(buffer.ReadableBytes(), 13);
  EXPECT_EQ(buffer.RetriveAll(), "hello, world!");
  EXPECT_EQ(buffer.ReadableBytes(), 0);

  char ca2[] = "world";
  std::string s2("!");
  buffer.Append("hello");
  buffer.Append(", ");
  buffer.Append(ca2, 5);
  buffer.Append(s2);
  buffer.Append(s2.data(), s2.size());
  EXPECT_EQ(buffer.ReadableBytes(), 14);
  EXPECT_EQ(buffer.RetriveAll(), "hello, world!!");
  EXPECT_EQ(buffer.ReadableBytes(), 0);
}

TEST_F(BufferTest, AppendMoreThanInitSize) {
  net::Buffer buffer(10);
  EXPECT_EQ(buffer.ReadableBytes(), 0);
  buffer.Append("hello, world!");
  EXPECT_EQ(buffer.ReadableBytes(), 13);
  EXPECT_EQ(buffer.RetriveAll(), "hello, world!");
  EXPECT_EQ(buffer.ReadableBytes(), 0);
}

TEST_F(BufferTest, Find) {
  net::Buffer buffer("hello\n");
  EXPECT_EQ(buffer.ReadableBytes(), 6);
  EXPECT_EQ(buffer.Find('m'), nullptr);
  auto pos = buffer.Find('e');
  EXPECT_NE(pos, nullptr);
  EXPECT_EQ(buffer.RetriveTo(pos), "h");
  EXPECT_EQ(buffer.ReadableBytes(), 5);
  EXPECT_EQ(buffer.Find('h'), nullptr);
  pos = buffer.Find('l');
  EXPECT_NE(pos, nullptr);
  EXPECT_EQ(buffer.RetriveTo(pos + 1), "el");
  EXPECT_EQ(buffer.ReadableBytes(), 3);
  pos = buffer.Find('l');
  EXPECT_NE(pos, nullptr);
  EXPECT_EQ(buffer.RetriveAll(), "lo\n");
  EXPECT_EQ(buffer.ReadableBytes(), 0);
}

TEST_F(BufferTest, Search) {
  net::Buffer buffer("hello, world!\r\n");
  EXPECT_EQ(buffer.ReadableBytes(), 15);
  EXPECT_EQ(buffer.Search("le"), nullptr);
  auto pos = buffer.Search(", ");
  EXPECT_NE(pos, nullptr);
  EXPECT_EQ(buffer.RetriveTo(pos), "hello");
  EXPECT_EQ(buffer.ReadableBytes(), 10);
  buffer.HasRead(2);
  EXPECT_EQ(buffer.ReadableBytes(), 8);
  pos = buffer.Search("\r\n");
  EXPECT_EQ(buffer.RetriveTo(pos), "world!");
  EXPECT_EQ(buffer.ReadableBytes(), 2);
  EXPECT_EQ(buffer.RetriveAll(), "\r\n");
  EXPECT_EQ(buffer.ReadableBytes(), 0);
}
