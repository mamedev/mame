// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

-----------------------------------

  High Seas Havoc
   (c)1993 Data East

   This is an unusual Data East PCB based on the Sega Genesis / Megadrive.  The Game was also released on the home system.


Produttore  Data East
N.revisione CG-2

CPU
1x MC68000P8 (main)(12c)
1x Z8400B (sound)(1a)
1x custom SEGA315-5660-FC1004 (QFP208)(5d)
1x PIC16C55 (7a)
1x oscillator 53.6931MHz (osc1)

ROMs
2x TMS27C040 (25,26)(11a,9a)
2x PEEL18CV8 (4b,5b)

Note
1x JAMMA edge connector
1x trimmer (volume)(vr1)
2x 6 switches dip (dsw1,dsw2)
-----------------------------------
PCB markings:"DE-0407-2 MADE IN JAPAN"
-----------------------------------

Thanks to DOX, the "mystery chip" has been identified (and verified) as a PIC.
It's read protected, but has been decapped and read out.  It hangs off I/O port
B; see the link handlers below and the 'Protection' section in the comments
above init_hshavoc().


TODO: - what holds the 68000 off at start-up, and what drives the PIC's port C,
        are both guesses that happen to work (see 'Talking to the PIC');
      - which signal pulls i8 of the 5b PEEL low is not known; it follows
        whoever other than the 68000 owns the bus, so the regions those bus
        masters read are still listed by address (see 'Program ROM scrambling').
      See the comments above init_hshavoc().
*/


#include "emu.h"

#include "megadriv.h"

#include "cpu/m68000/m68000.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "sound/ymopn.h"


namespace {

class hshavoc_state : public md_ctrl_state
{
public:
	hshavoc_state(const machine_config &mconfig, device_type type, const char *tag) :
		md_ctrl_state(mconfig, type, tag),
		m_pic(*this, "pic"),
		m_coin(*this, "COIN"),
		m_player(*this, "P%u", 1U),
		m_dsw(*this, "DSW%u", 1U)
	{ }

	void hshavoc(machine_config &config) ATTR_COLD;

	void init_hshavoc() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<pic16c55_device> m_pic;

	required_ioport m_coin;
	required_ioport_array<2> m_player;
	required_ioport_array<2> m_dsw;

	uint8_t m_ctrl_data = 0xff;
	uint8_t m_ctrl_ctrl = 0x00;
	uint8_t m_exp_data = 0xff;
	uint8_t m_exp_ctrl = 0x00;

	uint8_t m_link_data = 0xff;
	uint8_t m_link_ctrl = 0x00;
	uint8_t m_pic_portb = 0xff;

	bool m_pic_ready = false;
	uint8_t m_pic_porta = 0xff;
	emu_timer *m_ready_timer = nullptr;

	uint8_t ctrl_data_r();
	void ctrl_data_w(uint8_t data) { m_ctrl_data = data; }
	uint8_t ctrl_ctrl_r() { return m_ctrl_ctrl; }
	void ctrl_ctrl_w(uint8_t data) { m_ctrl_ctrl = data & 0x7f; }
	uint8_t exp_data_r();
	void exp_data_w(uint8_t data) { m_exp_data = data; }
	uint8_t exp_ctrl_r() { return m_exp_ctrl; }
	void exp_ctrl_w(uint8_t data) { m_exp_ctrl = data & 0x7f; }

	uint8_t link_data_r();
	void link_data_w(uint8_t data);
	uint8_t link_ctrl_r() { return m_link_ctrl; }
	void link_ctrl_w(uint8_t data) { m_link_ctrl = data & 0x7f; }
	void pic_porta_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t pic_portb_r();
	void pic_portb_w(offs_t offset, uint8_t data, uint8_t mem_mask);

	TIMER_CALLBACK_MEMBER(pic_ready_check);

