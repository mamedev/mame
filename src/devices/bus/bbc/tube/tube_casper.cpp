// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Casper 68000 2nd Processor

**********************************************************************/


#include "emu.h"
#include "tube_casper.h"
#include "softlist_dev.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_TUBE_CASPER, bbc_tube_casper_device, "bbc_tube_casper", "Casper 68000 2nd Processor")


//-------------------------------------------------
//  ADDRESS_MAP( tube_casper_mem )
//-------------------------------------------------

void bbc_tube_casper_device::tube_casper_mem(address_map &map)
{
	map(0x00000, 0x03fff).rom().region("casper_rom", 0);
	map(0x10000, 0x1001f).rw("via6522_1", FUNC(via6522_device::read), FUNC(via6522_device::write)).umask16(0x00ff);
	map(0x20000, 0x3ffff).ram();
}

//-------------------------------------------------
//  ROM( tube_casper )
//-------------------------------------------------

ROM_START( tube_casper )
	ROM_REGION(0x4000, "casper_rom", 0)
	ROM_LOAD16_BYTE("casper.ic9",  0x0000, 0x2000, CRC(4105cbf4) SHA1(a3efeb6fb144da55b47c718239967ed0af4fff72))
	ROM_LOAD16_BYTE("casper.ic10", 0x0001, 0x2000, CRC(f25bc320) SHA1(297db56283bb3164c31c21331837213cea426837))

	ROM_REGION(0x8000, "host_rom", 0)
	ROM_LOAD("rom1.rom", 0x0000, 0x4000, CRC(602b6a36) SHA1(7b24746dbcacb8772468532e92832d5c7f6648fd))
	ROM_LOAD("rom2.rom", 0x4000, 0x4000, CRC(7c9efb43) SHA1(4195ce1ed928178fd645a267872a5b4f325d078a))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(bbc_tube_casper_device::device_add_mconfig)
	MCFG_DEVICE_ADD(m_m68000, M68000, XTAL(4'000'000))
	MCFG_DEVICE_PROGRAM_MAP(tube_casper_mem)

	MCFG_DEVICE_ADD(m_via6522_0, VIA6522, XTAL(4'000'000) / 2)
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(m_via6522_1, via6522_device, write_pa))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(m_via6522_1, via6522_device, write_cb1))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(m_via6522_1, via6522_device, write_ca1))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(DEVICE_SELF_OWNER, bbc_tube_slot_device, irq_w))

	MCFG_DEVICE_ADD(m_via6522_1, VIA6522, XTAL(4'000'000) / 2)
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(m_via6522_0, via6522_device, write_pa))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(m_via6522_0, via6522_device, write_cb1))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(m_via6522_0, via6522_device, write_ca1))
	MCFG_VIA6522_IRQ_HANDLER(INPUTLINE(m_m68000, M68K_IRQ_1))

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("flop_ls_casper", "bbc_flop_68000")
MACHINE_CONFIG_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_tube_casper_device::device_rom_region() const
{
	return ROM_NAME( tube_casper );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_tube_casper_device - constructor
//-------------------------------------------------

bbc_tube_casper_device::bbc_tube_casper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_TUBE_CASPER, tag, owner, clock),
		device_bbc_tube_interface(mconfig, *this),
		m_m68000(*this, "m68000"),
		m_via6522_0(*this, "via6522_0"),
		m_via6522_1(*this, "via6522_1"),
		m_casper_rom(*this, "casper_rom"),
		m_host_rom(*this, "host_rom")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_casper_device::device_start()
{
	m_slot = dynamic_cast<bbc_tube_slot_device *>(owner());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_tube_casper_device::device_reset()
{
	machine().root_device().membank("bank4")->configure_entry(13, memregion("host_rom")->base() + 0x0000);
	machine().root_device().membank("bank4")->configure_entry(14, memregion("host_rom")->base() + 0x4000);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(bbc_tube_casper_device::host_r)
{
	return m_via6522_0->read(space, offset & 0xf);
}

WRITE8_MEMBER(bbc_tube_casper_device::host_w)
{
	m_via6522_0->write(space, offset & 0xf, data);
}
