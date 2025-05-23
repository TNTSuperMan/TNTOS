#include <cstdio>
#include <cstring>
#include "../syscall.h"

static const int kWidth = 200, kHeight = 130;

bool IsInside(int x, int y) {
  return 4 <= x && x < 4 + kWidth && 24 <= y && y < 24 + kHeight;
}

int main(int argc, char** argv) {
  auto [layer_id, err_openwin]
    = SyscallOpenWindow(kWidth + 8, kHeight + 28, 10, 10, "paint");
  if (err_openwin) {
    return err_openwin;
  }

  AppEvent events[1];
  bool press = false;
  while (true) {
    auto [ n, err ] = SyscallReadEvent(events, 1);
    if (err) {
      printf("ReadEvent failed: %s\n", strerror(err));
      break;
    }
    if (events[0].type == AppEvent::kQuit) {
      break;
    } else if (events[0].type == AppEvent::kMouseMove) {
      auto& arg = events[0].arg.mouse_move;
      const auto prev_x = arg.x - arg.dx, prev_y = arg.y - arg.dy;
      if (press && IsInside(prev_x, prev_y) && IsInside(arg.x, arg.y)) {
        SyscallWinDrawLine(layer_id, prev_x, prev_y, arg.x, arg.y, 0x000000);
      }
    } else if (events[0].type == AppEvent::kMouseButton) {
      auto& arg = events[0].arg.mouse_button;
      if (arg.button == 0) {
        press = arg.press;
        SyscallWinFillRectangle(layer_id, arg.x, arg.y, 1, 1, 0x000000);
      }
    }
  }

  SyscallCloseWindow(layer_id);
  return 0;
}
