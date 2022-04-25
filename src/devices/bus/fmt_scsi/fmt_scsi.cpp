// license:BSD-3-Clause
// copyright-holders:r09
/****************************************************************************

    Fujitsu FM Towns SCSI card slot

    This is a dedicated 30-pin slot for the FMT-121 SCSI Card. It is only
    present on the Model 1 and 2; all later models integrate the SCSI
    controller directly on the motherboard.

****************************************************************************/

#include "emu.h"
#include "fmt_scsi.h"

#include "fmt121.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(FMT_SCSI_SLOT, fmt_scsi_slot_device, "fmt_scsi_slot", "FM Towns SCSI card slot")

//**************************************************************************
//  FMT_SCSI SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  fmt_scsi_slot_device - construction
//-------------------------------------------------

fmt_scsi_slot_device::fmt_scsi_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, FMT_SCSI_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<fmt_scsi_card_interface>(mconfig, *this)
	, m_card(nullptr)
	, m_irq_handler(*this)
	, m_drq_handler(*this)
{
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void fmt_scsi_slot_device::device_resolve_objects()
{
	m_card = get_card_device();
	if (m_card != nullptr)
		m_card->m_slot = this;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void fmt_scsi_slot_device::device_start()
{
	m_irq_handler.resolve_safe();
	m_drq_handler.resolve_safe();
}


//-------------------------------------------------
//  read - I/O read access
//-------------------------------------------------

u8 fmt_scsi_slot_device::read(address_space &space, offs_t offset)
{
	if (m_card)
		return m_card->fmt_scsi_read(offset);
	else
		return space.unmap();
}


//-------------------------------------------------
//  write - I/O write access
//-------------------------------------------------

void fmt_scsi_slot_device::write(offs_t offset, u8 data)
{
	if (m_card)
		m_card->fmt_scsi_write(offset, data);
}

//-------------------------------------------------
//  data_read - data read access
//-------------------------------------------------

u8 fmt_scsi_slot_device::data_read()
{
	if (m_card)
		return m_card->fmt_scsi_data_read();
	else
		return 0;
}

//-------------------------------------------------
//  data_write - data write access
//-------------------------------------------------

void fmt_scsi_slot_device::data_write(u8 data)
{
	if (m_card)
		m_card->fmt_scsi_data_write(data);
}

void fmt_scsi_slot_device::irq_w(int state)
{
	m_irq_handler(state);
}

void fmt_scsi_slot_device::drq_w(int state)
{
	m_drq_handler(state);
}

//**************************************************************************
//  FMT_SCSI CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  fmt_scsi_card_interface - construction
//-------------------------------------------------

fmt_scsi_card_interface::fmt_scsi_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "fmtscsicard")
	, m_slot(nullptr)
{
}


//-------------------------------------------------
//  interface_pre_start - called before the
//  device's own start function
//-------------------------------------------------

void fmt_scsi_card_interface::interface_pre_start()
{
	if (!m_slot)
		throw device_missing_dependencies();
}


//-------------------------------------------------
//  fmt_scsi_default_devices - add standard options
//  for main slots
//-------------------------------------------------

void fmt_scsi_default_devices(device_slot_interface &device)
{
	device.option_add("fmt121", FMT121);
}
