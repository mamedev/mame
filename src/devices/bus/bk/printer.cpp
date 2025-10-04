// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Printer interface for ROM BASIC printer driver and compatibles

    Simulates IRPR protocol (inverted data lines, no /ACK signal).

    OSBK 4.1 settings: SET LP IRPR,BK10,NOTRANSL ; TIMER.SAV OFF

***************************************************************************/

#include "emu.h"
#include "printer.h"


//**************************************************************************
//  CONSTANTS/MACROS
//**************************************************************************

#define VERBOSE 0


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BK_PRINTER, bk_printer_device, "bk_printer", "Printer Interface")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bk_printer_device - constructor
//-------------------------------------------------

bk_printer_device::bk_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BK_PRINTER, tag, owner, clock)
	, device_bk_parallel_interface(mconfig, *this)
	, m_centronics(*this, "centronics")
	, m_cent_data_out(*this, "cent_data_out")
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bk_printer_device::device_add_mconfig(machine_config &config)
{
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->set_output_latch(OUTPUT_LATCH(config, m_cent_data_out));
	m_centronics->busy_handler().set([this] (int state) { m_data = (state << 8); });
}

void bk_printer_device::device_start()
{
	save_item(NAME(m_data));
}

uint16_t bk_printer_device::io_r()
{
	return m_data;
}

void bk_printer_device::io_w(uint16_t data, bool word)
{
	if (!BIT(data, 8)) m_cent_data_out->write(~data);
	m_centronics->write_strobe(BIT(data, 8));
}
