// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

    Nintendo Virtual Boy cart emulation
    (through slot devices)

 ***********************************************************************************************************/


#include "emu.h"
#include "slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VBOY_CART_SLOT, vboy_cart_slot_device, "vboy_cart_slot", "Nintendo Virtual Boy Cartridge Slot")

//**************************************************************************
//    vboy cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_vboy_cart_interface - constructor
//-------------------------------------------------

device_vboy_cart_interface::device_vboy_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_rom(nullptr),
		m_rom_size(0),
		m_rom_mask(0)
{
}


//-------------------------------------------------
//  ~device_vboy_cart_interface - destructor
//-------------------------------------------------

device_vboy_cart_interface::~device_vboy_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_vboy_cart_interface::rom_alloc(uint32_t size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = (uint32_t *)device().machine().memory().region_alloc(std::string(tag).append(VBOYSLOT_ROM_REGION_TAG).c_str(), size, 4, ENDIANNESS_LITTLE)->base();
		m_rom_size = size/4;
		m_rom_mask = m_rom_size - 1;
	}
}


//-------------------------------------------------
//  ram_alloc - alloc the space for the ram
//-------------------------------------------------

void device_vboy_cart_interface::eeprom_alloc(uint32_t size)
{
	m_eeprom.resize(size/sizeof(uint32_t));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vboy_cart_slot_device - constructor
//-------------------------------------------------
vboy_cart_slot_device::vboy_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VBOY_CART_SLOT, tag, owner, clock),
	device_image_interface(mconfig, *this),
	device_slot_interface(mconfig, *this),
	m_type(VBOY_STD),
	m_cart(nullptr)
{
}


//-------------------------------------------------
//  vboy_cart_slot_device - destructor
//-------------------------------------------------

vboy_cart_slot_device::~vboy_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vboy_cart_slot_device::device_start()
{
	m_cart = dynamic_cast<device_vboy_cart_interface *>(get_card_device());
}


//-------------------------------------------------
//  vboy PCB
//-------------------------------------------------

struct vboy_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const vboy_slot slot_list[] =
{
	{ VBOY_STD,       "vb_rom" },
	{ VBOY_EEPROM,    "vb_eeprom" }
};

static int vboy_get_pcb_id(const char *slot)
{
	for (auto & elem : slot_list)
	{
		if (!core_stricmp(elem.slot_option, slot))
			return elem.pcb_id;
	}

	return 0;
}

#if 0
static const char *vboy_get_slot(int type)
{
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (slot_list[i].pcb_id == type)
			return slot_list[i].slot_option;
	}

	return "vb_rom";
}
#endif

/*-------------------------------------------------
 call load
 -------------------------------------------------*/

image_init_result vboy_cart_slot_device::call_load()
{
	if (m_cart)
	{
		uint8_t *ROM;
		uint32_t len = !loaded_through_softlist() ? length() : get_software_region_length("rom");
		bool has_eeprom = loaded_through_softlist() && get_software_region("eeprom");

		if (len > 0x200000)
		{
			seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
			return image_init_result::FAIL;
		}

		// always alloc 0x200000 so to be able to directly map the region
		// to the address map (speeding up emulation a bit)
		m_cart->rom_alloc(0x200000, tag());
		if (has_eeprom)
			m_cart->eeprom_alloc(get_software_region_length("eeprom"));

		ROM = (uint8_t *)m_cart->get_rom_base();

		if (!loaded_through_softlist())
			fread(ROM, len);
		else
			memcpy(ROM, get_software_region("rom"), len);

		if (len < 0x080000) { memcpy(ROM + 0x040000, ROM, 0x040000); }
		if (len < 0x100000) { memcpy(ROM + 0x080000, ROM, 0x080000); }
		if (len < 0x200000) { memcpy(ROM + 0x100000, ROM, 0x100000); }

		if (!loaded_through_softlist())
			m_type = vboy_get_pcb_id("vb_rom");
		else
		{
			const char *pcb_name = get_feature("slot");
			if (pcb_name)
				m_type = vboy_get_pcb_id(pcb_name);
		}

		//printf("Type: %s\n", vboy_get_slot(m_type));

		return image_init_result::PASS;
	}

	return image_init_result::PASS;
}


/*-------------------------------------------------
 call_unload
 -------------------------------------------------*/

void vboy_cart_slot_device::call_unload()
{
	if (m_cart && m_cart->get_eeprom_base() && m_cart->get_eeprom_size())
		battery_save(m_cart->get_eeprom_base(), m_cart->get_eeprom_size() * 4);
}

/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string vboy_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("vb_rom");
}

/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ32_MEMBER(vboy_cart_slot_device::read_cart)
{
	if (m_cart)
		return m_cart->read_cart(space, offset, mem_mask);
	else
		return 0xffffffff;
}

/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ32_MEMBER(vboy_cart_slot_device::read_eeprom)
{
	if (m_cart)
		return m_cart->read_eeprom(space, offset, mem_mask);
	else
		return 0xffffffff;
}

/*-------------------------------------------------
 write
 -------------------------------------------------*/

WRITE32_MEMBER(vboy_cart_slot_device::write_eeprom)
{
	if (m_cart)
		m_cart->write_eeprom(space, offset, data, mem_mask);
}
