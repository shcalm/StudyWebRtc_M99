/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_PACING_INTERVAL_BUDGET_H_
#define MODULES_PACING_INTERVAL_BUDGET_H_

#include <stddef.h>
#include <stdint.h>

namespace webrtc {

// TODO(tschumim): Reflector IntervalBudget so that we can set a under- and
// over-use budget in ms.
//hua2 主要在周期模式中使用。interval内最大可使用的数据量
class IntervalBudget {
 public:
  explicit IntervalBudget(int initial_target_rate_kbps);//hua2 不能进行隐式转换 explicit
  IntervalBudget(int initial_target_rate_kbps, bool can_build_up_underuse);
  void set_target_rate_kbps(int target_rate_kbps);//hua2 设置目标码率

  // TODO(tschumim): Unify IncreaseBudget and UseBudget to one function.
  void IncreaseBudget(int64_t delta_time_ms);//hua2 增加delta的预算
  void UseBudget(size_t bytes);//hua2 减少预算

  size_t bytes_remaining() const;//hua2 还有多少预算可用
  double budget_ratio() const;//hua2 预算比率
  int target_rate_kbps() const; //hua2 目标码率

 private:
  int target_rate_kbps_;
  int64_t max_bytes_in_budget_;
  int64_t bytes_remaining_;
  bool can_build_up_underuse_;//hua2 是否可以把前一个interval的预算接着用
};

}  // namespace webrtc

#endif  // MODULES_PACING_INTERVAL_BUDGET_H_
