// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*****************************************************************************
 *
 * includes/wswan_snd.h
 *
 ****************************************************************************/

#pragma once

#ifndef _WSWAN_SND_H_
#define _WSWAN_SND_H_

#include "emu.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct CHAN
{
	CHAN() :
	freq(0),
	period(0),
	pos(0),
	vol_left(0),
	vol_right(0),
	on(0),
	signal(0) { }

	UINT16  freq;           /* frequency */
	UINT32  period;         /* period */
	UINT32  pos;            /* position */
	UINT8   vol_left;       /* volume left */
	UINT8   vol_right;      /* volume right */
	UINT8   on;         /* on/off */
	INT8    signal;         /* signal */
};


// ======================> wswan_sound_device

class wswan_sound_device : public device_t,
							public device_sound_interface
{
public:
	wswan_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~wswan_sound_device() { }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

public:
	DECLARE_WRITE8_MEMBER( port_w );

private:
	void wswan_ch_set_freq( CHAN *ch, UINT16 freq );

private:
	sound_stream *m_channel;
	CHAN m_audio1;     /* Audio channel 1 */
	CHAN m_audio2;     /* Audio channel 2 */
	CHAN m_audio3;     /* Audio channel 3 */
	CHAN m_audio4;     /* Audio channel 4 */
	INT8    m_sweep_step;     /* Sweep step */
	UINT32  m_sweep_time;     /* Sweep time */
	UINT32  m_sweep_count;        /* Sweep counter */
	UINT8   m_noise_type;     /* Noise generator type */
	UINT8   m_noise_reset;        /* Noise reset */
	UINT8   m_noise_enable;       /* Noise enable */
	UINT16  m_sample_address;     /* Sample address */
	UINT8   m_audio2_voice;       /* Audio 2 voice */
	UINT8   m_audio3_sweep;       /* Audio 3 sweep */
	UINT8   m_audio4_noise;       /* Audio 4 noise */
	UINT8   m_mono;           /* mono */
	UINT8   m_voice_data;     /* voice data */
	UINT8   m_output_volume;      /* output volume */
	UINT8   m_external_stereo;    /* external stereo */
	UINT8   m_external_speaker;   /* external speaker */
	UINT16  m_noise_shift;        /* Noise counter shift register */
	UINT8   m_master_volume;      /* Master volume */
};

extern const device_type WSWAN_SND;

#endif
