/*****************************************************************************
 *
 * audio/t6721.h
 *
 ****************************************************************************/

#ifndef __T6721_H__
#define __T6721_H__

#include "devcb.h"
/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

class t6721_device : public device_t
{
public:
	t6721_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~t6721_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type T6721;


#define MCFG_T6721_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, T6721, 0)

/*----------- defined in audio/t6721.c -----------*/

DECLARE_WRITE8_DEVICE_HANDLER(t6721_speech_w);
DECLARE_READ8_DEVICE_HANDLER(t6721_speech_r);


#endif /* __TED7360_H__ */
