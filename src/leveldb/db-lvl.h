#pragma once

#include <leveldb/db.h>

namespace kv {
	namespace level {
		void setup_db_export(v8::Handle<v8::Object>& exports);

		template<class K, class V> class db : public node::ObjectWrap {
		public:
			typedef K key_type;
			typedef V value_type;
			typedef leveldb::DB db_type;
			typedef leveldb::Slice slice_type;
			typedef leveldb::Options option_type;
			typedef leveldb::ReadOptions readoption_type;
			typedef leveldb::WriteOptions writeoption_type;
			typedef leveldb::Status status_type;
			typedef leveldb::Comparator comparator_type;

			static void setup_export(v8::Handle<v8::Object>& exports);

		private:
			static NAN_METHOD(ctor);
			static NAN_METHOD(put);
			static NAN_METHOD(get);
			static NAN_METHOD(del);
			static NAN_METHOD(write);

		private:
			db(db_type* pdb);
			~db();

		private:
			db_type* _db;
		};
	}
}
