// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

    Emerson Arcadia 2001 (and clones) cart emulation
    (through slot devices)

 ***********************************************************************************************************/


#include "emu.h"
#include "slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type EA2001_CART_SLOT = &device_creator<arcadia_cart_slot_device>;

//**************************************************************************
//    ARCADIA Cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_arcadia_cart_interface - constructor
//-------------------------------------------------

device_arcadia_cart_interface::device_arcadia_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_rom(NULL),
		m_rom_size(0)
{
}


//-------------------------------------------------
//  ~device_arcadia_cart_interface - destructor
//-------------------------------------------------

device_arcadia_cart_interface::~device_arcadia_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_arcadia_cart_interface::rom_alloc(UINT32 size, const char *tag)
{
	if (m_rom == NULL)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(EA2001SLOT_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arcadia_cart_slot_device - constructor
//-------------------------------------------------
arcadia_cart_slot_device::arcadia_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, EA2001_CART_SLOT, "Emerson Arcadia Cartridge Slot", tag, owner, clock, "arcadia_cart_slot", __FILE__),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this),
						m_type(ARCADIA_STD), m_cart(nullptr)
{
}


//-------------------------------------------------
//  arcadia_cart_slot_device - destructor
//-------------------------------------------------

arcadia_cart_slot_device::~arcadia_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arcadia_cart_slot_device::device_start()
{
	m_cart = dynamic_cast<device_arcadia_cart_interface *>(get_card_device());
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void arcadia_cart_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
}


//-------------------------------------------------
//  ARCADIA PCB
//-------------------------------------------------

struct arcadia_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const arcadia_slot slot_list[] =
{
	{ ARCADIA_STD,       "std" },
	{ ARCADIA_GOLF,      "golf" }
};

static int arcadia_get_pcb_id(const char *slot)
{
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (!core_stricmp(slot_list[i].slot_option, slot))
			return slot_list[i].pcb_id;
	}

	return 0;
}

#if 0
static const char *arcadia_get_slot(int type)
{
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (slot_list[i].pcb_id == type)
			return slot_list[i].slot_option;
	}

	return "std";
}
#endif

/*-------------------------------------------------
 call load
 -------------------------------------------------*/

bool arcadia_cart_slot_device::call_load()
{
	if (m_cart)
	{
		UINT32 len = (software_entry() == NULL) ? length() : get_software_region_length("rom");

		m_cart->rom_alloc(len, tag());

		if (software_entry() == NULL)
			fread(m_cart->get_rom_base(), len);
		else
			memcpy(m_cart->get_rom_base(), get_software_region("rom"), len);

		if (software_entry() == NULL)
		{
			// we need to identify Golf!
			m_type = ARCADIA_STD;


// the patch below is kept in case it could be used to identify golf cart from fullpath
#if 0
			// this is a testpatch for the golf cartridge
			// so to make it work on a standard arcadia 2001
			// cart (i.e. mapping the hi 2K to $2000)
			// not enough yet (some pointers stored as data?)
			patch[]= {
				// addr,   orig. value, patched value
				{ 0x0077,  0x40,        0x20 },
				{ 0x011e,  0x40,        0x20 },
				{ 0x0348,  0x40,        0x20 },
				{ 0x03be,  0x40,        0x20 },
				{ 0x04ce,  0x40,        0x20 },
				{ 0x04da,  0x40,        0x20 },
				{ 0x0562,  0x42,        0x22 },
				{ 0x0617,  0x40,        0x20 },
				{ 0x0822,  0x40,        0x20 },
				{ 0x095e,  0x42,        0x22 },
				{ 0x09d3,  0x42,        0x22 },
				{ 0x0bb0,  0x42,        0x22 },
				{ 0x0efb,  0x40,        0x20 },
				{ 0x0ec1,  0x43,        0x23 },
				{ 0x0f00,  0x40,        0x20 },
				{ 0x0f12,  0x40,        0x20 },
				{ 0x0ff5,  0x43,        0x23 },
				{ 0x0ff7,  0x41,        0x21 },
				{ 0x0ff9,  0x40,        0x20 },
				{ 0x0ffb,  0x41,        0x21 },
				{ 0x20ec,  0x42,        0x22 }
			};
#endif

		}
		else
		{
			const char *pcb_name = get_feature("slot");
			if (pcb_name)
				m_type = arcadia_get_pcb_id(pcb_name);
		}

		//printf("Type: %s\n", arcadia_get_slot(m_type));

		return IMAGE_INIT_PASS;
	}

	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
 call softlist load
 -------------------------------------------------*/

bool arcadia_cart_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry);
	return TRUE;
}



/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

void arcadia_cart_slot_device::get_default_card_software(std::string &result)
{
	software_get_default_slot(result, "std");
}

/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ8_MEMBER(arcadia_cart_slot_device::read_rom)
{
	if (m_cart)
		return m_cart->read_rom(space, offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 write
 -------------------------------------------------*/

READ8_MEMBER(arcadia_cart_slot_device::extra_rom)
{
	if (m_cart)
		return m_cart->extra_rom(space, offset);
	else
		return 0xff;
}
