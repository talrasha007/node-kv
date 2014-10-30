#include <nan.h>

#include "lmdb/setup-lmdb.h"
#include "leveldb/setup-lvl.h"

using namespace v8;

void init(v8::Handle<v8::Object> exports) {
	exports->Set(NanNew("lmdb"), kv::lmdb::v8export());
	exports->Set(NanNew("leveldb"), kv::level::v8export());
}

NODE_MODULE(kv, init);