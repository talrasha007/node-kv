#pragma once

#include <lmdb/lmdb.h>

namespace kv {
	namespace lmdb {
		class env : public node::ObjectWrap {
		public:
			static void setup_export(v8::Handle<v8::Object> exports);

		private:
			static NAN_METHOD(ctor);
			static NAN_METHOD(setMapSize);
			static NAN_METHOD(setMaxDbs);

			static NAN_METHOD(open);
			static NAN_METHOD(close);

			static NAN_METHOD(sync);

		private:
			env();

		private:
			MDB_env *_env;
		};
	}
}
