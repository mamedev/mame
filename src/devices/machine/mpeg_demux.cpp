// license:BSD-3-Clause
// copyright-holders:Alexandre Derumier
/***************************************************************************

    MPEG-1 system stream demultiplexer.  See mpeg_demux.h.

    The state machine follows the stream byte by byte, without buffering a
    packet, so a decoder can be fed as its input FIFO is written.

***************************************************************************/

#include "emu.h"
#include "mpeg_demux.h"

void mpeg_demux::reset()
{
	state = DEMUX_IDLE;
	packet_body = false;
	length_decreasing = false;
	length = 0;
	dts_present = false;
	scr = pts = dts = 0;
	scr_temp = pts_temp = dts_temp = 0;
	scr_updated = pts_updated = dts_updated = false;
	program_end = false;
}

void mpeg_demux::register_save_state(device_t &device, int index)
{
	device.save_item(packet_body, "mpeg_demux_packet_body", index);
	device.save_item(scr, "mpeg_demux_scr", index);
	device.save_item(pts, "mpeg_demux_pts", index);
	device.save_item(dts, "mpeg_demux_dts", index);
	device.save_item(scr_updated, "mpeg_demux_scr_updated", index);
	device.save_item(pts_updated, "mpeg_demux_pts_updated", index);
	device.save_item(dts_updated, "mpeg_demux_dts_updated", index);
	device.save_item(program_end, "mpeg_demux_program_end", index);
	device.save_item(state, "mpeg_demux_state", index);
	device.save_item(length_decreasing, "mpeg_demux_length_decreasing", index);
	device.save_item(length, "mpeg_demux_length", index);
	device.save_item(dts_present, "mpeg_demux_dts_present", index);
	device.save_item(scr_temp, "mpeg_demux_scr_temp", index);
	device.save_item(pts_temp, "mpeg_demux_pts_temp", index);
	device.save_item(dts_temp, "mpeg_demux_dts_temp", index);
}

