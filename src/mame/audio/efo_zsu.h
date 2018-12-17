// license:BSD-3-Clause
// copyright-holders:David Haywood,AJR

#ifndef MAME_AUDIO_EFO_ZSU_H
#define MAME_AUDIO_EFO_ZSU_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/gen_latch.h"
#include "machine/40105.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "machine/cedar_magnet_board.h"

DECLARE_DEVICE_TYPE(EFO_ZSU,            efo_zsu_device)
DECLARE_DEVICE_TYPE(EFO_ZSU1,           efo_zsu1_device)
DECLARE_DEVICE_TYPE(CEDAR_MAGNET_SOUND, cedar_magnet_sound_device)


class efo_zsu_device : public device_t
{
public:
	// construction/destruction
	efo_zsu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	required_device<z80ctc_device> m_ctc0;
	required_device<z80ctc_device> m_ctc1;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<cmos_40105_device> m_fifo;
	required_device<msm5205_device> m_adpcm;

	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(adpcm_fifo_w);

	void zsu_io(address_map &map);
	void zsu_map(address_map &map);
protected:
	efo_zsu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

private:
	DECLARE_WRITE8_MEMBER(ay1_porta_w);

	DECLARE_WRITE_LINE_MEMBER(ctc1_z0_w);
	DECLARE_WRITE_LINE_MEMBER(ctc1_z1_w);
	DECLARE_WRITE_LINE_MEMBER(ctc1_z2_w);
	DECLARE_WRITE_LINE_MEMBER(ctc0_z0_w);
	DECLARE_WRITE_LINE_MEMBER(ctc0_z1_w);
	DECLARE_WRITE_LINE_MEMBER(ctc0_z2_w);
	DECLARE_WRITE_LINE_MEMBER(fifo_dor_w);
};


class efo_zsu1_device : public efo_zsu_device
{
public:
	// construction/destruction
	efo_zsu1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override;
};


class cedar_magnet_sound_device : public efo_zsu_device, public cedar_magnet_board_interface
{
public:
	// construction/destruction
	cedar_magnet_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	TIMER_CALLBACK_MEMBER(reset_assert_callback) override;

	void cedar_magnet_sound_map(address_map &map);
protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

private:
	DECLARE_WRITE8_MEMBER(ay0_porta_w);
};

#endif // MAME_AUDIO_EFO_ZSU_H
