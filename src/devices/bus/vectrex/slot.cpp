// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

    GCE Vectrex cart emulation
    (through slot devices)

 ***********************************************************************************************************/

#include "emu.h"
#include "slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VECTREX_CART_SLOT, vectrex_cart_slot_device, "vectrex_cart_slot", "GCE Vectrex Cartridge Slot")

//**************************************************************************
//    Vectrex Cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_vectrex_cart_interface - constructor
//-------------------------------------------------

device_vectrex_cart_interface::device_vectrex_cart_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "vectrexcart"),
	m_rom(nullptr),
	m_rom_size(0)
{
}


//-------------------------------------------------
//  ~device_vectrex_cart_interface - destructor
//-------------------------------------------------

device_vectrex_cart_interface::~device_vectrex_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_vectrex_cart_interface::rom_alloc(uint32_t size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(VECSLOT_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vectrex_cart_slot_device - constructor
//-------------------------------------------------
vectrex_cart_slot_device::vectrex_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VECTREX_CART_SLOT, tag, owner, clock),
	device_image_interface(mconfig, *this),
	device_single_card_slot_interface<device_vectrex_cart_interface>(mconfig, *this),
	m_type(VECTREX_STD),
	m_vec3d(VEC3D_NONE),
	m_cart(nullptr)
{
}


//-------------------------------------------------
//  vectrex_cart_slot_device - destructor
//-------------------------------------------------

vectrex_cart_slot_device::~vectrex_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vectrex_cart_slot_device::device_start()
{
	m_cart = get_card_device();
}


//-------------------------------------------------
//  Vectrex PCB
//-------------------------------------------------

struct vectrex_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const vectrex_slot slot_list[] =
{
	{ VECTREX_STD,    "vec_rom" },
	{ VECTREX_64K,    "vec_rom64k" },
	{ VECTREX_SRAM,   "vec_sram" }
};

#if 0
static int vectrex_get_pcb_id(const char *slot)
{
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (!strcmp(slot_list[i].slot_option, slot))
			return slot_list[i].pcb_id;
	}

	return 0;
}
#endif

static const char *vectrex_get_slot(int type)
{
	for (auto &elem : slot_list)
	{
		if (elem.pcb_id == type)
			return elem.slot_option;
	}

	return "vec_rom";
}

/*-------------------------------------------------
 call load
 -------------------------------------------------*/

image_init_result vectrex_cart_slot_device::call_load()
{
	if (m_cart)
	{
		uint32_t size = !loaded_through_softlist() ? length() : get_software_region_length("rom");
		uint8_t *ROM;

		if (size > 0x10000)
		{
			seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
			return image_init_result::FAIL;
		}

		m_cart->rom_alloc((size < 0x1000) ? 0x1000 : size, tag());
		ROM = m_cart->get_rom_base();

		if (!loaded_through_softlist())
			fread(ROM, size);
		else
			memcpy(ROM, get_software_region("rom"), size);

		// Verify the file is accepted by the Vectrex bios
		if (memcmp(ROM, "g GCE", 5))
		{
			seterror(IMAGE_ERROR_UNSPECIFIED, "Invalid image");
			return image_init_result::FAIL;
		}

		// determine type
		m_type = VECTREX_STD;
		if (!memcmp(ROM + 0x06, "SRAM", 4))
			m_type = VECTREX_SRAM;
		if (size > 0x8000)
			m_type = VECTREX_64K;

		//printf("Type: %s\n", vectrex_get_slot(m_type));

		// determine 3D setup (to help video setup at machine_start)
		if (!memcmp(ROM + 0x11, "NARROW", 6) && (ROM[0x39] == 0x0c))
			m_vec3d = VEC3D_NARROW;

		if (!memcmp(ROM + 0x11, "CRAZY COASTER", 13))
			m_vec3d = VEC3D_CCOAST;

		if (!memcmp(ROM + 0x11, "3D MINE STORM", 13))
			m_vec3d = VEC3D_MINEST;

		return image_init_result::PASS;
	}

	return image_init_result::PASS;
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string vectrex_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
	{
		const char *slot_string;
		uint32_t size = hook.image_file()->size();
		std::vector<uint8_t> rom(size);
		int type = VECTREX_STD;

		hook.image_file()->read(&rom[0], size);

		if (!memcmp(&rom[0x06], "SRAM", 4))
			type = VECTREX_SRAM;
		if (size > 0x8000)
			type = VECTREX_64K;

		slot_string = vectrex_get_slot(type);

		//printf("type: %s\n", slot_string);

		return std::string(slot_string);
	}

	return software_get_default_slot("vec_rom");
}

/*-------------------------------------------------
 read_rom
 -------------------------------------------------*/

uint8_t vectrex_cart_slot_device::read_rom(offs_t offset)
{
	if (m_cart)
		return m_cart->read_rom(offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 write_ram
 -------------------------------------------------*/

void vectrex_cart_slot_device::write_ram(offs_t offset, uint8_t data)
{
	if (m_cart)
		m_cart->write_ram(offset, data);
}

/*-------------------------------------------------
 write_bank
 -------------------------------------------------*/

void vectrex_cart_slot_device::write_bank(uint8_t data)
{
	if (m_cart)
		m_cart->write_bank(data);
}
