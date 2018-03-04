// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn 80286 2nd Processor

**********************************************************************/


#include "emu.h"
#include "tube_80286.h"
#include "softlist_dev.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_TUBE_80286, bbc_tube_80286_device, "bbc_tube_80286", "Acorn 80286 2nd Processor")


//-------------------------------------------------
//  ADDRESS_MAP( tube_80286_mem )
//-------------------------------------------------

ADDRESS_MAP_START(bbc_tube_80286_device::tube_80286_mem)
	ADDRESS_MAP_GLOBAL_MASK(0xfffff)
	AM_RANGE(0x00000, 0xbffff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0xc0000, 0xc3fff) AM_ROM AM_REGION("bootstrap", 0) AM_MIRROR(0x3c000)
ADDRESS_MAP_END

//-------------------------------------------------
//  ADDRESS_MAP( tube_80286_io )
//-------------------------------------------------

ADDRESS_MAP_START(bbc_tube_80286_device::tube_80286_io)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x50, 0x51) AM_READ8(disable_boot_rom, 0x00ff)
	AM_RANGE(0x60, 0x61) AM_WRITE8(irq_latch_w, 0x00ff)
	AM_RANGE(0x80, 0x8f) AM_DEVREADWRITE8("ula", tube_device, parasite_r, parasite_w, 0x00ff)
ADDRESS_MAP_END

//-------------------------------------------------
//  ROM( tube_80286 )
//-------------------------------------------------

ROM_START( tube_80286 )
	ROM_REGION(0x4000, "bootstrap", 0)
	ROM_LOAD16_BYTE("M512_LO_IC31.rom", 0x0000, 0x2000, CRC(c0df8707) SHA1(7f6d843d5aea6bdb36cbd4623ae942b16b96069d)) // 2201,287-02
	ROM_LOAD16_BYTE("M512_HI_IC32.rom", 0x0001, 0x2000, CRC(e47f10b2) SHA1(45dc8d7e7936afbec6de423569d9005a1c350316)) // 2201,288-02
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(bbc_tube_80286_device::device_add_mconfig)
	MCFG_CPU_ADD("i80286", I80286, XTAL(12'000'000) / 2)
	MCFG_CPU_PROGRAM_MAP(tube_80286_mem)
	MCFG_CPU_IO_MAP(tube_80286_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE(DEVICE_SELF, bbc_tube_80286_device, irq_callback)

	MCFG_TUBE_ADD("ula")
	MCFG_TUBE_PNMI_HANDLER(INPUTLINE("i80286", INPUT_LINE_NMI))
	MCFG_TUBE_PIRQ_HANDLER(INPUTLINE("i80286", INPUT_LINE_INT0))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1M")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("flop_ls_80186", "bbc_flop_80186")
MACHINE_CONFIG_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_tube_80286_device::device_rom_region() const
{
	return ROM_NAME( tube_80286 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_tube_80286_device - constructor
//-------------------------------------------------

bbc_tube_80286_device::bbc_tube_80286_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_TUBE_80286, tag, owner, clock),
		device_bbc_tube_interface(mconfig, *this),
		m_i80286(*this, "i80286"),
		m_ula(*this, "ula"),
		m_ram(*this, "ram"),
		m_bootstrap(*this, "bootstrap")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_80286_device::device_start()
{
	m_slot = dynamic_cast<bbc_tube_slot_device *>(owner());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_tube_80286_device::device_reset()
{
	m_ula->reset();
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(bbc_tube_80286_device::host_r)
{
	return m_ula->host_r(space, offset);
}

WRITE8_MEMBER(bbc_tube_80286_device::host_w)
{
	m_ula->host_w(space, offset, data);
}

READ8_MEMBER(bbc_tube_80286_device::disable_boot_rom)
{
	m_i80286->space(AS_PROGRAM).install_ram(0xc0000, 0xfffff, m_ram->pointer() + 0xc0000);

	return 0xff;
}

WRITE8_MEMBER(bbc_tube_80286_device::irq_latch_w)
{
	m_irq_latch = data;
}

//-------------------------------------------------
//  irq vector callback
//-------------------------------------------------

IRQ_CALLBACK_MEMBER(bbc_tube_80286_device::irq_callback)
{
	return m_irq_latch;
}
