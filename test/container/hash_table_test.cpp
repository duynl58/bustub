//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// hash_table_test.cpp
//
// Identification: test/container/hash_table_test.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <thread>  // NOLINT
#include <vector>

#include "common/logger.h"
#include "container/hash/linear_probe_hash_table.h"
#include "gtest/gtest.h"
#include "murmur3/MurmurHash3.h"

namespace bustub {

TEST(HashTableTest, HashTable_Constructor) {
  auto *disk_manager = new DiskManager("test.db");
  auto *bpm = new BufferPoolManager(30, disk_manager);

  LinearProbeHashTable<int, int, IntComparator> ht("blah", bpm, IntComparator(), 20, HashFunction<int>());

  EXPECT_EQ(ht.GetSize(), 20);
  ht.Resize(5);
  EXPECT_EQ(ht.GetSize(), 20);  // do not reduce in size
  ht.Resize(30);
  EXPECT_EQ(ht.GetSize(), 60);  // double the size

  disk_manager->ShutDown();
  remove("test.db");
  delete disk_manager;
  delete bpm;
}


TEST(HashTableTest, HashTable_Insert) {
  auto *disk_manager = new DiskManager("test.db");
  auto *bpm = new BufferPoolManager(30, disk_manager);

  LinearProbeHashTable<int, int, IntComparator> ht("blah", bpm, IntComparator(), 20, HashFunction<int>());
  auto header_page = ht.HeaderPage();
  EXPECT_EQ(header_page->NumBlocks(), 1);

  std::unordered_map<int, slot_offset_t> expected_index;
  std::unordered_map<slot_offset_t, int> key_at_index;
  for(int i = 1; i < 10; i += 2) {
    EXPECT_EQ(ht.Insert(nullptr, i, i * 2), true);\
    auto slot_index = ht.GetSlotIndex(i);
    while (key_at_index.find(slot_index) != key_at_index.end()) {
      slot_index ++;
    }
    expected_index.insert({i, slot_index});
    key_at_index.insert({slot_index, i});
  }
  auto block_page = ht.BlockPage(header_page, 0);
  for(int i = 1; i < 10; i += 2) {
    auto slot_offset = expected_index.find(i);
    LOG_INFO("Offset %d", static_cast<int>(slot_offset->second));
    EXPECT_EQ(block_page->KeyAt(slot_offset->second), static_cast<int>(i));
    EXPECT_EQ(block_page->ValueAt(slot_offset->second), static_cast<int>(i * 2));
  }

  disk_manager->ShutDown();
  remove("test.db");
  delete disk_manager;
  delete bpm;
}

// NOLINTNEXTLINE
TEST(HashTableTest, DISABLED_SampleTest) {
  auto *disk_manager = new DiskManager("test.db");
  auto *bpm = new BufferPoolManager(50, disk_manager);

  LinearProbeHashTable<int, int, IntComparator> ht("blah", bpm, IntComparator(), 1000, HashFunction<int>());

  // insert a few values
  for (int i = 0; i < 5; i++) {
    ht.Insert(nullptr, i, i);
    std::vector<int> res;
    ht.GetValue(nullptr, i, &res);
    EXPECT_EQ(1, res.size()) << "Failed to insert " << i << std::endl;
    EXPECT_EQ(i, res[0]);
  }

  // check if the inserted values are all there
  for (int i = 0; i < 5; i++) {
    std::vector<int> res;
    ht.GetValue(nullptr, i, &res);
    EXPECT_EQ(1, res.size()) << "Failed to keep " << i << std::endl;
    EXPECT_EQ(i, res[0]);
  }

  // insert one more value for each key
  for (int i = 0; i < 5; i++) {
    if (i == 0) {
      // duplicate values for the same key are not allowed
      EXPECT_FALSE(ht.Insert(nullptr, i, 2 * i));
    } else {
      EXPECT_TRUE(ht.Insert(nullptr, i, 2 * i));
    }
    ht.Insert(nullptr, i, 2 * i);
    std::vector<int> res;
    ht.GetValue(nullptr, i, &res);
    if (i == 0) {
      // duplicate values for the same key are not allowed
      EXPECT_EQ(1, res.size());
      EXPECT_EQ(i, res[0]);
    } else {
      EXPECT_EQ(2, res.size());
      if (res[0] == i) {
        EXPECT_EQ(2 * i, res[1]);
      } else {
        EXPECT_EQ(2 * i, res[0]);
        EXPECT_EQ(i, res[1]);
      }
    }
  }

  // look for a key that does not exist
  std::vector<int> res;
  ht.GetValue(nullptr, 20, &res);
  EXPECT_EQ(0, res.size());

  // delete some values
  for (int i = 0; i < 5; i++) {
    EXPECT_TRUE(ht.Remove(nullptr, i, i));
    std::vector<int> res;
    ht.GetValue(nullptr, i, &res);
    if (i == 0) {
      // (0, 0) is the only pair with key 0
      EXPECT_EQ(0, res.size());
    } else {
      EXPECT_EQ(1, res.size());
      EXPECT_EQ(2 * i, res[0]);
    }
  }

  // delete all values
  for (int i = 0; i < 5; i++) {
    if (i == 0) {
      // (0, 0) has been deleted
      EXPECT_FALSE(ht.Remove(nullptr, i, 2 * i));
    } else {
      EXPECT_TRUE(ht.Remove(nullptr, i, 2 * i));
    }
  }
  disk_manager->ShutDown();
  remove("test.db");
  delete disk_manager;
  delete bpm;
}

}  // namespace bustub
