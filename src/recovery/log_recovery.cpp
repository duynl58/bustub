//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// log_recovery.cpp
//
// Identification: src/recovery/log_recovery.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "recovery/log_recovery.h"

#include "common/logger.h"
#include "storage/page/table_page.h"

namespace bustub {
/*
 * deserialize a log record from log buffer
 * @return: true means deserialize succeed, otherwise can't deserialize cause
 * incomplete log record
 */
bool LogRecovery::DeserializeLogRecord(const char *data, LogRecord *log_record) {
  try {
    // deserialize log_record HEADER from data
    auto size = std::atoi(data);
    auto lsn = std::atoi(data + 4);
    auto txn_id = std::atoi(data + 8);
    auto prev_lsn = std::atoi(data + 12);
    auto log_type = LogRecordType(std::atoi(data + 16));
    // deserialize INSERT/DELETE LogRecord
    if (log_type == LogRecordType::INSERT || log_record->GetLogRecordType() == LogRecordType::APPLYDELETE ||
        log_record->GetLogRecordType() == LogRecordType::MARKDELETE ||
        log_record->GetLogRecordType() == LogRecordType::ROLLBACKDELETE) {
      auto rid = RID(std::atol(data + LogRecord::HEADER_SIZE));
      auto tuple = Tuple(rid);
      tuple.DeserializeFrom(data + LogRecord::HEADER_SIZE + sizeof(RID));
      *log_record = LogRecord(txn_id, prev_lsn, log_type, rid, tuple);
    }
    // deserialize UPDATE LogRecord
    if (log_record->GetLogRecordType() == LogRecordType::UPDATE) {
      auto rid = RID(std::atol(data + LogRecord::HEADER_SIZE));
      auto old_tuple = Tuple(rid);
      old_tuple.DeserializeFrom(data + LogRecord::HEADER_SIZE + sizeof(RID));
      auto new_tuple = Tuple(rid);
      new_tuple.DeserializeFrom(data + LogRecord::HEADER_SIZE + sizeof(RID) + sizeof(int32_t) + old_tuple.GetLength());
      *log_record = LogRecord(txn_id, prev_lsn, log_type, rid, old_tuple, new_tuple);
    }
    // deserialize NEWPAGE LogRecord
    if (log_record->GetLogRecordType() == LogRecordType::NEWPAGE) {
      auto prev_page_id = std::atoi(data + LogRecord::HEADER_SIZE);
      auto new_page_id = this->disk_manager_->AllocatePage();
      *log_record = LogRecord(txn_id, prev_lsn, log_type, prev_page_id, new_page_id);
    }
    // deserialize successfuly
    log_record->SetLSN(lsn);
    assert(size == log_record->GetSize());
    return true;
  } catch (std::exception &exp) {
    LOG_INFO("Can't deserialize log_record, meet: %s", exp.what());
  }
  return false;
}

/*
 *redo phase on TABLE PAGE level(table/table_page.h)
 *read log file from the beginning to end (you must prefetch log records into
 *log buffer to reduce unnecessary I/O operations), remember to compare page's
 *LSN with log_record's sequence number, and also build active_txn_ table &
 *lsn_mapping_ table
 */
void LogRecovery::Redo() {}

/*
 *undo phase on TABLE PAGE level(table/table_page.h)
 *iterate through active txn map and undo each operation
 */
void LogRecovery::Undo() {}

}  // namespace bustub
