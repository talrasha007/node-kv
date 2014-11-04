#include <nan.h>

#include "../kv-types.h"

#include "batch-lvl.h"

using namespace v8;
using namespace kv;
using namespace level;

void batch::setup_export(Handle<Object>& exports) {
	// Prepare constructor template
	Local<FunctionTemplate> batchTpl = NanNew<FunctionTemplate>(batch::ctor);
	batchTpl->SetClassName(NanNew("Batch"));
	batchTpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add functions to the prototype
	NODE_SET_PROTOTYPE_METHOD(batchTpl, "clear", batch::clear);

	// Set exports
	exports->Set(NanNew("Batch"), batchTpl->GetFunction());
}

NAN_METHOD(batch::ctor) {
	NanScope();
	batch* ptr = new batch;
	ptr->Wrap(args.This());
	NanReturnValue(args.This());
}

NAN_METHOD(batch::clear) {
	NanScope();

	batch* bat = ObjectWrap::Unwrap<batch>(args.This());
	bat->_batch.Clear();

	NanReturnUndefined();
}
