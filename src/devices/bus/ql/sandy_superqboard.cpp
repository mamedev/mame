// license:BSD-3-Clause
// copyright-holders:Curt Coder, Phill Harvey-Smith
/**********************************************************************

    Sandy SuperQBoard/SuperQMouse (with HD upgrade) emulation

**********************************************************************/

#include "sandy_superqboard.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define WD1772_TAG      "ic3"
#define TTL74273_TAG    "ic10"
#define CENTRONICS_TAG  "j2"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SANDY_SUPERQBOARD = &device_creator<sandy_superqboard_t>;
const device_type SANDY_SUPERQBOARD_512K = &device_creator<sandy_superqboard_512k_t>;
const device_type SANDY_SUPERQMOUSE = &device_creator<sandy_superqmouse_t>;
const device_type SANDY_SUPERQMOUSE_512K = &device_creator<sandy_superqmouse_512k_t>;


//-------------------------------------------------
//  ROM( sandy_superqboard )
//-------------------------------------------------

ROM_START( sandy_superqboard )
	ROM_REGION( 0x8000, "rom", 0 )
	ROM_DEFAULT_BIOS("v118y")
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
	if (state)
	{
		m_status |= ST_BUSY;
	}
	else
	{
		m_status &= ~ST_BUSY;
	}

	check_interrupt();
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( sandy_superqboard )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( sandy_superqboard )
	MCFG_DEVICE_ADD(WD1772_TAG, WD1772, XTAL_16MHz/2)
	MCFG_FLOPPY_DRIVE_ADD(WD1772_TAG":0", sandy_superqboard_floppies, "35hd", sandy_superqboard_t::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD1772_TAG":1", sandy_superqboard_floppies, nullptr, sandy_superqboard_t::floppy_formats)

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
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


//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( mouse_x_changed )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( sandy_superqboard_t::mouse_x_changed )
{
	if (newval < oldval)
	{
		m_status |= ST_X_DIR;
	}
	else
	{
		m_status &= ~ST_X_DIR;
	}

	m_status |= ST_X_INT;

	check_interrupt();
}


//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( mouse_y_changed )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( sandy_superqboard_t::mouse_y_changed )
{
	if (newval < oldval)
	{
		m_status |= ST_Y_DIR;
	}
	else
	{
		m_status &= ~ST_Y_DIR;
	}

	m_status |= ST_Y_INT;

	check_interrupt();
}


//-------------------------------------------------
//  INPUT_PORTS( sandy_superqmouse )
//-------------------------------------------------

INPUT_PORTS_START( sandy_superqmouse )
	PORT_START("mouse_x")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_CHANGED_MEMBER(DEVICE_SELF, sandy_superqboard_t, mouse_x_changed, nullptr)

	PORT_START("mouse_y")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_CHANGED_MEMBER(DEVICE_SELF, sandy_superqboard_t, mouse_y_changed, nullptr)

	PORT_START("mouse_buttons")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Middle Mouse Button") PORT_CODE(MOUSECODE_BUTTON3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Right Mouse Button") PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor sandy_superqmouse_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( sandy_superqmouse );
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor sandy_superqmouse_512k_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( sandy_superqmouse );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sandy_superqboard_t - constructor
//-------------------------------------------------

sandy_superqboard_t::sandy_superqboard_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SANDY_SUPERQBOARD, "Sandy SuperQBoard 256K", tag, owner, clock, "ql_sqboard256", __FILE__),
	device_ql_expansion_card_interface(mconfig, *this),
	m_fdc(*this, WD1772_TAG),
	m_floppy0(*this, WD1772_TAG":0"),
	m_floppy1(*this, WD1772_TAG":1"),
	m_centronics(*this, CENTRONICS_TAG),
	m_latch(*this, TTL74273_TAG),
	m_rom(*this, "rom"),
	m_ram(*this, "ram"),
	m_buttons(*this, "mouse_buttons"),
	m_ram_size(256*1024),
	m_fd6(0),
	m_fd7(0),
	m_status(0)
{
}

