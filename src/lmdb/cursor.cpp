#include <nan.h>

#include "../kv-types.h"

#include "txn.h"
#include "db.h"
#include "cursor.h"

using namespace v8;
using namespace kv;
using namespace kv::lmdb;

#define CURSOR_EXPORT(KT, VT) cursor<KT, VT>::setup_export(exports);
void kv::lmdb::setup_cursor_export(v8::Handle<v8::Object>& exports) {
	KV_TYPE_EACH(CURSOR_EXPORT);
}

template<class K, class V> void cursor<K, V>::setup_export(Handle<Object>& exports) {
	char class_name[64];
	sprintf(class_name, "Cursor_%s_%s", K::type_name, V::type_name);

	// Prepare constructor template
	Local<FunctionTemplate> cursorTpl = NanNew<FunctionTemplate>(cursor::ctor);
	cursorTpl->SetClassName(NanNew(class_name));
	cursorTpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add functions to the prototype
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "close", cursor::close);

	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "del", cursor::del);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "current", cursor::current);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "next", cursor::next);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "prev", cursor::prev);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "nextDup", cursor::nextDup);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "prevDup", cursor::prevDup);

	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "seek", cursor::seek);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "gte", cursor::gte);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "first", cursor::first);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "last", cursor::last);

	// Set exports
	exports->Set(NanNew(class_name), cursorTpl->GetFunction());
}

#define KVCURSOR cursor<K, V>
#define KVCURSOR_METHOD(fn) template <class K, class V> NAN_METHOD(KVCURSOR::fn)

KVCURSOR_METHOD(ctor) {
	NanScope();

	db<K, V> *dw = ObjectWrap::Unwrap<db<K, V> >(args[0]->ToObject());
	txn *tw = ObjectWrap::Unwrap<txn>(args[1]->ToObject());

	MDB_cursor *cur = NULL;
	int rc = mdb_cursor_open(tw->_txn, dw->_dbi, &cur);
	if (rc != 0) {
		NanThrowError(mdb_strerror(rc));
		NanReturnUndefined();
	}

	cursor *ptr = new cursor(cur);
	ptr->Wrap(args.This());
	NanReturnValue(args.This());
}

KVCURSOR_METHOD(close) {
	NanScope();

	cursor *cw = ObjectWrap::Unwrap<cursor>(args.This());
	mdb_cursor_close(cw->_cursor);
	cw->_cursor = NULL;

	NanReturnUndefined();
}

KVCURSOR_METHOD(del) {
	NanScope();

	cursor *cw = ObjectWrap::Unwrap<cursor>(args.This());
	int rc = mdb_cursor_del(cw->_cursor, 0);
	if (rc != 0) {
		NanThrowError(mdb_strerror(rc));
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

template<class K, class V> template<MDB_cursor_op OP> NAN_METHOD(KVCURSOR::cursorOp) {
	NanScope();

	cursor *cw = ObjectWrap::Unwrap<cursor>(args.This());

	MDB_val key = { 0, 0 }, data = { 0, 0 };
	int rc = mdb_cursor_get(cw->_cursor, &key, &data, OP);
	if (rc == MDB_NOTFOUND) {
		NanReturnNull();
	} if (rc != 0) {
		NanThrowError(mdb_strerror(rc));
		NanReturnUndefined();
	}

	Local<Array> ret = NanNew<Array>(2);
	ret->Set(0, K((const char*)key.mv_data, key.mv_size).v8value());
	ret->Set(1, V((const char*)data.mv_data, data.mv_size).v8value());
	NanReturnValue(ret);
}

template<class K, class V> template<MDB_cursor_op OP> NAN_METHOD(KVCURSOR::cursorKeyOp) {
	NanScope();

	cursor *cw = ObjectWrap::Unwrap<cursor>(args.This());

	K k = K(args[0]);

	MDB_val key = { 0, 0 }, data = { 0, 0 };
	key.mv_data = (void*)k.data();
	key.mv_size = k.size();

	int rc = mdb_cursor_get(cw->_cursor, &key, &data, OP);
	if (rc == MDB_NOTFOUND) {
		NanReturnNull();
	} else if (rc != 0) {
		NanThrowError(mdb_strerror(rc));
		NanReturnUndefined();
	}

	Local<Array> ret = NanNew<Array>(2);
	ret->Set(0, K((const char*)key.mv_data, key.mv_size).v8value());
	ret->Set(1, V((const char*)data.mv_data, data.mv_size).v8value());
	NanReturnValue(ret);
}

template<class K, class V> template<MDB_cursor_op OP> NAN_METHOD(KVCURSOR::cursorKeyValOp) {
	NanScope();

	cursor *cw = ObjectWrap::Unwrap<cursor>(args.This());

	K k = K(args[0]);
	V v = V(args[1]);

	MDB_val key = { 0, 0 }, data = { 0, 0 };
	key.mv_data = (void*)k.data();
	key.mv_size = k.size();
	data.mv_data = (void*)v.data();
	data.mv_size = v.size();

	int rc = mdb_cursor_get(cw->_cursor, &key, &data, OP);
	if (rc == MDB_NOTFOUND) {
		NanReturnNull();
	} else if (rc != 0) {
		NanThrowError(mdb_strerror(rc));
		NanReturnUndefined();
	}

	Local<Array> ret = NanNew<Array>(2);
	ret->Set(0, K((const char*)key.mv_data, key.mv_size).v8value());
	ret->Set(1, V((const char*)data.mv_data, data.mv_size).v8value());
	NanReturnValue(ret);
}

KVCURSOR_METHOD(current) { return cursorOp<MDB_GET_CURRENT>(args); }
KVCURSOR_METHOD(next) { return cursorOp<MDB_NEXT>(args); }
KVCURSOR_METHOD(prev) { return cursorOp<MDB_PREV>(args); }
KVCURSOR_METHOD(nextDup) { return cursorOp<MDB_NEXT_DUP>(args); }
KVCURSOR_METHOD(prevDup) { return cursorOp<MDB_PREV_DUP>(args); }

KVCURSOR_METHOD(first) { return cursorOp<MDB_FIRST>(args); }
KVCURSOR_METHOD(last) { return cursorOp<MDB_LAST>(args); }
KVCURSOR_METHOD(firstDup) { return cursorOp<MDB_FIRST_DUP>(args); }
KVCURSOR_METHOD(lastDup) { return cursorOp<MDB_LAST_DUP>(args); }

KVCURSOR_METHOD(seek) {
	return args.Length() == 1 ? cursorKeyOp<MDB_SET>(args) : cursorKeyValOp<MDB_GET_BOTH>(args);
}

KVCURSOR_METHOD(gte) {
	return args.Length() == 1 ? cursorKeyOp<MDB_SET_RANGE>(args) : cursorKeyValOp<MDB_GET_BOTH_RANGE>(args);
}

template <class K, class V> cursor<K, V>::cursor(MDB_cursor *cur) : _cursor(cur) {

}

template <class K, class V> cursor<K, V>::~cursor() {
	if (_cursor) {
		printf("LMDB_cursor warning: unclosed cursor closed by dtor.\n");
		mdb_cursor_close(_cursor);
	}
}
