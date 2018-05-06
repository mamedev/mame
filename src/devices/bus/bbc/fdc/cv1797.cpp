// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Computer Village 1797 FDC

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/ComputerVillage_FDC.html

**********************************************************************/


#include "emu.h"
#include "cv1797.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_CV1797, bbc_cv1797_device,  "bbc_cv1797", "Computer Village 1797 FDC")


//-------------------------------------------------
//  MACHINE_DRIVER( cv1797 )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( bbc_cv1797_device::floppy_formats )
	FLOPPY_ACORN_SSD_FORMAT,
	FLOPPY_ACORN_DSD_FORMAT,
	FLOPPY_FSD_FORMAT
FLOPPY_FORMATS_END0

static void bbc_floppies_525(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD);
	device.option_add("525sd",   FLOPPY_525_SD);
	device.option_add("525ssdd", FLOPPY_525_SSDD);
	device.option_add("525dd",   FLOPPY_525_DD);
	device.option_add("525qd",   FLOPPY_525_QD);
}

ROM_START( cv1797 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_DEFAULT_BIOS("lvldos")
	ROM_SYSTEM_BIOS(0, "lvldos", "LVL Dos 0.91")
	ROMX_LOAD("lvldos91.rom", 0x0000, 0x2000, CRC(69d7653a) SHA1(9cd39d011290b97d5ba05b6c745689a8553be5fc), ROM_BIOS(1))
	ROM_RELOAD(0x2000, 0x2000)
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(bbc_cv1797_device::device_add_mconfig)
	MCFG_FD1797_ADD("fd1797", XTAL(8'000'000) / 8)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(*this, bbc_cv1797_device, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(*this, bbc_cv1797_device, fdc_drq_w))
	MCFG_WD_FDC_HLD_CALLBACK(WRITELINE(*this, bbc_cv1797_device, motor_w))
	MCFG_FLOPPY_DRIVE_ADD("fd1797:0", bbc_floppies_525, "525qd", floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fd1797:1", bbc_floppies_525, "525qd", floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

const tiny_rom_entry *bbc_cv1797_device::device_rom_region() const
{
	return ROM_NAME( cv1797 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_cv1797_device - constructor
//-------------------------------------------------

bbc_cv1797_device::bbc_cv1797_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_CV1797, tag, owner, clock),
		device_bbc_fdc_interface(mconfig, *this),
		m_dfs_rom(*this, "dfs_rom"),
		m_fdc(*this, "fd1797"),
		m_floppy0(*this, "fd1797:0"),
		m_floppy1(*this, "fd1797:1")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_cv1797_device::device_start()
{
	device_t* cpu = machine().device("maincpu");
	address_space& space = cpu->memory().space(AS_PROGRAM);
	m_slot = dynamic_cast<bbc_fdc_slot_device *>(owner());

	space.install_readwrite_handler(0xfe80, 0xfe80, read8_delegate(FUNC(fd1797_device::read), (fd1797_device *)m_fdc), write8_delegate(FUNC(fd1797_device::write), (fd1797_device *)m_fdc));
	space.install_readwrite_handler(0xfe84, 0xfe9f, read8_delegate(FUNC(bbc_cv1797_device::fd1797l_read), this), write8_delegate(FUNC(bbc_cv1797_device::fd1797l_write), this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_cv1797_device::device_reset()
{
	machine().root_device().membank("bank4")->configure_entry(12, memregion("dfs_rom")->base());

	m_fdc->soft_reset();
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(bbc_cv1797_device::fd1797l_read)
{
	return m_drive_control;
}

WRITE8_MEMBER(bbc_cv1797_device::fd1797l_write)
{
	floppy_image_device *floppy = nullptr;

	m_drive_control = data;
	logerror("fdc: Drive control %02x\n", data);
	// bit 0: drive select
	floppy_image_device *floppy0 = m_floppy0->get_device();
	floppy_image_device *floppy1 = m_floppy1->get_device();
	floppy = (BIT(data, 0) ? floppy1 : floppy0);
	m_fdc->set_floppy(floppy);

	// bit 1: side select
	if (floppy)
		floppy->ss_w(BIT(data, 1));

	// bit 2: density
	m_fdc->dden_w(BIT(data, 2));

	// bit 6: reset
	//if (BIT(data, 6)) m_fdc->soft_reset();
}

WRITE_LINE_MEMBER(bbc_cv1797_device::fdc_intrq_w)
{
	m_slot->intrq_w(state);
}

WRITE_LINE_MEMBER(bbc_cv1797_device::fdc_drq_w)
{
	m_slot->drq_w(state);
}

WRITE_LINE_MEMBER(bbc_cv1797_device::motor_w)
{
	if (m_floppy0->get_device()) m_floppy0->get_device()->mon_w(!state);
	if (m_floppy1->get_device()) m_floppy1->get_device()->mon_w(!state);
}
