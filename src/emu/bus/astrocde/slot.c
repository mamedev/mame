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

const device_type ASTROCADE_CART_SLOT = &device_creator<astrocade_cart_slot_device>;

//**************************************************************************
//    Astrocade Cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_astrocade_cart_interface - constructor
//-------------------------------------------------

device_astrocade_cart_interface::device_astrocade_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_rom(NULL),
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

void device_astrocade_cart_interface::rom_alloc(UINT32 size, const char *tag)
{
	if (m_rom == NULL)
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
astrocade_cart_slot_device::astrocade_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, ASTROCADE_CART_SLOT, "Bally Astrocade Cartridge Slot", tag, owner, clock, "astrocade_cart_slot", __FILE__),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this),
						m_type(ASTROCADE_STD)
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
	m_cart = dynamic_cast<device_astrocade_cart_interface *>(get_card_device());
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void astrocade_cart_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
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
	{ ASTROCADE_512K, "rom_512k" }
};

static int astrocade_get_pcb_id(const char *slot)
{
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (!core_stricmp(slot_list[i].slot_option, slot))
			return slot_list[i].pcb_id;
	}

	return 0;
}

static const char *astrocade_get_slot(int type)
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

bool astrocade_cart_slot_device::call_load()
{
	if (m_cart)
	{
		UINT32 size = (software_entry() == NULL) ? length() : get_software_region_length("rom");
		m_cart->rom_alloc(size, tag());

		if (software_entry() == NULL)
			fread(m_cart->get_rom_base(), size);
		else
			memcpy(m_cart->get_rom_base(), get_software_region("rom"), size);

		if (software_entry() == NULL)
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

		return IMAGE_INIT_PASS;
	}

	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
 call softlist load
 -------------------------------------------------*/

bool astrocade_cart_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry);
	return TRUE;
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

void astrocade_cart_slot_device::get_default_card_software(std::string &result)
{
	if (open_image_file(mconfig().options()))
	{
		const char *slot_string = "rom";
		UINT32 size = core_fsize(m_file);
		int type = ASTROCADE_STD;

		if (size == 0x40000)
			type = ASTROCADE_256K;
		if (size == 0x80000)
			type = ASTROCADE_512K;

		slot_string = astrocade_get_slot(type);

		//printf("type: %s\n", slot_string);
		clear();

		result.assign(slot_string);
		return;
	}

	software_get_default_slot(result, "rom");
}

/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ8_MEMBER(astrocade_cart_slot_device::read_rom)
{
	if (m_cart)
		return m_cart->read_rom(space, offset);
	else
		return 0xff;
}
