#ifndef NET_INCLUDE_NET_RPC_RPC_PUBLISHER_HPP_
#define NET_INCLUDE_NET_RPC_RPC_PUBLISHER_HPP_

#include "net/tcp/tcp_connection.hpp"
#include "net/rpc/rpc_message_builder.hpp"

#include <unordered_map>
#include <unordered_set>

namespace net {

class RpcPublisher {
 public:
  RpcPublisher() = default;

  void CreateTopic(std::string topic) {
    std::lock_guard<std::mutex> lg(mutex_);
    topic_to_subscribers_.emplace(std::move(topic), std::unordered_set<TcpConnectionPtr>{});
  }
  void DeleteTopic(const std::string &topic) {
    std::lock_guard<std::mutex> lg(mutex_);
    topic_to_subscribers_.erase(topic);
  }

  void Subscribe(const TcpConnectionPtr &conn, const std::string &topic) {
    {
      std::lock_guard<std::mutex> lg(mutex_);
      topic_to_subscribers_[topic].insert(conn);
    }
    auto reply = RpcMessageBuilder::SubscribeReply(0);
    conn->Send(reply);
  }
  void UnSubscribe(const TcpConnectionPtr &conn, const std::string &topic) {
    bool success;
    {
      std::lock_guard<std::mutex> lg(mutex_);
      if (!topic_to_subscribers_.count(topic)) {
        success = false;
      } else {
        topic_to_subscribers_[topic].erase(conn);
        success = true;
      }
    }
    auto reply = RpcMessageBuilder::UnSubscribeReply(0, success);
    conn->Send(reply);
  }

  template<typename T>
  void Publish(const std::string &topic, const T &data) {
    std::lock_guard<std::mutex> lg(mutex_);
    if (!topic_to_subscribers_.count(topic)) return;
    if (topic_to_subscribers_[topic].empty()) return;
    Serializer serializer;
    serializer << topic << data;
    BufferPtr rpc_content = serializer.GetBuffer();
    BufferPtr rpc_header = RpcMessageBuilder::PublishRequestHeader(0, rpc_content);
    for (auto &subscriber : topic_to_subscribers_[topic]) {
      if (!subscriber->Connected()) continue; // 忽略已断开的连接, 理论上应该不会出现这种情况
      subscriber->Send(rpc_header);
      subscriber->Send(rpc_content);
    }
  }

  void DeleteSubscriber(const TcpConnectionPtr &conn) {
    std::lock_guard<std::mutex> lg(mutex_);
    for (auto &[topic, subscribers] : topic_to_subscribers_) {
      subscribers.erase(conn);
    }
  }

 private:
  std::unordered_map<std::string, std::unordered_set<TcpConnectionPtr>> topic_to_subscribers_;
  std::mutex mutex_;
};

}

#endif // NET_INCLUDE_NET_RPC_RPC_PUBLISHER_HPP_
