// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Philip Bennett
/*****************************************************************************

  MB87078 6-bit,4-channel electronic volume controller emulator


*****************************************************************************/

#ifndef __MB87078_H__
#define __MB87078_H__


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_MB87078_GAIN_CHANGED_CB(_devcb) \
	devcb = &mb87078_device::set_gain_changed_callback(*device, DEVCB_##_devcb);

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class mb87078_device : public device_t
{
public:
	mb87078_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~mb87078_device() {}

	template<class _Object> static devcb_base &set_gain_changed_callback(device_t &device, _Object object) { return downcast<mb87078_device &>(device).m_gain_changed_cb.set_callback(object); }

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
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	int          m_gain[4];       /* gain index 0-63,64,65 */
	int          m_channel_latch; /* current channel */
	UINT8        m_latch[2][4];   /* 6bit+3bit 4 data latches */
	UINT8        m_reset_comp;

	devcb_write8 m_gain_changed_cb;
};

extern const device_type MB87078;

#endif  /* __MB87078_H__ */