	void program_map(address_map &map) ATTR_COLD;
};


void hshavoc_state::machine_start()
{
	md_ctrl_state::machine_start();

	m_ready_timer = timer_alloc(FUNC(hshavoc_state::pic_ready_check), this);

	save_item(NAME(m_ctrl_data));
	save_item(NAME(m_ctrl_ctrl));
	save_item(NAME(m_exp_data));
	save_item(NAME(m_exp_ctrl));

	save_item(NAME(m_link_data));
	save_item(NAME(m_link_ctrl));
	save_item(NAME(m_pic_portb));
	save_item(NAME(m_pic_ready));
	save_item(NAME(m_pic_porta));
}

void hshavoc_state::machine_reset()
{
	md_ctrl_state::machine_reset();

	m_pic_ready = false;
	m_pic_porta = 0xff;
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}


/*
    The PIC is wired to I/O port B instead of a controller.  The game sets the
    port direction to 0x71 at 0x000dbe, giving:

        bit 6 (TH)  out   serial clock, counted by the PIC on T0CKI/TMR0, and
                          also the start strobe it polls on its port B bit 7
        bit 4       out   serial data   -> PIC port B bit 6
        bit 3       in    serial data   <- PIC port B bit 5

    The 68000 shifts "DATA EAST CORPORATION" out a bit at a time and latches the
    reply in the same loop; the PIC verifies the string in three chunks (8, 8 and
    5 bytes), restarts from scratch if any of them fails to match, and sends back
    "MX-32" if they all pass.  That exchange completes with no mismatches, so
    this wiring is confirmed rather than assumed.

    Bits 1 and 2 are read here as the coin inputs, but the PIC also drives its
    own port B bits 0 and 1 as outputs at 0x087 and 0x08a, and on the 68000 side
    only bits 1, 2, 3 and 7 are inputs.  Two PIC outputs plus two coin inputs do
    not fit in what is left, so one of the two assignments is probably wrong.
*/
uint8_t hshavoc_state::link_data_r()
{
	uint8_t input = m_coin->read() | 0x79;

	if (!BIT(m_pic_portb, 5))
		input &= ~0x08;

	return (m_link_data & m_link_ctrl) | (input & ~m_link_ctrl);
}

void hshavoc_state::link_data_w(uint8_t data)
{
	uint8_t const old = m_link_data;
	m_link_data = data;

	if (BIT(old, 6) != BIT(data, 6))
		m_pic->set_input_line(PIC16C5x_T0CKI, BIT(data, 6) ? ASSERT_LINE : CLEAR_LINE);
}

void hshavoc_state::pic_porta_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_pic_porta = (m_pic_porta & ~mem_mask) | (data & mem_mask);

	/* Port A bit 1 is taken to be the clear input of the PEEL at 4b (see 'What
	   the PEELs do' above init_hshavoc()).  The PIC holds it low
	   from 0x086 until 0x0d4, while the 68000 runs its boot code and the boot
	   handshake happens, so it marks the moment the 68000 has to start - but it
	   is also pulsed low for a single instruction at the top of the self test
	   at 0x000, so wait and see whether it stays low.  Bits 0 and 3 are taken
	   to select the descrambler setting; since every part of the ROM only ever
	   runs under one setting, init_hshavoc() applies them statically. */
	if (!m_pic_ready && !BIT(m_pic_porta, 1))
		m_ready_timer->adjust(attotime::from_usec(20));
}

TIMER_CALLBACK_MEMBER(hshavoc_state::pic_ready_check)
{
	if (!m_pic_ready && !BIT(m_pic_porta, 1))
	{
		m_pic_ready = true;
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	}
}

uint8_t hshavoc_state::pic_portb_r()
{
	uint8_t result = 0x3f;

	if (BIT(m_link_data, 4)) result |= 0x40;   // serial data from the 68000

	if (BIT(m_link_data, 6)) result |= 0x80;   // strobe, polled at PIC 0x08b

	return result;
}

void hshavoc_state::pic_portb_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_pic_portb = (m_pic_portb & ~mem_mask) | (data & mem_mask);
}

/*
    I/O port A carries the JAMMA controls as two banks of seven bits, selected
    by bit 0 of I/O port B.  Bit 6 of each bank is the start button for that
    player; the read routine at 0x6a6 shifts it up into bit 7.
*/
uint8_t hshavoc_state::ctrl_data_r()
{
	uint8_t const input = m_player[BIT(m_link_data, 0)]->read();

	return (m_ctrl_data & m_ctrl_ctrl) | (input & ~m_ctrl_ctrl);
}

