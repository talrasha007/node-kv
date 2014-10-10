{
  "targets": [
    {
      "target_name": "kv",
	  "include_dirs" : [
        "<!(node -e \"require('nan')\")",
        "3rd-party"
      ],
      "sources": [
        "3rd-party/liblmdb/mdb.c",
        "3rd-party/liblmdb/midl.c"
      ]
    }
  ]
}