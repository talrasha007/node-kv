#pragma once

#include <lmdb/lmdb.h>

namespace kv {
	namespace lmdb {
		void setup_db_export(v8::Handle<v8::Object>& exports);

		template <class K, class V> class db : public node::ObjectWrap {
		public:
			static void setup_export(v8::Handle<v8::Object>& exports);

		private:
			static NAN_METHOD(ctor);

			static NAN_METHOD(close);

		private:
			db(MDB_env* env, MDB_dbi dbi);

		private:
			MDB_dbi _dbi;
			MDB_env *_env;
		};
	}
}
