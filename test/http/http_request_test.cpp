#include <net/http/http_request.hpp>

#include "net_test.hpp"

class HttpRequestTest : public testing::Test {};

TEST_F(HttpRequestTest, ParseUrl) {
  net::HttpRequest request;
  request.SetUrl("/home");
  EXPECT_EQ(request.GetRouteUrl(), "/home");
  EXPECT_EQ(request.GetRawParams(), "");
  request.SetUrl("/home?");
  EXPECT_EQ(request.GetRouteUrl(), "/home");
  EXPECT_EQ(request.GetRawParams(), "");
  request.SetUrl("/home?a=b");
  EXPECT_EQ(request.GetRouteUrl(), "/home");
  EXPECT_EQ(request.GetRawParams(), "a=b");
  request.SetUrl("/home/");
  EXPECT_EQ(request.GetRouteUrl(), "/home/");
  EXPECT_EQ(request.GetRawParams(), "");
  request.SetUrl("/home/?");
  EXPECT_EQ(request.GetRouteUrl(), "/home/");
  EXPECT_EQ(request.GetRawParams(), "");
  request.SetUrl("/home/?a=b");
  EXPECT_EQ(request.GetRouteUrl(), "/home/");
  EXPECT_EQ(request.GetRawParams(), "a=b");
}
