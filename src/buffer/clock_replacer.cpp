//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// clock_replacer.cpp
//
// Identification: src/buffer/clock_replacer.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/clock_replacer.h"

#include "common/logger.h"

namespace bustub {

ClockReplacer::ClockReplacer(size_t num_pages) {
  this->ref_flag_.clear();
  this->pin_pos_.clear();
  this->frames_.clear();
  this->clock_hand_ = this->frames_.begin();
}

ClockReplacer::~ClockReplacer() = default;

bool ClockReplacer::Victim(frame_id_t *frame_id) {
  std::lock_guard<std::mutex> lock(this->mux_);
  if (this->frames_.size() <= 0) {
    frame_id = nullptr;
    return false;
  }
  while (true) {
    if (this->clock_hand_ == this->frames_.end()) {
      this->clock_hand_ = this->frames_.begin();
    }
    auto f_id = *(this->clock_hand_);
    if (f_id >= 0 && !this->ref_flag_[f_id]) {
      *frame_id = f_id;
      // remove the frame
      this->PinImpl(f_id);
      return true;
    }
    // set ref_flag of f_id to false (cause it's true)
    if (f_id >= 0) {
      this->ref_flag_[f_id] = false;
    }
    // rotate the clock hand
    this->clock_hand_++;
  }
}

// this function is not thread-safe. Don't call this directly
void ClockReplacer::PinImpl(frame_id_t frame_id) {
  auto found_iterator = this->pin_pos_.find(frame_id);
  if (found_iterator != this->pin_pos_.end()) {
    if (found_iterator->second == this->clock_hand_) {
      this->clock_hand_++;
    }
    this->ref_flag_.erase(frame_id);
    this->frames_.erase(found_iterator->second);
    this->pin_pos_.erase(frame_id);
  }
}

void ClockReplacer::Pin(frame_id_t frame_id) {
  std::lock_guard<std::mutex> lock(this->mux_);
  this->PinImpl(frame_id);
}

void ClockReplacer::Unpin(frame_id_t frame_id) {
  std::lock_guard<std::mutex> lock(this->mux_);
  if (this->ref_flag_.find(frame_id) != this->ref_flag_.end()) {
    this->ref_flag_[frame_id] = true;
    return;
  }
  this->frames_.push_back(frame_id);
  this->ref_flag_.insert({frame_id, true});
  this->pin_pos_.insert({frame_id, std::prev(this->frames_.end())});
}

size_t ClockReplacer::Size() {
  std::lock_guard<std::mutex> lock(this->mux_);
  return this->frames_.size();
}

}  // namespace bustub
