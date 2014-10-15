node-kv
=======

[中文文档猛戳此处](https://github.com/talrasha007/node-kv/blob/master/README.zhcn.md)

An embeded key-value store for node.js, extremely fast.

## Features
- Multiple kv engine support. (Currently, LMDB only, LevelDB/RocksDB will be supported soon.)
- High speed. LMDB: >1,000,000op/s for get, >100,000op/s for write.
- Embeded, easy to use.

## Install
```
npm install node-kv
```

## Test & Benchmark
```
git clone https://github.com/talrasha007/node-kv.git
npm install
npm install -g matcha mocha
mocha   # Run unit test.
matcha  # Run benchmark.
```
