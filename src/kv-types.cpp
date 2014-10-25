#include <nan.h>

#include "kv-types.h"

using namespace v8;
using namespace kv;

binary_type::binary_type(Handle<Value> val) {
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

string_type::string_type(Handle<Value> val) : _is_allocated(true) {
	Local<String> toStr = val->ToString();
	int buf_size = toStr->Utf8Length() + 1;
	_data = new char[buf_size];
	_size = toStr->WriteUtf8(_data, buf_size);
}

string_type::string_type(const char* val, size_t sz) : _data((char*)val), _size(sz), _is_allocated(false) {

}

string_type::~string_type() {
	if (_is_allocated) delete[] _data;
}

Local<Value> string_type::v8value() {
	return NanNew(_data);
}

const char* string_type::data() {
	return _data;
}

size_t string_type::size() {
	return _size;
}

hex_type::hex_type(Handle<Value> val) : _mem(NULL), _is_allocated(false) {
	NanUtf8String utf8(val);
	size_t sz = utf8.Size() / 2;

	char *dest = sz > sizeof(_buf) ? (_is_allocated = true, _mem = new char[sz]) : _buf;

	size_t cnt = 0;
	for (const char* cur = *utf8; cnt < sz; cur += 2, cnt++) {
		char tmp[3] = { *cur, *(cur + 1) ? *(cur + 1) : '0', 0 };
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
	char *dest = _size > sizeof(buf) / 2 ? new char[_size * 2] : buf;
	char *src = _mem ? _mem : _buf;

	char *cur = dest;
	for (size_t i = 0; i < _size; i++) {
		sprintf(cur, "%02hhx", src[i]);
		cur += 2;
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
	const char* binary_type::type_name = "binary";
	const char* string_type::type_name = "string";
	const char* hex_type::type_name = "hex";

	template<> const char* number_type<double>::type_name = "number";
	template<> const char* number_type<int32_t>::type_name = "int32";
	template<> const char* number_type<uint32_t>::type_name = "uint32";
	template<> const char* number_type<int64_t>::type_name = "int64";
}