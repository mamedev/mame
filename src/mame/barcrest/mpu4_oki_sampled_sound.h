// license:BSD-3-Clause
// copyright-holders:David Haywood, James Wallace

#ifndef MAME_BARCREST_MPU4_OKI_SAMPLED_SOUND_H
#define MAME_BARCREST_MPU4_OKI_SAMPLED_SOUND_H

#pragma once

#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "sound/okim6376.h"

DECLARE_DEVICE_TYPE(MPU4_OKI_SAMPLED_SOUND, mpu4_oki_sampled_sound)

class mpu4_oki_sampled_sound : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	mpu4_oki_sampled_sound(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto cb2_handler() { return m_cb2_handler.bind(); }

	void ic3_write(offs_t offset, uint8_t data);
	uint8_t ic3_read(offs_t offset);

	void ic4_write(offs_t offset, uint8_t data);
	uint8_t ic4_read(offs_t offset);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write_line m_cb2_handler;

	required_device<ptm6840_device> m_ptm_ic3ss;
	required_device<pia6821_device> m_pia_ic4ss;
	required_device<okim6376_device> m_msm6376;

	uint8_t m_expansion_latch;
	uint8_t m_global_volume;
	uint8_t m_t1;
	uint8_t m_t3l;
	uint8_t m_t3h;
	uint8_t m_last_reset;

	void pia_gb_porta_w(uint8_t data);
	void pia_gb_portb_w(uint8_t data);
	uint8_t pia_gb_portb_r();
	void pia_gb_ca2_w(int state);
	void pia_gb_cb2_w(int state);
};

#endif // MAME_BARCREST_MPU4_OKI_SAMPLED_SOUND_H
