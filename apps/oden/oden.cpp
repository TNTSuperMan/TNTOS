#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../syscall.h"

#include "context.hpp"

int main(int argc, char** argv) {
  if(argc != 2){
    printf("usage: oden [path]\n");
    return 0;
  }
  auto res = SyscallOpenFile(argv[1], 0);
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

  OdenContext* ctx = new OdenContext();
  ctx->eval(script);
  
  return 0;
}
