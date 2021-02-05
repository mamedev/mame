// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

    V-Tech CreatiVision cart emulation
    (through slot devices)

 ***********************************************************************************************************/

#include "emu.h"
#include "slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(CRVISION_CART_SLOT, crvision_cart_slot_device, "crvision_cart_slot", "CreatiVision Cartridge Slot")

//**************************************************************************
//    CreatiVision Cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_crvision_cart_interface - constructor
//-------------------------------------------------

device_crvision_cart_interface::device_crvision_cart_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "crvisioncart"),
	m_rom(nullptr),
	m_rom_size(0)
{
}


//-------------------------------------------------
//  ~device_crvision_cart_interface - destructor
//-------------------------------------------------

device_crvision_cart_interface::~device_crvision_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_crvision_cart_interface::rom_alloc(uint32_t size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(CRVSLOT_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  crvision_cart_slot_device - constructor
//-------------------------------------------------
crvision_cart_slot_device::crvision_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CRVISION_CART_SLOT, tag, owner, clock),
	device_image_interface(mconfig, *this),
	device_single_card_slot_interface<device_crvision_cart_interface>(mconfig, *this),
	m_type(CRV_4K), m_cart(nullptr)
{
}


//-------------------------------------------------
//  crvision_cart_slot_device - destructor
//-------------------------------------------------

crvision_cart_slot_device::~crvision_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void crvision_cart_slot_device::device_start()
{
	m_cart = get_card_device();
}


//-------------------------------------------------
//  APF PCB
//-------------------------------------------------

struct crvision_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const crvision_slot slot_list[] =
{
	{ CRV_4K,  "crv_rom4k" },
	{ CRV_6K,  "crv_rom6k" },
	{ CRV_8K,  "crv_rom8k" },
	{ CRV_10K, "crv_rom10k" },
	{ CRV_12K, "crv_rom12k" },
	{ CRV_16K, "crv_rom16k" },
	{ CRV_18K, "crv_rom18k" }
};

static int crvision_get_pcb_id(const char *slot)
{
	for (auto & elem : slot_list)
	{
		if (!strcmp(elem.slot_option, slot))
			return elem.pcb_id;
	}

	return 0;
}

static const char *crvision_get_slot(int type)
{
	for (auto & elem : slot_list)
	{
		if (elem.pcb_id == type)
			return elem.slot_option;
	}

	return "crv_rom4k";
}


/*-------------------------------------------------
 call load
 -------------------------------------------------*/

image_init_result crvision_cart_slot_device::call_load()
{
	if (m_cart)
	{
		uint32_t size = !loaded_through_softlist() ? length() : get_software_region_length("rom");

		if (size > 0x4800)
		{
			seterror(IMAGE_ERROR_UNSPECIFIED, "Image extends beyond the expected size for an APF cart");
			return image_init_result::FAIL;
		}

		m_cart->rom_alloc(size, tag());

		if (!loaded_through_softlist())
			fread(m_cart->get_rom_base(), size);
		else
			memcpy(m_cart->get_rom_base(), get_software_region("rom"), size);

		if (!loaded_through_softlist())
		{
			m_type = CRV_4K;

			switch (size)
			{
				case 0x4800:
					m_type = CRV_18K;
					break;
				case 0x4000:
					m_type = CRV_16K;
					break;
				case 0x3000:
					m_type = CRV_12K;
					break;
				case 0x2800:
					m_type = CRV_10K;
					break;
				case 0x2000:
					m_type = CRV_8K;
					break;
				case 0x1800:
					m_type = CRV_6K;
					break;
				case 0x1000:
				default:
					break;
			}
		}
		else
		{
			const char *pcb_name = get_feature("slot");
			if (pcb_name)
				m_type = crvision_get_pcb_id(pcb_name);
		}

		printf("Type: %s\n", crvision_get_slot(m_type));

		return image_init_result::PASS;
	}

	return image_init_result::PASS;
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string crvision_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
	{
		const char *slot_string;
		uint32_t size = hook.image_file()->size();
		int type = CRV_4K;

		switch (size)
		{
			case 0x4800:
				type = CRV_18K;
				break;
			case 0x4000:
				type = CRV_16K;
				break;
			case 0x3000:
				type = CRV_12K;
				break;
			case 0x2800:
				type = CRV_10K;
				break;
			case 0x2000:
				type = CRV_8K;
				break;
			case 0x1800:
				type = CRV_6K;
				break;
			case 0x1000:
			default:
				break;
		}

		slot_string = crvision_get_slot(type);

		//printf("type: %s\n", slot_string);

		return std::string(slot_string);
	}

	return software_get_default_slot("crv_rom4k");
}

/*-------------------------------------------------
 read_rom
 -------------------------------------------------*/

uint8_t crvision_cart_slot_device::read_rom40(offs_t offset)
{
	if (m_cart)
		return m_cart->read_rom40(offset);
	else
		return 0xff;
}

uint8_t crvision_cart_slot_device::read_rom80(offs_t offset)
{
	if (m_cart)
		return m_cart->read_rom80(offset);
	else
		return 0xff;
}
