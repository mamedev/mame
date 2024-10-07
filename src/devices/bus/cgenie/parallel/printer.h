// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie Printer Interface EG2012

***************************************************************************/

#ifndef MAME_BUS_CGENIE_PARALLEL_PRINTER_H
#define MAME_BUS_CGENIE_PARALLEL_PRINTER_H

#pragma once

#include "parallel.h"
#include "bus/centronics/ctronics.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cgenie_printer_device

class cgenie_printer_device : public device_t, public device_cg_parallel_interface
{
public:
	// construction/destruction
	cgenie_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void pa_w(uint8_t data) override;
	virtual uint8_t pb_r() override;
	virtual void pb_w(uint8_t data) override;

private:
	void busy_w(int state);
	void perror_w(int state);
	void select_w(int state);
	void fault_w(int state);

	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_latch;

	int m_centronics_busy;
	int m_centronics_out_of_paper;
	int m_centronics_unit_sel;
	int m_centronics_ready;
};

// device type definition
DECLARE_DEVICE_TYPE(CGENIE_PRINTER, cgenie_printer_device)

#endif // MAME_BUS_CGENIE_PARALLEL_PRINTER_H
