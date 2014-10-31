#include <nan.h>

#include "../kv-types.h"

#include "db-lvl.h"
#include "cursor-lvl.h"

using namespace v8;
using namespace kv;
using namespace kv::level;

#define CURSOR_EXPORT(KT, VT) cursor<KT, VT>::setup_export(exports);
void kv::level::setup_cursor_export(v8::Handle<v8::Object>& exports) {
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
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "current", cursor::current);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "next", cursor::next);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "prev", cursor::prev);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "first", cursor::first);
	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "last", cursor::last);

	NODE_SET_METHOD(cursorTpl->PrototypeTemplate(), "gte", cursor::gte);

	// Set exports
	exports->Set(NanNew(class_name), cursorTpl->GetFunction());
}

#define KVCURSOR cursor<K, V>
#define KVCURSOR_METHOD(fn) template <class K, class V> NAN_METHOD(KVCURSOR::fn)

KVCURSOR_METHOD(ctor) {
	NanScope();

	db<K, V> *dw = ObjectWrap::Unwrap<db<K, V> >(args[0]->ToObject());
	iterator_type *cur = dw->_db->NewIterator(db<K, V>::readoption_type());

	cursor *ptr = new cursor(cur);
	ptr->Wrap(args.This());
	NanReturnValue(args.This());
}

template<class K, class V> template<void (KVCURSOR::iterator_type::*FN)()> NAN_METHOD(KVCURSOR::cursorOp) {
	NanScope();

	cursor *cw = ObjectWrap::Unwrap<cursor>(args.This());
	(cw->_cursor->*FN)();
	if (!cw->_cursor->Valid()) {
		NanReturnNull();
	}

	db<K, V>::slice_type key(cw->_cursor->key());
	db<K, V>::slice_type val(cw->_cursor->value());

	Local<Array> ret = NanNew<Array>(2);
	ret->Set(0, K(key.data(), key.size()).v8value());
	ret->Set(1, V(val.data(), val.size()).v8value());
	NanReturnValue(ret);
}

KVCURSOR_METHOD(current) {
	NanScope();

	cursor *cw = ObjectWrap::Unwrap<cursor>(args.This());
	if (!cw->_cursor->Valid()) {
		NanReturnNull();
	}

	db<K, V>::slice_type key(cw->_cursor->key());
	db<K, V>::slice_type val(cw->_cursor->value());

	Local<Array> ret = NanNew<Array>(2);
	ret->Set(0, K(key.data(), key.size()).v8value());
	ret->Set(1, V(val.data(), val.size()).v8value());
	NanReturnValue(ret);
}

KVCURSOR_METHOD(next) {
	return cursorOp<&iterator_type::Next>(args);
}

KVCURSOR_METHOD(prev) {
	return cursorOp<&iterator_type::Prev>(args);
}

KVCURSOR_METHOD(first) {
	return cursorOp<&iterator_type::SeekToFirst>(args);
}

KVCURSOR_METHOD(last) {
	return cursorOp<&iterator_type::SeekToLast>(args);
}

KVCURSOR_METHOD(gte) {
	NanScope();

	cursor *cw = ObjectWrap::Unwrap<cursor>(args.This());

	K k(args[0]);
	db<K, V>::slice_type query(k.data(), k.size());
	cw->_cursor->Seek(query);

	if (!cw->_cursor->Valid()) {
		NanReturnNull();
	}

	db<K, V>::slice_type key(cw->_cursor->key());
	db<K, V>::slice_type val(cw->_cursor->value());

	Local<Array> ret = NanNew<Array>(2);
	ret->Set(0, K(key.data(), key.size()).v8value());
	ret->Set(1, V(val.data(), val.size()).v8value());
	NanReturnValue(ret);
}

template <class K, class V> cursor<K, V>::cursor(iterator_type *cur) : _cursor(cur) {

}

template <class K, class V> cursor<K, V>::~cursor() {
	delete _cursor;
}
