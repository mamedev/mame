/*************************************************************************

    SMC91C9X ethernet controller implementation

    by Aaron Giles

**************************************************************************/

#ifndef __SMC91C9X__
#define __SMC91C9X__

#include "devlegcy.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*smc91c9x_irq_func)(device_t *device, int state);


struct smc91c9x_config
{
	smc91c9x_irq_func   interrupt;
};



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_SMC91C94_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, SMC91C94, 0) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_SMC91C96_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, SMC91C96, 0) \
	MCFG_DEVICE_CONFIG(_config)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

DECLARE_READ16_DEVICE_HANDLER( smc91c9x_r );
DECLARE_WRITE16_DEVICE_HANDLER( smc91c9x_w );


/* ----- device interface ----- */
class smc91c9x_device : public device_t
{
public:
	smc91c9x_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~smc91c9x_device() { global_free(m_token); }

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


class smc91c94_device : public smc91c9x_device
{
public:
	smc91c94_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type SMC91C94;

class smc91c96_device : public smc91c9x_device
{
public:
	smc91c96_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type SMC91C96;


#endif
