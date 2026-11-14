// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model 16/16B/6000 68000 CPU board emulation

    m16:   6 MHz long board 8709235 (68000 at U22), stock PALs:
           CONT0 swaps the bytes of the Z80 window, no burst mode.
    m16b:  6 MHz short board 8000181 (AX-9416) with the 12/16B:44
           XENIX 3 upgrade: CONT0 is BURST, 7800D6/D7 drive USEIRQ*.
    t6000: 8 MHz board 8000247 (AX-9006): as m16b.

    Z80 ports DE (control latch) and DF (window address A22-A15).
    The Z80 window is 8000-BFFF, enabled while CONT1 is clear.

    68000 I/O at 7800D0-7800D7: AM9519 (D0/D1), MMU set 1 limit/offset
    (D2/D3), set 0 limit/offset (D4/D5), USEIRQ* (D6/D7).

    The DE latch (MC3482A) has no clear input and the board has no
    power-on reset, so the 68000 HALT* and RESET* lines are undefined
    until software first writes DE; the boot ROM never does. The latch
    powers up here with HALT set. With HALT clear the 68000 fetches an
    odd stack pointer (FFFFFFFF) and takes an address error while
    stacking it. A real 68000 halts on that double fault, but the MAME
    core re-enters address error processing forever, which wastes host
    time while the 68000 is unused.

**********************************************************************/

#include "emu.h"
#include "m68k.h"

#include <algorithm>


DEFINE_DEVICE_TYPE(TRS80M2_M68K_M16, trs80m2_m68k_m16_device, "trs80m2_m68k_m16", "Tandy Model 16 68000 CPU board")
DEFINE_DEVICE_TYPE(TRS80M2_M68K_M16B, trs80m2_m68k_m16b_device, "trs80m2_m68k_m16b", "Tandy Model 16B 68000 CPU board")
DEFINE_DEVICE_TYPE(TRS80M2_M68K_T6000, trs80m2_m68k_t6000_device, "trs80m2_m68k_t6000", "Tandy 6000 68000 CPU board")


trs80m2_m68k_device_base::trs80m2_m68k_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const XTAL &cpu_clock, bool burst, bool useirq) :
	device_t(mconfig, type, tag, owner, clock),
	device_trs80m2_card_interface(mconfig, *this),
	m_subcpu(*this, "m68k"),
	m_uic(*this, "am9519"),
	m_cpu_clock(cpu_clock),
	m_burst(burst),
	m_useirq(useirq),
	m_window_timer(nullptr),
	m_window_installed(false),
	m_m68k_space(nullptr),
	m_ual(0),
	m_tcl(0x04),
	m_z80_master(false)
{
}

trs80m2_m68k_m16_device::trs80m2_m68k_m16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	trs80m2_m68k_device_base(mconfig, TRS80M2_M68K_M16, tag, owner, clock, 24_MHz_XTAL / 4, false, false)
{
}

trs80m2_m68k_m16b_device::trs80m2_m68k_m16b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	trs80m2_m68k_device_base(mconfig, TRS80M2_M68K_M16B, tag, owner, clock, 24_MHz_XTAL / 4, true, true)
{
}

trs80m2_m68k_t6000_device::trs80m2_m68k_t6000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	trs80m2_m68k_device_base(mconfig, TRS80M2_M68K_T6000, tag, owner, clock, 48_MHz_XTAL / 6, true, true)
{
}

ROM_START( trs80m2_m68k_m16 )
	ROM_REGION( 0x340, "plds", 0 )
	ROM_LOAD( "pal16r6_7c60.u12", 0x000, 0x104, CRC(6dd0719d) SHA1(c6590b3fd22f6c61bf9936d065e98ad8325eb45a) )
	ROM_LOAD( "pal16r6_4c33.u34", 0x104, 0x104, NO_DUMP )
	ROM_LOAD( "pal16r8_88ac.u49", 0x208, 0x104, CRC(e211ef5c) SHA1(636a39bf06afe128422cca106ac24ecd49e6f959) )
	ROM_LOAD( "pal12l6_22ca.u57", 0x30c, 0x034, CRC(f0a2317c) SHA1(c4a02d90baa3a61118bcd3bd426819a394123a58) )
