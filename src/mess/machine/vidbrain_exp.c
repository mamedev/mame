/**********************************************************************

    VideoBrain Expansion Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "machine/vidbrain_exp.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VIDEOBRAIN_EXPANSION_SLOT = &device_creator<videobrain_expansion_slot_device>;



//**************************************************************************
//  DEVICE VIDEOBRAIN_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_videobrain_expansion_card_interface - constructor
//-------------------------------------------------

device_videobrain_expansion_card_interface::device_videobrain_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_rom(NULL),
		m_ram(NULL),
		m_rom_mask(0),
		m_ram_mask(0)
{
	m_slot = dynamic_cast<videobrain_expansion_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_videobrain_expansion_card_interface - destructor
//-------------------------------------------------

device_videobrain_expansion_card_interface::~device_videobrain_expansion_card_interface()
{
}


//-------------------------------------------------
//  videobrain_roml_pointer - get low ROM pointer
//-------------------------------------------------

UINT8* device_videobrain_expansion_card_interface::videobrain_rom_pointer(running_machine &machine, size_t size)
{
	if (m_rom == NULL)
	{
		m_rom = auto_alloc_array(machine, UINT8, size);

		m_rom_mask = size - 1;
	}

	return m_rom;
}


//-------------------------------------------------
//  videobrain_ram_pointer - get RAM pointer
//-------------------------------------------------

UINT8* device_videobrain_expansion_card_interface::videobrain_ram_pointer(running_machine &machine, size_t size)
{
	if (m_ram == NULL)
	{
		m_ram = auto_alloc_array(machine, UINT8, size);

		m_ram_mask = size - 1;
	}

	return m_ram;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  videobrain_expansion_slot_device - constructor
//-------------------------------------------------

videobrain_expansion_slot_device::videobrain_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, VIDEOBRAIN_EXPANSION_SLOT, "VideoBrain expansion port", tag, owner, clock),
		device_slot_interface(mconfig, *this),
		device_image_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  videobrain_expansion_slot_device - destructor
//-------------------------------------------------

videobrain_expansion_slot_device::~videobrain_expansion_slot_device()
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void videobrain_expansion_slot_device::device_config_complete()
{
	// inherit a copy of the static data
	const videobrain_expansion_slot_interface *intf = reinterpret_cast<const videobrain_expansion_slot_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<videobrain_expansion_slot_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_extres_cb, 0, sizeof(m_out_extres_cb));
	}

	// set brief and instance name
	update_names();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void videobrain_expansion_slot_device::device_start()
{
	m_cart = dynamic_cast<device_videobrain_expansion_card_interface *>(get_card_device());

	// resolve callbacks
	m_out_extres_func.resolve(m_out_extres_cb, *this);
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

bool videobrain_expansion_slot_device::call_load()
{
	if (m_cart)
	{
		size_t size = 0;

		if (software_entry() == NULL)
		{
			size = length();

			fread(m_cart->videobrain_rom_pointer(machine(), size), size);
		}
		else
		{
			size = get_software_region_length("rom");
			if (size) memcpy(m_cart->videobrain_rom_pointer(machine(), size), get_software_region("rom"), size);

			size = get_software_region_length("ram");
			if (size) memset(m_cart->videobrain_ram_pointer(machine(), size), 0, size);
		}
	}

	return IMAGE_INIT_PASS;
}


//-------------------------------------------------
//  call_softlist_load -
//-------------------------------------------------

bool videobrain_expansion_slot_device::call_softlist_load(char *swlist, char *swname, rom_entry *start_entry)
{
	load_software_part_region(this, swlist, swname, start_entry);

	return true;
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

const char * videobrain_expansion_slot_device::get_default_card_software(const machine_config &config, emu_options &options)
{
	return software_get_default_slot(config, options, this, "standard");
}


//-------------------------------------------------
//  bo_r - cartridge data read
//-------------------------------------------------

UINT8 videobrain_expansion_slot_device::bo_r(address_space &space, offs_t offset, int cs1, int cs2)
{
	UINT8 data = 0;

	if (m_cart != NULL)
	{
		data = m_cart->videobrain_bo_r(space, offset, cs1, cs2);
	}

	return data;
}


//-------------------------------------------------
//  bo_w - cartridge data write
//-------------------------------------------------

void videobrain_expansion_slot_device::bo_w(address_space &space, offs_t offset, UINT8 data, int cs1, int cs2)
{
	if (m_cart != NULL)
	{
		m_cart->videobrain_bo_w(space, offset, data, cs1, cs2);
	}
}


READ8_MEMBER( videobrain_expansion_slot_device::cs1_r ) { return bo_r(space, offset + 0x1000, 0, 1); }
WRITE8_MEMBER( videobrain_expansion_slot_device::cs1_w ) { bo_w(space, offset + 0x1000, data, 0, 1); }
READ8_MEMBER( videobrain_expansion_slot_device::cs2_r ) { return bo_r(space, offset + 0x1800, 1, 0); }
WRITE8_MEMBER( videobrain_expansion_slot_device::cs2_w ) { bo_w(space, offset + 0x1800, data, 1, 0); }
READ8_MEMBER( videobrain_expansion_slot_device::unmap_r ) { return bo_r(space, offset + 0x3000, 1, 0); }
WRITE8_MEMBER( videobrain_expansion_slot_device::unmap_w ) { bo_w(space, offset + 0x3000, data, 1, 0); }


WRITE_LINE_MEMBER( videobrain_expansion_slot_device::extres_w ) { m_out_extres_func(state); }
