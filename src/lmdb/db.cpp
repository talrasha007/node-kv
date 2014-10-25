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
	NODE_SET_METHOD(dbiTpl->PrototypeTemplate(), "exists", db::exists);
	// TODO: wrap mdb_stat too

	// Set exports
	exports->Set(NanNew(class_name), dbiTpl->GetFunction());
}

#define KVDB db<K, V>
#define KVDB_METHOD(fn) template <class K, class V> NAN_METHOD(KVDB::fn)

KVDB_METHOD(ctor) {
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
		rc = mdb_dbi_open(txn, *name, allowdup ? MDB_CREATE | MDB_DUPSORT : MDB_CREATE, &dbi);
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

	db* ptr = new db(ew, dbi);
	ptr->Wrap(args.This());
	NanReturnValue(args.This());
}

KVDB_METHOD(close) {
	NanScope();

	db *dw = ObjectWrap::Unwrap<db>(args.This());
	mdb_dbi_close(dw->_env->_env, dw->_dbi);

	if (dw->_cur) {
		mdb_cursor_close(dw->_cur);
		dw->_cur = NULL;
	}

	NanReturnUndefined();
}

class kv::lmdb::txn_scope {
public:
	txn_scope(Local<Value> arg, MDB_env *env) : _env(NULL), _txn(NULL), _readonly(false), _created(false), _commit(false) {
		if (arg->IsObject()) {
			_txn = node::ObjectWrap::Unwrap<txn>(arg->ToObject())->_txn;
		} else {
			_created = true;
			mdb_txn_begin(env, NULL, 0, &_txn);
		}
	}

	txn_scope(Local<Value> arg, env *env) : _env(env), _txn(NULL), _readonly(true), _created(false), _commit(false) {
		if (arg->IsObject()) {
			_txn = node::ObjectWrap::Unwrap<txn>(arg->ToObject())->_txn;
		}
		else {
			_created = true;
			_txn = env->require_readlock();
		}
	}

	~txn_scope() {
		if (_created) {
			if (_readonly) {
				_env->release_readlock();
			} else {
				if (!_commit) mdb_txn_abort(_txn);
				else mdb_txn_commit(_txn);
			}
		}
	}

	bool is_readonly() {
		return _readonly;
	}

	bool is_created() {
		return _created;
	}

	MDB_txn *operator*() {
		return _txn;
	}

	void commit() {
		_commit = true;
	}

private:
	env* _env;
	MDB_txn *_txn;
	bool _readonly;
	bool _created;
	bool _commit;
};

KVDB_METHOD(get) {
	NanScope();

	db *dw = ObjectWrap::Unwrap<db>(args.This());
	K key = K(args[0]);
	txn_scope tc(args[1], dw->_env);

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

KVDB_METHOD(put) {
	NanScope();

	db *dw = ObjectWrap::Unwrap<db>(args.This());
	K key = K(args[0]);
	V val = V(args[1]);
	txn_scope tc(args[2], dw->_env->_env);

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

KVDB_METHOD(del) {
	NanScope();

	db *dw = ObjectWrap::Unwrap<db>(args.This());
	K key = K(args[0]);
	txn_scope tc(args[1], dw->_env->_env);

	MDB_val k;
	k.mv_data = (void*)key.data();
	k.mv_size = key.size();

	int rc = mdb_del(*tc, dw->_dbi, &k, NULL);

	if (rc == MDB_NOTFOUND) {
		NanReturnValue(NanNew(false));
	}

	if (rc != 0) {
		NanThrowError(mdb_strerror(rc));
		NanReturnUndefined();
	}

	tc.commit();
	NanReturnValue(NanNew(true));
}

KVDB_METHOD(exists) {
	NanScope();

	db *dw = ObjectWrap::Unwrap<db>(args.This());
	K key = K(args[0]);
	V val = V(args[1]);

	MDB_val k, v;
	k.mv_data = (void*)key.data();
	k.mv_size = key.size();
	v.mv_data = (void*)val.data();
	v.mv_size = val.size();

	txn_scope tc(args[2], dw->_env);
	MDB_cursor *cur = NULL;
	if (tc.is_created()) {
		if (!dw->_cur) mdb_cursor_open(*tc, dw->_dbi, &dw->_cur);
		else mdb_cursor_renew(*tc, dw->_cur);
		cur = dw->_cur;
	} else {
		mdb_cursor_open(*tc, dw->_dbi, &cur);
	}

	int rc = mdb_cursor_get(cur, &k, &v, MDB_GET_BOTH);
	if (!tc.is_created()) {
		mdb_cursor_close(cur);
	}

	if (rc == MDB_NOTFOUND) {
		NanReturnValue(NanNew(false));
	} else if (rc == 0) {
		NanReturnValue(NanNew(true));
	} else {
		NanThrowError(mdb_strerror(rc));
		NanReturnUndefined();
	}
}

template <class K, class V> db<K, V>::db(env* env, MDB_dbi dbi) : _dbi(dbi), _env(env), _cur(NULL) {

}

template <class K, class V> db<K, V>::~db() {
	if (_cur) mdb_cursor_close(_cur);
}
