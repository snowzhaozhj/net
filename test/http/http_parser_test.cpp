#include <net/http/http_parser.hpp>

#include "net_test.hpp"

class HttpParserTest : public testing::Test {};

TEST_F(HttpParserTest, Success) {
  std::string message = "GET /index.html HTTP/1.1\r\n"
                        "Content-Type: application/json\r\n"
                        "Content-Length: 5\r\n"
                        "\r\n"
                        "12345";
  auto buffer = std::make_shared<net::Buffer>(message);
  net::HttpRequest request;
  EXPECT_TRUE(net::Parse(buffer, request));
  EXPECT_EQ(request.GetMethod(), net::HttpMethod::Get);
  EXPECT_EQ(request.GetUrl(), "/index.html");
  EXPECT_EQ(request.GetVersion(), net::HttpVersion::Http11);
  EXPECT_EQ(request.GetHeader("Content-Type"), "application/json");
  EXPECT_EQ(request.GetHeader("Content-Length"), "5");
  EXPECT_EQ(request.GetContent(), "12345");
}

TEST_F(HttpParserTest, Failed) {
  std::string message = "GET /index.html HTTP/1.1\r\n"
                        "12345";
  auto buffer = std::make_shared<net::Buffer>(message);
  net::HttpRequest request;
  EXPECT_FALSE(net::Parse(buffer, request));
}
