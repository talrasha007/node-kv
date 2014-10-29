{
  "targets": [
    {
      "target_name": "kv",
	  "include_dirs" : [
        "<!(node -e \"require('nan')\")",
        "3rd-party/include"
      ],
      "dependencies": [
        "<(module_root_dir)/3rd-party/leveldb/leveldb.gyp:leveldb"
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
          "OS == 'win'", {
            "defines": [
              "_HAS_EXCEPTIONS=0"
            ],
            "msvs_settings": {
              "VCCLCompilerTool": {
                "DisableSpecificWarnings": ["4013", "4018", "4024", "4146", "4047", "4244", "4996"]
              }
            }
          }
        ],
        [
          "OS=='linux'", {
            "cflags_cc": [ "-std=c++0x" ]
          }
        ]
      ]
    }
  ]
}