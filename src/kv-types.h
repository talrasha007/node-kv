#pragma once

#include <stdint.h>

namespace kv {
	class binary_type {
	public:
		static const char* type_name;

		binary_type(v8::Handle<v8::Value> val);
		binary_type(const char* val, size_t sz);

		v8::Local<v8::Value> v8value();
		const char* data();
		size_t size();

	private:
		const char* _data;
		size_t _size;
	};

	class string_type {
	public:
		static const char* type_name;

		string_type(v8::Handle<v8::Value> val);
		string_type(const char* val, size_t sz);
		~string_type();

		v8::Local<v8::Value> v8value();
		const char* data();
		size_t size();

	private:
		char* _data;
		size_t _size;
		bool _is_allocated;
	};

	class hex_type {
	public:
		static const char* type_name;

		hex_type(v8::Handle<v8::Value> val);
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

		number_type(v8::Handle<v8::Value> val);

		number_type(const char* val, size_t size) {
			_val = *(NUMBER*)val;
		}

		number_type(NUMBER val = 0) : _val(val) {

		}

		v8::Local<v8::Value> v8value() {
			return NanNew(_val);
		}

		const char* data() {
			return (const char*)&_val;
		}

		size_t size() {
			return sizeof(_val);
		}

	private:
		NUMBER _val;
	};

	template<> inline number_type<double>::number_type(v8::Handle<v8::Value> val) {
		_val = val->NumberValue();
	}

	template<> inline number_type<int32_t>::number_type(v8::Handle<v8::Value> val) {
		_val = val->Int32Value();
	}

	template<> inline number_type<uint32_t>::number_type(v8::Handle<v8::Value> val) {
		_val = val->Uint32Value();
	}

	template<> inline number_type<int64_t>::number_type(v8::Handle<v8::Value> val) {
		NanUtf8String utf8(val);
		_val = atoll(*utf8);
	}

	template<> inline v8::Local<v8::Value> number_type<int64_t>::v8value() {
		char buf[32];
		sprintf(buf, "%lld", (long long int)_val);
		return NanNew(buf);
	}
}

#define KV_VALTYPE_EACH(KT, EXP) \
	EXP(KT, kv::binary_type)\
	EXP(KT, kv::string_type)\
	EXP(KT, kv::hex_type)\
	EXP(KT, kv::number_type<int32_t>)\
	EXP(KT, kv::number_type<uint32_t>)\
	EXP(KT, kv::number_type<int64_t>)

#define KV_TYPE_EACH(EXP) \
	KV_VALTYPE_EACH(kv::binary_type, EXP) \
	KV_VALTYPE_EACH(kv::string_type, EXP) \
	KV_VALTYPE_EACH(kv::hex_type, EXP) \
	KV_VALTYPE_EACH(kv::number_type<int32_t>, EXP) \
	KV_VALTYPE_EACH(kv::number_type<uint32_t>, EXP) \
	KV_VALTYPE_EACH(kv::number_type<int64_t>, EXP)
