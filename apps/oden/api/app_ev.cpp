#include "./app_ev.hpp"
#include "../../syscall.h"

duk_ret_t readEvent(duk_context *ctx) {
  AppEvent ev[1];
  auto [ n, err ] = SyscallReadEvent(ev, 1);
  if(err){
    duk_error(ctx, DUK_ERR_ERROR, "Failed to read event");
    return 0;
  }
  duk_push_object(ctx);
  switch(ev[0].type){
    case AppEvent::kQuit:
      duk_push_string(ctx, "quit");
      duk_put_prop_string(ctx, -2, "type");
      break;
    case AppEvent::kMouseMove:
      duk_push_string(ctx, "mouse_move");
      duk_put_prop_string(ctx, -2, "type");

      duk_push_int(ctx, ev[0].arg.mouse_move.x);
      duk_put_prop_string(ctx, -2, "x");

      duk_push_int(ctx, ev[0].arg.mouse_move.y);
      duk_put_prop_string(ctx, -2, "y");

      duk_push_int(ctx, ev[0].arg.mouse_move.dx);
      duk_put_prop_string(ctx, -2, "dx");

      duk_push_int(ctx, ev[0].arg.mouse_move.dy);
      duk_put_prop_string(ctx, -2, "dy");

      duk_push_uint(ctx, ev[0].arg.mouse_move.buttons);
      duk_put_prop_string(ctx, -2, "buttons");
      break;
    case AppEvent::kMouseButton:
      duk_push_string(ctx, "mouse_button");
      duk_put_prop_string(ctx, -2, "type");

      duk_push_int(ctx, ev[0].arg.mouse_button.x);
      duk_put_prop_string(ctx, -2, "x");

      duk_push_int(ctx, ev[0].arg.mouse_button.y);
      duk_put_prop_string(ctx, -2, "y");

      duk_push_int(ctx, ev[0].arg.mouse_button.button);
      duk_put_prop_string(ctx, -2, "button");
      break;
    case AppEvent::kTimerTimeout:
      duk_push_string(ctx, "timer");
      duk_put_prop_string(ctx, -2, "type");

      duk_push_number(ctx, ev[0].arg.timer.timeout);
      duk_put_prop_string(ctx, -2, "timeout");

      duk_push_int(ctx, ev[0].arg.timer.value);
      duk_put_prop_string(ctx, -2, "value");
      break;
    case AppEvent::kKeyPush:
      duk_push_string(ctx, "keypush");
      duk_put_prop_string(ctx, -2, "type");

      duk_push_uint(ctx, ev[0].arg.keypush.modifier);
      duk_put_prop_string(ctx, -2, "modifier");

      duk_push_uint(ctx, ev[0].arg.keypush.keycode);
      duk_put_prop_string(ctx, -2, "keycode");

      duk_push_lstring(ctx, (const char*)&ev[0].arg.keypush.ascii, 1);
      duk_put_prop_string(ctx, -2, "ascii");

      duk_push_boolean(ctx, ev[0].arg.keypush.press);
      duk_put_prop_string(ctx, -2, "press");
      break;
    default:
      duk_error(ctx, DUK_ERR_ERROR, "Unknown event readed");
      return 0;
  }
  return 1;
}
