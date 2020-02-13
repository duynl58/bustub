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

namespace bustub {

ClockReplacer::ClockReplacer(size_t num_pages) : num_frames_{num_pages}, current_pos_{0} {
    this->ref_flag_.clear();
    this->frames_.clear();
    this->frames_.resize(num_pages, -1);
}

ClockReplacer::~ClockReplacer() = default;

bool ClockReplacer::Victim(frame_id_t *frame_id) {
    const std::lock_guard<std::shared_mutex> guard(this->clock_hand_mux_);
    auto old_pos = this->current_pos_;
    do {
        auto f_id = this->frames_[this->current_pos_];
        if (f_id >= 0 && !this->ref_flag_[f_id]) {
            *frame_id = f_id;
            return true;
        }
        // set ref_flag of f_id to false (cause it's true)
        if (f_id >= 0) this->ref_flag_[f_id] = false;
        this->current_pos_ = (this->current_pos_ + 1) % this->num_frames_;
    } while (this->current_pos_ != old_pos);
    frame_id = nullptr;
    return false;
}

void ClockReplacer::Pin(frame_id_t frame_id) {}

void ClockReplacer::Unpin(frame_id_t frame_id) {}

size_t ClockReplacer::Size() { return 0; }

}  // namespace bustub
