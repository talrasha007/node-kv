#pragma once

#include <leveldb/write_batch.h>

namespace kv {
	namespace level {
		class batch : public node::ObjectWrap {
		public:
			typedef leveldb::WriteBatch batch_type;

			static void setup_export(v8::Handle<v8::Object>& exports);

		private:
			static NAN_METHOD(ctor);
			static NAN_METHOD(clear);

		private:
			template<class K, class V> friend class db;
			batch_type _batch;
		};
	}
}
