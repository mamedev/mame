// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn 8271 and 1770 FDC

**********************************************************************/


#include "emu.h"
#include "acorn.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_ACORN8271, bbc_acorn8271_device, "bbc_acorn8271", "Acorn 8721 FDC")
DEFINE_DEVICE_TYPE(BBC_ACORN1770, bbc_acorn1770_device, "bbc_acorn1770", "Acorn 1770 FDC")


//-------------------------------------------------
//  MACHINE_DRIVER( acorn )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( bbc_acorn8271_device::floppy_formats )
	FLOPPY_ACORN_SSD_FORMAT,
	FLOPPY_ACORN_DSD_FORMAT,
	FLOPPY_ACORN_ADFS_OLD_FORMAT,
	FLOPPY_ACORN_DOS_FORMAT,
	FLOPPY_FSD_FORMAT,
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END0

static void bbc_floppies_525(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD);
	device.option_add("525sd",   FLOPPY_525_SD);
	device.option_add("525ssdd", FLOPPY_525_SSDD);
	device.option_add("525dd",   FLOPPY_525_DD);
	device.option_add("525qd",   FLOPPY_525_QD);
}

ROM_START( acorn8271 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_DEFAULT_BIOS("dnfs120")
	// Acorn
	ROM_SYSTEM_BIOS(0, "dfs090", "Acorn DFS 0.90")
	ROMX_LOAD("dfs090.rom", 0x0000, 0x2000, CRC(3ce609cf) SHA1(5cc0f14b8f46855c70eaa653cca4ad079b458732), ROM_BIOS(0))
	ROM_RELOAD(             0x2000, 0x2000)
	ROM_SYSTEM_BIOS(1, "dfs098", "Acorn DFS 0.98")
	ROMX_LOAD("dfs098.rom", 0x0000, 0x2000, CRC(90852e7d) SHA1(6df3552d5426f3a4625b9a0c7829bdba03f05e84), ROM_BIOS(1))
	ROM_RELOAD(             0x2000, 0x2000)
	ROM_SYSTEM_BIOS(2, "dnfs100", "Acorn DFS 1.00")
	ROMX_LOAD("dnfs100.rom", 0x0000, 0x4000, CRC(7e367e8c) SHA1(161f585dc45665ea77433c84afd2f95049f7f5a0), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "dnfs120", "Acorn DFS 1.20")
	ROMX_LOAD("dnfs120.rom", 0x0000, 0x4000, CRC(8ccd2157) SHA1(7e3c536baeae84d6498a14e8405319e01ee78232), ROM_BIOS(3))
	// Watford Electronics
	ROM_SYSTEM_BIOS(4, "wdfs130", "Watford Electronics DFS 1.30")
	ROMX_LOAD("wedfs130.rom", 0x0000, 0x4000, CRC(153edf1f) SHA1(01455e8762fe21a5fbb0c383793bcc6e9c34904c), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "wdfs141", "Watford Electronics DFS 1.41")
	ROMX_LOAD("wedfs141.rom", 0x0000, 0x4000, CRC(fda3f58d) SHA1(4f3984ebb35709b39369cb516440dd0589920337), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "wdfs142", "Watford Electronics DFS 1.42")
	ROMX_LOAD("wedfs142.rom", 0x0000, 0x4000, CRC(aef838de) SHA1(0caee270eddd8feb3fa75f721775e9b80f853358), ROM_BIOS(6))
	ROM_SYSTEM_BIOS(7, "wdfs143", "Watford Electronics DFS 1.43")
	ROMX_LOAD("wedfs143.rom", 0x0000, 0x4000, CRC(3755ee18) SHA1(69bc258880b8e163ff85a293a25e5f00931d030b), ROM_BIOS(7))
	ROM_SYSTEM_BIOS(8, "wdfs144", "Watford Electronics DFS 1.44")
	ROMX_LOAD("wedfs144.rom", 0x0000, 0x4000, CRC(9fb8d13f) SHA1(387d2468c6e1360f5b531784ce95d5f71a50c2b5), ROM_BIOS(8))
ROM_END

ROM_START( acorn1770 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_DEFAULT_BIOS("dfs223")
	// Acorn
	ROM_SYSTEM_BIOS(0, "dfs210", "Acorn DFS 2.10")
	ROMX_LOAD("dfs v2.10,acorn.rom", 0x0000, 0x4000, CRC(4f828787) SHA1(112a315e1598cb4db2abcfe9d89fcd97444b276d), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "dfs220", "Acorn DFS 2.20")
	ROMX_LOAD("dfs v2.20,acorn.rom", 0x0000, 0x4000, CRC(2844001e) SHA1(d72f11af35756ac648095ba1992211cf6741dd80), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "dfs223", "Acorn DFS 2.23")
	ROMX_LOAD("dfs v2.23,acorn.rom", 0x0000, 0x4000, CRC(7891f9b7) SHA1(0d7ed0b0b3852cb61970ada1993244f2896896aa), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "dfs225", "Acorn DFS 2.25")
	ROMX_LOAD("dfs v2.25,acorn.rom", 0x0000, 0x4000, CRC(f855a75b) SHA1(f11271748e6303c60182955e5fa478624b616fcf), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "dfs226", "Acorn DFS 2.26")
	ROMX_LOAD("dfs v2.26,acorn.rom", 0x0000, 0x4000, CRC(5ae33e94) SHA1(cf2ebc422a8d24ec6f1a0320520c38a0e704109a), ROM_BIOS(4))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_acorn8271_device::device_add_mconfig(machine_config &config)
{
	I8271(config, m_fdc, DERIVED_CLOCK(1, 4));
	m_fdc->intrq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_fdc_slot_device::intrq_w));
	m_fdc->hdl_wr_callback().set(FUNC(bbc_acorn8271_device::motor_w));
	m_fdc->opt_wr_callback().set(FUNC(bbc_acorn8271_device::side_w));

	FLOPPY_CONNECTOR(config, m_floppy0, bbc_floppies_525, "525qd", bbc_acorn8271_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy1, bbc_floppies_525, "525qd", bbc_acorn8271_device::floppy_formats).enable_sound(true);
}

