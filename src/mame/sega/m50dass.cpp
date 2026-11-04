// license:BSD-3-Clause
// copyright-holders:Tomás García-Merás Capote (ClawGrip)

/*******************************************************************************************

    Sega DASS (Dual Active Seat System) board, found on the Megalo 50 cabinet.

    The Megalo 50 holds up to three game boards (A, B and C) that the players choose with
    the select buttons. This board switches between them, keeps the credits, plays its own
    sounds, and drives the two moving seats of the deluxe cabinet. When a start button is
    pressed it sends coin pulses to the selected board. Games that support the seats drive
    them through their coin lockout outputs, one per seat. The seat motors are driven open
    loop, with no position feedback.

    Sega PCB 837-8683 (171-6259B):
     -Z0840004PSC Z80 CPU and 8 MHz oscillator (OSC1)
     -MB8464A-10LL or LH5268AD-10LL SRAM, backed by a supercap
     -NEC D71054C (8254)
     -Two Sega 315-5296 I/O chips
     -YM-2413
     -MB3773 watchdog and MB3771
     -3 banks of eight dip switches
     -15 relays, 3 x 74HC4066, TDA3810, TL064 and a 560-5207 transformer on each of the
      A, B and C sound inputs
    Boards with EPR-14705A and EPR-16665 also exist, both undumped.

    Credit display codes: E1 ROM, E2 RAM, E3-E4 I/O chips, E5 no game board, CL credits
    cleared (hold test and press service), CH check mode, FP free play.

    TODO:
    - CPU and PIT clock dividers unverified. The PIT one gives the 250 Hz tick that makes
      the music tempo sensible.
    - OSC2, next to the YM-2413, is unreadable in the photos: its frequency comes from the
      note table.
    - Only game board A is emulated. The game sound switching and the TDA3810 aren't.
    - First I/O chip port A d1, d3, d5: a signal from each board, sampled when it gets
      selected.
    - Board switching outputs: second I/O chip ports D and H, and port G d0-d3 and d6-d7.
    - NMI (turns everything off and halts, probably on power failure).

*******************************************************************************************/

#include "emu.h"
#include "m50dass.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"


DEFINE_DEVICE_TYPE(MEGALO50_DASS, m50dass_device, "m50dass", "Sega DASS")

m50dass_device::m50dass_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MEGALO50_DASS, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_pit(*this, "pit")
	, m_io(*this, "io%u", 0U)
	, m_ym2413(*this, "ym2413")
	, m_digits(*this, "digit%u", 0U)
	, m_start_lamps(*this, "start_lamp%u", 1U)
	, m_seat_motors(*this, "seat%u_motor", 1U)
	, m_p1_in_cb(*this, 0xff)
	, m_p2_in_cb(*this, 0xff)
	, m_seats(0xff)
{
}

void m50dass_device::device_start()
{
	save_item(NAME(m_seats));
}


/*******************************************************************************************
    Host interface
*******************************************************************************************/

template <u32 N>
void m50dass_device::seat_w(int state)
{
	// d0-d2 seat 1 of boards A-C, d3-d5 seat 2, pulled low by the coin lockout driver
	u8 const mask = 0x01 << (N * 3);
	m_seats = state ? (m_seats & ~mask) : (m_seats | mask);
}

template void m50dass_device::seat_w<0>(int state);
template void m50dass_device::seat_w<1>(int state);

u8 m50dass_device::board_inputs_r()
{
	return output_pins(1, 1, m_io[1]->debug_peek_output(1));
}

void m50dass_device::reset_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
}


/*******************************************************************************************
    Board I/O
*******************************************************************************************/

u8 m50dass_device::output_pins(int which, offs_t port, u8 data)
{
	// pulled up while the port is an input, which keeps the active low outputs off
	return BIT(m_io[which]->read(0x0f), port) ? data : 0xff;
}

void m50dass_device::io0_portc_w(offs_t offset, u8 data)
{
	// d2 is high while switching boards
	u8 const pins = output_pins(0, offset, data);
	m_seat_motors[0] = BIT(~pins, 4);
	m_seat_motors[1] = BIT(~pins, 5);

	machine().bookkeeping().coin_counter_w(0, BIT(~pins, 6));
	machine().bookkeeping().coin_counter_w(1, BIT(~pins, 7));
}

