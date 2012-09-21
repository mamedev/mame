/**********************************************************************

    Commodore CBM-II Expansion Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "machine/cbm2exp.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CBM2_EXPANSION_SLOT = &device_creator<cbm2_expansion_slot_device>;



//**************************************************************************
//  DEVICE CBM2_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_cbm2_expansion_card_interface - constructor
//-------------------------------------------------

device_cbm2_expansion_card_interface::device_cbm2_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
	  m_bank1(NULL),
	  m_bank2(NULL),
	  m_bank3(NULL),
	  m_ram(NULL),
	  m_nvram(NULL),
	  m_nvram_size(0),
	  m_bank1_mask(0),
	  m_bank2_mask(0),
	  m_bank3_mask(0),
	  m_ram_mask(0)
{
	m_slot = dynamic_cast<cbm2_expansion_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_cbm2_expansion_card_interface - destructor
//-------------------------------------------------

device_cbm2_expansion_card_interface::~device_cbm2_expansion_card_interface()
{
}


//-------------------------------------------------
//  cbm2_bank1_pointer - get bank 1 pointer
//-------------------------------------------------

UINT8* device_cbm2_expansion_card_interface::cbm2_bank1_pointer(running_machine &machine, size_t size)
{
	if (m_bank1 == NULL)
	{
		m_bank1 = auto_alloc_array(machine, UINT8, size);

		m_bank1_mask = size - 1;
	}

	return m_bank1;
}


//-------------------------------------------------
//  cbm2_bank2_pointer - get bank 2 pointer
//-------------------------------------------------

UINT8* device_cbm2_expansion_card_interface::cbm2_bank2_pointer(running_machine &machine, size_t size)
{
	if (m_bank2 == NULL)
	{
		m_bank2 = auto_alloc_array(machine, UINT8, size);

		m_bank2_mask = size - 1;
	}

	return m_bank2;
}


//-------------------------------------------------
//  cbm2_bank3_pointer - get bank 3 pointer
//-------------------------------------------------

UINT8* device_cbm2_expansion_card_interface::cbm2_bank3_pointer(running_machine &machine, size_t size)
{
	if (m_bank3 == NULL)
	{
		m_bank3 = auto_alloc_array(machine, UINT8, size);

		m_bank3_mask = size - 1;
	}

	return m_bank1;
}


//-------------------------------------------------
//  cbm2_ram_pointer - get RAM pointer
//-------------------------------------------------

UINT8* device_cbm2_expansion_card_interface::cbm2_ram_pointer(running_machine &machine, size_t size)
{
	if (m_ram == NULL)
	{
		m_ram = auto_alloc_array(machine, UINT8, size);

		m_ram_mask = size - 1;
	}

	return m_ram;
}


//-------------------------------------------------
//  cbm2_ram_pointer - get NVRAM pointer
//-------------------------------------------------

UINT8* device_cbm2_expansion_card_interface::cbm2_nvram_pointer(running_machine &machine, size_t size)
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
//  cbm2_expansion_slot_device - constructor
//-------------------------------------------------

cbm2_expansion_slot_device::cbm2_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, CBM2_EXPANSION_SLOT, "CBM-II expansion port", tag, owner, clock),
		device_slot_interface(mconfig, *this),
		device_image_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  cbm2_expansion_slot_device - destructor
//-------------------------------------------------

cbm2_expansion_slot_device::~cbm2_expansion_slot_device()
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void cbm2_expansion_slot_device::device_config_complete()
{
	// set brief and instance name
	update_names();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cbm2_expansion_slot_device::device_start()
{
	m_cart = dynamic_cast<device_cbm2_expansion_card_interface *>(get_card_device());

	// inherit bus clock
	if (clock() == 0)
	{
		cbm2_expansion_slot_device *root = machine().device<cbm2_expansion_slot_device>(CBM2_EXPANSION_SLOT_TAG);
		assert(root);
		set_unscaled_clock(root->clock());
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cbm2_expansion_slot_device::device_reset()
{
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

bool cbm2_expansion_slot_device::call_load()
{
	size_t size = 0;

	if (m_cart)
	{
		if (software_entry() == NULL)
		{
			size = length();

			if (!mame_stricmp(filetype(), "20"))
			{
				fread(m_cart->cbm2_bank1_pointer(machine(), size), size);
			}
			else if (!mame_stricmp(filetype(), "40"))
			{
				fread(m_cart->cbm2_bank2_pointer(machine(), size), size);
			}
			else if (!mame_stricmp(filetype(), "60"))
			{
				fread(m_cart->cbm2_bank3_pointer(machine(), size), size);
			}
		}
		else
		{
			size = get_software_region_length("bank1");
			if (size) memcpy(m_cart->cbm2_bank1_pointer(machine(), size), get_software_region("bank1"), size);

			size = get_software_region_length("bank2");
			if (size) memcpy(m_cart->cbm2_bank2_pointer(machine(), size), get_software_region("bank2"), size);

			size = get_software_region_length("bank3");
			if (size) memcpy(m_cart->cbm2_bank3_pointer(machine(), size), get_software_region("bank3"), size);

			size = get_software_region_length("ram");
			if (size) memset(m_cart->cbm2_ram_pointer(machine(), size), 0, size);

			size = get_software_region_length("nvram");
			if (size) memset(m_cart->cbm2_nvram_pointer(machine(), size), 0, size);
		}
	}

	return IMAGE_INIT_PASS;
}


//-------------------------------------------------
//  call_softlist_load -
//-------------------------------------------------

bool cbm2_expansion_slot_device::call_softlist_load(char *swlist, char *swname, rom_entry *start_entry)
{
	load_software_part_region(this, swlist, swname, start_entry);

	return true;
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

const char * cbm2_expansion_slot_device::get_default_card_software(const machine_config &config, emu_options &options)
{
	return software_get_default_slot(config, options, this, "standard");
}


//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

UINT8 cbm2_expansion_slot_device::read(address_space &space, offs_t offset, UINT8 data, int csbank1, int csbank2, int csbank3)
{
	if (m_cart != NULL)
	{
		data = m_cart->cbm2_bd_r(space, offset, data, csbank1, csbank2, csbank3);
	}

	return data;
}


//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void cbm2_expansion_slot_device::write(address_space &space, offs_t offset, UINT8 data, int csbank1, int csbank2, int csbank3)
{
	if (m_cart != NULL)
	{
		m_cart->cbm2_bd_w(space, offset, data, csbank1, csbank2, csbank3);
	}
}


//-------------------------------------------------
//  phi2 - system clock frequency
//-------------------------------------------------

int cbm2_expansion_slot_device::phi2()
{
	return clock();
}
