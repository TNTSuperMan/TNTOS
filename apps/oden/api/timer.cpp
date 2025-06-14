#include "timer.hpp"
#include "../../syscall.h"

duk_ret_t createTimer(duk_context *ctx) {
  unsigned int type = duk_require_number(ctx, -3);
  int value = duk_require_number(ctx, -2);
  unsigned long ms = duk_require_number(ctx, -1);

  const auto timeout = SyscallCreateTimer(type, value, ms);

  duk_push_number(ctx, timeout.value);

  return 1;
}
