// license:BSD-3-Clause
// copyright-holders:Dave Rand
/**********************************************************************

    Xerox 820-II disk-controller daughterboard slot

**********************************************************************/

#include "emu.h"
#include "dbslot.h"

#include "fdc.h"
#include "sasi.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(XEROX820_DBSLOT, xerox820_dbslot_device, "xerox820_dbslot", "Xerox 820-II disk daughterboard slot")


//**************************************************************************
//  device_xerox820_dbslot_card_interface
//**************************************************************************

device_xerox820_dbslot_card_interface::device_xerox820_dbslot_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "xerox820dbslot")
	, m_slot(dynamic_cast<xerox820_dbslot_device *>(device.owner()))
{
}

// the card device types are exposed through this interface via
// DEFINE_DEVICE_TYPE_PRIVATE, which does not instantiate the device_finder
// templates; instantiate them here so required_device/optional_device of the
// interface resolve rather than producing undefined symbol errors
template class device_finder<device_xerox820_dbslot_card_interface, false>;
template class device_finder<device_xerox820_dbslot_card_interface, true>;


//**************************************************************************
//  xerox820_dbslot_device
//**************************************************************************

xerox820_dbslot_device::xerox820_dbslot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XEROX820_DBSLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_xerox820_dbslot_card_interface>(mconfig, *this)
	, device_z80daisy_interface(mconfig, *this)
	, m_intrq_cb(*this)
	, m_drq_cb(*this)
	, m_int_cb(*this)
	, m_card(nullptr)
{
}

void xerox820_dbslot_device::device_start()
{
	m_card = get_card_device();
}


//**************************************************************************
//  SLOT OPTIONS
//**************************************************************************

void xerox820_dbslot_cards(device_slot_interface &device)
{
	device.option_add("fdc", XEROX820_FDC);             // FD1797, 8" drives
	device.option_add("fdc5", XEROX820_FDC5);           // FD1797, 5.25" drives
	device.option_add("fdcbox5", XEROX820_FDC_BOX5);    // FD1797 + 16/8 RX024 5.25" box
	device.option_add("sasi", XEROX820_SASI);           // SA1403D, 8" floppies + ST-506 rigid
}
