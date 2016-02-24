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

const device_type M5_CART_SLOT = &device_creator<m5_cart_slot_device>;

//**************************************************************************
//    M5 Cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_m5_cart_interface - constructor
//-------------------------------------------------

device_m5_cart_interface::device_m5_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_rom(NULL),
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

void device_m5_cart_interface::rom_alloc(UINT32 size, const char *tag)
{
	if (m_rom == NULL)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(M5SLOT_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
	}
}


//-------------------------------------------------
//  ram_alloc - alloc the space for the ram
//-------------------------------------------------

void device_m5_cart_interface::ram_alloc(UINT32 size)
{
	m_ram.resize(size);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  m5_cart_slot_device - constructor
//-------------------------------------------------
m5_cart_slot_device::m5_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, M5_CART_SLOT, "M5 Cartridge Slot", tag, owner, clock, "m5_cart_slot", __FILE__),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this),
						m_type(M5_STD), m_cart(nullptr)
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
	m_cart = dynamic_cast<device_m5_cart_interface *>(get_card_device());
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void m5_cart_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
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

bool m5_cart_slot_device::call_load()
{
	if (m_cart)
	{
		m_type=M5_STD;

		if (software_entry() != NULL)
		{
			const char *pcb_name = get_feature("slot");
			//software_info *name=m_software_info_ptr;
			if (pcb_name) //is it ram cart?
				m_type = m5_get_pcb_id(m_full_software_name.c_str());
			else
				m_type=M5_STD; //standard cart(no feature line in xml)
		}

		if (m_type == M5_STD || m_type>2) //carts with roms
		{
			UINT32 size = (software_entry() == NULL) ? length() : get_software_region_length("rom");

			if (size > 0x5000 && m_type == M5_STD)
			{
				seterror(IMAGE_ERROR_UNSPECIFIED, "Image extends beyond the expected size for an M5 cart");
				return IMAGE_INIT_FAIL;
			}

			m_cart->rom_alloc(size, tag());

			if (software_entry() == NULL)
				fread(m_cart->get_rom_base(), size);
			else
				memcpy(m_cart->get_rom_base(), get_software_region("rom"), size);

		}
		if (!M5_STD)
			if (get_software_region("ram"))
				m_cart->ram_alloc(get_software_region_length("ram"));


		//printf("Type: %s\n", m5_get_slot(m_type));
	}

	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
 call softlist load
 -------------------------------------------------*/

bool m5_cart_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	machine().rom_load().load_software_part_region(*this, swlist, swname, start_entry);
	return TRUE;
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string m5_cart_slot_device::get_default_card_software()
{
	std::string result;
	if (open_image_file(mconfig().options()))
	{
		const char *slot_string = "std";
		//UINT32 size = core_fsize(m_file);
		int type = M5_STD;


		slot_string = m5_get_slot(type);

		//printf("type: %s\n", slot_string);
		clear();

		result.assign(slot_string);
		return result;
	}

	return software_get_default_slot("std");
}

/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ8_MEMBER(m5_cart_slot_device::read_rom)
{
	if (m_cart)
		return m_cart->read_rom(space, offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ8_MEMBER(m5_cart_slot_device::read_ram)
{
	if (m_cart)
		return m_cart->read_ram(space, offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 write
 -------------------------------------------------*/

WRITE8_MEMBER(m5_cart_slot_device::write_ram)
{
	if (m_cart)
		m_cart->write_ram(space, offset, data);
}