// the two 6-switch DIP banks share the expansion port, selected by TH
uint8_t hshavoc_state::exp_data_r()
{
	uint8_t const input = m_dsw[BIT(m_exp_data, 6)]->read();

	return (m_exp_data & m_exp_ctrl) | (input & ~m_exp_ctrl);
}


void hshavoc_state::program_map(address_map &map)
{
	megadriv_68k_base_map(map);

	map(0x000000, 0x1fffff).rom();
	map(0x200000, 0x2023ff).ram();

	map(0xa10003, 0xa10003).rw(FUNC(hshavoc_state::ctrl_data_r), FUNC(hshavoc_state::ctrl_data_w));
	map(0xa10005, 0xa10005).rw(FUNC(hshavoc_state::link_data_r), FUNC(hshavoc_state::link_data_w));
	map(0xa10007, 0xa10007).rw(FUNC(hshavoc_state::exp_data_r), FUNC(hshavoc_state::exp_data_w));
	map(0xa10009, 0xa10009).rw(FUNC(hshavoc_state::ctrl_ctrl_r), FUNC(hshavoc_state::ctrl_ctrl_w));
	map(0xa1000b, 0xa1000b).rw(FUNC(hshavoc_state::link_ctrl_r), FUNC(hshavoc_state::link_ctrl_w));
	map(0xa1000d, 0xa1000d).rw(FUNC(hshavoc_state::exp_ctrl_r), FUNC(hshavoc_state::exp_ctrl_w));
}

INPUT_PORTS_START( hshavoc )
	PORT_START("COIN")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x08, 0x08, "Timer" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "30 Seconds" )
	PORT_DIPSETTING(    0x08, "60 Seconds" )
	PORT_DIPNAME( 0x10, 0x10, "Start Input" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "Start Buttons" )
	PORT_DIPSETTING(    0x00, "Button 1/2" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT(     0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) // only 6 switches

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, "Damage Taken" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( High ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Highest ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Controls ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Dual ) )
	PORT_BIT(     0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) // only 6 switches
INPUT_PORTS_END


void hshavoc_state::hshavoc(machine_config &config)
{
	md_ntsc(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &hshavoc_state::program_map);

	PIC16C55(config, m_pic, 4'000'000); // clock unknown
	m_pic->write_a().set(FUNC(hshavoc_state::pic_porta_w));
	m_pic->read_b().set(FUNC(hshavoc_state::pic_portb_r));
	m_pic->write_b().set(FUNC(hshavoc_state::pic_portb_w));
	// the PIC will not leave its start-up self test until reading its port C
	// 16320 times accumulates to exactly 0x80, which needs a value congruent
	// to 2 mod 4.  What actually drives that port, and through the four strobes
	// on port A, is unknown - this is a guess that satisfies the arithmetic.
	m_pic->read_c().set_constant(0x02);

	/* The 68000 and the PIC bit-bang a serial link.  The 68000 clocks a bit
	   about every 58us and the PIC has to answer inside that, so the quantum
	   has to stay well under it.  A maximum quantum of 10us was seen to work
	   but showed an intermittent failure once, and the margin depends on the
	   PIC clock, which is a placeholder here - so perfect quantum it is. */
	config.set_perfect_quantum(m_maincpu);
}


ROM_START( hshavoc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "d-25.11a", 0x000000, 0x080000, CRC(6a155060) SHA1(ecb47bd428786e50e300a062b5038f943419a389) )
	ROM_LOAD16_BYTE( "d-26.9a",  0x000001, 0x080000, CRC(1afa84fe) SHA1(041296e0360b7747aedc2d948c39e06ba03a7d08) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "peel18cv8s.4b.bin",  0x000, 0x155, CRC(b5fb1d5f) SHA1(f0ac80471d97f77f415b5a1f153e1fce66720963) )
	ROM_LOAD( "peel18cv8s.5b.bin",  0x000, 0x155, CRC(efc7ceea) SHA1(1c31a56bc4b83bfa708048b7de4cee7a24537500) )

	ROM_REGION( 0x407, "pic", 0 ) // pic16c55
	ROM_LOAD( "ms02-.7a",  0x000, 0x407, CRC(7163eb63) SHA1(d9c2cb7d24ce070d43597fabf012ebce47693954) ) // decapped
