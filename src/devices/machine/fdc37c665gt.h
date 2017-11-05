// license:BSD-3-Clause
// copyright-holders:smf
/*
* fdc37c665gt.h
*
*/

#ifndef MAME_MACHINE_FDC37C665GT_H
#define MAME_MACHINE_FDC37C665GT_H

#pragma once

#include "ins8250.h"

class fdc37c665gt_device : public device_t
{
public:
	// construction/destruction
	fdc37c665gt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<ns16550_device> m_uart1;
	required_device<ns16550_device> m_uart2;
};

// device type definition
DECLARE_DEVICE_TYPE(FDC37C665GT, fdc37c665gt_device)

#endif // MAME_MACHINE_FDC37C665GT_H
