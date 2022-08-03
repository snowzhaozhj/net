#include <net/rpc/rpc_router.hpp>

#include "net_test.hpp"

namespace net {

class RpcRouterTest : public testing::Test {
 public:
  RpcRouter router_;
};

TEST_F(RpcRouterTest, NoArgNoRetLambda) {
  bool say_hello_called = false;
  router_.Register("SayHello", [&say_hello_called]() {
    say_hello_called = true;
  });
  RpcRequestBody request;
  request.method = "SayHello";
  request.args = SerializeArgsToBuffer();
  auto reply = router_.Call(request);
  EXPECT_EQ(reply.error_code, RpcErrorCode::kSuccess);
  EXPECT_TRUE(say_hello_called);

  say_hello_called = false;
  request.method = "SayHi";
  reply = router_.Call(request);
  EXPECT_EQ(reply.error_code, RpcErrorCode::kMethodNotFound);
  EXPECT_FALSE(say_hello_called);
}

int Add(int a, int b, int &c) {
  return a + b + c;
}

TEST_F(RpcRouterTest, HasArgHasRetNormalFunc) {
  router_.Register("Add", Add);
  RpcRequestBody request;
  request.method = "Add";
  request.args = SerializeArgsToBuffer(1, 2, 3);
  auto reply = router_.Call(request);
  EXPECT_EQ(reply.error_code, RpcErrorCode::kSuccess);
  EXPECT_EQ(GetReturnValueFromBuffer<int>(reply.ret), 6);
}

TEST_F(RpcRouterTest, NoArgHasReturnStdFunc) {
  std::function<char()> return_c = []() { return 'c'; };
  router_.Register("ReturnC", return_c);
  RpcRequestBody request;
  request.method = "ReturnC";
  request.args = SerializeArgsToBuffer();
  auto reply = router_.Call(request);
  EXPECT_EQ(reply.error_code, RpcErrorCode::kSuccess);
  EXPECT_EQ(GetReturnValueFromBuffer<char>(reply.ret), 'c');
}

}
