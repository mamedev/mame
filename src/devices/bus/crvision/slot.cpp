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

const device_type CRVISION_CART_SLOT = &device_creator<crvision_cart_slot_device>;

//**************************************************************************
//    CreatiVision Cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_crvision_cart_interface - constructor
//-------------------------------------------------

device_crvision_cart_interface::device_crvision_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
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

void device_crvision_cart_interface::rom_alloc(UINT32 size, const char *tag)
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
crvision_cart_slot_device::crvision_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, CRVISION_CART_SLOT, "CreatiVision Cartridge Slot", tag, owner, clock, "crvision_cart_slot", __FILE__),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this),
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
	m_cart = dynamic_cast<device_crvision_cart_interface *>(get_card_device());
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void crvision_cart_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
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
		if (!core_stricmp(elem.slot_option, slot))
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

bool crvision_cart_slot_device::call_load()
{
	if (m_cart)
	{
		UINT32 size = (software_entry() == nullptr) ? length() : get_software_region_length("rom");

		if (size > 0x4800)
		{
			seterror(IMAGE_ERROR_UNSPECIFIED, "Image extends beyond the expected size for an APF cart");
			return IMAGE_INIT_FAIL;
		}

		m_cart->rom_alloc(size, tag());

		if (software_entry() == nullptr)
			fread(m_cart->get_rom_base(), size);
		else
			memcpy(m_cart->get_rom_base(), get_software_region("rom"), size);

		if (software_entry() == nullptr)
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

		return IMAGE_INIT_PASS;
	}

	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
 call softlist load
 -------------------------------------------------*/

bool crvision_cart_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry);
	return TRUE;
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string crvision_cart_slot_device::get_default_card_software()
{
	if (open_image_file(mconfig().options()))
	{
		const char *slot_string;
		UINT32 size = core_fsize(m_file);
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
		clear();

		return std::string(slot_string);
	}

	return software_get_default_slot("crv_rom4k");
}

/*-------------------------------------------------
 read_rom
 -------------------------------------------------*/

READ8_MEMBER(crvision_cart_slot_device::read_rom40)
{
	if (m_cart)
		return m_cart->read_rom40(space, offset);
	else
		return 0xff;
}

READ8_MEMBER(crvision_cart_slot_device::read_rom80)
{
	if (m_cart)
		return m_cart->read_rom80(space, offset);
	else
		return 0xff;
}
