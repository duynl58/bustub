//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// seq_scan_executor.cpp
//
// Identification: src/execution/seq_scan_executor.cpp
//
// Copyright (c) 2015-19, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include "execution/executors/seq_scan_executor.h"

namespace bustub {

SeqScanExecutor::SeqScanExecutor(ExecutorContext *exec_ctx, const SeqScanPlanNode *plan)
    : AbstractExecutor(exec_ctx), plan_(plan) {}

void SeqScanExecutor::Init() {
  const auto &catalog = this->GetExecutorContext()->GetCatalog();
  this->table_ = catalog->GetTable(this->plan_->GetTableOid())->table_.get();
  this->iter_ = std::make_unique<TableIterator>(this->table_->Begin(this->GetExecutorContext()->GetTransaction()));
}

bool SeqScanExecutor::Next(Tuple *tuple) {
  auto &iter = *(this->iter_);
  while (iter != this->table_->End()) {
    auto tup = *(iter++);
    auto eval = true;
    if (this->plan_->GetPredicate() != nullptr) {
      eval = this->plan_->GetPredicate()->Evaluate(&tup, this->GetOutputSchema()).GetAs<bool>();
    }
    if (eval) {
      *tuple = Tuple(tup);
      return true;
    }
  }
  return false;
}

}  // namespace bustub
