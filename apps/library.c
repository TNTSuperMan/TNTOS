#include "sys/time.h"
#include "syscall.h"

extern int main(int, char**);

void _start(int argc, char** argv) {
  SyscallExit(main(argc, argv));
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
