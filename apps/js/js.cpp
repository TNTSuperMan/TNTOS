#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../syscall.h"
#include "elk.h"

char errbuf[256];

jsval_t print(js *js, jsval_t *args, int nargs){
  for(int i = 0;i < nargs;i++){
    if(js_type(args[i]) == JS_STR){
      size_t len;
      char* str = js_getstr(js, args[i], &len);
      printf("%.*s\n", len, str);
    }else{
      printf("%s\n", js_str(js, args[i]));
    }
  }
  return js_mkundef();
}

jsval_t stats(js *js, jsval_t *args, int nargs){
  jsval_t statobj = js_mkobj(js);
  js_set(js, statobj, "size", js_mknum(js->size));
  js_set(js, statobj, "lwm", js_mknum(js->lwm));
  js_set(js, statobj, "css", js_mknum(js->css));
  js_set(js, statobj, "brk", js_mknum(js->brk));

  return statobj;
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

jsval_t writeFile(js *js, jsval_t *args, int nargs){
  if(nargs == 0){
    return js_mkerr(js, "path must be a string");
  }else if(nargs == 1){
    if(js_type(args[0]) != JS_STR){
      return js_mkerr(js, "path must be a string");
    }else{
      return js_mkerr(js, "input must be a string");
    }
  }else if(js_type(args[1]) != JS_STR){
    return js_mkerr(js, "input must be a string");
  }
  size_t len;
  char* path = js_getstr(js, args[0], &len);
  FILE* fp = fopen(path, "w");
  if(fp == nullptr){
    sprintf(errbuf, "failed to open: %s", path);
    return js_mkerr(js, errbuf);
  }

  char* input = js_getstr(js, args[1], &len);
  size_t res = fwrite(input, 1, len, fp);
  fclose(fp);
  if(res != len){
    sprintf(errbuf, "failed to write: %s", path);
    return js_mkerr(js, errbuf);
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
  
  char mem[4096]; // メモ: oomはメモリ不足例外
  js *js = js_create(mem, sizeof(mem));
  const jsval_t global = js_glob(js);

  js_set(js, global, "print", js_mkfun(print));
  js_set(js, global, "stats", js_mkfun(stats));
  js_set(js, global, "readFile", js_mkfun(readFile));
  js_set(js, global, "writeFile", js_mkfun(writeFile));

  jsval_t val = js_eval(js, script, fs);

  if(js_type(val) == JS_ERR){
    printf("%s\n", js_str(js, val));
  }

  return 0;
}
