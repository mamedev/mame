/*********************************************************

    Konami 053260 PCM/ADPCM Sound Chip

*********************************************************/

#pragma once

#ifndef __K053260_H__
#define __K053260_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_K053260_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, K053260, _clock)
#define MCFG_K053260_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, K053260, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct k053260_interface
{
	const char *rgnoverride;
};


struct k053260_channel
{
	k053260_channel() :
		rate(0),
		size(0),
		start(0),
		bank(0),
		volume(0),
		play(0),
		pan(0),
		pos(0),
		loop(0),
		ppcm(0),
		ppcm_data(0) {}

	UINT32      rate;
	UINT32      size;
	UINT32      start;
	UINT32      bank;
	UINT32      volume;
	int         play;
	UINT32      pan;
	UINT32      pos;
	int         loop;
	int         ppcm; /* packed PCM ( 4 bit signed ) */
	int         ppcm_data;
};


// ======================> k053260_device

class k053260_device : public device_t,
						public device_sound_interface
{
public:
	k053260_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k053260_device() { }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

public:
	DECLARE_WRITE8_MEMBER( k053260_w );
	DECLARE_READ8_MEMBER( k053260_r );

private:
	void InitDeltaTable( int rate, int clock );
	void check_bounds( int channel );

private:
	sound_stream *              m_channel;
	int                         m_mode;
	int                         m_regs[0x30];
	UINT8                       *m_rom;
	int                         m_rom_size;
	UINT32                      *m_delta_table;
	k053260_channel             m_channels[4];
	const k053260_interface     *m_intf;
};

extern const device_type K053260;


#endif /* __K053260_H__ */
