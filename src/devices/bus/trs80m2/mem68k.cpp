// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model 16/16B/6000 68000 memory board emulation

    128k/256k: Model 16 8000127, Model 16B AX-9238
    512k/1m:   Tandy 6000 8000229, AX-9007 (E1-E2 512K, E2-E3 1M)

    The board is linked to the 68000 CPU board by ribbon cables, not
    through the Z80 bus. S1 compares BA18-BA22 against the board base;
    switch 2 enables the board.

**********************************************************************/

#include "emu.h"
#include "mem68k.h"

#include <algorithm>


DEFINE_DEVICE_TYPE(TRS80M2_MEM128K, trs80m2_mem128k_device, "trs80m2_mem128k", "Tandy Model 16 68000 128K memory board")
DEFINE_DEVICE_TYPE(TRS80M2_MEM256K, trs80m2_mem256k_device, "trs80m2_mem256k", "Tandy Model 16 68000 256K memory board")
DEFINE_DEVICE_TYPE(TRS80M2_MEM512K, trs80m2_mem512k_device, "trs80m2_mem512k", "Tandy 6000 68000 512K memory board")
DEFINE_DEVICE_TYPE(TRS80M2_MEM1M, trs80m2_mem1m_device, "trs80m2_mem1m", "Tandy 6000 68000 1M memory board")


static INPUT_PORTS_START( trs80m2_mem68k )
	PORT_START("S1")
	PORT_DIPNAME( 0x02, 0x02, "Board enable" ) PORT_DIPLOCATION("S1:2")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "BA18" ) PORT_DIPLOCATION("S1:3")
	PORT_DIPSETTING( 0x00, "0" )
	PORT_DIPSETTING( 0x04, "1" )
	PORT_DIPNAME( 0x10, 0x00, "BA19" ) PORT_DIPLOCATION("S1:5")
	PORT_DIPSETTING( 0x00, "0" )
	PORT_DIPSETTING( 0x10, "1" )
	PORT_DIPNAME( 0x20, 0x00, "BA22" ) PORT_DIPLOCATION("S1:6")
	PORT_DIPSETTING( 0x00, "0" )
	PORT_DIPSETTING( 0x20, "1" )
	PORT_DIPNAME( 0x40, 0x00, "BA20" ) PORT_DIPLOCATION("S1:7")
	PORT_DIPSETTING( 0x00, "0" )
	PORT_DIPSETTING( 0x40, "1" )
	PORT_DIPNAME( 0x80, 0x00, "BA21" ) PORT_DIPLOCATION("S1:8")
	PORT_DIPSETTING( 0x00, "0" )
	PORT_DIPSETTING( 0x80, "1" )
INPUT_PORTS_END

static INPUT_PORTS_START( trs80m2_mem68k6k )
	PORT_START("S1")
	PORT_DIPNAME( 0x02, 0x02, "Board enable" ) PORT_DIPLOCATION("S1:2")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "BA19" ) PORT_DIPLOCATION("S1:5")
	PORT_DIPSETTING( 0x00, "0" )
	PORT_DIPSETTING( 0x10, "1" )
	PORT_DIPNAME( 0x20, 0x00, "BA22" ) PORT_DIPLOCATION("S1:6")
	PORT_DIPSETTING( 0x00, "0" )
	PORT_DIPSETTING( 0x20, "1" )
	PORT_DIPNAME( 0x40, 0x00, "BA20" ) PORT_DIPLOCATION("S1:7")
	PORT_DIPSETTING( 0x00, "0" )
	PORT_DIPSETTING( 0x40, "1" )
	PORT_DIPNAME( 0x80, 0x00, "BA21" ) PORT_DIPLOCATION("S1:8")
	PORT_DIPSETTING( 0x00, "0" )
	PORT_DIPSETTING( 0x80, "1" )
INPUT_PORTS_END

ioport_constructor trs80m2_mem68k_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( trs80m2_mem68k );
}

ioport_constructor trs80m2_mem68k_t6000_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( trs80m2_mem68k6k );
}


trs80m2_mem68k_device_base::trs80m2_mem68k_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, offs_t size) :
	device_t(mconfig, type, tag, owner, clock),
	device_trs80m2_card_interface(mconfig, *this),
	m_s1(*this, "S1"),
	m_size(size),
	m_ram(*this, "ram", size, ENDIANNESS_BIG)
{
}

trs80m2_mem68k_device::trs80m2_mem68k_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, offs_t size) :
	trs80m2_mem68k_device_base(mconfig, type, tag, owner, clock, size)
{
}

trs80m2_mem128k_device::trs80m2_mem128k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	trs80m2_mem68k_device(mconfig, TRS80M2_MEM128K, tag, owner, clock, 0x20000)
{
}

trs80m2_mem256k_device::trs80m2_mem256k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	trs80m2_mem68k_device(mconfig, TRS80M2_MEM256K, tag, owner, clock, 0x40000)
{
}

trs80m2_mem68k_t6000_device::trs80m2_mem68k_t6000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, offs_t size) :
	trs80m2_mem68k_device_base(mconfig, type, tag, owner, clock, size)
{
}

trs80m2_mem512k_device::trs80m2_mem512k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	trs80m2_mem68k_t6000_device(mconfig, TRS80M2_MEM512K, tag, owner, clock, 0x80000)
{
}

trs80m2_mem1m_device::trs80m2_mem1m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	trs80m2_mem68k_t6000_device(mconfig, TRS80M2_MEM1M, tag, owner, clock, 0x100000)
{
}

offs_t trs80m2_mem68k_device::base()
{
	uint8_t const s1 = m_s1->read();

	return (BIT(s1, 2) << 18) | (BIT(s1, 4) << 19) | (BIT(s1, 6) << 20) | (BIT(s1, 7) << 21) | (BIT(s1, 5) << 22);
}

offs_t trs80m2_mem68k_t6000_device::base()
{
	uint8_t const s1 = m_s1->read();
	offs_t const ba19 = (m_size == 0x80000) ? (BIT(s1, 4) << 19) : 0;

	return ba19 | (BIT(s1, 6) << 20) | (BIT(s1, 7) << 21) | (BIT(s1, 5) << 22);
}

void trs80m2_mem68k_device_base::device_start()
{
	std::fill_n(&m_ram[0], m_size / 2, 0xffff);
}

void trs80m2_mem68k_device_base::m68k_map(address_space &space)
{
	if (!BIT(m_s1->read(), 1))
		return;

	offs_t const start = base();
	offs_t const end = start + m_size - 1;

	space.install_ram(start, end, 0x800000, &m_ram[0]);
}
