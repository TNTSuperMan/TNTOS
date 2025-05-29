/**
 * @file main.cpp
 *
 * カーネル本体のプログラムを書いたファイル．
 */

#include <cstdint>
#include <cstddef>
#include <cstdio>

#include <deque>
#include <limits>
#include <numeric>
#include <vector>

#include "io/frame_buffer_config.hpp"
#include "memory/memory_map.hpp"
#include "graphic/graphics.hpp"
#include "io/mouse.hpp"
#include "graphic/font.hpp"
#include "console/console.hpp"
#include "io/pci.hpp"
#include "console/logger.hpp"
#include "usb/xhci/xhci.hpp"
#include "legacy/interrupt.hpp"
#include "legacy/asmfunc.h"
#include "memory/segment.hpp"
#include "memory/paging.hpp"
#include "memory/memory_manager.hpp"
#include "app/window.hpp"
#include "graphic/layer.hpp"
#include "app/message.hpp"
#include "io/timer.hpp"
#include "io/acpi.hpp"
#include "io/keyboard.hpp"
#include "app/task.hpp"
#include "console/terminal.hpp"
#include "io/fat.hpp"
#include "app/syscall.hpp"
#include "legacy/uefi.hpp"

__attribute__((format(printf, 1, 2))) int printk(const char* format, ...) {
  va_list ap;
  int result;
  char s[1024];

  va_start(ap, format);
  result = vsprintf(s, format, ap);
  va_end(ap);

  console->PutString(s);
  return result;
}

alignas(16) uint8_t kernel_main_stack[1024 * 1024];

// デスクトップの右下（タスクバーの右端）に現在時刻を表示する
void TaskWallclock(uint64_t task_id, int64_t data) {
  __asm__("cli");
  Task& task = task_manager->CurrentTask();
  auto clock_window = std::make_shared<Window>(
      8 * 10, 16 * 2, screen_config.pixel_format);
  const auto clock_window_layer_id = layer_manager->NewLayer()
    .SetWindow(clock_window)
    .SetDraggable(false)
    .Move(ScreenSize() - clock_window->Size() - Vector2D<int>{4, 8})
    .ID();
  layer_manager->UpDown(clock_window_layer_id, 2);
  __asm__("sti");

  auto draw_current_time = [&]() {
    EFI_TIME t;
    uefi_rt->GetTime(&t, nullptr);

    FillRectangle(*clock_window->Writer(),
                  {0, 0}, clock_window->Size(), {0, 0, 0});

    char s[64];
    sprintf(s, "%04d-%02d-%02d", t.Year, t.Month, t.Day);
    WriteString(*clock_window->Writer(), {0, 0}, s, {255, 255, 255});
    sprintf(s, "%02d:%02d:%02d", t.Hour, t.Minute, t.Second);
    WriteString(*clock_window->Writer(), {0, 16}, s, {255, 255, 255});

    Message msg{Message::kLayer, task_id};
    msg.arg.layer.layer_id = clock_window_layer_id;
    msg.arg.layer.op = LayerOperation::Draw;

    __asm__("cli");
    task_manager->SendMessage(1, msg);
    __asm__("sti");
  };

  draw_current_time();
  timer_manager->AddTimer(
      Timer{timer_manager->CurrentTick(), 1, task_id});

  while (true) {
    __asm__("cli");
    auto msg = task.ReceiveMessage();
    if (!msg) {
      task.Sleep();
      __asm__("sti");
      continue;
    }
    __asm__("sti");

    if (msg->type == Message::kTimerTimeout) {
      draw_current_time();
      timer_manager->AddTimer(
          Timer{msg->arg.timer.timeout + kTimerFreq, 1, task_id});
    }
  }
}

extern "C" void KernelMainNewStack(
    const FrameBufferConfig& frame_buffer_config_ref,
    const MemoryMap& memory_map_ref,
    const acpi::RSDP& acpi_table,
    void* volume_image,
    EFI_RUNTIME_SERVICES* rt) {
  MemoryMap memory_map{memory_map_ref};
  uefi_rt = rt;

  InitializeGraphics(frame_buffer_config_ref);
  InitializeConsole();

  SetLogLevel(kWarn);

  InitializeSegmentation();
  InitializePaging();
  InitializeMemoryManager(memory_map);
  InitializeTSS();
  InitializeInterrupt();

  fat::Initialize(volume_image);
  InitializeFont();
  InitializePCI();

  InitializeLayer();
  layer_manager->Draw({{0, 0}, ScreenSize()});

  acpi::Initialize(acpi_table);
  InitializeLAPICTimer();

  InitializeSyscall();

  InitializeTask();
  Task& main_task = task_manager->CurrentTask();

  usb::xhci::Initialize();
  InitializeKeyboard();
  InitializeMouse();

  app_loads = new std::map<fat::DirectoryEntry*, AppLoadInfo>;
  task_manager->NewTask()
    .InitContext(TaskTerminal, 0)
    .Wakeup();

  task_manager->NewTask()
    .InitContext(TaskWallclock, 0)
    .Wakeup();

  char str[128];

  while (true) {
    __asm__("cli");
    const auto tick = timer_manager->CurrentTick();
    __asm__("sti");

    __asm__("cli");
    auto msg = main_task.ReceiveMessage();
    if (!msg) {
      main_task.Sleep();
      __asm__("sti");
      continue;
    }

    __asm__("sti");

    switch (msg->type) {
    case Message::kInterruptXHCI:
      usb::xhci::ProcessEvents();
      break;
    case Message::kKeyPush:
      if (msg->arg.keyboard.press &&
                 msg->arg.keyboard.keycode == 59 /* F2 */) {
        task_manager->NewTask()
          .InitContext(TaskTerminal, 0)
          .Wakeup();
      } else {
        auto act = active_layer->GetActive();
        __asm__("cli");
        auto task_it = layer_task_map->find(act);
        __asm__("sti");
        if (task_it != layer_task_map->end()) {
          __asm__("cli");
          task_manager->SendMessage(task_it->second, *msg);
          __asm__("sti");
        }
      }
      break;
    case Message::kLayer:
      ProcessLayerMessage(*msg);
      __asm__("cli");
      task_manager->SendMessage(msg->src_task, Message{Message::kLayerFinish});
      __asm__("sti");
      break;
    }
  }
}

extern "C" void __cxa_pure_virtual() {
  while (1) __asm__("hlt");
}
