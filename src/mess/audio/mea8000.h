/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Philips MEA 8000 emulation.

**********************************************************************/

#ifndef MEA8000_H
#define MEA8000_H

#include "devlegcy.h"

class mea8000_device : public device_t
{
public:
	mea8000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~mea8000_device() { global_free(m_token); }

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

extern const device_type MEA8000;


/* ---------- configuration ------------ */

struct mea8000_interface
{
	/* output channel */
	const char *           channel;

	/* 1-bit 'ready' output, not negated */
	devcb_write8 req_out_func;
};


#define MCFG_MEA8000_ADD(_tag, _intrf)	      \
	MCFG_DEVICE_ADD(_tag, MEA8000, 0)	      \
	MCFG_DEVICE_CONFIG(_intrf)

/* interface to CPU via address/data bus*/
extern DECLARE_READ8_DEVICE_HANDLER  ( mea8000_r );
extern DECLARE_WRITE8_DEVICE_HANDLER ( mea8000_w );

#endif
