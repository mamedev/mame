// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie Floppy Disc Controller

    TODO:
    - What's the exact FD1793 model?
    - How does it turn off the motor?
    - How does it switch between FM/MFM?

***************************************************************************/

#include "floppy.h"
#include "formats/cgenie_dsk.h"
#include "bus/generic/carts.h"


//**************************************************************************
//  CONSTANTS/MACROS
//**************************************************************************

#define VERBOSE 0

// set to 1 to test fm disk formats
#define FM_MODE 0


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CGENIE_FDC = &device_creator<cgenie_fdc_device>;

FLOPPY_FORMATS_MEMBER( cgenie_fdc_device::floppy_formats )
	FLOPPY_CGENIE_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( cgenie_floppies )
	SLOT_INTERFACE("sssd", FLOPPY_525_SSSD)
	SLOT_INTERFACE("sd",   FLOPPY_525_SD)
	SLOT_INTERFACE("ssdd", FLOPPY_525_SSDD)
	SLOT_INTERFACE("dd",   FLOPPY_525_DD)
	SLOT_INTERFACE("ssqd", FLOPPY_525_SSQD)
	SLOT_INTERFACE("qd",   FLOPPY_525_QD)
SLOT_INTERFACE_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( cgenie_fdc )
	ROM_REGION(0x3000, "software", 0)
	ROM_LOAD("cgdos.rom", 0x0000, 0x2000, CRC(2a96cf74) SHA1(6dcac110f87897e1ee7521aefbb3d77a14815893))
ROM_END

const rom_entry *cgenie_fdc_device::device_rom_region() const
{
	return ROM_NAME( cgenie_fdc );
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( cgenie_fdc )
	MCFG_FD1793x_ADD("fd1793", XTAL_16MHz / 4 / 4)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(cgenie_fdc_device, intrq_w))

	MCFG_FLOPPY_DRIVE_ADD("fd1793:0", cgenie_floppies, "ssdd", cgenie_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fd1793:1", cgenie_floppies, "ssdd", cgenie_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fd1793:2", cgenie_floppies, NULL,   cgenie_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fd1793:3", cgenie_floppies, NULL,   cgenie_fdc_device::floppy_formats)

//	MCFG_SOFTWARE_LIST_ADD("floppy_list", "cgenie_flop")

	MCFG_GENERIC_SOCKET_ADD("socket", generic_plain_slot, "cgenie_socket")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_LOAD(cgenie_fdc_device, socket_load)

	MCFG_SOFTWARE_LIST_ADD("cart_list", "cgenie_cart")
MACHINE_CONFIG_END

machine_config_constructor cgenie_fdc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cgenie_fdc );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cgenie_fdc_device - constructor
//-------------------------------------------------

cgenie_fdc_device::cgenie_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CGENIE_FDC, "Colour Genie Floppy Disc Controller", tag, owner, clock, "cgenie_fdc", __FILE__),
	device_expansion_interface(mconfig, *this),
	m_fdc(*this, "fd1793"),
	m_floppy0(*this, "fd1793:0"),
	m_floppy1(*this, "fd1793:1"),
	m_floppy2(*this, "fd1793:2"),
	m_floppy3(*this, "fd1793:3"),
	m_socket(*this, "socket"),
	m_floppy(NULL)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cgenie_fdc_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cgenie_fdc_device::device_reset()
{
	// dos rom
	m_slot->m_program->install_rom(0xc000, 0xdfff, memregion("software")->base());

	// memory mapped i/o
	m_slot->m_program->install_write_handler(0xffe0, 0xffe3, 0, 0x10, write8_delegate(FUNC(cgenie_fdc_device::select_w), this));
	m_slot->m_program->install_read_handler( 0xffec, 0xffef, 0, 0x10,  read8_delegate(FUNC(fd1793_t::read),  m_fdc.target()));
	m_slot->m_program->install_write_handler(0xffec, 0xffef, 0, 0x10, write8_delegate(FUNC(fd1793_t::write), m_fdc.target()));

	// map extra socket
	if (m_socket->exists())
	{
		m_slot->m_program->install_read_handler(0xe000, 0xefff, read8_delegate(FUNC(generic_slot_device::read_rom), (generic_slot_device *) m_socket));
	}
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

DEVICE_IMAGE_LOAD_MEMBER( cgenie_fdc_device, socket_load )
{
	UINT32 size = m_socket->common_get_size("rom");

	if (size > 0x1000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported ROM size");
		return IMAGE_INIT_FAIL;
	}

	m_socket->rom_alloc(0x1000, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_socket->common_load_rom(m_socket->get_rom_base(), size, "rom");

	return IMAGE_INIT_PASS;
}

WRITE_LINE_MEMBER( cgenie_fdc_device::intrq_w )
{
	if (VERBOSE)
		logerror("cgenie_fdc_device::intrq_w: %d\n", state);

	// forward to host
	m_slot->int_w(state);
}

WRITE8_MEMBER( cgenie_fdc_device::select_w )
{
	if (VERBOSE)
		logerror("cgenie_fdc_device::motor_w: 0x%02x\n", data);

	if (FM_MODE)
		m_fdc->dden_w(1);

	m_floppy = NULL;

	if (BIT(data, 0)) m_floppy = m_floppy0->get_device();
	if (BIT(data, 1)) m_floppy = m_floppy1->get_device();
	if (BIT(data, 2)) m_floppy = m_floppy2->get_device();
	if (BIT(data, 3)) m_floppy = m_floppy3->get_device();

	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
	{
		m_floppy->ss_w(BIT(data, 4));
		m_floppy->mon_w(0);
	}
}
