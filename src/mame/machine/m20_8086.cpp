// license:BSD-3-Clause
// copyright-holders:Carl
#include "emu.h"
#include "m20_8086.h"

DEFINE_DEVICE_TYPE(M20_8086, m20_8086_device, "m20_8086", "Olivetti M20 8086 Adapter")

m20_8086_device::m20_8086_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, M20_8086, tag, owner, clock),
	m_8086(*this, "8086"),
	m_maincpu(*this, finder_base::DUMMY_TAG),
	m_pic(*this, finder_base::DUMMY_TAG),
	m_ram(*this, finder_base::DUMMY_TAG),
	m_8086_halt(true)
{
}

void m20_8086_device::device_start()
{
	uint8_t* ram = m_ram->pointer();
	m_8086->space(AS_PROGRAM).install_readwrite_bank(0x00000,  m_ram->size() - 0x4001, "mainram");
	membank("highram")->set_base(ram);
	membank("mainram")->set_base(&ram[0x4000]);
	membank("vram")->set_base(memshare(":videoram")->ptr());
	membank("vram2")->set_base(memshare(":videoram")->ptr());
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


const tiny_rom_entry *m20_8086_device::device_rom_region() const
{
	return ROM_NAME( m20_8086 );
}

void m20_8086_device::i86_prog(address_map &map)
{
	map(0xe0000, 0xe3fff).bankrw("vram2");
	map(0xf0000, 0xf3fff).bankrw("highram");
	map(0xf4000, 0xf7fff).bankrw("vram");
	map(0xfc000, 0xfffff).rom().region("8086", 0);
}

void m20_8086_device::i86_io(address_map &map)
{
	map(0x4000, 0x4fff).rw(FUNC(m20_8086_device::z8000_io_r), FUNC(m20_8086_device::z8000_io_w));
	map(0x7ffa, 0x7ffd).w(FUNC(m20_8086_device::handshake_w));
}

void m20_8086_device::device_add_mconfig(machine_config &config)
{
	I8086(config, m_8086, XTAL(24'000'000)/3);
	m_8086->set_addrmap(AS_PROGRAM, &m20_8086_device::i86_prog);
	m_8086->set_addrmap(AS_IO, &m20_8086_device::i86_io);
	m_8086->set_irq_acknowledge_callback(FUNC(m20_8086_device::int_cb));
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
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		m_8086_halt = true;
	}
	else
	{
		m_8086->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_8086->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_8086_halt = false;
	}
}
