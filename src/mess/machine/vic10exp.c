/**********************************************************************

    Commodore VIC-10 Expansion Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "machine/vic10exp.h"
#include "formats/cbm_crt.h"
#include "formats/imageutl.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type VIC10_EXPANSION_SLOT = &device_creator<vic10_expansion_slot_device>;



//**************************************************************************
//  DEVICE VIC10_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_vic10_expansion_card_interface - constructor
//-------------------------------------------------

device_vic10_expansion_card_interface::device_vic10_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig,device),
	  m_exram(NULL),
	  m_lorom(NULL),
	  m_uprom(NULL)
{
	m_slot = dynamic_cast<vic10_expansion_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_vic10_expansion_card_interface - destructor
//-------------------------------------------------

device_vic10_expansion_card_interface::~device_vic10_expansion_card_interface()
{
}



//-------------------------------------------------
//  vic10_lorom_pointer - get lower ROM pointer
//-------------------------------------------------

UINT8* device_vic10_expansion_card_interface::vic10_lorom_pointer(running_machine &machine, size_t size)
{
	if (m_lorom == NULL)
	{
		m_lorom = auto_alloc_array(machine, UINT8, size);
	}

	return m_lorom;
}


//-------------------------------------------------
//  vic10_uprom_pointer - get upper ROM pointer
//-------------------------------------------------

UINT8* device_vic10_expansion_card_interface::vic10_uprom_pointer(running_machine &machine, size_t size)
{
	if (m_uprom == NULL)
	{
		m_uprom = auto_alloc_array(machine, UINT8, size);
	}

	return m_uprom;
}


//-------------------------------------------------
//  vic10_exram_pointer - get expanded RAM pointer
//-------------------------------------------------

UINT8* device_vic10_expansion_card_interface::vic10_exram_pointer(running_machine &machine, size_t size)
{
	if (m_exram == NULL)
	{
		m_exram = auto_alloc_array(machine, UINT8, size);
	}

	return m_exram;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic10_expansion_slot_device - constructor
//-------------------------------------------------

vic10_expansion_slot_device::vic10_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, VIC10_EXPANSION_SLOT, "VIC-10 expansion port", tag, owner, clock),
		device_slot_interface(mconfig, *this),
		device_image_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  vic10_expansion_slot_device - destructor
//-------------------------------------------------

vic10_expansion_slot_device::~vic10_expansion_slot_device()
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void vic10_expansion_slot_device::device_config_complete()
{
	// inherit a copy of the static data
	const vic10_expansion_slot_interface *intf = reinterpret_cast<const vic10_expansion_slot_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<vic10_expansion_slot_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
    	memset(&m_out_irq_cb, 0, sizeof(m_out_irq_cb));
    	memset(&m_out_sp_cb, 0, sizeof(m_out_sp_cb));
    	memset(&m_out_cnt_cb, 0, sizeof(m_out_cnt_cb));
    	memset(&m_out_res_cb, 0, sizeof(m_out_res_cb));
	}

	// set brief and instance name
	update_names();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic10_expansion_slot_device::device_start()
{
	m_cart = dynamic_cast<device_vic10_expansion_card_interface *>(get_card_device());

	// resolve callbacks
	m_out_irq_func.resolve(m_out_irq_cb, *this);
	m_out_sp_func.resolve(m_out_sp_cb, *this);
	m_out_cnt_func.resolve(m_out_cnt_cb, *this);
	m_out_res_func.resolve(m_out_res_cb, *this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic10_expansion_slot_device::device_reset()
{
	res_w(ASSERT_LINE);
	res_w(CLEAR_LINE);
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

bool vic10_expansion_slot_device::call_load()
{
	if (m_cart)
	{
		size_t size = 0;

		if (software_entry() == NULL)
		{
			size = length();

			if (!mame_stricmp(filetype(), "80"))
			{
				fread(m_cart->vic10_lorom_pointer(machine(), 0x2000), 0x2000);

				if (size == 0x4000)
				{
					fread(m_cart->vic10_uprom_pointer(machine(), 0x2000), 0x2000);
				}
			}
			else if (!mame_stricmp(filetype(), "e0")) fread(m_cart->vic10_uprom_pointer(machine(), size), size);
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

					if (roml_size) roml = m_cart->vic10_lorom_pointer(machine(), roml_size);
					if (romh_size) romh = m_cart->vic10_uprom_pointer(machine(), romh_size);

					cbm_crt_read_data(m_file, roml, romh);
				}
			}
		}
		else
		{
			size = get_software_region_length("lorom");
			if (size) memcpy(m_cart->vic10_lorom_pointer(machine(), size), get_software_region("lorom"), size);

			size = get_software_region_length("uprom");
			if (size) memcpy(m_cart->vic10_uprom_pointer(machine(), size), get_software_region("uprom"), size);

			size = get_software_region_length("exram");
			if (size) m_cart->vic10_exram_pointer(machine(), size);
		}
	}

	return IMAGE_INIT_PASS;
}


//-------------------------------------------------
//  call_softlist_load -
//-------------------------------------------------

bool vic10_expansion_slot_device::call_softlist_load(char *swlist, char *swname, rom_entry *start_entry)
{
	load_software_part_region(this, swlist, swname, start_entry);

	return true;
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

const char * vic10_expansion_slot_device::get_default_card_software(const machine_config &config, emu_options &options)
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
//  screen_update -
//-------------------------------------------------

UINT32 vic10_expansion_slot_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool value = false;

	if (m_cart != NULL)
	{
		value = m_cart->vic10_screen_update(screen, bitmap, cliprect);
	}

	return value;
}


//-------------------------------------------------
//  cd_r - cartridge data read
//-------------------------------------------------

UINT8 vic10_expansion_slot_device::cd_r(address_space &space, offs_t offset, int lorom, int uprom, int exram)
{
	UINT8 data = 0;

	if (m_cart != NULL)
	{
		data = m_cart->vic10_cd_r(space, offset, lorom, uprom, exram);
	}

	return data;
}


//-------------------------------------------------
//  cd_w - cartridge data write
//-------------------------------------------------

void vic10_expansion_slot_device::cd_w(address_space &space, offs_t offset, UINT8 data, int lorom, int uprom, int exram)
{
	if (m_cart != NULL)
	{
		m_cart->vic10_cd_w(space, offset, data, lorom, uprom, exram);
	}
}

WRITE_LINE_MEMBER( vic10_expansion_slot_device::port_res_w ) { if (m_cart != NULL) m_cart->vic10_res_w(state); }

READ_LINE_MEMBER( vic10_expansion_slot_device::p0_r ) { int state = 0; if (m_cart != NULL) state = m_cart->vic10_p0_r(); return state; }
WRITE_LINE_MEMBER( vic10_expansion_slot_device::p0_w ) { if (m_cart != NULL) m_cart->vic10_p0_w(state); }
WRITE_LINE_MEMBER( vic10_expansion_slot_device::irq_w ) { m_out_irq_func(state); }
WRITE_LINE_MEMBER( vic10_expansion_slot_device::sp_w ) { m_out_sp_func(state); if (m_cart != NULL) m_cart->vic10_sp_w(state); }
WRITE_LINE_MEMBER( vic10_expansion_slot_device::cnt_w ) { m_out_cnt_func(state); if (m_cart != NULL) m_cart->vic10_cnt_w(state); }
WRITE_LINE_MEMBER( vic10_expansion_slot_device::res_w ) { m_out_res_func(state); }
