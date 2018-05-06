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

void bbc_tube_80286_device::tube_80286_mem(address_map &map)
{
	map.global_mask(0xfffff);
	map(0x00000, 0xbffff).ram().share("ram");
	map(0xc0000, 0xc3fff).rom().region("bootstrap", 0).mirror(0x3c000);
}

//-------------------------------------------------
//  ADDRESS_MAP( tube_80286_io )
//-------------------------------------------------

void bbc_tube_80286_device::tube_80286_io(address_map &map)
{
	map.unmap_value_high();
	map(0x50, 0x50).r(this, FUNC(bbc_tube_80286_device::disable_boot_rom));
	map(0x60, 0x60).w(this, FUNC(bbc_tube_80286_device::irq_latch_w));
	map(0x80, 0x8f).rw("ula", FUNC(tube_device::parasite_r), FUNC(tube_device::parasite_w)).umask16(0x00ff);
}

//-------------------------------------------------
//  ROM( tube_80286 )
//-------------------------------------------------

ROM_START( tube_80286 )
	ROM_REGION(0x4000, "bootstrap", 0)
	ROM_LOAD16_BYTE("m512_lo.ic31", 0x0000, 0x2000, CRC(c0df8707) SHA1(7f6d843d5aea6bdb36cbd4623ae942b16b96069d)) // 2201,287-02
	ROM_LOAD16_BYTE("m512_hi.ic32", 0x0001, 0x2000, CRC(e47f10b2) SHA1(45dc8d7e7936afbec6de423569d9005a1c350316)) // 2201,288-02
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(bbc_tube_80286_device::device_add_mconfig)
	MCFG_DEVICE_ADD("i80286", I80286, XTAL(12'000'000) / 2)
	MCFG_DEVICE_PROGRAM_MAP(tube_80286_mem)
	MCFG_DEVICE_IO_MAP(tube_80286_io)
	MCFG_DEVICE_IRQ_ACKNOWLEDGE_DEVICE(DEVICE_SELF, bbc_tube_80286_device, irq_callback)

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
