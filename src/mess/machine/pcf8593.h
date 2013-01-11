/*********************************************************************

    Philips PCF8593 CMOS clock/calendar circuit

    (c) 2001-2007 Tim Schuerewegen

*********************************************************************/

#ifndef __PCF8593_H__
#define __PCF8593_H__

#include "emu.h"


/***************************************************************************
    MACROS
***************************************************************************/

class pcf8593_device : public device_t
{
public:
	pcf8593_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~pcf8593_device() { global_free(m_token); }

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

extern const device_type PCF8593;


#define MCFG_PCF8593_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PCF8593, 0)
#define MCFG_PCF8593_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
/* pins */
void pcf8593_pin_scl(device_t *device, int data);
void pcf8593_pin_sda_w(device_t *device, int data);
int  pcf8593_pin_sda_r(device_t *device);

/* load/save */
void pcf8593_load(device_t *device, emu_file *file);
void pcf8593_save(device_t *device, emu_file *file);

#endif /* __PCF8593_H__ */
