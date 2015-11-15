// license:BSD-3-Clause
// copyright-holders:smf
/*
* fdc37c665gt.h
*
*/

#ifndef _FDC37C665GT_H_
#define _FDC37C665GT_H_

#pragma once

#include "ins8250.h"

class fdc37c665gt_device : public device_t
{
public:
	// construction/destruction
	fdc37c665gt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start();
	virtual machine_config_constructor device_mconfig_additions() const;

private:
	required_device<ns16550_device> m_uart1;
	required_device<ns16550_device> m_uart2;
};

// device type definition
extern const device_type FDC37C665GT;

#endif