// One byte through the state machine.  packet_body says whether the *next*
// byte belongs to an elementary stream payload, exactly as the registered
// output of the hardware does.
void mpeg_demux::byte(u8 data, u8 stream_filter)
{
	scr_updated = false;
	pts_updated = false;
	dts_updated = false;
	program_end = false;

	if (length_decreasing)
	{
		if (length == 1)
		{
			length_decreasing = false;
			packet_body = false;
		}
		length--;
	}

	switch (state)
	{
	case DEMUX_PACK5:
		state = DEMUX_IDLE;
		scr = scr_temp;
		scr_updated = true;
		break;

	case DEMUX_PACK4:
		state = DEMUX_PACK5;
		scr_temp = (scr_temp & ~s64(0x7f)) | ((data >> 1) & 0x7f);
		break;

	case DEMUX_PACK3:
		state = DEMUX_PACK4;
		scr_temp = (scr_temp & ~(s64(0xff) << 7)) | (s64(data) << 7);
		break;

	case DEMUX_PACK2:
		state = DEMUX_PACK3;
		scr_temp = (scr_temp & ~(s64(0x7f) << 15)) | (s64((data >> 1) & 0x7f) << 15);
		break;

	case DEMUX_PACK1:
		state = DEMUX_PACK2;
		scr_temp = (scr_temp & ~(s64(0xff) << 22)) | (s64(data) << 22);
		break;

	case DEMUX_PACK0:
		state = DEMUX_PACK1;
		scr_temp = (scr_temp & ~(s64(0x7) << 30)) | (s64((data >> 1) & 0x7) << 30);
		break;

	case DEMUX_PES8:
		// only reached when a PTS was present; a DTS cannot occur without one
		state = DEMUX_IDLE;
		pts = pts_temp;
		pts_updated = true;
		if (dts_present)
		{
			dts = dts_temp;
		}
		else
		{
			// no DTS.  VMPEG uses the PTS instead, and so does ffprobe.
			dts = pts_temp;
		}
		dts_updated = true;
		break;

	case DEMUX_PES_DTS4:
		if (BIT(data, 0))
		{
			dts_temp = (dts_temp & ~s64(0x7f)) | ((data >> 1) & 0x7f);
			packet_body = true;
			state = DEMUX_PES8;
		}
		else
		{
			state = DEMUX_IDLE;
		}
		break;

	case DEMUX_PES_DTS3:
		state = DEMUX_PES_DTS4;
		dts_temp = (dts_temp & ~(s64(0xff) << 7)) | (s64(data) << 7);
		break;

	case DEMUX_PES_DTS2:
		if (BIT(data, 0))
		{
			dts_temp = (dts_temp & ~(s64(0x7f) << 15)) | (s64((data >> 1) & 0x7f) << 15);
			state = DEMUX_PES_DTS3;
		}
		else
		{
			state = DEMUX_IDLE;
		}
		break;

	case DEMUX_PES_DTS1:
		state = DEMUX_PES_DTS2;
		dts_temp = (dts_temp & ~(s64(0xff) << 22)) | (s64(data) << 22);
		break;

	case DEMUX_PES_DTS0:
		if ((data & 0xf1) == 0x11)
		{
			dts_temp = (dts_temp & ~(s64(0x7) << 30)) | (s64((data >> 1) & 0x7) << 30);
			state = DEMUX_PES_DTS1;
		}
		else
		{
			state = DEMUX_IDLE;
		}
		break;

	case DEMUX_PES7:
		if (BIT(data, 0))
		{
			pts_temp = (pts_temp & ~s64(0x7f)) | ((data >> 1) & 0x7f);
			if (dts_present)
			{
				state = DEMUX_PES_DTS0;
			}
			else
			{
				packet_body = true;
				state = DEMUX_PES8;
			}
		}
		else
		{
			state = DEMUX_IDLE;
		}
		break;

	case DEMUX_PES6:
		state = DEMUX_PES7;
		pts_temp = (pts_temp & ~(s64(0xff) << 7)) | (s64(data) << 7);
		break;

	case DEMUX_PES5:
		if (BIT(data, 0))
		{
			pts_temp = (pts_temp & ~(s64(0x7f) << 15)) | (s64((data >> 1) & 0x7f) << 15);
			state = DEMUX_PES6;
		}
		else
		{
			state = DEMUX_IDLE;
		}
		break;

	case DEMUX_PES4:
		state = DEMUX_PES5;
		pts_temp = (pts_temp & ~(s64(0xff) << 22)) | (s64(data) << 22);
		break;

	case DEMUX_PES3:
		// second byte of the STD buffer size, ignored
		state = DEMUX_PES2;
		break;

	case DEMUX_PES2:
		if ((data & 0xf1) == 0x21)
		{
			// PTS only
			pts_temp = (pts_temp & ~(s64(0x7) << 30)) | (s64((data >> 1) & 0x7) << 30);
			dts_present = false;
			state = DEMUX_PES4;
		}
		else if ((data & 0xf1) == 0x31)
		{
			// PTS and DTS
			pts_temp = (pts_temp & ~(s64(0x7) << 30)) | (s64((data >> 1) & 0x7) << 30);
			dts_present = true;
			state = DEMUX_PES4;
		}
		else if (data == 0x0f)
		{
			// neither
			state = DEMUX_IDLE;
			packet_body = true;
		}
		else if ((data & 0xc0) == 0x40)
		{
			// STD buffer size
			state = DEMUX_PES3;
		}
		else if (data == 0xff)
		{
			// stuffing byte
			state = DEMUX_PES2;
		}
		else
		{
			state = DEMUX_IDLE;
		}
		break;

	case DEMUX_PES1:
		state = DEMUX_PES2;
		length = (length & 0xff00) | data;
		length_decreasing = true;
		break;

	case DEMUX_PES0:
		state = DEMUX_PES1;
		length = (length & 0x00ff) | (u16(data) << 8);
		break;

	case DEMUX_MAGIC_MATCH:
		if (data == 0xba)
		{
			state = DEMUX_PACK0;
		}
		else if ((data & 0xf0) == 0xc0 || (data & 0xf0) == 0xe0)
		{
			// audio (0xc0) or video (0xe0) elementary stream
			state = ((data & 0x0f) == stream_filter) ? DEMUX_PES0 : DEMUX_IDLE;
		}
		else if (data == 0xb9)
		{
			program_end = true;
			state = DEMUX_IDLE;
		}
		else
		{
			state = DEMUX_IDLE;
		}
		break;

	case DEMUX_MAGIC2:
		if (data == 0x01)
			state = DEMUX_MAGIC_MATCH;
		else if (data == 0x00)
			state = DEMUX_MAGIC2;
		else
			state = DEMUX_IDLE;
		break;

	case DEMUX_MAGIC0:
		state = (data == 0x00) ? DEMUX_MAGIC2 : DEMUX_IDLE;
		break;

	case DEMUX_IDLE:
	default:
		if (data == 0x00 && !packet_body)
			state = DEMUX_MAGIC0;
		else
			state = DEMUX_IDLE;
		break;
	}
}
