// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    Alpha Denshi ALPHA-8201 family protection emulation

***************************************************************************/

#ifndef _ALPHA8201_H_
#define _ALPHA8201_H_

#include "emu.h"

class alpha_8201_device : public device_t
{
public:
	alpha_8201_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~alpha_8201_device() {}

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

private:
	// devices/pointers
	required_device<cpu_device> m_mcu;

	// internal state
	UINT8* m_shared_ram;
};


extern const device_type ALPHA_8201;


#endif /* _ALPHA8201_H_ */
