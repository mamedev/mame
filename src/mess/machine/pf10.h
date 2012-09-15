/***************************************************************************

    Epson PF-10

    Serial floppy drive

***************************************************************************/

#ifndef __PF10_H__
#define __PF10_H__

#include "devcb.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

#if 0
struct pf10_interface
{
};
#endif


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
/* serial interface in (to the host computer) */
READ_LINE_DEVICE_HANDLER( pf10_txd1_r );
WRITE_LINE_DEVICE_HANDLER( pf10_rxd1_w );

/* serial interface out (to another floppy drive) */
READ_LINE_DEVICE_HANDLER( pf10_txd2_r );
WRITE_LINE_DEVICE_HANDLER( pf10_rxd2_w );


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

class pf10_device : public device_t
{
public:
	pf10_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~pf10_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
private:
	// internal state
	void *m_token;
};

extern const device_type PF10;


#define MCFG_PF10_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PF10, 0) \


#endif /* __PF10_H__ */
