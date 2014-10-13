#include <nan.h>

#include "kv-types.h"

using namespace v8;
using namespace kv;

const char* binary_type::type_name = "binary";

binary_type::binary_type(Handle<Value>& val) {
	_data = node::Buffer::Data(val);
	_size = node::Buffer::Length(val);
}

binary_type::binary_type(const char* val, size_t sz) : _data(val), _size(sz) {

}

Local<Value> binary_type::v8value() {
	return NanNewBufferHandle(_data, _size);
}

const char* binary_type::data() {
	return _data;
}

size_t binary_type::size() {
	return _size;
}

const char* hex_type::type_name = "hex";

hex_type::hex_type(Handle<Value>& val) : _mem(NULL), _is_allocated(false) {
	NanUtf8String utf8(val);
	size_t sz = utf8.Size() / 2;

	char *dest = sz > sizeof(_buf) ? (_is_allocated = true, _mem = new char[sz]) : _buf;

	size_t cnt = 0;
	for (const char* cur = *utf8; cnt < sz; cur += 2, cnt++) {
		char tmp[3] = { *cur, *(cur + 1), 0 };
		dest[cnt] = (char)strtol(tmp, NULL, 16);
	}

	_size = sz;
}

hex_type::hex_type(const char* val, size_t sz) : _size(sz), _mem((char*)val), _is_allocated(false) {

}

hex_type::~hex_type() {
	if (_is_allocated) delete[] _mem;
}

Local<Value> hex_type::v8value() {
	char buf[1025];
	char* dest = _size > sizeof(buf) / 2 ? new char[_size * 2] : buf;
	char* src = _mem ? _mem : _buf;

	for (size_t i = 0; i < _size; i++) {
		sprintf(dest, "%02hhx", src[i]);
		dest += 2;
	}

	Local<String> ret = NanNew(dest);
	if (dest != buf) delete[] dest;
	return ret;
}

const char* hex_type::data() {
	return _mem ? _mem : _buf;
}

size_t hex_type::size() {
	return _size;
}

namespace kv {
	template<> const char* number_type<double>::type_name = "number";

	template<> number_type<double>::number_type(Handle<Value>& val) {
		_val = val->NumberValue();
	}

	template<> const char* number_type<int32_t>::type_name = "int32";

	template<> number_type<int32_t>::number_type(Handle<Value>& val) {
		_val = val->Int32Value();
	}

	template<> const char* number_type<uint32_t>::type_name = "uint32";

	template<> number_type<uint32_t>::number_type(Handle<Value>& val) {
		_val = val->Uint32Value();
	}

	template<> const char* number_type<int64_t>::type_name = "int64";

	template<> number_type<int64_t>::number_type(Handle<Value>& val) {
		NanUtf8String utf8(val);
		_val = atoll(*utf8);
	}

	template<> Local<Value> number_type<int64_t>::v8value() {
		char buf[32];
		sprintf(buf, "%lld", _val);
		return NanNew(buf);
	}
}