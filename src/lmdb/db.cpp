#include <nan.h>

#include "../kv-types.h"

#include "env.h"
#include "db.h"

using namespace v8;
using namespace kv;
using namespace kv::lmdb;

#define DB_EXPORT(KT, VT) db<KT, VT>::setup_export(exports);
void kv::lmdb::setup_db_export(v8::Handle<v8::Object>& exports) {
	KV_TYPE_EACH(DB_EXPORT);
}

template <class K, class V> void db<K, V>::setup_export(Handle<Object>& exports) {
	char class_name[64];
	sprintf(class_name, "DB_%s_%s", K::type_name, V::type_name);

	// Prepare constructor template
	Local<FunctionTemplate> dbiTpl = NanNew<FunctionTemplate>(db::ctor);
	dbiTpl->SetClassName(NanNew(class_name));
	dbiTpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add functions to the prototype
	NODE_SET_METHOD(dbiTpl->PrototypeTemplate(), "close", db::close);
	// TODO: wrap mdb_stat too

	// Set exports
	exports->Set(NanNew(class_name), dbiTpl->GetFunction());
}

template <class K, class V> NAN_METHOD((db<K, V>::ctor)) {
	int rc = 0;
	NanScope();

	MDB_txn *txn;
	MDB_dbi dbi;
	env *ew = node::ObjectWrap::Unwrap<env>(args[0]->ToObject());

	if (args[1]->IsObject()) {
		Local<Object> options = args[1]->ToObject();
		NanUtf8String name(options->Get(NanNew("name")));
		//bool allowdup = options->Get(NanNew("allowDup"))->BooleanValue();

		// Open transaction
		rc = mdb_txn_begin(ew->_env, NULL, 0, &txn);
		if (rc != 0) {
			mdb_txn_abort(txn);
			NanThrowError(mdb_strerror(rc));
			NanReturnUndefined();
		}

		// Open database
		rc = mdb_dbi_open(txn, *name, MDB_CREATE, &dbi);
		if (rc != 0) {
			mdb_txn_abort(txn);
			NanThrowError(mdb_strerror(rc));
			NanReturnUndefined();
		}

		// Commit transaction
		rc = mdb_txn_commit(txn);
		if (rc != 0) {
			NanThrowError(mdb_strerror(rc));
			NanReturnUndefined();
		}
	}
	else {
		NanThrowError("Invalid parameters.");
		NanReturnUndefined();
	}

	db* ptr = new db(ew->_env, dbi);
	ptr->Wrap(args.This());
	NanReturnValue(args.This());
}

template <class K, class V> NAN_METHOD((db<K, V>::close)) {
	NanScope();

	db *dw = ObjectWrap::Unwrap<db>(args.This());
	mdb_dbi_close(dw->_env, dw->_dbi);

	NanReturnUndefined();
}

template <class K, class V> db<K, V>::db(MDB_env* env, MDB_dbi dbi) : _dbi(dbi), _env(env) {

}
