// license:BSD-3-Clause
// copyright-holders:hap
/**********************************************************************

    Texas Instruments TMS1024, TMS1025 I/O expander emulation

**********************************************************************





**********************************************************************/

#ifndef _TMS1024_H_
#define _TMS1024_H_

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TMS1024_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TMS1024, 0)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tms1024_device

class tms1024_device : public device_t
{
public:
	tms1024_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
};

// device type definition
extern const device_type TMS1024;


#endif /* _TMS1024_H_ */
