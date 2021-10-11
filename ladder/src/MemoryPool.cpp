#include <MemoryPool.h>

namespace ladder {

MemoryPoolImpl* MemoryPoolImpl::instance() {
  return MemoryPoolImpl::create();
}

MemoryPoolImpl* MemoryPoolImpl::create(
  size_t num_blocks, size_t block_size)
{
  static MemoryPoolImpl* instance = \
    new MemoryPoolImpl(num_blocks, block_size);
  return instance_ = instance;
}

void MemoryPoolImpl::release() {
  if(instance_) {
    delete instance_;
    instance_ = nullptr;
  }
}

MemoryPoolImpl::MemoryPoolImpl(size_t num_blocks,
                               size_t block_size) : 
  num_blocks_(num_blocks), block_size_(block_size)
{
  blocks_ = new MemoryBlock;
  MemoryBlock* block = blocks_;
  for(size_t i = 0; i < num_blocks; ++i) {
    block = block->next_ = new MemoryBlock;
    block->memory_ = new char[block_size_];
  }
}

MemoryPoolImpl::~MemoryPoolImpl() {
  MemoryBlock* block = blocks_->next_;
  MemoryBlock* prev = nullptr;
  blocks_->next_ = nullptr;
  while(block) {
    delete block->memory_;
    block->memory_ = nullptr;
    prev = block;
    block = block->next_;
    delete prev;
    prev = nullptr;
  }
}

char* MemoryPoolImpl::allocate() {
  {
    std::lock_guard<std::mutex> lock(mutex_blocks_);
    MemoryBlock* block = blocks_->next_;
    if(block == NULL) {
      return nullptr;
      // TODO: expand memory pool?
    }
    blocks_->next_ = block->next_;
    char* memory = block->memory_;
    delete block;
    return memory;
  }
}

char* MemoryPoolImpl::allocate(size_t n) {
  if(n > block_size_) {
    return nullptr;
  }
  return this->allocate();
}

void MemoryPoolImpl::free(void* memory) {
  MemoryBlock* block = new MemoryBlock;
  block->memory_ = reinterpret_cast<char*>(memory);
  block->next_ = blocks_->next_;
  {
    std::lock_guard<std::mutex> lock(mutex_blocks_);
    blocks_->next_ = block;
  }
}

MemoryPoolImpl* MemoryPoolImpl::instance_ = nullptr;

} // namespace ladder
