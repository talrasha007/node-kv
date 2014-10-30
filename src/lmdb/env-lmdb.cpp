#include <nan.h>
#include <lmdb/lmdb.h>

#include "env-lmdb.h"

using namespace v8;
using namespace kv;
using namespace kv::lmdb;

void env::setup_export(Handle<Object>& exports) {
	// Prepare constructor template
	Local<FunctionTemplate> envTpl = NanNew<FunctionTemplate>(env::ctor);
	envTpl->SetClassName(NanNew("Env"));
	envTpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add functions to the prototype
	NODE_SET_METHOD(envTpl->PrototypeTemplate(), "setMapSize", env::setMapSize);
	NODE_SET_METHOD(envTpl->PrototypeTemplate(), "setMaxDbs", env::setMaxDbs);

	NODE_SET_METHOD(envTpl->PrototypeTemplate(), "open", env::open);
	NODE_SET_METHOD(envTpl->PrototypeTemplate(), "close", env::close);
	NODE_SET_METHOD(envTpl->PrototypeTemplate(), "sync", env::sync);

	// Set exports
	exports->Set(NanNew("Env"), envTpl->GetFunction());
}

NAN_METHOD(env::ctor) {
	NanScope();

	env *ptr = new env();
	int rc = mdb_env_create(&ptr->_env);

	if (rc != 0) {
		mdb_env_close(ptr->_env);
		NanThrowError(mdb_strerror(rc));
		NanReturnUndefined();
	}

	ptr->Wrap(args.This());
	NanReturnValue(args.This());
}

NAN_METHOD(env::setMapSize) {
	NanScope();

	env *ew = ObjectWrap::Unwrap<env>(args.This());

	if (!ew->_env) {
		NanThrowError("The environment is already closed.");
		NanReturnUndefined();
	}

	if (args[0]->IsNumber()) mdb_env_set_mapsize(ew->_env, size_t(args[0]->NumberValue()));
	NanReturnUndefined();
}

NAN_METHOD(env::setMaxDbs) {
	NanScope();

	env *ew = ObjectWrap::Unwrap<env>(args.This());

	if (!ew->_env) {
		NanThrowError("The environment is already closed.");
		NanReturnUndefined();
	}

	if (args[0]->IsNumber()) mdb_env_set_maxdbs(ew->_env, args[0]->Uint32Value());
	NanReturnUndefined();
}

NAN_METHOD(env::open) {
	int rc = 0;
	NanScope();

	env *ew = ObjectWrap::Unwrap<env>(args.This());
	if (!ew->_env) {
		NanThrowError("The environment is already closed.");
		NanReturnUndefined();
	}

	NanUtf8String path(args[0]);
	int flags = MDB_NOSYNC;
	rc = mdb_env_open(ew->_env, *path, flags, 0664);

	if (rc != 0) {
		mdb_env_close(ew->_env);
		ew->_env = NULL;
		NanThrowError(mdb_strerror(rc));
		NanReturnUndefined();
	}

	int cleared = 0;
	mdb_reader_check(ew->_env, &cleared);

	NanReturnUndefined();
}

NAN_METHOD(env::close) {
	NanScope();

	env *ew = ObjectWrap::Unwrap<env>(args.This());

	if (!ew->_env) {
		NanThrowError("The environment is already closed.");
		NanReturnUndefined();
	}

	if (ew->_read_lock) mdb_txn_abort(ew->_read_lock);
	ew->_read_lock = NULL;

	mdb_env_close(ew->_env);
	ew->_env = NULL;

	NanReturnUndefined();
}

struct uv_env_sync {
	uv_work_t request;
	NanCallback* callback;
	env *ew;
	MDB_env *dbenv;
	int rc;
};

void sync_cb(uv_work_t *request) {
	// Performing the sync (this will be called on a separate thread)
	uv_env_sync *d = static_cast<uv_env_sync*>(request->data);
	d->rc = mdb_env_sync(d->dbenv, 1);
}

void after_sync_cb(uv_work_t *request, int) {
	// Executed after the sync is finished
	uv_env_sync *d = static_cast<uv_env_sync*>(request->data);
	const unsigned argc = 1;
	Handle<Value> argv[argc];

	if (d->rc == 0) {
		argv[0] = NanNull();
	}
	else {
		argv[0] = Exception::Error(NanNew<String>(mdb_strerror(d->rc)));
	}

	d->callback->Call(argc, argv);
	delete d->callback;
	delete d;
}

NAN_METHOD(env::sync) {
	NanScope();

	env *ew = ObjectWrap::Unwrap<env>(args.This());

	if (!ew->_env) {
		NanThrowError("The environment is already closed.");
		NanReturnUndefined();
	}

	Handle<Function> callback = Handle<Function>::Cast(args[0]);

	uv_env_sync *d = new uv_env_sync;
	d->request.data = d;
	d->ew = ew;
	d->dbenv = ew->_env;
	d->callback = new NanCallback(callback);

	uv_queue_work(uv_default_loop(), &d->request, sync_cb, after_sync_cb);

	NanReturnUndefined();
}

env::env() : _env(NULL), _read_lock(NULL) {

}

env::~env() {
	if (_read_lock) mdb_txn_abort(_read_lock);
	if (_env) mdb_env_close(_env);
}

MDB_txn* env::require_readlock() {
	if (_read_lock) {
		mdb_txn_renew(_read_lock);
	} else {
		mdb_txn_begin(_env, NULL, MDB_RDONLY, &_read_lock);
	}

	return _read_lock;
}

void env::release_readlock() {
	mdb_txn_reset(_read_lock);
}