template <u32 N>
void m50dass_device::digit_w(offs_t offset, u8 data)
{
	m_digits[N] = bitswap<8>(~output_pins(0, offset, data), 0, 1, 2, 3, 4, 5, 6, 7);
}

void m50dass_device::io1_portg_w(offs_t offset, u8 data)
{
	u8 const pins = output_pins(1, offset, data);
	m_start_lamps[0] = BIT(~pins, 4);
	m_start_lamps[1] = BIT(~pins, 5);
}


/*******************************************************************************************
    Machine configuration
*******************************************************************************************/

void m50dass_device::program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xe000, 0xffff).ram().share("nvram");
}

void m50dass_device::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).nopw(); // MB3773 watchdog clock
	map(0x10, 0x13).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x20, 0x2f).rw(m_io[0], FUNC(sega_315_5296_device::read), FUNC(sega_315_5296_device::write));
	map(0x40, 0x4f).rw(m_io[1], FUNC(sega_315_5296_device::read), FUNC(sega_315_5296_device::write));
	map(0x60, 0x61).w(m_ym2413, FUNC(ym2413_device::write));
}

void m50dass_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_maincpu, 8_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &m50dass_device::program_map);
	m_maincpu->set_addrmap(AS_IO, &m50dass_device::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	PIT8254(config, m_pit);
	m_pit->set_clk<2>(8_MHz_XTAL / 4);
	m_pit->out_handler<2>().set(m_pit, FUNC(pit8254_device::write_clk1));
	m_pit->out_handler<1>().set_inputline(m_maincpu, INPUT_LINE_IRQ0).invert();

	SEGA_315_5296(config, m_io[0]);
	m_io[0]->in_pa_callback().set_constant(0xfe); // d0, d2, d4 low when board A, B, C is connected
	m_io[0]->in_pb_callback().set_ioport("IN1");
	m_io[0]->out_pc_callback().set(FUNC(m50dass_device::io0_portc_w));
	m_io[0]->out_pd_callback().set(FUNC(m50dass_device::digit_w<1>));
	m_io[0]->in_pe_callback().set_ioport("SW1");
	m_io[0]->in_pf_callback().set_ioport("SW2");
	m_io[0]->in_pg_callback().set_ioport("SW3");
	m_io[0]->out_ph_callback().set(FUNC(m50dass_device::digit_w<0>));

	SEGA_315_5296(config, m_io[1]);
	m_io[1]->in_pa_callback().set([this] () -> u8 { return m_seats; });
	m_io[1]->in_pc_callback().set([this] () -> u8 { return m_p1_in_cb(); });
	m_io[1]->in_pe_callback().set([this] () -> u8 { return m_p2_in_cb(); });
	m_io[1]->in_pf_callback().set_ioport("IN2");
	m_io[1]->out_pg_callback().set(FUNC(m50dass_device::io1_portg_w));

	YM2413(config, m_ym2413, 3'579'545);
	m_ym2413->add_route(ALL_OUTPUTS, *this, 1.0);
}


/*******************************************************************************************
    Inputs
*******************************************************************************************/

