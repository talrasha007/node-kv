#include <nan.h>

#include "db-rocks.h"
#include "cursor-rocks.h"
#include "batch-rocks.h"

#include "setup-rocks.h"

using namespace v8;

Handle<Object> kv::rocks::v8export() {
	Local<Object> ret = NanNew<Object>();

	setup_db_export(ret);
	setup_cursor_export(ret);
	batch::setup_export(ret);

	return ret;
}
