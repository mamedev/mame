// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6502.c

    6502, NES variant

***************************************************************************/

#include "emu.h"
#include "n2a03.h"

const device_type N2A03 = &device_creator<n2a03_device>;

READ8_MEMBER(n2a03_device::psg1_4014_r)
{
	return m_apu->read(space, 0x14);
}

READ8_MEMBER(n2a03_device::psg1_4015_r)
{
	return m_apu->read(space, 0x15);
}

WRITE8_MEMBER(n2a03_device::psg1_4015_w)
{
	m_apu->write(space, 0x15, data);
}

WRITE8_MEMBER(n2a03_device::psg1_4017_w)
{
	m_apu->write(space, 0x17, data);
}


// on various drivers          output port 0x4014 is used for external hardware   (not used by APU?)
//                       input/output port 0x4016  ^                              (not used by APU?)
//                       input        port 0x4017  ^                              ( APU_IRQCTRL )
// is there a fall through where every write is seen by other hw, or do these addresses really not touch the APU?? APU_IRQCTRL can definitely be written by can it be read back?

static ADDRESS_MAP_START( n2a03_map, AS_PROGRAM, 8, n2a03_device )
	AM_RANGE(0x4000, 0x4013) AM_DEVREADWRITE("nesapu", nesapu_device, read, write)
	AM_RANGE(0x4014, 0x4014) AM_READ(psg1_4014_r) // AM_WRITE(sprite_dma_0_w)
	AM_RANGE(0x4015, 0x4015) AM_READWRITE(psg1_4015_r, psg1_4015_w) /* PSG status / first control register */
	//AM_RANGE(0x4016, 0x4016) AM_READWRITE(vsnes_in0_r, vsnes_in0_w)
	AM_RANGE(0x4017, 0x4017) /*AM_READ(vsnes_in1_r)*/ AM_WRITE(psg1_4017_w)
ADDRESS_MAP_END



n2a03_device::n2a03_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	m6502_device(mconfig, N2A03, "N2A03", tag, owner, clock, "n2a03", __FILE__),
	m_apu(*this, "nesapu"),
	m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0, ADDRESS_MAP_NAME(n2a03_map))
{
}

offs_t n2a03_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	return disassemble_generic(buffer, pc, oprom, opram, options, disasm_entries);
}

void n2a03_device::device_start()
{
	if(!m_apu->started())
		throw device_missing_dependencies();

	if(direct_disabled)
		mintf = new mi_2a03_nd;
	else
		mintf = new mi_2a03_normal;

	m_apu->set_tag_memory(tag());

	init();
}

UINT8 n2a03_device::mi_2a03_normal::read(UINT16 adr)
{
	return program->read_byte(adr);
}

UINT8 n2a03_device::mi_2a03_normal::read_sync(UINT16 adr)
{
	return sdirect->read_byte(adr);
}

UINT8 n2a03_device::mi_2a03_normal::read_arg(UINT16 adr)
{
	return direct->read_byte(adr);
}

void n2a03_device::mi_2a03_normal::write(UINT16 adr, UINT8 val)
{
	program->write_byte(adr, val);
}

UINT8 n2a03_device::mi_2a03_nd::read(UINT16 adr)
{
	return program->read_byte(adr);
}

UINT8 n2a03_device::mi_2a03_nd::read_sync(UINT16 adr)
{
	return sprogram->read_byte(adr);
}

UINT8 n2a03_device::mi_2a03_nd::read_arg(UINT16 adr)
{
	return program->read_byte(adr);
}

void n2a03_device::mi_2a03_nd::write(UINT16 adr, UINT8 val)
{
	program->write_byte(adr, val);
}

const address_space_config *n2a03_device::memory_space_config(address_spacenum spacenum) const
{
	switch(spacenum)
	{
	case AS_PROGRAM:           return &m_program_config;
	case AS_DECRYPTED_OPCODES: return has_configured_map(AS_DECRYPTED_OPCODES) ? &sprogram_config : nullptr;
	default:                   return nullptr;
	}
}

static MACHINE_CONFIG_FRAGMENT( n2a03_device )
	MCFG_SOUND_ADD("nesapu", NES_APU, DERIVED_CLOCK(1,1) )

	MCFG_SOUND_ROUTE(ALL_OUTPUTS, ":mono", 0.50)

MACHINE_CONFIG_END

machine_config_constructor n2a03_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( n2a03_device );
}


#include "cpu/m6502/n2a03.inc"
