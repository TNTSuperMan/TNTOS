#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../syscall.h"

#include "context.hpp"

int main(int argc, char** argv) {
  if(argc == 1){
    printf("Welcome to oden\n");

    OdenContext* ctx = new OdenContext();
    char line[1024];
    while (fgets(line, sizeof(line), stdin)){
      const char* result = ctx->eval(line);
      if(result != nullptr){
        printf("%s\n", result);
      }
    }
    printf("\n");
  }else if(argc == 2){
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
  }else{
    printf("usage: oden [path]\n");
  }
  
  return 0;
}
