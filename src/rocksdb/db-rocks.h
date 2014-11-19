#pragma once

#include <rocksdb/db.h>

namespace kv {
	namespace rocks {
		void setup_db_export(v8::Handle<v8::Object>& exports);

		template<class K, class V> class db : public node::ObjectWrap {
		public:
			static void setup_export(v8::Handle<v8::Object>& exports);

		private:
			static NAN_METHOD(ctor);
			static NAN_METHOD(put);
			static NAN_METHOD(get);
			static NAN_METHOD(del);
			static NAN_METHOD(write);

		private:
			db(rocksdb::ColumnFamilyHandle *db);
			~db();

		private:
			rocksdb::ColumnFamilyHandle *_cf;
		};
	} // namespace rocks
} // namespace kv
