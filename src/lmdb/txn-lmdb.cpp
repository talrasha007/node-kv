#include <nan.h>

#include "env-lmdb.h"
#include "txn-lmdb.h"

using namespace v8;
using namespace kv;
using namespace kv::lmdb;

void txn::setup_export(Handle<Object>& exports) {
	// Prepare constructor template
	Local<FunctionTemplate> txnTpl = NanNew<FunctionTemplate>(txn::ctor);
	txnTpl->SetClassName(NanNew("Txn"));
	txnTpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add functions to the prototype
	NODE_SET_PROTOTYPE_METHOD(txnTpl, "commit", txn::commit);
	NODE_SET_PROTOTYPE_METHOD(txnTpl, "abort", txn::abort);
	NODE_SET_PROTOTYPE_METHOD(txnTpl, "reset", txn::reset);
	NODE_SET_PROTOTYPE_METHOD(txnTpl, "renew", txn::renew);

	// Set exports
	exports->Set(NanNew("Txn"), txnTpl->GetFunction());
}

NAN_METHOD(txn::ctor) {
	NanScope();

	env *ew = ObjectWrap::Unwrap<env>(args[0]->ToObject());
	txn *ptr = new txn(ew->_env, args[1]->BooleanValue());

	int rc = mdb_txn_begin(ptr->_env, NULL, ptr->_readonly ? MDB_RDONLY : 0, &ptr->_txn);

	if (rc != 0) {
		NanThrowError(mdb_strerror(rc));
		NanReturnUndefined();
	}

	ptr->Wrap(args.This());
	NanReturnValue(args.This());
}

NAN_METHOD(txn::commit) {
	NanScope();

	txn *tw = ObjectWrap::Unwrap<txn>(args.This());

	if (!tw->_txn) {
		NanThrowError("The transaction is already closed.");
		NanReturnUndefined();
	}

	int rc = mdb_txn_commit(tw->_txn);
	tw->_txn = NULL;
	if (rc != 0) {
		NanThrowError(mdb_strerror(rc));
		NanReturnUndefined();
	}

	NanReturnUndefined();
}

NAN_METHOD(txn::abort) {
	NanScope();

	txn *tw = ObjectWrap::Unwrap<txn>(args.This());

	if (!tw->_txn) {
		NanThrowError("The transaction is already closed.");
		NanReturnUndefined();
	}

	mdb_txn_abort(tw->_txn);
	tw->_txn = NULL;

	NanReturnUndefined();
}

NAN_METHOD(txn::reset) {
	NanScope();

	txn *tw = ObjectWrap::Unwrap<txn>(args.This());

	if (!tw->_txn) {
		NanThrowError("The transaction is already closed.");
		NanReturnUndefined();
	}

	if (!tw->_readonly) {
		NanThrowError("Only readonly transaction can be reset.");
		NanReturnUndefined();
	}

	mdb_txn_reset(tw->_txn);
	NanReturnUndefined();
}

NAN_METHOD(txn::renew) {
	NanScope();

	txn *tw = ObjectWrap::Unwrap<txn>(args.This());

	if (tw->_readonly) {
		if (!tw->_txn) {
			NanThrowError("The transaction is already closed.");
			NanReturnUndefined();
		}

		int rc = mdb_txn_renew(tw->_txn);
		if (rc != 0) {
			NanThrowError(mdb_strerror(rc));
			NanReturnUndefined();
		}
	}
	else {
		if (tw->_txn) {
			NanThrowError("The transaction is still opened.");
			NanReturnUndefined();
		}

		int rc = mdb_txn_begin(tw->_env, NULL, 0, &tw->_txn);
		if (rc != 0) {
			NanThrowError(mdb_strerror(rc));
			NanReturnUndefined();
		}
	}

	NanReturnUndefined();
}

txn::txn(MDB_env* env, bool readonly) : _readonly(readonly), _txn(NULL), _env(env) {

}

txn::~txn() {
	if (_txn) {
		printf("LMDB_txn warning: unclosed txn aborted by dtor.");
		mdb_txn_abort(_txn);
	}
}
