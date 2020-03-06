#ifndef ORBIT_LINUX_TRACING_LINUX_PERF_UTILS_H_
#define ORBIT_LINUX_TRACING_LINUX_PERF_UTILS_H_

#include <asm/perf_regs.h>
#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <linux/version.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <ctime>

inline int perf_event_open(struct perf_event_attr* attr, pid_t pid, int cpu,
                           int group_fd, unsigned long flags) {
  return syscall(__NR_perf_event_open, attr, pid, cpu, group_fd, flags);
}

namespace LinuxTracing {

// This must be in sync with the struct perf_sample_id defined below.
static constexpr uint64_t SAMPLE_TYPE_BASIC_FLAGS =
    PERF_SAMPLE_TID | PERF_SAMPLE_TIME | PERF_SAMPLE_CPU;

// TODO: As this amount of memory has to be copied from the ring buffer for each
//  sample, this constant should be made available in some setting.
// Max to pass to perf_event_open without getting an error is (1u << 16u) - 8,
// because the kernel stores this in a short and because of alignment reasons.
// But the size the kernel actually returns is smaller, because the maximum size
// of the entire record the kernel is willing to return is (1u << 16u) - 8.
// If we want the size we pass to coincide with the size we get, we need to pass
// a lower value. For the current layout of perf_record_with_stack, the maximum
// size is 65312, but let's leave some extra room.
static constexpr uint16_t SAMPLE_STACK_USER_SIZE = 65000;

// Sample all registers: they might all be necessary for DWARF-based stack
// unwinding.
static constexpr uint64_t SAMPLE_REGS_USER_ALL =
    (1lu << PERF_REG_X86_AX) | (1lu << PERF_REG_X86_BX) |
    (1lu << PERF_REG_X86_CX) | (1lu << PERF_REG_X86_DX) |
    (1lu << PERF_REG_X86_SI) | (1lu << PERF_REG_X86_DI) |
    (1lu << PERF_REG_X86_BP) | (1lu << PERF_REG_X86_SP) |
    (1lu << PERF_REG_X86_IP) | (1lu << PERF_REG_X86_FLAGS) |
    (1lu << PERF_REG_X86_CS) | (1lu << PERF_REG_X86_SS) |
    (1lu << PERF_REG_X86_R8) | (1lu << PERF_REG_X86_R9) |
    (1lu << PERF_REG_X86_R10) | (1lu << PERF_REG_X86_R11) |
    (1lu << PERF_REG_X86_R12) | (1lu << PERF_REG_X86_R13) |
    (1lu << PERF_REG_X86_R14) | (1lu << PERF_REG_X86_R15);

// This struct must be in sync with the SAMPLE_TYPE_BASIC_FLAGS, as the bits set
// in perf_event_attr::sample_type determine the fields this struct should have.
struct perf_sample_id {
  uint32_t pid, tid; /* if PERF_SAMPLE_TID */
  uint64_t time;     /* if PERF_SAMPLE_TIME */
  uint32_t cpu, res; /* if PERF_SAMPLE_CPU */
};

struct perf_sample_regs_user_all {
  uint64_t abi;
  uint64_t ax;
  uint64_t bx;
  uint64_t cx;
  uint64_t dx;
  uint64_t si;
  uint64_t di;
  uint64_t bp;
  uint64_t sp;
  uint64_t ip;
  uint64_t flags;
  uint64_t cs;
  uint64_t ss;
  uint64_t r8;
  uint64_t r9;
  uint64_t r10;
  uint64_t r11;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
};

struct perf_sample_stack_user {
  uint64_t size;                     /* if PERF_SAMPLE_STACK_USER */
  char data[SAMPLE_STACK_USER_SIZE]; /* if PERF_SAMPLE_STACK_USER */
  uint64_t dyn_size; /* if PERF_SAMPLE_STACK_USER && size != 0 */
};

inline void perf_event_enable(int file_descriptor) {
  ioctl(file_descriptor, PERF_EVENT_IOC_RESET, 0);
  ioctl(file_descriptor, PERF_EVENT_IOC_ENABLE, 0);
}

inline void perf_event_disable(int file_descriptor) {
  ioctl(file_descriptor, PERF_EVENT_IOC_DISABLE, 0);
}

// perf_event_open for fork and exit.
int32_t task_event_open(int32_t cpu);

// perf_event_open for context switches.
int32_t pid_context_switch_event_open(pid_t pid);

int32_t cpu_context_switch_event_open(int32_t cpu);

// perf_event_open for stack sampling.
int32_t sample_mmap_task_event_open(pid_t pid, uint64_t period_ns);

// perf_event_open for uprobes.
int32_t uprobe_event_open(const char* module, uint64_t function_offset,
                          pid_t pid, int32_t cpu);

int32_t uprobe_stack_event_open(const char* module, uint64_t function_offset,
                                pid_t pid, int32_t cpu);

int32_t uretprobe_event_open(const char* module, uint64_t function_offset,
                             pid_t pid, int32_t cpu);

int32_t uretprobe_stack_event_open(const char* module, uint64_t function_offset,
                                   pid_t pid, int32_t cpu);

// perf_event_open for tracepoint events.
int32_t tracepoint_event_open(const char* tracepoint_category,
                              const char* tracepoint_name, pid_t pid, int32_t cpu);
}  // namespace LinuxTracing

// Format is based on the content the the event's format file:
// /sys/kernel/debug/tracing/events/<category>/<name>/format
struct __attribute__((__packed__)) AmdGpuSchedRunJobFormat {
  uint16_t common_type;
  uint8_t common_flags;
  uint8_t common_preempt_count;
  int32_t common_pid;
  uint64_t sched_job_id;
  int32_t timeline;  // needs special handling
  uint32_t context;
  uint32_t seqno;
  uint64_t ring_name; // ???, this is declared as char*
  uint32_t num_ibs;
};

struct __attribute__((__packed__)) AmdGpuCsIoctlFormat {
  uint16_t common_type;
  uint8_t common_flags;
  uint8_t common_preempt_count;
  int32_t common_pid;
  uint64_t sched_job_id;
  int32_t timeline;  // needs special handling
  uint32_t context;
  uint32_t seqno;
  uint64_t dma_fence;  // print the pointer address
  uint64_t ring_name;  // print the pointer address
  uint32_t num_ibs;

};

struct __attribute__((__packed__)) DmaFenceSignaledFormat {
  uint16_t common_type;
  uint8_t common_flags;
  uint8_t common_preempt_count;
  int32_t common_pid;
  int32_t driver;
  int32_t timeline;
  uint32_t context;
  uint32_t seqno;
};

// TODO: For debugging purposes, remove later.
void print_amdgpu_sched_run_job(AmdGpuSchedRunJobFormat* format);
void print_amdgpu_cs_ioctl(AmdGpuCsIoctlFormat* format);
void print_dma_fence_signaled(DmaFenceSignaledFormat* format);

int32_t get_tracepoint_id(const char* tracepoint_category,
                          const char* tracepoint_name);

#endif  // ORBIT_LINUX_TRACING_LINUX_PERF_UTILS_H_
