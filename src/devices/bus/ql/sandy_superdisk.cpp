// license:BSD-3-Clause
// copyright-holders:Curt Coder, Phill Harvey-Smith
/**********************************************************************

    Sandy Super Disk emulation

**********************************************************************/

#include "sandy_superdisk.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define WD1772_TAG      "wd1772"
#define TTL74273_TAG    "ttl74273"
#define CENTRONICS_TAG  "centronics"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SANDY_SUPER_DISK = &device_creator<sandy_super_disk_t>;


//-------------------------------------------------
//  ROM( sandy_super_disk )
//-------------------------------------------------

ROM_START( sandy_super_disk )
	ROM_REGION( 0x4000, "rom", 0 )
	ROM_LOAD( "sandysuperdisk.rom", 0x0000, 0x4000, CRC(b52077da) SHA1(bf531758145ffd083e01c1cf9c45d0e9264a3b53) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *sandy_super_disk_t::device_rom_region() const
{
	return ROM_NAME( sandy_super_disk );
}


//-------------------------------------------------
//  SLOT_INTERFACE( sandy_super_disk_floppies )
//-------------------------------------------------

static SLOT_INTERFACE_START( sandy_super_disk_floppies )
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD )
SLOT_INTERFACE_END


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( sandy_super_disk_t::floppy_formats )
	FLOPPY_QL_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  centronics
//-------------------------------------------------

WRITE_LINE_MEMBER( sandy_super_disk_t::busy_w )
{
	m_busy = state;
	check_interrupt();
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( sandy_super_disk )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( sandy_super_disk )
	MCFG_DEVICE_ADD(WD1772_TAG, WD1772, 8000000)
	MCFG_FLOPPY_DRIVE_ADD(WD1772_TAG":0", sandy_super_disk_floppies, "35dd", sandy_super_disk_t::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD1772_TAG":1", sandy_super_disk_floppies, nullptr, sandy_super_disk_t::floppy_formats)

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(sandy_super_disk_t, busy_w))
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD(TTL74273_TAG, CENTRONICS_TAG)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor sandy_super_disk_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sandy_super_disk );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sandy_super_disk_t - constructor
//-------------------------------------------------

sandy_super_disk_t::sandy_super_disk_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SANDY_SUPER_DISK, "Sandy Super Disk", tag, owner, clock, "ql_sdisk", __FILE__),
	device_ql_expansion_card_interface(mconfig, *this),
	m_fdc(*this, WD1772_TAG),
	m_floppy0(*this, WD1772_TAG":0"),
	m_floppy1(*this, WD1772_TAG":1"),
	m_centronics(*this, CENTRONICS_TAG),
	m_latch(*this, TTL74273_TAG),
	m_rom(*this, "rom"),
	m_busy(1), m_fd6(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sandy_super_disk_t::device_start()
{
	// state saving
	save_item(NAME(m_busy));
	save_item(NAME(m_fd6));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sandy_super_disk_t::device_reset()
{
	m_fdc->reset();
	m_fdc->set_floppy(nullptr);
	m_fdc->dden_w(0);

	m_latch->write(0);
	m_centronics->write_strobe(1);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

UINT8 sandy_super_disk_t::read(address_space &space, offs_t offset, UINT8 data)
{
	if ((offset & 0xf0000) == 0xc0000)
	{
		if ((offset & 0xffc0) == 0x3fc0)
		{
			switch ((offset >> 2) & 0x03)
			{
			case 0:
				data = m_fdc->read(space, offset & 0x03);
				break;

			case 3:
				/*

				    bit     description

				    0       BUSY
				    1
				    2
				    3
				    4
				    5
				    6
				    7

				*/

				data = m_busy;
				break;
			}
		}
		else if (offset < 0xc4000)
		{
			data = m_rom->base()[offset & 0x3fff];
		}
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void sandy_super_disk_t::write(address_space &space, offs_t offset, UINT8 data)
{
	if ((offset & 0xf0000) == 0xc0000)
	{
		if ((offset & 0xffc0) == 0x3fc0)
		{
			switch ((offset >> 2) & 0x03)
			{
			case 0:
				m_fdc->write(space, offset & 0x03, data);
				break;

			case 1:
				{
				/*

				    bit     description

				    0       SIDE ONE
				    1       DSEL0
				    2       DSEL1
				    3       M ON0
				    4       /DDEN
				    5       STROBE inverted
				    6       enable printer interrupt
				    7

				*/

				floppy_image_device *floppy = nullptr;

				if (BIT(data, 1))
				{
					floppy = m_floppy0->get_device();
				}
				else if (BIT(data, 2))
				{
					floppy = m_floppy1->get_device();
				}

				m_fdc->set_floppy(floppy);

				if (floppy)
				{
					floppy->ss_w(BIT(data, 0));
					floppy->mon_w(!BIT(data, 3));
				}

				m_fdc->dden_w(BIT(data, 4));

				m_centronics->write_strobe(!BIT(data, 5));

				m_fd6 = BIT(data, 6);
				check_interrupt();
				}
				break;

			case 2:
				m_latch->write(data);
				break;
			}
		}
	}
}

void sandy_super_disk_t::check_interrupt()
{
	int extint = m_fd6 && m_busy;

	m_slot->extintl_w(extint ? ASSERT_LINE : CLEAR_LINE);
}
