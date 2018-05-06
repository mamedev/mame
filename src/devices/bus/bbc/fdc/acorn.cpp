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

ROM_START( acorn8271 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_DEFAULT_BIOS("dnfs120")
	// Acorn
	ROM_SYSTEM_BIOS(0, "dfs090", "Acorn DFS 0.90")
	ROMX_LOAD("dfs090.rom", 0x0000, 0x2000, CRC(3ce609cf) SHA1(5cc0f14b8f46855c70eaa653cca4ad079b458732), ROM_BIOS(1))
	ROM_RELOAD(             0x2000, 0x2000)
	ROM_SYSTEM_BIOS(1, "dfs098", "Acorn DFS 0.98")
	ROMX_LOAD("dfs098.rom", 0x0000, 0x2000, CRC(90852e7d) SHA1(6df3552d5426f3a4625b9a0c7829bdba03f05e84), ROM_BIOS(2))
	ROM_RELOAD(             0x2000, 0x2000)
	ROM_SYSTEM_BIOS(2, "dnfs100", "Acorn DFS 1.00")
	ROMX_LOAD("dnfs100.rom", 0x0000, 0x4000, CRC(7e367e8c) SHA1(161f585dc45665ea77433c84afd2f95049f7f5a0), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "dnfs120", "Acorn DFS 1.20")
	ROMX_LOAD("dnfs120.rom", 0x0000, 0x4000, CRC(8ccd2157) SHA1(7e3c536baeae84d6498a14e8405319e01ee78232), ROM_BIOS(4))
	// Amcom
	ROM_SYSTEM_BIOS(4, "amdfs0000", "Amcom DFS 00000")
	ROMX_LOAD("amcomdfs_00000.rom", 0x0000, 0x2000, CRC(28947e95) SHA1(43eb9bfc70bb710e01c0c9d48703ea812f9266e4), ROM_BIOS(5))
	ROM_RELOAD(                     0x2000, 0x2000)
	ROM_SYSTEM_BIOS(5, "amdfs7259", "Amcom DFS A7259")
	ROMX_LOAD("amcomdfs_a7259.rom", 0x0000, 0x2000, CRC(a3366a98) SHA1(7b8b08edf5ebf17fcbcfbb2af0c189ac903285c2), ROM_BIOS(6))
	ROM_RELOAD(                     0x2000, 0x2000)
	ROM_SYSTEM_BIOS(6, "amdfs7874", "Amcom DFS A7874")
	ROMX_LOAD("amcomdfs_a7874.rom", 0x0000, 0x2000, CRC(a7efeee8) SHA1(b6b06c7435d570d25a10fff1c703d16942deb8fb), ROM_BIOS(7))
	ROM_RELOAD(0x2000, 0x2000)
	ROM_SYSTEM_BIOS(7, "amdfs4084", "Amcom DFS B4084")
	ROMX_LOAD("amcomdfs_b4084.rom", 0x0000, 0x2000, CRC(487b049b) SHA1(559e056fe77ede2c87a314fcd8d3e2cab2b49b09), ROM_BIOS(8))
	ROM_RELOAD(                     0x2000, 0x2000)
	ROM_SYSTEM_BIOS(8, "amdfs4088", "Amcom DFS B4088")
	ROMX_LOAD("amcomdfs_b4088.rom", 0x0000, 0x2000, CRC(3b6e2f82) SHA1(3f5e5039a926c980957f169de2112793eb1a9890), ROM_BIOS(9))
	ROM_RELOAD(                     0x2000, 0x2000)
	ROM_SYSTEM_BIOS(9, "amdfs4218", "Amcom DFS B4218")
	ROMX_LOAD("amcomdfs_b4218.rom", 0x0000, 0x2000, CRC(98248af5) SHA1(17986b029d641047555eedc538d6790f0cf911a9), ROM_BIOS(10))
	ROM_RELOAD(                     0x2000, 0x2000)
	// Watford Electronics
	ROM_SYSTEM_BIOS(10, "wdfs110", "Watford Electronics DFS 1.10")
	ROMX_LOAD("wedfs110.rom", 0x0000, 0x4000, CRC(73c47a91) SHA1(658eb78b104806e76a0e51de9eb238a51daf646d), ROM_BIOS(11))
	ROM_SYSTEM_BIOS(11, "wdfs130", "Watford Electronics DFS 1.30")
	ROMX_LOAD("wedfs130.rom", 0x0000, 0x4000, CRC(153edf1f) SHA1(01455e8762fe21a5fbb0c383793bcc6e9c34904c), ROM_BIOS(12))
	ROM_SYSTEM_BIOS(12, "wdfs141", "Watford Electronics DFS 1.41")
	ROMX_LOAD("wedfs141.rom", 0x0000, 0x4000, CRC(fda3f58d) SHA1(4f3984ebb35709b39369cb516440dd0589920337), ROM_BIOS(13))
	ROM_SYSTEM_BIOS(13, "wdfs142", "Watford Electronics DFS 1.42")
	ROMX_LOAD("wedfs142.rom", 0x0000, 0x4000, CRC(aef838de) SHA1(0caee270eddd8feb3fa75f721775e9b80f853358), ROM_BIOS(14))
	ROM_SYSTEM_BIOS(14, "wdfs143", "Watford Electronics DFS 1.43")
	ROMX_LOAD("wedfs143.rom", 0x0000, 0x4000, CRC(3755ee18) SHA1(69bc258880b8e163ff85a293a25e5f00931d030b), ROM_BIOS(15))
	ROM_SYSTEM_BIOS(15, "wdfs144", "Watford Electronics DFS 1.44")
	ROMX_LOAD("wedfs144.rom", 0x0000, 0x4000, CRC(9fb8d13f) SHA1(387d2468c6e1360f5b531784ce95d5f71a50c2b5), ROM_BIOS(16))
	// Computer Users Club
	ROM_SYSTEM_BIOS(16, "bsdos219", "BS-DOS 2.19")
	ROMX_LOAD("bs-dos219.rom", 0x0000, 0x4000, CRC(299b17b2) SHA1(175fdcc802a7d94d7c36d6f3fd52a192a7cfbc98), ROM_BIOS(17))
	ROM_SYSTEM_BIOS(17, "bsdos222", "BS-DOS 2.22")
	ROMX_LOAD("bs-dos222.rom", 0x0000, 0x4000, CRC(63cf49d1) SHA1(92948e5843b9d9715354e21f14de2453bc250bf4), ROM_BIOS(18))
	// Others
	ROM_SYSTEM_BIOS(18, "cucdos", "C.U.C DOS 1.00")
	ROMX_LOAD("cucdos.rom", 0x0000, 0x2000, CRC(4adf8ecb) SHA1(ca3c1f9e89799459b6fd5197304129441443ca54), ROM_BIOS(19))
	ROM_RELOAD(             0x2000, 0x2000)
ROM_END

ROM_START( acorn1770 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_DEFAULT_BIOS("dfs223")
	// Acorn
	ROM_SYSTEM_BIOS(0, "dfs210", "Acorn DFS 2.10")
	ROMX_LOAD("dfs v2.10,1985,acorn.bin", 0x0000, 0x4000, CRC(4f828787) SHA1(112a315e1598cb4db2abcfe9d89fcd97444b276d), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "dfs220", "Acorn DFS 2.20")
	ROMX_LOAD("dfs v2.20,1986,acorn.bin", 0x0000, 0x4000, CRC(2844001e) SHA1(d72f11af35756ac648095ba1992211cf6741dd80), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "dfs223", "Acorn DFS 2.23")
	ROMX_LOAD("dfs v2.23,1986,acorn.bin", 0x0000, 0x4000, CRC(7891f9b7) SHA1(0d7ed0b0b3852cb61970ada1993244f2896896aa), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "dfs225", "Acorn DFS 2.25")
	ROMX_LOAD("dfs v2.25,1986,acorn.bin", 0x0000, 0x4000, CRC(f855a75b) SHA1(f11271748e6303c60182955e5fa478624b616fcf), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "dfs226", "Acorn DFS 2.26")
	ROMX_LOAD("dfs v2.26,1986,acorn.bin", 0x0000, 0x4000, CRC(5ae33e94) SHA1(cf2ebc422a8d24ec6f1a0320520c38a0e704109a), ROM_BIOS(5))
	// Advanced Computer Products
	ROM_SYSTEM_BIOS(5, "acp108", "Advanced 1770 DFS 1.08")
	ROMX_LOAD("advanced 1770 dfs 1.08,acp.rom", 0x0000, 0x4000, CRC(eb0eaa34) SHA1(d16ba3c8ed5e5ab6af62aad13a8e567b1c3639c2), ROM_BIOS(6))
	ROM_SYSTEM_BIOS(6, "acp200", "Advanced 1770 DFS 2.00")
	ROMX_LOAD("advanced 1770 dfs 2.00,acp.rom", 0x0000, 0x4000, CRC(65c0d170) SHA1(6907806e2b5b904a6f2041c11f8ccbd298d63ab9), ROM_BIOS(7))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(bbc_acorn8271_device::device_add_mconfig)
	MCFG_DEVICE_ADD("i8271", I8271, 0)
	MCFG_I8271_IRQ_CALLBACK(WRITELINE(*this, bbc_acorn8271_device, fdc_intrq_w))
	MCFG_I8271_HDL_CALLBACK(WRITELINE(*this, bbc_acorn8271_device, motor_w))
	MCFG_I8271_OPT_CALLBACK(WRITELINE(*this, bbc_acorn8271_device, side_w))
	MCFG_FLOPPY_DRIVE_ADD("i8271:0", bbc_floppies_525, "525qd", bbc_acorn8271_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("i8271:1", bbc_floppies_525, "525qd", bbc_acorn8271_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(bbc_acorn1770_device::device_add_mconfig)
	MCFG_WD1770_ADD("wd1770", XTAL(16'000'000) / 2)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(*this, bbc_acorn1770_device, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(*this, bbc_acorn1770_device, fdc_drq_w))
	MCFG_FLOPPY_DRIVE_ADD("wd1770:0", bbc_floppies_525, "525qd", bbc_acorn8271_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("wd1770:1", bbc_floppies_525, "525qd", bbc_acorn8271_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END


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
	: device_t(mconfig, BBC_ACORN8271, tag, owner, clock),
	device_bbc_fdc_interface(mconfig, *this),
	m_dfs_rom(*this, "dfs_rom"),
	m_fdc(*this, "i8271"),
	m_floppy0(*this, "i8271:0"),
	m_floppy1(*this, "i8271:1")
{
}

bbc_acorn1770_device::bbc_acorn1770_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_ACORN1770, tag, owner, clock),
	device_bbc_fdc_interface(mconfig, *this),
	m_dfs_rom(*this, "dfs_rom"),
	m_fdc(*this, "wd1770"),
	m_floppy0(*this, "wd1770:0"),
	m_floppy1(*this, "wd1770:1")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_acorn8271_device::device_start()
{
	device_t* cpu = machine().device("maincpu");
	address_space& space = cpu->memory().space(AS_PROGRAM);
	m_slot = dynamic_cast<bbc_fdc_slot_device *>(owner());

	space.install_device(0xfe80, 0xfe83, *m_fdc, &i8271_device::map);
	space.install_readwrite_handler(0xfe84, 0xfe9f, READ8_DEVICE_DELEGATE(m_fdc, i8271_device, data_r), WRITE8_DEVICE_DELEGATE(m_fdc, i8271_device, data_w));
}

void bbc_acorn1770_device::device_start()
{
	device_t* cpu = machine().device("maincpu");
	address_space& space = cpu->memory().space(AS_PROGRAM);
	m_slot = dynamic_cast<bbc_fdc_slot_device *>(owner());

	space.install_readwrite_handler(0xfe80, 0xfe83, READ8_DELEGATE(bbc_acorn1770_device, wd1770l_read), WRITE8_DELEGATE(bbc_acorn1770_device, wd1770l_write));
	space.install_readwrite_handler(0xfe84, 0xfe9f, READ8_DEVICE_DELEGATE(m_fdc, wd1770_device, read), WRITE8_DEVICE_DELEGATE(m_fdc, wd1770_device, write));

	m_drive_control = 0xfe;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_acorn8271_device::device_reset()
{
	machine().root_device().membank("bank4")->configure_entry(12, memregion("dfs_rom")->base());

	m_fdc->soft_reset();
}

void bbc_acorn1770_device::device_reset()
{
	machine().root_device().membank("bank4")->configure_entry(12, memregion("dfs_rom")->base());
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE_LINE_MEMBER(bbc_acorn8271_device::motor_w)
{
	if (m_floppy0->get_device()) m_floppy0->get_device()->mon_w(!state);
	if (m_floppy1->get_device()) m_floppy1->get_device()->mon_w(!state);
}

WRITE_LINE_MEMBER(bbc_acorn8271_device::side_w)
{
	if (m_floppy0->get_device()) m_floppy0->get_device()->ss_w(state);
	if (m_floppy1->get_device()) m_floppy1->get_device()->ss_w(state);
}

WRITE_LINE_MEMBER(bbc_acorn8271_device::fdc_intrq_w)
{
	m_slot->intrq_w(state);
}


READ8_MEMBER(bbc_acorn1770_device::wd1770l_read)
{
	return m_drive_control;
}

WRITE8_MEMBER(bbc_acorn1770_device::wd1770l_write)
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

WRITE_LINE_MEMBER(bbc_acorn1770_device::fdc_intrq_w)
{
	m_slot->intrq_w(state);
}

WRITE_LINE_MEMBER(bbc_acorn1770_device::fdc_drq_w)
{
	m_slot->drq_w(state);
}
