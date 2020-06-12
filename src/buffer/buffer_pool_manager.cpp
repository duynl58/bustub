//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// buffer_pool_manager.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/buffer_pool_manager.h"
#include "common/logger.h"

// TODO(duynl58): remember to use RWLatch for all pages

namespace bustub {

BufferPoolManager::BufferPoolManager(size_t pool_size, DiskManager *disk_manager, LogManager *log_manager)
    : pool_size_(pool_size), disk_manager_(disk_manager), log_manager_(log_manager) {
  // We allocate a consecutive memory space for the buffer pool.
  pages_ = new Page[pool_size_];
  replacer_ = new ClockReplacer(pool_size);

  // Initially, every page is in the free list.
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.emplace_back(static_cast<int>(i));
  }
}

BufferPoolManager::~BufferPoolManager() {
  delete[] pages_;
  delete replacer_;
}

bool BufferPoolManager::allPinned() {
  for (size_t fid = 0; fid < this->pool_size_; fid++) {
    auto page = GetPages() + fid;
    if (page->pin_count_ <= 0) {
      return false;
      break;
    }
  }
  return true;
}

frame_id_t BufferPoolManager::victimPage() {
  frame_id_t frame_id;
  if (!free_list_.empty()) {
    frame_id = free_list_.front();
    free_list_.pop_front();
    return frame_id;
  }
  if (!replacer_->Victim(&frame_id)) {
    return -1;
  }
  auto page = GetPages() + frame_id;
  LOG_DEBUG("Page id %d, is dirty %d", page->page_id_, page->IsDirty());
  page_table_.erase(page->GetPageId());
  if (page->IsDirty()) {
    LOG_DEBUG("Page %d is dirty, writing", page->GetPageId());
    disk_manager_->WritePage(page->GetPageId(), page->GetData());
  }
  return frame_id;
}

Page *BufferPoolManager::FetchPageImpl(page_id_t page_id) {
  // 1.     Search the page table for the requested page (P).
  // 1.1    If P exists, pin it and return it immediately.
  // 1.2    If P does not exist, find a replacement page (R) from either the free list or the replacer.
  //        Note that pages are always found from the free list first.
  // 2.     If R is dirty, write it back to the disk.
  // 3.     Delete R from the page table and insert P.
  // 4.     Update P's metadata, read in the page content from disk, and then return a pointer to P.
  std::lock_guard<std::mutex> lock(this->latch_);
  if (this->allPinned()) return nullptr;
  auto iterator = page_table_.find(page_id);
  // step 1.1.
  if (iterator != page_table_.end()) {
    // page_id found
    replacer_->Pin(iterator->second);
    return GetPages() + iterator->second;
  }
  // step 2. (includes step 1.2.)
  auto frame_id = this->victimPage();
  if (frame_id < 0) return nullptr;
  auto page = GetPages() + frame_id;
  // step 3.
  page_table_.insert({page_id, frame_id});
  // step 4.
  page->page_id_ = page_id;
  page->ResetMemory();
  page->pin_count_ = 1;
  page->is_dirty_ = false;
  disk_manager_->ReadPage(page_id, page->GetData());
  return page;
}

bool BufferPoolManager::UnpinPageImpl(page_id_t page_id, bool is_dirty) {
  std::lock_guard<std::mutex> lock(this->latch_);
  auto iterator = page_table_.find(page_id);
  if (iterator == page_table_.end()) {
    return false;
  }
  auto page = GetPages() + iterator->second;
  page->pin_count_--;
  if (page->pin_count_ <= 0) {
    replacer_->Unpin(iterator->second);
  }
  page->is_dirty_ |= is_dirty;
  return true;
}

bool BufferPoolManager::FlushPageImpl(page_id_t page_id) {
  // Make sure you call DiskManager::WritePage!
  std::lock_guard<std::mutex> lock(this->latch_);
  auto iterator = page_table_.find(page_id);
  if (iterator == page_table_.end()) {
    return false;
  }
  auto page = GetPages() + iterator->second;
  disk_manager_->WritePage(page_id, page->GetData());
  page->is_dirty_ = false;
  return true;
}

Page *BufferPoolManager::NewPageImpl(page_id_t *page_id) {
  // 0.   Make sure you call DiskManager::AllocatePage!
  // 1.   If all the pages in the buffer pool are pinned, return nullptr.
  // 2.   Pick a victim page P from either the free list or the replacer. Always pick from the free list first.
  // 3.   Update P's metadata, zero out memory and add P to the page table.
  // 4.   Set the page ID output parameter. Return a pointer to P.
  std::lock_guard<std::mutex> lock(this->latch_);
  // step 1.
  if (this->allPinned()) return nullptr;
  // step 2.
  auto frame_id = this->victimPage();
  if (frame_id < 0) return nullptr;
  auto page = GetPages() + frame_id;
  LOG_DEBUG("Frame to be victimized %d", frame_id);
  // step 3.
  page->page_id_ = disk_manager_->AllocatePage();
  page->ResetMemory();
  page->pin_count_ = 1;
  page->is_dirty_ = false;
  // step 4.
  page_table_.insert({page->page_id_, frame_id});
  *page_id = page->page_id_;
  return page;
}

bool BufferPoolManager::DeletePageImpl(page_id_t page_id) {
  // 0.   Make sure you call DiskManager::DeallocatePage!
  // 1.   Search the page table for the requested page (P).
  // 1.   If P does not exist, return true.
  // 2.   If P exists, but has a non-zero pin-count, return false. Someone is using the page.
  // 3.   Otherwise, P can be deleted. Remove P from the page table, reset its metadata and return it to the free list.
  return false;
}

void BufferPoolManager::FlushAllPagesImpl() {
  // You can do it!
}

}  // namespace bustub
