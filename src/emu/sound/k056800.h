/*********************************************************

    Konami 056800 MIRAC sound interface

*********************************************************/

#ifndef __K056800_H__
#define __K056800_H__

#include "devlegcy.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*k056800_irq_cb)(running_machine &, int);


struct k056800_interface
{
	k056800_irq_cb       irq_cb;
};

class k056800_device : public device_t
{
public:
	k056800_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k056800_device() { global_free(m_token); }

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

extern const device_type K056800;



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_K056800_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K056800, 0) \
	MCFG_DEVICE_CONFIG(_interface)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

DECLARE_READ32_DEVICE_HANDLER( k056800_host_r );
DECLARE_WRITE32_DEVICE_HANDLER( k056800_host_w );
DECLARE_READ16_DEVICE_HANDLER( k056800_sound_r );
DECLARE_WRITE16_DEVICE_HANDLER( k056800_sound_w );


#endif /* __K056800_H__ */


