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
  
  char mem[1024];
  js *js = js_create(mem, sizeof(mem));
  js_set(js, js_glob(js), "print", js_mkfun(print));
  js_eval(js, script, fs);

  return 0;
}
