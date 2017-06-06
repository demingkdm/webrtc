/*
 *  Copyright 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// This file contains interfaces for RtpReceivers
// http://w3c.github.io/webrtc-pc/#rtcrtpreceiver-interface

#ifndef WEBRTC_API_RTPRECEIVERINTERFACE_H_
#define WEBRTC_API_RTPRECEIVERINTERFACE_H_

#include <string>
#include <vector>

#include "webrtc/api/mediatypes.h"
#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/api/proxy.h"
#include "webrtc/api/rtpparameters.h"
#include "webrtc/base/refcount.h"
#include "webrtc/base/scoped_ref_ptr.h"

namespace webrtc {

enum class RtpSourceType {
  SSRC,
  CSRC,
};

class RtpSource {
 public:
  RtpSource() = delete;
  RtpSource(int64_t timestamp_ms, uint32_t source_id, RtpSourceType source_type)
      : timestamp_ms_(timestamp_ms),
        source_id_(source_id),
        source_type_(source_type) {}

  int64_t timestamp_ms() const { return timestamp_ms_; }
  void update_timestamp_ms(int64_t timestamp_ms) {
    RTC_DCHECK_LE(timestamp_ms_, timestamp_ms);
    timestamp_ms_ = timestamp_ms;
  }

  // The identifier of the source can be the CSRC or the SSRC.
  uint32_t source_id() const { return source_id_; }

  // The source can be either a contributing source or a synchronization source.
  RtpSourceType source_type() const { return source_type_; }

  // This isn't implemented yet and will always return an empty Optional.
  // TODO(zhihuang): Implement this to return real audio level.
  rtc::Optional<int8_t> audio_level() const { return rtc::Optional<int8_t>(); }

 private:
  int64_t timestamp_ms_;
  uint32_t source_id_;
  RtpSourceType source_type_;
};

class RtpReceiverObserverInterface {
 public:
  // Note: Currently if there are multiple RtpReceivers of the same media type,
  // they will all call OnFirstPacketReceived at once.
  //
  // In the future, it's likely that an RtpReceiver will only call
  // OnFirstPacketReceived when a packet is received specifically for its
  // SSRC/mid.
  virtual void OnFirstPacketReceived(cricket::MediaType media_type) = 0;

 protected:
  virtual ~RtpReceiverObserverInterface() {}
};

class RtpReceiverInterface : public rtc::RefCountInterface {
 public:
  virtual rtc::scoped_refptr<MediaStreamTrackInterface> track() const = 0;

  // Audio or video receiver?
  virtual cricket::MediaType media_type() const = 0;

  // Not to be confused with "mid", this is a field we can temporarily use
  // to uniquely identify a receiver until we implement Unified Plan SDP.
  virtual std::string id() const = 0;

  // The WebRTC specification only defines RTCRtpParameters in terms of senders,
  // but this API also applies them to receivers, similar to ORTC:
  // http://ortc.org/wp-content/uploads/2016/03/ortc.html#rtcrtpparameters*.
  virtual RtpParameters GetParameters() const = 0;
  // Currently, doesn't support changing any parameters, but may in the future.
  virtual bool SetParameters(const RtpParameters& parameters) = 0;

  // Does not take ownership of observer.
  // Must call SetObserver(nullptr) before the observer is destroyed.
  virtual void SetObserver(RtpReceiverObserverInterface* observer) = 0;

  // TODO(zhihuang): Remove the default implementation once the subclasses
  // implement this. Currently, the only relevant subclass is the
  // content::FakeRtpReceiver in Chromium.
  virtual std::vector<RtpSource> GetSources() const {
    return std::vector<RtpSource>();
  }

 protected:
  virtual ~RtpReceiverInterface() {}
};

// Define proxy for RtpReceiverInterface.
// TODO(deadbeef): Move this to .cc file and out of api/. What threads methods
// are called on is an implementation detail.
BEGIN_SIGNALING_PROXY_MAP(RtpReceiver)
  PROXY_SIGNALING_THREAD_DESTRUCTOR()
  PROXY_CONSTMETHOD0(rtc::scoped_refptr<MediaStreamTrackInterface>, track)
  PROXY_CONSTMETHOD0(cricket::MediaType, media_type)
  PROXY_CONSTMETHOD0(std::string, id)
  PROXY_CONSTMETHOD0(RtpParameters, GetParameters);
  PROXY_METHOD1(bool, SetParameters, const RtpParameters&)
  PROXY_METHOD1(void, SetObserver, RtpReceiverObserverInterface*);
  PROXY_CONSTMETHOD0(std::vector<RtpSource>, GetSources);
  END_PROXY_MAP()

}  // namespace webrtc

#endif  // WEBRTC_API_RTPRECEIVERINTERFACE_H_