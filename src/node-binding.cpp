#include <nan.h>

#include "lmdb/env.h"

void init(v8::Handle<v8::Object> exports) {
}

NODE_MODULE(kv, init);