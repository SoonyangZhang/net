// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "logging.h"
#include "base_alarm.h"
namespace basic {

BaseAlarm::BaseAlarm(std::unique_ptr<Delegate> delegate)
    : delegate_(std::move(delegate)), deadline_(QuicTime::Zero()) {}

BaseAlarm::~BaseAlarm() {}

void BaseAlarm::Set(QuicTime new_deadline) {
  DCHECK(!IsSet());
  DCHECK(new_deadline.IsInitialized());
  deadline_ = new_deadline;
  SetImpl();
}

void BaseAlarm::Cancel() {
  if (!IsSet()) {
    // Don't try to cancel an alarm that hasn't been set.
    return;
  }
  deadline_ = QuicTime::Zero();
  CancelImpl();
}

void BaseAlarm::Update(QuicTime new_deadline, QuicTime::Delta granularity) {
  if (!new_deadline.IsInitialized()) {
    Cancel();
    return;
  }
  if (std::abs((new_deadline - deadline_).ToMicroseconds()) <
      granularity.ToMicroseconds()) {
    return;
  }
  const bool was_set = IsSet();
  deadline_ = new_deadline;
  if (was_set) {
    UpdateImpl();
  } else {
    SetImpl();
  }
}

bool BaseAlarm::IsSet() const {
  return deadline_.IsInitialized();
}

void BaseAlarm::Fire() {
  if (!IsSet()) {
    return;
  }

  deadline_ = QuicTime::Zero();
  delegate_->OnAlarm();
}

void BaseAlarm::UpdateImpl() {
  // CancelImpl and SetImpl take the new deadline by way of the deadline_
  // member, so save and restore deadline_ before canceling.
  const QuicTime new_deadline = deadline_;

  deadline_ = QuicTime::Zero();
  CancelImpl();

  deadline_ = new_deadline;
  SetImpl();
}

}