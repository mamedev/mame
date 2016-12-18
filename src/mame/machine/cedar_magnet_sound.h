// license:BSD-3-Clause
// copyright-holders:David Haywood

#pragma once

#ifndef CEDAR_MAGNET_SOUND_DEF
#define CEDAR_MAGNET_SOUND_DEF

#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "machine/cedar_magnet_board.h"

extern const device_type CEDAR_MAGNET_SOUND;

#define MCFG_CEDAR_MAGNET_SOUND_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, CEDAR_MAGNET_SOUND, 0)


class cedar_magnet_sound_device :  public cedar_magnet_board_device
{
public:
	// construction/destruction
	cedar_magnet_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	required_device<z80ctc_device> m_ctc0;
	required_device<z80ctc_device> m_ctc1;

	DECLARE_READ8_MEMBER(soundlatch_r);
	DECLARE_WRITE8_MEMBER(adpcm_latch_w);
	DECLARE_WRITE8_MEMBER(ay1_porta_w);

	uint8_t m_adpcm_data;

	void write_command(uint8_t data);
	uint8_t m_command;

	DECLARE_WRITE_LINE_MEMBER(ctc1_z0_w);
	DECLARE_WRITE_LINE_MEMBER(ctc1_z1_w);
	DECLARE_WRITE_LINE_MEMBER(ctc1_z2_w);
	DECLARE_WRITE_LINE_MEMBER(ctc0_z0_w);
	DECLARE_WRITE_LINE_MEMBER(ctc0_z1_w);
	DECLARE_WRITE_LINE_MEMBER(ctc0_z2_w);
	DECLARE_WRITE_LINE_MEMBER(ctc0_int_w);
	DECLARE_WRITE_LINE_MEMBER(ctc1_int_w);

	TIMER_CALLBACK_MEMBER(reset_assert_callback) override;

	int m_fake_counter;
	INTERRUPT_GEN_MEMBER(fake_irq);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
};

#endif
