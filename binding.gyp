{
  "targets": [
    {
      "target_name": "kv",
	  "include_dirs" : [
        "<!(node -e \"require('nan')\")",
        "3rd-party/include"
      ],
      "sources": [
        "3rd-party/liblmdb/mdb.c",
        "3rd-party/liblmdb/midl.c",
        "src/node-binding.cpp",
        "src/kv-types.cpp",
        "src/lmdb/setup.cpp",
        "src/lmdb/env.cpp",
        "src/lmdb/db.cpp",
        "src/lmdb/cursor.cpp",
        "src/lmdb/txn.cpp"
      ],
      "conditions": [
        [
          "OS=='linux'", {
            "cflags_cc": [ "-std=c++0x" ]
          }
        ]
      ]
    }
  ]
}