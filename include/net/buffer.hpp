#ifndef NET_INCLUDE_NET_BUFFER_HPP_
#define NET_INCLUDE_NET_BUFFER_HPP_

#include "net/log.hpp"
#include "net/noncopyable.hpp"

#include <cstddef>
#include <vector>

namespace net {

class Buffer : public noncopyable {
 public:
  static constexpr size_t kInitSize = 1024;

  explicit Buffer(size_t sz = kInitSize)
      : buffer_(sz),
        write_pos_(0),
        read_pos_(0) {}
  Buffer(const char *ptr, size_t len)
      : buffer_(ptr, ptr + len),
        write_pos_(len),
        read_pos_(0) {}
  explicit Buffer(std::string_view s) : Buffer(s.data(), s.size()) {}

  Buffer(Buffer &&other)
      : buffer_(std::move(other.buffer_)),
        write_pos_(other.write_pos_),
        read_pos_(other.read_pos_) {
    other.Reset();
  }
  Buffer &operator=(Buffer &&other) {
    buffer_ = std::move(other.buffer_);
    write_pos_ = other.write_pos_;
    read_pos_ = other.read_pos_;
    other.Reset();
    return *this;
  }

  [[nodiscard]] size_t ReadableBytes() const { return write_pos_ - read_pos_; };
  [[nodiscard]] size_t WritableBytes() const { return buffer_.size() - write_pos_; }
  [[nodiscard]] const char *GetReadPtr() const { return buffer_.data() + read_pos_; }
  [[nodiscard]] const char *GetWritePtr() const { return buffer_.data() + write_pos_; }
  [[nodiscard]] char *GetWritePtr() { return buffer_.data() + write_pos_; }

  void EnsureWritableBytes(size_t n) {
    if (WritableBytes() < n) {
      Resize(write_pos_ + n);
    }
  }

  void HasWritten(size_t n) { write_pos_ += n; }
  void HasRead(size_t n) { read_pos_ += n; }

  void Resize(size_t n) { buffer_.resize(n); }
  void Reset() {
    write_pos_ = 0;
    read_pos_ = 0;
  }

  /* 最好调用以下方法来操作Buffer */
  [[nodiscard]] const char *Find(char ch) const {
    // 使用std::find而不是memchr
    const char *pos = std::find(GetReadPtr(), GetWritePtr(), ch);
    return pos == GetWritePtr() ? nullptr : pos;
  }
  [[nodiscard]] const char *Search(const char *ptr, size_t len) const {
    const char *pos = std::search(GetReadPtr(), GetWritePtr(),
                                  ptr, ptr + len);
    return pos == GetWritePtr() ? nullptr : pos;
  }
  [[nodiscard]] const char *Search(std::string_view str) const {
    return Search(str.data(), str.size());
  }

  void Append(const char *ptr, size_t len) {
    EnsureWritableBytes(len);
    std::copy(ptr, ptr + len, GetWritePtr());
    HasWritten(len);
  }
  void Append(std::string_view s) {
    Append(s.data(), s.size());
  }

  std::string Retrive(size_t len) {
    NET_ASSERT(len <= ReadableBytes());
    std::string s(GetReadPtr(), len);
    HasRead(len);
    return s;
  }
  std::string RetriveTo(const char *ptr) {
    NET_ASSERT(ptr <= GetWritePtr());
    return Retrive(ptr - GetReadPtr());
  }
  /// @note 转换成字符串后，将会调用Reset函数重置Buffer
  std::string RetriveAll() {
    std::string s = RetriveTo(GetWritePtr());
    Reset();
    return s;
  }

 private:
  std::vector<char> buffer_;
  size_t write_pos_;
  size_t read_pos_;
};

using BufferPtr = std::shared_ptr<Buffer>;

}

#endif //NET_INCLUDE_NET_BUFFER_HPP_
