#pragma once

#ifndef __TMS6100_H__
#define __TMS6100_H__

#include "devlegcy.h"

/* TMS 6100 memory controller */

WRITE_LINE_DEVICE_HANDLER( tms6100_m0_w );
WRITE_LINE_DEVICE_HANDLER( tms6100_m1_w );
WRITE_LINE_DEVICE_HANDLER( tms6100_romclock_w );
DECLARE_WRITE8_DEVICE_HANDLER( tms6100_addr_w );

READ_LINE_DEVICE_HANDLER( tms6100_data_r );

class tms6100_device : public device_t
{
public:
	tms6100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms6100_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~tms6100_device() { global_free(m_token); }

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

extern const device_type TMS6100;

class m58819_device : public tms6100_device
{
public:
	m58819_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type M58819;


#endif /* __TMS6100_H__ */