static INPUT_PORTS_START(m50dass)
	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_SERVICE1)
	PORT_SERVICE_NO_TOGGLE(0x08, IP_ACTIVE_LOW)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_SERVICE2) PORT_NAME("Check Mode") PORT_TOGGLE // coins, service and test go straight to the game
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN2")
	PORT_BIT(0x0b, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select Game C")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select Game A")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select Game B")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_START2)

	PORT_START("SW1")
	PORT_DIPNAME(0x0f, 0x0f, DEF_STR(Coinage)) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(   0x07, DEF_STR(4C_1C))
	PORT_DIPSETTING(   0x08, DEF_STR(3C_1C))
	PORT_DIPSETTING(   0x09, DEF_STR(2C_1C))
	PORT_DIPSETTING(   0x06, DEF_STR(2C_3C))
	PORT_DIPSETTING(   0x0f, DEF_STR(1C_1C))
	PORT_DIPSETTING(   0x0e, DEF_STR(1C_2C))
	PORT_DIPSETTING(   0x0d, DEF_STR(1C_3C))
	PORT_DIPSETTING(   0x0c, DEF_STR(1C_4C))
	PORT_DIPSETTING(   0x0b, DEF_STR(1C_5C))
	PORT_DIPSETTING(   0x0a, DEF_STR(1C_6C))
	PORT_DIPSETTING(   0x00, DEF_STR(Free_Play))
	PORT_DIPSETTING(   0x05, "No Credits") // coins are counted, but starting sends nothing
	PORT_DIPSETTING(   0x04, "No Credits")
	PORT_DIPSETTING(   0x03, "No Credits")
	PORT_DIPSETTING(   0x02, "No Credits")
	PORT_DIPSETTING(   0x01, "No Credits")
	PORT_DIPNAME(0x30, 0x30, "Game Coin Pulse") PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(   0x20, "100 ms")
	PORT_DIPSETTING(   0x30, "200 ms")
	PORT_DIPSETTING(   0x10, "300 ms")
	PORT_DIPSETTING(   0x00, "400 ms")
	PORT_DIPUNUSED_DIPLOC(0x40, IP_ACTIVE_LOW, "SW1:7")
	PORT_DIPUNUSED_DIPLOC(0x80, IP_ACTIVE_LOW, "SW1:8")

	PORT_START("SW2")
	PORT_DIPNAME(0x01, 0x01, "Board A Moving Seats") PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(   0x01, DEF_STR(Off))
	PORT_DIPSETTING(   0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, "Board B Moving Seats") PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(   0x02, DEF_STR(Off))
	PORT_DIPSETTING(   0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, "Board C Moving Seats") PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(   0x04, DEF_STR(Off))
	PORT_DIPSETTING(   0x00, DEF_STR(On))
	PORT_DIPNAME(0x18, 0x18, "Auto Game Change") PORT_DIPLOCATION("SW2:4,5") // with no credits and the controls left alone
	PORT_DIPSETTING(   0x18, DEF_STR(Off))
	PORT_DIPSETTING(   0x10, "After 2 Minutes")
	PORT_DIPSETTING(   0x08, "After 4 Minutes")
	PORT_DIPSETTING(   0x00, "After 6 Minutes")
	PORT_DIPNAME(0x20, 0x20, "Credit Limit") PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(   0x20, "9")
	PORT_DIPSETTING(   0x00, "24")
	PORT_DIPNAME(0x40, 0x40, "Coins") PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(   0x40, "Credit Counter")
	PORT_DIPSETTING(   0x00, "Straight to Game")
	// gives the game 9 credits, plays music and moves seat 1 with a player 1 control
	PORT_DIPNAME(0x80, 0x80, "Demo Mode") PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(   0x80, DEF_STR(Off))
	PORT_DIPSETTING(   0x00, DEF_STR(On))

	PORT_START("SW3")
	PORT_DIPUNUSED_DIPLOC(0x01, IP_ACTIVE_LOW, "SW3:1")
	PORT_DIPUNUSED_DIPLOC(0x02, IP_ACTIVE_LOW, "SW3:2")
	PORT_DIPUNUSED_DIPLOC(0x04, IP_ACTIVE_LOW, "SW3:3")
	PORT_DIPUNUSED_DIPLOC(0x08, IP_ACTIVE_LOW, "SW3:4")
	PORT_DIPUNUSED_DIPLOC(0x10, IP_ACTIVE_LOW, "SW3:5")
	PORT_DIPUNUSED_DIPLOC(0x20, IP_ACTIVE_LOW, "SW3:6")
	PORT_DIPUNUSED_DIPLOC(0x40, IP_ACTIVE_LOW, "SW3:7")
	PORT_DIPUNUSED_DIPLOC(0x80, IP_ACTIVE_LOW, "SW3:8")
INPUT_PORTS_END

ioport_constructor m50dass_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(m50dass);
}


/*******************************************************************************************
    ROM definition
*******************************************************************************************/

ROM_START(m50dass)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("epr-14705.ic18", 0x00000, 0x20000, CRC(b863d2c5) SHA1(f6aa309bc8be15c26d91f3cf048ea140a9ca12eb)) // Strings "MEGALO50  910322", "**  SHUNICHI  **" on ROM

	ROM_REGION(0x117, "pld", 0)
	ROM_LOAD("315-5592_gal16v8a.ic19", 0x000, 0x117, NO_DUMP)
ROM_END

const tiny_rom_entry *m50dass_device::device_rom_region() const
{
	return ROM_NAME(m50dass);
}
