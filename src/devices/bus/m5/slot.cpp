// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

    M5 cart emulation
    (through slot devices)

 ***********************************************************************************************************/


#include "emu.h"
#include "slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(M5_CART_SLOT, m5_cart_slot_device, "m5_cart_slot", "M5 Cartridge Slot")

//**************************************************************************
//    M5 Cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_m5_cart_interface - constructor
//-------------------------------------------------

device_m5_cart_interface::device_m5_cart_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "m5cart"),
	m_rom(nullptr),
	m_rom_size(0)
{
}


//-------------------------------------------------
//  ~device_m5_cart_interface - destructor
//-------------------------------------------------

device_m5_cart_interface::~device_m5_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_m5_cart_interface::rom_alloc(uint32_t size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(M5SLOT_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
	}
}


//-------------------------------------------------
//  ram_alloc - alloc the space for the ram
//-------------------------------------------------

void device_m5_cart_interface::ram_alloc(uint32_t size)
{
	m_ram.resize(size);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  m5_cart_slot_device - constructor
//-------------------------------------------------
m5_cart_slot_device::m5_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, M5_CART_SLOT, tag, owner, clock),
	device_image_interface(mconfig, *this),
	device_single_card_slot_interface(mconfig, *this),
	m_type(M5_STD),
	m_cart(nullptr)
{
}


//-------------------------------------------------
//  m5_cart_slot_device - destructor
//-------------------------------------------------

m5_cart_slot_device::~m5_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m5_cart_slot_device::device_start()
{
	m_cart = get_card_device();
}


//-------------------------------------------------
//  M5 PCB
//-------------------------------------------------

struct m5_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const m5_slot slot_list[] =
{
	{EM_5,"em-5"},
	{MEM64KBI,"64kbi"},
	{MEM64KBF,"64kbf"},
	{MEM64KRX,"64krx"}
};

static int m5_get_pcb_id(const char *slot)
{
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (!core_stricmp(slot_list[i].slot_option, slot))
			return slot_list[i].pcb_id;
	}

	return 0;
}

static const char *m5_get_slot(int type)
{
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (slot_list[i].pcb_id == type)
			return slot_list[i].slot_option;
	}

	return "std";
}


/*-------------------------------------------------
 call load
 -------------------------------------------------*/

image_init_result m5_cart_slot_device::call_load()
{
	if (m_cart)
	{
		m_type=M5_STD;

		if (loaded_through_softlist())
		{
			const char *pcb_name = get_feature("slot");
			if (pcb_name) //is it ram cart?
				m_type = m5_get_pcb_id(full_software_name().c_str());
			else
				m_type=M5_STD; //standard cart(no feature line in xml)
		}

		if (m_type == M5_STD || m_type>2) //carts with roms
		{
			uint32_t size = !loaded_through_softlist() ? length() : get_software_region_length("rom");

			if (size > 0x5000 && m_type == M5_STD)
			{
				seterror(IMAGE_ERROR_UNSPECIFIED, "Image extends beyond the expected size for an M5 cart");
				return image_init_result::FAIL;
			}

			m_cart->rom_alloc(size, tag());

			if (!loaded_through_softlist())
				fread(m_cart->get_rom_base(), size);
			else
				memcpy(m_cart->get_rom_base(), get_software_region("rom"), size);

		}
		if (!M5_STD)
			if (get_software_region("ram"))
				m_cart->ram_alloc(get_software_region_length("ram"));


		//printf("Type: %s\n", m5_get_slot(m_type));
	}

	return image_init_result::PASS;
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string m5_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	std::string result;
	if (hook.image_file())
	{
		const char *slot_string = "std";
		//uint32_t size = core_fsize(m_file);
		int type = M5_STD;


		slot_string = m5_get_slot(type);

		//printf("type: %s\n", slot_string);

		result.assign(slot_string);
		return result;
	}

	return software_get_default_slot("std");
}

/*-------------------------------------------------
 read
 -------------------------------------------------*/

uint8_t m5_cart_slot_device::read_rom(offs_t offset)
{
	if (m_cart)
		return m_cart->read_rom(offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 read
 -------------------------------------------------*/

uint8_t m5_cart_slot_device::read_ram(offs_t offset)
{
	if (m_cart)
		return m_cart->read_ram(offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 write
 -------------------------------------------------*/

void m5_cart_slot_device::write_ram(offs_t offset, uint8_t data)
{
	if (m_cart)
		m_cart->write_ram(offset, data);
}
