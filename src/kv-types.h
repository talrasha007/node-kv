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

		int compare(const number_type& o) const {
			return _val > o._val ? 1 : _val < o._val ? -1 : 0;
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

	template<class T, class CT, class ST> struct lvl_rocks_comparator {
		static CT* get_cmp() { return NULL; }
	};

	template<class N, class CT, class ST> struct lvl_rocks_comparator<number_type<N>, CT, ST> : public CT{
		static CT* get_cmp() {
			static lvl_rocks_comparator cmp;
			return &cmp;
		}

		typedef number_type<N> key_type;

		int Compare(const ST& a, const ST& b) const {
			key_type ka(a.data(), a.size()), kb(b.data(), b.size());
			return ka.compare(kb);
		}

		const char* Name() const {
			return key_type::type_name;
		}

		void FindShortestSeparator(std::string*, const ST&) const { }
		void FindShortSuccessor(std::string*) const { }
	};
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
