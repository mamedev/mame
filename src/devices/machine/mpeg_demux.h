// license:BSD-3-Clause
// copyright-holders:Alexandre Derumier
/***************************************************************************

    MPEG-1 system stream demultiplexer.

    Recovers one elementary stream, and the timestamps that pace it, from an
    ISO/IEC 11172-1 system stream fed to it byte by byte.  MPEG decoder chips
    usually contain one of these, selecting their stream by number, so each
    decoder in a system sees the whole stream and takes its own part of it.

***************************************************************************/

#ifndef MAME_MACHINE_MPEG_DEMUX_H
#define MAME_MACHINE_MPEG_DEMUX_H

#pragma once

class device_t;

class mpeg_demux
{
public:
	// Feed one byte of the system stream.  stream_filter is the low nibble of
	// the stream id to keep.  packet_body says whether the NEXT byte belongs
	// to the selected elementary stream, so the caller tests it before the
	// call, exactly as a registered output would present it.
	void byte(u8 data, u8 stream_filter);

	void reset();

	// Register state with an owning device.
	void register_save_state(device_t &device, int index = 0) ATTR_COLD;

	bool packet_body = false;

	// the most recent timestamps, in 90 kHz units
	s64 scr = 0, pts = 0, dts = 0;

	// pulses, valid for the byte that completed the field
	bool scr_updated = false, pts_updated = false, dts_updated = false;
	bool program_end = false;

private:
	enum : u8
	{
		DEMUX_IDLE = 0, DEMUX_MAGIC0, DEMUX_MAGIC2, DEMUX_MAGIC_MATCH,
		DEMUX_PACK0, DEMUX_PACK1, DEMUX_PACK2, DEMUX_PACK3, DEMUX_PACK4, DEMUX_PACK5,
		DEMUX_PES0, DEMUX_PES1, DEMUX_PES2, DEMUX_PES3, DEMUX_PES4, DEMUX_PES5,
		DEMUX_PES6, DEMUX_PES7, DEMUX_PES8,
		DEMUX_PES_DTS0, DEMUX_PES_DTS1, DEMUX_PES_DTS2, DEMUX_PES_DTS3, DEMUX_PES_DTS4
	};

	u8 state = DEMUX_IDLE;
	bool length_decreasing = false;
	u16 length = 0;
	bool dts_present = false;
	s64 scr_temp = 0, pts_temp = 0, dts_temp = 0;
};

// MPEG system clock values are 33 bits, so the difference between two of them
// wraps and is signed.
inline s64 mpeg_timestamp_diff(s64 a, s64 b)
{
	s64 d = (a - b) & 0x1ffffffffLL;
	if (d & 0x100000000LL)
		d -= 0x200000000LL;
	return d;
}

#endif // MAME_MACHINE_MPEG_DEMUX_H
