// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_SOUND_SN76496_H
#define MAME_SOUND_SN76496_H

#pragma once


DECLARE_DEVICE_TYPE(SN76496,  sn76496_device)
DECLARE_DEVICE_TYPE(Y2404,    y2404_device)
DECLARE_DEVICE_TYPE(SN76489,  sn76489_device)
DECLARE_DEVICE_TYPE(SN76489A, sn76489a_device)
DECLARE_DEVICE_TYPE(SN76494,  sn76494_device)
DECLARE_DEVICE_TYPE(SN94624,  sn94624_device)
DECLARE_DEVICE_TYPE(NCR8496,  ncr8496_device)
DECLARE_DEVICE_TYPE(PSSJ3,    pssj3_device)
DECLARE_DEVICE_TYPE(GAMEGEAR, gamegear_device)
DECLARE_DEVICE_TYPE(SEGAPSG,  segapsg_device)


class sn76496_base_device : public device_t, public device_sound_interface
{
public:
	auto ready_cb() { return m_ready_handler.bind(); }
	void stereo_w(u8 data);
	void write(u8 data);
	int ready_r() { return m_ready_state ? 1 : 0; }

protected:
	sn76496_base_device(const machine_config &mconfig, device_type type, const char *tag,
			int feedbackmask, int noisetap1, int noisetap2, bool negate, bool stereo, int clockdivider,
			bool ncr, bool sega, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;
	virtual void sound_stream_update(sound_stream &stream) override;

	TIMER_CALLBACK_MEMBER(delayed_ready);

private:
	inline bool     in_noise_mode();
	void            register_for_save_states();

	bool            m_ready_state;
	devcb_write_line m_ready_handler;

	sound_stream*   m_sound;

	const int32_t     m_feedback_mask;    // mask for feedback
	const int32_t     m_whitenoise_tap1;  // mask for white noise tap 1 (higher one, usually bit 14)
	const int32_t     m_whitenoise_tap2;  // mask for white noise tap 2 (lower one, usually bit 13)
	const bool      m_negate;           // output negate flag
	const bool      m_stereo;           // whether we're dealing with stereo or not
	const int32_t     m_clock_divider;    // clock divider
	const bool      m_ncr_style_psg;    // flag to ignore writes to regs 1,3,5,6,7 with bit 7 low
	const bool      m_sega_style_psg;   // flag to make frequency zero acts as if it is one more than max (0x3ff+1) or if it acts like 0; the initial register is pointing to 0x3 instead of 0x0; the volume reg is preloaded with 0xF instead of 0x0

	int32_t           m_vol_table[16];    // volume table (for 4-bit to db conversion)
	int32_t           m_register[8];      // registers
	int32_t           m_last_register;    // last register written
	int32_t           m_volume[4];        // db volume of voice 0-2 and noise
	uint32_t          m_RNG;              // noise generator LFSR
	int32_t           m_current_clock;
	int32_t           m_stereo_mask;      // the stereo output mask
	int32_t           m_period[4];        // Length of 1/2 of waveform
	int32_t           m_count[4];         // Position within the waveform
	int32_t           m_output[4];        // 1-bit output of each channel, pre-volume

	emu_timer *m_ready_timer;
};

// SN76496: Whitenoise verified, phase verified, periodic verified (by Michael Zapf)
class sn76496_device : public sn76496_base_device
{
public:
	sn76496_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// Y2404 not verified yet. todo: verify; (don't be fooled by the Y, it's a TI chip, not Yamaha)
class y2404_device : public sn76496_base_device
{
public:
	y2404_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// SN76489 not verified yet. todo: verify;
class sn76489_device : public sn76496_base_device
{
public:
	sn76489_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// SN76489A: whitenoise verified, phase verified, periodic verified (by plgdavid)
class sn76489a_device : public sn76496_base_device
{
public:
	sn76489a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// SN76494 not verified, (according to datasheet: same as sn76489a but without the /8 divider)
class sn76494_device : public sn76496_base_device
{
public:
	sn76494_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// SN94624 whitenoise verified, phase verified, period verified; verified by PlgDavid
class sn94624_device : public sn76496_base_device
{
public:
	sn94624_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// NCR8496 whitenoise verified, phase verified; verified by ValleyBell & NewRisingSun
class ncr8496_device : public sn76496_base_device
{
public:
	ncr8496_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// PSSJ-3 whitenoise verified, phase verified; verified by ValleyBell & NewRisingSun
class pssj3_device : public sn76496_base_device
{
public:
	pssj3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// Verified by Justin Kerk
class gamegear_device : public sn76496_base_device
{
public:
	gamegear_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// todo: verify; from smspower wiki, assumed to have same invert as gamegear
class segapsg_device : public sn76496_base_device
{
public:
	segapsg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

#endif // MAME_SOUND_SN76496_H
