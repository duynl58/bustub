//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// hash_join_executor.cpp
//
// Identification: src/execution/hash_join_executor.cpp
//
// Copyright (c) 2015-19, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include "execution/executors/hash_join_executor.h"

#include <memory>
#include <vector>

namespace bustub {

HashJoinExecutor::HashJoinExecutor(ExecutorContext *exec_ctx, const HashJoinPlanNode *plan,
                                   std::unique_ptr<AbstractExecutor> &&left, std::unique_ptr<AbstractExecutor> &&right)
    : AbstractExecutor(exec_ctx),
      plan_(plan),
      exprs_(1, this->plan_->Predicate()),
      left_(std::move(left)),
      right_(std::move(right)),
      jht_("HashJoin", exec_ctx->GetBufferPoolManager(), jht_comp_, jht_num_buckets_, jht_hash_fn_) {}

void HashJoinExecutor::Init() {
  // Initialize the outer relation and populate the data into hash table
  Tuple tuple;
  this->left_->Init();
  while (this->left_->Next(&tuple)) {
    auto hash_val = this->HashValues(&tuple, this->left_->GetOutputSchema(), this->exprs_);
    this->jht_.Insert(this->GetExecutorContext()->GetTransaction(), hash_val, tuple);
  }
  // Init right executor
  this->right_->Init();
}

bool HashJoinExecutor::Next(Tuple *tuple) {
  if (tuple == nullptr) {
    this->right_->Next(tuple);
    this->current_hash_val_ = this->HashValues(tuple, this->right_->GetOutputSchema(), this->exprs_);
  }
  const auto &found_tuples = this->jht_.GetValue(this->GetExecutorContext()->GetTransaction(), this->current_hash_val_);
  if (this->current_tup_ite_ == found_tuples.end()) {
    if (!this->right_->Next(tuple)) {
      return false;
    }
    this->current_hash_val_ = this->HashValues(tuple, this->right_->GetOutputSchema(), this->exprs_);
    this->current_tup_ite_ =
        this->jht_.GetValue(this->GetExecutorContext()->GetTransaction(), this->current_hash_val_).begin();
  }
  auto left_tup = *(this->current_tup_ite_++);
  auto eval = this->plan_->Predicate()->EvaluateJoin(&left_tup, this->left_->GetOutputSchema(), tuple,
                                                     this->right_->GetOutputSchema());
  if (eval.GetAs<bool>()) {
    return true;
  }
  return this->Next(tuple);
}
}  // namespace bustub
