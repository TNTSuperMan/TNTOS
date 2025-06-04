#include "./file.hpp"
#include "../../syscall.h"

duk_ret_t readFile(duk_context *ctx) {
  const char* path = duk_require_string(ctx, -1);
  duk_pop(ctx);

  size_t len;
  SyscallResult res = SyscallOpenFile(path, 0);
  if(res.error) {
    printf("path: %s", path);
    duk_error(ctx, DUK_ERR_ERROR, "Failed to open file");
    return 0;
  }

  const int fd = res.value;
  res = SyscallMapFile(fd, &len, 0);
  if(res.error) {
    duk_error(ctx, DUK_ERR_ERROR, strerror(res.error));
    return 0;
  }
  duk_push_string(ctx, reinterpret_cast<const char*>(res.value));
  return 1;
}

duk_ret_t writeFile(duk_context *ctx) {
  size_t len;
  const char* path = duk_require_string(ctx, -2);
  const char* input = duk_require_lstring(ctx, -1, &len);
  duk_pop(ctx);
  duk_pop(ctx);
  
  FILE* fp = fopen(path, "w");
  if(fp == nullptr){
    printf("path: %s", path);
    duk_error(ctx, DUK_ERR_ERROR, "failed to open file");
    return 0;
  }

  size_t res = fwrite(input, 1, len, fp);
  fclose(fp);
  if(res != len){
    printf("path: %s", path);
    duk_error(ctx, DUK_ERR_ERROR, "failed to write file");
    return 0;
  }
  return 0;
}
