#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include "../syscall.h"

static constexpr int kRadius = 90;
static constexpr int blockSize = 20;
static constexpr int blockPadding = 5;
enum BlockViewState : char{
  k0,k1,k2,k3,k4,k5,k6,k7,k8,k9,unknown,flag
};

void DrawScreen(BlockViewState** blocks,uint64_t layer_id,int width,int height){
  for(int x = 0;x < width;x++){
    for(int y = 0;y < height;y++){
      int realX = (blockPadding+blockSize)*x+blockPadding,realY=(blockPadding+blockSize)*y+blockPadding+20;
      SyscallWinFillRectangle(layer_id,realX,realY,blockSize,blockSize,0xbbbbbb);
      switch(blocks[x][y]){
        case unknown:
          SyscallWinFillRectangle(layer_id,realX,realY,blockSize,blockSize,0xC6C6C6);
          SyscallWinDrawLine(layer_id,realX,realY,realX+blockSize,realY,0);
          SyscallWinDrawLine(layer_id,realX,realY,realX,realY+blockSize,0);
          SyscallWinDrawLine(layer_id,realX+blockSize,realY+blockSize,realX+blockSize,realY,0);
          SyscallWinDrawLine(layer_id,realX+blockSize,realY+blockSize,realX,realY+blockSize,0);
          break;
        case flag:
          SyscallWinFillRectangle(layer_id,realX,realY,blockSize,blockSize,0xC6C6C6);
          SyscallWinDrawLine(layer_id,realX,realY,realX+blockSize,realY,0);
          SyscallWinDrawLine(layer_id,realX,realY,realX,realY+blockSize,0);
          SyscallWinDrawLine(layer_id,realX+blockSize,realY+blockSize,realX+blockSize,realY,0);
          SyscallWinDrawLine(layer_id,realX+blockSize,realY+blockSize,realX,realY+blockSize,0);

          SyscallWinFillRectangle(layer_id,realX + 2,realY + 2,blockSize - 4,(blockSize - 4) / 2,0xff0000);
          SyscallWinFillRectangle(layer_id,realX + 2,realY + 2,3,blockSize - 4,0xff0000);
          break;
        case k0:
          break;
        default:
          char* numc = new char[2];
          numc[0] = blocks[x][y] + 48;
          numc[1] = 0;
          SyscallWinWriteString(layer_id,realX + 5,realY,0,numc);
          break;
      }
    }
  }
}

int main(int argc, char** argv) {
  int opt;
  int width=10,height=10,minecount=10;
  while((opt = getopt(argc,argv,"w:h:m")) != -1){
    switch(opt){
      case 'w': width = atoi(optarg); break;
      case 'h': height = atoi(optarg); break;
      case 'm': minecount = atoi(optarg); break;
    }
  }
  if((width * height) < minecount){
    fprintf(stderr,"too many mine");
    return -1;
  }
  auto [layer_id, err_openwin]
    = SyscallOpenWindow((blockSize + blockPadding) * width + blockPadding, 20 + (blockSize + blockPadding) * height + blockPadding, 10, 10, "mine");
  if (err_openwin) {
    return err_openwin;
  }

  
  BlockViewState **showblocks = new BlockViewState*[width];
  for(int i = 0;i < width;i++){
    showblocks[i] = new BlockViewState[height];
  }
  for(int x = 0;x < width;x++){
    for(int y = 0;y < height;y++){
      showblocks[x][y] = unknown;
    }
  }
  char **minedata = new char*[width];
  for(int i = 0;i < width;i++){
    minedata[i] = new char[height];
  }
  for(int x = 0;x < width;x++){
    for(int y = 0;y < height;y++){
      minedata[x][y] = 0;
    }
  }
  for(int i = 0;i < minecount;i++){
    regen:
    int x = rand() % width;
    int y = rand() % height;
    if(minedata[x][y] == 0){
      minedata[x][y] = 1;
    }else{
      goto regen;
    }
  }
  DrawScreen(showblocks,layer_id,width,height);
  SyscallWinRedraw(layer_id | LAYER_NO_REDRAW);
  AppEvent events[1];
  while (true) {
    auto [ n, err ] = SyscallReadEvent(events, 1);
    if (err) {
      fprintf(stderr, "ReadEvent failed: %s\n", strerror(err));
      SyscallCloseWindow(layer_id);
      return -1;
    }
    if (events[0].type == AppEvent::kQuit) {
      SyscallCloseWindow(layer_id);
      return 0;
    }else if(events[0].type == AppEvent::kMouseButton){
      auto& arg = events[0].arg.mouse_button;
      if(arg.press){
        fprintf(stderr,"(%d,%d): %d\n",arg.x,arg.y,arg.button);
      }
    }
  }
  return 0;
}
