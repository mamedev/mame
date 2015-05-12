// license:BSD-3-Clause
// copyright-holders:smf
#ifndef __DS128X_H__
#define __DS128X_H__

#include "mc146818.h"

#define MCFG_DS12885_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DS12885, XTAL_32_768kHz)

// ======================> mc146818_device

class ds12885_device : public mc146818_device
{
public:
	// construction/destruction
	ds12885_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual int data_size() { return 128; }
};

// device type definition
extern const device_type DS12885;

#endif
