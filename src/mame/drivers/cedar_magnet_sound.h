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
#include "cedar_magnet_board.h"

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

	uint8_t top_port14_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void write_command(uint8_t data);
	uint8_t m_command;

	void ctc1_z0_w(int state);
	void ctc1_z1_w(int state);
	void ctc1_z2_w(int state);
	void ctc0_z0_w(int state);
	void ctc0_z1_w(int state);
	void ctc0_z2_w(int state);
	void ctc0_int_w(int state);
	void ctc1_int_w(int state);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
};

#endif
