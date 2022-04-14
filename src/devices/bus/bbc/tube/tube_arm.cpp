// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn ANC13 ARM Evaluation System

**********************************************************************/


#include "emu.h"
#include "tube_arm.h"
#include "softlist_dev.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_TUBE_ARM, bbc_tube_arm_device, "bbc_tube_arm", "ARM Evaluation System")


//-------------------------------------------------
//  ADDRESS_MAP( tube_arm_mem )
//-------------------------------------------------

void bbc_tube_arm_device::tube_arm_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000000, 0x03fffff).ram().share(RAM_TAG);
	map(0x0400000, 0x0ffffff).noprw();
	map(0x1000000, 0x100001f).rw(m_ula, FUNC(tube_device::parasite_r), FUNC(tube_device::parasite_w)).umask32(0x000000ff);
	map(0x3000000, 0x3003fff).rom().region("bootstrap", 0).mirror(0xc000);
}

//-------------------------------------------------
//  ROM( tube_arm )
//-------------------------------------------------

ROM_START( tube_arm )
	ROM_REGION32_LE(0x4000, "bootstrap", 0)
	ROM_DEFAULT_BIOS("101")
	ROM_SYSTEM_BIOS(0, "101", "Executive v1.00 (14th August 1986)")
	ROMX_LOAD("armeval_101.rom", 0x0000, 0x4000, CRC(cab85473) SHA1(f86bbc4894e62725b8ef22d44e7f44d37c98ac14), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "100", "Executive v1.00 (6th June 1986)")
	ROMX_LOAD("armeval_100.rom", 0x0000, 0x4000, CRC(ed80462a) SHA1(ba33eaf1a23cfef6fc1b88aa516ca2b3693e69d9), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "005", "Brazil v-.005 (8th August 1986)")
	ROMX_LOAD("brazil_005.rom", 0x0000, 0x4000, CRC(7c27c098) SHA1(abcc71cbc43489e89a87aac64e67b17daef5895a), ROM_BIOS(2))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_tube_arm_device::device_add_mconfig(machine_config &config)
{
	ARM(config, m_maincpu, 20_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &bbc_tube_arm_device::tube_arm_mem);

	TUBE(config, m_ula);
	m_ula->pnmi_handler().set_inputline(m_maincpu, ARM_FIRQ_LINE);
	m_ula->pirq_handler().set_inputline(m_maincpu, ARM_IRQ_LINE);

	/* internal ram */
	RAM(config, m_ram).set_default_size("4M").set_default_value(0);

	/* software lists */
	SOFTWARE_LIST(config, "flop_ls_arm").set_original("bbc_flop_arm");
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_tube_arm_device::device_rom_region() const
{
	return ROM_NAME( tube_arm );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_tube_arm_device - constructor
//-------------------------------------------------

bbc_tube_arm_device::bbc_tube_arm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_TUBE_ARM, tag, owner, clock)
	, device_bbc_tube_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_ula(*this, "ula")
	, m_ram(*this, "ram")
	, m_bootstrap(*this, "bootstrap")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_arm_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_tube_arm_device::device_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	/* enable the reset vector to be fetched from ROM */
	m_maincpu->space(AS_PROGRAM).install_rom(0x000000, 0x003fff, 0x3fc000, m_bootstrap->base());

	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_write_tap(
			0x0000000, 0x03fffff,
			"rom_shadow_w",
			[this] (offs_t offset, u32 &data, u32 mem_mask)
			{
				/* delete this tap */
				m_rom_shadow_tap.remove();

				/* install ram */
				m_maincpu->space(AS_PROGRAM).install_ram(0x0000000, 0x03fffff, m_ram->pointer());
			},
			&m_rom_shadow_tap);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_tube_arm_device::host_r(offs_t offset)
{
	return m_ula->host_r(offset);
}

void bbc_tube_arm_device::host_w(offs_t offset, uint8_t data)
{
	m_ula->host_w(offset, data);
}
