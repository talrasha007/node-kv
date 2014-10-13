#include <nan.h>

#include "kv-types.h"

using namespace kv;

template<> number_type<double>::number_type(Handle<Value>& val) {
	_val = val->NumberValue();
}

template<> number_type<int32_t>::number_type(Handle<Value>& val) {
	_val = val->Int32Value();
}

template<> number_type<uint32_t>::number_type(Handle<Value>& val) {
	_val = val->Uint32Value();
}

template<> number_type<int64_t>::number_type(Handle<Value>& val) {
	NanUtf8String utf8(val);
	_val = atoll(*utf8);
}

template<> Local<Value> number_type<int64_t>::v8value() {
	char buf[32];
	sprintf(buf, "%d", _val);
	return NanNew(buf);
}
