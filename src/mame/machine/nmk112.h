/*************************************************************************

    nmk112.h

**************************************************************************/

#ifndef __NMK112_H__
#define __NMK112_H__

#include "devlegcy.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct nmk112_interface
{
	const char *rgn0, *rgn1;
	UINT8 disable_page_mask;
};

class nmk112_device : public device_t
{
public:
	nmk112_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~nmk112_device() { global_free(m_token); }

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

extern const device_type NMK112;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_NMK112_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, NMK112, 0) \
	MCFG_DEVICE_CONFIG(_interface)

/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

WRITE8_DEVICE_HANDLER( nmk112_okibank_w );
WRITE16_DEVICE_HANDLER( nmk112_okibank_lsb_w );


#endif /* __NMK112_H__ */
