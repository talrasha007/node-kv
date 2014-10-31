#include <nan.h>

#include "db-lvl.h"
#include "cursor-lvl.h"
#include "batch-lvl.h"

#include "setup-lvl.h"

using namespace v8;

Handle<Object> kv::level::v8export() {
	Local<Object> ret = NanNew<Object>();

	setup_db_export(ret);
	setup_cursor_export(ret);
	batch::setup_export(ret);

	return ret;
}
