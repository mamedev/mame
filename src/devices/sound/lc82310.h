// license:BSD-3-Clause
// copyright-holders:windyfairy
/**********************************************************************

    Sanyo LC82310 MP3 decoder

**********************************************************************/

#ifndef MAME_SOUND_LC82310_H
#define MAME_SOUND_LC82310_H

#pragma once

#include "mp3_audio.h"

class lc82310_device : public device_t,
					   public device_sound_interface
{
public:
	lc82310_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void zcsctl_w(int state);
	void ckctl_w(int state);
	void dictl_w(int state);
	int doctl_r();
	int demand_r();

	void dimpg_w(uint8_t data);

	void reset_playback();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;

private:
	enum : uint8_t
	{
		ACCEPTING_CMD,
		ACCEPTING_PARAM,
	};

	enum : uint8_t
	{
		CMD_UNK10 = 0x10,
		CMD_UNK11 = 0x11,
		CMD_UNK12 = 0x12, // Set to 0 when writing data and 1 when not writing data
		CMD_UNK13_VOL = 0x13,
		CMD_UNK15_VOL = 0x15,
		CMD_UNK17 = 0x17,
		CMD_UNK18 = 0x18,
		CMD_SET_CONFIGURATION = 0x22, // has PLLOFF and SLEEP bits
		CMD_UNK80 = 0x80,
		CMD_UNK81 = 0x81,
		CMD_UNK82 = 0x82,
		CMD_GET_ERROR_STATUS = 0x83,
		CMD_UNK84 = 0x84,
		CMD_UNK85 = 0x85,
	};

	void handle_command(uint8_t cmd, uint8_t param);

	void fill_buffer();
	void append_buffer(sound_stream &stream, int &pos, int scount);

	sound_stream *stream;
	std::unique_ptr<mp3_audio> mp3dec;

	std::array<uint8_t, 0x1000> mp3data;
	std::array<short, 1152*2> samples;

	uint32_t m_mp3data_count;
	int32_t m_sample_count, m_samples_idx;
	int32_t m_frame_channels;
	float m_output_gain[2];

	uint8_t m_csctl;
	uint8_t m_ckctl;
	uint8_t m_dictl;
	uint8_t m_doctl;
	uint8_t m_ctl_state;
	uint8_t m_ctl_cmd;
	uint8_t m_ctl_bits;
	uint8_t m_ctl_byte;
	uint8_t m_ctl_out_byte;
};

DECLARE_DEVICE_TYPE(LC82310, lc82310_device)

#endif // MAME_SOUND_LC82310_H
