#pragma once

#include <lmdb/lmdb.h>

namespace kv {
	namespace lmdb {
		class txn : public node::ObjectWrap {
		public:
			static void setup_export(v8::Handle<v8::Object>& exports);

		private:
			static NAN_METHOD(ctor);

			static NAN_METHOD(commit);
			static NAN_METHOD(abort);
			static NAN_METHOD(reset);
			static NAN_METHOD(renew);

		private:
			txn(MDB_env *env, bool readonly);

		private:
			friend class txn_scope;
			template <class K, class V> friend class db;

			bool _readonly;

			MDB_txn *_txn;
			MDB_env *_env;
		};
	}
}
