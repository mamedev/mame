// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Philip Bennett
/*****************************************************************************

  MB87078 6-bit,4-channel electronic volume controller emulator


*****************************************************************************/

#ifndef MAME_MACHINE_MB87078_H
#define MAME_MACHINE_MB87078_H

#pragma once

class mb87078_device : public device_t
{
public:
	mb87078_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto gain_changed() { return m_gain_changed_cb.bind(); }

	void data_w(int data, int dsel);
	void reset_comp_w(int level);

	/* gain_decibel_r will return 'channel' gain on the device.
	   Returned value represents channel gain expressed in decibels,
	   Range from 0 to -32.0 (or -256.0 for -infinity) */
	float gain_decibel_r(int channel);

	/* gain_percent_r will return 'channel' gain on the device.
	   Returned value represents channel gain expressed in percents of maximum volume.
	   Range from 100 to 0. (100 = 0dB; 50 = -6dB; 0 = -infinity)
	   This function is designed for use with MAME mixer_xxx() functions. */
	int gain_percent_r(int channel);

	void gain_recalc();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	int          m_gain[4];       /* gain index 0-63,64,65 */
	int          m_channel_latch; /* current channel */
	uint8_t        m_latch[2][4];   /* 6bit+3bit 4 data latches */
	uint8_t        m_reset_comp;

	devcb_write8 m_gain_changed_cb;
};

DECLARE_DEVICE_TYPE(MB87078, mb87078_device)

#endif // MAME_MACHINE_MB87078_H
