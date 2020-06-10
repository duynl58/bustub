//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// hash_table_header_page.cpp
//
// Identification: src/storage/page/hash_table_header_page.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "storage/page/hash_table_header_page.h"
#include "common/exception.h"
#include "common/logger.h"

namespace bustub {
page_id_t HashTableHeaderPage::GetBlockPageId(size_t index) { return this->block_page_ids_[index]; }

page_id_t HashTableHeaderPage::GetPageId() const { return this->page_id_; }

void HashTableHeaderPage::SetPageId(bustub::page_id_t page_id) { this->page_id_ = page_id; }

lsn_t HashTableHeaderPage::GetLSN() const { return this->lsn_; }

void HashTableHeaderPage::SetLSN(lsn_t lsn) { this->lsn_ = lsn; }

void HashTableHeaderPage::AddBlockPageId(page_id_t page_id) {
  if (next_ind_ > this->GetSize()) {
    throw new Exception("Reach limit of block_page_ids_");
  }
  this->block_page_ids_[next_ind_] = page_id;
  next_ind_++;
}

size_t HashTableHeaderPage::NumBlocks() { return this->next_ind_; }

void HashTableHeaderPage::SetSize(size_t size) { this->size_ = size; }

size_t HashTableHeaderPage::GetSize() const { return this->size_; }

}  // namespace bustub
