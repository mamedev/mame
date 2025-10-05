// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Printer interface

***************************************************************************/

#ifndef MAME_BUS_BK_PRINTER_H
#define MAME_BUS_BK_PRINTER_H

#pragma once

#include "parallel.h"

#include "bus/centronics/ctronics.h"
#include "machine/output_latch.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bk_printer_device

class bk_printer_device : public device_t, public device_bk_parallel_interface
{
public:
	// construction/destruction
	bk_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override { m_data = 0; };

	virtual uint16_t io_r() override;
	virtual void io_w(uint16_t data, bool word) override;

private:
	virtual void device_add_mconfig(machine_config &config) override;

	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;

	uint16_t m_data;
};

// device type definition
DECLARE_DEVICE_TYPE(BK_PRINTER, bk_printer_device)

#endif // MAME_BUS_BK_PRINTER_H
