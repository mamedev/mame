// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sandy SuperQBoard (with HD upgrade) emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "sandy_superqboard.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define WD1772_TAG		"ic3"
#define TTL74273_TAG	"ic10"
#define CENTRONICS_TAG	"j2"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SANDY_SUPERQBOARD = &device_creator<sandy_superqboard_t>;


//-------------------------------------------------
//  ROM( sandy_superqboard )
//-------------------------------------------------

ROM_START( sandy_superqboard )
	ROM_REGION( 0x8000, "rom", 0 )
	ROM_DEFAULT_BIOS("v121n")
	ROM_SYSTEM_BIOS( 0, "v118y", "v1.18" )
	ROMX_LOAD( "sandy_disk_controller_v1.18y_1984.ic2", 0x0000, 0x8000, CRC(d02425be) SHA1(e730576e3e0c6a1acad042c09e15fc62a32d8fbd), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v119", "v1.19N" )
	ROMX_LOAD( "sandysuperqboard_119n.ic2", 0x0000, 0x8000, CRC(5df04059) SHA1(51403f82a2eed3ef689e880936d1613e2b29c218), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "v121n", "v1.21N" )
	ROMX_LOAD( "sandy_disk_controller_v1.21n_1984_tk2.ic2", 0x0000, 0x8000, CRC(6a7a6a12) SHA1(a3a233e4f6c8450055fa537601a2a2eef143edca), ROM_BIOS(3) )

	ROM_REGION( 0x100, "plds", 0 )
	ROM_LOAD( "gal16v8.ic5", 0x000, 0x100, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *sandy_superqboard_t::device_rom_region() const
{
	return ROM_NAME( sandy_superqboard );
}


//-------------------------------------------------
//  SLOT_INTERFACE( sandy_superqboard_floppies )
//-------------------------------------------------

static SLOT_INTERFACE_START( sandy_superqboard_floppies )
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD )
	SLOT_INTERFACE( "35hd", FLOPPY_35_HD )
SLOT_INTERFACE_END


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( sandy_superqboard_t::floppy_formats )
	FLOPPY_QL_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  centronics
//-------------------------------------------------

WRITE_LINE_MEMBER( sandy_superqboard_t::busy_w )
{
	m_busy = state;
	check_interrupt();
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( sandy_superqboard )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( sandy_superqboard )
	MCFG_DEVICE_ADD(WD1772_TAG, WD1772x, XTAL_16MHz/2)
	MCFG_FLOPPY_DRIVE_ADD(WD1772_TAG":0", sandy_superqboard_floppies, "35hd", sandy_superqboard_t::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD1772_TAG":1", sandy_superqboard_floppies, NULL, sandy_superqboard_t::floppy_formats)

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_printers, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(sandy_superqboard_t, busy_w))
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD(TTL74273_TAG, CENTRONICS_TAG)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor sandy_superqboard_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sandy_superqboard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sandy_superqboard_t - constructor
//-------------------------------------------------

sandy_superqboard_t::sandy_superqboard_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SANDY_SUPERQBOARD, "Sandy SuperQBoard", tag, owner, clock, "sandy_superqboard", __FILE__),
	device_ql_expansion_card_interface(mconfig, *this),
	m_fdc(*this, WD1772_TAG),
	m_floppy0(*this, WD1772_TAG":0"),
	m_floppy1(*this, WD1772_TAG":1"),
	m_centronics(*this, CENTRONICS_TAG),
	m_latch(*this, TTL74273_TAG),
	m_rom(*this, "rom"),
	m_ram(*this, "ram"),
	m_busy(1),
	m_int2(0),
	m_int3(0),
	m_fd6(0),
	m_fd7(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sandy_superqboard_t::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sandy_superqboard_t::device_reset()
{
	m_fdc->reset();
	m_fdc->set_floppy(NULL);
	m_fdc->dden_w(0);

	m_latch->write(0);
	m_centronics->write_strobe(1);
	
	m_int2 = 0;
	m_int3 = 0;
	m_fd6 = 0;
	m_fd7 = 0;
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

UINT8 sandy_superqboard_t::read(address_space &space, offs_t offset, UINT8 data)
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

					bit		description

					0 		BUSY
					1 		mouse pin 8
					2 		mouse pin 1
					3 		mouse pin 2
					4 		mouse pin 4 flip-flop Q
					5 		mouse pin 3 flip-flop Q
					6 		INT3
					7 		INT2

				*/

				data = m_busy;
				data |= m_int3 << 6;
				data |= m_int2 << 7;
				break;
			}
		}
		else if (offset < 0xc8000)
		{
			data = m_rom->base()[offset & 0x7fff];
		}
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void sandy_superqboard_t::write(address_space &space, offs_t offset, UINT8 data)
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

					bit		description

					0 		SIDE ONE
					1 		DSEL0
					2 		DSEL1
					3 		M ON0
					4 		/DDEN
					5 		STROBE inverted
					6 		enable printer interrupt (GAL pin 11)
					7 		enable mouse interrupt (GAL pin 9)

				*/

				floppy_image_device *floppy = NULL;

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
					floppy->mon_w(BIT(data, 3));
				}

				m_fdc->dden_w(BIT(data, 4));

				m_centronics->write_strobe(!BIT(data, 5));

				m_fd6 = BIT(data, 6);
				m_fd7 = BIT(data, 7);
				check_interrupt();
				}
				break;

			case 2:
				m_latch->write(data);
				break;

			case 4:
				m_int2 = 0;
				m_int3 = 0;
				check_interrupt();
				break;

			case 5:
				m_fdc->set_unscaled_clock(XTAL_16MHz >> !BIT(data, 0));
				break;
			}
		}
	}
}

void sandy_superqboard_t::check_interrupt()
{
	int extint = (m_fd6 && m_busy) || (m_fd7 && (m_int2 || m_int3));

	m_slot->extintl_w(extint ? ASSERT_LINE : CLEAR_LINE);
}
