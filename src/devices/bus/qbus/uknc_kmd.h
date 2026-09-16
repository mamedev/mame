// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    UKNC floppy controller (device driver MZ.SYS)

***************************************************************************/

#ifndef MAME_BUS_QBUS_UKNC_KMD_H
#define MAME_BUS_QBUS_UKNC_KMD_H

#pragma once

#include "qbus.h"

#include "machine/1801vp128.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> uknc_kmd_device

class uknc_kmd_device : public device_t, public device_qbus_card_interface
{
public:
	// construction/destruction
	uknc_kmd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<k1801vp128_device> m_fdc;
};


// device type definition
DECLARE_DEVICE_TYPE(UKNC_KMD, uknc_kmd_device)

#endif // MAME_BUS_QBUS_UKNC_KMD_H
