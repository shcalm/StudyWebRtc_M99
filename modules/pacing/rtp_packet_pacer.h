/*
 *  Copyright (c) 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_PACING_RTP_PACKET_PACER_H_
#define MODULES_PACING_RTP_PACKET_PACER_H_

#include <stdint.h>

#include "absl/types/optional.h"
#include "api/units/data_rate.h"
#include "api/units/data_size.h"
#include "api/units/time_delta.h"
#include "api/units/timestamp.h"
#include "modules/rtp_rtcp/include/rtp_packet_sender.h"
//hua2 interface for paced_sender
namespace webrtc {

class RtpPacketPacer {
 public:
  virtual ~RtpPacketPacer() = default;

  virtual void CreateProbeCluster(DataRate bitrate, int cluster_id) = 0;

  // Temporarily pause all sending.
  virtual void Pause() = 0;

  // Resume sending packets.
  virtual void Resume() = 0;
  //hua2 congestion 
  virtual void SetCongestionWindow(DataSize congestion_window_size) = 0;
  virtual void UpdateOutstandingData(DataSize outstanding_data) = 0;

  // Sets the pacing rates. Must be called once before packets can be sent.
  //hua2 设置当前码率
  virtual void SetPacingRates(DataRate pacing_rate, DataRate padding_rate) = 0;

  // Time since the oldest packet currently in the queue was added.
  //hua2 最老的包的距离now的time delta
  virtual TimeDelta OldestPacketWaitTime() const = 0;

  // Sum of payload + padding bytes of all packets currently in the pacer queue.
  //hua2 所有包的大小
  virtual DataSize QueueSizeData() const = 0;

  // Returns the time when the first packet was sent.
  //hua2 第一个包的发送时间
  virtual absl::optional<Timestamp> FirstSentPacketTime() const = 0;

  // Returns the expected number of milliseconds it will take to send the
  // current packets in the queue, given the current size and bitrate, ignoring
  // priority.
  //hua2 当前队列的包的预期发送时间，就是包的大小 除以 码率
  virtual TimeDelta ExpectedQueueTime() const = 0;

  // Set the average upper bound on pacer queuing delay. The pacer may send at
  // a higher rate than what was configured via SetPacingRates() in order to
  // keep ExpectedQueueTimeMs() below `limit_ms` on average.
  //hua2 pacer发送的最大时延，默认是2s
  virtual void SetQueueTimeLimit(TimeDelta limit) = 0;

  // Currently audio traffic is not accounted by pacer and passed through.
  // With the introduction of audio BWE audio traffic will be accounted for
  // the pacer budget calculation. The audio traffic still will be injected
  // at high priority.
  //hua2 audio数据是否包含在pacer的时间计算中
  virtual void SetAccountForAudioPackets(bool account_for_audio) = 0;
  //hua2 rtp的header是否包含在总数据量的计算中
  virtual void SetIncludeOverhead() = 0;
  //hua2 其他包头是否包含
  virtual void SetTransportOverhead(DataSize overhead_per_packet) = 0;
};

}  // namespace webrtc
#endif  // MODULES_PACING_RTP_PACKET_PACER_H_
