#include "duktape/duktape.h"

class OdenContext {
    duk_context* ctx;
public:
    OdenContext();
    ~OdenContext();
    const char* eval(const char *script);
};
