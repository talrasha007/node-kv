#include <nan.h>

#include "env-lmdb.h"
#include "txn-lmdb.h"
#include "cursor-lmdb.h"

#include "db-lmdb.h"

#include "setup-lmdb.h"

using namespace v8;

Handle<Object> kv::lmdb::v8export() {
	Local<Object> ret = NanNew<Object>();

	env::setup_export(ret);
	txn::setup_export(ret);
	setup_db_export(ret);
	setup_cursor_export(ret);

	return ret;
}
