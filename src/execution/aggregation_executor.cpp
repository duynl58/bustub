//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// aggregation_executor.cpp
//
// Identification: src/execution/aggregation_executor.cpp
//
// Copyright (c) 2015-19, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include "execution/executors/aggregation_executor.h"

#include <memory>
#include <vector>

namespace bustub {

AggregationExecutor::AggregationExecutor(ExecutorContext *exec_ctx, const AggregationPlanNode *plan,
                                         std::unique_ptr<AbstractExecutor> &&child)
    : AbstractExecutor(exec_ctx),
      plan_(plan),
      child_(std::move(child)),
      aht_(plan->GetAggregates(), plan->GetAggregateTypes()),
      aht_iterator_(aht_.Begin()) {}

const AbstractExecutor *AggregationExecutor::GetChildExecutor() const { return child_.get(); }

const Schema *AggregationExecutor::GetOutputSchema() { return plan_->OutputSchema(); }

void AggregationExecutor::Init() {
  this->child_->Init();
  Tuple tuple;
  while (this->child_->Next(&tuple)) {
    auto key = this->MakeKey(&tuple);
    auto value = this->MakeVal(&tuple);
    this->aht_.InsertCombine(key, value);
  }
  this->aht_iterator_ = this->aht_.Begin();
}

bool AggregationExecutor::Next(Tuple *tuple) {
  while (this->aht_iterator_ != this->aht_.End()) {
    const auto &agg_key = this->aht_iterator_.Key();
    const auto &agg_val = this->aht_iterator_.Val();
    ++this->aht_iterator_;
    if ((this->plan_->GetHaving() == nullptr) ||
        (this->plan_->GetHaving()->EvaluateAggregate(agg_key.group_bys_, agg_val.aggregates_).GetAs<bool>())) {
      std::vector<Value> result;
      for (auto &column : this->GetOutputSchema()->GetColumns()) {
        result.push_back(column.GetExpr()->EvaluateAggregate(agg_key.group_bys_, agg_val.aggregates_));
      }
      *tuple = Tuple(result, this->GetOutputSchema());
      return true;
    }
  }
  return false;
}

}  // namespace bustub
