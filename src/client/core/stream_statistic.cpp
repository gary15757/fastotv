/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

    This file is part of FastoTV.

    FastoTV is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FastoTV is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FastoTV. If not, see <http://www.gnu.org/licenses/>.
*/

#include "client/core/stream_statistic.h"

#define UNKNOWN_STREAM_TEXT "   "
#define ONLY_VIDEO_TEXT "M-V"
#define ONLY_AUDIO_TEXT "M-A"
#define VIDEO_AUDIO_TEXT "A-V"

namespace fasto {
namespace fastotv {
namespace client {
namespace core {

Stats::Stats()
    : frame_drops_early(0),
      frame_drops_late(0),
      frame_processed(0),
      master_clock(0),
      audio_clock(0),
      video_clock(0),
      fmt(UNKNOWN_STREAM),
      audio_queue_size(0),
      video_queue_size(0),
      video_bandwidth(0),
      audio_bandwidth(0),
      start_ts_(common::time::current_mstime()) {}

clock_t Stats::GetDiffStreams() const {
  if (fmt == (HAVE_VIDEO_STREAM | HAVE_AUDIO_STREAM)) {
    return audio_clock - video_clock;
  } else if (fmt == HAVE_VIDEO_STREAM) {
    return master_clock - video_clock;
  } else if (fmt == HAVE_AUDIO_STREAM) {
    return master_clock - audio_clock;
  }

  return 0;
}

double Stats::GetFps() const {
  common::time64_t diff = common::time::current_mstime() - start_ts_;
  if (diff <= 0) {
    return 0.0;
  }

  double fps_per_msec = static_cast<double>(frame_processed) / diff;
  return fps_per_msec * 1000;
}

std::string ConvertStreamFormatToString(stream_foramat_t fmt) {
  if (fmt == (fasto::fastotv::client::core::HAVE_VIDEO_STREAM | fasto::fastotv::client::core::HAVE_AUDIO_STREAM)) {
    return VIDEO_AUDIO_TEXT;
  } else if (fmt == fasto::fastotv::client::core::HAVE_VIDEO_STREAM) {
    return ONLY_VIDEO_TEXT;
  } else if (fmt == fasto::fastotv::client::core::HAVE_AUDIO_STREAM) {
    return ONLY_AUDIO_TEXT;
  }

  return UNKNOWN_STREAM_TEXT;
}

}  // namespace core
}  // namespace client
}  // namespace fastotv
}  // namespace fasto
