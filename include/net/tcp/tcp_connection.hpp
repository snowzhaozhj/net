#ifndef NET_INCLUDE_NET_TCP_TCP_CONNECTION_HPP_
#define NET_INCLUDE_NET_TCP_TCP_CONNECTION_HPP_

#include "net/reactor/reactor.hpp"
#include "net/util/context.hpp"

namespace net {

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection> {
 public:
  using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
  using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
  using MessageCallback = std::function<void(const TcpConnectionPtr &, const BufferPtr &)>;
  using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
  using CloseCallback = std::function<void(const TcpConnectionPtr &)>;

  /// 为了能够使用ObjectPool，将初始化逻辑放到了Init函数中
  TcpConnection()
      : reactor_(nullptr),
        channel_(-1),
        local_addr_(0),
        peer_addr_(0),
        input_buffer_(std::make_shared<Buffer>()),
        output_buffer_(std::make_shared<Buffer>()),
        state_(State::Connecting) {}

  /// 每次获取一个TcpConnection对象后，使用Init函数进行初始化
  void Init(Reactor *reactor, int conn_fd) {
    InetAddress local_addr(net::GetLocalAddr(conn_fd));
    InetAddress peer_addr(net::GetPeerAddr(conn_fd));
    Init(reactor, conn_fd, local_addr, peer_addr);
  }

  /// 每次获取一个TcpConnection对象后，使用Init函数进行初始化
  void Init(Reactor *reactor, int conn_fd, const InetAddress &local_addr, const InetAddress &peer_addr) {
    reactor_ = reactor;
    channel_ = Channel(conn_fd);
    local_addr_ = local_addr;
    peer_addr_ = peer_addr;
    input_buffer_->Reset();
    output_buffer_->Reset();
    state_.store(State::Connecting, std::memory_order_relaxed);
    context_.Clear();

    channel_.SetReadCallback([this] { HandleRead(); });
    channel_.SetWriteCallback([this] { HandleWrite(); });
    channel_.SetCloseCallback([this] { HandleClose(); });
    channel_.SetErrorCallback([this] { HandleError(); });
  }

  void SetConnectionCallback(ConnectionCallback &&cb) {
    connection_callback_ = std::move(cb);
  }
  void SetConnectionCallback(const ConnectionCallback &cb) {
    connection_callback_ = cb;
  }
  void SetMessageCallback(MessageCallback &&cb) {
    message_callback_ = std::move(cb);
  }
  void SetMessageCallback(const MessageCallback &cb) {
    message_callback_ = cb;
  }
  void SetWriteCompleteCallback(WriteCompleteCallback &&cb) {
    write_complete_callback_ = std::move(cb);
  }
  void SetWriteCompleteCallback(const WriteCompleteCallback &cb) {
    write_complete_callback_ = cb;
  }
  void SetCloseCallback(CloseCallback &&cb) {
    close_callback_ = std::move(cb);
  }

  // 可用于在ConnectionCallback中判断是Establish时调用的，还是Destroy时调用的
  bool Connected() {
    return state_.load(std::memory_order_acquire) == State::Connected;
  }

  /// 建立连接，将channel注册到Reactor中，并且调用ConnectionCallback
  /// @note 请确保在Establish之前已经Init
  /// @note 将Establish任务提交到某个Reactor之后，TcpConnection的后续操作默认会在该Reactor绑定的线程上执行
  void Establish() {
    state_.store(State::Connected, std::memory_order_relaxed);
    channel_.EnableRead();
    reactor_->UpdateChannel(&channel_);
    if (connection_callback_) {
      connection_callback_(shared_from_this());
    }
  }
  /// 销毁连接，将channel从Reactor中移除，并且调用ConnectionCallback
  void Destroy() {
    state_.store(State::Disconnected, std::memory_order_release);
    channel_.DisableAll();
    if (connection_callback_) {
      connection_callback_(shared_from_this());
    }
    reactor_->RemoveChannel(&channel_);
    net::Close(channel_.GetFd());
  }

  /// 向对端发送数据
  /// @note 线程安全
  void Send(const BufferPtr &buffer) {
    if (state_.load(std::memory_order_acquire) != State::Connected) return;
    if (reactor_->InCurrentReactorThread()) {
      RealSend(buffer->GetReadPtr(), buffer->ReadableBytes());
    } else {
      reactor_->SubmitTask([buffer, this]() {
        RealSend(buffer->GetReadPtr(), buffer->ReadableBytes());
      });
    }
  }
  /// 向对端发送数据
  /// @note 线程安全
  void Send(std::string_view content) {
    Send(content.data(), content.size());
  }
  /// 向对端发送数据
  /// @note 线程安全
  void Send(const char *data, size_t len) {
    if (state_.load(std::memory_order_acquire) != State::Connected) return;
    if (reactor_->InCurrentReactorThread()) {
      RealSend(data, len);
    } else {
      // 从其他线程发送数据，将会添加一个发送任务到当前Reactor的任务队列中
      // 这种情况下，在实际发送时，可能data指针指向的内容已经被释放, 因此需要复制一份
      reactor_->SubmitTask([this, str = std::string(data, len)] {
        RealSend(str.data(), str.size());
      });
    }
  }

  /// 主动关闭连接
  /// @note 线程安全
  void Shutdown() {
    if (state_.load(std::memory_order_acquire) != State::Connected) return;
    state_.store(State::Disconnecting, std::memory_order_release);
    if (reactor_->InCurrentReactorThread()) {
      RealShutdown();
    } else {
      reactor_->SubmitTask([this] { RealShutdown(); });
    }
  }

  Reactor *GetReactor() const { return reactor_; }
  Context &GetContext() { return context_; }
  InetAddress &GetLocalAddr() { return local_addr_; }
  InetAddress &GetPeerAddr() { return peer_addr_; }

 private:
  void HandleRead() {
    ssize_t n = net::Read(channel_.GetFd(), input_buffer_);
    if (n > 0) {
      if (message_callback_) {
        message_callback_(shared_from_this(), input_buffer_);
      }
    } else if (n == 0) {
      HandleClose();
    } else {
      HandleError();
    }
  }
  void HandleWrite() {
    if (!channel_.WriteEnabled()) return;
    size_t n = net::Write(channel_.GetFd(), output_buffer_);
    if (n > 0) {
      output_buffer_->HasWritten(n);
      if (output_buffer_->ReadableBytes() == 0) {
        channel_.DisableWrite();
        reactor_->UpdateChannel(&channel_);
        if (write_complete_callback_) {
          reactor_->SubmitTask([this, self = shared_from_this()] {
            write_complete_callback_(self);
          });
        }
        if (state_.load(std::memory_order_relaxed) == State::Disconnecting) {
          RealShutdown();
        }
      }
    }
  }
  void HandleClose() {
    LOG_INFO("HandleClose");
    channel_.DisableAll();
    reactor_->RemoveChannel(&channel_);
    close_callback_(shared_from_this());
  }
  void HandleError() {
    // 什么都不做
  }

  void RealSend(const char *data, size_t len) {
    // 如果输出缓冲区中没有数据，则直接写入
    ssize_t nwrote = 0;
    if (!channel_.WriteEnabled() && output_buffer_->ReadableBytes() == 0) {
      nwrote = net::Write(channel_.GetFd(), data, len);
      if (nwrote >= 0) {
        if (nwrote == len && write_complete_callback_) {
          reactor_->SubmitTask([this, self = shared_from_this()] {
            write_complete_callback_(self);
          });
        }
      } else {
        nwrote = 0;
      }
    }
    // 如果没有进行直接写入，或者直接写入没有写完，则添加到缓冲区中
    if (nwrote < len) {
      output_buffer_->Append(data + nwrote, len - nwrote);
      if (!channel_.WriteEnabled()) {
        channel_.EnableWrite();
        reactor_->UpdateChannel(&channel_);
      }
    }
  }

  void RealShutdown() {
    if (!channel_.WriteEnabled()) {
      net::ShutDown(channel_.GetFd(), SHUT_WR);
    }
  }

  enum class State {
    Connecting,
    Connected,
    Disconnecting,
    Disconnected,
  };

  Reactor *reactor_;
  Channel channel_;
  InetAddress local_addr_;
  InetAddress peer_addr_;
  BufferPtr input_buffer_;
  BufferPtr output_buffer_;

  ConnectionCallback connection_callback_;
  MessageCallback message_callback_;
  WriteCompleteCallback write_complete_callback_;
  CloseCallback close_callback_;

  std::atomic<State> state_;

  Context context_;
};

using TcpConnectionPtr = TcpConnection::TcpConnectionPtr;

} // namespace net

#endif //NET_INCLUDE_NET_TCP_TCP_CONNECTION_HPP_
