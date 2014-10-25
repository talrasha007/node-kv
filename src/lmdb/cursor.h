#pragma once

#include <lmdb/lmdb.h>

namespace kv {
	namespace lmdb {
		void setup_cursor_export(v8::Handle<v8::Object>& exports);

		template <class K, class V> class cursor : public node::ObjectWrap {
		public:
			static void setup_export(v8::Handle<v8::Object>& exports);

		private:
			static NAN_METHOD(ctor);

			static NAN_METHOD(close);
			static NAN_METHOD(del);
			template <MDB_cursor_op OP> static NAN_METHOD(cursorOp);
			template <MDB_cursor_op OP> static NAN_METHOD(cursorKeyOp);
			template <MDB_cursor_op OP> static NAN_METHOD(cursorKeyValOp);

			static NAN_METHOD(current);
			static NAN_METHOD(next);
			static NAN_METHOD(prev);
			static NAN_METHOD(nextDup);
			static NAN_METHOD(prevDup);

			static NAN_METHOD(first);
			static NAN_METHOD(last);
			static NAN_METHOD(firstDup);
			static NAN_METHOD(lastDup);

			static NAN_METHOD(seek);
			static NAN_METHOD(lowerBound);

		private:
			cursor(MDB_cursor* cur);
			~cursor();

		private:
			MDB_cursor* _cursor;
		};
	}
}