#include <nan.h>

#include "env-rocks.h"

using namespace v8;
using namespace rocksdb;

namespace kv { namespace rocks {

void env::setup_export(v8::Handle<v8::Object>& exports) {
	// Prepare constructor template
	Local<FunctionTemplate> envTpl = NanNew<FunctionTemplate>(env::ctor);
	envTpl->SetClassName(NanNew("Env"));
	envTpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add functions to the prototype

	// Set exports
	exports->Set(NanNew("Env"), envTpl->GetFunction());

}

NAN_METHOD(env::ctor) {
	NanScope();

	NanUtf8String path(args[0]);
	DB* pdb;

	Options opt;
	opt.create_if_missing = true;
	if (args[1]->IsNumber()) opt.write_buffer_size = size_t(args[1]->NumberValue());

	Status s = DB::Open(opt, *path, &pdb);
	if (!s.ok()) {
		NanThrowError(s.ToString().c_str());
		NanReturnUndefined();
	}

	env* ptr = new env(pdb);
	ptr->Wrap(args.This());
	NanReturnValue(args.This());
}

env::env(DB *db) : _db(db) {

}

env::~env() {
	delete _db;
}

} } // namespace kv | namespace rocks
