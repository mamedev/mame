// license:BSD-3-Clause
// copyright-holders:hap
/*

  OKI MSM6588 ADPCM Recorder

*/

#ifndef MAME_SOUND_OKIM6588_H
#define MAME_SOUND_OKIM6588_H

#pragma once

#include "sound/okiadpcm.h"


class okim6588_device : public device_t, public device_sound_interface
{
public:
	okim6588_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto write_mon() { return m_write_mon.bind(); }
	void set_mcum_pin(int state) { m_chip_mode = state ? CHIP_MODE_MCU : CHIP_MODE_STANDALONE; }

	// D0-D3 (MCU mode)
	void data_w(u8 data);
	u8 data_r();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;

private:
	enum chip_mode : u8
	{
		CHIP_MODE_STANDALONE = 0,
		CHIP_MODE_MCU
	};

	enum command_state : u8
	{
		COMMAND_READY = 0,
		COMMAND_SAMP,
		COMMAND_EXT,
		COMMAND_VDS
	};

	enum run_state : u8
	{
		RUN_STOP = 0,
		RUN_PAUSE,
		RUN_PLAY_SERIAL,
		RUN_PLAY_EXT,
		RUN_RECORD_SERIAL,
		RUN_RECORD_EXT,
	};

	devcb_write_line m_write_mon;

	chip_mode m_chip_mode;
	command_state m_command_state;
	run_state m_run_state;

	sound_stream *m_stream;

	u8 m_adpcm_data;
	oki_adpcm_state m_adpcm;
	bool m_rec_mode;
	u16 m_samp_fdiv;
	u8 m_vds_bit;

	emu_timer *m_adpcm_timer;
	emu_timer *m_mon_timer;

	TIMER_CALLBACK_MEMBER(clock_adpcm);
	TIMER_CALLBACK_MEMBER(set_mon);
	s16 get_adpcm_sample(u8 data);
	void reset_adpcm();
};


DECLARE_DEVICE_TYPE(OKIM6588, okim6588_device)

#endif // MAME_SOUND_OKIM6588_H
