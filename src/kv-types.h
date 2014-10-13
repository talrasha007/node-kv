#pragma once

#include <stdint.h>

namespace kv {
	using namespace v8;

	class binary_type {
	public:
		binary_type(Handle<Value>& val) {
			_data = node::Buffer::Data(val);
			_size = node::Buffer::Length(val);
		}

		binary_type(const char* val, size_t sz) : _data(val), _size(sz) {

		}

		Local<Value> v8value() {
			return NanNewBufferHandle(_data, _size);
		}

		const char* data() {
			return _data;
		}

		size_t size() {
			return _size;
		}

	private:
		const char* _data;
		size_t _size;
	};

	class hex_type {
	public:
		hex_type(Handle<Value>& val) : _mem(NULL), _is_allocated(false) {
			NanUtf8String utf8(val);
			int sz = utf8.Size() / 2;

			char *dest = sz > sizeof(_buf) ? (_is_allocated = true, _mem = new char[sz]) : _buf;

			int cnt = 0;
			for (const char* cur = *utf8; cnt < sz; cur += 2, cnt++) {
				char tmp[3] = { *cur, *(cur + 1), 0 };
				dest[cnt] = (char)strtol(tmp, NULL, 16);
			}

			_size = sz;
		}

		hex_type(const char* val, size_t sz) : _size(sz),  _mem((char*)val), _is_allocated(false) {

		}

		~hex_type() {
			if (_is_allocated) delete[] _mem;
		}

		Local<Value> v8value() {
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

		const char* data() {
			return _mem ? _mem : _buf;
		}

		size_t size() {
			return _size;
		}

	private:
		char _buf[512];
		size_t _size;

		char* _mem;
		bool _is_allocated;
	};

	template<class NUMBER> class number_type {
	public:
		number_type(Handle<Value>& val);
		number_type(NUMBER val = 0) : _val(val) {

		}

		Local<Value> v8value() {
			return NanNew(_val);
		}

		const char* data() {
			return (const char*)_val;
		}

		size_t size() {
			return sizeof(_val);
		}

	private:
		NUMBER _val;
	};
}
