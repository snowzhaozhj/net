#include <net/log.hpp>

#include "net_test.hpp"

#if DEBUG_ENABLED

class LogTest : public testing::Test {};

void CallFatal() {
  LOG_FATAL("fatal message");
}

TEST_F(LogTest, Fatal) {
  EXPECT_DEATH(CallFatal(), "");
}

#endif
