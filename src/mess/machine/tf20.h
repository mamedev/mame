/***************************************************************************

    Epson TF-20

    Dual floppy drive with HX-20 factory option

***************************************************************************/

#ifndef __TF20_H__
#define __TF20_H__

#include "devcb.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

#if 0
struct tf20_interface
{
};
#endif


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* serial interface in (to the host computer) */
WRITE_LINE_DEVICE_HANDLER( tf20_txs_w );
READ_LINE_DEVICE_HANDLER( tf20_rxs_r );
WRITE_LINE_DEVICE_HANDLER( tf20_pouts_w );
READ_LINE_DEVICE_HANDLER( tf20_pins_r );

#ifdef UNUSED_FUNCTION
/* serial interface out (to another terminal) */
WRITE_LINE_DEVICE_HANDLER( tf20_txc_r );
READ_LINE_DEVICE_HANDLER( tf20_rxc_w );
WRITE_LINE_DEVICE_HANDLER( tf20_poutc_r );
READ_LINE_DEVICE_HANDLER( tf20_pinc_w );
#endif

INPUT_PORTS_EXTERN( tf20 );


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

class tf20_device : public device_t
{
public:
	tf20_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tf20_device() { global_free(m_token); }

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

extern const device_type TF20;


#define MCFG_TF20_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TF20, 0) \


#endif /* __TF20_H__ */
