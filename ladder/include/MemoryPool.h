#ifndef LADDER_MEMORY_POOL_H
#define LADDER_MEMORY_POOL_H

#include <stdlib.h>

#include <mutex>

namespace ladder {

struct MemoryBlock {
  char* memory_;
  MemoryBlock* next_;
  MemoryBlock() : memory_(NULL), next_(NULL) {};
};

class MemoryPoolImpl {

public:
  static MemoryPoolImpl* instance();
  static void release();
  static MemoryPoolImpl* create(size_t num_blocks = 256,
                                size_t block_size = 1024);

  char* allocate(size_t n);
  void free(void* memory);

private:
  MemoryPoolImpl(size_t num_blocks,
                 size_t block_size);
  ~MemoryPoolImpl();
    
  char* allocate();

  static MemoryPoolImpl* instance_;
  MemoryBlock* blocks_; // memory block list header
  std::mutex mutex_blocks_;
  size_t num_blocks_;
  size_t block_size_;
};

template <typename T>
class MemoryPool {

public:
  template<typename... Args>
  static T* allocate(Args&& ...args) {
    auto pool = MemoryPoolImpl::instance();
    if(pool == nullptr) {
      return nullptr;
    }
    
    char* memory = pool->allocate(sizeof(T));
    if(memory == nullptr) {
      return nullptr;
    }
    
    return new (memory) T(std::forward<Args>(args)...);
  }

  static T* allocate_n(size_t n) {
    auto pool = MemoryPoolImpl::instance();
    if(pool == nullptr) {
      return nullptr;
    }
    
    char* memory = pool->allocate(n * sizeof(T));
    if(memory == nullptr) {
      return nullptr;
    }
    
    return new (memory) T[n];
  }

  static void free(T* memory, bool is_array=false) {
    auto instance = MemoryPoolImpl::instance();
    if(instance) {
      instance->free(memory);
    }
    else if(!is_array) {
      delete memory;
    }
    else {
      delete []memory;
    }
  }

};

template <typename T>
class MemoryWrapper {

public:
  template<typename... Args>
  MemoryWrapper(Args&& ...args) : 
    ptr_(MemoryPool<T>::allocate(std::forward<Args>(args)...)),
    arr_(false),
    success_(true)
  {
    if(ptr_ == nullptr) {
      success_ = false;
      ptr_ = new T(std::forward<Args>(args)...);
    }
  }

  MemoryWrapper(size_t n) : 
    ptr_(MemoryPool<T>::allocate_n(n)),
    arr_(true),
    success_(true)
  {
    if(ptr_ == nullptr) {
      success_ = false;
      ptr_ = new T[n];
    }
  }

  ~MemoryWrapper() {
    if(success_) MemoryPool<T>::free(ptr_, arr_);
    else if(arr_) {
      delete [] ptr_;
    }
    else {
      delete ptr_;
    }
    ptr_ = nullptr;
  }

  T* get() {return ptr_;}

private:
  T* ptr_;
  bool arr_;
  bool success_;

};

} // namespace ladder

#endif