sandy_superqboard_t::sandy_superqboard_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int ram_size) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, __FILE__),
	device_ql_expansion_card_interface(mconfig, *this),
	m_fdc(*this, WD1772_TAG),
	m_floppy0(*this, WD1772_TAG":0"),
	m_floppy1(*this, WD1772_TAG":1"),
	m_centronics(*this, CENTRONICS_TAG),
	m_latch(*this, TTL74273_TAG),
	m_rom(*this, "rom"),
	m_ram(*this, "ram"),
	m_buttons(*this, "mouse_buttons"),
	m_ram_size(ram_size),
	m_fd6(0),
	m_fd7(0),
	m_status(0)
{
}

sandy_superqboard_512k_t::sandy_superqboard_512k_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sandy_superqboard_t(mconfig, SANDY_SUPERQBOARD_512K, "Sandy SuperQBoard 512K", tag, owner, clock, "ql_sqboard512", __FILE__, 512*1024) { }

sandy_superqmouse_t::sandy_superqmouse_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sandy_superqboard_t(mconfig, SANDY_SUPERQMOUSE, "Sandy SuperQMouse", tag, owner, clock, "ql_sqmouse", __FILE__, 256*1024) { }

sandy_superqmouse_512k_t::sandy_superqmouse_512k_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sandy_superqboard_t(mconfig, SANDY_SUPERQMOUSE_512K, "Sandy SuperQMouse 512K", tag, owner, clock, "ql_sqmouse512", __FILE__, 512*1024) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sandy_superqboard_t::device_start()
{
	// allocate memory
	m_ram.allocate(m_ram_size);

	// state saving
	save_item(NAME(m_fd6));
	save_item(NAME(m_fd7));
	save_item(NAME(m_status));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sandy_superqboard_t::device_reset()
{
	m_fdc->reset();
	m_fdc->set_floppy(nullptr);
	m_fdc->dden_w(0);

	m_latch->write(0);
	m_centronics->write_strobe(1);

	m_fd6 = 0;
	m_fd7 = 0;
	m_status = 0;

	check_interrupt();
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
			switch ((offset >> 2) & 0x07)
			{
			case 0:
				data = m_fdc->read(space, offset & 0x03);
				break;

			case 3:
				/*

				    bit     description

				    0       BUSY
				    1       mouse pin 8 (middle button)
				    2       mouse pin 1 (right button)
				    3       mouse pin 2 (left button)
				    4       mouse pin 4 flip-flop Q (Y direction)
				    5       mouse pin 3 flip-flop Q (X direction)
				    6       INT3 (Y interrupt)
				    7       INT2 (X interrupt)

				*/

				data = m_buttons->read() & 0x0e;
				data |= m_status & 0xf1;
				break;

			case 4:
				m_status &= ~(ST_Y_INT | ST_X_INT);
				check_interrupt();
				break;
			}
		}
		else if (offset < 0xc8000)
		{
			data = m_rom->base()[offset & 0x7fff];
		}
	}

	if (offset >= 0x40000 && offset < 0xc0000)
	{
		if ((offset - 0x40000) < m_ram_size)
		{
			data = m_ram[offset - 0x40000];
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
			switch ((offset >> 2) & 0x07)
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
				    6       enable printer interrupt (GAL pin 11)
				    7       enable mouse interrupt (GAL pin 9)

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
				m_fd7 = BIT(data, 7);

				check_interrupt();
				}
				break;

			case 2:
				m_latch->write(data);
				break;

			case 4:
				m_status &= ~(ST_Y_INT | ST_X_INT);
				check_interrupt();
				break;

			case 5:
				m_fdc->set_unscaled_clock(XTAL_16MHz / (BIT(data, 0) ? 1 : 2));
				break;
			}
		}
	}

	if (offset >= 0x40000 && offset < 0xc0000)
	{
		if ((offset - 0x40000) < m_ram_size)
		{
			m_ram[offset - 0x40000] = data;
		}
	}
}

void sandy_superqboard_t::check_interrupt()
{
	bool busy_int = m_fd6 && (m_status & ST_BUSY);
	bool mouse_int = m_fd7 && (m_status & (ST_Y_INT | ST_X_INT));
	bool extint = busy_int || mouse_int;

	m_slot->extintl_w(extint ? ASSERT_LINE : CLEAR_LINE);
}
