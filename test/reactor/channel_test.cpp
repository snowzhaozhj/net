#include <net/reactor/channel.hpp>

#include "net_test.hpp"

class ChannelTest : public testing::Test {};

TEST_F(ChannelTest, GetFd) {
  net::Channel channel(5);
  EXPECT_EQ(channel.GetFd(), 5);
}

TEST_F(ChannelTest, State) {
  net::Channel channel(1);
  EXPECT_EQ(channel.GetState(), net::Channel::State::Add);
  channel.SetState(net::Channel::State::Mod);
  EXPECT_EQ(channel.GetState(), net::Channel::State::Mod);
  channel.SetState(net::Channel::State::Add);
  EXPECT_EQ(channel.GetState(), net::Channel::State::Add);
}

TEST_F(ChannelTest, HandleEvents) {
  int num = 0;
  net::Channel channel(5);
  channel.SetReadCallback([&] { num += 1; });
  channel.SetWriteCallback([&] { num += 2; });
  channel.SetCloseCallback([&] { num += 4; });
  channel.SetErrorCallback([&] { num += 8; });

  channel.SetREvents(EPOLLIN);
  channel.HandleEvents();
  EXPECT_EQ(num, 1);

  channel.SetREvents(EPOLLOUT);
  channel.HandleEvents();
  EXPECT_EQ(num, 3);

  channel.SetREvents(EPOLLHUP);
  channel.HandleEvents();
  EXPECT_EQ(num, 7);

  channel.SetREvents(EPOLLERR);
  channel.HandleEvents();
  EXPECT_EQ(num, 15);

  num = 0;
  int revents = 0;
  revents |= EPOLLIN;
  revents |= EPOLLOUT;
  channel.SetREvents(revents);
  channel.HandleEvents();
  EXPECT_EQ(num, 3);
}
