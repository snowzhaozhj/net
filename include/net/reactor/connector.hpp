#ifndef NET_INCLUDE_NET_CLIENT_CONNECTOR_HPP_
#define NET_INCLUDE_NET_CLIENT_CONNECTOR_HPP_

#include "net/reactor/reactor.hpp"

namespace net {

/// 连接某个InetAddress
class Connector : noncopyable {
  static constexpr Duration kInitRetryDelay = std::chrono::milliseconds(500);
  static constexpr Duration kMaxRetryDelay = std::chrono::seconds(30);
 public:
  using NewConnectionCallback = std::function<void(int fd)>;

  Connector(Reactor *reactor, const InetAddress &server_addr)
      : reactor_(reactor),
        server_addr_(server_addr),
        retry_delay_(kInitRetryDelay),
        timer_id_(-1),
        state_(State::Disconnected),
        stopped_(false) {
  }
  ~Connector() {
    if (!stopped_) {
      Stop();
    }
  }

  [[nodiscard]] const InetAddress &GetServerAddr() const { return server_addr_; }

  void SetNewConnectionCallback(NewConnectionCallback &&cb) {
    new_connection_callback_ = std::move(cb);
  }

  /// 向Reactor中添加一个启动任务
  /// @note 线程安全(可从另一个线程调用该函数)
  void Start() {
    stopped_ = false;
    reactor_->SubmitTask([this] { RealStart(); });
  }

  /// 暂停连接，会尝试将向Reactor中添加的Retry Timer取消
  /// @note 非线程安全
  /// @note 重复调用该函数是安全的
  void Stop() {
    stopped_ = true;
    if (timer_id_ > 0) {  // 是有效的TimerId
      reactor_->CancleTimer(timer_id_);
      timer_id_ = -1;
    }
  }

  /// 重新连接
  /// @note 非线程安全
  void Restart() {
    state_ = State::Disconnected;
    stopped_ = false;
    RealStart();
  }

 private:
  /// 开始进行连接
  /// @note 非线程安全
  void RealStart() {
    if (stopped_) return;
    NET_ASSERT(state_ == State::Disconnected);
    int fd = net::NewNonBlockTcpSocketFd();
    int ret = net::Connect(fd, server_addr_);
    int saved_errno = ret == 0 ? 0 : errno;
    switch (saved_errno) {
      case 0:
      case EINPROGRESS:
      case EINTR:
      case EISCONN:
        Connecting(fd);
        break;

      case EAGAIN:
      case EADDRINUSE:
      case EADDRNOTAVAIL:
      case ECONNREFUSED:
      case ENETUNREACH:
        Retry(fd);
        break;

      case EACCES:
      case EPERM:
      case EAFNOSUPPORT:
      case EALREADY:
      case EBADF:
      case EFAULT:
      case ENOTSOCK:
      default:
        LOG_ERROR("Unexpected Error in Connect");
        net::Close(fd);
        break;
    }
  }

  /// 进行连接，将fd注册到Reactor中去
  /// @note 非线程安全
  void Connecting(int fd) {
    state_ = State::Connecting;
    channel_ = std::make_unique<Channel>(fd);
    channel_->SetWriteCallback([this] { HandleWrite(); });
    channel_->SetErrorCallback([this] { HandleError(); });
    channel_->EnableWrite();
    reactor_->UpdateChannel(channel_.get());
  }

  /// 尝试重新连接
  /// @note 非线程安全
  void Retry(int fd) {
    net::Close(fd);
    state_ = State::Disconnected;
    if (!stopped_) {
      reactor_->AddTimerAfter(retry_delay_, [this] { RealStart(); });
      retry_delay_ = std::min(retry_delay_ * 2, kMaxRetryDelay);
    }
  }

  void HandleWrite() {
    NET_ASSERT(state_ == State::Connecting);
    int fd = ResetAndRemoveChannel();
    int err = net::GetSocketError(fd);
    if (err) {
      LOG_ERROR("GetSocketError {}", strerror(err));
      Retry(fd);
    } else if (net::IsSelfConnect(fd)) {
      LOG_ERROR("IsSelfConnect");
      Retry(fd);
    } else {
      state_ = State::Connected;
      new_connection_callback_(fd);
    }
  }
  void HandleError() {
    LOG_ERROR("Connector GetError");
    NET_ASSERT(state_ == State::Connecting);
    int fd = ResetAndRemoveChannel();
    Retry(fd);
  }

  int ResetAndRemoveChannel() {
    int fd = channel_->GetFd();
    channel_->DisableAll();
    reactor_->RemoveChannel(channel_.get());
    channel_.reset();
    return fd;
  }

  enum class State {
    Disconnected,
    Connecting,
    Connected,
  };

  Reactor *reactor_;
  InetAddress server_addr_;
  NewConnectionCallback new_connection_callback_;
  Duration retry_delay_;
  TimerQueue::TimerId timer_id_;
  State state_;
  std::unique_ptr<Channel> channel_;
  bool stopped_;
};

} // namespace net

#endif //NET_INCLUDE_NET_CLIENT_CONNECTOR_HPP_
