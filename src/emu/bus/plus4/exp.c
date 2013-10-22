// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore Plus/4 Expansion Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "exp.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PLUS4_EXPANSION_SLOT = &device_creator<plus4_expansion_slot_device>;



//**************************************************************************
//  DEVICE PLUS4_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_plus4_expansion_card_interface - constructor
//-------------------------------------------------

device_plus4_expansion_card_interface::device_plus4_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_c1l(NULL),
		m_c1h(NULL),
		m_c2l(NULL),
		m_c2h(NULL),
		m_ram(NULL),
		m_nvram(NULL),
		m_nvram_size(0),
		m_c1l_mask(0),
		m_c1h_mask(0),
		m_c2l_mask(0),
		m_c2h_mask(0),
		m_ram_mask(0)
{
	m_slot = dynamic_cast<plus4_expansion_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_plus4_expansion_card_interface - destructor
//-------------------------------------------------

device_plus4_expansion_card_interface::~device_plus4_expansion_card_interface()
{
}


//-------------------------------------------------
//  plus4_c1l_pointer - get low ROM 1 pointer
//-------------------------------------------------

UINT8* device_plus4_expansion_card_interface::plus4_c1l_pointer(running_machine &machine, size_t size)
{
	if (m_c1l == NULL)
	{
		m_c1l = auto_alloc_array(machine, UINT8, size);

		m_c1l_mask = size - 1;
	}

	return m_c1l;
}


//-------------------------------------------------
//  plus4_c1h_pointer - get low ROM 1 pointer
//-------------------------------------------------

UINT8* device_plus4_expansion_card_interface::plus4_c1h_pointer(running_machine &machine, size_t size)
{
	if (m_c1h == NULL)
	{
		m_c1h = auto_alloc_array(machine, UINT8, size);

		m_c1h_mask = size - 1;
	}

	return m_c1h;
}


//-------------------------------------------------
//  plus4_c2l_pointer - get low ROM 1 pointer
//-------------------------------------------------

UINT8* device_plus4_expansion_card_interface::plus4_c2l_pointer(running_machine &machine, size_t size)
{
	if (m_c2l == NULL)
	{
		m_c2l = auto_alloc_array(machine, UINT8, size);

		m_c2l_mask = size - 1;
	}

	return m_c2l;
}


//-------------------------------------------------
//  plus4_c2h_pointer - get low ROM 1 pointer
//-------------------------------------------------

UINT8* device_plus4_expansion_card_interface::plus4_c2h_pointer(running_machine &machine, size_t size)
{
	if (m_c2h == NULL)
	{
		m_c2h = auto_alloc_array(machine, UINT8, size);

		m_c2h_mask = size - 1;
	}

	return m_c2h;
}


//-------------------------------------------------
//  plus4_ram_pointer - get RAM pointer
//-------------------------------------------------

UINT8* device_plus4_expansion_card_interface::plus4_ram_pointer(running_machine &machine, size_t size)
{
	if (m_ram == NULL)
	{
		m_ram = auto_alloc_array(machine, UINT8, size);

		m_ram_mask = size - 1;
	}

	return m_ram;
}


//-------------------------------------------------
//  plus4_ram_pointer - get NVRAM pointer
//-------------------------------------------------

UINT8* device_plus4_expansion_card_interface::plus4_nvram_pointer(running_machine &machine, size_t size)
{
	if (m_nvram == NULL)
	{
		m_nvram = auto_alloc_array(machine, UINT8, size);

		m_nvram_mask = size - 1;
		m_nvram_size = size;
	}

	return m_nvram;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  plus4_expansion_slot_device - constructor
//-------------------------------------------------

plus4_expansion_slot_device::plus4_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, PLUS4_EXPANSION_SLOT, "Expansion Port", tag, owner, clock, "plus4_expansion_slot", __FILE__),
		device_slot_interface(mconfig, *this),
		device_image_interface(mconfig, *this),
		m_write_irq(*this),
		m_read_dma_cd(*this),
		m_write_dma_cd(*this),
		m_write_aec(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void plus4_expansion_slot_device::device_start()
{
	m_card = dynamic_cast<device_plus4_expansion_card_interface *>(get_card_device());

	// resolve callbacks
	m_write_irq.resolve_safe();
	m_read_dma_cd.resolve_safe(0xff);
	m_write_dma_cd.resolve_safe();
	m_write_aec.resolve_safe();

	// inherit bus clock
	if (clock() == 0)
	{
		plus4_expansion_slot_device *root = machine().device<plus4_expansion_slot_device>(PLUS4_EXPANSION_SLOT_TAG);
		assert(root);
		set_unscaled_clock(root->clock());
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void plus4_expansion_slot_device::device_reset()
{
	if (get_card_device())
	{
		get_card_device()->reset();
	}
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

bool plus4_expansion_slot_device::call_load()
{
	if (m_card)
	{
		size_t size = 0;

		if (software_entry() == NULL)
		{
			// TODO
		}
		else
		{
			size = get_software_region_length("c1l");
			if (size) memcpy(m_card->plus4_c1l_pointer(machine(), size), get_software_region("c1l"), size);

			size = get_software_region_length("c1h");
			if (size) memcpy(m_card->plus4_c1h_pointer(machine(), size), get_software_region("c1h"), size);

			size = get_software_region_length("c2l");
			if (size) memcpy(m_card->plus4_c2l_pointer(machine(), size), get_software_region("c2l"), size);

			size = get_software_region_length("c2h");
			if (size) memcpy(m_card->plus4_c2h_pointer(machine(), size), get_software_region("c2h"), size);

			size = get_software_region_length("ram");
			if (size) memset(m_card->plus4_ram_pointer(machine(), size), 0, size);

			size = get_software_region_length("nvram");
			if (size) memset(m_card->plus4_nvram_pointer(machine(), size), 0, size);
		}
	}

	return IMAGE_INIT_PASS;
}


//-------------------------------------------------
//  call_softlist_load -
//-------------------------------------------------

bool plus4_expansion_slot_device::call_softlist_load(char *swlist, char *swname, rom_entry *start_entry)
{
	load_software_part_region(this, swlist, swname, start_entry);

	return true;
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

const char * plus4_expansion_slot_device::get_default_card_software(const machine_config &config, emu_options &options)
{
	return software_get_default_slot(config, options, this, "standard");
}


//-------------------------------------------------
//  cd_r - cartridge data read
//-------------------------------------------------

UINT8 plus4_expansion_slot_device::cd_r(address_space &space, offs_t offset, UINT8 data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h)
{
	if (m_card != NULL)
	{
		data = m_card->plus4_cd_r(space, offset, data, ba, cs0, c1l, c1h, cs1, c2l, c2h);
	}

	return data;
}


//-------------------------------------------------
//  cd_w - cartridge data write
//-------------------------------------------------

void plus4_expansion_slot_device::cd_w(address_space &space, offs_t offset, UINT8 data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h)
{
	if (m_card != NULL)
	{
		m_card->plus4_cd_w(space, offset, data, ba, cs0, c1l, c1h, cs1, c2l, c2h);
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( plus4_expansion_cards )
//-------------------------------------------------

SLOT_INTERFACE_START( plus4_expansion_cards )
	SLOT_INTERFACE("c1551", C1551)
	SLOT_INTERFACE("sid", PLUS4_SID)

	// the following need ROMs from the software list
	SLOT_INTERFACE_INTERNAL("standard", PLUS4_STD)
SLOT_INTERFACE_END
