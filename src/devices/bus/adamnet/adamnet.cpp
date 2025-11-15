// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco ADAMnet bus emulation

**********************************************************************/

#include "emu.h"
#include "adamnet.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ADAMNET,      adamnet_device,      "adamnet",      "ADAMnet bus")
DEFINE_DEVICE_TYPE(ADAMNET_SLOT, adamnet_slot_device, "adamnet_slot", "ADAMnet slot")



//**************************************************************************
//  DEVICE ADAMNET CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_adamnet_card_interface - constructor
//-------------------------------------------------

device_adamnet_card_interface::device_adamnet_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "adamnet"),
	m_bus(nullptr)
{
}


//-------------------------------------------------
//  ~device_adamnet_card_interface - destructor
//-------------------------------------------------

device_adamnet_card_interface::~device_adamnet_card_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  adamnet_slot_device - constructor
//-------------------------------------------------
adamnet_slot_device::adamnet_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ADAMNET_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_adamnet_card_interface>(mconfig, *this),
	m_bus(*this, finder_base::DUMMY_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adamnet_slot_device::device_start()
{
	device_adamnet_card_interface *dev = get_card_device();
	if (dev) m_bus->add_device(&dev->device());
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  adamnet_device - constructor
//-------------------------------------------------

adamnet_device::adamnet_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ADAMNET, tag, owner, clock),
	m_txd(1),
	m_reset(CLEAR_LINE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adamnet_device::device_start()
{
}


//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void adamnet_device::device_stop()
{
	m_device_list.reset();
}


//-------------------------------------------------
//  add_adamnet_card - add ADAMNET card
//-------------------------------------------------

void adamnet_device::add_device(device_t *target)
{
	auto entry = new daisy_entry(target);

	entry->m_interface->m_bus = this;

	m_device_list.append(*entry);
}


//-------------------------------------------------
//  daisy_entry - constructor
//-------------------------------------------------

adamnet_device::daisy_entry::daisy_entry(device_t *device)
	: m_next(nullptr),
		m_device(device),
		m_interface(nullptr),
		m_txd(1)
{
	device->interface(m_interface);
}


//-------------------------------------------------
//  rxd_r - receive data
//-------------------------------------------------

int adamnet_device::rxd_r()
{
	int state = m_txd;//1;

	daisy_entry *entry = m_device_list.first();

	while (entry)
	{
		state &= entry->m_txd;

		entry = entry->next();
	}

	return state;
}


//-------------------------------------------------
//  rxd_r - receive data
//-------------------------------------------------

int adamnet_device::rxd_r(device_t *device)
{
	int state = m_txd;

	daisy_entry *entry = m_device_list.first();

	while (entry)
	{
		//if (strcmp(entry->m_device->tag(), device->tag()))
		{
			state &= entry->m_txd;
		}

		entry = entry->next();
	}

	return state;
}


//-------------------------------------------------
//  txd_w - transmit data
//-------------------------------------------------

void adamnet_device::txd_w(int state)
{
	if (m_txd != state)
	{
		m_txd = state;
	}
}


//-------------------------------------------------
//  txd_w - transmit data
//-------------------------------------------------

void adamnet_device::txd_w(device_t *device, int state)
{
	daisy_entry *entry = m_device_list.first();

	while (entry)
	{
		if (!strcmp(entry->m_device->tag(), device->tag()))
		{
			if (entry->m_txd != state)
			{
				entry->m_txd = state;
			}
			break;
		}

		entry = entry->next();
	}
}


//-------------------------------------------------
//  reset_r - bus reset
//-------------------------------------------------

int adamnet_device::reset_r()
{
	return m_reset;
}


//-------------------------------------------------
//  reset_w - bus reset
//-------------------------------------------------

void adamnet_device::reset_w(int state)
{
	m_reset = state;

	daisy_entry *entry = m_device_list.first();

	while (entry)
	{
		entry->m_interface->adamnet_reset_w(state);

		entry = entry->next();
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( adamnet_devices )
//-------------------------------------------------

// slot devices
#include "ddp.h"
#include "fdc.h"
#include "kb.h"
#include "printer.h"
#include "spi.h"

void adamnet_devices(device_slot_interface &device)
{
	device.option_add("ddp", ADAM_DDP);
	device.option_add("fdc", ADAM_FDC);
	device.option_add("fdc_320kb", ADAM_FDC_320KB);
	device.option_add("fdc_a720dipi", ADAM_FDC_A720DIPI);
	device.option_add("fdc_fp720at", ADAM_FDC_FP720AT);
	device.option_add("fdc_mihddd", ADAM_FDC_MIHDDD);
	device.option_add("kb", ADAM_KB);
	device.option_add("prn", ADAM_PRN);
	device.option_add("spi", ADAM_SPI);
}
