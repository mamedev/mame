// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ALA13 - Acorn Plus 3

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_Plus3.html

    The Acorn Plus 3 was a hardware module that connected independently
    of the Plus 1 and provided a double-density 3.5" disc drive connected
    through a WD1770 drive controller and an ADFS ROM. There were two
    versions of the Plus 3 produced: A Single-sided and a Double-sided
    drive version.

**********************************************************************/


#include "emu.h"
#include "plus3.h"
#include "softlist.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ELECTRON_PLUS3 = device_creator<electron_plus3_device>;


//-------------------------------------------------
//  MACHINE_DRIVER( plus3 )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER(floppy_formats)
	FLOPPY_ACORN_SSD_FORMAT,
	FLOPPY_ACORN_DSD_FORMAT,
	FLOPPY_ACORN_ADFS_OLD_FORMAT
FLOPPY_FORMATS_END0

SLOT_INTERFACE_START(electron_floppies)
	SLOT_INTERFACE("35dd",    FLOPPY_35_DD)
	SLOT_INTERFACE("525sd",   FLOPPY_525_SD)
	SLOT_INTERFACE("525dd",   FLOPPY_525_DD)
	SLOT_INTERFACE("525qd",   FLOPPY_525_QD)
SLOT_INTERFACE_END


MACHINE_CONFIG_FRAGMENT( plus3 )
	/* fdc */
	MCFG_WD1770_ADD("fdc", XTAL_16MHz / 2)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", electron_floppies, "35dd", floppy_formats)
	MCFG_SLOT_FIXED(true)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", electron_floppies, nullptr, floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("flop_ls", "electron_flop")

	/* pass-through */
	MCFG_ELECTRON_PASSTHRU_EXPANSION_SLOT_ADD(nullptr)
MACHINE_CONFIG_END


ROM_START( plus3 )
	// Bank 4 Disc
	ROM_REGION(0x4000, "exp_rom", 0)
	ROM_DEFAULT_BIOS("adfs")
	// ADFS
	ROM_SYSTEM_BIOS(0, "adfs100", "Acorn ADFS 1.00")
	ROMX_LOAD("adfs.rom", 0x0000, 0x4000, CRC(3289bdc6) SHA1(e7c7a1094d50a3579751df2007269067c8ff6812), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "adfs113", "PRES ADFS 1.13")
	ROMX_LOAD("pres_adfs_113.rom", 0x0000, 0x4000, CRC(f06ca04a) SHA1(3c8221d63457c552aa2c9a502db632ce1dea66b4), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "adfs115", "PRES ADFS 1.15")
	ROMX_LOAD("pres_adfs_115.rom", 0x0000, 0x4000, CRC(8f81edc3) SHA1(32007425058a7b0f8bd5c17b3c22552aa3a03eca), ROM_BIOS(3))
	// DFS
	ROM_SYSTEM_BIOS(3, "dfs200", "Advanced 1770 DFS 2.00")
	ROMX_LOAD("acp_dfs1770_200.rom", 0x0000, 0x4000, CRC(5a3a13c7) SHA1(d5dad7ab5a0237c44d0426cd85a8ec86545747e0), ROM_BIOS(4))
ROM_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor electron_plus3_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( plus3 );
}

const tiny_rom_entry *electron_plus3_device::device_rom_region() const
{
	return ROM_NAME( plus3 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_plus3_device - constructor
//-------------------------------------------------

electron_plus3_device::electron_plus3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_PLUS3, "Acorn Plus 3 Disc Expansion", tag, owner, clock, "electron_plus3", __FILE__),
		device_electron_expansion_interface(mconfig, *this),
		m_exp_rom(*this, "exp_rom"),
		m_fdc(*this, "fdc"),
		m_floppy0(*this, "fdc:0"),
		m_floppy1(*this, "fdc:1")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_plus3_device::device_start()
{
	address_space& space = machine().device("maincpu")->memory().space(AS_PROGRAM);
	m_slot = dynamic_cast<electron_expansion_slot_device *>(owner());

	space.install_readwrite_handler(0xfcc0, 0xfcc0, READ8_DELEGATE(electron_plus3_device, wd1770_status_r), WRITE8_DELEGATE(electron_plus3_device, wd1770_status_w));
	space.install_readwrite_handler(0xfcc4, 0xfcc7, READ8_DEVICE_DELEGATE(m_fdc, wd1770_t, read), WRITE8_DEVICE_DELEGATE(m_fdc, wd1770_t, write));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void electron_plus3_device::device_reset()
{
	machine().root_device().membank("bank2")->configure_entry(4, memregion("exp_rom")->base());
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(electron_plus3_device::wd1770_status_r)
{
	return 0xff;
}


WRITE8_MEMBER(electron_plus3_device::wd1770_status_w)
{
	floppy_image_device *floppy = nullptr;

	m_drive_control = data;

	// bit 0, 1: drive select
	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	if (BIT(data, 1)) floppy = m_floppy1->get_device();
	m_fdc->set_floppy(floppy);

	// bit 2: side select
	if (floppy)
		floppy->ss_w(BIT(data, 2));

	// bit 3: density
	m_fdc->dden_w(BIT(data, 3));

	// bit 5: reset
	if (!BIT(data, 5)) m_fdc->soft_reset();
}
