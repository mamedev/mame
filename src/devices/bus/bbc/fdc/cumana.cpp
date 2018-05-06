// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Cumana QFS 8877A FDC

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Cumana_QFS.html

**********************************************************************/


#include "emu.h"
#include "cumana.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_CUMANA1, bbc_cumana1_device, "bbc_cumana1", "Cumana QFS 8877A FDC")
DEFINE_DEVICE_TYPE(BBC_CUMANA2, bbc_cumana2_device, "bbc_cumana2", "Cumana QFS Issue 2 8877A FDC")


//-------------------------------------------------
//  MACHINE_DRIVER( cumana )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( bbc_cumanafdc_device::floppy_formats )
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

ROM_START( cumana1 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_DEFAULT_BIOS("qfs102")
	ROM_SYSTEM_BIOS(0, "qfs102", "QFS 1.02")
	ROMX_LOAD("qfs102.rom", 0x0000, 0x4000, CRC(083a9285) SHA1(4bf31a420a9d752285b088ee4f08a64563527662), ROM_BIOS(1))
ROM_END

ROM_START( cumana2 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_DEFAULT_BIOS("qfs200")
	ROM_SYSTEM_BIOS(0, "qfs200", "QFS 2.00")
	ROMX_LOAD("qfs200.rom", 0x0000, 0x4000, CRC(5863962a) SHA1(0dced741482321938842746ee47090ae443d7ad5), ROM_BIOS(1))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(bbc_cumana1_device::device_add_mconfig)
	MCFG_MB8877_ADD("mb8877a", XTAL(16'000'000) / 16)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(*this, bbc_cumanafdc_device, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(*this, bbc_cumanafdc_device, fdc_drq_w))
	MCFG_WD_FDC_HLD_CALLBACK(WRITELINE(*this, bbc_cumanafdc_device, motor_w))
	MCFG_FLOPPY_DRIVE_ADD("mb8877a:0", bbc_floppies_525, "525qd", floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("mb8877a:1", bbc_floppies_525, "525qd", floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(bbc_cumana2_device::device_add_mconfig)
	MCFG_MB8877_ADD("mb8877a", XTAL(16'000'000) / 16)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(*this, bbc_cumanafdc_device, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(*this, bbc_cumanafdc_device, fdc_drq_w))
	MCFG_WD_FDC_HLD_CALLBACK(WRITELINE(*this, bbc_cumanafdc_device, motor_w))
	MCFG_FLOPPY_DRIVE_ADD("mb8877a:0", bbc_floppies_525, "525qd", floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("mb8877a:1", bbc_floppies_525, "525qd", floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

const tiny_rom_entry *bbc_cumana1_device::device_rom_region() const
{
	return ROM_NAME( cumana1 );
}

const tiny_rom_entry *bbc_cumana2_device::device_rom_region() const
{
	return ROM_NAME( cumana2 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_cumanafdc_device - constructor
//-------------------------------------------------

bbc_cumanafdc_device::bbc_cumanafdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_bbc_fdc_interface(mconfig, *this),
	m_dfs_rom(*this, "dfs_rom"),
	m_fdc(*this, "mb8877a"),
	m_floppy0(*this, "mb8877a:0"),
	m_floppy1(*this, "mb8877a:1")
{
}

bbc_cumana1_device::bbc_cumana1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	bbc_cumanafdc_device(mconfig, BBC_CUMANA1, tag, owner, clock)
{
	m_invert = true;
}

bbc_cumana2_device::bbc_cumana2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	bbc_cumanafdc_device(mconfig, BBC_CUMANA2, tag, owner, clock)
{
	m_invert = false;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_cumanafdc_device::device_start()
{
	device_t* cpu = machine().device("maincpu");
	address_space& space = cpu->memory().space(AS_PROGRAM);
	m_slot = dynamic_cast<bbc_fdc_slot_device *>(owner());

	space.install_readwrite_handler(0xfe80, 0xfe83, READ8_DELEGATE(bbc_cumanafdc_device, ctrl_r), WRITE8_DELEGATE(bbc_cumanafdc_device, ctrl_w));
	space.install_readwrite_handler(0xfe84, 0xfe9f, READ8_DEVICE_DELEGATE(m_fdc, mb8877_device, read), WRITE8_DEVICE_DELEGATE(m_fdc, mb8877_device, write));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_cumanafdc_device::device_reset()
{
	machine().root_device().membank("bank4")->configure_entry(12, memregion("dfs_rom")->base());

	m_fdc->soft_reset();
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(bbc_cumanafdc_device::ctrl_r)
{
	return m_drive_control;
}

WRITE8_MEMBER(bbc_cumanafdc_device::ctrl_w)
{
	floppy_image_device *floppy = nullptr;

	m_drive_control = data;

	// bit 0: drive select
	floppy_image_device *floppy0 = m_fdc->subdevice<floppy_connector>("0")->get_device();
	floppy_image_device *floppy1 = m_fdc->subdevice<floppy_connector>("1")->get_device();
	floppy = (BIT(data, 0) ? floppy1 : floppy0);
	m_fdc->set_floppy(floppy);

	// Not sure why QFS 1.02 inverts these two bits, need to see an original board to verify
	if (m_invert)
	{
		// bit 1: side select
		if (floppy)
			floppy->ss_w(!BIT(data, 1));

		// bit 2: density
		m_fdc->dden_w(!BIT(data, 2));
	}
	else
	{
		// bit 1: side select
		if (floppy)
			floppy->ss_w(BIT(data, 1));

		// bit 2: density
		m_fdc->dden_w(BIT(data, 2));
	}
	// bit 3: reset
	if (BIT(data, 3)) m_fdc->soft_reset();

	// bit 4: interrupt enable
	m_fdc_ie = BIT(data, 4);
}

WRITE_LINE_MEMBER(bbc_cumanafdc_device::fdc_intrq_w)
{
	if (m_fdc_ie)
		m_slot->intrq_w(state);
}

WRITE_LINE_MEMBER(bbc_cumanafdc_device::fdc_drq_w)
{
	if (m_fdc_ie)
		m_slot->drq_w(state);
}

WRITE_LINE_MEMBER(bbc_cumanafdc_device::motor_w)
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
