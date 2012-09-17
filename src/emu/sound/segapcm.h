/*********************************************************/
/*    SEGA 8bit PCM                                      */
/*********************************************************/

#pragma once

#ifndef __SEGAPCM_H__
#define __SEGAPCM_H__

#include "devlegcy.h"

#define   BANK_256    (11)
#define   BANK_512    (12)
#define   BANK_12M    (13)
#define   BANK_MASK7    (0x70<<16)
#define   BANK_MASKF    (0xf0<<16)
#define   BANK_MASKF8   (0xf8<<16)

struct sega_pcm_interface
{
	int  bank;
};

DECLARE_WRITE8_DEVICE_HANDLER( sega_pcm_w );
DECLARE_READ8_DEVICE_HANDLER( sega_pcm_r );

class segapcm_device : public device_t,
                                  public device_sound_interface
{
public:
	segapcm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~segapcm_device() { global_free(m_token); }

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

extern const device_type SEGAPCM;


#endif /* __SEGAPCM_H__ */