void bbc_acorn1770_device::device_add_mconfig(machine_config &config)
{
	WD1770(config, m_fdc, DERIVED_CLOCK(1, 1));
	m_fdc->set_force_ready(true);
	m_fdc->intrq_wr_callback().set(FUNC(bbc_acorn1770_device::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(bbc_acorn1770_device::fdc_drq_w));

	FLOPPY_CONNECTOR(config, m_floppy0, bbc_floppies_525, "525qd", bbc_acorn8271_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy1, bbc_floppies_525, "525qd", bbc_acorn8271_device::floppy_formats).enable_sound(true);
}


const tiny_rom_entry *bbc_acorn8271_device::device_rom_region() const
{
	return ROM_NAME( acorn8271 );
}

const tiny_rom_entry *bbc_acorn1770_device::device_rom_region() const
{
	return ROM_NAME( acorn1770 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_acornfdc_device - constructor
//-------------------------------------------------

bbc_acorn8271_device::bbc_acorn8271_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_ACORN8271, tag, owner, clock)
	, device_bbc_fdc_interface(mconfig, *this)
	, m_fdc(*this, "i8271")
	, m_floppy0(*this, "i8271:0")
	, m_floppy1(*this, "i8271:1")
{
}

bbc_acorn1770_device::bbc_acorn1770_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_ACORN1770, tag, owner, clock)
	, device_bbc_fdc_interface(mconfig, *this)
	, m_fdc(*this, "wd1770")
	, m_floppy0(*this, "wd1770:0")
	, m_floppy1(*this, "wd1770:1")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_acorn8271_device::device_start()
{
}

void bbc_acorn1770_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_acorn8271_device::read(offs_t offset)
{
	uint8_t data;

	if (offset & 0x04)
	{
		data = m_fdc->data_r();
	}
	else
	{
		data = m_fdc->read(offset & 0x03);
	}
	return data;
}

void bbc_acorn8271_device::write(offs_t offset, uint8_t data)
{
	if (offset & 0x04)
	{
		m_fdc->data_w(data);
	}
	else
	{
		m_fdc->write(offset & 0x03, data);
	}
}

WRITE_LINE_MEMBER(bbc_acorn8271_device::motor_w)
{
	if (m_floppy0->get_device()) m_floppy0->get_device()->mon_w(!state);
	if (m_floppy1->get_device()) m_floppy1->get_device()->mon_w(!state);
	m_fdc->ready_w(!state);
}

WRITE_LINE_MEMBER(bbc_acorn8271_device::side_w)
{
	if (m_floppy0->get_device()) m_floppy0->get_device()->ss_w(state);
	if (m_floppy1->get_device()) m_floppy1->get_device()->ss_w(state);
}


uint8_t bbc_acorn1770_device::read(offs_t offset)
{
	uint8_t data = 0xff;

	if (offset & 0x04)
	{
		data = m_fdc->read(offset & 0x03);
	}
	else
	{
		data = 0xfe;
	}
	return data;
}

void bbc_acorn1770_device::write(offs_t offset, uint8_t data)
{
	if (offset & 0x04)
	{
		m_fdc->write(offset & 0x03, data);
	}
	else
	{
		floppy_image_device *floppy = nullptr;

		// bit 0, 1: drive select
		if (BIT(data, 0)) floppy = m_floppy0->get_device();
		if (BIT(data, 1)) floppy = m_floppy1->get_device();
		m_fdc->set_floppy(floppy);

		// bit 2: side select
		if (floppy)
			floppy->ss_w(BIT(data, 2));

		// bit 3: density
		m_fdc->dden_w(BIT(data, 3));

		// bit 4: interrupt enabled
		m_fdc_ie = !BIT(data, 4);

		// bit 5: reset
		if (!BIT(data, 5)) m_fdc->soft_reset();
	}
}

WRITE_LINE_MEMBER(bbc_acorn1770_device::fdc_intrq_w)
{
	m_slot->intrq_w((state && m_fdc_ie) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(bbc_acorn1770_device::fdc_drq_w)
{
	m_slot->drq_w((state && m_fdc_ie) ? ASSERT_LINE : CLEAR_LINE);
}
