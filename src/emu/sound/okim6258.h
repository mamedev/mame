#pragma once

#ifndef __OKIM6258_H__
#define __OKIM6258_H__

#define FOSC_DIV_BY_1024    0
#define FOSC_DIV_BY_768     1
#define FOSC_DIV_BY_512     2

#define TYPE_3BITS          0
#define TYPE_4BITS          1

#define OUTPUT_10BITS       0
#define OUTPUT_12BITS       1


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_OKIM6258_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, OKIM6258, _clock)
#define MCFG_OKIM6258_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, OKIM6258, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct okim6258_interface
{
	int divider;
	int adpcm_type;
	int output_12bits;
};


// ======================> okim6258_device

class okim6258_device : public device_t,
						public device_sound_interface
{
public:
	okim6258_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~okim6258_device() { }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

public:
	DECLARE_READ8_MEMBER( okim6258_status_r );
	DECLARE_WRITE8_MEMBER( okim6258_data_w );
	DECLARE_WRITE8_MEMBER( okim6258_ctrl_w );

public:
	void set_divider(int val);
	void set_clock(int val);
	int get_vclk();

private:
	void okim6258_state_save_register();
	INT16 clock_adpcm(UINT8 nibble);

private:
	UINT8  m_status;

	UINT32 m_master_clock;    /* master clock frequency */
	UINT32 m_divider;         /* master clock divider */
	UINT8 m_adpcm_type;       /* 3/4 bit ADPCM select */
	UINT8 m_data_in;          /* ADPCM data-in register */
	UINT8 m_nibble_shift;     /* nibble select */
	sound_stream *m_stream;   /* which stream are we playing on? */

	UINT8 m_output_bits;

	INT32 m_signal;
	INT32 m_step;
};

extern const device_type OKIM6258;


#endif /* __OKIM6258_H__ */
