#pragma once

#ifndef __X1_010_H__
#define __X1_010_H__

#define SETA_NUM_CHANNELS 16


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_X1_010_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, X1_010, _clock)
#define MCFG_X1_010_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, X1_010, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct x1_010_interface
{
	int adr;    /* address */
};


// ======================> x1_010_device

class x1_010_device : public device_t,
					  public device_sound_interface
{
public:
	x1_010_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~x1_010_device() { }

    void seta_sound_enable_w(int data);

protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

public:
	DECLARE_READ8_MEMBER( seta_sound_r );
	DECLARE_WRITE8_MEMBER( seta_sound_w );

	DECLARE_READ16_MEMBER( seta_sound_word_r );
	DECLARE_WRITE16_MEMBER( seta_sound_word_w );

private:
	int m_rate;                               // Output sampling rate (Hz)
	sound_stream *m_stream;                   // Stream handle
	int m_address;                            // address eor data
	const UINT8 *m_region;                    // region name
	int m_sound_enable;                       // sound output enable/disable
	UINT8   m_reg[0x2000];                    // X1-010 Register & wave form area
	UINT8   m_HI_WORD_BUF[0x2000];            // X1-010 16bit access ram check avoidance work
	UINT32  m_smp_offset[SETA_NUM_CHANNELS];
	UINT32  m_env_offset[SETA_NUM_CHANNELS];

	UINT32 m_base_clock;
};

extern const device_type X1_010;


#endif /* __X1_010_H__ */