ROM_END


/*

Program ROM scrambling
----------------------

The 68000 data bus is scrambled.  Ten of the sixteen bits are simply permuted,
which the PCB traces do for free; the remaining six - bits 0, 1, 2, 8, 9 and 10 -
also go through the PEEL at 5b, which applies one of six fixed functions to
them.  Which one is decided by the PEEL at 4b and, through it, by the PIC, so on
the board the decryption depends on what the PIC has selected when a word is
fetched rather than on a list of address ranges.  Statically it comes down to:

    most of the ROM              setting 0 - type 1 and type 0 alternating with
                                 address bits A1-A4
    0x000000-0x000007            always type 0, because they are fetched while
    0x000c42-0x00109b            the PIC holds low what is taken to be the
                                 clear input of 4b
    0x03d000-0x03e1ff            settings 1 to 6, see 'The 0x3d000 block'
    0x03c000-0x03c2ff            function 0, with i8 of 5b low, because only
    0x04043a-0x04ff39            the VDP (by DMA) or the Z80 (through its bank
    0x053f94-0x0544b3            window) ever reads them
    0x0e8000-0x0fffff

The startup block at 0xc42 holds the boot chain (a run of 'move.l #imm,-(A7)'
pushes terminated by an rts, so the pushed addresses run as a chain of
subroutines), the JAMMA input reading code and the "DATA EAST CORPORATION"
string the PIC checks.  The PIC pulls port A bit 1 - taken to be the 4b clear,
see 'What the PEELs do' - low at 0x086, just before it starts listening, and
lets go at 0x0d4 once the exchange is over, so the reset vectors and everything
the 68000 runs up to that point decrypt as type 0.

In the four function 0 regions the permutation below is followed by function 0,
which exchanges two of the three pairs and adds no XOR.  That leaves 0x0000 and
0xffff alone, which is why those regions look unencrypted and why they match the
Mega Drive release byte for byte.

Three of those four regions can be checked against the Mega Drive release
(Captain Lang), which they match byte-for-byte: 0x4043a-0x4e239 at a shift of
-0x50, 0x53f94-0x544b3 at -0x1604, and 0xe8000 up at 0.  0x4043a is where the
index tables that precede it end and the first sprite DMA starts.  Beyond
0x4e23a the graphics are arcade-only - the coin-op HUD and score digits have no
home equivalent - so there is no reference for them, but they take the same
function and the set is zero-padded up to 0x4ff3a, where setting 0 resumes and
picks the Mega Drive data up again.  0x3c000-0x3c2ff is arcade-only too, bounded
by 0xff fill below and blank EPROM above.  Note that 0x4e23a appears as a
pointer inside the tables at 0x40406, but it is the end of the first sprite set,
not of the region.

These regions are what the other two bus masters on the board read.  Read
watchpoints over all four, together with a log of every write to the VDP control
port and of every Z80 access to its bank window, were taken over the attract
mode and a game up to stage 2, and they show
    - no read of any of the four regions by the 68000 itself;
    - 9613 DMAs from ROM, every one of them inside the regions;
    - 615225 Z80 reads through the bank window, every one of them between 0xe8000
      and 0xfffff, which is sound data rather than graphics.
That fits i8 being high while the 68000 is driving the bus and low while the VDP
or the Z80 is, with the address ranges only a way of saying which data those two
ever fetch.  It was measured in MAME rather than on the board, and which pin
feeds i8 - BGACK, or something in the bus arbitration of the Sega chip - is not
known.  The ranges cannot simply be replaced by a test on the bus master in the
driver, because MAME's VDP DMA and Z80 bank window both read the 68000 address
space directly, with nothing to say who is asking.

When looking for further regions of this kind, the giveaway is that a raw 0x0000
word - eight transparent pixels, which tile data is full of - comes out of
setting 0 as 0x0701 on a type-1 word and 0x0606 on a type-0 one.  A high density
of those two values is a far sharper indicator than any statistical measure of
the decrypted result.


What the PEELs do
-----------------

Both fuse maps have been read with jedutil.  Which pin does what is deduced from
those equations together with the ROM and PIC contents; none of it has been
traced on a board.

5b transforms the six bits as three pairs, (0,1), (2,8) and (9,10).  Exactly one
assignment of its pins to those bits makes its equations reproduce, entry for
entry, both 64-value tables the bulk of the ROM needs.  Under that assignment
its 32 control settings give six distinct functions, numbered as in peel_5b():

    0              b1   | b0   | b8      | b2    | b9       | b10        i8 low
    1  type 1      b0^1 | b1   | b2      | b8^1  | b9^b10^1 | b10^1      i1 low,  i9 low
    2  type 0      b0   | b1^1 | b2^b8^1 | b8    | b10^1    | b9^b10^1   i1 low,  i9 high
    3              b1^1 | b0   | b8^1    | b2^b8 | b10^1    | b9         i1 high, i9 low, state 0
    4              b1   | b0^1 | b2^b8   | b2^1  | b10      | b9^1       i1 high, i9 low, state 1
    5              b0^1 | b1^1 | b2^1    | b8^1  | b9       | b9^b10^1   i1 high, i9 high

with i8 high and i12 low on every line but the first.  0 exchanges the members
of (0,1) and (2,8) outright, and 3 and 4 those of (0,1) and (9,10), at most
inverting one of them; everywhere else a pair either keeps its members in place
or mixes them with an XOR.

'state' is 5b's one registered output.  i1 is pin 1, which on an 18CV8 is also
the register clock, so the state is loaded on each rising edge of i1 and held
until the next.  With i8 high the value loaded is a function of the six
scrambled bits on the inputs at that moment, taken from the word fetched just
before the edge; peel_5b_state() is that function.

4b provides i1 and i9.  Its pins 13 to 16 carry a 4-bit counter, but the
counter's outputs are only enabled while its i2 is low, and o18 and o19 decode
the pins rather than the registers.  The decryption follows address bits A1-A4
exactly, so those pins are evidently driven by the address bus and the counter
itself plays no part.  o18 drives i9 of 5b and o19 drives i1.  The other inputs
that matter are i1 - also 4b's own clock - i4, and i3, an asynchronous clear;
rf12 holds i4 as it was on the last rising edge of i1.  The three bits (i1, i4,
rf12) are the 'setting' used in init_hshavoc(), and PLD_4B[] there lists o19
and o18 for each of the seven settings and each value of A1-A4, plus the clear.
With the clear held o18 is high and o19 low whatever the address, which is
type 0.

Those three inputs come from the PIC: port A bit 3 is i1, bit 0 is i4 and bit 1
is i3.  That is not measured either, but it is not a fit - see below.


The 0x3d000 block
-----------------

This holds six copies of the per-life handshake routine, at 0x3d000, 0x3d302,
0x3d604, 0x3d900, 0x3dc02 and 0x3df04, separated by nop padding.  They are not
reached by any visible call - the dispatch table at 0x3a424 is the
authoritative list of entry points, and looking for them by code signature
instead will miss the ones whose entry decrypts into something unrecognisable.

Each entry of that table pairs a copy with a serial routine that runs first and
sends the PIC one byte.  The PIC's dispatcher at 0x0e1 branches on it to a short
block that sets port A, and read with the wiring above each block leaves 4b in
a different setting:

    serial routine   sends   PIC block   setting   copy
    0x3a4ee          0x11    0x1a9       1         0x3d000
    0x3a6b6          0x16    0x1b2       2         0x3d302
    0x3a87e          0x1d    0x1b9       3         0x3d604
    0x3aa46          0x23    0x1bf       4         0x3d900
    0x3ac0e          0x2c    0x1c4       5         0x3dc02
    0x3add6          0x2f    0x1cb       6         0x3df04

Each copy ends by calling the subroutine at its own offset 0xe0 (0x3d0e0,
0x3d3e2, 0x3d6e4, ...; all branches in the copies are PC-relative), which sends
0x08; that goes to the block at 0x1a1 and puts 4b back to setting 0.  Logging
port A against the PIC's program counter across a two player game shows exactly
that alternation.

The settings follow independently from the ROM and from the PIC: solving for
(i1, i4, rf12) against the decrypted first copy gives 1 to 6, in order, and
reading them out of the PIC's port A sequences gives the same values, the idle 0
included.  Two independent sources agreeing on all seven is the main reason to
trust the model.

The sixth copy needs 5b's state register as well.  Where a setting puts i1 high
and i9 low the function is 3 or 4 depending on the state, and the state is
loaded on the rising edge of i1 - o19 of 4b - from the word fetched just before.
In copies 1 to 5 every such load gives 0; in the sixth fourteen of them give 1,
and those are exactly where the sixth copy differs.  Across the six copies that
is 564 loads which affect the result, all of them right.  With it all six copies
decrypt to the same 376 words, and the padding between them to nops.

The loads are done here in address order, and none of them falls on a branch
target inside the routine.  How they relate to the 68000's actual bus cycles -
prefetch, and the I/O accesses the routine is full of - is not known.

Each copy writes 0xfffe30-0xfffe32 with the reply it reads back from the PIC,
which is what the protection below compares.  Decrypting it statically is right
even though the hardware does it on the fly: each copy only ever runs with its
own setting, so every address still has exactly one correct decryption.


Protection
----------

The PIC is not a boot check - it feeds the game a running answer that it needs
throughout play, and getting it wrong spoils the game gradually rather than
stopping it.  There are two separate chains.

The first runs once at boot.  The 68000 clocks "DATA EAST CORPORATION" out to
the PIC and the PIC verifies it in three chunks (8, 8 and 5 bytes), sending back
"MX-32" if they all pass.  The reply lands in 0xfffe80; 0x00113a copies it to
0xfffe36 and compares against "MX-3" at 0x001150, with 0x002dc8 checking the
trailing '2' separately.  A mismatch sets 0xfffe34 to 1.

The second runs once per life.  The code at 0x3a400 takes a counter mod 8,
indexes the table of eight (handshake routine, serial routine) pairs at
0x3a424, and pushes the pair so they run off the return chain.  The six
handshake routines it dispatches to are the 0x3d000 block; each leaves three
bytes of the PIC's reply in 0xfffe30.  Meanwhile 0x3a464 fetches the expected
value into 0xfffe2c from the table of thirteen longwords at 0x3a492, indexed by
a counter at 0xfffe28 that wraps at twelve.  Those thirteen longwords spell out

    KIDOTAYKIMDAOYASIRKTAMZAFJISINSATISITUZ

which is the second string held in the PIC, recited back a piece at a time.

Three copies of that comparison exist, at 0x727e, 0x10a9c and 0x3a3b6.  Each
XORs the two longwords, masks the low byte off, and returns quietly on a match.
On a mismatch it advances a failure counter at 0xfffe2a and writes a sabotage
level into 0xfffe34 from the table at 0x3a3e4 (1,2,3,4,5,6,7,1).  Thirty-three
places in the game read 0xfffe34 and the effects escalate life by life: level 1
swaps left and right on odd-numbered stages, toggling every 128 frames from the
frame counter at 0xfff908, and later levels give a two-second timer, a
character that walks at a crawl and finally the game cutting the match short.
Anyone who sees that pattern of symptoms is looking at this counter, not at a
decryption bug - and note that the alternating swap looks random until you know
about the frame counter.


Talking to the PIC
------------------

Two things about the chip's start-up are worth writing down, because both cost
a lot of time to find.

It will not leave its power-on self test until reading its port C 16320 times
accumulates to exactly 0x80, which needs a value congruent to 2 mod 4.  Port C
is driven as a bidirectional bus with four strobes on port A, so there is real
hardware on it, but what that is - plausibly the PEELs - is unknown.  The
constant supplied in the machine config satisfies the arithmetic and nothing
more.

That self test takes some 163000 instructions, while the 68000 reaches the boot
handshake about a hundred instructions after reset and never retries, so on the
board something must hold the 68000 off until the PIC is listening.  What does
so is not known.  Port A bit 1 is used here as the cue to release it: it is
taken to be the clear input of 4b (see 'What the PEELs do'), which the PIC pulls
low at 0x086, just before listening, and lets go at 0x0d4 once the exchange is
over.  So it marks the right moment, but
it is not itself the line that holds the 68000.  It is also pulsed low for a
single instruction at the top of the self test at 0x000, so releasing on the
first low edge lets the 68000 run through the whole test; the timer waits to see
whether it stays low.

The symptom when any of this is wrong is a first bit sampled without waiting for
a clock edge, leaving the exchange one bit out of step, which makes the
handshake succeed or fail seemingly at random.  In a PIC trace the healthy case
reaches 0x03f on the first pass through the bit loop; the broken one jumps
straight to 0x040.  With the release timed correctly the boot exchange completes
with zero mismatches.

*/

