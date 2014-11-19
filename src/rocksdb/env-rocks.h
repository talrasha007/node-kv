#pragma once

#include <rocksdb/db.h>

namespace kv {
	namespace rocks {
		class env : public node::ObjectWrap {
		public:
			static void setup_export(v8::Handle<v8::Object>& exports);

		private:
			template <class K, class V> friend class db;

		private:
			static NAN_METHOD(ctor);

		private:
			env(rocksdb::DB *db);
			~env();

		private:
			rocksdb::DB *_db;
		};
	} // namespace rocks
} // namespace kv
