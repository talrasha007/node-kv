#pragma once

#include <lmdb/lmdb.h>

namespace kv {
	namespace lmdb {
		class env;
		void setup_db_export(v8::Handle<v8::Object>& exports);

		template <class K, class V> class db : public node::ObjectWrap {
		public:
			typedef K key_type;
			typedef V value_type;

			static void setup_export(v8::Handle<v8::Object>& exports);

		private:
			static NAN_METHOD(ctor);

			static NAN_METHOD(close);
			static NAN_METHOD(get);
			static NAN_METHOD(put);
			static NAN_METHOD(del);

			static NAN_METHOD(exists);

		private:
			db(env* env, MDB_dbi dbi);
			~db();

		private:
			template <class KK, class VV> friend class cursor;

			MDB_dbi _dbi;
			env *_env;
			MDB_cursor *_cur;
		};

		class txn_scope;
	}
}
