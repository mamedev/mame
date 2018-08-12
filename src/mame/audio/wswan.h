// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*****************************************************************************
 *
 * includes/wswan.h
 *
 ****************************************************************************/

#ifndef MAME_AUDIO_WSWAN_H
#define MAME_AUDIO_WSWAN_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wswan_sound_device

class wswan_sound_device : public device_t,
	public device_sound_interface,
	public device_rom_interface
{
public:
	wswan_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER( port_w );

protected:
	struct CHAN
	{
		CHAN() :
		freq(0),
		period(0),
		pos(0),
		vol_left(0),
		vol_right(0),
		on(0),
		offset(0),
		signal(0) { }

		uint16_t  freq;           /* frequency */
		uint16_t  period;         /* period */
		uint32_t  pos;            /* position */
		uint8_t   vol_left;       /* volume left */
		uint8_t   vol_right;      /* volume right */
		uint8_t   on;         /* on/off */
		uint8_t   offset;         /* sample offset */
		uint8_t    signal;        /* signal */
	};

	// device-level overrides
	virtual void device_start() override;
	virtual void device_clock_changed() override;
	virtual void device_reset() override;

	virtual void rom_bank_updated() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	void wswan_ch_set_freq( CHAN *ch, uint16_t freq );
	int fetch_sample(int channel, int offset);

	sound_stream *m_channel;
	CHAN m_audio1;     /* Audio channel 1 */
	CHAN m_audio2;     /* Audio channel 2 */
	CHAN m_audio3;     /* Audio channel 3 */
	CHAN m_audio4;     /* Audio channel 4 */
	int8_t    m_sweep_step;     /* Sweep step */
	uint32_t  m_sweep_time;     /* Sweep time */
	uint32_t  m_sweep_count;        /* Sweep counter */
	uint8_t   m_noise_type;     /* Noise generator type */
	uint8_t   m_noise_reset;        /* Noise reset */
	uint8_t   m_noise_enable;       /* Noise enable */
	uint8_t   m_noise_output;       /* Noise output */
	uint16_t  m_sample_address;     /* Sample address */
	uint8_t   m_audio2_voice;       /* Audio 2 voice */
	uint8_t   m_audio3_sweep;       /* Audio 3 sweep */
	uint8_t   m_audio4_noise;       /* Audio 4 noise */
	uint8_t   m_mono;           /* mono */
	uint8_t   m_output_volume;      /* output volume */
	uint8_t   m_external_stereo;    /* external stereo */
	uint8_t   m_external_speaker;   /* external speaker */
	uint16_t  m_noise_shift;        /* Noise counter shift register */
	uint8_t   m_master_volume;      /* Master volume */
};

DECLARE_DEVICE_TYPE(WSWAN_SND, wswan_sound_device)

#endif // MAME_AUDIO_WSWAN_H
