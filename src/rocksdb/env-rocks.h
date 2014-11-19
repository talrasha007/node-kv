#pragma once

#include <vector>
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
			static NAN_METHOD(open);

		private:
			env();
			~env();

		private:
			int register_db(const rocksdb::ColumnFamilyDescriptor& desc);

		private:
			rocksdb::DB *_db;
			std::vector<rocksdb::ColumnFamilyDescriptor> _desc;
			std::vector<rocksdb::ColumnFamilyHandle*> _handles;
		};
	} // namespace rocks
} // namespace kv
