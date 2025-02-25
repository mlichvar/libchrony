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
	{ 0, "Normal" },
	{ 1, "Insert second" },
	{ 2, "Delete second" },
	{ 3, "Not synchronized" },
	{ 0 }
};

static const Field tracking_report_fields[] = {
	{ "Reference ID", TYPE_UINT32, CHRONY_CONTENT_REFERENCE_ID },
	{ "Address", TYPE_ADDRESS, CHRONY_CONTENT_ADDRESS },
	{ "Stratum", TYPE_UINT16, CHRONY_CONTENT_COUNT },
	{ "Leap status", TYPE_UINT16, CHRONY_CONTENT_ENUM, leap_enums },
	{ "Reference time", TYPE_TIMESPEC, CHRONY_CONTENT_TIME },
	{ "Current correction", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_SECONDS },
	{ "Last offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_SECONDS },
	{ "RMS offset", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "Frequency offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_PPM },
	{ "Residual frequency", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_PPM },
	{ "Skew", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_PPM },
	{ "Root delay", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "Root dispersion", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "Last update interval", TYPE_FLOAT, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ NULL }
};

static const Field num_sources_fields[] = {
	{ "Sources", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ NULL }
};

static const Field request_by_index_fields[] = {
	{ "Index", TYPE_UINT32, CHRONY_CONTENT_INDEX },
	{ NULL }
};

static const Constant sources_state_enums[] = {
	{ 0, "Selected" },
	{ 1, "Nonselectable" },
	{ 2, "Falseticker" },
	{ 3, "Jittery" },
	{ 4, "Unselected" },
	{ 5, "Selectable" },
	{ 0 }
};

static const Constant sources_mode_enums[] = {
	{ 0, "Client" },
	{ 1, "Peer" },
	{ 2, "Reference clock" },
	{ 0 }
};

static const Field sources_report_fields[] = {
	{ "Address\0Reference ID", TYPE_ADDRESS_OR_UINT32_IN_ADDRESS, CHRONY_CONTENT_NONE },
	{ "Poll", TYPE_INT16, CHRONY_CONTENT_INTERVAL_LOG2_SECONDS },
	{ "Stratum", TYPE_UINT16, CHRONY_CONTENT_COUNT },
	{ "State", TYPE_UINT16, CHRONY_CONTENT_ENUM, sources_state_enums },
	{ "Mode", TYPE_UINT16, CHRONY_CONTENT_ENUM, sources_mode_enums },
	{ "Flags", TYPE_UINT16, CHRONY_CONTENT_NONE },
	{ "Reachability", TYPE_UINT16, CHRONY_CONTENT_BITS },
	{ "Last sample ago", TYPE_UINT32, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ "Original last sample offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_SECONDS },
	{ "Adjusted last sample offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_SECONDS },
	{ "Last sample error", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ NULL }
};

static const Field sourcestats_report_fields[] = {
	{ "Reference ID", TYPE_UINT32, CHRONY_CONTENT_REFERENCE_ID },
	{ "Address", TYPE_ADDRESS, CHRONY_CONTENT_ADDRESS },
	{ "Samples", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Runs", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Span", TYPE_UINT32, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ "Standard deviation", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "Residual frequency", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_PPM },
	{ "Skew", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_PPM },
	{ "Offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_SECONDS },
	{ "Offset error", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ NULL }
};

static const Constant selectdata_state_enums[] = {
	{ 'N', "Ignored" },
	{ 's', "Not synchronized" },
	{ 'M', "Missing samples" },
	{ 'd', "Unacceptable distance" },
	{ 'D', "Large distance" },
	{ '~', "Jittery" },
	{ 'w', "Waiting for others" },
	{ 'W', "Missing selectable sources" },
	{ 'S', "Stale" },
	{ 'O', "Orphan" },
	{ 'T', "Not trusted" },
	{ 'P', "Not preferred" },
	{ 'U', "Waiting for update" },
	{ 'x', "Falseticker" },
	{ '+', "Combined" },
	{ '*', "Best" },
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
	{ "Reference ID", TYPE_UINT32, CHRONY_CONTENT_REFERENCE_ID },
	{ "Address", TYPE_ADDRESS, CHRONY_CONTENT_ADDRESS },
	{ "State", TYPE_UINT8, CHRONY_CONTENT_ENUM, selectdata_state_enums },
	{ "Authentication", TYPE_UINT8, CHRONY_CONTENT_BOOLEAN },
	{ "Leap status", TYPE_UINT8, CHRONY_CONTENT_ENUM, leap_enums },
	{ "Reserved #1", TYPE_UINT8, CHRONY_CONTENT_NONE },
	{ "Configured options", TYPE_UINT16, CHRONY_CONTENT_FLAGS, selectdata_option_flags },
	{ "Effective options", TYPE_UINT16, CHRONY_CONTENT_FLAGS, selectdata_option_flags },
	{ "Last sample ago", TYPE_UINT32, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ "Score", TYPE_FLOAT, CHRONY_CONTENT_RATIO },
	{ "Low limit", TYPE_FLOAT, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ "High limit", TYPE_FLOAT, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ NULL }
};

static const Field activity_report_fields[] = {
	{ "Online sources", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Offline sources", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Burst online-return sources", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Burst offline-return sources", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Unresolved sources", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ NULL }
};

static const Field request_by_address_fields[] = {
	{ "Address", TYPE_ADDRESS, CHRONY_CONTENT_ADDRESS },
	{ NULL }
};

static const Constant authdata_mode_enums[] = {
	{ 0, "None" },
	{ 1, "Symmetric key" },
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
	{ "Mode", TYPE_UINT16, CHRONY_CONTENT_ENUM, authdata_mode_enums },
	{ "Key type", TYPE_UINT16, CHRONY_CONTENT_ENUM, authdata_keytype_enums },
	{ "Key ID", TYPE_UINT32, CHRONY_CONTENT_INDEX },
	{ "Key length", TYPE_UINT16, CHRONY_CONTENT_LENGTH_BITS },
	{ "Key establishment attempts", TYPE_UINT16, CHRONY_CONTENT_COUNT },
	{ "Last key establishment ago", TYPE_UINT32, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ "Cookies", TYPE_UINT16, CHRONY_CONTENT_COUNT },
	{ "Cookie length", TYPE_UINT16, CHRONY_CONTENT_LENGTH_BYTES },
	{ "NAK", TYPE_UINT16, CHRONY_CONTENT_BOOLEAN },
	{ "Reserved #1", TYPE_UINT16, CHRONY_CONTENT_NONE },
	{ NULL }
};

static const Constant ntp_mode_enums[] = {
	{ 1, "Active symmetric" },
	{ 2, "Passive symmetric" },
	{ 4, "Server" },
	{ 0 }
};

static const Constant ntp_timestamping_enums[] = {
	{ 'D', "Daemon" },
	{ 'K', "Kernel" },
	{ 'H', "Hardware" },
	{ 0 }
};

static const Constant ntp_flags[] = {
	{ 0x200, "Test1" },
	{ 0x100, "Test2" },
	{ 0x80, "Test3" },
	{ 0x40, "Test5" },
	{ 0x20, "Test6" },
	{ 0x10, "Test7" },
	{ 0x8, "TestA" },
	{ 0x4, "TestC" },
	{ 0x2, "TestB" },
	{ 0x1, "TestD" },
	{ 0x4000, "Interleaved" },
	{ 0x8000, "Authenticated" },
	{ 0 }
};

static const Field ntpdata_report_fields[] = {
	{ "Remote address", TYPE_ADDRESS, CHRONY_CONTENT_ADDRESS },
	{ "Local address", TYPE_ADDRESS, CHRONY_CONTENT_ADDRESS },
	{ "Remote port", TYPE_UINT16, CHRONY_CONTENT_PORT },
	{ "Leap status", TYPE_UINT8, CHRONY_CONTENT_ENUM, leap_enums },
	{ "Version", TYPE_UINT8, CHRONY_CONTENT_COUNT },
	{ "Mode", TYPE_UINT8, CHRONY_CONTENT_ENUM, ntp_mode_enums },
	{ "Stratum", TYPE_UINT8, CHRONY_CONTENT_COUNT },
	{ "Poll", TYPE_INT8, CHRONY_CONTENT_INTERVAL_LOG2_SECONDS },
	{ "Precision", TYPE_INT8, CHRONY_CONTENT_INTERVAL_LOG2_SECONDS },
	{ "Root delay", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "Root dispersion", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "Reference ID", TYPE_UINT32, CHRONY_CONTENT_REFERENCE_ID },
	{ "Reference time", TYPE_TIMESPEC, CHRONY_CONTENT_TIME },
	{ "Offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_SECONDS },
	{ "Peer delay", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "Peer dispersion", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "Response time", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "Jitter asymmetry", TYPE_FLOAT, CHRONY_CONTENT_RATIO },
	{ "Flags", TYPE_UINT16, CHRONY_CONTENT_FLAGS, ntp_flags },
	{ "Transmit timestamping", TYPE_UINT8, CHRONY_CONTENT_ENUM, ntp_timestamping_enums },
	{ "Receive timestamping", TYPE_UINT8, CHRONY_CONTENT_ENUM, ntp_timestamping_enums },
	{ "Transmitted messages", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Received messages", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Received valid messages", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Received good messages", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Reserved #1", TYPE_UINT32, CHRONY_CONTENT_NONE },
	{ "Reserved #2", TYPE_UINT32, CHRONY_CONTENT_NONE },
	{ "Reserved #3", TYPE_UINT32, CHRONY_CONTENT_NONE },
	{ NULL }
};

static const Field ntpdata2_report_fields[] = {
	{ "Remote address", TYPE_ADDRESS, CHRONY_CONTENT_ADDRESS },
	{ "Local address", TYPE_ADDRESS, CHRONY_CONTENT_ADDRESS },
	{ "Remote port", TYPE_UINT16, CHRONY_CONTENT_PORT },
	{ "Leap status", TYPE_UINT8, CHRONY_CONTENT_ENUM, leap_enums },
	{ "Version", TYPE_UINT8, CHRONY_CONTENT_COUNT },
	{ "Mode", TYPE_UINT8, CHRONY_CONTENT_ENUM, ntp_mode_enums },
	{ "Stratum", TYPE_UINT8, CHRONY_CONTENT_COUNT },
	{ "Poll", TYPE_INT8, CHRONY_CONTENT_INTERVAL_LOG2_SECONDS },
	{ "Precision", TYPE_INT8, CHRONY_CONTENT_INTERVAL_LOG2_SECONDS },
	{ "Root delay", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "Root dispersion", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "Reference ID", TYPE_UINT32, CHRONY_CONTENT_REFERENCE_ID },
	{ "Reference time", TYPE_TIMESPEC, CHRONY_CONTENT_TIME },
	{ "Offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_SECONDS },
	{ "Peer delay", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "Peer dispersion", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "Response time", TYPE_FLOAT, CHRONY_CONTENT_MEASURE_SECONDS },
	{ "Jitter asymmetry", TYPE_FLOAT, CHRONY_CONTENT_RATIO },
	{ "Flags", TYPE_UINT16, CHRONY_CONTENT_FLAGS, ntp_flags },
	{ "Transmit timestamping", TYPE_UINT8, CHRONY_CONTENT_ENUM, ntp_timestamping_enums },
	{ "Receive timestamping", TYPE_UINT8, CHRONY_CONTENT_ENUM, ntp_timestamping_enums },
	{ "Transmitted messages", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Received messages", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Received valid messages", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Received good messages", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Kernel transmit timestamps", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Kernel receive timestamps", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Hardware transmit timestamps", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Hardware receive timestamps", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Reserved #1", TYPE_UINT32, CHRONY_CONTENT_NONE },
	{ "Reserved #2", TYPE_UINT32, CHRONY_CONTENT_NONE },
	{ "Reserved #3", TYPE_UINT32, CHRONY_CONTENT_NONE },
	{ "Reserved #4", TYPE_UINT32, CHRONY_CONTENT_NONE },
	{ NULL }
};

static const Field serverstats_report_fields[] = {
	{ "Received NTP requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Received command requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Dropped NTP requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Dropped command requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Dropped client log records", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ NULL }
};

static const Field serverstats2_report_fields[] = {
	{ "Received NTP requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Accepted NTS-KE connections", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Received command requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Dropped NTP requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Dropped NTS-KE connections", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Dropped command requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Dropped client log records", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Received authenticated NTP requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ NULL }
};

static const Field serverstats3_report_fields[] = {
	{ "Received NTP requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Accepted NTS-KE connections", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Received command requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Dropped NTP requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Dropped NTS-KE connections", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Dropped command requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Dropped client log records", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Received authenticated NTP requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Received interleaved NTP requests", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "Held NTP timestamps", TYPE_UINT32, CHRONY_CONTENT_COUNT },
	{ "NTP timestamp span", TYPE_UINT32, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ NULL }
};

static const Field serverstats4_report_fields[] = {
	{ "Received NTP requests", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "Accepted NTS-KE connections", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "Received command requests", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "Dropped NTP requests", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "Dropped NTS-KE connections", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "Dropped command requests", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "Dropped client log records", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "Received authenticated NTP requests", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "Received interleaved NTP requests", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "Held NTP timestamps", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "NTP timestamp span", TYPE_UINT64, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ "Served daemon RX timestamps", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "Served daemon TX timestamps", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "Served kernel RX timestamps", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "Served kernel TX timestamps", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "Served hardware RX timestamps", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "Served hardware TX timestamps", TYPE_UINT64, CHRONY_CONTENT_COUNT },
	{ "Reserved #1", TYPE_UINT64, CHRONY_CONTENT_NONE },
	{ "Reserved #2", TYPE_UINT64, CHRONY_CONTENT_NONE },
	{ "Reserved #3", TYPE_UINT64, CHRONY_CONTENT_NONE },
	{ "Reserved #4", TYPE_UINT64, CHRONY_CONTENT_NONE },
	{ NULL }
};

static const Field rtcdata_report_fields[] = {
	{ "Reference time", TYPE_TIMESPEC, CHRONY_CONTENT_TIME },
	{ "Samples", TYPE_UINT16, CHRONY_CONTENT_COUNT },
	{ "Runs", TYPE_UINT16, CHRONY_CONTENT_COUNT },
	{ "Span", TYPE_UINT32, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ "Offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_SECONDS },
	{ "Frequency offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_PPM },
	{ NULL }
};

static const Constant smoothing_flags[] = {
	{ 0x1, "active" },
	{ 0x2, "leaponly" },
	{ 0 }
};

static const Field smoothing_report_fields[] = {
	{ "Flags", TYPE_UINT32, CHRONY_CONTENT_FLAGS, smoothing_flags },
	{ "Offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_SECONDS },
	{ "Frequency offset", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_PPM },
	{ "Wander", TYPE_FLOAT, CHRONY_CONTENT_OFFSET_PPM_PER_SECOND },
	{ "Last update ago", TYPE_FLOAT, CHRONY_CONTENT_INTERVAL_SECONDS },
	{ "Remaining time", TYPE_FLOAT, CHRONY_CONTENT_INTERVAL_SECONDS },
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
