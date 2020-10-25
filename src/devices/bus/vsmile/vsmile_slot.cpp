// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***********************************************************************************************************

    V.Smile cart emulation
    (through slot devices)

 ***********************************************************************************************************/


#include "emu.h"
#include "vsmile_slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VSMILE_CART_SLOT, vsmile_cart_slot_device, "vsmile_cart_slot", "V.Smile Cartridge Slot")

//**************************************************************************
//    V.Smile cartridge interface
//**************************************************************************

//-------------------------------------------------
//  device_vsmile_cart_interface - constructor
//-------------------------------------------------

device_vsmile_cart_interface::device_vsmile_cart_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "vsmilecart")
	, m_rom(nullptr)
	, m_rom_size(0)
{
}


//-------------------------------------------------
//  ~device_vsmile_cart_interface - destructor
//-------------------------------------------------

device_vsmile_cart_interface::~device_vsmile_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_vsmile_cart_interface::rom_alloc(uint32_t size, const char *tag)
{
	if (m_rom == nullptr)
	{
		// We always alloc 8MB of ROM region
		m_rom = (uint16_t *)device().machine().memory().region_alloc(std::string(tag).append(VSMILE_SLOT_ROM_REGION_TAG).c_str(), size, 2, ENDIANNESS_BIG)->base();
		m_rom_size = size;
	}
}


//-------------------------------------------------
//  nvram_alloc - alloc the space for the nvram
//-------------------------------------------------

void device_vsmile_cart_interface::nvram_alloc(uint32_t size)
{
	m_nvram.resize(size / sizeof(uint16_t));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vsmile_cart_slot_device - constructor
//-------------------------------------------------
vsmile_cart_slot_device::vsmile_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VSMILE_CART_SLOT, tag, owner, clock),
	device_image_interface(mconfig, *this),
	device_single_card_slot_interface<device_vsmile_cart_interface>(mconfig, *this),
	m_type(VSMILE_STD),
	m_cart(nullptr)
{
}


//-------------------------------------------------
//  vsmile_cart_slot_device - destructor
//-------------------------------------------------

vsmile_cart_slot_device::~vsmile_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vsmile_cart_slot_device::device_start()
{
	m_cart = get_card_device();
}


//-------------------------------------------------
//  V.Smile PCB
//-------------------------------------------------

struct vsmile_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const vsmile_slot slot_list[] =
{
	{ VSMILE_STD, "vsmile_rom" },
	{ VSMILE_NVRAM, "vsmile_nvram" },
};

static int vsmile_get_pcb_id(const char *slot)
{
	for (auto & elem : slot_list)
	{
		if (!core_stricmp(elem.slot_option, slot))
			return elem.pcb_id;
	}

	return 0;
}

/*-------------------------------------------------
 call load
 -------------------------------------------------*/

image_init_result vsmile_cart_slot_device::call_load()
{
	if (m_cart)
	{
		uint32_t size = loaded_through_softlist() ? get_software_region_length("rom") : length();
		if (size > 0x1000000)
		{
			seterror(IMAGE_ERROR_UNSPECIFIED, "Attempted loading a cart larger than 16MB");
			return image_init_result::FAIL;
		}

		m_cart->rom_alloc(size, tag());
		uint8_t *rom = (uint8_t *)m_cart->get_rom_base();

		if (!loaded_through_softlist())
		{
			fread(rom, size);
			m_type = VSMILE_STD;
		}
		else
		{
			const char *pcb_name = get_feature("slot");

			memcpy(rom, get_software_region("rom"), size);

			if (pcb_name)
				m_type = vsmile_get_pcb_id(pcb_name);

			osd_printf_info("V.Smile: Detected (XML) %s\n", pcb_name ? pcb_name : "NONE");
		}

		if (m_type == VSMILE_NVRAM)
		{
			m_cart->nvram_alloc(0x200000);
		}

		if (m_cart->get_nvram_size())
		{
			battery_load(m_cart->get_nvram_base(), m_cart->get_nvram_size(), 0x00);
		}

		return image_init_result::PASS;
	}

	return image_init_result::PASS;
}


/*-------------------------------------------------
 call_unload
 -------------------------------------------------*/

void vsmile_cart_slot_device::call_unload()
{
	if (m_cart && m_cart->get_nvram_size())
	{
		battery_save(m_cart->get_nvram_base(), m_cart->get_nvram_size());
	}
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string vsmile_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("vsmile_rom");
}


/*-------------------------------------------------
 cart accessors
 -------------------------------------------------*/

uint16_t vsmile_cart_slot_device::bank0_r(offs_t offset)
{
	return m_cart->bank0_r(offset);
}

uint16_t vsmile_cart_slot_device::bank1_r(offs_t offset)
{
	return m_cart->bank1_r(offset);
}

uint16_t vsmile_cart_slot_device::bank2_r(offs_t offset)
{
	return m_cart->bank2_r(offset);
}

uint16_t vsmile_cart_slot_device::bank3_r(offs_t offset)
{
	return m_cart->bank3_r(offset);
}

void vsmile_cart_slot_device::bank0_w(offs_t offset, uint16_t data)
{
	m_cart->bank0_w(offset, data);
}

void vsmile_cart_slot_device::bank1_w(offs_t offset, uint16_t data)
{
	m_cart->bank1_w(offset, data);
}

void vsmile_cart_slot_device::bank2_w(offs_t offset, uint16_t data)
{
	m_cart->bank2_w(offset, data);
}

void vsmile_cart_slot_device::bank3_w(offs_t offset, uint16_t data)
{
	m_cart->bank3_w(offset, data);
}

void vsmile_cart_slot_device::set_cs2(bool cs2)
{
	m_cart->set_cs2(cs2);
}
