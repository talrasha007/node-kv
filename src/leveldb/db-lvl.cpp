#include <nan.h>
#include <leveldb/comparator.h>

#include "../kv-types.h"

#include "batch-lvl.h"
#include "db-lvl.h"

using namespace v8;
using namespace kv;
using namespace kv::level;

template<class T, class CT, class ST> struct lvl_comparator {
	CT* get_cmp() { return NULL; }
};

template<class N, class CT, class ST> struct lvl_comparator<number_type<N>, CT, ST> : public CT {
	CT* get_cmp() { return this; }

	typedef number_type<N> key_type;

	int Compare(const ST& a, const ST& b) const {
		key_type ka(a.data(), a.size()), kb(b.data(), b.size());
		return ka.compare(kb);
	}

	const char* Name() const {
		return key_type::type_name;
	}

	void FindShortestSeparator(std::string*, const leveldb::Slice&) const { }
	void FindShortSuccessor(std::string*) const { }
};

#define DB_EXPORT(KT, VT) db<KT, VT>::setup_export(exports);
void kv::level::setup_db_export(v8::Handle<v8::Object>& exports) {
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
	NODE_SET_METHOD(dbiTpl->PrototypeTemplate(), "get", db::get);
	NODE_SET_METHOD(dbiTpl->PrototypeTemplate(), "put", db::put);
	NODE_SET_METHOD(dbiTpl->PrototypeTemplate(), "del", db::del);

	// Set exports
	exports->Set(NanNew(class_name), dbiTpl->GetFunction());
}

#define KVDB db<K, V>
#define KVDB_METHOD(fn) template <class K, class V> NAN_METHOD(KVDB::fn)

KVDB_METHOD(ctor) {
	NanScope();

	db_type* pdb;
	option_type opt;
	lvl_comparator<K, comparator_type, slice_type> cmp;

	opt.create_if_missing = true;
	opt.comparator = cmp.get_cmp();

	status_type s = db_type::Open(opt, "", &pdb);

	if (!s.ok()) {
		NanThrowError("Leveldb: Open db error.");
		NanReturnUndefined();
	}

	db* ptr = new db(pdb);
	ptr->Wrap(args.This());
	NanReturnValue(args.This());
}

KVDB_METHOD(get) {
	NanScope();

	db *dw = ObjectWrap::Unwrap<db>(args.This());

	K k = K(args[0]);
	slice_type key(k.data(), k.size());

	std::string val;
	status_type s = dw->_db->Get(readoption_type(), key, &val);

	if (!s.ok()) {
		NanThrowError("Leveldb: Get error.");
		NanReturnUndefined();
	}

	V v(val.data(), val.size());
	NanReturnValue(v.v8value());
}

KVDB_METHOD(put) {
	NanScope();

	db *dw = ObjectWrap::Unwrap<db>(args.This());

	K k = K(args[0]);
	V v = V(args[0]);
	slice_type key(k.data(), k.size()), val(v.data(), v.size());

	status_type s = dw->_db->Put(writeoption_type(), key, val);

	if (!s.ok()) {
		NanThrowError("Leveldb: Set error.");
	}

	NanReturnUndefined();
}

KVDB_METHOD(del) {
	NanScope();

	db *dw = ObjectWrap::Unwrap<db>(args.This());

	K k = K(args[0]);
	slice_type key(k.data(), k.size());

	status_type s = dw->_db->Delete(writeoption_type(), key);

	if (!s.ok()) {
		NanThrowError("Leveldb: Set error.");
	}

	NanReturnUndefined();
}

template <class K, class V> db<K, V>::db(db_type* pdb) : _db(pdb) {

}

template <class K, class V> db<K, V>::~db() {
	delete _db;
}
