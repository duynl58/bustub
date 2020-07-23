//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// insert_executor.cpp
//
// Identification: src/execution/insert_executor.cpp
//
// Copyright (c) 2015-19, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include "execution/executors/insert_executor.h"
#include "common/logger.h"

#include <memory>

namespace bustub {

InsertExecutor::InsertExecutor(ExecutorContext *exec_ctx, const InsertPlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_exe_(std::move(child_executor)) {}

const Schema *InsertExecutor::GetOutputSchema() {
  const auto &catalog = this->GetExecutorContext()->GetCatalog();
  return &catalog->GetTable(this->plan_->TableOid())->schema_;
}

void InsertExecutor::Init() {
  const auto &catalog = this->GetExecutorContext()->GetCatalog();
  this->table_ = catalog->GetTable(this->plan_->TableOid())->table_.get();
  // Initialize all children
  if (!this->plan_->IsRawInsert()) {
    this->child_exe_->Init();
  }
}

bool InsertExecutor::Next([[maybe_unused]] Tuple *) {
  // is raw insert -> get all raw tuples and insert
  RID rid;
  Tuple tuple;
  if (this->plan_->IsRawInsert()) {
    auto numberOfTups = this->plan_->RawValues().size();
    for (size_t idx = 0; idx < numberOfTups; idx++) {
      tuple = Tuple(this->plan_->RawValuesAt(idx), this->GetOutputSchema());
      auto success = this->table_->InsertTuple(tuple, &rid, this->GetExecutorContext()->GetTransaction());
      // LOG_DEBUG("Insert tuple successfully: %s", tuple.ToString(this->GetOutputSchema()).c_str());
      if (!success) return success;
    }
    // LOG_DEBUG("Insert %d tuples into table", int(numberOfTups));
    return true;
  }
  // get tuples from child executor, and then insert them
  while (this->child_exe_->Next(&tuple)) {
    // LOG_DEBUG("Read tuple: %s", tuple.ToString(this->GetOutputSchema()).c_str());
    auto success = this->table_->InsertTuple(tuple, &rid, this->GetExecutorContext()->GetTransaction());
    if (!success) return success;
  }
  return true;
}

}  // namespace bustub
