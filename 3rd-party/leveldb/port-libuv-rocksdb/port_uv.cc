// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "port_uv.h"

#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include "util/logging.h"

namespace rocksdb {
namespace port {

Mutex::Mutex(bool) { uv_mutex_init(&mu_); }

Mutex::~Mutex() { uv_mutex_destroy(&mu_); }

void Mutex::Lock() { uv_mutex_lock(&mu_); }

void Mutex::Unlock() { uv_mutex_unlock(&mu_); }

RWMutex::RWMutex() { uv_rwlock_init(&mu_); }

RWMutex::~RWMutex() { uv_rwlock_destroy(&mu_); }

void RWMutex::ReadLock() { uv_rwlock_rdlock(&mu_); }

void RWMutex::WriteLock() { uv_rwlock_wrlock(&mu_); }

void RWMutex::ReadUnlock() { uv_rwlock_rdunlock(&mu_); }

void RWMutex::WriteUnlock() { uv_rwlock_wrunlock(&mu_); }

CondVar::CondVar(Mutex* mu) : mu_(mu) { uv_cond_init(&cv_); }

CondVar::~CondVar() { uv_cond_destroy(&cv_); }

void CondVar::Wait() { uv_cond_wait(&cv_, &mu_->mu_); }

bool CondVar::TimedWait(uint64_t abs_time_us) { return ETIMEDOUT == uv_cond_timedwait(&cv_, &mu_->mu_, abs_time_us); }

void CondVar::Signal() { uv_cond_signal(&cv_); }

void CondVar::SignalAll() { uv_cond_broadcast(&cv_); }

void InitOnce(OnceType* once, void (*initializer)()) { uv_once(once, initializer); }

}  // namespace port
}  // namespace leveldb
