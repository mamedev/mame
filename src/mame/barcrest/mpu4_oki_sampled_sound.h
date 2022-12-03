// license:BSD-3-Clause
// copyright-holders:David Haywood, James Wallace

#ifndef MAME_MACHINE_MPU4_OKI_SAMPLED_SOUND_H
#define MAME_MACHINE_MPU4_OKI_SAMPLED_SOUND_H

#pragma once

#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "sound/okim6376.h"


#ifdef MAME_DEBUG
#define MPU4SSVERBOSE 1
#else
#define MPU4SSVERBOSE 0
#endif

#define LOG_SS(x)   do { if (MPU4SSVERBOSE) logerror x; } while (0)

DECLARE_DEVICE_TYPE(MPU4_OKI_SAMPLED_SOUND, mpu4_oki_sampled_sound)

class mpu4_oki_sampled_sound : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	mpu4_oki_sampled_sound(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void ic3_write(offs_t offset, uint8_t data);
	uint8_t ic3_read(offs_t offset);

	void ic4_write(offs_t offset, uint8_t data);
	uint8_t ic4_read(offs_t offset);

	auto cb2_handler() { return m_cb2_handler.bind(); }

	void pia_gb_porta_w(uint8_t data);
	void pia_gb_portb_w(uint8_t data);
	uint8_t pia_gb_portb_r();
	DECLARE_WRITE_LINE_MEMBER(pia_gb_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pia_gb_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(output_cb2) { m_cb2_handler(state); }


protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	devcb_write_line m_cb2_handler;


private:
	required_device<ptm6840_device> m_ptm_ic3ss;
	required_device<pia6821_device> m_pia_ic4ss;
	required_device<okim6376_device> m_msm6376;

	uint8_t m_expansion_latch = 0;
	uint8_t m_global_volume = 0;
	uint8_t m_t1 = 0;
	uint8_t m_t3l = 0;
	uint8_t m_t3h = 0;
	uint8_t m_last_reset = 0;
};

#endif // MAME_MACHINE_MPU4_OKI_SAMPLED_SOUND_H