// the six functions the PEEL at 5b can apply, numbered as in the table above
static uint16_t peel_5b(uint16_t w, int which)
{
	uint8_t const b0 = BIT(w, 0), b1 = BIT(w, 1), b2 = BIT(w, 2);
	uint8_t const b8 = BIT(w, 8), b9 = BIT(w, 9), b10 = BIT(w, 10);
	uint8_t n0, n1, n2, n8, n9, n10;

	switch (which)
	{
	case 0:
		n0 = b1;       n1 = b0;       n2 = b8;
		n8 = b2;       n9 = b9;       n10 = b10;
		break;

	case 1:
		n0 = b0 ^ 1;   n1 = b1;       n2 = b2;
		n8 = b8 ^ 1;   n9 = b9 ^ b10 ^ 1;   n10 = b10 ^ 1;
		break;

	case 2:
		n0 = b0;       n1 = b1 ^ 1;   n2 = b2 ^ b8 ^ 1;
		n8 = b8;       n9 = b10 ^ 1;  n10 = b9 ^ b10 ^ 1;
		break;

	case 3:
		n0 = b1 ^ 1;   n1 = b0;       n2 = b8 ^ 1;
		n8 = b2 ^ b8;  n9 = b10 ^ 1;  n10 = b9;
		break;

	case 4:
		n0 = b1;       n1 = b0 ^ 1;   n2 = b2 ^ b8;
		n8 = b2 ^ 1;   n9 = b10;      n10 = b9 ^ 1;
		break;

	default: // 5
		n0 = b0 ^ 1;   n1 = b1 ^ 1;   n2 = b2 ^ 1;
		n8 = b8 ^ 1;   n9 = b9;       n10 = b9 ^ b10 ^ 1;
		break;
	}

	return (w & 0xf8f8)
			| (n0 << 0) | (n1 << 1) | (n2 << 2)
			| (n8 << 8) | (n9 << 9) | (n10 << 10);
}

