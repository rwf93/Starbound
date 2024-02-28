#ifndef STAR_PROMISE_HPP
#define STAR_PROMISE_HPP

#include "StarAtomicSharedPtr.hpp"

namespace Star {

STAR_EXCEPTION(PromiseException, StarException);

// Wraps a ThreadInvoker, and can hold the value for consumption by more than
// one thread, and is sharable.  The return value is not taken on get(), and
// access to the value via get() by multiple threads is safe.
template<typename Result>
class Promise {
public:
  Promise();
  Promise(ThreadInvoker<Result> invoker);
  Promise(Result result);

  bool valid() const;
  operator bool() const;

  bool fulfilled() const;

  Maybe<Result const&> maybe() const;
  Maybe<Result&> maybe();

  Result const& get() const;
  Result& get();

private:
  void checkValid();

  struct Data {
    mutable Mutex mutex;
    Maybe<ThreadInvoker<Result>> invoker;
    Maybe<Result> result;
  };

  AtomicSharedPtr<Data> m_data;
};

template<typename Result>
Promise<Result>::Promise() {}

template<typename Result>
Promise<Result>::Promise(ThreadInvoker<Result> invoker) {
  m_data = make_shared<Data>(Data{{}, std::move(invoker), {}});
}

template<typename Result>
Promise<Result>::Promise(Result result) {
  m_data = make_shared<Data>(Data{{}, {}, std::move(result)});
}

template<typename Result>
bool Promise<Result>::valid() const {
  return m_data.valid();
}

template<typename Result>
Promise<Result>::operator bool() const {
  return (bool)m_data;
}

template<typename Result>
bool Promise<Result>::fulfilled() const {
  if (!m_data)
    return false;
  MutexLocker locker(m_data->mutex);
  return m_data->result.isValid();
}

template<typename Result>
Maybe<Result const&> Promise<Result>::maybe() const {
  if (!m_data)
    return {};
  MutexLocker locker(m_data->mutex);
  return *m_data->result;
}

template<typename Result>
Maybe<Result&> Promise<Result>::maybe() {
  if (!m_data)
    return {};
  MutexLocker locker(m_data->mutex);
  return *m_data->result;
}

template<typename Result>
Result const& Promise<Result>::get() const {
  checkValid();
  MutexLocker locker(m_data->mutex);
  if (m_data->invoker) {
    m_data->result = m_data->invoker->finish();
    m_data->invoker.reset();
  }
  return *m_data->result;
}

template<typename Result>
Result& Promise<Result>::get() {
  checkValid();
  MutexLocker locker(m_data->mutex);
  if (m_data->invoker) {
    m_data->result = m_data->invoker->finish();
    m_data->invoker.reset();
  }
  return *m_data->result;
}

template<typename Result>
void Promise<Result>::checkValid() {
  if (!m_data)
    throw PromiseException("Unset Promise was accessed");
}

}

#endif
