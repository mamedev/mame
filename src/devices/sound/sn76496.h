// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#pragma once

#ifndef __SN76496_H__
#define __SN76496_H__


extern const device_type SN76496;
extern const device_type U8106;
extern const device_type Y2404;
extern const device_type SN76489;
extern const device_type SN76489A;
extern const device_type SN76494;
extern const device_type SN94624;
extern const device_type NCR7496;
extern const device_type GAMEGEAR;
extern const device_type SEGAPSG;

#define MCFG_SN76496_READY_HANDLER(_devcb) \
	devcb = &sn76496_base_device::set_ready_handler(*device, DEVCB_##_devcb);

class sn76496_base_device : public device_t, public device_sound_interface
{
public:
	sn76496_base_device(const machine_config &mconfig, device_type type, std::string name, std::string tag,
		int feedbackmask, int noisetap1, int noisetap2, bool negate, bool stereo, int clockdivider, int sega,
		device_t *owner, UINT32 clock, std::string shortname, std::string source);

	// static configuration helpers
	template<class _Object> static devcb_base &set_ready_handler(device_t &device, _Object object) { return downcast<sn76496_base_device &>(device).m_ready_handler.set_callback(object); }

	DECLARE_WRITE8_MEMBER( stereo_w );
	void write(UINT8 data);
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ_LINE_MEMBER( ready_r ) { return m_ready_state ? 1 : 0; }

protected:
	virtual void    device_start() override;
	virtual void    sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	inline bool     in_noise_mode();
	void            register_for_save_states();
	void            countdown_cycles();

	bool            m_ready_state;

	devcb_write_line m_ready_handler;

	sound_stream*   m_sound;

	const INT32     m_feedback_mask;    // mask for feedback
	const INT32     m_whitenoise_tap1;  // mask for white noise tap 1 (higher one, usually bit 14)
	const INT32     m_whitenoise_tap2;  // mask for white noise tap 2 (lower one, usually bit 13)
	const bool      m_negate;           // output negate flag
	const bool      m_stereo;           // whether we're dealing with stereo or not
	const INT32     m_clock_divider;    // clock divider
	const bool      m_sega_style_psg;   // flag for if frequency zero acts as if it is one more than max (0x3ff+1) or if it acts like 0; AND if the initial register is pointing to 0x3 instead of 0x0 AND if the volume reg is preloaded with 0xF instead of 0x0

	INT32           m_vol_table[16];    // volume table (for 4-bit to db conversion)
	INT32           m_register[8];      // registers
	INT32           m_last_register;    // last register written
	INT32           m_volume[4];        // db volume of voice 0-2 and noise
	UINT32          m_RNG;              // noise generator LFSR
	INT32           m_current_clock;
	INT32           m_stereo_mask;      // the stereo output mask
	INT32           m_period[4];        // Length of 1/2 of waveform
	INT32           m_count[4];         // Position within the waveform
	INT32           m_output[4];        // 1-bit output of each channel, pre-volume
	INT32           m_cycles_to_ready;  // number of cycles until the READY line goes active
};

// SN76496: Whitenoise verified, phase verified, periodic verified (by Michael Zapf)
class sn76496_device : public sn76496_base_device
{
public:
	sn76496_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

// U8106 not verified yet. todo: verify; (a custom marked sn76489? only used on mr. do and maybe other universal games)
class u8106_device : public sn76496_base_device
{
public:
	u8106_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

// Y2404 not verified yet. todo: verify; (don't be fooled by the Y, it's a TI chip, not Yamaha)
class y2404_device : public sn76496_base_device
{
public:
	y2404_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

// SN76489 not verified yet. todo: verify;
class sn76489_device : public sn76496_base_device
{
public:
	sn76489_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

// SN76489A: whitenoise verified, phase verified, periodic verified (by plgdavid)
class sn76489a_device : public sn76496_base_device
{
public:
	sn76489a_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

// SN76494 not verified, (according to datasheet: same as sn76489a but without the /8 divider)
class sn76494_device : public sn76496_base_device
{
public:
	sn76494_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

// SN94624 whitenoise verified, phase verified, period verified; verified by PlgDavid
class sn94624_device : public sn76496_base_device
{
public:
	sn94624_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

// NCR7496 not verified; info from smspower wiki
class ncr7496_device : public sn76496_base_device
{
public:
	ncr7496_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

// Verified by Justin Kerk
class gamegear_device : public sn76496_base_device
{
public:
	gamegear_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

// todo: verify; from smspower wiki, assumed to have same invert as gamegear
class segapsg_device : public sn76496_base_device
{
public:
	segapsg_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

#endif /* __SN76496_H__ */
