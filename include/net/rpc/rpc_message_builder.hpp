#ifndef NET_INCLUDE_NET_RPC_RPC_MESSAGE_BUILDER_HPP_
#define NET_INCLUDE_NET_RPC_RPC_MESSAGE_BUILDER_HPP_

#include "net/rpc/rpc_protocol.hpp"

namespace net {

class RpcMessageBuilder {
 public:
  static BufferPtr HeartBeatRequest(uint32_t id) {
    auto header = NewRpcHeader(RpcMessageType::kHeartBeatRequest, id);
    return RpcHeaderToBuffer(header);
  }
  static BufferPtr HeartBeatReply(uint32_t id) {
    auto header = NewRpcHeader(RpcMessageType::kHeartBeatReply, id);
    return RpcHeaderToBuffer(header);
  }
  static BufferPtr RpcRequestHeader(uint32_t id, const BufferPtr &content) {
    auto header = NewRpcHeader(RpcMessageType::kRpcRequest, id, content->ReadableBytes());
    return RpcHeaderToBuffer(header);
  }
  static BufferPtr RpcReplyHeader(uint32_t id, const BufferPtr &content) {
    auto header = NewRpcHeader(RpcMessageType::kRpcReply, id, content->ReadableBytes());
    return RpcHeaderToBuffer(header);
  }
  static BufferPtr RegisterRequestHeader(uint32_t id, const BufferPtr &content) {
    auto header = NewRpcHeader(RpcMessageType::kRegisterRequest, id, content->ReadableBytes());
    return RpcHeaderToBuffer(header);
  }
  static BufferPtr RegisterReply(uint32_t id) {
    auto header = NewRpcHeader(RpcMessageType::kRegisterReply, id);
    return RpcHeaderToBuffer(header);
  }
  static BufferPtr DiscoverRequestHeader(uint32_t id, const BufferPtr &content) {
    auto header = NewRpcHeader(RpcMessageType::kDiscoverRequest, id, content->ReadableBytes());
    return RpcHeaderToBuffer(header);
  }
  static BufferPtr DiscoverReplyHeader(uint32_t id, const BufferPtr &content) {
    auto header = NewRpcHeader(RpcMessageType::kDiscoverReply, id, content->ReadableBytes());
    return RpcHeaderToBuffer(header);
  }
  static BufferPtr PublishRequestHeader(uint32_t id, const BufferPtr &content) {
    auto header = NewRpcHeader(RpcMessageType::kPublishRequest, id, content->ReadableBytes());
    return RpcHeaderToBuffer(header);
  }
  static BufferPtr SubscribeReply(uint32_t id) {
    auto header = NewRpcHeader(RpcMessageType::kSubscribeReply, id);
    return RpcHeaderToBuffer(header);
  }
  static BufferPtr UnSubscribeReply(uint32_t id, bool success) {
    auto header = NewRpcHeader(RpcMessageType::kUnSubscribeReply, id, 1);
    Serializer s;
    s << header;
    NET_ASSERT(s.ReadableBytes() == kRpcHeaderSize);
    s << success;
    NET_ASSERT(s.ReadableBytes() == kRpcHeaderSize + 1);
    return s.GetBuffer();
  }

 private:
  static RpcHeader NewRpcHeader(RpcMessageType type, uint32_t id, uint32_t len = 0) {
    return RpcHeader{
        .version = kRpcVersion,
        .message_type = type,
        .id = id,
        .content_length = len,
    };
  }
  static BufferPtr RpcHeaderToBuffer(const RpcHeader &header) {
    Serializer s;
    s << header;
    NET_ASSERT(s.ReadableBytes() == kRpcHeaderSize);
    return s.GetBuffer();
  }
};

}

#endif // NET_INCLUDE_NET_RPC_RPC_MESSAGE_BUILDER_HPP_
