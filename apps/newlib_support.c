#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>

#include "syscall.h"

int close(int fd) {
  errno = EBADF;
  return -1;
}

int fstat(int fd, struct stat* buf) {
  errno = EBADF;
  return -1;
}

pid_t getpid(void) {
  return 0;
}

int isatty(int fd) {
  struct SyscallResult res = SyscallIsTerminal(fd);
  if (res.error == 0) {
    return res.value;
  }
  errno = res.error;
  return -1;
}

int kill(pid_t pid, int sig) {
  errno = EPERM;
  return -1;
}

off_t lseek(int fd, off_t offset, int whence) {
  errno = EBADF;
  return -1;
}

int open(const char* path, int flags) {
  struct SyscallResult res = SyscallOpenFile(path, flags);
  if (res.error == 0) {
    return res.value;
  }
  errno = res.error;
  return -1;
}

int posix_memalign(void** memptr, size_t alignment, size_t size) {
  void* p = malloc(size + alignment - 1);
  if (!p) {
    return ENOMEM;
  }
  uintptr_t addr = (uintptr_t)p;
  *memptr = (void*)((addr + alignment - 1) & ~(uintptr_t)(alignment - 1));
  return 0;
}

ssize_t read(int fd, void* buf, size_t count) {
  struct SyscallResult res = SyscallReadFile(fd, buf, count);
  if (res.error == 0) {
    return res.value;
  }
  errno = res.error;
  return -1;
}

caddr_t sbrk(int incr) {
  static uint64_t dpage_end = 0;
  static uint64_t program_break = 0;

  if (dpage_end == 0 || dpage_end < program_break + incr) {
    int num_pages = (incr + 4095) / 4096;
    struct SyscallResult res = SyscallDemandPages(num_pages, 0);
    if (res.error) {
      errno = ENOMEM;
      return (caddr_t)-1;
    }
    program_break = res.value;
    dpage_end = res.value + 4096 * num_pages;
  }

  const uint64_t prev_break = program_break;
  program_break += incr;
  return (caddr_t)prev_break;
}

ssize_t write(int fd, const void* buf, size_t count) {
  struct SyscallResult res = SyscallPutString(fd, buf, count);
  if (res.error == 0) {
    return res.value;
  }
  errno = res.error;
  return -1;
}

void _exit(int status) {
  SyscallExit(status);
}

int is_leap_year(int y){
  return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

int gettimeofday(struct timeval *tv, void* tz) {
  EFI_TIME t;
  SyscallGetTime(&t);

  // 時刻構造体 to UNIXエポック?(Copilotさんありがと!)
  // TODO 表示が4年ぐらうずれる、要微調整

  int days = 0;
  for(int y = 1970; y < t.Year; y++) {
    if(is_leap_year(y)){
      days += 366;
    }else{
      days += 365;
    }
  }

  for(int m = 1; m < t.Month; m++) {
    switch(m){
      case 1:  days += 31;
      case 2:
        if(is_leap_year(t.Year)){
          days += 29;
        }else{
          days += 28;
        }
      case 3:  days += 31;
      case 4:  days += 30;
      case 5:  days += 31;
      case 6:  days += 30;
      case 7:  days += 31;
      case 8:  days += 31;
      case 9:  days += 30;
      case 10: days += 31;
      case 11: days += 30;
      case 12: days += 31;
    }
  }

  days += t.Day - 1;

  __time_t seconds = days * 86400ULL;

  seconds += t.Hour * 3600;
  seconds += t.Minute * 60;
  seconds += t.Second;

  tv->tv_sec = seconds;
  tv->tv_usec = t.Nanosecond / 1000;
  
  return 0;
}
