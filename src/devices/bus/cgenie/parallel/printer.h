// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie Printer Interface EG2012

***************************************************************************/

#pragma once

#ifndef __CGENIE_PARALLEL_PRINTER_H__
#define __CGENIE_PARALLEL_PRINTER_H__

#include "emu.h"
#include "parallel.h"
#include "bus/centronics/ctronics.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cgenie_printer_device

class cgenie_printer_device : public device_t, public device_parallel_interface
{
public:
	// construction/destruction
	cgenie_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER(busy_w);
	DECLARE_WRITE_LINE_MEMBER(perror_w);
	DECLARE_WRITE_LINE_MEMBER(select_w);
	DECLARE_WRITE_LINE_MEMBER(fault_w);

protected:
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_start();
	virtual void device_reset();

	virtual void pa_w(UINT8 data);
	virtual UINT8 pb_r();
	virtual void pb_w(UINT8 data);

private:
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_latch;

	int m_centronics_busy;
	int m_centronics_out_of_paper;
	int m_centronics_unit_sel;
	int m_centronics_ready;
};

// device type definition
extern const device_type CGENIE_PRINTER;

#endif // __CGENIE_PARALLEL_PRINTER_H__
