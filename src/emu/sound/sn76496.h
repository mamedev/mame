#pragma once

#ifndef __SN76496_H__
#define __SN76496_H__

#include "devlegcy.h"

READ_LINE_DEVICE_HANDLER( sn76496_ready_r );
DECLARE_WRITE8_DEVICE_HANDLER( sn76496_w );
DECLARE_WRITE8_DEVICE_HANDLER( sn76496_stereo_w );

class sn76496_device : public device_t,
                                  public device_sound_interface
{
public:
	sn76496_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	sn76496_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~sn76496_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type SN76496;

class u8106_device : public sn76496_device
{
public:
	u8106_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type U8106;

class y2404_device : public sn76496_device
{
public:
	y2404_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type Y2404;

class sn76489_device : public sn76496_device
{
public:
	sn76489_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type SN76489;

class sn76489a_device : public sn76496_device
{
public:
	sn76489a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type SN76489A;

class sn76494_device : public sn76496_device
{
public:
	sn76494_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type SN76494;

class sn94624_device : public sn76496_device
{
public:
	sn94624_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type SN94624;

class ncr7496_device : public sn76496_device
{
public:
	ncr7496_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type NCR7496;

class gamegear_device : public sn76496_device
{
public:
	gamegear_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type GAMEGEAR;

class segapsg_device : public sn76496_device
{
public:
	segapsg_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type SEGAPSG;


/*****************************************************************
    New class implementation
    Michael Zapf, June 2012
*****************************************************************/

extern const device_type SN76496_NEW;
extern const device_type U8106_NEW;
extern const device_type Y2404_NEW;
extern const device_type SN76489_NEW;
extern const device_type SN76489A_NEW;
extern const device_type SN76494_NEW;
extern const device_type SN94624_NEW;
extern const device_type NCR7496_NEW;
extern const device_type GAMEGEAR_NEW;
extern const device_type SEGAPSG_NEW;

struct sn76496_config
{
	devcb_write_line		ready;
};

class sn76496_base_device : public device_t, public device_sound_interface
{
public:
	sn76496_base_device(const machine_config &mconfig, device_type type,  const char *name, const char *tag,
		int feedbackmask, int noisetap1, int noisetap2, bool negate, bool stereo, int clockdivider, int freq0,
		device_t *owner, UINT32 clock);
	DECLARE_READ_LINE_MEMBER( ready_r );
	DECLARE_WRITE8_MEMBER( stereo_w );
	void write(UINT8 data);
	DECLARE_WRITE8_MEMBER( write );

protected:
	void	device_start();
	void	sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

private:
	inline bool 	in_noise_mode();
	void			register_for_save_states();
	void			countdown_cycles();

	bool			m_ready_state;

	devcb_resolved_write_line	m_ready;

	sound_stream*	m_sound;

	const INT32 	m_feedback_mask;	// mask for feedback
	const INT32 	m_whitenoise_tap1;	// mask for white noise tap 1 (higher one, usually bit 14)
	const INT32 	m_whitenoise_tap2;	// mask for white noise tap 2 (lower one, usually bit 13)
	const bool		m_negate;			// output negate flag
	const bool		m_stereo;			// whether we're dealing with stereo or not
	const INT32 	m_clock_divider;	// clock divider
	const bool		m_freq0_is_max;		// flag for if frequency zero acts as if it is one more than max (0x3ff+1) or if it acts like 0

	INT32			m_vol_table[16];	// volume table (for 4-bit to db conversion)
	INT32			m_register[8];		// registers
	INT32			m_last_register;	// last register written
	INT32			m_volume[4];		// db volume of voice 0-2 and noise
	UINT32			m_RNG;				// noise generator LFSR
	INT32			m_current_clock;
	INT32			m_stereo_mask;		// the stereo output mask
	INT32			m_period[4];		// Length of 1/2 of waveform
	INT32			m_count[4];			// Position within the waveform
	INT32			m_output[4];		// 1-bit output of each channel, pre-volume
	INT32			m_cycles_to_ready;	// number of cycles until the READY line goes active
};

// SN76496: Whitenoise verified, phase verified, periodic verified (by Michael Zapf)
class sn76496_new_device : public sn76496_base_device
{
public:
	sn76496_new_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	:  sn76496_base_device(mconfig, SN76496_NEW, "SN76496_NEW", tag, 0x10000, 0x04, 0x08, false, false, 8, true, owner, clock)
	{ }
};

// U8106 not verified yet. todo: verify; (a custom marked sn76489? only used on mr. do and maybe other universal games)
class u8106_new_device : public sn76496_base_device
{
public:
	u8106_new_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	:  sn76496_base_device(mconfig, U8106_NEW, "U8106_NEW", tag, 0x4000, 0x01, 0x02, true, false, 8, true, owner, clock)
	{ }
};

// Y2404 not verified yet. todo: verify; (don't be fooled by the Y, it's a TI chip, not Yamaha)
class y2404_new_device : public sn76496_base_device
{
public:
	y2404_new_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	:  sn76496_base_device(mconfig, Y2404_NEW, "Y2404_NEW", tag, 0x10000, 0x04, 0x08, false, false, 8, true, owner, clock)
	{ }
};

// SN76489 not verified yet. todo: verify;
class sn76489_new_device : public sn76496_base_device
{
public:
	sn76489_new_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	:  sn76496_base_device(mconfig, SN76489_NEW, "SN76489_NEW", tag, 0x4000, 0x01, 0x02, true, false, 8, true, owner, clock)
	{ }
};

// SN76489A: whitenoise verified, phase verified, periodic verified (by plgdavid)
class sn76489a_new_device : public sn76496_base_device
{
public:
	sn76489a_new_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	:  sn76496_base_device(mconfig, SN76489A_NEW, "SN76489A_NEW", tag, 0x10000, 0x04, 0x08, false, false, 8, true, owner, clock)
	{ }
};

// SN76494 not verified, (according to datasheet: same as sn76489a but without the /8 divider)
class sn76494_new_device : public sn76496_base_device
{
public:
	sn76494_new_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	:  sn76496_base_device(mconfig, SN76494_NEW, "SN76494_NEW", tag, 0x10000, 0x04, 0x08, false, false, 1, true, owner, clock)
	{ }
};

// SN94624 whitenoise verified, phase verified, period verified; verified by PlgDavid
class sn94624_new_device : public sn76496_base_device
{
public:
	sn94624_new_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	:  sn76496_base_device(mconfig, SN94624_NEW, "SN94624_NEW", tag, 0x4000, 0x01, 0x02, true, false, 1, true, owner, clock)
	{ }
};

// NCR7496 not verified; info from smspower wiki
class ncr7496_new_device : public sn76496_base_device
{
public:
	ncr7496_new_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	:  sn76496_base_device(mconfig, NCR7496_NEW, "NCR7496_NEW", tag, 0x8000, 0x02, 0x20, false, false, 8, true, owner, clock)
	{ }
};

// Verified by Justin Kerk
class gamegear_new_device : public sn76496_base_device
{
public:
	gamegear_new_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	:  sn76496_base_device(mconfig, GAMEGEAR_NEW, "Game Gear PSG_NEW", tag, 0x8000, 0x01, 0x08, true, true, 8, false, owner, clock)
	{ }
};

// todo: verify; from smspower wiki, assumed to have same invert as gamegear
class segapsg_new_device : public sn76496_base_device
{
public:
	segapsg_new_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	:  sn76496_base_device(mconfig, SEGAPSG_NEW, "SEGA VDP PSG_NEW", tag, 0x8000, 0x01, 0x08, true, false, 8, false, owner, clock)
	{ }
};

#endif /* __SN76496_H__ */
