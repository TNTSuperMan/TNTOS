#include "sys/time.h"
#include "syscall.h"

extern int main(int, char**);

void _start(int argc, char** argv) {
  SyscallExit(main(argc, argv));
}

int gettimeofday(struct timeval *tv, void* tz) {
  struct SyscallResult tick = SyscallGetCurrentTick();
  if(tick.error) return -1;
  tv->tv_sec = tick.value / 1000;
  tv->tv_usec = tick.value % 1000;
  return 0;
}
