#pragma once

#include <leveldb/iterator.h>

namespace kv {
	namespace level {
		void setup_cursor_export(v8::Handle<v8::Object>& exports);

		template <class K, class V> class cursor : public node::ObjectWrap {
		public:
			typedef K key_type;
			typedef V value_type;
			typedef leveldb::DB db_type;
			typedef leveldb::Iterator iterator_type;

			static void setup_export(v8::Handle<v8::Object>& exports);

		private:
			template <void (iterator_type::*FN)()> static NAN_METHOD(cursorOp);

			static NAN_METHOD(ctor);

			static NAN_METHOD(key);
			static NAN_METHOD(value);

			static NAN_METHOD(next);
			static NAN_METHOD(prev);

			static NAN_METHOD(first);
			static NAN_METHOD(last);

			static NAN_METHOD(gte);

		private:
			cursor(iterator_type* cur);
			~cursor();

		private:
			iterator_type* _cursor;
		};
	}
}