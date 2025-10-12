// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Loopback cart for ROM test routines

***************************************************************************/

#ifndef MAME_BUS_BK_LOOPBACK_H
#define MAME_BUS_BK_LOOPBACK_H

#pragma once

#include "parallel.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bk_loopback_device

class bk_loopback_device : public device_t, public device_bk_parallel_interface
{
public:
	// construction/destruction
	bk_loopback_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override { m_data = 0; };

	virtual uint16_t io_r() override;
	virtual void io_w(uint16_t data, bool word) override;

private:
	uint16_t m_data;
};

// device type definition
DECLARE_DEVICE_TYPE(BK_LOOPBACK, bk_loopback_device)

#endif // MAME_BUS_BK_LOOPBACK_H
