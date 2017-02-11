// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Opus Floppy Disc Controllers

    EDOS: http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Opus_DiscController.html
    2791: http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Opus_DDoss.html
    2793:
    1770: http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Opus_D-DOS.html

**********************************************************************/


#include "emu.h"
#include "opus.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type BBC_OPUS2791 = &device_creator<bbc_opus2791_device>;
const device_type BBC_OPUS2793 = &device_creator<bbc_opus2793_device>;
const device_type BBC_OPUS1770 = &device_creator<bbc_opus1770_device>;


//-------------------------------------------------
//  MACHINE_DRIVER( opus2791 )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( floppy_formats )
	FLOPPY_ACORN_SSD_FORMAT,
	FLOPPY_ACORN_DSD_FORMAT,
	FLOPPY_FSD_FORMAT,
	FLOPPY_OPUS_DDOS_FORMAT
FLOPPY_FORMATS_END0

static SLOT_INTERFACE_START( bbc_floppies_525 )
	SLOT_INTERFACE("525sssd", FLOPPY_525_SSSD)
	SLOT_INTERFACE("525sd",   FLOPPY_525_SD)
	SLOT_INTERFACE("525ssdd", FLOPPY_525_SSDD)
	SLOT_INTERFACE("525dd",   FLOPPY_525_DD)
	SLOT_INTERFACE("525qd",   FLOPPY_525_QD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( opus2791 )
	MCFG_WD2791_ADD("fdc", XTAL_16MHz / 16)
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(bbc_opusfdc_device, fdc_drq_w))
	MCFG_WD_FDC_HLD_CALLBACK(WRITELINE(bbc_opusfdc_device, motor_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", bbc_floppies_525, "525qd", floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", bbc_floppies_525, "525qd", floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( opus2793 )
	MCFG_WD2793_ADD("fdc", XTAL_16MHz / 16)
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(bbc_opusfdc_device, fdc_drq_w))
	MCFG_WD_FDC_HLD_CALLBACK(WRITELINE(bbc_opusfdc_device, motor_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", bbc_floppies_525, "525qd", floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", bbc_floppies_525, "525qd", floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( opus1770 )
	MCFG_WD1770_ADD("fdc", XTAL_16MHz / 2)
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(bbc_opusfdc_device, fdc_drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", bbc_floppies_525, "525qd", floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", bbc_floppies_525, "525qd", floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

ROM_START( opus2791 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_DEFAULT_BIOS("ddos315")
	ROM_SYSTEM_BIOS(0, "ddos315", "Opus DDOS 3.15")
	ROMX_LOAD("opus-ddos315.rom", 0x0000, 0x4000, CRC(5f06701c) SHA1(9e250dc7ddcde35b19e8f29f2cfe95a79f46d473), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "edos04", "Opus EDOS 0.4")
	ROMX_LOAD("opus-edos04.rom", 0x0000, 0x4000, CRC(1d8a3860) SHA1(05f461464707b4ca24636c9e726af561f227ccdb), ROM_BIOS(2))
ROM_END

ROM_START( opus2793 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_DEFAULT_BIOS("ddos335")
	ROM_SYSTEM_BIOS(0, "ddos335", "Opus DDOS 3.35")
	ROMX_LOAD("opus-ddos335.rom", 0x0000, 0x4000, CRC(e33167fb) SHA1(42fbc9932db2087708da41cb1ffa94358683cf7a), ROM_BIOS(1))
ROM_END

ROM_START( opus1770 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_DEFAULT_BIOS("ddos346")
	ROM_SYSTEM_BIOS(0, "ddos345", "Opus DDOS 3.45")
	ROMX_LOAD("opus-ddos345.rom", 0x0000, 0x4000, CRC(c0163b95) SHA1(1c5a68e08abbb7ffe663151c59088f750d2287a9), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "ddos346", "Opus DDOS 3.46")
	ROMX_LOAD("opus-ddos346.rom", 0x0000, 0x4000, CRC(bf9c35cf) SHA1(a1ad3e9acbd15400e7da1e50bc6673835cde1fe7), ROM_BIOS(2))
ROM_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor bbc_opus2791_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( opus2791 );
}

machine_config_constructor bbc_opus2793_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( opus2793 );
}

machine_config_constructor bbc_opus1770_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( opus1770 );
}


const tiny_rom_entry *bbc_opus2791_device::device_rom_region() const
{
	return ROM_NAME( opus2791 );
}

const tiny_rom_entry *bbc_opus2793_device::device_rom_region() const
{
	return ROM_NAME( opus2793 );
}

const tiny_rom_entry *bbc_opus1770_device::device_rom_region() const
{
	return ROM_NAME( opus1770 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_opusfdc_device - constructor
//-------------------------------------------------

bbc_opusfdc_device::bbc_opusfdc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_bbc_fdc_interface(mconfig, *this),
	m_dfs_rom(*this, "dfs_rom"),
	m_fdc(*this, "fdc"),
	m_floppy0(*this, "fdc:0"),
	m_floppy1(*this, "fdc:1")
{
}

bbc_opus2791_device::bbc_opus2791_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_opusfdc_device(mconfig, BBC_OPUS2791, "Opus 2791 FDC", tag, owner, clock, "bbc_opus2791", __FILE__)
{
}

bbc_opus2793_device::bbc_opus2793_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_opusfdc_device(mconfig, BBC_OPUS2793, "Opus 2793 FDC", tag, owner, clock, "bbc_opus2793", __FILE__)
{
}

bbc_opus1770_device::bbc_opus1770_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_opusfdc_device(mconfig, BBC_OPUS1770, "Opus D-DOS(B) 1770 FDC", tag, owner, clock, "bbc_opus1770", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_opusfdc_device::device_start()
{
	device_t* cpu = machine().device("maincpu");
	address_space& space = cpu->memory().space(AS_PROGRAM);
	m_slot = dynamic_cast<bbc_fdc_slot_device *>(owner());

	space.install_readwrite_handler(0xfe80, 0xfe83, READ8_DEVICE_DELEGATE(m_fdc, wd_fdc_t, read), WRITE8_DEVICE_DELEGATE(m_fdc, wd_fdc_t, write));
	space.install_readwrite_handler(0xfe84, 0xfe84, READ8_DELEGATE(bbc_opusfdc_device, ctrl_r), WRITE8_DELEGATE(bbc_opusfdc_device, ctrl_w));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_opusfdc_device::device_reset()
{
	machine().root_device().membank("bank4")->configure_entry(12, memregion("dfs_rom")->base());

	m_fdc->soft_reset();
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(bbc_opusfdc_device::ctrl_r)
{
	return m_drive_control;
}

WRITE8_MEMBER(bbc_opusfdc_device::ctrl_w)
{
	floppy_image_device *floppy = nullptr;

	m_drive_control = data;

	// bit 0: drive select
	floppy_image_device *floppy0 = m_fdc->subdevice<floppy_connector>("0")->get_device();
	floppy_image_device *floppy1 = m_fdc->subdevice<floppy_connector>("1")->get_device();
	floppy = (BIT(data, 0) ? floppy1 : floppy0);
	m_fdc->set_floppy(floppy);

	// bit 1: side select
	if (floppy)
		floppy->ss_w(BIT(data, 1));

	// bit 6: density
	m_fdc->dden_w(!BIT(data, 6));
}

WRITE_LINE_MEMBER(bbc_opusfdc_device::fdc_intrq_w)
{
	m_slot->intrq_w(state);
}

WRITE_LINE_MEMBER(bbc_opusfdc_device::fdc_drq_w)
{
	m_slot->drq_w(state);
}

WRITE_LINE_MEMBER(bbc_opusfdc_device::motor_w)
{
	for (int i = 0; i != 2; i++) {
		char devname[8];
		sprintf(devname, "%d", i);
		floppy_connector *con = m_fdc->subdevice<floppy_connector>(devname);
		if (con) {
			con->get_device()->mon_w(!state);
		}
	}
}
