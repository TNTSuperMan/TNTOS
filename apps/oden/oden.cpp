#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../syscall.h"
#include "duktape/duktape.h"
#include "duktape/duk_module_duktape.h"

#include "api/app_ev.hpp"
#include "api/file.hpp"
#include "api/time.hpp"
#include "api/timer.hpp"
#include "api/window.hpp"

duk_ret_t print(duk_context *ctx) {
  int nargs = duk_get_top(ctx);
  for(int i = 0;i < nargs; i++) {
    printf("%s\n", duk_to_string(ctx, -1));
    duk_pop(ctx);
  }
  return 0;
}

duk_ret_t mod_search(duk_context *ctx) {
  const char *module_id = duk_require_string(ctx, -4);
  if(strcmp(module_id, "oden") == 0){

    duk_push_object(ctx);
  
    duk_push_c_function(ctx, readFile, 1);
    duk_put_prop_string(ctx, -2, "readFile");
  
    duk_push_c_function(ctx, writeFile, 2);
    duk_put_prop_string(ctx, -2, "writeFile");
  
    duk_push_c_function(ctx, readEvent, 0);
    duk_put_prop_string(ctx, -2, "readEvent");

    duk_push_c_function(ctx, getTime, 0);
    duk_put_prop_string(ctx, -2, "getTime");

    duk_push_c_function(ctx, createTimer, 3);
    duk_put_prop_string(ctx, -2, "createTimer");

    duk_push_c_function(ctx, openWindow, 5);
    duk_put_prop_string(ctx, -2, "openWindow");

    duk_push_c_function(ctx, closeWindow, 1);
    duk_put_prop_string(ctx, -2, "closeWindow");

    duk_put_prop_string(ctx, -2, "exports");

    duk_pop(ctx);
    return 0;
  }else{
    SyscallResult res = SyscallOpenFile(module_id, 0);
    if(res.error){
      fprintf(stderr, "Failed to resolve %s: %s\n", module_id, strerror(res.error));
      duk_error(ctx, DUK_ERR_ERROR, "Failed to resolve file");
      return 0;
    }
    const int fd = res.value;
    size_t fs;
    res = SyscallMapFile(fd, &fs, 0);
    if(res.error){
      duk_error(ctx, DUK_ERR_ERROR, strerror(res.error));
      return 0;
    }

    const char* module = reinterpret_cast<char*>(res.value);
    duk_push_string(ctx, module);
    return 1;
  }
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

  duk_module_duktape_init(ctx);

  duk_push_c_function(ctx, print, DUK_VARARGS);
  duk_put_global_string(ctx, "print");
  
  duk_get_global_string(ctx, "Duktape");
  duk_push_c_function(ctx, mod_search, 4);
  duk_put_prop_string(ctx, -2, "modSearch");
  duk_pop(ctx);

  if(duk_peval_string(ctx, script)){
    printf("%s\n", duk_safe_to_string(ctx, -1));
  }

  duk_destroy_heap(ctx);
  return 0;
}
