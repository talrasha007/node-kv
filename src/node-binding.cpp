#include <nan.h>

#include "lmdb/env.h"

using namespace v8;

void init(v8::Handle<v8::Object> exports) {
	Local<Object> lmdb = NanNew<Object>();
	kv::lmdb::env::setup_export(lmdb);

	exports->Set(NanNew("lmdb"), lmdb);
}

NODE_MODULE(kv, init);