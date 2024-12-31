#ifndef __DIFFTRACE_H
#define __DIFFTRACE_H

#include <bits/stdc++.h>
#include <queue>
#include <unordered_set>
#include <algorithm>
#include "difftest-def.h"

class store_trace_t {
public:
  uint64_t paddr;
  uint64_t data;
  uint8_t  mask;

  store_trace_t(uint64_t paddr, uint64_t data, uint8_t mask) : paddr(paddr), data(data), mask(mask) {
    do_align();
  }
  store_trace_t(uint64_t paddr, uint64_t data, int len) : paddr(paddr), data(data), mask((1U << len) - 1) {
    if (len != 8) {
      this->data &= (1UL << (len * 8)) - 1UL;
    }
    do_align();
  };

private:
  void do_align() {
    uint64_t offset = paddr % 8UL;
    if (offset) {
      int len = std::log2((long)mask + 1);
      paddr = paddr - offset;
      data &= (1UL << (len * 8)) - 1UL;
      data <<= offset << 3;
      mask <<= offset;
    }
  }
};

class store_tracker_t {
public:
  store_tracker_t() : dirty_accessed(false) {}

  inline bool state() { return dirty_accessed; }
  inline void state_reset() { dirty_accessed = false; }
  inline void reset() {
    dirty.clear();
    state_reset();
  }
  inline void on_store(uint64_t address) {
    dirty.insert(hash_key(address));
  }
  inline void on_read(uint64_t address) {
    auto key = hash_key(address);
    if (dirty.count(key) > 0 || (check_next(address) && dirty.count(key + 1) > 0)) {
      dirty_accessed = true;
    }
  }

private:
  bool dirty_accessed;
  std::unordered_set<uint64_t> dirty;

protected:
  inline virtual uint64_t hash_key(uint64_t address) {
    return address / sizeof(uint64_t);
  }
  inline virtual bool check_next(uint64_t address) {
    return address % sizeof(uint64_t);
  }
};


class diff_trace_t
{
private:
  std::queue<store_trace_t> store_trace;
  // self-modified code. fence.i is the barrier
  store_tracker_t smc_tracker;
  // pte access. sfence.vma is the barrier
  store_tracker_t pte_tracker;
  // Writing satp does not imply any ordering constraints between
  // page-table updates and subsequent address translations.
  // Note: satp_written is too coarse-grained and must be optimized in the future.
  bool satp_written = false;

  enum class MemAccessType { INSTRUCTION, LOAD, STORE, PTW };
  static const char *accessTypeString(MemAccessType value) {
    switch (value) {
      case MemAccessType::INSTRUCTION: return "instr";
      case MemAccessType::LOAD:        return "load";
      case MemAccessType::STORE:       return "store";
      case MemAccessType::PTW:         return "ptw";
      default:                         return "unknown";
    }
  }

