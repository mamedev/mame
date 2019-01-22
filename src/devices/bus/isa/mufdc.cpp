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

#include "emu.h"
#include "mufdc.h"

#include "formats/naslite_dsk.h"
#include "formats/pc_dsk.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_FDC344, fdc344_device, "fdc344", "Ably-Tech FDC-344")
DEFINE_DEVICE_TYPE(ISA8_FDCMAG, fdcmag_device, "fdcmag", "Magitronic Multi Floppy Controller Card")

FLOPPY_FORMATS_MEMBER( mufdc_device::floppy_formats )
	FLOPPY_PC_FORMAT,
	FLOPPY_NASLITE_FORMAT
FLOPPY_FORMATS_END

static void drives(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("35hd", FLOPPY_35_HD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mufdc_device::device_add_mconfig(machine_config &config)
{
	mcs3201_device &mcs3201(MCS3201(config, m_fdc, 24_MHz_XTAL));
	mcs3201.input_handler().set(FUNC(mufdc_device::fdc_input_r));
	mcs3201.intrq_wr_callback().set(FUNC(mufdc_device::fdc_irq_w));
	mcs3201.drq_wr_callback().set(FUNC(mufdc_device::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", drives, "35hd", mufdc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", drives, "35hd", mufdc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:2", drives, nullptr, mufdc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:3", drives, nullptr, mufdc_device::floppy_formats);
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

const tiny_rom_entry *fdc344_device::device_rom_region() const
{
	return ROM_NAME( fdc344 );
}

ROM_START( fdcmag )
	ROM_REGION(0x2000, "option", 0)
	ROM_LOAD("magitronic_40.u2", 0x0000, 0x2000, CRC(41a5371b) SHA1(9c4443169a0b104395404274470e62b8b65efcf4))
ROM_END

const tiny_rom_entry *fdcmag_device::device_rom_region() const
{
	return ROM_NAME( fdcmag );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mufdc_device - constructor
//-------------------------------------------------

mufdc_device::mufdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_isa8_card_interface( mconfig, *this),
	m_fdc(*this, "fdc"),
	m_config(*this, "configuration")
{
}

fdc344_device::fdc344_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mufdc_device(mconfig, ISA8_FDC344, tag, owner, clock)
{
}

fdcmag_device::fdcmag_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mufdc_device(mconfig, ISA8_FDCMAG, tag, owner, clock)
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
	m_isa->install_rom(this, 0xc8000, 0xc9fff, shortname(), "option");
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

uint8_t mufdc_device::dack_r(int line)
{
	return m_fdc->dma_r();
}

void mufdc_device::dack_w(int line, uint8_t data)
{
	return m_fdc->dma_w(data);
}

void mufdc_device::dack_line_w(int line, int state)
{
	//m_fdc->dack_w(state);
}

void mufdc_device::eop_w(int state)
{
	m_fdc->tc_w(state == ASSERT_LINE);
}
