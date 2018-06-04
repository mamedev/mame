// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn ANC04 Z80 2nd processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ANC04_Z802ndproc.html

**********************************************************************/


#include "emu.h"
#include "tube_z80.h"
#include "softlist_dev.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_TUBE_Z80, bbc_tube_z80_device, "bbc_tube_z80", "Acorn Z80 2nd Processor")


//-------------------------------------------------
//  ADDRESS_MAP( tube_z80_mem )
//-------------------------------------------------

void bbc_tube_z80_device::tube_z80_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(this, FUNC(bbc_tube_z80_device::mem_r), FUNC(bbc_tube_z80_device::mem_w));
}

//-------------------------------------------------
//  ADDRESS_MAP( tube_z80_fetch )
//-------------------------------------------------

void bbc_tube_z80_device::tube_z80_fetch(address_map &map)
{
	map(0x000, 0xffff).r(this, FUNC(bbc_tube_z80_device::opcode_r));
}

//-------------------------------------------------
//  ADDRESS_MAP( tube_z80_io )
//-------------------------------------------------

void bbc_tube_z80_device::tube_z80_io(address_map &map)
{
	map(0x00, 0x07).mirror(0xff00).rw("ula", FUNC(tube_device::parasite_r), FUNC(tube_device::parasite_w));
}

//-------------------------------------------------
//  ROM( tube_z80 )
//-------------------------------------------------

ROM_START( tube_z80 )
	ROM_REGION(0x1000, "rom", 0)
	ROM_LOAD("z80_120.rom", 0x0000, 0x1000, CRC(315bfc20) SHA1(069077df498599a9c880d4ec9f4bc53fcc602d82))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(bbc_tube_z80_device::device_add_mconfig)
	MCFG_DEVICE_ADD("z80", Z80, XTAL(12'000'000) / 2)
	MCFG_DEVICE_PROGRAM_MAP(tube_z80_mem)
	MCFG_DEVICE_OPCODES_MAP(tube_z80_fetch)
	MCFG_DEVICE_IO_MAP(tube_z80_io)
	MCFG_DEVICE_IRQ_ACKNOWLEDGE_DEVICE(DEVICE_SELF, bbc_tube_z80_device, irq_callback)

	MCFG_TUBE_ADD("ula")
	MCFG_TUBE_HIRQ_HANDLER(WRITELINE(DEVICE_SELF_OWNER, bbc_tube_slot_device, irq_w))
	MCFG_TUBE_PNMI_HANDLER(WRITELINE(*this, bbc_tube_z80_device, nmi_w))
	MCFG_TUBE_PIRQ_HANDLER(INPUTLINE("z80", INPUT_LINE_IRQ0))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
	MCFG_RAM_DEFAULT_VALUE(0x00)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("flop_ls_z80", "bbc_flop_z80")
MACHINE_CONFIG_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_tube_z80_device::device_rom_region() const
{
	return ROM_NAME( tube_z80 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_tube_z80_device - constructor
//-------------------------------------------------

bbc_tube_z80_device::bbc_tube_z80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_TUBE_Z80, tag, owner, clock),
		device_bbc_tube_interface(mconfig, *this),
		m_z80(*this, "z80"),
		m_ula(*this, "ula"),
		m_ram(*this, "ram"),
		m_rom(*this, "rom"),
		m_rom_enabled(true)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_z80_device::device_start()
{
	m_slot = dynamic_cast<bbc_tube_slot_device *>(owner());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_tube_z80_device::device_reset()
{
	m_ula->reset();

	m_rom_enabled = true;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(bbc_tube_z80_device::host_r)
{
	return m_ula->host_r(space, offset);
}

WRITE8_MEMBER(bbc_tube_z80_device::host_w)
{
	m_ula->host_w(space, offset, data);
}


READ8_MEMBER(bbc_tube_z80_device::opcode_r)
{
	if (!machine().side_effects_disabled())
	{
		if (offset == 0x0066 && m_z80->input_state(INPUT_LINE_NMI))
			m_rom_enabled = true;
		else if (offset >= 0x8000)
			m_rom_enabled = false;
	}
	return m_z80->space(AS_PROGRAM).read_byte(offset);
}


READ8_MEMBER(bbc_tube_z80_device::mem_r)
{
	uint8_t data;

	if (m_rom_enabled && (offset < 0x1000))
		data = m_rom->base()[offset & 0xfff];
	else
		data = m_ram->pointer()[offset];

	return data;
}

WRITE8_MEMBER(bbc_tube_z80_device::mem_w)
{
	m_ram->pointer()[offset] = data;
}

WRITE_LINE_MEMBER(bbc_tube_z80_device::nmi_w)
{
	m_z80->set_input_line(INPUT_LINE_NMI, state);
}


//-------------------------------------------------
//  irq vector callback
//-------------------------------------------------

IRQ_CALLBACK_MEMBER(bbc_tube_z80_device::irq_callback)
{
	return 0xfe;
}