  void difftest_log_mem(MemAccessType t, uint64_t paddr, uint64_t data, int len) {
    difftest_log("mem_%-5s addr: 0x%lx, data: 0x%016lx, len: %d", accessTypeString(t), paddr, data, len);
    if (t == MemAccessType::STORE) {
      smc_tracker.on_store(paddr);
      pte_tracker.on_store(paddr);
      bool do_trace = !is_amo;
#ifdef CONFIG_DIFF_AMO_STORE
      do_trace = true;
#endif
#ifdef CONFIG_DIFF_NO_TRACE
      do_trace = false;
#endif
      if (do_trace) {

#ifdef CPU_XIANGSHAN
        bool store_miss_align = (paddr & (len - 1)) != 0;

        if (store_miss_align) {
#define LIMITING_SHIFT(data) std::min((uint64_t)63ULL, (uint64_t)data)

          bool cross_access_width = ((paddr >> 4) & 1ULL) == (((paddr + len - 1) >> 4) & 1ULL);
          uint64_t st_mask = (len == 1) ? 0x1ULL : (len == 2) ? 0x3ULL : (len == 4) ? 0xfULL : (len == 8) ? 0xffULL : 0xdeadbeefULL;
          uint64_t st_data_mask = (len == 1) ? 0xffULL : (len == 2) ? 0xffffULL : (len == 4) ? 0xffffffffULL : (len == 8) ? 0xffffffffffffffffULL : 0xdeadbeefULL;

          if (cross_access_width) {
            uint64_t inside_addr = paddr - (paddr % 16ULL);
            uint64_t inside_data = (paddr % 16ULL) > 8 ? 0 : (data & st_data_mask) << LIMITING_SHIFT((paddr % 16ULL) << 3);
            uint8_t  inside_mask = (st_mask << (paddr % 16ULL)) & 0xffULL;
            store_trace_t inside_trace{inside_addr, inside_data, inside_mask};
            store_trace.push(inside_trace);
          } 
          else {
          uint64_t low_addr  = paddr - (paddr % 8ULL);
          uint64_t high_addr = paddr - (paddr % 16ULL) + 16ULL;

          uint64_t low_data  = (data & (st_data_mask >> ((paddr % len) << 3))) << LIMITING_SHIFT((8 - len + (paddr % len)) << 3);
          uint64_t high_data = data >> LIMITING_SHIFT((len - (paddr % len)) << 3) & (st_data_mask >> LIMITING_SHIFT((len - (paddr % len)) << 3));;

          uint8_t low_mask  = (st_mask >> (paddr % len)) << LIMITING_SHIFT(8 - len + (paddr % len));
          uint8_t high_mask = st_mask >> (len - (paddr % len));

          store_trace_t low_trace {low_addr, low_data, low_mask};
          store_trace_t high_trace{high_addr, high_data, high_mask};

          store_trace.push(low_trace);
          store_trace.push(high_trace);
          }
        } 
        else {
          store_trace_t trace{paddr, data, len};
          store_trace.push(trace);
        }
#else
        store_trace_t trace{paddr, data, len};
        store_trace.push(trace);
#endif // CPU_XIANGSHAN

      }
      is_amo = false;
    }
    else if (t == MemAccessType::INSTRUCTION) {
      smc_tracker.on_read(paddr);
    }
    else if (t == MemAccessType::PTW) {
      pte_tracker.on_read(paddr);
    }
  }

  void difftest_log_mem(MemAccessType t, uint64_t paddr, void *data, int len) {
    difftest_log_mem(t, paddr, *(const uint64_t *)data, len);
  }

public:
  bool enable_difftest_logs = false;
  bool has_touched_vm = false;
  bool is_amo = false;
  bool sc_failed = false;

  void difftest_log(const char *__restrict __fmt, ...) {
    if (enable_difftest_logs) [[unlikely]] {
      va_list args;
      va_start(args, __fmt);
      fprintf(stderr, "[Spike] ");
      vfprintf(stderr, __fmt, args);
      fprintf(stderr, "\n");
      fflush(stderr);
      va_end(args);
    }
  }

#define __DIFFTEST_LOG_INTERFACE(name, type)                                      \
  void inline difftest_log_mem_##name(uint64_t paddr, void *data, int len) {      \
    difftest_log_mem(MemAccessType::type, paddr, *(const uint64_t *)data, len); \
  }

  __DIFFTEST_LOG_INTERFACE(instr, INSTRUCTION)
  __DIFFTEST_LOG_INTERFACE(load, LOAD)
  __DIFFTEST_LOG_INTERFACE(store, STORE)
  __DIFFTEST_LOG_INTERFACE(ptw, PTW)

  int dut_store_commit(uint64_t *addr, uint64_t *data, uint8_t *mask) {
    if (store_trace.empty()) {
      printf("Store commit error: the store trace is empty.\n");
      return -1;
    }

    store_trace_t ref = store_trace.front();
    store_trace_t dut{*addr, *data, *mask};
    if (ref.paddr != dut.paddr || ref.data != dut.data || ref.mask != dut.mask) {
      *addr = ref.paddr;
      *data = ref.data;
      *mask = ref.mask;
      return -1;
    }

    store_trace.pop();
    return 0;
  }

  void on_fence_i() {
    smc_tracker.reset();
  }

  void on_sfence_vma() {
    pte_tracker.reset();
    satp_written = false;
  }

  void on_satp_update(bool is_safe) {
    if (!is_safe && has_touched_vm) {
      satp_written = true;
    }
  }

  void clear_ambiguation_state() {
    smc_tracker.state_reset();
    pte_tracker.state_reset();
    satp_written = false;
  }

  bool in_ambiguation_state() {
    bool s = smc_tracker.state() || pte_tracker.state() || satp_written;
    clear_ambiguation_state();
    return s;
  }
};

#endif
