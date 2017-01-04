// license:BSD-3-Clause
// copyright-holders:David Haywood,AJR

#pragma once

#ifndef CEDAR_MAGNET_SOUND_DEF
#define CEDAR_MAGNET_SOUND_DEF

#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/gen_latch.h"
#include "machine/40105.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "machine/cedar_magnet_board.h"

extern const device_type EFO_ZSU;
extern const device_type EFO_ZSU1;
extern const device_type CEDAR_MAGNET_SOUND;

#define MCFG_CEDAR_MAGNET_SOUND_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, CEDAR_MAGNET_SOUND, 0)


class efo_zsu_device : public device_t
{
public:
	// construction/destruction
	efo_zsu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, u32 clock, const char *shortname, const char *source);
	efo_zsu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	required_device<z80ctc_device> m_ctc0;
	required_device<z80ctc_device> m_ctc1;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<cmos_40105_device> m_fifo;
	required_device<msm5205_device> m_adpcm;

	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(adpcm_fifo_w);
	DECLARE_WRITE8_MEMBER(ay1_porta_w);

	DECLARE_WRITE_LINE_MEMBER(ctc1_z0_w);
	DECLARE_WRITE_LINE_MEMBER(ctc1_z1_w);
	DECLARE_WRITE_LINE_MEMBER(ctc1_z2_w);
	DECLARE_WRITE_LINE_MEMBER(ctc0_z0_w);
	DECLARE_WRITE_LINE_MEMBER(ctc0_z1_w);
	DECLARE_WRITE_LINE_MEMBER(ctc0_z2_w);
	DECLARE_WRITE_LINE_MEMBER(fifo_dor_w);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;

private:
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

	DECLARE_WRITE8_MEMBER(ay0_porta_w);

	TIMER_CALLBACK_MEMBER(reset_assert_callback) override;

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;

private:
};

#endif
