#include <nan.h>

#include "lmdb/setup-lmdb.h"
#include "leveldb/setup-lvl.h"

#include <rocksdb/db.h>
#include <rocksdb/slice.h>
#include <rocksdb/options.h>
using namespace rocksdb;

using namespace v8;

void init(v8::Handle<v8::Object> exports) {
	std::string kDBPath = "d:\\tmp\\rocksdb_column_families_example";
	DB* db;
	Options options;
	options.create_if_missing = true;
	Status s = DB::Open(options, kDBPath, &db);

	exports->Set(NanNew("lmdb"), kv::lmdb::v8export());
	exports->Set(NanNew("leveldb"), kv::level::v8export());
}

NODE_MODULE(kv, init);