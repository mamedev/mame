/**********************************************************************

    Commodore VIC-20 Expansion Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "formats/cbm_crt.h"
#include "machine/vic20exp.h"
#include "formats/imageutl.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type VIC20_EXPANSION_SLOT = &device_creator<vic20_expansion_slot_device>;


//**************************************************************************
//  DEVICE VIC20_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_vic20_expansion_card_interface - constructor
//-------------------------------------------------

device_vic20_expansion_card_interface::device_vic20_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
	  m_blk1(NULL),
	  m_blk2(NULL),
	  m_blk3(NULL),
	  m_blk5(NULL),
	  m_ram(NULL),
	  m_nvram(NULL),
	  m_nvram_size(0)
{
	m_slot = dynamic_cast<vic20_expansion_slot_device *>(device.owner());
}


//-------------------------------------------------
//  vic20_blk1_pointer - get block 1 pointer
//-------------------------------------------------

UINT8* device_vic20_expansion_card_interface::vic20_blk1_pointer(running_machine &machine, size_t size)
{
	if (m_blk1 == NULL)
	{
		m_blk1 = auto_alloc_array(machine, UINT8, size);
	}

	return m_blk1;
}


//-------------------------------------------------
//  vic20_blk2_pointer - get block 2 pointer
//-------------------------------------------------

UINT8* device_vic20_expansion_card_interface::vic20_blk2_pointer(running_machine &machine, size_t size)
{
	if (m_blk2 == NULL)
	{
		m_blk2 = auto_alloc_array(machine, UINT8, size);
	}

	return m_blk2;
}


//-------------------------------------------------
//  vic20_blk3_pointer - get block 3 pointer
//-------------------------------------------------

UINT8* device_vic20_expansion_card_interface::vic20_blk3_pointer(running_machine &machine, size_t size)
{
	if (m_blk3 == NULL)
	{
		m_blk3 = auto_alloc_array(machine, UINT8, size);
	}

	return m_blk3;
}


//-------------------------------------------------
//  vic20_blk5_pointer - get block 5 pointer
//-------------------------------------------------

UINT8* device_vic20_expansion_card_interface::vic20_blk5_pointer(running_machine &machine, size_t size)
{
	if (m_blk5 == NULL)
	{
		m_blk5 = auto_alloc_array(machine, UINT8, size);
	}

	return m_blk5;
}


//-------------------------------------------------
//  vic20_ram_pointer - get RAM pointer
//-------------------------------------------------

UINT8* device_vic20_expansion_card_interface::vic20_ram_pointer(running_machine &machine, size_t size)
{
	if (m_ram == NULL)
	{
		m_ram = auto_alloc_array(machine, UINT8, size);
	}

	return m_ram;
}


//-------------------------------------------------
//  vic20_nvram_pointer - get NVRAM pointer
//-------------------------------------------------

UINT8* device_vic20_expansion_card_interface::vic20_nvram_pointer(running_machine &machine, size_t size)
{
	if (m_nvram == NULL)
	{
		m_nvram = auto_alloc_array(machine, UINT8, size);

		m_nvram_mask = size - 1;
		m_nvram_size = size;
	}

	return m_nvram;
}


//-------------------------------------------------
//  ~device_vic20_expansion_card_interface - destructor
//-------------------------------------------------

device_vic20_expansion_card_interface::~device_vic20_expansion_card_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic20_expansion_slot_device - constructor
//-------------------------------------------------

vic20_expansion_slot_device::vic20_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, VIC20_EXPANSION_SLOT, "VIC-20 expansion port", tag, owner, clock),
		device_slot_interface(mconfig, *this),
		device_image_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  vic20_expansion_slot_device - destructor
//-------------------------------------------------

vic20_expansion_slot_device::~vic20_expansion_slot_device()
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void vic20_expansion_slot_device::device_config_complete()
{
	// inherit a copy of the static data
	const vic20_expansion_slot_interface *intf = reinterpret_cast<const vic20_expansion_slot_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<vic20_expansion_slot_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
    	memset(&m_out_irq_cb, 0, sizeof(m_out_irq_cb));
    	memset(&m_out_nmi_cb, 0, sizeof(m_out_nmi_cb));
    	memset(&m_out_res_cb, 0, sizeof(m_out_res_cb));
	}

	// set brief and instance name
	update_names();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic20_expansion_slot_device::device_start()
{
	m_cart = dynamic_cast<device_vic20_expansion_card_interface *>(get_card_device());

	// resolve callbacks
	m_out_irq_func.resolve(m_out_irq_cb, *this);
	m_out_nmi_func.resolve(m_out_nmi_cb, *this);
	m_out_res_func.resolve(m_out_res_cb, *this);

	// inherit bus clock
	if (clock() == 0)
	{
		vic20_expansion_slot_device *root = machine().device<vic20_expansion_slot_device>(VIC20_EXPANSION_SLOT_TAG);
		assert(root);
		set_unscaled_clock(root->clock());
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic20_expansion_slot_device::device_reset()
{
	port_res_w(ASSERT_LINE);
	port_res_w(CLEAR_LINE);
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

bool vic20_expansion_slot_device::call_load()
{
	if (m_cart)
	{
		size_t size = 0;

		if (software_entry() == NULL)
		{
			if (!mame_stricmp(filetype(), "20")) fread(m_cart->vic20_blk1_pointer(machine(), 0x2000), 0x2000);
			else if (!mame_stricmp(filetype(), "40")) fread(m_cart->vic20_blk2_pointer(machine(), 0x2000), 0x2000);
			else if (!mame_stricmp(filetype(), "60")) fread(m_cart->vic20_blk3_pointer(machine(), 0x2000), 0x2000);
			else if (!mame_stricmp(filetype(), "70")) fread(m_cart->vic20_blk3_pointer(machine(), 0x2000) + 0x1000, 0x1000);
			else if (!mame_stricmp(filetype(), "a0")) fread(m_cart->vic20_blk5_pointer(machine(), 0x2000), 0x2000);
			else if (!mame_stricmp(filetype(), "b0")) fread(m_cart->vic20_blk5_pointer(machine(), 0x2000) + 0x1000, 0x1000);
			else if (!mame_stricmp(filetype(), "crt"))
			{
				// read the header
				UINT8 header[2];
				fread(&header, 2);
				UINT16 address = pick_integer_le(header, 0, 2);

				if (LOG) logerror("Address %04x\n", address);

				switch (address)
				{
				case 0x2000: fread(m_cart->vic20_blk1_pointer(machine(), 0x2000), 0x2000); break;
				case 0x4000: fread(m_cart->vic20_blk2_pointer(machine(), 0x2000), 0x2000); break;
				case 0x6000: fread(m_cart->vic20_blk3_pointer(machine(), 0x2000), 0x2000); break;
				case 0x7000: fread(m_cart->vic20_blk3_pointer(machine(), 0x2000) + 0x1000, 0x1000); break;
				case 0xa000: fread(m_cart->vic20_blk5_pointer(machine(), 0x2000), 0x2000); break;
				case 0xb000: fread(m_cart->vic20_blk5_pointer(machine(), 0x2000) + 0x1000, 0x1000); break;
				default: return IMAGE_INIT_FAIL;
				}
			}
		}
		else
		{
			size = get_software_region_length("blk1");
			if (size) memcpy(m_cart->vic20_blk1_pointer(machine(), size), get_software_region("blk1"), size);

			size = get_software_region_length("blk2");
			if (size) memcpy(m_cart->vic20_blk2_pointer(machine(), size), get_software_region("blk2"), size);

			size = get_software_region_length("blk3");
			if (size) memcpy(m_cart->vic20_blk3_pointer(machine(), size), get_software_region("blk3"), size);

			size = get_software_region_length("blk5");
			if (size) memcpy(m_cart->vic20_blk5_pointer(machine(), size), get_software_region("blk5"), size);

			size = get_software_region_length("ram");
			if (size) memcpy(m_cart->vic20_ram_pointer(machine(), size), get_software_region("ram"), size);

			size = get_software_region_length("nvram");
			if (size) memcpy(m_cart->vic20_nvram_pointer(machine(), size), get_software_region("nvram"), size);

		}
	}

	return IMAGE_INIT_PASS;
}


//-------------------------------------------------
//  call_softlist_load -
//-------------------------------------------------

bool vic20_expansion_slot_device::call_softlist_load(char *swlist, char *swname, rom_entry *start_entry)
{
	load_software_part_region(this, swlist, swname, start_entry);

	return true;
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

const char * vic20_expansion_slot_device::get_default_card_software(const machine_config &config, emu_options &options)
{
	return software_get_default_slot(config, options, this, "standard");
}


//-------------------------------------------------
//  cd_r - cartridge data read
//-------------------------------------------------

UINT8 vic20_expansion_slot_device::cd_r(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	if (m_cart != NULL)
	{
		data = m_cart->vic20_cd_r(space, offset, data, ram1, ram2, ram3, blk1, blk2, blk3, blk5, io2, io3);
	}

	return data;
}


//-------------------------------------------------
//  cd_w - cartridge data write
//-------------------------------------------------

void vic20_expansion_slot_device::cd_w(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	if (m_cart != NULL)
	{
		m_cart->vic20_cd_w(space, offset, data, ram1, ram2, ram3, blk1, blk2, blk3, blk5, io2, io3);
	}
}

WRITE_LINE_MEMBER( vic20_expansion_slot_device::port_res_w ) { if (m_cart != NULL) m_cart->vic20_res_w(state); }

WRITE_LINE_MEMBER( vic20_expansion_slot_device::irq_w ) { m_out_irq_func(state); }
WRITE_LINE_MEMBER( vic20_expansion_slot_device::nmi_w ) { m_out_nmi_func(state); }
WRITE_LINE_MEMBER( vic20_expansion_slot_device::res_w ) { m_out_res_func(state); }
