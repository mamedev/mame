/*************************************************************************

    decocomn.h

**************************************************************************/

#pragma once
#ifndef __DECOCOMN_H__
#define __DECOCOMN_H__

#include "devcb.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


struct decocomn_interface
{
	const char         *screen;
};

class decocomn_device : public device_t
{
public:
	decocomn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~decocomn_device() { global_free(m_token); }

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

extern const device_type DECOCOMN;



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_DECOCOMN_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, DECOCOMN, 0) \
	MCFG_DEVICE_CONFIG(_interface)

/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

WRITE16_DEVICE_HANDLER( decocomn_nonbuffered_palette_w );
WRITE16_DEVICE_HANDLER( decocomn_buffered_palette_w );
WRITE16_DEVICE_HANDLER( decocomn_palette_dma_w );

WRITE16_DEVICE_HANDLER( decocomn_priority_w );
READ16_DEVICE_HANDLER( decocomn_priority_r );

READ16_DEVICE_HANDLER( decocomn_71_r );

#endif
