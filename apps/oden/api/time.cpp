#include "time.hpp"
#include "../../syscall.h"

duk_ret_t getTime(duk_context *ctx) {
  EFI_TIME t;
  SyscallGetTime(&t);
  duk_push_object(ctx);

  duk_push_number(ctx, t.Day);
  duk_put_prop_string(ctx, -2, "day");
  
  duk_push_number(ctx, t.Daylight);
  duk_put_prop_string(ctx, -2, "daylight");

  duk_push_number(ctx, t.Hour);
  duk_put_prop_string(ctx, -2, "hour");
  
  duk_push_number(ctx, t.Minute);
  duk_put_prop_string(ctx, -2, "minute");

  duk_push_number(ctx, t.Month);
  duk_put_prop_string(ctx, -2, "month");
  
  duk_push_number(ctx, t.Nanosecond);
  duk_put_prop_string(ctx, -2, "nanosecond");

  duk_push_number(ctx, t.Pad1);
  duk_put_prop_string(ctx, -2, "pad1");
  
  duk_push_number(ctx, t.Pad2);
  duk_put_prop_string(ctx, -2, "pad2");

  duk_push_number(ctx, t.Second);
  duk_put_prop_string(ctx, -2, "second");
  
  duk_push_number(ctx, t.TimeZone);
  duk_put_prop_string(ctx, -2, "timezone");
  
  duk_push_number(ctx, t.Year);
  duk_put_prop_string(ctx, -2, "year");

  return 1;
}