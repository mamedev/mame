/*********************************************************

    Capcom Q-Sound system

*********************************************************/

#pragma once

#ifndef __QSOUND_H__
#define __QSOUND_H__

#define QSOUND_CLOCK 4000000    /* default 4MHz clock */

#define QSOUND_CLOCKDIV 166     /* Clock divider */
#define QSOUND_CHANNELS 16
typedef INT8 QSOUND_SRC_SAMPLE; /* 8 bit source ROM samples */
typedef stream_sample_t QSOUND_SAMPLE;


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_QSOUND_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, QSOUND, _clock)
#define MCFG_QSOUND_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, QSOUND, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct QSOUND_CHANNEL
{
	INT32 bank;     // bank (x16)
	INT32 address;  // start address
	INT32 pitch;    // pitch
	INT32 reg3;     // unknown (always 0x8000)
	INT32 loop;     // loop address
	INT32 end;      // end address
	INT32 vol;      // master volume
	INT32 pan;      // Pan value
	INT32 reg9;     // unknown

	/* Work variables */
	INT32 key;      // Key on / key off
	INT32 lvol;     // left volume
	INT32 rvol;     // right volume
	INT32 lastdt;   // last sample value
	INT32 offset;   // current offset counter
};


// ======================> qsound_device

class qsound_device : public device_t,
						public device_sound_interface
{
public:
	qsound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~qsound_device() { }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_stop();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

public:
	DECLARE_WRITE8_MEMBER( qsound_w );
	DECLARE_READ8_MEMBER( qsound_r );

private:
	void qsound_set_command(int data, int value);

private:
	int m_data;                 // register latch data
	sound_stream *m_stream;     // Audio stream
	QSOUND_CHANNEL m_channel[QSOUND_CHANNELS];
	UINT32 m_sample_rom_length;
	QSOUND_SRC_SAMPLE *m_sample_rom;    // Q sound sample ROM

	int m_pan_table[33];    // Pan volume table
	float m_frq_ratio;      // Frequency ratio

	FILE *m_fpRawDataL;
	FILE *m_fpRawDataR;
};

extern const device_type QSOUND;


#endif /* __QSOUND_H__ */
