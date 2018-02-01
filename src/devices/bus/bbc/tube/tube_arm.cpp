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

ADDRESS_MAP_START(bbc_tube_arm_device::tube_arm_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000000, 0x03fffff) AM_READWRITE8(ram_r, ram_w, 0xffffffff)
	AM_RANGE(0x1000000, 0x100001f) AM_DEVREADWRITE8("ula", tube_device, parasite_r, parasite_w, 0x000000ff)
	AM_RANGE(0x3000000, 0x3003fff) AM_ROM AM_REGION("bootstrap", 0) AM_MIRROR(0xc000)
ADDRESS_MAP_END

//-------------------------------------------------
//  ROM( tube_arm )
//-------------------------------------------------

ROM_START( tube_arm )
	ROM_REGION(0x4000, "bootstrap", 0)
	ROM_DEFAULT_BIOS("101")
	ROM_SYSTEM_BIOS(0, "101", "Executive v1.00 (14th August 1986)")
	ROMX_LOAD("ARMeval_101.rom", 0x0000, 0x4000, CRC(cab85473) SHA1(f86bbc4894e62725b8ef22d44e7f44d37c98ac14), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "100", "Executive v1.00 (6th June 1986)")
	ROMX_LOAD("ARMeval_100.rom", 0x0000, 0x4000, CRC(ed80462a) SHA1(ba33eaf1a23cfef6fc1b88aa516ca2b3693e69d9), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "005", "Brazil v-.005 (8th August 1986)")
	ROMX_LOAD("Brazil_005.rom", 0x0000, 0x4000, CRC(7c27c098) SHA1(abcc71cbc43489e89a87aac64e67b17daef5895a), ROM_BIOS(3))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(bbc_tube_arm_device::device_add_mconfig)
	MCFG_CPU_ADD("arm", ARM, XTAL(20'000'000) / 3)
	MCFG_CPU_PROGRAM_MAP(tube_arm_mem)

	MCFG_TUBE_ADD("ula")
	MCFG_TUBE_HIRQ_HANDLER(DEVWRITELINE(DEVICE_SELF_OWNER, bbc_tube_slot_device, irq_w))
	MCFG_TUBE_PNMI_HANDLER(INPUTLINE("arm", ARM_FIRQ_LINE))
	MCFG_TUBE_PIRQ_HANDLER(INPUTLINE("arm", ARM_IRQ_LINE))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_DEFAULT_VALUE(0x00)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("flop_ls_arm", "bbc_flop_arm")
MACHINE_CONFIG_END

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
	: device_t(mconfig, BBC_TUBE_ARM, tag, owner, clock),
		device_bbc_tube_interface(mconfig, *this),
		m_arm(*this, "arm"),
		m_ula(*this, "ula"),
		m_ram(*this, "ram"),
		m_bootstrap(*this, "bootstrap"),
		m_rom_select(true)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_arm_device::device_start()
{
	m_slot = dynamic_cast<bbc_tube_slot_device *>(owner());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_tube_arm_device::device_reset()
{
	/* enable the reset vector to be fetched from ROM */
	m_rom_select = true;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(bbc_tube_arm_device::host_r)
{
	return m_ula->host_r(space, offset);
}

WRITE8_MEMBER(bbc_tube_arm_device::host_w)
{
	m_ula->host_w(space, offset, data);
}


READ8_MEMBER(bbc_tube_arm_device::ram_r)
{
	uint8_t data;

	if (m_rom_select)
		data = m_bootstrap->base()[offset & 0x3fff];
	else
		data = m_ram->pointer()[offset];

	return data;
}

WRITE8_MEMBER(bbc_tube_arm_device::ram_w)
{
	/* clear ROM select on first write */
	if (!machine().side_effect_disabled()) m_rom_select = false;

	m_ram->pointer()[offset] = data;
}
