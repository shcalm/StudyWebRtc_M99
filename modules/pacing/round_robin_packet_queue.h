/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_PACING_ROUND_ROBIN_PACKET_QUEUE_H_
#define MODULES_PACING_ROUND_ROBIN_PACKET_QUEUE_H_

#include <stddef.h>
#include <stdint.h>

#include <list>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <unordered_map>

#include "absl/types/optional.h"
#include "api/transport/webrtc_key_value_config.h"
#include "api/units/data_size.h"
#include "api/units/time_delta.h"
#include "api/units/timestamp.h"
#include "modules/rtp_rtcp/include/rtp_rtcp_defines.h"
#include "modules/rtp_rtcp/source/rtp_packet_to_send.h"
#include "system_wrappers/include/clock.h"

namespace webrtc {

class RoundRobinPacketQueue {
 public:
  RoundRobinPacketQueue(Timestamp start_time,
                        const WebRtcKeyValueConfig* field_trials);
  ~RoundRobinPacketQueue();
//hua2 插入不同优先级的报文
  void Push(int priority,
            Timestamp enqueue_time,//入队时间
            uint64_t enqueue_order,//入队次序，priority相同时候，比较order
            std::unique_ptr<RtpPacketToSend> packet);
//hua2 pop
  std::unique_ptr<RtpPacketToSend> Pop();

  bool Empty() const;
//hua2 报文的总个数
  size_t SizeInPackets() const;
//hua2 报文的字节数
  DataSize Size() const;
  // If the next packet, that would be returned by Pop() if called
  // now, is an audio packet this method returns the enqueue time
  // of that packet. If queue is empty or top packet is not audio,
  // returns nullopt.
  //hua2下一个pop的报文如果是音频包，则返回该包时间戳，非音频包则返回nullopt
  absl::optional<Timestamp> LeadingAudioPacketEnqueueTime() const;
//hua2 队列中最旧的报文时间戳
  Timestamp OldestEnqueueTime() const;
//hua2 TODO
  TimeDelta AverageQueueTime() const;
  void UpdateQueueTime(Timestamp now);
  void SetPauseState(bool paused, Timestamp now);
  void SetIncludeOverhead();
  void SetTransportOverhead(DataSize overhead_per_packet);

 private:
  struct QueuedPacket {
   public:
    QueuedPacket(int priority,
                 Timestamp enqueue_time,
                 uint64_t enqueue_order,
                 std::multiset<Timestamp>::iterator enqueue_time_it,
                 std::unique_ptr<RtpPacketToSend> packet);
    QueuedPacket(const QueuedPacket& rhs);
    ~QueuedPacket();

    bool operator<(const QueuedPacket& other) const;

    int Priority() const;
    RtpPacketMediaType Type() const;
    uint32_t Ssrc() const;
    Timestamp EnqueueTime() const;
    bool IsRetransmission() const;
    uint64_t EnqueueOrder() const;
    RtpPacketToSend* RtpPacket() const;

    std::multiset<Timestamp>::iterator EnqueueTimeIterator() const;
    void UpdateEnqueueTimeIterator(std::multiset<Timestamp>::iterator it);
    void SubtractPauseTime(TimeDelta pause_time_sum);

   private:
    int priority_;
    Timestamp enqueue_time_;  // Absolute time of pacer queue entry.
    uint64_t enqueue_order_;
    bool is_retransmission_;  // Cached for performance.
    std::multiset<Timestamp>::iterator enqueue_time_it_;
    // Raw pointer since priority_queue doesn't allow for moving
    // out of the container.
    RtpPacketToSend* owned_packet_;
  };

  class PriorityPacketQueue : public std::priority_queue<QueuedPacket> {
   public:
    using const_iterator = container_type::const_iterator;
    const_iterator begin() const;
    const_iterator end() const;
  };
  //hua2 首先比较优先级，然后比较size大小
  struct StreamPrioKey {
    StreamPrioKey(int priority, DataSize size)
        : priority(priority), size(size) {}

    bool operator<(const StreamPrioKey& other) const {
      if (priority != other.priority)
        return priority < other.priority;
      return size < other.size;
    }

    const int priority;
    const DataSize size;
  };

  struct Stream {
    Stream();
    Stream(const Stream&);

    virtual ~Stream();
//hua2 已经发送的报文总大小，发送少的优先被调度
    DataSize size;
    uint32_t ssrc;
//hua2 一个根据StreamPrioKey确定优先级的优先队列
    PriorityPacketQueue packet_queue;

    // Whenever a packet is inserted for this stream we check if `priority_it`
    // points to an element in `stream_priorities_`, and if it does it means
    // this stream has already been scheduled, and if the scheduled priority is
    // lower than the priority of the incoming packet we reschedule this stream
    // with the higher priority.
    std::multimap<StreamPrioKey, uint32_t>::iterator priority_it;
  };

  void Push(QueuedPacket packet);

  DataSize PacketSize(const QueuedPacket& packet) const;
  void MaybePromoteSinglePacketToNormalQueue();

  Stream* GetHighestPriorityStream();

  // Just used to verify correctness.
  bool IsSsrcScheduled(uint32_t ssrc) const;

  DataSize transport_overhead_per_packet_;

  Timestamp time_last_updated_;

  bool paused_;
  size_t size_packets_;
  DataSize size_;
  DataSize max_size_;
  TimeDelta queue_time_sum_;//hua2 所有包的入队时间总和
  TimeDelta pause_time_sum_;//hua2 pause时间总和

  // A map of streams used to prioritize from which stream to send next. We use
  // a multimap instead of a priority_queue since the priority of a stream can
  // change as a new packet is inserted, and a multimap allows us to remove and
  // then reinsert a StreamPrioKey if the priority has increased.
  std::multimap<StreamPrioKey, uint32_t> stream_priorities_;//hua2 streampriokey->ssrc

  // A map of SSRCs to Streams.
  std::unordered_map<uint32_t, Stream> streams_;//hua2 ssrc-> stream

  // The enqueue time of every packet currently in the queue. Used to figure out
  // the age of the oldest packet in the queue.
  std::multiset<Timestamp> enqueue_times_;

  absl::optional<QueuedPacket> single_packet_queue_;

  bool include_overhead_;
};
}  // namespace webrtc

#endif  // MODULES_PACING_ROUND_ROBIN_PACKET_QUEUE_H_
