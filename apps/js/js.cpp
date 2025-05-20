#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../syscall.h"
#include "elk.h"

jsval_t print(js *js, jsval_t *args, int nargs){
  for(int i = 0;i < nargs;i++){
    printf("%s\n", js_str(js, args[i]));
  }
  return js_mkundef();
}

jsval_t readFile(js *js, jsval_t *args, int nargs){
  if(nargs < 1 || js_type(args[0]) != JS_STR){
    return js_mkerr(js, "path must be a string");
  }
  size_t len;
  const char* path = js_getstr(js, args[0], &len);
  SyscallResult res = SyscallOpenFile(path, 0);
  if(res.error){
    return js_mkerr(js, strerror(res.error));
  }

  const int fd = res.value;
  res = SyscallMapFile(fd, &len, 0);
  if(res.error){
    return js_mkerr(js, strerror(res.error));
  }

  return js_mkstr(js, reinterpret_cast<const void*>(res.value), len);
}

int main(int argc, char** argv) {
  if(argc != 2){
    printf("usage: js [path]\n");
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
  
  char mem[4096]; // メモ: oomはメモリ不足例外
  js *js = js_create(mem, sizeof(mem));

  js_set(js, js_glob(js), "print", js_mkfun(print));
  js_set(js, js_glob(js), "readFile", js_mkfun(readFile));

  jsval_t val = js_eval(js, script, fs);

  if(js_type(val) == JS_ERR){
    printf("%s\n", js_str(js, val));
  }

  return 0;
}
