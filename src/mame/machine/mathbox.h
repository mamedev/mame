/*
 * mathbox.h: math box simulation (Battlezone/Red Baron/Tempest)
 *
 * Copyright Eric Smith
 *
 */

#include "devlegcy.h"

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_MATHBOX_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MATHBOX, 0)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

WRITE8_DEVICE_HANDLER( mathbox_go_w );
READ8_DEVICE_HANDLER( mathbox_status_r );
READ8_DEVICE_HANDLER( mathbox_lo_r );
READ8_DEVICE_HANDLER( mathbox_hi_r );

/* ----- device interface ----- */
class mathbox_device : public device_t
{
public:
	mathbox_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~mathbox_device() { global_free(m_token); }

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

extern const device_type MATHBOX;

