#include "window.hpp"
#include "../../syscall.h"

duk_ret_t openWindow(duk_context *ctx) {
  int w = duk_require_int(ctx, -5);
  int h = duk_require_int(ctx, -4);
  int x = duk_require_int(ctx, -3);
  int y = duk_require_int(ctx, -2);
  const char* title = duk_require_string(ctx, -1);

  const auto [lid, err] = SyscallOpenWindow(w, h, x, y, title);
  if(err) {
    printf("%d", err);
    duk_error(ctx, DUK_ERR_ERROR, "Failed to open window");
    return 0;
  }

  duk_push_uint(ctx, lid);

  return 1;
}

duk_ret_t closeWindow(duk_context *ctx) {
  unsigned int lid = duk_require_uint(ctx, -1);

  SyscallCloseWindow(lid);

  return 0;
}

duk_ret_t redrawWindow(duk_context *ctx) {
  unsigned int lid = duk_require_uint(ctx, -1);

  SyscallWinRedraw(lid);

  return 0;
}

duk_ret_t drawString(duk_context *ctx) {
  int lid = duk_require_uint(ctx, -5);
  int x = duk_require_int(ctx, -4);
  int y = duk_require_int(ctx, -3);
  int c = duk_require_uint(ctx, -2);
  const char* s = duk_require_string(ctx, -1);
  
  SyscallWinWriteString(lid, x, y, c, s);

  return 0;
}

duk_ret_t drawRectangle(duk_context *ctx) {
  int lid = duk_require_uint(ctx, -6);
  int x = duk_require_int(ctx, -5);
  int y = duk_require_int(ctx, -4);
  int w = duk_require_int(ctx, -3);
  int h = duk_require_int(ctx, -2);
  int c = duk_require_uint(ctx, -1);

  SyscallWinFillRectangle(lid, x, y, w, h, c);

  return 0;
}
