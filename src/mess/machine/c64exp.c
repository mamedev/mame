/**********************************************************************

    Commodore 64 Expansion Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "machine/c64exp.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_EXPANSION_SLOT = &device_creator<c64_expansion_slot_device>;



//**************************************************************************
//  DEVICE C64_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_c64_expansion_card_interface - constructor
//-------------------------------------------------

device_c64_expansion_card_interface::device_c64_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
	  m_roml(NULL),
	  m_romh(NULL),
	  m_ram(NULL),
	  m_nvram(NULL),
	  m_nvram_size(0),
	  m_roml_mask(0),
	  m_romh_mask(0),
	  m_ram_mask(0),
	  m_game(1),
	  m_exrom(1)
{
	m_slot = dynamic_cast<c64_expansion_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_c64_expansion_card_interface - destructor
//-------------------------------------------------

device_c64_expansion_card_interface::~device_c64_expansion_card_interface()
{
}


//-------------------------------------------------
//  c64_roml_pointer - get low ROM pointer
//-------------------------------------------------

UINT8* device_c64_expansion_card_interface::c64_roml_pointer(running_machine &machine, size_t size)
{
	if (m_roml == NULL)
	{
		m_roml = auto_alloc_array(machine, UINT8, size);

		m_roml_mask = size - 1;
	}

	return m_roml;
}


//-------------------------------------------------
//  c64_romh_pointer - get high ROM pointer
//-------------------------------------------------

UINT8* device_c64_expansion_card_interface::c64_romh_pointer(running_machine &machine, size_t size)
{
	if (m_romh == NULL)
	{
		m_romh = auto_alloc_array(machine, UINT8, size);

		m_romh_mask = size - 1;
	}

	return m_romh;
}


//-------------------------------------------------
//  c64_ram_pointer - get RAM pointer
//-------------------------------------------------

UINT8* device_c64_expansion_card_interface::c64_ram_pointer(running_machine &machine, size_t size)
{
	if (m_ram == NULL)
	{
		m_ram = auto_alloc_array(machine, UINT8, size);

		m_ram_mask = size - 1;
	}

	return m_ram;
}


//-------------------------------------------------
//  c64_ram_pointer - get NVRAM pointer
//-------------------------------------------------

UINT8* device_c64_expansion_card_interface::c64_nvram_pointer(running_machine &machine, size_t size)
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
//  c64_expansion_slot_device - constructor
//-------------------------------------------------

c64_expansion_slot_device::c64_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, C64_EXPANSION_SLOT, "C64 expansion port", tag, owner, clock),
		device_slot_interface(mconfig, *this),
		device_image_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  c64_expansion_slot_device - destructor
//-------------------------------------------------

c64_expansion_slot_device::~c64_expansion_slot_device()
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void c64_expansion_slot_device::device_config_complete()
{
	// inherit a copy of the static data
	const c64_expansion_slot_interface *intf = reinterpret_cast<const c64_expansion_slot_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<c64_expansion_slot_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
    	memset(&m_in_dma_cd_cb, 0, sizeof(m_in_dma_cd_cb));
    	memset(&m_out_dma_cd_cb, 0, sizeof(m_out_dma_cd_cb));
    	memset(&m_out_irq_cb, 0, sizeof(m_out_irq_cb));
    	memset(&m_out_nmi_cb, 0, sizeof(m_out_nmi_cb));
    	memset(&m_out_dma_cb, 0, sizeof(m_out_dma_cb));
    	memset(&m_out_reset_cb, 0, sizeof(m_out_reset_cb));
	}

	// set brief and instance name
	update_names();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_expansion_slot_device::device_start()
{
	m_cart = dynamic_cast<device_c64_expansion_card_interface *>(get_card_device());

	// resolve callbacks
	m_in_dma_cd_func.resolve(m_in_dma_cd_cb, *this);
	m_out_dma_cd_func.resolve(m_out_dma_cd_cb, *this);
	m_out_irq_func.resolve(m_out_irq_cb, *this);
	m_out_nmi_func.resolve(m_out_nmi_cb, *this);
	m_out_dma_func.resolve(m_out_dma_cb, *this);
	m_out_reset_func.resolve(m_out_reset_cb, *this);

	// inherit bus clock
	if (clock() == 0)
	{
		c64_expansion_slot_device *root = machine().device<c64_expansion_slot_device>(C64_EXPANSION_SLOT_TAG);
		assert(root);
		set_unscaled_clock(root->clock());
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_expansion_slot_device::device_reset()
{
	port_reset_w(ASSERT_LINE);
	port_reset_w(CLEAR_LINE);
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

bool c64_expansion_slot_device::call_load()
{
	if (m_cart)
	{
		size_t size = 0;

		if (software_entry() == NULL)
		{
			size = length();

			if (!mame_stricmp(filetype(), "80"))
			{
				fread(m_cart->c64_roml_pointer(machine(), size), size);
				m_cart->m_exrom = (0);

				if (size == 0x4000)
				{
					m_cart->m_game = 0;
				}
			}
			else if (!mame_stricmp(filetype(), "a0"))
			{
				fread(m_cart->c64_romh_pointer(machine(), 0x2000), 0x2000);

				m_cart->m_exrom = 0;
				m_cart->m_game = 0;
			}
			else if (!mame_stricmp(filetype(), "e0"))
			{
				fread(m_cart->c64_romh_pointer(machine(), 0x2000), 0x2000);

				m_cart->m_game = 0;
			}
			else if (!mame_stricmp(filetype(), "crt"))
			{
				size_t roml_size = 0;
				size_t romh_size = 0;
				int exrom = 1;
				int game = 1;

				if (cbm_crt_read_header(m_file, &roml_size, &romh_size, &exrom, &game))
				{
					UINT8 *roml = NULL;
					UINT8 *romh = NULL;

					if (roml_size) roml = m_cart->c64_roml_pointer(machine(), roml_size);
					if (romh_size) romh = m_cart->c64_romh_pointer(machine(), romh_size);

					cbm_crt_read_data(m_file, roml, romh);
				}

				m_cart->m_exrom = exrom;
				m_cart->m_game = game;
			}
		}
		else
		{
			size = get_software_region_length("uprom");

			if (size)
			{
				// Ultimax (VIC-10) cartridge
				memcpy(m_cart->c64_romh_pointer(machine(), size), get_software_region("uprom"), size);

				size = get_software_region_length("lorom");
				if (size) memcpy(m_cart->c64_roml_pointer(machine(), size), get_software_region("lorom"), size);

				m_cart->m_exrom = 1;
				m_cart->m_game = 0;
			}
			else
			{
				// Commodore 64/128 cartridge
				size = get_software_region_length("roml");
				if (size) memcpy(m_cart->c64_roml_pointer(machine(), size), get_software_region("roml"), size);

				size = get_software_region_length("romh");
				if (size) memcpy(m_cart->c64_romh_pointer(machine(), size), get_software_region("romh"), size);

				size = get_software_region_length("ram");
				if (size) memset(m_cart->c64_ram_pointer(machine(), size), 0, size);

				size = get_software_region_length("nvram");
				if (size) memset(m_cart->c64_nvram_pointer(machine(), size), 0, size);

				m_cart->m_exrom = atol(get_feature("exrom"));
				m_cart->m_game = atol(get_feature("game"));
			}
		}
	}

	return IMAGE_INIT_PASS;
}


//-------------------------------------------------
//  call_softlist_load -
//-------------------------------------------------

bool c64_expansion_slot_device::call_softlist_load(char *swlist, char *swname, rom_entry *start_entry)
{
	load_software_part_region(this, swlist, swname, start_entry);

	return true;
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

const char * c64_expansion_slot_device::get_default_card_software(const machine_config &config, emu_options &options)
{
	if (open_image_file(options))
	{
		if (!mame_stricmp(filetype(), "crt"))
		{
			return cbm_crt_get_card(m_file);
		}

		clear();
	}

	return software_get_default_slot(config, options, this, "standard");
}


//-------------------------------------------------
//  cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_expansion_slot_device::cd_r(address_space &space, offs_t offset, UINT8 data, int ba, int roml, int romh, int io1, int io2)
{
	if (m_cart != NULL)
	{
		data = m_cart->c64_cd_r(space, offset, data, ba, roml, romh, io1, io2);
	}

	return data;
}


//-------------------------------------------------
//  cd_w - cartridge data write
//-------------------------------------------------

void c64_expansion_slot_device::cd_w(address_space &space, offs_t offset, UINT8 data, int ba, int roml, int romh, int io1, int io2)
{
	if (m_cart != NULL)
	{
		m_cart->c64_cd_w(space, offset, data, ba, roml, romh, io1, io2);
	}
}


//-------------------------------------------------
//  game_r - GAME read
//-------------------------------------------------

int c64_expansion_slot_device::game_r(offs_t offset, int ba, int rw, int hiram)
{
	int state = 1;

	if (m_cart != NULL)
	{
		state = m_cart->c64_game_r(offset, ba, rw, hiram);
	}

	return state;
}


//-------------------------------------------------
//  exrom_r - EXROM read
//-------------------------------------------------

int c64_expansion_slot_device::exrom_r(offs_t offset, int ba, int rw, int hiram)
{
	int state = 1;

	if (m_cart != NULL)
	{
		state = m_cart->c64_exrom_r(offset, ba, rw, hiram);
	}

	return state;
}


WRITE_LINE_MEMBER( c64_expansion_slot_device::port_reset_w ) { if (m_cart != NULL) m_cart->c64_reset_w(state); }


//-------------------------------------------------
//  dma_cd_r - DMA read
//-------------------------------------------------

UINT8 c64_expansion_slot_device::dma_cd_r(offs_t offset)
{
	return m_in_dma_cd_func(offset);
}


//-------------------------------------------------
//  dma_cd_w - DMA write
//-------------------------------------------------

void c64_expansion_slot_device::dma_cd_w(offs_t offset, UINT8 data)
{
	m_out_dma_cd_func(offset, data);
}


WRITE_LINE_MEMBER( c64_expansion_slot_device::irq_w ) { m_out_irq_func(state); }
WRITE_LINE_MEMBER( c64_expansion_slot_device::nmi_w ) { m_out_nmi_func(state); }
WRITE_LINE_MEMBER( c64_expansion_slot_device::dma_w ) { m_out_dma_func(state); }
WRITE_LINE_MEMBER( c64_expansion_slot_device::reset_w ) { m_out_reset_func(state); }


//-------------------------------------------------
//  phi2 - system clock frequency
//-------------------------------------------------

int c64_expansion_slot_device::phi2()
{
	return clock();
}


//-------------------------------------------------
//  dotclock - dot clock frequency
//-------------------------------------------------

int c64_expansion_slot_device::dotclock()
{
	return phi2() * 8;
}
