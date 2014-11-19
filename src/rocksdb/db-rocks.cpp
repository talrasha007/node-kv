#include <nan.h>
#include <rocksdb/slice.h>
#include <rocksdb/options.h>
#include <rocksdb/comparator.h>

#include "../kv-types.h"

#include "env-rocks.h"
#include "batch-rocks.h"
#include "db-rocks.h"

using namespace v8;
using namespace rocksdb;

namespace kv { namespace rocks {

#define DB_EXPORT(KT, VT) db<KT, VT>::setup_export(exports);
void setup_db_export(v8::Handle<v8::Object>& exports) {
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
	NODE_SET_PROTOTYPE_METHOD(dbiTpl, "get", db::get);
	NODE_SET_PROTOTYPE_METHOD(dbiTpl, "put", db::put);
	NODE_SET_PROTOTYPE_METHOD(dbiTpl, "del", db::del);
	NODE_SET_PROTOTYPE_METHOD(dbiTpl, "write", db::write);

	// Set exports
	exports->Set(NanNew(class_name), dbiTpl->GetFunction());
}

#define KVDB db<K, V>
#define KVDB_METHOD(fn) template <class K, class V> NAN_METHOD(KVDB::fn)

KVDB_METHOD(ctor) {
	NanScope();

	NanUtf8String name(args[1]);

	ColumnFamilyHandle* cf;
	env *ew = node::ObjectWrap::Unwrap<env>(args[0]->ToObject());

	ColumnFamilyOptions opt;
	typedef lvl_rocks_comparator<K, Comparator, Slice> cmp;
	if (cmp::get_cmp()) opt.comparator = cmp::get_cmp();

	Status s = ew->_db->CreateColumnFamily(opt, *name, &cf);
	if (!s.ok()) {
		NanThrowError(s.ToString().c_str());
		NanReturnUndefined();
	}

	db* ptr = new db(ew->_db, cf);
	ptr->Wrap(args.This());
	NanReturnValue(args.This());
}

KVDB_METHOD(get) {
	NanScope();

	db *dw = ObjectWrap::Unwrap<db>(args.This());

	K k(args[0]);
	Slice key(k.data(), k.size());

	std::string val;
	Status s = dw->_db->Get(ReadOptions(), dw->_cf, key, &val);

	if (s.IsNotFound()) {
		NanReturnNull();
	}

	if (!s.ok()) {
		NanThrowError(s.ToString().c_str());
		NanReturnUndefined();
	}

	V v(val.data(), val.size());
	NanReturnValue(v.v8value());
}

KVDB_METHOD(put) {
	NanScope();

	db *dw = ObjectWrap::Unwrap<db>(args.This());

	K k(args[0]);
	V v(args[1]);
	Slice key(k.data(), k.size()), val(v.data(), v.size());

	if (args[2]->IsObject()) {
		batch *bw = ObjectWrap::Unwrap<batch>(args[2]->ToObject());
		bw->_batch.Put(dw->_cf, key, val);
	} else {
		Status s = dw->_db->Put(WriteOptions(), dw->_cf, key, val);

		if (!s.ok()) {
			NanThrowError(s.ToString().c_str());
		}
	}

	NanReturnUndefined();
}

KVDB_METHOD(del) {
	NanScope();

	db *dw = ObjectWrap::Unwrap<db>(args.This());

	K k(args[0]);
	Slice key(k.data(), k.size());

	if (args[1]->IsObject()) {
		batch *bw = ObjectWrap::Unwrap<batch>(args[1]->ToObject());
		bw->_batch.Delete(dw->_cf, key);
	} else {
		Status s = dw->_db->Delete(WriteOptions(), dw->_cf, key);

		if (!s.ok()) {
			NanThrowError(s.ToString().c_str());
		}
	}

	NanReturnUndefined();
}

KVDB_METHOD(write) {
	NanScope();

	db *dw = ObjectWrap::Unwrap<db>(args.This());
	batch *bw = ObjectWrap::Unwrap<batch>(args[0]->ToObject());

	Status s = dw->_db->Write(WriteOptions(), &bw->_batch);

	if (!s.ok()) {
		NanThrowError(s.ToString().c_str());
	}

	NanReturnUndefined();
}

template <class K, class V> db<K, V>::db(rocksdb::DB *pdb, ColumnFamilyHandle *cf) : _db(pdb), _cf(cf) {

}

template <class K, class V> db<K, V>::~db() {
	delete _cf;
}

} } // namespace kv | namespace rocks
