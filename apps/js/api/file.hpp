#include "../duktape/duktape.h"
#include "../../syscall.h"

duk_ret_t readFile(duk_context *ctx);
duk_ret_t writeFile(duk_context *ctx);
