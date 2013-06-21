/**********************************************************************

    Commodore 64 Expansion Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "machine/c64exp.h"



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
		device_t(mconfig, C64_EXPANSION_SLOT, "C64 expansion port", tag, owner, clock, "c64_expansion_slot", __FILE__),
		device_slot_interface(mconfig, *this),
		device_image_interface(mconfig, *this),
		m_read_dma_cd(*this),
		m_write_dma_cd(*this),
		m_write_irq(*this),
		m_write_nmi(*this),
		m_write_dma(*this),
		m_write_reset(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_expansion_slot_device::device_start()
{
	m_card = dynamic_cast<device_c64_expansion_card_interface *>(get_card_device());

	// resolve callbacks
	m_read_dma_cd.resolve_safe(0);
	m_write_dma_cd.resolve_safe();
	m_write_irq.resolve_safe();
	m_write_nmi.resolve_safe();
	m_write_dma.resolve_safe();
	m_write_reset.resolve_safe();

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
	if (get_card_device())
	{
		get_card_device()->reset();
	}
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

bool c64_expansion_slot_device::call_load()
{
	if (m_card)
	{
		size_t size = 0;

		if (software_entry() == NULL)
		{
			size = length();

			if (!mame_stricmp(filetype(), "80"))
			{
				fread(m_card->c64_roml_pointer(machine(), size), size);
				m_card->m_exrom = (0);

				if (size == 0x4000)
				{
					m_card->m_game = 0;
				}
			}
			else if (!mame_stricmp(filetype(), "a0"))
			{
				fread(m_card->c64_romh_pointer(machine(), 0x2000), 0x2000);

				m_card->m_exrom = 0;
				m_card->m_game = 0;
			}
			else if (!mame_stricmp(filetype(), "e0"))
			{
				fread(m_card->c64_romh_pointer(machine(), 0x2000), 0x2000);

				m_card->m_game = 0;
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

					if (roml_size) roml = m_card->c64_roml_pointer(machine(), roml_size);
					if (romh_size) romh = m_card->c64_romh_pointer(machine(), romh_size);

					cbm_crt_read_data(m_file, roml, romh);
				}

				m_card->m_exrom = exrom;
				m_card->m_game = game;
			}
		}
		else
		{
			size = get_software_region_length("uprom");

			if (size)
			{
				// Ultimax (VIC-10) cartridge
				memcpy(m_card->c64_romh_pointer(machine(), size), get_software_region("uprom"), size);

				size = get_software_region_length("lorom");
				if (size) memcpy(m_card->c64_roml_pointer(machine(), size), get_software_region("lorom"), size);

				m_card->m_exrom = 1;
				m_card->m_game = 0;
			}
			else
			{
				// Commodore 64/128 cartridge
				size = get_software_region_length("roml");
				if (size) memcpy(m_card->c64_roml_pointer(machine(), size), get_software_region("roml"), size);

				size = get_software_region_length("romh");
				if (size) memcpy(m_card->c64_romh_pointer(machine(), size), get_software_region("romh"), size);

				size = get_software_region_length("ram");
				if (size) memset(m_card->c64_ram_pointer(machine(), size), 0, size);

				size = get_software_region_length("nvram");
				if (size) memset(m_card->c64_nvram_pointer(machine(), size), 0, size);

				if (get_feature("exrom") != NULL) m_card->m_exrom = atol(get_feature("exrom"));
				if (get_feature("game") != NULL) m_card->m_game = atol(get_feature("game"));
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

UINT8 c64_expansion_slot_device::cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (m_card != NULL)
	{
		data = m_card->c64_cd_r(space, offset, data, sphi2, ba, roml, romh, io1, io2);
	}

	return data;
}


//-------------------------------------------------
//  cd_w - cartridge data write
//-------------------------------------------------

void c64_expansion_slot_device::cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (m_card != NULL)
	{
		m_card->c64_cd_w(space, offset, data, sphi2, ba, roml, romh, io1, io2);
	}
}


//-------------------------------------------------
//  game_r - GAME read
//-------------------------------------------------

int c64_expansion_slot_device::game_r(offs_t offset, int sphi2, int ba, int rw, int hiram)
{
	int state = 1;

	if (m_card != NULL)
	{
		state = m_card->c64_game_r(offset, sphi2, ba, rw, hiram);
	}

	return state;
}


//-------------------------------------------------
//  exrom_r - EXROM read
//-------------------------------------------------

int c64_expansion_slot_device::exrom_r(offs_t offset, int sphi2, int ba, int rw, int hiram)
{
	int state = 1;

	if (m_card != NULL)
	{
		state = m_card->c64_exrom_r(offset, sphi2, ba, rw, hiram);
	}

	return state;
}
