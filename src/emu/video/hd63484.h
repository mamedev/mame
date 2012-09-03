/*************************************************************************

  HD63484 ACRTC
  Advanced CRT Controller.

**************************************************************************/

#ifndef __HD63484_H__
#define __HD63484_H__

#include "devcb.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class hd63484_device : public device_t
{
public:
	hd63484_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~hd63484_device() { global_free(m_token); }

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

extern const device_type HD63484;


#define HD63484_RAM_SIZE 0x100000

typedef struct _hd63484_interface hd63484_interface;
struct _hd63484_interface
{
	int        skattva_hack;
};

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_HD63484_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, HD63484, 0) \
	MCFG_DEVICE_CONFIG(_interface)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

READ16_DEVICE_HANDLER( hd63484_status_r );
WRITE16_DEVICE_HANDLER( hd63484_address_w );
WRITE16_DEVICE_HANDLER( hd63484_data_w );
READ16_DEVICE_HANDLER( hd63484_data_r );

READ16_DEVICE_HANDLER( hd63484_ram_r );
READ16_DEVICE_HANDLER( hd63484_regs_r );
WRITE16_DEVICE_HANDLER( hd63484_ram_w );
WRITE16_DEVICE_HANDLER( hd63484_regs_w );

#endif /* __HD63484_H__ */


