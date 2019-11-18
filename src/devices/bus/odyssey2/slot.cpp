// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

    Magnavox Odyssey 2 cart emulation
    (through slot devices)

 ***********************************************************************************************************/


#include "emu.h"
#include "slot.h"

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

device_o2_cart_interface::device_o2_cart_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "odyssey2cart")
	, m_rom(nullptr)
	, m_rom_size(0)
{
}


//-------------------------------------------------
//  ~device_o2_cart_interface - destructor
//-------------------------------------------------

device_o2_cart_interface::~device_o2_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_o2_cart_interface::rom_alloc(uint32_t size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(O2SLOT_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
	}
}


//-------------------------------------------------
//  ram_alloc - alloc the space for the ram
//-------------------------------------------------

void device_o2_cart_interface::ram_alloc(uint32_t size)
{
	m_ram.resize(size);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  o2_cart_slot_device - constructor
//-------------------------------------------------
o2_cart_slot_device::o2_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, O2_CART_SLOT, tag, owner, clock)
	, device_image_interface(mconfig, *this)
	, device_single_card_slot_interface<device_o2_cart_interface>(mconfig, *this)
	, m_type(O2_STD)
	, m_cart(nullptr)
{
}


//-------------------------------------------------
//  o2_cart_slot_device - destructor
//-------------------------------------------------

o2_cart_slot_device::~o2_cart_slot_device()
{
}

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
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const o2_slot slot_list[] =
{
	{ O2_STD,      "o2_rom" },
	{ O2_ROM12,    "o2_rom12" },
	{ O2_ROM16,    "o2_rom16" },
	{ O2_CHESS,    "o2_chess" },
	{ O2_VOICE,    "o2_voice" }
};

static int o2_get_pcb_id(const char *slot)
{
	for (auto & elem : slot_list)
	{
		if (!core_stricmp(elem.slot_option, slot))
			return elem.pcb_id;
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

image_init_result o2_cart_slot_device::call_load()
{
	if (m_cart)
	{
		uint32_t size = !loaded_through_softlist() ? length() : get_software_region_length("rom");
		m_cart->rom_alloc(size, tag());

		if (!loaded_through_softlist())
			fread(m_cart->get_rom_base(), size);
		else
			memcpy(m_cart->get_rom_base(), get_software_region("rom"), size);

		if (!loaded_through_softlist())
		{
			m_type = O2_STD;
			if (size == 12288)
				m_type = O2_ROM12;
			if (size == 16384)
				m_type = O2_ROM16;
		}
		else
		{
			const char *pcb_name = get_feature("slot");
			if (pcb_name)
				m_type = o2_get_pcb_id(pcb_name);
		}

		//printf("Type: %s\n", o2_get_slot(m_type));

		return image_init_result::PASS;
	}

	return image_init_result::PASS;
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string o2_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
	{
		const char *slot_string;
		uint32_t size = hook.image_file()->size();
		int type = O2_STD;

		if (size == 12288)
			type = O2_ROM12;
		if (size == 16384)
			type = O2_ROM16;

		slot_string = o2_get_slot(type);

		//printf("type: %s\n", slot_string);

		return std::string(slot_string);
	}

	return software_get_default_slot("o2_rom");
}

/*-------------------------------------------------
 read_rom**
 -------------------------------------------------*/

READ8_MEMBER(o2_cart_slot_device::read_rom04)
{
	if (m_cart)
		return m_cart->read_rom04(space, offset);
	else
		return 0xff;
}

READ8_MEMBER(o2_cart_slot_device::read_rom0c)
{
	if (m_cart)
		return m_cart->read_rom0c(space, offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 io_write
 -------------------------------------------------*/

WRITE8_MEMBER(o2_cart_slot_device::io_write)
{
	if (m_cart)
		m_cart->io_write(space, offset, data);
}


#include "bus/odyssey2/rom.h"
#include "bus/odyssey2/chess.h"
#include "bus/odyssey2/voice.h"

void o2_cart(device_slot_interface &device)
{
	device.option_add_internal("o2_rom",    O2_ROM_STD);
	device.option_add_internal("o2_rom12",  O2_ROM_12K);
	device.option_add_internal("o2_rom16",  O2_ROM_16K);
	device.option_add_internal("o2_chess",  O2_ROM_CHESS);
	device.option_add_internal("o2_voice",  O2_ROM_VOICE);
}
