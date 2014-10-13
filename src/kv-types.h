#pragma once

#include <stdint.h>

namespace kv {
	class binary_type {
	public:
		static const char* type_name;

		binary_type(v8::Handle<v8::Value>& val);
		binary_type(const char* val, size_t sz);

		v8::Local<v8::Value> v8value();
		const char* data();
		size_t size();

	private:
		const char* _data;
		size_t _size;
	};

	class hex_type {
	public:
		static const char* type_name;

		hex_type(v8::Handle<v8::Value>& val);
		hex_type(const char* val, size_t sz);
		~hex_type();

		v8::Local<v8::Value> v8value();
		const char* data();
		size_t size();

	private:
		char _buf[512];
		size_t _size;

		char* _mem;
		bool _is_allocated;
	};

	template<class NUMBER> class number_type {
	public:
		static const char* type_name;

		number_type(v8::Handle<v8::Value>& val);
		number_type(NUMBER val = 0) : _val(val) {

		}

		v8::Local<v8::Value> v8value() {
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

#define KV_VALTYPE_EACH(KT, EXP) \
	EXP(KT, kv::binary_type)\
	EXP(KT, kv::hex_type)\
	EXP(KT, kv::number_type<int32_t>)\
	EXP(KT, kv::number_type<uint32_t>)\
	EXP(KT, kv::number_type<int64_t>)

#define KV_TYPE_EACH(EXP) \
	KV_VALTYPE_EACH(kv::binary_type, EXP) \
	KV_VALTYPE_EACH(kv::hex_type, EXP) \
	KV_VALTYPE_EACH(kv::number_type<int32_t>, EXP) \
	KV_VALTYPE_EACH(kv::number_type<uint32_t>, EXP) \
	KV_VALTYPE_EACH(kv::number_type<int64_t>, EXP)
