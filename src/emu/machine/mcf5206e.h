/***************************************************************************

    Konami 033906

***************************************************************************/

#pragma once

#ifndef __MCF5206E_PERIPHERAL_H__
#define __MCF5206E_PERIPHERAL_H__

#include "emu.h"



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_MCF5206E_PERIPHERAL_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MCF5206E_PERIPHERAL, 0) \


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/



// ======================> mcf5206e_peripheral_device

class mcf5206e_peripheral_device :  public device_t
{
public:
	// construction/destruction
	mcf5206e_peripheral_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ32_MEMBER( dev_r );
	DECLARE_WRITE32_MEMBER( dev_w );
	DECLARE_READ32_MEMBER( seta2_coldfire_regs_r );
	DECLARE_WRITE32_MEMBER( seta2_coldfire_regs_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset() { }
	virtual void device_post_load() { }
	virtual void device_clock_changed() { }
	UINT32 m_coldfire_regs[0x400/4];

private:
};


// device type definition
extern const device_type MCF5206E_PERIPHERAL;

#endif  /* __MCF5206E_PERIPHERAL_H__ */
