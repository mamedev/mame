/***************************************************************************

    RP5H01


***************************************************************************/

#ifndef __RP5H01_H__
#define __RP5H01_H__

#include "devlegcy.h"

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class rp5h01_device : public device_t
{
public:
	rp5h01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~rp5h01_device() { global_free(m_token); }

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

extern const device_type RP5H01;


#define MCFG_RP5H01_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, RP5H01, 0)

/*
 * Device uses memory region
 * with the same tag as the one
 * assigned to device.
 */

/***************************************************************************
    PROTOTYPES
***************************************************************************/

DECLARE_WRITE8_DEVICE_HANDLER( rp5h01_enable_w );	/* /CE */
DECLARE_WRITE8_DEVICE_HANDLER( rp5h01_reset_w );	/* RESET */
DECLARE_WRITE8_DEVICE_HANDLER( rp5h01_cs_w );	/* CS */
DECLARE_WRITE8_DEVICE_HANDLER( rp5h01_clock_w );	/* DATA CLOCK (active low) */
DECLARE_WRITE8_DEVICE_HANDLER( rp5h01_test_w );		/* TEST */
DECLARE_READ8_DEVICE_HANDLER( rp5h01_counter_r );	/* COUNTER OUT */
DECLARE_READ8_DEVICE_HANDLER( rp5h01_data_r );		/* DATA */

#endif /* __RP5H01_H__ */