// what the state register of the PEEL at 5b loads, with i8 high, from a word
static bool peel_5b_state(uint16_t w)
{
	return ((w & 0x0706) == 0x0204) || ((w & 0x0707) == 0x0506)
			|| ((w & 0x0707) == 0x0101) || ((w & 0x0607) == 0x0604)
			|| ((w & 0x0507) == 0x0007) || ((w & 0x0307) == 0x0201)
			|| ((w & 0x0107) == 0x0002);
}


void hshavoc_state::init_hshavoc()
{
	uint16_t *src = (uint16_t *)memregion("maincpu")->base();

	/* o19 (bit 1) and o18 (bit 0) of the PEEL at 4b, by address bits A1-A4, for
	   each setting the PIC can select through port A, and with its clear held
	   low in the last row.  o18 is taken to drive i9 of 5b and o19 i1 (see
	   'What the PEELs do'). */
	static constexpr uint8_t PLD_4B[8][16] =
	{
		{ 0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 1, 0,  0, 1, 0, 0 },   // 0, PIC command 0x08
		{ 0, 2, 3, 2,  0, 2, 3, 2,  0, 2, 2, 2,  0, 2, 2, 2 },   // 1, 0x11
		{ 2, 0, 3, 3,  2, 0, 2, 3,  2, 0, 3, 2,  2, 0, 2, 2 },   // 2, 0x16
		{ 3, 2, 0, 2,  2, 2, 0, 2,  2, 2, 0, 2,  3, 2, 0, 2 },   // 3, 0x1d
		{ 0, 2, 2, 2,  0, 2, 2, 2,  0, 3, 3, 2,  0, 3, 2, 2 },   // 4, 0x23
		{ 2, 0, 3, 2,  2, 0, 3, 2,  2, 0, 2, 2,  2, 0, 2, 2 },   // 5, 0x2c
		{ 2, 2, 1, 3,  2, 2, 0, 3,  2, 2, 1, 2,  2, 2, 0, 2 },   // 6, 0x2f
		{ 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1 }    // clear held low
	};

	// what is selected when each part of the ROM is fetched; anything else is 0
	static constexpr struct { int start, end, setting; } SETTINGS[] =
	{
		{ 0x000000, 0x000008, 7 },   // reset vectors, fetched with the clear held
		{ 0x000c42, 0x00109c, 7 },   // boot code, likewise
		{ 0x03d000, 0x03d302, 1 },   // per-life handshake routines
		{ 0x03d302, 0x03d604, 2 },
		{ 0x03d604, 0x03d900, 3 },
		{ 0x03d900, 0x03dc02, 4 },
		{ 0x03dc02, 0x03df04, 5 },
		{ 0x03df04, 0x03e200, 6 }
	};

	// what only the VDP and the Z80 read, so with i8 of 5b low
	static constexpr struct { int start, end; } FUNCTION_0[] =
	{
		{ 0x03c000, 0x03c300 },   // arcade-only HUD tiles (timer, health bar)
		{ 0x04043a, 0x04ff3a },   // sprite tiles and arcade HUD
		{ 0x053f94, 0x0544b4 },   // further tiles
		{ 0x0e8000, 0x100000 }    // sound data, read by the Z80
	};

	bool state = false;
	uint16_t prev = 0;

	for (int x = 0; x < 0x100000 / 2; x++)
	{
		// the permutation the PCB traces do, the same everywhere
		uint16_t const w = bitswap<16>(src[x],
								7, 15,6, 14,
								5, 2, 1, 10,
								13,4, 12,3,
								11,0, 8, 9 );

		bool i8 = true;
		for (auto &r : FUNCTION_0)
		{
			if (x >= r.start / 2 && x < r.end / 2)
			{
				i8 = false;
				break;
			}
		}

		int setting = 0;
		for (auto &r : SETTINGS)
		{
			if (x >= r.start / 2 && x < r.end / 2)
			{
				setting = r.setting;
				break;
			}
		}

		uint8_t const ctl = PLD_4B[setting][x & 0x0f];
		uint8_t const ctl_prev = PLD_4B[setting][(x - 1) & 0x0f];

		// 5b loads its state on the rising edge of i1, from the word before
		if (BIT(ctl, 1) && !BIT(ctl_prev, 1))
			state = peel_5b_state(prev);

		int const which = !i8 ? 0
				: !BIT(ctl, 1) ? (BIT(ctl, 0) ? 2 : 1)
				: BIT(ctl, 0) ? 5
				: state ? 4 : 3;

		src[x] = peel_5b(w, which);
		prev = w;
	}

	{
		address_space &space = m_maincpu->space(AS_PROGRAM);
		space.nop_write(0x200000, 0x201fff);
	}

	init_megadriv();

	m_vdp->stop_timers();
}

} // anonymous namespace


GAME( 1994, hshavoc, 0, hshavoc, hshavoc, hshavoc_state, init_hshavoc, ROT0, "Data East Corporation", "High Seas Havoc", MACHINE_UNEMULATED_PROTECTION )
