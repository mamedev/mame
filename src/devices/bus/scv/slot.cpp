// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

    Epoch Super Cassette Vision cart emulation
    (through slot devices)

 ***********************************************************************************************************/


#include "emu.h"
#include "slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type SCV_CART_SLOT = &device_creator<scv_cart_slot_device>;

//**************************************************************************
//    SCV cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_scv_cart_interface - constructor
//-------------------------------------------------

device_scv_cart_interface::device_scv_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_rom(NULL),
		m_rom_size(0)
{
}


//-------------------------------------------------
//  ~device_scv_cart_interface - destructor
//-------------------------------------------------

device_scv_cart_interface::~device_scv_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_scv_cart_interface::rom_alloc(UINT32 size, const char *tag)
{
	if (m_rom == NULL)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(SCVSLOT_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
	}
}


//-------------------------------------------------
//  ram_alloc - alloc the space for the ram
//-------------------------------------------------

void device_scv_cart_interface::ram_alloc(UINT32 size)
{
	m_ram.resize(size);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  scv_cart_slot_device - constructor
//-------------------------------------------------
scv_cart_slot_device::scv_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, SCV_CART_SLOT, "SCV Cartridge Slot", tag, owner, clock, "scv_cart_slot", __FILE__),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this),
						m_type(SCV_8K), m_cart(nullptr)
{
}


//-------------------------------------------------
//  scv_cart_slot_device - destructor
//-------------------------------------------------

scv_cart_slot_device::~scv_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void scv_cart_slot_device::device_start()
{
	m_cart = dynamic_cast<device_scv_cart_interface *>(get_card_device());
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void scv_cart_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
}


//-------------------------------------------------
//  SCV PCB
//-------------------------------------------------

struct scv_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const scv_slot slot_list[] =
{
	{ SCV_8K,       "rom8k" },
	{ SCV_16K,      "rom16k" },
	{ SCV_32K,      "rom32k" },
	{ SCV_32K_RAM,  "rom32k_ram" },
	{ SCV_64K,      "rom64k" },
	{ SCV_128K,     "rom128k" },
	{ SCV_128K_RAM, "rom128k_ram" }
};

static int scv_get_pcb_id(const char *slot)
{
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (!core_stricmp(slot_list[i].slot_option, slot))
			return slot_list[i].pcb_id;
	}

	return 0;
}

static const char *scv_get_slot(int type)
{
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (slot_list[i].pcb_id == type)
			return slot_list[i].slot_option;
	}

	return "rom8k";
}


/*-------------------------------------------------
 call load
 -------------------------------------------------*/

bool scv_cart_slot_device::call_load()
{
	if (m_cart)
	{
		UINT8 *ROM;
		UINT32 len = (software_entry() == NULL) ? length() : get_software_region_length("rom");
		bool has_ram = (software_entry() != NULL) && get_software_region("ram");

		if (len > 0x20000)
		{
			seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
			return IMAGE_INIT_FAIL;
		}

		m_cart->rom_alloc(len, tag());
		if (has_ram)
			m_cart->ram_alloc(get_software_region_length("ram"));

		ROM = m_cart->get_rom_base();

		if (software_entry() == NULL)
			fread(ROM, len);
		else
			memcpy(ROM, get_software_region("rom"), len);

		if (software_entry() == NULL)
			m_type = get_cart_type(ROM, len);
		else
		{
			const char *pcb_name = get_feature("slot");
			if (pcb_name)
				m_type = scv_get_pcb_id(pcb_name);
		}

		// for the moment we only support RAM from softlist and in the following configurations
		// 1) 32K ROM + 8K RAM; 2) 128K ROM + 4K RAM
		if (m_type == SCV_32K && has_ram)
			m_type = SCV_32K_RAM;
		if (m_type == SCV_128K && has_ram)
			m_type = SCV_128K_RAM;

		//printf("Type: %s\n", scv_get_slot(m_type));

		return IMAGE_INIT_PASS;
	}

	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
 call softlist load
 -------------------------------------------------*/

bool scv_cart_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry);
	return TRUE;
}



/*-------------------------------------------------
 get_cart_type - code to detect NVRAM type from
 fullpath
 -------------------------------------------------*/

int scv_cart_slot_device::get_cart_type(UINT8 *ROM, UINT32 len)
{
	int type = SCV_8K;

	// TO DO: is there any way to identify carts with RAM?!?
	switch (len)
	{
		case 0x2000:
			type = SCV_8K;
			break;
		case 0x4000:
			type = SCV_16K;
			break;
		case 0x8000:
			type = SCV_32K;
			break;
		case 0x10000:
			type = SCV_64K;
			break;
		case 0x20000:
			type = SCV_128K;
			break;
	}

	return type;
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

void scv_cart_slot_device::get_default_card_software(std::string &result)
{
	if (open_image_file(mconfig().options()))
	{
		const char *slot_string = "rom8k";
		UINT32 len = core_fsize(m_file);
		dynamic_buffer rom(len);
		int type;

		core_fread(m_file, &rom[0], len);

		type = get_cart_type(&rom[0], len);
		slot_string = scv_get_slot(type);

		//printf("type: %s\n", slot_string);
		clear();

		result.assign(slot_string);
		return;
	}

	software_get_default_slot(result, "rom8k");
}

/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ8_MEMBER(scv_cart_slot_device::read_cart)
{
	if (m_cart)
		return m_cart->read_cart(space, offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 write
 -------------------------------------------------*/

WRITE8_MEMBER(scv_cart_slot_device::write_cart)
{
	if (m_cart)
		m_cart->write_cart(space, offset, data);
}


/*-------------------------------------------------
 write_bank
 -------------------------------------------------*/

WRITE8_MEMBER(scv_cart_slot_device::write_bank)
{
	if (m_cart)
		m_cart->write_bank(space, offset, data);
}
