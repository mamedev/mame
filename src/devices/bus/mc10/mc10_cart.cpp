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
        17  A5                 34  GND

	SEL is an input to the MC-10 that allows the cartridge to remove
	the internal chips from the bus.

*********************************************************************/

#include "emu.h"
#include "mc10_cart.h"

#include "mcx128.h"
#include "pak.h"
#include "ram.h"

//#define VERBOSE 1
#include "logmacro.h"

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
	m_nmi_callback.resolve_safe();
	m_cart = get_card_device();
}

//-------------------------------------------------
//  set_nmi_line
//-------------------------------------------------

void mc10cart_slot_device::set_nmi_line(int state)
{
	m_nmi_callback(state);
}

//-------------------------------------------------
//  call_load
//-------------------------------------------------

image_init_result mc10cart_slot_device::call_load()
{
	if (!m_cart)
		return image_init_result::PASS;

	memory_region *romregion(loaded_through_softlist() ? memregion("rom") : nullptr);
	if (loaded_through_softlist() && !romregion)
	{
		seterror(IMAGE_ERROR_INVALIDIMAGE, "Software list item has no 'rom' data area");
		return image_init_result::FAIL;
	}

	u32 const len(loaded_through_softlist() ? romregion->bytes() : length());
	if (len > m_cart->max_rom_length())
	{
		seterror(IMAGE_ERROR_UNSUPPORTED, "Unsupported cartridge size");
		return image_init_result::FAIL;
	}

	if (!loaded_through_softlist())
	{
		LOG("Allocating %u byte cartridge ROM region\n", len);
		romregion = machine().memory().region_alloc(subtag("rom").c_str(), len, 1, ENDIANNESS_BIG);
		u32 const cnt(fread(romregion->base(), len));
		if (cnt != len)
		{
			seterror(IMAGE_ERROR_UNSPECIFIED, "Error reading cartridge file");
			return image_init_result::FAIL;
		}
	}

	return m_cart->load();
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
}

//-------------------------------------------------
//  interface_pre_start
//-------------------------------------------------

void device_mc10cart_interface::interface_pre_start()
{
	if (!m_owning_slot)
		throw emu_fatalerror("Expected device().owner() to be of type mc10cart_slot_device");
}

/*-------------------------------------------------
    max_rom_length
-------------------------------------------------*/

int device_mc10cart_interface::max_rom_length() const
{
	return 0;
}

/*-------------------------------------------------
    load
-------------------------------------------------*/

image_init_result device_mc10cart_interface::load()
{
	return image_init_result::FAIL;
}

//-------------------------------------------------
//  mc10_cart_add_basic_devices
//-------------------------------------------------

void mc10_cart_add_basic_devices(device_slot_interface &device)
{
	// basic devices
	device.option_add("mcx128", MC10_PAK_MCX128);
	device.option_add("pak", MC10_PAK);
	device.option_add("ram", MC10_PAK_RAM);
}

//-------------------------------------------------
//  alice_cart_add_basic_devices
//-------------------------------------------------

void alice_cart_add_basic_devices(device_slot_interface &device)
{
	// basic devices
	device.option_add("alice128", ALICE_PAK_MCX128);
	device.option_add("pak", MC10_PAK);
	device.option_add("ram", MC10_PAK_RAM);
}

//-------------------------------------------------
//  alice32_cart_add_basic_devices
//-------------------------------------------------

void alice32_cart_add_basic_devices(device_slot_interface &device)
{
	// basic devices
	device.option_add("pak", MC10_PAK);
	device.option_add("ram", MC10_PAK_RAM);
}
