#include <nan.h>

#include "env.h"
#include "txn.h"
#include "cursor.h"

#include "db.h"

#include "setup.h"

using namespace v8;

Handle<Object> kv::lmdb::v8export() {
	Local<Object> ret = NanNew<Object>();

	env::setup_export(ret);
	txn::setup_export(ret);
	setup_db_export(ret);

	return ret;
}
