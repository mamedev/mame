// license:BSD-3-Clause
// copyright-holders:Carl
#include "m20_8086.h"
#include "machine/ram.h"

const device_type M20_8086 = &device_creator<m20_8086_device>;

m20_8086_device::m20_8086_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, M20_8086, "Olivetti M20 8086 Adapter", tag, owner, clock, "m20_8086", __FILE__),
	m_8086(*this, "8086"),
	m_maincpu(*this, ":maincpu"),
	m_pic(*this, ":i8259"),
	m_8086_halt(true)
{
}

void m20_8086_device::device_start()
{
	UINT8* ram = machine().device<ram_device>("ram")->pointer();
	m_8086->space(AS_PROGRAM).install_readwrite_bank(0x00000,  machine().device<ram_device>("ram")->size() - 0x4001, "mainram");
	membank("highram")->set_base(ram);
	membank("mainram")->set_base(&ram[0x4000]);
	membank("vram")->set_base(memshare(":p_videoram")->ptr());
	membank("vram2")->set_base(memshare(":p_videoram")->ptr());
}

void m20_8086_device::device_reset()
{
	m_8086_halt = true;
	m_nvi = m_vi = 0;
	m_8086->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

ROM_START( m20_8086 )
	ROM_REGION(0x4000, "8086", 0)
	ROM_LOAD("apb-1086-2.0.bin", 0x0000, 0x4000, CRC(8c05be93) SHA1(2bb424afd874cc6562e9642780eaac2391308053))
ROM_END


const rom_entry *m20_8086_device::device_rom_region() const
{
	return ROM_NAME( m20_8086 );
}

static ADDRESS_MAP_START(i86_prog, AS_PROGRAM, 16, m20_8086_device)
	AM_RANGE(0xe0000, 0xe3fff) AM_RAMBANK("vram2")
	AM_RANGE(0xf0000, 0xf3fff) AM_RAMBANK("highram")
	AM_RANGE(0xf4000, 0xf7fff) AM_RAMBANK("vram")
	AM_RANGE(0xfc000, 0xfffff) AM_ROM AM_REGION("8086",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(i86_io, AS_IO, 16, m20_8086_device)
	AM_RANGE(0x4000, 0x4fff) AM_READWRITE(z8000_io_r, z8000_io_w)
	AM_RANGE(0x7ffa, 0x7ffd) AM_WRITE(handshake_w)
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( m20_8086 )
	MCFG_CPU_ADD("8086", I8086, XTAL_24MHz/3)
	MCFG_CPU_PROGRAM_MAP(i86_prog)
	MCFG_CPU_IO_MAP(i86_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(m20_8086_device, int_cb)
MACHINE_CONFIG_END

machine_config_constructor m20_8086_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( m20_8086 );
}

READ16_MEMBER(m20_8086_device::z8000_io_r)
{
	return m_maincpu->space(AS_IO).read_word(offset << 1, mem_mask);
}

WRITE16_MEMBER(m20_8086_device::z8000_io_w)
{
	m_maincpu->space(AS_IO).write_word(offset << 1, data, mem_mask);
}

IRQ_CALLBACK_MEMBER(m20_8086_device::int_cb)
{
	if(m_nvi)
	{
		m_nvi = false;
		m_8086->set_input_line(INPUT_LINE_IRQ0, m_vi ? ASSERT_LINE : CLEAR_LINE);
		return 0xff;
	}
	else
		return m_pic->acknowledge() << 1;
}

WRITE_LINE_MEMBER(m20_8086_device::nvi_w)
{
	m_nvi = state;
	m_8086->set_input_line(INPUT_LINE_IRQ0, (state || m_vi) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(m20_8086_device::vi_w)
{
	m_vi = state;
	m_8086->set_input_line(INPUT_LINE_IRQ0, (state || m_nvi) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE16_MEMBER(m20_8086_device::handshake_w)
{
	if(!offset)
	{
		m_8086->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		m_8086_halt = true;
	}
	else
	{
		m_8086->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_8086->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_8086_halt = false;
	}
}
