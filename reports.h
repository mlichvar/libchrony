/*
 * Copyright (C) 2023  Miroslav Lichvar
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

static const Constant leap_enums[] = {
	{ 0, "normal" },
	{ 1, "insert second" },
	{ 2, "delete second" },
	{ 3, "not synchronized" },
	{ 0 }
};

static const Field tracking_report_fields[] = {
	{ "reference ID", TYPE_UINT32, CHRONY_CONTENT_REFERENCE_ID },
	{ "address", TYPE_ADDRESS, CHRONY_CONTENT_ADDRESS },
	{ "stratum", TYPE_UINT16, CHRONY_CONTENT_COUNT },
	{ "leap status", TYPE_UINT16, CHRONY_CONTENT_ENUM, leap_enums },
	{ "reference time", TYPE_TIMESPEC, CHRONY_CONTENT_TIME },
	{ "current correction", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_SECONDS },
	{ "last offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_SECONDS },
	{ "RMS offset", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "frequency offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_PPM },
	{ "residual frequency", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_PPM },
	{ "skew", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_PPM },
	{ "root delay", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "root dispersion", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "last update interval", TYPE_FLOAT, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ NULL }
};

static const Field num_sources_fields[] = {
	{ "sources", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ NULL }
};

static const Field request_by_index_fields[] = {
	{ "index", TYPE_UINT32, CHRONY_CONTENT_INDEX },
	{ NULL }
};

static const Constant sources_state_enums[] = {
	{ 0, "selected" },
	{ 1, "nonselectable" },
	{ 2, "falseticker" },
	{ 3, "jittery" },
	{ 4, "unselected" },
	{ 5, "selectable" },
	{ 0 }
};

static const Constant sources_mode_enums[] = {
	{ 0, "client" },
	{ 1, "peer" },
	{ 2, "reference clock" },
	{ 0 }
};

static const Field sources_report_fields[] = {
	{ "address\0reference ID", TYPE_ADDRESS_OR_UINT32_IN_ADDRESS, CHRONY_CONTENT_NONE },
	{ "poll", TYPE_INT16, CHRONY_CONTENT_INTERVAL_LOG2_SECONDS },
	{ "stratum", TYPE_UINT16, CHRONY_CONTENT_COUNT },
	{ "state", TYPE_UINT16, CHRONY_CONTENT_ENUM, sources_state_enums },
	{ "mode", TYPE_UINT16, CHRONY_CONTENT_ENUM, sources_mode_enums },
	{ "flags", TYPE_UINT16, CHRONY_CONTENT_NONE },
	{ "reachability", TYPE_UINT16, CHRONY_CONTENT_BITS },
	{ "last sample ago", TYPE_UINT32, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ "original last sample offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_SECONDS },
	{ "adjusted last sample offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_SECONDS },
	{ "last sample error", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ NULL }
};

static const Field sourcestats_report_fields[] = {
	{ "reference ID", TYPE_UINT32, CHRONY_CONTENT_REFERENCE_ID },
	{ "address", TYPE_ADDRESS, CHRONY_CONTENT_ADDRESS },
	{ "samples", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "runs", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "span", TYPE_UINT32, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ "standard deviation", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "residual frequency", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_PPM },
	{ "skew", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_PPM },
	{ "offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_SECONDS },
	{ "offset error", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ NULL }
};

static const Constant selectdata_state_enums[] = {
	{ 'N', "ignored" },
	{ 's', "not synchronized" },
	{ 'M', "missing samples" },
	{ 'd', "unacceptable distance" },
	{ 'D', "large distance" },
	{ '~', "jittery" },
	{ 'w', "waiting for others" },
	{ 'W', "missing selectable sources" },
	{ 'S', "stale" },
	{ 'O', "orphan" },
	{ 'T', "not trusted" },
	{ 'P', "not preferred" },
	{ 'U', "waiting for update" },
	{ 'x', "falseticker" },
	{ '+', "combined" },
	{ '*', "best" },
	{ 0 }
};

static const Constant selectdata_option_flags[] = {
	{ 0x1, "noselect" },
	{ 0x2, "prefer" },
	{ 0x4, "trust" },
	{ 0x8, "require" },
	{ 0 }
};

static const Field selectdata_report_fields[] = {
	{ "reference ID", TYPE_UINT32, CHRONY_CONTENT_REFERENCE_ID },
	{ "address", TYPE_ADDRESS, CHRONY_CONTENT_ADDRESS },
	{ "state", TYPE_UINT8, CHRONY_CONTENT_ENUM, selectdata_state_enums },
	{ "authentication", TYPE_UINT8, CHRONY_CONTENT_BOOLEAN },
	{ "leap status", TYPE_UINT8, CHRONY_CONTENT_ENUM, leap_enums },
	{ "reserved #1", TYPE_UINT8, CHRONY_CONTENT_NONE },
	{ "configured options", TYPE_UINT16, CHRONY_CONTENT_FLAGS, selectdata_option_flags },
	{ "effective options", TYPE_UINT16, CHRONY_CONTENT_FLAGS, selectdata_option_flags },
	{ "last sample ago", TYPE_UINT32, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ "score", TYPE_FLOAT, CHRONY_CONTENT_RATIO },
	{ "low limit", TYPE_FLOAT, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ "high limit", TYPE_FLOAT, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ NULL }
};

static const Field activity_report_fields[] = {
	{ "online sources", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "offline sources", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "burst online-return sources", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "burst offline-return sources", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "unresolved sources", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ NULL }
};

static const Field request_by_address_fields[] = {
	{ "address", TYPE_ADDRESS, CHRONY_CONTENT_ADDRESS },
	{ NULL }
};

static const Constant authdata_mode_enums[] = {
	{ 0, "none" },
	{ 1, "symmetric key" },
	{ 2, "NTS" },
	{ 0 }
};

static const Constant authdata_keytype_enums[] = {
	{ 1, "MD5" },
	{ 2, "SHA1" },
	{ 3, "SHA256" },
	{ 4, "SHA384" },
	{ 5, "SHA512" },
	{ 6, "SHA3-224" },
	{ 7, "SHA3-256" },
	{ 8, "SHA3-384" },
	{ 9, "SHA3-512" },
	{ 10, "TIGER" },
	{ 11, "WHIRLPOOL" },
	{ 13, "AES128" },
	{ 14, "AES256" },
	{ 15, "AEAD-AES-SIV-CMAC-256" },
	{ 30, "AEAD-AES-128-GCM-SIV" },
	{ 0 }
};

static const Field authdata_report_fields[] = {
	{ "mode", TYPE_UINT16, CHRONY_CONTENT_ENUM, authdata_mode_enums },
	{ "key type", TYPE_UINT16, CHRONY_CONTENT_ENUM, authdata_keytype_enums },
	{ "key ID", TYPE_UINT32, CHRONY_CONTENT_INDEX },
	{ "key length", TYPE_UINT16, CHRONY_CONTENT_LENGTH_BITS },
	{ "key establishment attempts", TYPE_UINT16, CHRONY_CONTENT_COUNT },
	{ "last key establishment ago", TYPE_UINT32, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ "cookies", TYPE_UINT16, CHRONY_CONTENT_COUNT },
	{ "cookie length", TYPE_UINT16, CHRONY_CONTENT_LENGTH_BYTES },
	{ "NAK", TYPE_UINT16, CHRONY_CONTENT_BOOLEAN },
	{ "reserved #1", TYPE_UINT16, CHRONY_CONTENT_NONE },
	{ NULL }
};

static const Constant ntp_mode_enums[] = {
	{ 1, "active symmetric" },
	{ 2, "passive symmetric" },
	{ 4, "server" },
	{ 0 }
};

static const Constant ntp_timestamping_enums[] = {
	{ 'D', "daemon" },
	{ 'K', "kernel" },
	{ 'H', "hardware" },
	{ 0 }
};

static const Constant ntp_flags[] = {
	{ 0x200, "test1" },
	{ 0x100, "test2" },
	{ 0x80, "test3" },
	{ 0x40, "test5" },
	{ 0x20, "test6" },
	{ 0x10, "test7" },
	{ 0x8, "testA" },
	{ 0x4, "testC" },
	{ 0x2, "testB" },
	{ 0x1, "testD" },
	{ 0x4000, "interleaved" },
	{ 0x8000, "authenticated" },
	{ 0 }
};

static const Field ntpdata_report_fields[] = {
	{ "remote address", TYPE_ADDRESS, CHRONY_CONTENT_ADDRESS },
	{ "local address", TYPE_ADDRESS, CHRONY_CONTENT_ADDRESS },
	{ "remote port", TYPE_UINT16, CHRONY_CONTENT_PORT },
	{ "leap status", TYPE_UINT8, CHRONY_CONTENT_ENUM, leap_enums },
	{ "version", TYPE_UINT8, CHRONY_CONTENT_COUNT },
	{ "mode", TYPE_UINT8, CHRONY_CONTENT_ENUM, ntp_mode_enums },
	{ "stratum", TYPE_UINT8, CHRONY_CONTENT_COUNT },
	{ "poll", TYPE_INT8, CHRONY_CONTENT_INTERVAL_LOG2_SECONDS },
	{ "precision", TYPE_INT8, CHRONY_CONTENT_INTERVAL_LOG2_SECONDS },
	{ "root delay", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "root dispersion", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "reference ID", TYPE_UINT32, CHRONY_CONTENT_REFERENCE_ID },
	{ "reference time", TYPE_TIMESPEC, CHRONY_CONTENT_TIME },
	{ "offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_SECONDS },
	{ "peer delay", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "peer dispersion", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "response time", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "jitter asymmetry", TYPE_FLOAT, CHRONY_CONTENT_RATIO },
	{ "flags", TYPE_UINT16, CHRONY_CONTENT_FLAGS, ntp_flags },
	{ "transmit timestamping", TYPE_UINT8, CHRONY_CONTENT_ENUM, ntp_timestamping_enums },
	{ "receive timestamping", TYPE_UINT8, CHRONY_CONTENT_ENUM, ntp_timestamping_enums },
	{ "transmitted messages", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "received messages", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "received valid messages", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "received good messages", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "reserved #1", TYPE_UINT32, CHRONY_CONTENT_NONE },
	{ "reserved #2", TYPE_UINT32, CHRONY_CONTENT_NONE },
	{ "reserved #3", TYPE_UINT32, CHRONY_CONTENT_NONE },
	{ NULL }
};

static const Field ntpdata2_report_fields[] = {
	{ "remote address", TYPE_ADDRESS, CHRONY_CONTENT_ADDRESS },
	{ "local address", TYPE_ADDRESS, CHRONY_CONTENT_ADDRESS },
	{ "remote port", TYPE_UINT16, CHRONY_CONTENT_PORT },
	{ "leap status", TYPE_UINT8, CHRONY_CONTENT_ENUM, leap_enums },
	{ "version", TYPE_UINT8, CHRONY_CONTENT_COUNT },
	{ "mode", TYPE_UINT8, CHRONY_CONTENT_ENUM, ntp_mode_enums },
	{ "stratum", TYPE_UINT8, CHRONY_CONTENT_COUNT },
	{ "poll", TYPE_INT8, CHRONY_CONTENT_INTERVAL_LOG2_SECONDS },
	{ "precision", TYPE_INT8, CHRONY_CONTENT_INTERVAL_LOG2_SECONDS },
	{ "root delay", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "root dispersion", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "reference ID", TYPE_UINT32, CHRONY_CONTENT_REFERENCE_ID },
	{ "reference time", TYPE_TIMESPEC, CHRONY_CONTENT_TIME },
	{ "offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_SECONDS },
	{ "peer delay", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "peer dispersion", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "response time", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "jitter asymmetry", TYPE_FLOAT, CHRONY_CONTENT_RATIO },
	{ "flags", TYPE_UINT16, CHRONY_CONTENT_FLAGS, ntp_flags },
	{ "transmit timestamping", TYPE_UINT8, CHRONY_CONTENT_ENUM, ntp_timestamping_enums },
	{ "receive timestamping", TYPE_UINT8, CHRONY_CONTENT_ENUM, ntp_timestamping_enums },
	{ "transmitted messages", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "received messages", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "received valid messages", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "received good messages", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "kernel transmit timestamps", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "kernel receive timestamps", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "hardware transmit timestamps", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "hardware receive timestamps", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "reserved #1", TYPE_UINT32, CHRONY_CONTENT_NONE },
	{ "reserved #2", TYPE_UINT32, CHRONY_CONTENT_NONE },
	{ "reserved #3", TYPE_UINT32, CHRONY_CONTENT_NONE },
	{ "reserved #4", TYPE_UINT32, CHRONY_CONTENT_NONE },
	{ NULL }
};

static const Field serverstats_report_fields[] = {
	{ "received NTP requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "received command requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "dropped NTP requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "dropped command requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "dropped client log records", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ NULL }
};

static const Field serverstats2_report_fields[] = {
	{ "received NTP requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "accepted NTS-KE connections", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "received command requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "dropped NTP requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "dropped NTS-KE connections", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "dropped command requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "dropped client log records", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "received authenticated NTP requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ NULL }
};

static const Field serverstats3_report_fields[] = {
	{ "received NTP requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "accepted NTS-KE connections", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "received command requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "dropped NTP requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "dropped NTS-KE connections", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "dropped command requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "dropped client log records", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "received authenticated NTP requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "received interleaved NTP requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "held NTP timestamps", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "NTP timestamp span", TYPE_UINT32, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ NULL }
};

static const Field serverstats4_report_fields[] = {
	{ "received NTP requests", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "accepted NTS-KE connections", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "received command requests", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "dropped NTP requests", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "dropped NTS-KE connections", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "dropped command requests", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "dropped client log records", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "received authenticated NTP requests", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "received interleaved NTP requests", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "held NTP timestamps", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "NTP timestamp span", TYPE_UINT64, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ "served daemon RX timestamps", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "served daemon TX timestamps", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "served kernel RX timestamps", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "served kernel TX timestamps", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "served hardware RX timestamps", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "served hardware TX timestamps", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "reserved #1", TYPE_UINT64, CHRONY_CONTENT_NONE },
	{ "reserved #2", TYPE_UINT64, CHRONY_CONTENT_NONE },
	{ "reserved #3", TYPE_UINT64, CHRONY_CONTENT_NONE },
	{ "reserved #4", TYPE_UINT64, CHRONY_CONTENT_NONE },
	{ NULL }
};

static const Field rtcdata_report_fields[] = {
	{ "reference time", TYPE_TIMESPEC, CHRONY_CONTENT_TIME },
	{ "samples", TYPE_UINT16, CHRONY_CONTENT_COUNT },
	{ "runs", TYPE_UINT16, CHRONY_CONTENT_COUNT },
	{ "span", TYPE_UINT32, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ "offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_SECONDS },
	{ "frequency offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_PPM },
	{ NULL }
};

static const Constant smoothing_flags[] = {
	{ 0x1, "active" },
	{ 0x2, "leaponly" },
	{ 0 }
};

static const Field smoothing_report_fields[] = {
	{ "flags", TYPE_UINT32, CHRONY_CONTENT_FLAGS, smoothing_flags },
	{ "offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_SECONDS },
	{ "frequency offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_PPM },
	{ "wander", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_PPM_PER_SECOND },
	{ "last update ago", TYPE_FLOAT, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ "remaining time", TYPE_FLOAT, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ NULL }
};

static const Report reports[] = {
	{
		.name = "tracking",
		.record_requests = { { 33 }, },
		.record_responses = { { 5, tracking_report_fields }, }
	},
	{
		.name = "sources",
		.count_requests = { { 14 }, },
		.count_responses = { { 2, num_sources_fields }, },
		.record_requests = { { 15, request_by_index_fields }, },
		.record_responses = { { 3, sources_report_fields }, },
	},
	{
		.name = "sourcestats",
		.count_requests = { { 14 }, },
		.count_responses = { { 2, num_sources_fields }, },
		.record_requests = { { 34, request_by_index_fields }, },
		.record_responses = { { 6, sourcestats_report_fields }, },
	},
	{
		.name = "selectdata",
		.count_requests = { { 14 }, },
		.count_responses = { { 2, num_sources_fields }, },
		.record_requests = { { 69, request_by_index_fields }, },
		.record_responses = { { 23, selectdata_report_fields }, },
	},
	{
		.name = "activity",
		.record_requests = { { 44 }, },
		.record_responses = { { 12, activity_report_fields }, }
	},
	{
		.name = "authdata",
		.count_requests = { { 14 }, },
		.count_responses = { { 2, num_sources_fields }, },
		.record_requests = { { 67, request_by_address_fields }, },
		.record_responses = { { 20, authdata_report_fields }, }
	},
	{
		.name = "ntpdata",
		.count_requests = { { 14 }, },
		.count_responses = { { 2, num_sources_fields }, },
		.record_requests = { { 57, request_by_address_fields }, },
		.record_responses = { { 16, ntpdata_report_fields },
				      { 26, ntpdata2_report_fields }, }
	},
	{
		.name = "serverstats",
		.record_requests = { { 54 }, },
		.record_responses = { { 14, serverstats_report_fields },
				      { 22, serverstats2_report_fields },
				      { 24, serverstats3_report_fields },
				      { 25, serverstats4_report_fields }, }
	},
	{
		.name = "rtcdata",
		.record_requests = { { 35 }, },
		.record_responses = { { 7, rtcdata_report_fields }, }
	},
	{
		.name = "smoothing",
		.record_requests = { { 51 }, },
		.record_responses = { { 13, smoothing_report_fields }, }
	},
};
