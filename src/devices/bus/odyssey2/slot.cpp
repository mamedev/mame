// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

    Magnavox Odyssey 2 cart emulation
    (through slot devices)

***********************************************************************************************************/

#include "emu.h"
#include "slot.h"

#include <tuple>


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(O2_CART_SLOT, o2_cart_slot_device, "o2_cart_slot", "Odyssey 2 Cartridge Slot")

//**************************************************************************
//    Odyssey 2 Cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_o2_cart_interface - constructor
//-------------------------------------------------

device_o2_cart_interface::device_o2_cart_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "odyssey2cart"),
	m_rom_size(0),
	m_exrom_size(0),
	m_voice_size(0)
{ }

device_o2_cart_interface::~device_o2_cart_interface() { }


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  o2_cart_slot_device - constructor
//-------------------------------------------------

o2_cart_slot_device::o2_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, O2_CART_SLOT, tag, owner, clock),
	device_cartrom_image_interface(mconfig, *this),
	device_single_card_slot_interface<device_o2_cart_interface>(mconfig, *this),
	m_type(O2_STD),
	m_cart(nullptr),
	m_conf(*this, "CONF"),
	m_b(0)
{ }

o2_cart_slot_device::~o2_cart_slot_device() { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void o2_cart_slot_device::device_start()
{
	m_cart = get_card_device();
}


//-------------------------------------------------
//  O2 PCB
//-------------------------------------------------

struct o2_slot
{
	int            pcb_id;
	const char     *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const o2_slot slot_list[] =
{
	{ O2_STD,      "o2_rom" },
	{ O2_4IN1,     "o2_4in1" },
	{ O2_RALLY,    "o2_rally" },
	{ O2_KTAA,     "o2_ktaa" },
	{ O2_CHESS,    "o2_chess" },
	{ O2_HOMECOMP, "o2_homecomp" },
	{ O2_TEST,     "o2_test" },
	{ O2_TESTPL,   "o2_testpl" },
	{ O2_VOICE,    "o2_voice" }
};

static int o2_get_pcb_id(const char *slot)
{
	if (slot)
	{
		for (auto & elem : slot_list)
		{
			if (!strcmp(elem.slot_option, slot))
				return elem.pcb_id;
		}
	}

	return 0;
}

static const char *o2_get_slot(int type)
{
	for (auto & elem : slot_list)
	{
		if (elem.pcb_id == type)
			return elem.slot_option;
	}

	return "o2_rom";
}


/*-------------------------------------------------
 call load
-------------------------------------------------*/

std::pair<std::error_condition, std::string> o2_cart_slot_device::call_load()
{
	std::error_condition err;

	if (m_cart)
	{
		if (loaded_through_softlist())
		{
			load_software_region("rom", m_cart->m_rom);
			load_software_region("exrom", m_cart->m_exrom);
			load_software_region("voice", m_cart->m_voice);
			m_cart->m_rom_size = get_software_region_length("rom");
			m_cart->m_exrom_size = get_software_region_length("exrom");
			m_cart->m_voice_size = get_software_region_length("voice");

			m_type = o2_get_pcb_id(get_feature("slot"));

			// Videopac+ determines whether the screen should have a border, with a gate connected
			// to the cartridge B pin. This way, old Videopac games can still run in full screen.
			const char *b_pin = get_feature("b_pin");
			m_b = b_pin && strtoul(b_pin, nullptr, 0) ? 1 : 0;
		}
		else
		{
			u32 const size = length();
			size_t actual;
			std::tie(err, m_cart->m_rom, actual) = read(image_core_file(), size);
			if (!err && (actual != size))
				err = std::errc::io_error;

			if (!err)
			{
				m_cart->m_rom_size = size;
				m_cart->m_exrom_size = 0;
				m_cart->m_voice_size = 0;
				m_b = 0;

				m_type = (size == 0x4000) ? O2_RALLY : O2_STD;
			}
		}

		if (m_cart->get_rom_size() > 0)
			m_cart->cart_init();
		else if (!err)
			err = image_error::UNSPECIFIED;
	}
	else
	{
		err = image_error::UNSPECIFIED;
	}

	return std::make_pair(err, std::string());
}


/*-------------------------------------------------
 get default card software
-------------------------------------------------*/

std::string o2_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
	{
		uint64_t size;
		hook.image_file()->length(size); // FIXME: check error return
		int const type = (size == 0x4000) ? O2_RALLY : O2_STD;
		char const *const slot_string = o2_get_slot(type);

		return std::string(slot_string);
	}

	return software_get_default_slot("o2_rom");
}


/*-------------------------------------------------
 read_rom**
-------------------------------------------------*/

u8 o2_cart_slot_device::read_rom04(offs_t offset)
{
	if (m_cart)
		return m_cart->read_rom04(offset);
	else
		return 0xff;
}

u8 o2_cart_slot_device::read_rom0c(offs_t offset)
{
	if (m_cart)
		return m_cart->read_rom0c(offset);
	else
		return 0xff;
}


/*-------------------------------------------------
 io
-------------------------------------------------*/

void o2_cart_slot_device::io_write(offs_t offset, u8 data)
{
	if (m_cart)
		m_cart->io_write(offset, data);
}

u8 o2_cart_slot_device::io_read(offs_t offset)
{
	return (m_cart) ? m_cart->io_read(offset) : 0xff;
}

void o2_cart_slot_device::bus_write(u8 data)
{
	if (m_cart)
		m_cart->bus_write(data);
}

u8 o2_cart_slot_device::bus_read()
{
	return (m_cart) ? m_cart->bus_read() : 0xff;
}

int o2_cart_slot_device::t0_read()
{
	return (m_cart) ? m_cart->t0_read() : 0;
}

int o2_cart_slot_device::b_read()
{
	int b_user = m_conf->read() & 3;
	int b = b_user & 1;

	if (m_cart)
	{
		// cartridge may override pin B setting
		int b_cart = m_cart->b_read();

		if (b_cart >= 0)
			b = b_cart;
		else if (b_user & 2)
			b = m_b;
	}

	return b;
}


/*-------------------------------------------------
 input ports
-------------------------------------------------*/

// Pin B only affects Videopac+ consoles: cartridges can decide whether to mask display edges or not,
// by wiring the pin to either VCC or GND.

static INPUT_PORTS_START( o2_cart_slot )
	PORT_START("CONF")
	PORT_CONFNAME( 0x03, 0x02, "Cartridge Pin B" )
	PORT_CONFSETTING( 0x02, "Auto" ) // get setting from softwarelist
	PORT_CONFSETTING( 0x00, "0" )
	PORT_CONFSETTING( 0x01, "1" )
INPUT_PORTS_END

ioport_constructor o2_cart_slot_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(o2_cart_slot);
}


#include "bus/odyssey2/rom.h"
#include "bus/odyssey2/4in1.h"
#include "bus/odyssey2/rally.h"
#include "bus/odyssey2/ktaa.h"
#include "bus/odyssey2/chess.h"
#include "bus/odyssey2/homecomp.h"
#include "bus/odyssey2/test.h"
#include "bus/odyssey2/voice.h"

void o2_cart(device_slot_interface &device)
{
	device.option_add_internal("o2_rom",      O2_ROM_STD);
	device.option_add_internal("o2_4in1",     O2_ROM_4IN1);
	device.option_add_internal("o2_rally",    O2_ROM_RALLY);
	device.option_add_internal("o2_ktaa",     O2_ROM_KTAA);
	device.option_add_internal("o2_chess",    O2_ROM_CHESS);
	device.option_add_internal("o2_homecomp", O2_ROM_HOMECOMP);
	device.option_add_internal("o2_test",     O2_ROM_TEST);
	device.option_add_internal("o2_testpl",   O2_ROM_TESTPL);
	device.option_add_internal("o2_voice",    O2_ROM_VOICE);
}
