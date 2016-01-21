// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    Multi Unique FDC

    8-bit floppy controller, supports 4 drives with 360k, 720k,
    1.2MB or 1.44MB. It was sold under a few different names:

    - Ably-Tech FDC-344
    - Magitronic Multi Floppy Controller Card
    - Micro-Q (same as FDC-344?)
    - Modular Circuit Technology MCT-FDC-HD4 (not dumped)

***************************************************************************/

#include "mufdc.h"
#include "formats/naslite_dsk.h"
#include "formats/pc_dsk.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ISA8_FDC344 = &device_creator<fdc344_device>;
const device_type ISA8_FDCMAG = &device_creator<fdcmag_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( mufdc_device::floppy_formats )
	FLOPPY_PC_FORMAT,
	FLOPPY_NASLITE_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( drives )
	SLOT_INTERFACE("525hd", FLOPPY_525_HD)
	SLOT_INTERFACE("35hd", FLOPPY_35_HD)
	SLOT_INTERFACE("525dd", FLOPPY_525_DD)
	SLOT_INTERFACE("35dd", FLOPPY_35_DD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( mufdc_device )
	MCFG_MCS3201_ADD("fdc")
	MCFG_MCS3201_INPUT_HANDLER(READ8(mufdc_device, fdc_input_r))
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(mufdc_device, fdc_irq_w))
	MCFG_UPD765_DRQ_CALLBACK(WRITELINE(mufdc_device, fdc_drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", drives, "35hd", mufdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", drives, "35hd", mufdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:2", drives, nullptr, mufdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:3", drives, nullptr, mufdc_device::floppy_formats)
MACHINE_CONFIG_END

machine_config_constructor mufdc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( mufdc_device );
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( mufdc_device )
	PORT_START("configuration")
	PORT_DIPNAME(0x03, 0x02, "FDC FDD 1")
	PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(0x01, "360KB")
	PORT_DIPSETTING(0x03, "720KB")
	PORT_DIPSETTING(0x00, "1.2MB")
	PORT_DIPSETTING(0x02, "1.44MB")
	PORT_DIPNAME(0x0c, 0x08, "FDC FDD 2")
	PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(0x04, "360KB")
	PORT_DIPSETTING(0x0c, "720KB")
	PORT_DIPSETTING(0x00, "1.2MB")
	PORT_DIPSETTING(0x08, "1.44MB")
	PORT_DIPNAME(0x30, 0x20, "FDC FDD 3")
	PORT_DIPLOCATION("SW:5,6")
	PORT_DIPSETTING(0x10, "360KB")
	PORT_DIPSETTING(0x30, "720KB")
	PORT_DIPSETTING(0x00, "1.2MB")
	PORT_DIPSETTING(0x20, "1.44MB")
	PORT_DIPNAME(0xc0, 0x80, "FDC FDD 4")
	PORT_DIPLOCATION("SW:7,8")
	PORT_DIPSETTING(0x40, "360KB")
	PORT_DIPSETTING(0xc0, "720KB")
	PORT_DIPSETTING(0x00, "1.2MB")
	PORT_DIPSETTING(0x80, "1.44MB")
INPUT_PORTS_END

ioport_constructor mufdc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( mufdc_device );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( fdc344 )
	ROM_REGION(0x4000, "option", 0)
	ROM_LOAD("fdc344_42.u2", 0x0000, 0x4000, CRC(3e02567c) SHA1(b639d92435ecf2a6d4aefd3576a6955028f6bde7))
ROM_END

const rom_entry *fdc344_device::device_rom_region() const
{
	return ROM_NAME( fdc344 );
}

ROM_START( fdcmag )
	ROM_REGION(0x2000, "option", 0)
	ROM_LOAD("magitronic_40.u2", 0x0000, 0x2000, CRC(41a5371b) SHA1(9c4443169a0b104395404274470e62b8b65efcf4))
ROM_END

const rom_entry *fdcmag_device::device_rom_region() const
{
	return ROM_NAME( fdcmag );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mufdc_device - constructor
//-------------------------------------------------

mufdc_device::mufdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock, const char *name, const char *shortname) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, __FILE__),
	device_isa8_card_interface( mconfig, *this ),
	m_fdc(*this, "fdc"),
	m_config(*this, "configuration")
{
}

fdc344_device::fdc344_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	mufdc_device(mconfig, ISA8_FDC344, tag, owner, clock, "Ably-Tech FDC-344", "fdc344")
{
}

fdcmag_device::fdcmag_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	mufdc_device(mconfig, ISA8_FDCMAG, tag, owner, clock, "Magitronic Multi Floppy Controller Card", "fdcmag")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mufdc_device::device_start()
{
	set_isa_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mufdc_device::device_reset()
{
	m_isa->install_rom(this, 0xc8000, 0xc9fff, 0, 0, m_shortname.c_str(), "option");
	m_isa->install_device(0x3f0, 0x3f7, *m_fdc, &pc_fdc_interface::map);
	m_isa->set_dma_channel(2, this, true);
}


//**************************************************************************
//  FDC INTERFACE
//**************************************************************************

READ8_MEMBER( mufdc_device::fdc_input_r )
{
	return ~m_config->read();
}

WRITE_LINE_MEMBER( mufdc_device::fdc_irq_w )
{
	m_isa->irq6_w(state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER( mufdc_device::fdc_drq_w )
{
	m_isa->drq2_w(state ? ASSERT_LINE : CLEAR_LINE);
}

UINT8 mufdc_device::dack_r(int line)
{
	return m_fdc->dma_r();
}

void mufdc_device::dack_w(int line, UINT8 data)
{
	return m_fdc->dma_w(data);
}

void mufdc_device::eop_w(int state)
{
	m_fdc->tc_w(state == ASSERT_LINE);
}
