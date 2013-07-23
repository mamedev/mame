#pragma once

#ifndef __X1_010_H__
#define __X1_010_H__

#define SETA_NUM_CHANNELS 16

struct x1_010_interface
{
	int m_adr;    /* address */
};


class x1_010_device : public device_t,
									public device_sound_interface,
									public x1_010_interface
{
public:
	x1_010_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~x1_010_device() {}

	DECLARE_READ8_MEMBER ( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_READ16_MEMBER ( word_r );
	DECLARE_WRITE16_MEMBER( word_w );

	void enable_w(int data);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state

	/* Variables only used here */
	int m_rate;                               // Output sampling rate (Hz)
	sound_stream *  m_stream;                 // Stream handle
	const UINT8 *m_region;                    // region name
	int m_sound_enable;                       // sound output enable/disable
	UINT8   m_reg[0x2000];                // X1-010 Register & wave form area
	UINT8   m_HI_WORD_BUF[0x2000];            // X1-010 16bit access ram check avoidance work
	UINT32  m_smp_offset[SETA_NUM_CHANNELS];
	UINT32  m_env_offset[SETA_NUM_CHANNELS];

	UINT32 m_base_clock;
};

extern const device_type X1_010;


#endif /* __X1_010_H__ */
