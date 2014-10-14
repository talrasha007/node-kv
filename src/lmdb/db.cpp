#include <nan.h>

#include "../kv-types.h"

#include "env.h"
#include "txn.h"
#include "db.h"

using namespace v8;
using namespace kv;
using namespace kv::lmdb;

template<class T> int mdb_cmp_fn(const MDB_val *a, const MDB_val *b) {
	T ta((const char*)a->mv_data, a->mv_size), tb((const char*)b->mv_data, b->mv_size);
	return ta.compare(tb);
}

template<class T> struct mdb_cmp_setter {
	static void set_cmp(MDB_txn*, MDB_dbi) { }
	static void set_dup_cmp(MDB_txn*, MDB_dbi) { }
};

template<class N> struct mdb_cmp_setter<number_type<N> > {
	static void set_cmp(MDB_txn* txn, MDB_dbi dbi) {
		mdb_set_compare(txn, dbi, mdb_cmp_fn<number_type<N> >);
	}

	static void set_dup_cmp(MDB_txn* txn, MDB_dbi dbi) {
		mdb_set_dupsort(txn, dbi, mdb_cmp_fn<number_type<N> >);
	}
};

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
	NODE_SET_METHOD(dbiTpl->PrototypeTemplate(), "get", db::get);
	NODE_SET_METHOD(dbiTpl->PrototypeTemplate(), "put", db::put);
	NODE_SET_METHOD(dbiTpl->PrototypeTemplate(), "del", db::del);
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
		bool allowdup = options->Get(NanNew("allowDup"))->BooleanValue();

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

		// Set compare function.
		mdb_cmp_setter<K>::set_cmp(txn, dbi);
		if (allowdup) mdb_cmp_setter<V>::set_dup_cmp(txn, dbi);

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

class kv::lmdb::txn_scope {
public:
	txn_scope(Local<Value> arg, MDB_env *env, bool readonly = false) : _txn(NULL), _created(false), _commit(false) {
		if (arg->IsObject()) {
			_txn = node::ObjectWrap::Unwrap<txn>(arg->ToObject())->_txn;
		} else {
			_created = true;
			mdb_txn_begin(env, NULL, readonly ? MDB_RDONLY : 0, &_txn);
		}
	}

	~txn_scope() {
		if (_created) {
			if (!_commit) mdb_txn_abort(_txn);
			else mdb_txn_commit(_txn);
		}
	}

	MDB_txn *operator*() {
		return _txn;
	}

	void commit() {
		_commit = true;
	}

private:
	MDB_txn *_txn;
	bool _created;
	bool _commit;
};

template <class K, class V> NAN_METHOD((db<K, V>::get)) {
	NanScope();

	db *dw = ObjectWrap::Unwrap<db>(args.This());
	K key = K(args[0]);
	txn_scope tc(args[1], dw->_env, true);

	MDB_val k, v;
	k.mv_data = (void*)key.data();
	k.mv_size = key.size();

	int rc = mdb_get(*tc, dw->_dbi, &k, &v);

	if (rc == MDB_NOTFOUND) {
		NanReturnNull();
	}

	if (rc != 0) {
		NanThrowError(mdb_strerror(rc));
		NanReturnUndefined();
	}

	V val((const char*)v.mv_data, v.mv_size);
	NanReturnValue(val.v8value());
}

template <class K, class V> NAN_METHOD((db<K, V>::put)) {
	NanScope();

	db *dw = ObjectWrap::Unwrap<db>(args.This());
	K key = K(args[0]);
	V val = V(args[1]);
	txn_scope tc(args[2], dw->_env);

	MDB_val k, v;
	k.mv_data = (void*)key.data();
	k.mv_size = key.size();
	v.mv_data = (void*)val.data();
	v.mv_size = val.size();

	int rc = mdb_put(*tc, dw->_dbi, &k, &v, 0);
	if (rc != 0) {
		NanThrowError(mdb_strerror(rc));
		NanReturnUndefined();
	}

	tc.commit();
	NanReturnValue(NanNew(true));
}

template <class K, class V> NAN_METHOD((db<K, V>::del)) {
	NanScope();

	db *dw = ObjectWrap::Unwrap<db>(args.This());
	K key = K(args[0]);
	txn_scope tc(args[1], dw->_env);

	MDB_val k;
	k.mv_data = (void*)key.data();
	k.mv_size = key.size();

	int rc = mdb_del(*tc, dw->_dbi, &k, NULL);
	if (rc != 0) {
		NanThrowError(mdb_strerror(rc));
		NanReturnUndefined();
	}

	tc.commit();
	NanReturnValue(NanNew(true));
}

template <class K, class V> db<K, V>::db(MDB_env* env, MDB_dbi dbi) : _dbi(dbi), _env(env) {

}
