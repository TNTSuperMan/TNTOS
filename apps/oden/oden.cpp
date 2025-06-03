#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../syscall.h"
#include "duktape/duktape.h"

#include "api/file.hpp"

duk_ret_t print(duk_context *ctx) {
  int nargs = duk_get_top(ctx);
  for(int i = 0;i < nargs; i++) {
    printf("%s\n", duk_to_string(ctx, -1));
    duk_pop(ctx);
  }
  return 0;
}

int main(int argc, char** argv) {
  if(argc != 2){
    printf("usage: oden [path]\n");
    return 0;
  }
  SyscallResult res = SyscallOpenFile(argv[1], 0);
  if(res.error){
    fprintf(stderr, "%s: %s\n", strerror(res.error), argv[1]);
    return 1;
  }
  const int fd = res.value;
  size_t fs;
  res = SyscallMapFile(fd, &fs, 0);
  if(res.error){
    fprintf(stderr, "%s\n", strerror(res.error));
    return 1;
  }

  const char* script = reinterpret_cast<char*>(res.value);
  
  duk_context *ctx = duk_create_heap_default();

  duk_push_c_function(ctx, print, DUK_VARARGS);
  duk_put_global_string(ctx, "print");
  
  duk_push_c_function(ctx, readFile, 1);
  duk_put_global_string(ctx, "readFile");
  
  duk_push_c_function(ctx, writeFile, 2);
  duk_put_global_string(ctx, "writeFile");

  if(duk_peval_string(ctx, script)){
    printf("%s\n", duk_safe_to_string(ctx, -1));
  }

  duk_destroy_heap(ctx);
  return 0;
}
