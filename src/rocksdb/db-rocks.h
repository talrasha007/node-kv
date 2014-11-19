#pragma once

#include <rocksdb/db.h>

namespace kv {
	namespace rocks {
		void setup_db_export(v8::Handle<v8::Object>& exports);

		template<class K, class V> class db : public node::ObjectWrap {
		public:
			typedef K key_type;
			typedef V value_type;
			typedef rocksdb::DB db_type;
			typedef rocksdb::Cache cache_type;
			typedef rocksdb::Slice slice_type;
			typedef rocksdb::Options option_type;
			typedef rocksdb::ReadOptions readoption_type;
			typedef rocksdb::WriteOptions writeoption_type;
			typedef rocksdb::Status status_type;
			typedef rocksdb::Comparator comparator_type;

			static void setup_export(v8::Handle<v8::Object>& exports);

		private:
			static NAN_METHOD(ctor);
			static NAN_METHOD(init);

			static NAN_METHOD(put);
			static NAN_METHOD(get);
			static NAN_METHOD(del);
			static NAN_METHOD(write);

		private:
			template <class KK, class VV> friend class cursor;

		private:
			db(int idx);
			~db();

		private:
			int _cfidx;
			rocksdb::DB *_db;
			rocksdb::ColumnFamilyHandle *_cf;
		};
	} // namespace rocks
} // namespace kv