ROM_END

ROM_START( trs80m2_m68k_m16b )
	ROM_REGION( 0x208, "plds", 0 )
	ROM_LOAD( "pal16r6a_46d4.u36", 0x000, 0x104, CRC(65baa5d7) SHA1(363f87222e683388af22f79a058609aebbf2c7ac) )
	ROM_LOAD( "pal16r4.u48", 0x104, 0x104, CRC(c0eb3f2d) SHA1(f507448085f84d09a8667bdb8842c29895e09cad) )
ROM_END

ROM_START( trs80m2_m68k_t6000 )
	ROM_REGION( 0x504, "plds", 0 )
	ROM_LOAD( "pal16r6.u6", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16r6a_46d4.u36", 0x104, 0x104, CRC(65baa5d7) SHA1(363f87222e683388af22f79a058609aebbf2c7ac) )
	ROM_LOAD( "pal16l8a.u40", 0x208, 0x104, NO_DUMP )
	ROM_LOAD( "82s153.u45", 0x30c, 0x0eb, NO_DUMP )
	ROM_LOAD( "pal16r4.u48", 0x400, 0x104, NO_DUMP )
ROM_END

const tiny_rom_entry *trs80m2_m68k_m16_device::device_rom_region() const
{
	return ROM_NAME( trs80m2_m68k_m16 );
}

const tiny_rom_entry *trs80m2_m68k_m16b_device::device_rom_region() const
{
	return ROM_NAME( trs80m2_m68k_m16b );
}

const tiny_rom_entry *trs80m2_m68k_t6000_device::device_rom_region() const
{
	return ROM_NAME( trs80m2_m68k_t6000 );
}

void trs80m2_m68k_device_base::device_add_mconfig(machine_config &config)
{
	M68000(config, m_subcpu, m_cpu_clock);
	m_subcpu->set_addrmap(AS_PROGRAM, &trs80m2_m68k_device_base::m68000_mem);
	m_subcpu->set_addrmap(m68000_device::AS_USER_PROGRAM, &trs80m2_m68k_device_base::m68000_user_mem);
	m_subcpu->set_addrmap(m68000_device::AS_CPU_SPACE, &trs80m2_m68k_device_base::m68000_cpu_space);

	AM9519(config, m_uic);
	m_uic->out_int_callback().set_inputline(m_subcpu, M68K_IRQ_5);
}

void trs80m2_m68k_device_base::m68000_mem(address_map &map)
{
	map(0x000000, 0xffffff).rw(FUNC(trs80m2_m68k_device_base::timeout_r), FUNC(trs80m2_m68k_device_base::timeout_w));
	map(0x780000, 0x7fffff).mirror(0x800000).noprw();
	map(0x7800d0, 0x7800d1).mirror(0x87ff00).rw(FUNC(trs80m2_m68k_device_base::uic_r), FUNC(trs80m2_m68k_device_base::uic_w));
	map(0x7800d2, 0x7800d3).mirror(0x87ff00).lw8(NAME([this] (uint8_t data) { m_limit[1] = data; })).umask16(0xff00);
	map(0x7800d2, 0x7800d3).mirror(0x87ff00).lw8(NAME([this] (uint8_t data) { m_offset[1] = data; })).umask16(0x00ff);
	map(0x7800d4, 0x7800d5).mirror(0x87ff00).lw8(NAME([this] (uint8_t data) { m_limit[0] = data; })).umask16(0xff00);
	map(0x7800d4, 0x7800d5).mirror(0x87ff00).lw8(NAME([this] (uint8_t data) { m_offset[0] = data; })).umask16(0x00ff);

	if (m_useirq)
	{
		map(0x7800d6, 0x7800d7).mirror(0x87ff00).lrw16(
				NAME([this] () -> uint16_t { useirq_pulse(); return 0xffff; }),
				NAME([this] (uint16_t data) { useirq_pulse(); }));
	}
}

void trs80m2_m68k_device_base::m68000_user_mem(address_map &map)
{
	map(0x000000, 0xffffff).rw(FUNC(trs80m2_m68k_device_base::user_r), FUNC(trs80m2_m68k_device_base::user_w));
}

void trs80m2_m68k_device_base::m68000_cpu_space(address_map &map)
{
	map(0xfffffa, 0xfffffb).r(FUNC(trs80m2_m68k_device_base::iack_r));
}

void trs80m2_m68k_device_base::device_start()
{
	if (!m_subcpu->started() || !m_uic->started())
		throw device_missing_dependencies();

	m_m68k_space = &m_subcpu->space(AS_PROGRAM);
	m_m68k_space->specific(m_m68k_program);

	m_slot->bus().io().install_write_handler(0xde, 0xde, emu::rw_delegate(*this, FUNC(trs80m2_m68k_device_base::tcl_w)));
	m_slot->bus().io().install_write_handler(0xdf, 0xdf, emu::rw_delegate(*this, FUNC(trs80m2_m68k_device_base::ual_w)));

	m_window_timer = timer_alloc(FUNC(trs80m2_m68k_device_base::install_window), this);

	m_uic->ireq3_w(1);
	m_uic->ireq4_w(1);
	m_uic->ireq5_w(1);
	m_uic->ireq6_w(1);
	m_uic->ireq7_w(1);

	std::fill(std::begin(m_limit), std::end(m_limit), 0);
	std::fill(std::begin(m_offset), std::end(m_offset), 0);

	save_item(NAME(m_ual));
	save_item(NAME(m_tcl));
	save_item(NAME(m_limit));
	save_item(NAME(m_offset));
}

void trs80m2_m68k_device_base::device_reset()
{
	if (!m_window_installed)
	{
		m_window_timer->adjust(attotime::zero);
		remap_ram();
	}

	tcl_w(m_tcl & ~0x02);
}

TIMER_CALLBACK_MEMBER(trs80m2_m68k_device_base::install_window)
{
	if (m_window_installed)
		return;

	m_window_installed = true;

	trs80m2_bus_device &bus = m_slot->bus();

	for (u32 i = 0; i < trs80m2_bus_device::PAGE_ENTRIES; i++)
	{
		if (!bus.window_claimed(i))
		{
			bus.page_view(i).install_readwrite_handler(0x8000, 0xbfff,
					emu::rw_delegate(*this, FUNC(trs80m2_m68k_device_base::window_r)),
					emu::rw_delegate(*this, FUNC(trs80m2_m68k_device_base::window_w)));
		}
	}
}

void trs80m2_m68k_device_base::remap_ram()
{
	m_slot->bus().m68k_map(*m_m68k_space);
}

void trs80m2_m68k_device_base::tcl_w(uint8_t data)
{
	/*

	    bit     description

	    0       CONT0 (byte swap) / BURST
	    1       CONT1 (Z80 window disable)
	    2       HALT
	    3       RESET
	    4       CONT4
	    5       CONT5
	    6       CONT6
	    7       A14

	*/

	m_tcl = data;

	m_subcpu->set_input_line(INPUT_LINE_HALT, ((m_burst && BIT(data, 0)) || BIT(data, 2)) ? ASSERT_LINE : CLEAR_LINE);
	m_subcpu->set_input_line(INPUT_LINE_RESET, (BIT(data, 2) && BIT(data, 3)) ? ASSERT_LINE : CLEAR_LINE);

	m_uic->ireq0_w(BIT(data, 4));
	m_uic->ireq1_w(BIT(data, 5));
	m_uic->ireq2_w(BIT(data, 6));

	m_ual = (m_ual & 0x1fe) | BIT(data, 7);
}

void trs80m2_m68k_device_base::ual_w(uint8_t data)
{
	m_ual = (data << 1) | BIT(m_ual, 0);
}

offs_t trs80m2_m68k_device_base::z80_window_address(offs_t offset) const
{
	offs_t const address = (m_ual << 14) + (offset & 0x3fff);

	return (!m_burst && BIT(m_tcl, 0)) ? (address ^ 1) : address;
}

uint8_t trs80m2_m68k_device_base::window_r(offs_t offset)
{
	if (BIT(m_tcl, 1))
		return 0;

	m_z80_master = true;
	uint8_t const data = m_m68k_space->read_byte(z80_window_address(offset));
	m_z80_master = false;

	return data;
}

void trs80m2_m68k_device_base::window_w(offs_t offset, uint8_t data)
{
	if (BIT(m_tcl, 1))
		return;

	m_z80_master = true;
	m_m68k_space->write_byte(z80_window_address(offset), data);
	m_z80_master = false;
}

void trs80m2_m68k_device_base::useirq_pulse()
{
	if (machine().side_effects_disabled())
		return;

	m_slot->useirq_w(1);
	m_slot->useirq_w(0);
}

void trs80m2_m68k_device_base::pulse_ireq(int line)
{
	if (machine().side_effects_disabled())
		return;

	switch (line)
	{
	case 3: m_uic->ireq3_w(0); m_uic->ireq3_w(1); break;
	case 4: m_uic->ireq4_w(0); m_uic->ireq4_w(1); break;
	case 5: m_uic->ireq5_w(0); m_uic->ireq5_w(1); break;
	}
}

uint16_t trs80m2_m68k_device_base::timeout_r()
{
	if (m_z80_master)
	{
		pulse_ireq(5);
	}
	else if (!machine().side_effects_disabled())
	{
		pulse_ireq(4);
		m_subcpu->trigger_bus_error();
	}

	return 0xffff;
}

void trs80m2_m68k_device_base::timeout_w(uint16_t data)
{
	if (m_z80_master)
	{
		pulse_ireq(5);
	}
	else if (!machine().side_effects_disabled())
	{
		pulse_ireq(4);
		m_subcpu->trigger_bus_error();
	}
}

bool trs80m2_m68k_device_base::translate(offs_t &offset)
{
	int const set = BIT(offset, 22);
	uint32_t const page = ((offset >> 11) & 0xff) + m_offset[set];

	if (page > m_limit[set])
	{
		if (!machine().side_effects_disabled())
		{
			pulse_ireq(3);
			m_subcpu->trigger_bus_error();
		}
		return false;
	}

	offset = (offset & 0x7807ff) | (page << 11);
	return true;
}

uint16_t trs80m2_m68k_device_base::user_r(offs_t offset, uint16_t mem_mask)
{
	if (!translate(offset))
		return 0xffff;

	return m_m68k_program.read_word(offset << 1, mem_mask);
}

void trs80m2_m68k_device_base::user_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (translate(offset))
		m_m68k_program.write_word(offset << 1, data, mem_mask);
}

// the AM9519 sits on the low byte lane with C/D* driven by UDS*
uint16_t trs80m2_m68k_device_base::uic_r(offs_t offset, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
		return 0xff00 | m_uic->data_r();
	else
		return 0xff00 | m_uic->stat_r();
}

void trs80m2_m68k_device_base::uic_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
		m_uic->data_w(data & 0xff);
	else
		m_uic->cmd_w(data & 0xff);
}

uint16_t trs80m2_m68k_device_base::iack_r()
{
	if (machine().side_effects_disabled())
		return 0;

	return m_uic->iack_cb(*m_subcpu, M68K_IRQ_5) & 0xff;
}
