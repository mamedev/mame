// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

    Fairchild Channel F cart emulation
    (through slot devices)

 ***********************************************************************************************************/


#include "emu.h"
#include "slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(CHANF_CART_SLOT, channelf_cart_slot_device, "chanf_cart_slot", "Fairchild Channel F Cartridge Slot")

//**************************************************************************
//    Channel F cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_channelf_cart_interface - constructor
//-------------------------------------------------

device_channelf_cart_interface::device_channelf_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_rom(nullptr),
		m_rom_size(0)
{
}


//-------------------------------------------------
//  ~device_channelf_cart_interface - destructor
//-------------------------------------------------

device_channelf_cart_interface::~device_channelf_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_channelf_cart_interface::rom_alloc(uint32_t size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(CHANFSLOT_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
	}
}


//-------------------------------------------------
//  ram_alloc - alloc the space for the ram
//-------------------------------------------------

void device_channelf_cart_interface::ram_alloc(uint32_t size)
{
	m_ram.resize(size);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  channelf_cart_slot_device - constructor
//-------------------------------------------------
channelf_cart_slot_device::channelf_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CHANF_CART_SLOT, tag, owner, clock),
	device_image_interface(mconfig, *this),
	device_slot_interface(mconfig, *this),
	m_type(CF_CHESS), m_cart(nullptr)
{
}


//-------------------------------------------------
//  ~channelf_cart_slot_device - destructor
//-------------------------------------------------

channelf_cart_slot_device::~channelf_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void channelf_cart_slot_device::device_start()
{
	m_cart = dynamic_cast<device_channelf_cart_interface *>(get_card_device());
}


//-------------------------------------------------
//  Channel F PCB
//-------------------------------------------------

struct chanf_slot
{
	int                     pcb_id;
	const char              *slot_option;
};

// Here, we take the feature attribute from .xml (i.e. the PCB name) and we assign a unique ID to it
static const chanf_slot slot_list[] =
{
	{ CF_STD,      "std" },
	{ CF_MAZE,     "maze" },
	{ CF_HANGMAN,  "hangman" },
	{ CF_CHESS,    "chess" },
	{ CF_MULTI_OLD,"multi_old" },
	{ CF_MULTI,    "multi" }
};

static int chanf_get_pcb_id(const char *slot)
{
	for (auto & elem : slot_list)
	{
		if (!core_stricmp(elem.slot_option, slot))
			return elem.pcb_id;
	}

	return 0;
}

static const char *chanf_get_slot(int type)
{
	for (auto & elem : slot_list)
	{
		if (elem.pcb_id == type)
			return elem.slot_option;
	}

	return "chess";
}


/*-------------------------------------------------
 call load
 -------------------------------------------------*/

image_init_result channelf_cart_slot_device::call_load()
{
	if (m_cart)
	{
		uint32_t len = !loaded_through_softlist() ? length() : get_software_region_length("rom");
		m_cart->rom_alloc(len, tag());

		if (!loaded_through_softlist())
			fread(m_cart->get_rom_base(), len);
		else
			memcpy(m_cart->get_rom_base(), get_software_region("rom"), len);

		if (!loaded_through_softlist())
		{
			// we default to "chess" slot because some homebrew programs have been written to run
			// on PCBs with RAM at $2000-$2800 as Saba Schach!
			if (len == 0x40000)
				m_type = CF_MULTI;  // TODO1: differentiate multicart final and earlier from fullpath
			else
				m_type = CF_CHESS;  // TODO2: is there any way to detect Maze and Hangman from fullpath?

			m_cart->ram_alloc(0x800);
		}
		else
		{
			const char *pcb_name = get_feature("slot");
			if (pcb_name)
				m_type = chanf_get_pcb_id(pcb_name);

			if (get_software_region("ram"))
				m_cart->ram_alloc(get_software_region_length("ram"));
		}

		//printf("Type: %s\n", chanf_get_slot(m_type));

		return image_init_result::PASS;
	}

	return image_init_result::PASS;
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string channelf_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
	{
		const char *slot_string;
		uint32_t len = hook.image_file()->size();
		int type;

		if (len == 0x40000)
			type = CF_MULTI;
		else
			type = CF_CHESS;    // is there any way to detect the other carts from fullpath?

		slot_string = chanf_get_slot(type);

		//printf("type: %s\n", slot_string);

		return std::string(slot_string);
	}
	return software_get_default_slot("chess");
}

/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ8_MEMBER(channelf_cart_slot_device::read_rom)
{
	if (m_cart)
		return m_cart->read_rom(space, offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 read
 -------------------------------------------------*/

READ8_MEMBER(channelf_cart_slot_device::read_ram)
{
	if (m_cart)
		return m_cart->read_ram(space, offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 write
 -------------------------------------------------*/

WRITE8_MEMBER(channelf_cart_slot_device::write_ram)
{
	if (m_cart)
		m_cart->write_ram(space, offset, data);
}

/*-------------------------------------------------
 write
 -------------------------------------------------*/

WRITE8_MEMBER(channelf_cart_slot_device::write_bank)
{
	if (m_cart)
		m_cart->write_bank(space, offset, data);
}
