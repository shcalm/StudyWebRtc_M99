/*
 *  Copyright (c) 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_RTP_RTCP_INCLUDE_RTP_PACKET_SENDER_H_
#define MODULES_RTP_RTCP_INCLUDE_RTP_PACKET_SENDER_H_

#include <memory>
#include <vector>

#include "modules/rtp_rtcp/include/rtp_rtcp_defines.h"
#include "modules/rtp_rtcp/source/rtp_packet_to_send.h"

namespace webrtc {

class RtpPacketSender {
 public:
 //hua2 c++11 I think should equals virtual ~RtpPacketSender(){};
  virtual ~RtpPacketSender() = default;

  // Insert a set of packets into queue, for eventual transmission. Based on the
  // type of packets, they will be prioritized and scheduled relative to other
  // packets and the current target send rate.
  //hua2 rtpSender enqueue package to pacer
  virtual void EnqueuePackets(
      std::vector<std::unique_ptr<RtpPacketToSend>> packets) = 0;
};

}  // namespace webrtc

#endif  // MODULES_RTP_RTCP_INCLUDE_RTP_PACKET_SENDER_H_
