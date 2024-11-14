// license:BSD-3-Clause
// copyright-holders:David Haywood,AJR

#ifndef MAME_SHARED_EFO_ZSU_H
#define MAME_SHARED_EFO_ZSU_H

#pragma once

#include "cedar_magnet_board.h"

#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/gen_latch.h"
#include "machine/40105.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"

DECLARE_DEVICE_TYPE(EFO_ZSU,            efo_zsu_device)
DECLARE_DEVICE_TYPE(EFO_ZSU1,           efo_zsu1_device)
DECLARE_DEVICE_TYPE(CEDAR_MAGNET_SOUND, cedar_magnet_sound_device)


class efo_zsu_device : public device_t
{
public:
	// construction/destruction
	efo_zsu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void sound_command_w(u8 data);

protected:
	efo_zsu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	void adpcm_fifo_w(u8 data);

	void zsu_io(address_map &map) ATTR_COLD;
	void zsu_map(address_map &map) ATTR_COLD;

	void ay1_porta_w(u8 data);

	void ctc1_z0_w(int state);
	void ctc1_z1_w(int state);
	void ctc1_z2_w(int state);
	void ctc0_z0_w(int state);
	void ctc0_z1_w(int state);
	void ctc0_z2_w(int state);
	void fifo_dor_w(int state);

	TIMER_CALLBACK_MEMBER(fifo_shift);
	TIMER_CALLBACK_MEMBER(adpcm_clock);
	TIMER_CALLBACK_MEMBER(ctc0_ck0_restart);

protected:
	required_device_array<z80ctc_device, 2> m_ctc;
	required_device<z80ctc_channel_device> m_ctc0_ch0;
	required_device<z80ctc_channel_device> m_ctc0_ch2;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<cmos_40105_device> m_fifo;
	required_device<msm5205_device> m_adpcm;

	emu_timer *m_fifo_shift_timer;
	emu_timer *m_adpcm_clock_timer;
	emu_timer *m_ctc0_ck0_restart_timer;

	u8 m_ay1_porta;
};


class efo_zsu1_device : public efo_zsu_device
{
public:
	// construction/destruction
	efo_zsu1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};


class cedar_magnet_sound_device : public efo_zsu_device, public cedar_magnet_board_interface
{
public:
	// construction/destruction
	cedar_magnet_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual TIMER_CALLBACK_MEMBER(reset_assert_callback) override;

private:
	void cedar_magnet_sound_map(address_map &map) ATTR_COLD;

	void ay0_porta_w(u8 data);
};

#endif // MAME_SHARED_EFO_ZSU_H
