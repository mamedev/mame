/**  Konami 053252  **/
/* CRT and interrupt control unit */
#pragma once

#ifndef __K053252_H__
#define __K053252_H__

#include "devlegcy.h"

class k053252_device : public device_t
{
public:
	k053252_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k053252_device() { global_free(m_token); }

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

extern const device_type K053252;




struct k053252_interface
{
	const char         *screen;
	devcb_write_line   int1_en;
	devcb_write_line   int2_en;
	devcb_write_line   int1_ack;
	devcb_write_line   int2_ack;
//  devcb_write8       int_time;
	int                offsx;
	int                offsy;
};


#define MCFG_K053252_ADD(_tag, _clock, _interface) \
	MCFG_DEVICE_ADD(_tag, K053252, _clock) \
	MCFG_DEVICE_CONFIG(_interface)

/**  Konami 053252  **/
/* CRT and interrupt control unit */
DECLARE_READ8_DEVICE_HANDLER( k053252_r );  // CCU registers
DECLARE_WRITE8_DEVICE_HANDLER( k053252_w );



#endif  /* __K033906_H__ */
