// license:BSD-3-Clause
// copyright-holders:tim lindner
/*********************************************************************

    mc10cart.cpp

    MC-10 / Alice expansion slot - typically used for the 16K external
    RAM expansion.

         MC-10 and Alice pinout listing
       ---  -------           ---  -------
         1  GND                18  A6
         2  GND                19  A7
         3  D0                 20  A8
         4  D1                 21  A9
         5  D2                 22  A10
         6  D3                 23  A11
         7  D4                 24  A12
         8  D5                 25  A13
         9  D6                 26  A14
        10  D7                 27  A15
        11  R/!W               28  E
        12  A0                 29  SEL
        13  A1                 30  RESET
        14  A2                 31  !NMI
        15  A3                 32  +5V
        16  A3                 33  GND
        17  A5                 34  GDN


*********************************************************************/

#include "emu.h"
#include "mc10cart.h"

#include "mc10_ram.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//#define LOG_GENERAL   (1U << 0) //defined in logmacro.h already
#define LOG_NMI  (1U << 2) // shows switch changes
// #define VERBOSE (LOG_CART)

#include "logmacro.h"

#define LOGNMI(...)  LOGMASKED(LOG_NMI,  __VA_ARGS__)


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MC10CART_SLOT, mc10cart_slot_device, "mc10cart_slot", "MC-10 Cartridge Slot")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mc10cart_slot_device - constructor
//-------------------------------------------------
mc10cart_slot_device::mc10cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, MC10CART_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_mc10cart_interface>(mconfig, *this),
	device_image_interface(mconfig, *this),
	m_nmi_callback(*this),
	m_cart(nullptr),
	m_memspace(*this, finder_base::DUMMY_TAG, -1)
{
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc10cart_slot_device::device_start()
{
	m_nmi_line.value = line_value::CLEAR;
	m_nmi_callback.resolve();
	m_nmi_line.callback = &m_nmi_callback;

	m_cart = get_card_device();
}



//-------------------------------------------------
//  install_bank - install ram in host computer
//-------------------------------------------------

void mc10cart_slot_device::install_bank(offs_t start, offs_t end, uint8_t *data)
{
	m_memspace->install_ram(start, end, data);
}



//-------------------------------------------------
//  set_line
//-------------------------------------------------

void mc10cart_slot_device::set_line(line ln, mc10_cartridge_line &line, mc10cart_slot_device::line_value value)
{
	if ((line.value != value))
	{
		line.value = value;

		switch (ln)
		{
		case line::NMI:
			LOGNMI( "set_line: NMI, value: %s\n", value == line_value::CLEAR ? "CLEAR" : "ASSERT");
			break;
		}

		/* invoke the callback, if present */
		if (!(*line.callback).isnull())
			(*line.callback)(line.line);
	}
}




//-------------------------------------------------
//  set_line_value
//-------------------------------------------------

void mc10cart_slot_device::set_line_value(mc10cart_slot_device::line which, mc10cart_slot_device::line_value value)
{
	switch (which)
	{
	case mc10cart_slot_device::line::NMI:
		set_line(line::NMI, m_nmi_line, value);
		break;
	}
}


//-------------------------------------------------
//  get_line_value
//-------------------------------------------------

mc10cart_slot_device::line_value mc10cart_slot_device::get_line_value(mc10cart_slot_device::line which) const
{
	line_value result;
	switch (which)
	{
	case mc10cart_slot_device::line::NMI:
		result = m_nmi_line.value;
		break;
	default:
		result = line_value::CLEAR;
		break;
	}

	return result;
}


//-------------------------------------------------
//  call_load
//-------------------------------------------------

image_init_result mc10cart_slot_device::call_load()
{
// 	if (m_cart)
// 	{
// 		memory_region *cart_mem = m_cart->get_cart_memregion();
// 		u8 *base = cart_mem->base();
// 		offs_t read_length, cart_length = cart_mem->bytes();
//
// 		if (!loaded_through_softlist())
// 		{
// 			read_length = fread(base, cart_length);
// 		}
// 		else
// 		{
// 			read_length = get_software_region_length("rom");
// 			memcpy(base, get_software_region("rom"), read_length);
// 		}
//
// 		while (read_length < cart_length)
// 		{
// 			offs_t len = std::min(read_length, cart_length - read_length);
// 			memcpy(base + read_length, base, len);
// 			read_length += len;
// 		}
// 	}
// 	return image_init_result::PASS;
 	return image_init_result::FAIL;
}


//-------------------------------------------------
//  get_default_card_software
//-------------------------------------------------

std::string mc10cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("pak");
}



//**************************************************************************
//  DEVICE MC-10 CART INTERFACE - Implemented by devices that plug into
//  MC-10 cartridge slots
//**************************************************************************

template class device_finder<device_mc10cart_interface, false>;
template class device_finder<device_mc10cart_interface, true>;

//-------------------------------------------------
//  device_mc10cart_interface - constructor
//-------------------------------------------------

device_mc10cart_interface::device_mc10cart_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "mc10cart")
	, m_owning_slot(nullptr)
	, m_host(nullptr)
{
}


//-------------------------------------------------
//  ~device_mc10cart_interface - destructor
//-------------------------------------------------

device_mc10cart_interface::~device_mc10cart_interface()
{
}


//-------------------------------------------------
//  interface_config_complete
//-------------------------------------------------

void device_mc10cart_interface::interface_config_complete()
{
	m_owning_slot = dynamic_cast<mc10cart_slot_device *>(device().owner());
	m_host = m_owning_slot
			? dynamic_cast<device_mc10cart_host_interface *>(m_owning_slot->owner())
			: nullptr;
}


//-------------------------------------------------
//  interface_pre_start
//-------------------------------------------------

void device_mc10cart_interface::interface_pre_start()
{
	if (!m_owning_slot)
		throw emu_fatalerror("Expected device().owner() to be of type mc10cart_slot_device");
	if (!m_host)
		throw emu_fatalerror("Expected m_owning_slot->owner() to be of type device_mc10cart_host_interface");
}



//-------------------------------------------------
//  set_line_value
//-------------------------------------------------

void device_mc10cart_interface::set_line_value(mc10cart_slot_device::line line, mc10cart_slot_device::line_value value)
{
	owning_slot().set_line_value(line, value);
}


//-------------------------------------------------
//  mc10_cart_add_basic_devices
//-------------------------------------------------

void mc10_cart_add_basic_devices(device_slot_interface &device)
{
	// basic devices
	device.option_add("ram", MC10_PAK_RAM);
}

