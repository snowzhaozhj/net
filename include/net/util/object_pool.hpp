#ifndef NET_INCLUDE_NET_UTIL_OBJECT_POOL_HPP_
#define NET_INCLUDE_NET_UTIL_OBJECT_POOL_HPP_

#include <vector>
#include <memory>
#include <functional>

namespace net {

/// 非线程安全
template<typename T>
class ObjectPool {
  static constexpr int kInitPoolSize = 100;
  static constexpr int kOnceExtendSize = 10;
 public:
  explicit ObjectPool(int size = kInitPoolSize) {
    object_vec_.reserve(size);
    for (int i = 0; i < size; ++i) {
      object_vec_.push_back(new T);
    }
  }
  ~ObjectPool() {
    for (auto object: object_vec_) {
      delete object;
    }
  }

  /// 从线程池中获取一个对象，在返回的shared_ptr生命周期结束后，会自动将对象放回到对象池中
  std::shared_ptr<T> GetShared() {
    return std::shared_ptr<T>(Get(), [this](T *object) {
      Add(object);
    });
  }

  std::unique_ptr<T, std::function<void(T *)>> GetUnique() {
    return std::unique_ptr<T, std::function<void(T *)>>(Get(), [this](T *object) {
      Add(object);
    });
  }

  /// 从对象池中获取一个对象，返回原始指针
  /// @note 不建议直接使用该接口
  T *Get() {
    if (object_vec_.empty()) {  // 容量不够了，扩充对象池
      for (int i = 0; i < kOnceExtendSize; ++i) {
        object_vec_.push_back(new T);
      }
    }
    T *object = object_vec_.back();
    object_vec_.pop_back();
    return object;
  }

  /// 将一个对象放到对象池中，对象池将会接管该对象的生命周期(即析构的时候delete该对象)
  /// 最好只将Get()的返回值使用Add添加到对象池中
  /// @note 不建议使用该接口
  void Add(T *object) {
    object_vec_.push_back(object);
  }

 private:
  std::vector<T *> object_vec_;
};

namespace object_pool {

template<typename T>
inline ObjectPool<T> *GetTLSObjectPool() {
  thread_local static ObjectPool<T> object_pool_tls;
  return &object_pool_tls;
}

template<typename T>
inline std::shared_ptr<T> NewShared() {
  return GetTLSObjectPool<T>()->GetShared();
}

template<typename T>
inline std::unique_ptr<T, std::function<void(T *)>> NewUnique() {
  return GetTLSObjectPool<T>()->GetUnique();
}

template<typename T>
inline T *New() {
  return GetTLSObjectPool<T>()->Get();
}

template<typename T>
inline void Delete(T *object) {
  GetTLSObjectPool<T>()->Add(object);
}

} // namespace net::object_pool

} // namespace net

#endif //NET_INCLUDE_NET_UTIL_OBJECT_POOL_HPP_
