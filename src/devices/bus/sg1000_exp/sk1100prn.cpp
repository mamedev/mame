// license:BSD-3-Clause
// copyright-holders:Enik Land
/**********************************************************************

    Sega SK-1100 keyboard printer port emulation

**********************************************************************/

#include "emu.h"
#include "sk1100prn.h"
// slot devices
#include "sp400.h"
#include "kblink.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SK1100_PRINTER_PORT, sk1100_printer_port_device, "sk1100_printer_port", "Sega SK-1100 Printer Port")



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_sk1100_printer_port_interface - constructor
//-------------------------------------------------

device_sk1100_printer_port_interface::device_sk1100_printer_port_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "sk1000prn")
{
}


//-------------------------------------------------
//  ~device_sk1100_printer_port_interface - destructor
//-------------------------------------------------

device_sk1100_printer_port_interface::~device_sk1100_printer_port_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sk1100_printer_port_device - constructor
//-------------------------------------------------

sk1100_printer_port_device::sk1100_printer_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SK1100_PRINTER_PORT, tag, owner, clock),
	device_single_card_slot_interface<device_sk1100_printer_port_interface>(mconfig, *this),
	m_device(nullptr)
{
}


//-------------------------------------------------
//  sk1100_printer_port_device - destructor
//-------------------------------------------------

sk1100_printer_port_device::~sk1100_printer_port_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sk1100_printer_port_device::device_start()
{
	m_device = get_card_device();
}

void sk1100_printer_port_device::data_w(int state)
{
	if (m_device)
		m_device->input_data(state);
}

void sk1100_printer_port_device::reset_w(int state)
{
	if (m_device)
		m_device->input_reset(state);
}

void sk1100_printer_port_device::feed_w(int state)
{
	if (m_device)
		m_device->input_feed(state);
}

int sk1100_printer_port_device::fault_r()
{
	if (m_device)
		return m_device->output_fault();
	else
		return 1;
}

int sk1100_printer_port_device::busy_r()
{
	if (m_device)
		return m_device->output_busy();
	else
		return 1;
}

//-------------------------------------------------
//  SLOT_INTERFACE( sk1100_printer_port_devices )
//-------------------------------------------------

void sk1100_printer_port_devices(device_slot_interface &device)
{
	device.option_add("sp400", SP400_PRINTER); /* serial printer */
	device.option_add("kblink", SK1100_LINK_CABLE);
}
