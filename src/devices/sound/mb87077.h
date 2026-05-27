// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Philip Bennett, hap
/*****************************************************************************

  MB87077 6-bit, 4-channel electronic volume controller emulator

*****************************************************************************/

#ifndef MAME_SOUND_MB87077_H
#define MAME_SOUND_MB87077_H

#pragma once

class mb87077_device : public device_t
{
public:
	mb87077_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto gain_changed() { return m_gain_changed_cb.bind(); }

	void data_w(offs_t offset, u8 data);
	u8 data_r(offs_t offset);

	/* gain_percent_r will return 'channel' gain on the device.
	   Returned value represents channel gain expressed in percents of maximum volume.
	   Range from 100 to 0. (100 = 0dB; 50 = -6dB; 0 = -infinity)
	   This function is designed for use with MAME mixer_xxx() functions. */
	u8 gain_percent_r(offs_t offset) { return gain_factor_r(offset) * 100.0 + 0.5f; } // range 100 .. 0
	float gain_factor_r(offs_t offset) { return m_gains[m_gain_index[offset & 3]]; } // range 1.0 .. 0.0

protected:
	mb87077_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void gain_recalc();
	float m_gains[64+3];

	// internal state
	u8 m_gain_index[4]; // per-channel current index to m_gains
	u16 m_channel_latch[4];
	u8 m_control;
	u8 m_data;

	devcb_write8 m_gain_changed_cb;
};

class mb87078_device : public mb87077_device
{
public:
	mb87078_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

DECLARE_DEVICE_TYPE(MB87077, mb87077_device)
DECLARE_DEVICE_TYPE(MB87078, mb87078_device)

#endif // MAME_SOUND_MB87077_H
