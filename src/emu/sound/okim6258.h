// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
#pragma once

#ifndef __OKIM6258_H__
#define __OKIM6258_H__

#define FOSC_DIV_BY_1024    0
#define FOSC_DIV_BY_768     1
#define FOSC_DIV_BY_512     2

#define TYPE_3BITS          0
#define TYPE_4BITS          1

#define OUTPUT_10BITS       10
#define OUTPUT_12BITS       12


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_OKIM6258_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, OKIM6258, _clock)
#define MCFG_OKIM6258_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, OKIM6258, _clock)

#define MCFG_OKIM6258_DIVIDER(_div) \
	okim6258_device::set_start_div(*device, _div);

#define MCFG_OKIM6258_ADPCM_TYPE(_type) \
	okim6258_device::set_type(*device, _type);

#define MCFG_OKIM6258_OUT_BITS(_bits) \
	okim6258_device::set_outbits(*device, _bits);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> okim6258_device

class okim6258_device : public device_t,
						public device_sound_interface
{
public:
	okim6258_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~okim6258_device() { }

	// static configuration
	static void set_start_div(device_t &device, int div) { downcast<okim6258_device &>(device).m_start_divider = div; }
	static void set_type(device_t &device, int type) { downcast<okim6258_device &>(device).m_adpcm_type = type; }
	static void set_outbits(device_t &device, int outbit) { downcast<okim6258_device &>(device).m_output_bits = outbit; }

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
	UINT32 m_start_divider;
	UINT32 m_divider;         /* master clock divider */
	UINT8 m_adpcm_type;       /* 3/4 bit ADPCM select */
	UINT8 m_data_in;          /* ADPCM data-in register */
	UINT8 m_nibble_shift;     /* nibble select */
	sound_stream *m_stream;   /* which stream are we playing on? */

	UINT8 m_output_bits;      /* D/A precision is 10-bits but 12-bit data can be
                               output serially to an external DAC */

	INT32 m_signal;
	INT32 m_step;
};

extern const device_type OKIM6258;


#endif /* __OKIM6258_H__ */
