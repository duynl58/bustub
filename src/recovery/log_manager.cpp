//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// log_manager.cpp
//
// Identification: src/recovery/log_manager.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "recovery/log_manager.h"

namespace bustub {
/*
 * set enable_logging = true
 * Start a separate thread to execute flush to disk operation periodically
 * The flush can be triggered when timeout or the log buffer is full or buffer
 * pool manager wants to force flush (it only happens when the flushed page has
 * a larger LSN than persistent LSN)
 *
 * This thread runs forever until system shutdown/StopFlushThread
 */
void LogManager::RunFlushThread() {
  bustub::enable_logging = true;
  this->flush_thread_ = new std::thread([this]() {
    while (true) {
      std::unique_lock<std::mutex> lock(this->latch_);
      this->cv_.wait_for(lock, log_timeout, [this] { return this->force_flush_flag_; });
      this->force_flush_flag_ = false;
      if (this->stop_flush_thread_) {
        this->stop_flush_thread_ = false;
        return;
      }
      // if the LogBuffer is full || force flush || timeout -> flush
      std::swap(this->log_buffer_, this->flush_buffer_);
      this->disk_manager_->WriteLog(flush_buffer_, strlen(flush_buffer_));
      this->offset_ = 0;
      this->SetPersistentLSN(this->GetNextLSN());
      lock.unlock();
    }
  });
}

/*
 * Stop and join the flush thread, set enable_logging = false
 */
void LogManager::StopFlushThread() {
  bustub::enable_logging = false;
  this->stop_flush_thread_ = true;
  this->flush_thread_->join();
}

/**
 * Force flush the current LogManager
 */
void LogManager::ForceFlush() {
  std::lock_guard<std::mutex> lock(this->latch_);
  this->force_flush_flag_ = true;
  this->cv_.notify_one();
}

/*
 * append a log record into log buffer
 * you MUST set the log record's lsn within this method
 * @return: lsn that is assigned to this log record
 *
 *
 * example below
 * // First, serialize the must have fields(20 bytes in total)
 * log_record.lsn_ = next_lsn_++;
 * memcpy(log_buffer_ + offset_, &log_record, 20);
 * int pos = offset_ + 20;
 *
 * if (log_record.log_record_type_ == LogRecordType::INSERT) {
 *    memcpy(log_buffer_ + pos, &log_record.insert_rid_, sizeof(RID));
 *    pos += sizeof(RID);
 *    // we have provided serialize function for tuple class
 *    log_record.insert_tuple_.SerializeTo(log_buffer_ + pos);
 *  }
 *
 */
lsn_t LogManager::AppendLogRecord(LogRecord *log_record) {
  if (log_record->GetLogRecordType() == LogRecordType::INVALID) return INVALID_LSN;
  std::unique_lock<std::mutex> lock(this->latch_);
  log_record->SetLSN(this->next_lsn_++);
  // serialize header of log_record
  memcpy(log_buffer_ + offset_, log_record, LogRecord::HEADER_SIZE);
  // serialize data of log_record
  this->offset_ += LogRecord::HEADER_SIZE;
  // serialize INSERT LogRecord
  if (log_record->GetLogRecordType() == LogRecordType::INSERT) {
    memcpy(log_buffer_ + this->offset_, &(log_record->GetInsertRID()), sizeof(RID));
    this->offset_ += sizeof(RID);
    log_record->GetInsertTuple().SerializeTo(log_buffer_ + this->offset_);
    this->offset_ += log_record->GetInsertTuple().GetLength();
  }
  // serialize DELETE LogRecord
  if (log_record->GetLogRecordType() == LogRecordType::APPLYDELETE ||
      log_record->GetLogRecordType() == LogRecordType::MARKDELETE ||
      log_record->GetLogRecordType() == LogRecordType::ROLLBACKDELETE) {
    memcpy(log_buffer_ + this->offset_, &(log_record->GetDeleteRID()), sizeof(RID));
    this->offset_ += sizeof(RID);
    log_record->GetDeleteTuple().SerializeTo(log_buffer_ + this->offset_);
    this->offset_ += log_record->GetDeleteTuple().GetLength();
  }
  // serialize UPDATE LogRecord
  if (log_record->GetLogRecordType() == LogRecordType::UPDATE) {
    memcpy(log_buffer_ + this->offset_, &(log_record->GetUpdateRID()), sizeof(RID));
    this->offset_ += sizeof(RID);
    log_record->GetOriginalTuple().SerializeTo(log_buffer_ + this->offset_);
    log_record->GetUpdateTuple().SerializeTo(log_buffer_ + this->offset_ + log_record->GetOriginalTuple().GetLength());
    this->offset_ += log_record->GetOriginalTuple().GetLength() + log_record->GetUpdateTuple().GetLength();
  }
  // serialize NEWPAGE LogRecord
  if (log_record->GetLogRecordType() == LogRecordType::NEWPAGE) {
    memcpy(log_buffer_ + this->offset_, &(log_record->GetNewPageRecord()), sizeof(page_id_t));
    this->offset_ += sizeof(page_id_t);
  }
  // otherwise, there is no need to serialize anymore (besides the header) -> return
  return log_record->GetLSN();
}

}  // namespace bustub
