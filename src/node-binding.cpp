#include <nan.h>

#include "lmdb/setup.h"

using namespace v8;

void init(v8::Handle<v8::Object> exports) {
	exports->Set(NanNew("lmdb"), kv::lmdb::v8export());
}

NODE_MODULE(kv, init);