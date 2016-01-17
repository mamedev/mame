// license:BSD-3-Clause
// copyright-holders:Carl
#include "m24_z8000.h"

const device_type M24_Z8000 = &device_creator<m24_z8000_device>;

m24_z8000_device::m24_z8000_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, M24_Z8000, "Olivetti M24 Z8000 Adapter", tag, owner, clock, "m24_z8000", __FILE__),
	m_z8000(*this, "z8000"),
	m_maincpu(*this, ":maincpu"),
	m_pic(*this, ":mb:pic8259"),
	m_halt_out(*this),
	m_z8000_halt(true)
{
}

void m24_z8000_device::device_start()
{
	m_halt_out.resolve_safe();
}

void m24_z8000_device::device_reset()
{
	m_z8000_halt = true;
	m_z8000_mem = false;
	m_timer_irq = false;
	m_irq = 0;
	m_z8000->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

ROM_START( m24_z8000 )
	ROM_REGION(0x4000, "z8000", 0)
	ROM_LOAD("m24apb.bin", 0x0000, 0x4000, CRC(3b3d2895) SHA1(ff048cf61b090b147be7e29a929a0be7b3ac8409))
ROM_END


const rom_entry *m24_z8000_device::device_rom_region() const
{
	return ROM_NAME( m24_z8000 );
}

static ADDRESS_MAP_START(z8000_prog, AS_PROGRAM, 16, m24_z8000_device)
	AM_RANGE(0x40000, 0x43fff) AM_ROM AM_REGION("z8000", 0)
	AM_RANGE(0x50000, 0x53fff) AM_ROM AM_REGION("z8000", 0)
	AM_RANGE(0x70000, 0x73fff) AM_ROM AM_REGION("z8000", 0)
	AM_RANGE(0x00000, 0xfffff) AM_READWRITE(pmem_r, pmem_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(z8000_data, AS_DATA, 16, m24_z8000_device)
	AM_RANGE(0x40000, 0x43fff) AM_ROM AM_REGION("z8000", 0)
	AM_RANGE(0x70000, 0x73fff) AM_ROM AM_REGION("z8000", 0)
	AM_RANGE(0x00000, 0xfffff) AM_READWRITE(dmem_r, dmem_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(z8000_io, AS_IO, 16, m24_z8000_device)
	AM_RANGE(0x0080, 0x0081) AM_WRITE8(irqctl_w, 0x00ff)
	AM_RANGE(0x00a0, 0x00a1) AM_WRITE8(serctl_w, 0x00ff)
	AM_RANGE(0x00c0, 0x00c1) AM_DEVREADWRITE8("i8251", i8251_device, data_r, data_w, 0x00ff)
	AM_RANGE(0x00c2, 0x00c3) AM_DEVREADWRITE8("i8251", i8251_device, status_r, control_w, 0x00ff)
	AM_RANGE(0x0120, 0x0127) AM_DEVREADWRITE8("pit8253", pit8253_device, read, write, 0x00ff)
	AM_RANGE(0x80c0, 0x80c1) AM_READWRITE8(handshake_r, handshake_w, 0x00ff)
	AM_RANGE(0x8000, 0x83ff) AM_READWRITE(i86_io_r, i86_io_w)
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( m24_z8000 )
	MCFG_CPU_ADD("z8000", Z8001, XTAL_8MHz/2)
	MCFG_CPU_PROGRAM_MAP(z8000_prog)
	MCFG_CPU_DATA_MAP(z8000_data)
	MCFG_CPU_IO_MAP(z8000_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(m24_z8000_device, int_cb)
	MCFG_Z8000_MO(WRITELINE(m24_z8000_device, mo_w))

	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
	MCFG_PIT8253_CLK0(19660000/15)
	MCFG_PIT8253_OUT0_HANDLER(NULL) //8251
	MCFG_PIT8253_CLK1(19660000/15)
	MCFG_PIT8253_OUT1_HANDLER(NULL)
	MCFG_PIT8253_CLK2(19660000/15)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(m24_z8000_device, timer_irq_w))

	MCFG_DEVICE_ADD("i8251", I8251, 0)
MACHINE_CONFIG_END

machine_config_constructor m24_z8000_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( m24_z8000 );
}

const UINT8 m24_z8000_device::pmem_table[16][4] =
	{{0, 1, 2, 3}, {1, 2, 3, 255}, {4, 5, 6, 7}, {46, 40, 41, 42},
	{255, 255, 255, 255}, {255, 255, 255, 47}, {1, 2, 3, 255}, {255, 255, 255, 255},
	{1, 2, 8, 9}, {5, 6, 10, 11}, {1, 2, 8, 9}, {12, 13, 14, 15},
	{16, 17, 18, 19}, {20, 21, 22, 23}, {24, 25, 26, 27}, {28, 29, 30, 31}};

READ16_MEMBER(m24_z8000_device::pmem_r)
{
	UINT16 ret;
	UINT8 hostseg;
	offset <<= 1;
	if(!m_z8000_mem)
		return memregion(subtag("z8000").c_str())->u16(offset >> 1);

	hostseg = pmem_table[(offset >> 16) & 0xf][(offset >> 14) & 3];
	if(hostseg == 255)
		return 0;
	offset = (offset & 0x3fff) | (hostseg << 14);
	if((hostseg >= 40) && (hostseg <= 47))
		offset = (offset & 0xf0000) | BITSWAP16(offset,15,7,6,14,13,12,11,10,9,8,5,4,3,2,1,0); // move A6/A7 so CGA framebuffer appears linear
	ret = m_maincpu->space(AS_PROGRAM).read_word(offset, (mem_mask << 8) | (mem_mask >> 8));
	return (ret << 8) | (ret >> 8);
}

WRITE16_MEMBER(m24_z8000_device::pmem_w)
{
	UINT8 hostseg;
	data = (data << 8) | (data >> 8);
	offset <<= 1;
	hostseg = pmem_table[(offset >> 16) & 0xf][(offset >> 14) & 3];
	if(hostseg == 255)
		return;
	offset = (offset & 0x3fff) | (hostseg << 14);
	if((hostseg >= 40) && (hostseg <= 47))
		offset = (offset & 0xf0000) | BITSWAP16(offset,15,7,6,14,13,12,11,10,9,8,5,4,3,2,1,0);
	m_maincpu->space(AS_PROGRAM).write_word(offset, data, (mem_mask << 8) | (mem_mask >> 8));
}

const UINT8 m24_z8000_device::dmem_table[16][4] =
	{{0, 1, 2, 3}, {4, 5, 6, 7}, {4, 5, 6, 7}, {46, 40, 41, 42},
	{255, 255, 255, 255}, {1, 2, 3, 47}, {1, 2, 3, 255}, {255, 255, 255, 255},
	{5, 6, 10, 11}, {5, 6, 10, 11}, {1, 2, 8, 9}, {12, 13, 14, 15},
	{16, 17, 18, 19}, {20, 21, 22, 23}, {24, 25, 26, 27}, {28, 29, 30, 31}};

READ16_MEMBER(m24_z8000_device::dmem_r)
{
	UINT16 ret;
	UINT8 hostseg;
	offset <<= 1;
	hostseg = dmem_table[(offset >> 16) & 0xf][(offset >> 14) & 3];
	if(hostseg == 255)
		return 0;
	offset = (offset & 0x3fff) | (hostseg << 14);
	if((hostseg >= 40) && (hostseg <= 47))
		offset = (offset & 0xf0000) | BITSWAP16(offset,15,7,6,14,13,12,11,10,9,8,5,4,3,2,1,0);
	ret = m_maincpu->space(AS_PROGRAM).read_word(offset, (mem_mask << 8) | (mem_mask >> 8));
	return (ret << 8) | (ret >> 8);
}

WRITE16_MEMBER(m24_z8000_device::dmem_w)
{
	UINT8 hostseg;
	data = (data << 8) | (data >> 8);
	offset <<= 1;
	hostseg = dmem_table[(offset >> 16) & 0xf][(offset >> 14) & 3];
	if(hostseg == 255)
		return;
	offset = (offset & 0x3fff) | (hostseg << 14);
	if((hostseg >= 40) && (hostseg <= 47))
		offset = (offset & 0xf0000) | BITSWAP16(offset,15,7,6,14,13,12,11,10,9,8,5,4,3,2,1,0);
	m_maincpu->space(AS_PROGRAM).write_word(offset, data, (mem_mask << 8) | (mem_mask >> 8));
}

READ16_MEMBER(m24_z8000_device::i86_io_r)
{
	UINT16 ret = m_maincpu->space(AS_IO).read_word(offset << 1, (mem_mask << 8) | (mem_mask >> 8));
	return (ret << 8) | (ret >> 8);
}

WRITE16_MEMBER(m24_z8000_device::i86_io_w)
{
	data = (data << 8) | (data >> 8);
	m_maincpu->space(AS_IO).write_word(offset << 1, data, (mem_mask << 8) | (mem_mask >> 8));
}

WRITE8_MEMBER(m24_z8000_device::irqctl_w)
{
	m_irq = data;
}

WRITE8_MEMBER(m24_z8000_device::serctl_w)
{
	m_z8000_mem = (data & 0x20) ? true : false;
}

IRQ_CALLBACK_MEMBER(m24_z8000_device::int_cb)
{
	if (!irqline)
	{
		m_z8000->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
		return 0xff; // NVI, value ignored
	}
	else
		return m_pic->acknowledge();
}

READ8_MEMBER(m24_z8000_device::handshake_r)
{
	return 0;
}

WRITE8_MEMBER(m24_z8000_device::handshake_w)
{
	m_handshake = data;
	if(data & 1)
	{
		m_z8000->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_z8000->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
		m_z8000->mi_w(CLEAR_LINE);
		m_z8000_halt = false;
	}
	else
	{
		m_z8000->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_z8000_halt = true;
		m_z8000_mem = false;
		m_halt_out(CLEAR_LINE);
	}
}

WRITE_LINE_MEMBER(m24_z8000_device::mo_w)
{
	m_z8000->mi_w(state ? ASSERT_LINE : CLEAR_LINE);
	m_halt_out(state);
}

WRITE_LINE_MEMBER(m24_z8000_device::timer_irq_w)
{
	m_timer_irq = state ? true : false;
	m_z8000->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
}
