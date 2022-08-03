#ifndef NET_INCLUDE_NET_RPC_RPC_PROTOCOL_HPP_
#define NET_INCLUDE_NET_RPC_RPC_PROTOCOL_HPP_

#include "net/rpc/serializer.hpp"

#include <cstdint>
#include <string>

namespace net {

enum class RpcMessageType : uint8_t {
  kHeartBeatRequest = 0,    // 心跳报文
  kHeartBeatReply = 1,

  kRpcRequest = 2,          // RPC报文
  kRpcReply = 3,

  kRegisterRequest = 4,     // 服务注册报文
  kRegisterReply = 5,

  kDiscoverRequest = 6,     // 服务发现报文
  kDiscoverReply = 7,

  kPublishRequest = 8,      // 发布
  kPublishReply = 9,

  kSubscribeRequest = 10,   // 订阅
  kSubscribeReply = 11,

  kUnSubscribeRequest = 12, // 取消订阅
  kUnSubscribeReply = 13,
};

struct RpcHeader {
  uint8_t version;
  RpcMessageType message_type;
  uint32_t id;
  uint32_t content_length;
};
SERIALIZE4(RpcHeader, version, message_type, id, content_length);

// kRpcRequest的content部分
struct RpcRequestBody {
  std::string method;
  BufferPtr args; // 相比使用std::string来说，可以减少一次复制
};
SERIALIZE2(RpcRequestBody, method, args)

enum class RpcErrorCode {
  kSuccess,
  kMethodNotFound,
};

// kRpcReply的content部分
struct RpcReplyBody {
  RpcErrorCode error_code;
  BufferPtr ret;
};
SERIALIZE2(RpcReplyBody, error_code, ret)

struct RpcRegisterRequestBody {
  std::string ip_port;
  std::vector<std::string> methods;
};
SERIALIZE2(RpcRegisterRequestBody, ip_port, methods)

struct RpcDiscoverRequestBody {
  std::string method;
};
SERIALIZE1(RpcDiscoverRequestBody, method)

struct RpcDiscoverReplyBody {
  std::unordered_set<std::string> ip_port_set;
};
SERIALIZE1(RpcDiscoverReplyBody, ip_port_set)

// NOTE: 因为采用Fixed编码，所以uint32_t字段的长度为4
constexpr int kRpcHeaderSize = 10;  // 1 + 1 + 4 + 4
constexpr uint8_t kRpcVersion = 1;
constexpr std::chrono::seconds kHeartBeatInterval(30);
constexpr std::chrono::seconds kHeartBeatTimeout(20);

enum class RpcParseStatus {
  kParseHeader,
  kParseBody,
};

constexpr char kHeartBeatSendTimerKey[] = "heart_beat_send_timer";
constexpr char kHeartBeatTimeoutTimerKey[] = "heart_beat_timeout_timer";
constexpr char kRpcHeaderIdKey[] = "rpc_header_id";
constexpr char kRpcParseStatusKey[] = "rpc_parse_status";
constexpr char kRpcMessageTypeKey[] = "rpc_message_type";
constexpr char kRpcContentLengthKey[] = "rpc_content_length";

template<typename ...Args>
BufferPtr SerializeArgsToBuffer(Args &&...args) {
  if constexpr(sizeof...(Args) == 0) {  // 针对无参数场景的优化
    return std::make_shared<Buffer>(0);
  } else {
    Serializer serializer;
    (void) (serializer << ... << args);
    return serializer.GetBuffer();
  }
}

template<typename T>
T GetReturnValueFromBuffer(const BufferPtr &buffer) {
  Serializer serializer(buffer);
  T val = Get<T>(serializer);
  return val;
}

}

#endif // NET_INCLUDE_NET_RPC_RPC_PROTOCOL_HPP_
