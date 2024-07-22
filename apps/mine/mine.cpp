#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <random>
#include "../syscall.h"

static constexpr int kRadius = 90;
static constexpr int blockSize = 20;
static constexpr int blockPadding = 2;

enum BlockViewState : char{
  k0,k1,k2,k3,k4,k5,k6,k7,k8,k9,unknown,flag
};

struct BlockPos{
  int x;
  int y;
  bool isnone = true;
};

void DrawScreen(BlockViewState** blocks,uint64_t layer_id,int width,int height){
  for(int x = 0;x < width;x++){
    for(int y = 0;y < height;y++){
      int realX = (blockPadding+blockSize)*x+blockPadding,realY=(blockPadding+blockSize)*y+blockPadding+20;
      switch(blocks[x][y]){
        case unknown:
          SyscallWinFillRectangle(layer_id,realX,realY,blockSize,blockSize,0xbbbbbb);
          break;
        case flag:
          SyscallWinFillRectangle(layer_id,realX,realY,blockSize,blockSize,0xbbbbbb);

          SyscallWinFillRectangle(layer_id,realX + 2,realY + 2,blockSize - 4,(blockSize - 4) / 2,0xff0000);
          SyscallWinFillRectangle(layer_id,realX + 2,realY + 2,3,blockSize - 4,0xff0000);
          break;
        default:
          SyscallWinFillRectangle(layer_id,realX,realY,blockSize,blockSize,0xdddddd);
          if(blocks[x][y] != k0){
            char* numc = new char[2];
            numc[0] = blocks[x][y] + 48;
            numc[1] = 0;
            SyscallWinWriteString(layer_id,realX + 5,realY,0,numc);
          }
          
          break;
      }
    }
  }
}

BlockPos DetectMouse(int x,int y,int width,int height){
  BlockPos ret;
  if(y < 20 + blockPadding || x < blockPadding) return ret;
  for(int i = 0;i < width;i++){
    int min = (blockSize+blockPadding)*i+blockPadding;
    int max = (blockSize+blockPadding)*i+blockPadding+blockSize;
    if(min < x && x < max){
      ret.x = i;
      ret.isnone = false;
      break;
    }
  }
  if(ret.isnone) return ret;
  ret.isnone = true;
  for(int i = 0;i < height;i++){
    int min = (blockSize+blockPadding)*i+blockPadding;
    int max = 20+(blockSize+blockPadding)*i+blockPadding+blockSize;
    if(min < y && y < max){
      ret.y = i;
      ret.isnone = false;
      break;
    }
  }
  return ret;
}

BlockViewState** InitView(int width,int height){
  BlockViewState **showblocks = new BlockViewState*[width];
  for(int i = 0;i < width;i++){
    showblocks[i] = new BlockViewState[height];
  }
  for(int x = 0;x < width;x++){
    for(int y = 0;y < height;y++){
      showblocks[x][y] = unknown;
    }
  }
  return showblocks;
}

char** InitMine(int width,int height,int count){
  srand(SyscallGetCurrentTick().value);
  char **minedata = new char*[width];
  for(int i = 0;i < width;i++){
    minedata[i] = new char[height];
  }
  for(int x = 0;x < width;x++){
    for(int y = 0;y < height;y++){
      minedata[x][y] = 0;
    }
  }
  for(int i = 0;i < count;i++){
    regen:
    int x = rand() % width;
    int y = rand() % height;
    if(minedata[x][y] == 0){
      minedata[x][y] = 1;
    }else{
      goto regen;
    }
  }
  return minedata;
}

void OpenBlock(int x,int y,int width,int height,BlockViewState** state,char** mine){
  if(state[x][y] != unknown) return;
  bool left = x == 0;
  bool right = x == width - 1;
  bool up = y == 0;
  bool down = y == height - 1;
  int bomcount = 0;
  if(!left){
    if(mine[x-1][y]) bomcount++;
    if(!up){
      if(mine[x-1][y-1]) bomcount++;
    }
    if(!down){
      if(mine[x-1][y+1]) bomcount++;
    }
  }
  if(!right){
    if(mine[x+1][y]) bomcount++;
    if(!up){
      if(mine[x+1][y-1]) bomcount++;
    }
    if(!down){
      if(mine[x+1][y+1]) bomcount++;
    }
  }
  if(!up){
    if(mine[x][y-1]) bomcount++;
  }
  if(!down){
    if(mine[x][y+1]) bomcount++;
  }
  state[x][y] = (BlockViewState)bomcount;
  if(bomcount == 0){
    if(!left){
      OpenBlock(x-1,y,width,height,state,mine);
      if(!up) OpenBlock(x-1,y-1,width,height,state,mine);
      if(!down) OpenBlock(x-1,y+1,width,height,state,mine);
    }
    if(!right){
      OpenBlock(x+1,y,width,height,state,mine);
      if(!up) OpenBlock(x+1,y-1,width,height,state,mine);
      if(!down) OpenBlock(x+1,y+1,width,height,state,mine);
    }
    if(!up) OpenBlock(x,y+1,width,height,state,mine);
    if(!down) OpenBlock(x,y-1,width,height,state,mine);
  }
}

bool isWin(int width,int height,BlockViewState** state,char** mine){
  for(int x = 0;x < width;x++){
    for(int y=0;y < height;y++){
      if(!mine[x][y] && 10 <= (char)state[x][y]) return false;
    }
  }
  return true;
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
  if (err_openwin) return err_openwin;

  BlockViewState **showblocks = InitView(width,height);
  char **minedata = InitMine(width,height,minecount);
  
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
        if(arg.button != 2){
          BlockPos pos = DetectMouse(arg.x,arg.y,width,height);
          if(!pos.isnone){
            if(arg.button == 0){
              if(showblocks[pos.x][pos.y] == unknown){
                if(minedata[pos.x][pos.y]){
                  fprintf(stderr,"YOU ARE LOSER.\n");
                  SyscallCloseWindow(layer_id);
                  return 0;
                }else{
                  OpenBlock(pos.x,pos.y,width,height,showblocks,minedata);
                }
              }
            }else if(showblocks[pos.x][pos.y] == unknown){
              showblocks[pos.x][pos.y] = flag;
            }else if(showblocks[pos.x][pos.y] == flag){
              showblocks[pos.x][pos.y] = unknown;
            }
            DrawScreen(showblocks,layer_id,width,height);
            SyscallWinRedraw(layer_id | LAYER_NO_REDRAW);
            if(isWin(width,height,showblocks,minedata)){
              fprintf(stderr,"YOU ARE WINNER!\n");
              SyscallCloseWindow(layer_id);
              return 0;
            }
          }
        }
      }
    }
  }
  SyscallCloseWindow(layer_id);
  return 0;
}
