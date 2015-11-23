// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-10 Expansion Port emulation

**********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "exp.h"



//**************************************************************************
//  DEVICE DEFINITIONS
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
		m_lorom(*this, "lorom"),
		m_exram(*this, "exram"),
		m_uprom(*this, "uprom")
{
	m_slot = dynamic_cast<vic10_expansion_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_vic10_expansion_card_interface - destructor
//-------------------------------------------------

device_vic10_expansion_card_interface::~device_vic10_expansion_card_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic10_expansion_slot_device - constructor
//-------------------------------------------------

vic10_expansion_slot_device::vic10_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, VIC10_EXPANSION_SLOT, "VIC-10 expansion port", tag, owner, clock, "vic10_expansion_slot", __FILE__),
		device_slot_interface(mconfig, *this),
		device_image_interface(mconfig, *this),
		m_write_irq(*this),
		m_write_res(*this),
		m_write_cnt(*this),
		m_write_sp(*this), m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic10_expansion_slot_device::device_start()
{
	m_card = dynamic_cast<device_vic10_expansion_card_interface *>(get_card_device());

	// resolve callbacks
	m_write_irq.resolve_safe();
	m_write_res.resolve_safe();
	m_write_cnt.resolve_safe();
	m_write_sp.resolve_safe();

	// inherit bus clock
	if (clock() == 0)
	{
		vic10_expansion_slot_device *root = machine().device<vic10_expansion_slot_device>(VIC10_EXPANSION_SLOT_TAG);
		assert(root);
		set_unscaled_clock(root->clock());
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic10_expansion_slot_device::device_reset()
{
	if (get_card_device())
	{
		get_card_device()->reset();
	}
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

bool vic10_expansion_slot_device::call_load()
{
	if (m_card)
	{
		size_t size = 0;

		if (software_entry() == NULL)
		{
			size = length();

			if (!core_stricmp(filetype(), "80"))
			{
				fread(m_card->m_lorom, 0x2000);

				if (size == 0x4000)
				{
					fread(m_card->m_uprom, 0x2000);
				}
			}
			else if (!core_stricmp(filetype(), "e0"))
			{
				fread(m_card->m_uprom, size);
			}
			else if (!core_stricmp(filetype(), "crt"))
			{
				size_t roml_size = 0;
				size_t romh_size = 0;
				int exrom = 1;
				int game = 1;

				if (cbm_crt_read_header(m_file, &roml_size, &romh_size, &exrom, &game))
				{
					UINT8 *roml = NULL;
					UINT8 *romh = NULL;

					m_card->m_lorom.allocate(roml_size);
					m_card->m_uprom.allocate(romh_size);

					if (roml_size) roml = m_card->m_lorom;
					if (romh_size) romh = m_card->m_lorom;

					cbm_crt_read_data(m_file, roml, romh);
				}
			}
		}
		else
		{
			load_software_region("lorom", m_card->m_lorom);
			load_software_region("exram", m_card->m_exram);
			load_software_region("uprom", m_card->m_uprom);
		}
	}

	return IMAGE_INIT_PASS;
}


//-------------------------------------------------
//  call_softlist_load -
//-------------------------------------------------

bool vic10_expansion_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry);

	return true;
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

void vic10_expansion_slot_device::get_default_card_software(std::string &result)
{
	if (open_image_file(mconfig().options()))
	{
		if (!core_stricmp(filetype(), "crt"))
		{
			cbm_crt_get_card(result, m_file);
			return;
		}

		clear();
	}

	software_get_default_slot(result, "standard");
}


//-------------------------------------------------
//  cd_r - cartridge data read
//-------------------------------------------------

UINT8 vic10_expansion_slot_device::cd_r(address_space &space, offs_t offset, UINT8 data, int lorom, int uprom, int exram)
{
	if (m_card != NULL)
	{
		data = m_card->vic10_cd_r(space, offset, data, lorom, uprom, exram);
	}

	return data;
}


//-------------------------------------------------
//  cd_w - cartridge data write
//-------------------------------------------------

void vic10_expansion_slot_device::cd_w(address_space &space, offs_t offset, UINT8 data, int lorom, int uprom, int exram)
{
	if (m_card != NULL)
	{
		m_card->vic10_cd_w(space, offset, data, lorom, uprom, exram);
	}
}

READ_LINE_MEMBER( vic10_expansion_slot_device::p0_r ) { int state = 0; if (m_card != NULL) state = m_card->vic10_p0_r(); return state; }
WRITE_LINE_MEMBER( vic10_expansion_slot_device::p0_w ) { if (m_card != NULL) m_card->vic10_p0_w(state); }


//-------------------------------------------------
//  SLOT_INTERFACE( vic10_expansion_cards )
//-------------------------------------------------

// slot devices
#include "std.h"

SLOT_INTERFACE_START( vic10_expansion_cards )
	// the following need ROMs from the software list
	SLOT_INTERFACE_INTERNAL("standard", VIC10_STD)
SLOT_INTERFACE_END
