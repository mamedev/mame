// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

    Bally Astrocade cart emulation
    (through slot devices)

 ***********************************************************************************************************/


#include "emu.h"
#include "slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ASTROCADE_CART_SLOT, astrocade_cart_slot_device, "astrocade_cart_slot", "Bally Astrocade Cartridge Slot")

//**************************************************************************
//    Astrocade Cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_astrocade_cart_interface - constructor
//-------------------------------------------------

device_astrocade_cart_interface::device_astrocade_cart_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "astrocadecart"),
	m_rom(nullptr),
	m_rom_size(0)
{
}


//-------------------------------------------------
//  ~device_astrocade_cart_interface - destructor
//-------------------------------------------------

device_astrocade_cart_interface::~device_astrocade_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_astrocade_cart_interface::rom_alloc(uint32_t size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(ASTROCADESLOT_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  astrocade_cart_slot_device - constructor
//-------------------------------------------------
astrocade_cart_slot_device::astrocade_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ASTROCADE_CART_SLOT, tag, owner, clock),
	device_image_interface(mconfig, *this),
	device_single_card_slot_interface<device_astrocade_cart_interface>(mconfig, *this),
	m_type(ASTROCADE_STD), m_cart(nullptr)
{
}


//-------------------------------------------------
//  astrocade_cart_slot_device - destructor
//-------------------------------------------------

astrocade_cart_slot_device::~astrocade_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void astrocade_cart_slot_device::device_start()
{
	m_cart = get_card_device();
}


//-------------------------------------------------
//  ASTROCADE PCB
//-------------------------------------------------

struct astrocade_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const astrocade_slot slot_list[] =
{
	{ ASTROCADE_STD,  "rom" },
	{ ASTROCADE_256K, "rom_256k" },
	{ ASTROCADE_512K, "rom_512k" },
	{ ASTROCADE_CASS, "rom_cass" }
};

static int astrocade_get_pcb_id(const char *slot)
{
	for (auto & elem : slot_list)
	{
		if (!core_stricmp(elem.slot_option, slot))
			return elem.pcb_id;
	}

	return 0;
}

static const char *astrocade_get_slot(int type)
{
	for (auto & elem : slot_list)
	{
		if (elem.pcb_id == type)
			return elem.slot_option;
	}

	return "std";
}


/*-------------------------------------------------
 call load
 -------------------------------------------------*/

image_init_result astrocade_cart_slot_device::call_load()
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
			m_type = ASTROCADE_STD;

			if (size == 0x40000)
				m_type = ASTROCADE_256K;
			if (size == 0x80000)
				m_type = ASTROCADE_512K;
		}
		else
		{
			const char *pcb_name = get_feature("slot");
			if (pcb_name)
				m_type = astrocade_get_pcb_id(pcb_name);
		}

		//printf("Type: %s\n", astrocade_get_slot(m_type));

		return image_init_result::PASS;
	}

	return image_init_result::PASS;
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string astrocade_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
	{
		const char *slot_string;
		uint32_t size = hook.image_file()->size();
		int type = ASTROCADE_STD;

		if (size == 0x40000)
			type = ASTROCADE_256K;
		if (size == 0x80000)
			type = ASTROCADE_512K;

		slot_string = astrocade_get_slot(type);

		//printf("type: %s\n", slot_string);

		return std::string(slot_string);
	}

	return software_get_default_slot("rom");
}

/*-------------------------------------------------
 read
 -------------------------------------------------*/

uint8_t astrocade_cart_slot_device::read_rom(offs_t offset)
{
	if (m_cart)
		return m_cart->read_rom(offset);
	else
		return 0xff;
}
