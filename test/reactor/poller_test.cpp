#include <net/reactor/poller.hpp>

#include "net_test.hpp"

class PollerTest : public testing::Test {
 public:
  net::Poller poller_;
};

TEST_F(PollerTest, UpdateChannel) {
  net::Channel channel(1);
  channel.EnableRead();
  EXPECT_EQ(channel.GetState(), net::Channel::State::Add);
  poller_.UpdateChannel(&channel);
  EXPECT_EQ(channel.GetState(), net::Channel::State::Mod);
  channel.EnableWrite();
  poller_.UpdateChannel(&channel);
  EXPECT_EQ(channel.GetState(), net::Channel::State::Mod);
  channel.DisableAll();
  poller_.UpdateChannel(&channel);
  EXPECT_EQ(channel.GetState(), net::Channel::State::Add);
}

TEST_F(PollerTest, RemoveChannel) {
  net::Channel channel(1);
  channel.EnableRead();
  EXPECT_EQ(channel.GetState(), net::Channel::State::Add);
  poller_.UpdateChannel(&channel);
  EXPECT_EQ(channel.GetState(), net::Channel::State::Mod);
  poller_.RemoveChannel(&channel);
  EXPECT_EQ(channel.GetState(), net::Channel::State::Add);
}
