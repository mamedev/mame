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


TODO: three things need the board rather than more analysis, and between them
      they would close everything still open here.  Continuity from the PIC's
      port A pins to the control inputs of the two PEELs would give the rule
      for the 0x3d000 block, which is currently decrypted by replicating one
      copy; whether any PEEL I/O pin reaches the 68000 data bus would confirm
      or demolish the model of the scrambling as a whole; and what sits on the
      PIC's port C, together with what holds the 68000 off at start-up, would
      replace the two guesses in the machine config that happen to work.
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

	/* Port A bit 1 is pulsed low for a single instruction at the top of the
	   self test routine at 0x000, and then held low from 0x086 until 0x0d4 -
	   the window in which the boot handshake happens.  Only the second of
	   those means the PIC is ready, so wait and see whether it stays low. */
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
also go through the logic in the PEEL at 5b.  Which of two transforms it
applies alternates on a period of 16 words, driven by a counter in the PEEL at
4b; counter_decode[] below is that counter's output.

Most of the ROM decrypts with that scheme.  These regions do not:

    0x000000-0x000007   initial SSP and reset PC        always 'type 0'
    0x000c42-0x00109b   arcade-specific startup code    always 'type 0'
    0x03c000-0x03c2ff   arcade-only HUD tiles           transform 0, no XOR
    0x040440-0x04ff39   sprite tiles and arcade HUD     transform 0, no XOR
    0x053f94-0x0544b3   further tiles                   transform 0, no XOR
    0x0e8000-0x0fffff   graphics and data               transform 0, no XOR
    0x03d000-0x03e1ff   six copies of one routine       see 'The 0x3d000 block'

Older versions of this driver applied a second, different bitswap to the four
regions marked above.  There is no second bitswap: that permutation is the one
below followed by transform 0, which exchanges two of the three pairs and adds
no XOR.  It leaves 0x0000 and 0xffff alone, which is why those regions look
unencrypted and why the graphics match the Mega Drive release byte for byte.

The startup block at 0xc42 holds the boot chain (a run of 'move.l #imm,-(A7)'
pushes terminated by an rts, so the pushed addresses run as a chain of
subroutines), the JAMMA input reading code, and the "DATA EAST CORPORATION"
string the PIC checks.  Because it is uniformly type 0 it also decrypts the
reset vector, so the four hand-patched XORs that older versions of this driver
applied to src[0..3] are no longer needed.

Three of the four tile regions can be checked against the Mega Drive release
(Captain Lang), which they match byte-for-byte: 0x40440-0x4e239 at a shift of
-0x50, 0x53f94-0x544b3 at -0x1604, and 0xe8000 up at 0.  0x40440 is also where
the index tables that precede it end.  Beyond 0x4e23a the graphics are
arcade-only - the coin-op HUD and score digits have no home equivalent - so
there is no reference for them, but they use the same bitswap and the set is
zero-padded up to 0x4ff3a, where the main scheme resumes and picks the Mega
Drive data up again.  0x3c000-0x3c2ff is arcade-only too, bounded by 0xff fill
below and blank EPROM above.

Note that 0x4e23a is a real boundary that appears as a pointer inside the
tables at 0x40406, but it is the end of the first sprite set, not of the
scrambling region - a distinction that cost some time to work out.

When looking for further regions of this kind, the giveaway is that a raw
0x0000 word - eight transparent pixels, which tile data is full of - comes out
of the main scheme as 0x0701 on a type-1 word and 0x0606 on a type-0 one.  A
high density of those two values is a far sharper indicator than any
statistical measure of the decrypted result.


What the PEELs do
-----------------

The two PEEL18CV8s generate the scrambling control, and their fuse maps explain
the scheme completely enough to have cracked the 0x3d000 block with.

The one at 4b is a 4-bit synchronous binary counter (its four registered
outputs are a textbook ripple chain) with an asynchronous clear, plus a
one-clock delay of one input pin.  Its o18 output, with that input and its
delayed copy both low, is the exact complement of counter_decode[] below for
all sixteen entries - which is how that table was identified as the counter's
output rather than an arbitrary pattern.

The one at 5b transforms three input pairs into three output pairs, gated by
i1, i8, i9, i12 and an internal state bit.  Six outputs matches the six data
bits that need more than a permutation, and there is exactly one assignment of
pins to bits under which its equations reproduce, entry for entry, both of the
64-value tables this driver applies by hand: the pairs are (0,1), (2,8) and
(9,10), and the two transforms differ only in i9, which must therefore be
driven by the counter decode from 4b.

Across its 32 control settings 5b produces six distinct transforms; four of
them are peel_5b() below, and two of those four are the type 1 and type 0 that
the main scheme alternates between.  Written out on the pairs the six are:

    1 = type 1 above     b0^1 | b1   | b2      | b8^1  | b9^b10^1 | b10^1
    2 = type 0 above     b0   | b1^1 | b2^b8^1 | b8    | b10^1    | b9^b10^1
    3                    b1^1 | b0   | b8^1    | b2^b8 | b10^1    | b9
    4                    b1   | b0^1 | b2^b8   | b2^1  | b10      | b9^1
    5                    b0^1 | b1^1 | b2^1    | b8^1  | b9       | b9^b10^1

3 and 4 exchange the two members of every pair, the others do not - the
conditional swap the 5b equations describe.  Earlier versions of this driver
wrote the first two out as XOR keys with tests on the value being decrypted;
that gives identical results everywhere it was used, but it is not what the
hardware does, and it does not extend to the other four.

None of this has been confirmed on the board.  The discriminating measurement
is whether any of the PEEL I/O pins reach the 68000 data bus at all: if none
do, the scrambling was applied to the ROM image when it was built, the PEELs do
something else, and the match between the 4b counter decode and the pattern
this driver applies is a remarkable coincidence rather than an explanation.  The PIC's port A pins are
worth tracing at the same time - see 'The 0x3d000 block' below for why they
look like the thing that picks among these six.


The 0x3d000 block
-----------------

This holds six copies of the per-life handshake routine, at 0x3d000, 0x3d302,
0x3d604, 0x3d900, 0x3dc02 and 0x3df04, separated by nop padding.  They are not
reached by any visible call - the dispatch table at 0x3a424 is the
authoritative list of entry points, and looking for them by code signature
instead will miss the ones whose entry decrypts into something unrecognisable.

The main scheme does not decrypt them: the first branch in each comes out with
an odd target or an illegal opcode, the 68000 takes an exception, and the game
restarts.  What does work is the transforms above.  The ten bits the XOR stages
never touch are already correct here, so only six bits per word are wrong, and
choosing among T1, T3 and T5 by word position decrypts the first copy
completely - 62 of 62 accesses to 0xa10005 well formed, and a routine that
writes 0xfffe30, 0xfffe31 and 0xfffe32, which is exactly what the protection
below reads.

Five of the six copies then come out identical to the first, word for word,
under the same choice seen from a different starting point; every word of the
sixth admits a transform that agrees with it as well, and the chance of that
happening by accident is nil, so all six certainly hold the same code.

No rule based on the address reproduces that, because the selection does not
come from the address - it comes from the two PEELs, with the PIC setting them
up.  The counter in 4b drives 5b: its o18 goes to i9 and its o19 to i1, with i8
tied high and i12 and the state bit low.  That wiring is forced - it is the only
one that turns the counter into the exact sequence of transforms the first copy
needs.  The counter is cleared at the top of every routine, so every copy starts
from the same phase, and what distinguishes them is two more inputs of 4b: i1
and i4, the latter also appearing one clock late in rf12.

Solving for those against the decrypted first copy gives, read as a three bit
field,

    0x3d000 = 1   0x3d302 = 2   0x3d604 = 3
    0x3d900 = 4   0x3dc02 = 5   0x3df04 = 6

one per entry of the dispatch table, in order.  That is what the PIC selects.
Its dispatcher at 0x0e1 exchanges a byte with the 68000 and branches on the
value; 0x08, 0x11, 0x16, 0x1d, 0x23 and 0x2c each lead to a short block between
0x1a1 and 0x1d1 that drives a different combination of port A bits 0, 1 and 3
and returns to the exchange loop, while 0x04 - the byte these routines send
first - goes to a second level at 0x10e that dispatches on an internal index.
Logging port A against the PIC's own program counter across a full two player
game gives

    boot  B A  C A  D A  E A  F A  G A  B A  C A  B A  C A  D A  E A

with A the idle configuration restored after each exchange and B to G the six
distinct ones, cycled in step with the dispatch table.  So the board configures
the descrambler before running each routine and Data East encrypted each copy
for the configuration in force when it runs - which is also why every ROM
address still has exactly one correct decryption and decrypting statically is
right.

Five of the six copies decrypt to identical code under this model, 376 words out
of 376.  The sixth takes the value the sequence predicts and still leaves 37
words differing, all in the classes the model puts on transform 2 or 4, so it is
replicated from the first instead - its bytes are certainly right, since every
one of its words admits a transform agreeing with the first copy and the chance
of that by accident is nil.  Whether the remainder is a seventh control value,
an input of 4b held fixed here, or code that genuinely differs is not known.

What has not been checked on the board is which port A pin reaches which input
of the PEELs.  The model rests on the decryption coming out right rather than on
any measurement, so a continuity check there would either confirm it or show
that something else produces the same six sequences.


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
board something must hold the 68000 off until the PIC is listening.  Port A bit
1 is low exactly across the handshake window, from 0x086 until 0x0d4, so it is
used here as the cue to release the 68000 - but it cannot be the real signal,
since it goes high again at 0x0d4 while the PIC still needs the 68000 clocking
to send its reply.  Note also that it is pulsed low for a single instruction at
0x000, at the top of the self test, so releasing on the first low edge lets the
68000 run through the whole test; the timer waits to see whether it stays low.

The symptom when any of this is wrong is a first bit sampled without waiting for
a clock edge, leaving the exchange one bit out of step, which makes the
handshake succeed or fail seemingly at random.  In a PIC trace the healthy case
reaches 0x03f on the first pass through the bit loop; the broken one jumps
straight to 0x040.  With the release timed correctly the boot exchange completes
with zero mismatches.

*/

/*
    The six bits that the PEEL at 5b works on, as the three pairs (0,1), (2,8)
    and (9,10).  Its fuse map yields six functions of them, and every part of
    the ROM uses one of these five: 1 and 2 are the 'type 1' and 'type 0' the
    counter in 4b alternates between, 0 covers the graphics and everything from
    0xe8000 up, and 3 and 5 are used by the block at 0x3d000.  0 and 3 exchange
    the members of a pair, the others do not - the conditional swap the
    equations describe.  Writing them out this way rather than as tests on the
    data being decrypted is what the hardware actually does; the two happen to
    agree everywhere the main scheme is used.
*/
static uint16_t peel_5b(uint16_t w, int which)
{
	uint8_t const b0 = BIT(w, 0), b1 = BIT(w, 1), b2 = BIT(w, 2);
	uint8_t const b8 = BIT(w, 8), b9 = BIT(w, 9), b10 = BIT(w, 10);
	uint8_t n0, n1, n2, n8, n9, n10;

	switch (which)
	{
	case 0: // a pure exchange, no XOR at all
		n0 = b1;       n1 = b0;       n2 = b8;
		n8 = b2;       n9 = b9;       n10 = b10;
		break;

	case 1: // 'type 1'
		n0 = b0 ^ 1;   n1 = b1;       n2 = b2;
		n8 = b8 ^ 1;   n9 = b9 ^ b10 ^ 1;   n10 = b10 ^ 1;
		break;

	case 2: // 'type 0'
		n0 = b0;       n1 = b1 ^ 1;   n2 = b2 ^ b8 ^ 1;
		n8 = b8;       n9 = b10 ^ 1;  n10 = b9 ^ b10 ^ 1;
		break;

	case 5:
		n0 = b0 ^ 1;   n1 = b1 ^ 1;   n2 = b2 ^ 1;
		n8 = b8 ^ 1;   n9 = b9;       n10 = b9 ^ b10 ^ 1;
		break;

	default: // 3
		n0 = b1 ^ 1;   n1 = b0;       n2 = b8 ^ 1;
		n8 = b2 ^ b8;  n9 = b10 ^ 1;  n10 = b9;
		break;
	}

	return (w & 0xf8f8)
			| (n0 << 0) | (n1 << 1) | (n2 << 2)
			| (n8 << 8) | (n9 << 9) | (n10 << 10);
}


void hshavoc_state::init_hshavoc()
{
	uint16_t *src = (uint16_t *)memregion("maincpu")->base();

	/* The decode of the 4-bit counter in the PEEL at 4b, which is what picks
	   between the two transforms the bulk of the ROM alternates between.  This
	   used to be called 'typedat' and treated as a table; it is really the
	   output of that counter, and its period of 16 is the counter wrapping. */
	static const uint8_t counter_decode[16] = {
		1,1,1,1, 1,1,1,1,
		1,0,0,1, 1,0,1,1  };

	/* Regions that take transform 0 - a pure exchange of two of the three
	   pairs, with no XOR at all.  Everything from 0xe8000 up is like this, and
	   so are three blocks of tile data lower down.  When hunting for further
	   ones, note that a raw 0x0000 word - eight transparent pixels, which tile
	   data is full of - comes out of the main scheme as 0x0701 on a type-1
	   word and 0x0606 on a type-0 one; a high density of those two values is a
	   much sharper indicator than any statistical measure of the result. */
	static const struct { int start, end; } bitswap_only[] =
	{
		{ 0x03c000, 0x03c300 },   // arcade-only HUD tiles (timer, health bar)
		{ 0x040440, 0x04ff3a },   // sprite tiles and arcade HUD
		{ 0x053f94, 0x0544b4 },   // further tiles
		{ 0x0e8000, 0x100000 }    // graphics and data
	};

	for (int x = 0; x < 0x100000 / 2; x++)
	{
		bool plain = false;
		for (auto &r : bitswap_only)
		{
			if (x >= r.start / 2 && x < r.end / 2)
			{
				plain = true;
				break;
			}
		}

		// the permutation the PCB traces do, the same everywhere
		src[x] = bitswap<16>(src[x],
								7, 15,6, 14,
								5, 2, 1, 10,
								13,4, 12,3,
								11,0, 8, 9 );

		if (plain)
		{
			src[x] = peel_5b(src[x], 0);
			continue;
		}

		// the 0x3d000 block picks its own transforms, applied further down
		if (x >= 0x03d000 / 2 && x < 0x03e200 / 2)
			continue;

		int type = counter_decode[x & 0xf];

		/* The initial SSP/PC and the arcade-specific startup block at
		   0xc42-0x109b behave as if every word were 'type 0'. */
		if (x < 0x000008 / 2)
			type = 0;
		else if (x >= 0x000c42 / 2 && x < 0x00109c / 2)
			type = 0;

		src[x] = peel_5b(src[x], type ? 1 : 2);
	}

	/* The six per-life handshake routines all hold the same 0x2f0 bytes of
	   code.  The permutation above already puts ten of the sixteen bits in
	   place; the other six are transformed by the PEEL at 5b, and this block
	   uses three of the six functions that its fuse map yields, picked by word
	   position.  See 'The 0x3d000 block' above for why the first copy is
	   decrypted and replicated rather than each copy being decrypted in
	   place. */
	/* Which of the transforms applies is not a property of the address: it comes
	   out of the counter in 4b, whose o18 drives i9 of 5b and whose o19 drives
	   i1, with i8 tied high and i12 and the state bit low.  The counter is
	   cleared at the top of every routine, and the PIC sets two more of its
	   inputs beforehand - so a copy is characterised by (i1, i4) of 4b plus the
	   one-clock delayed copy of i4 in rf12.  Solving for those against the
	   decrypted first copy gives 1, 2, 3, 4, 5, 6 read as a three bit field,
	   one per entry of the dispatch table, which is what the PIC's six port A
	   configurations select.  Five of the six then decrypt to identical code.

	   The sixth does not - 37 of its 376 words still differ, all in the classes
	   that the model puts on transform 2 or 4 - so it is replicated from the
	   first, whose bytes are certainly right.  Whether that is a seventh
	   control value, or an input of 4b held fixed here, is not known. */

	static const struct { int base, i1, i4, rf12; } copies[] =
	{
		{ 0x03d000, 0, 0, 1 },
		{ 0x03d302, 0, 1, 0 },
		{ 0x03d604, 0, 1, 1 },
		{ 0x03d900, 1, 0, 0 },
		{ 0x03dc02, 1, 0, 1 },
		{ 0x03df04, 1, 1, 0 }   // the odd one out, replicated below
	};

	static const uint8_t sel[6][16] =
	{
		{ 1,3,5,3, 1,3,5,3, 1,3,3,3, 1,3,3,3 },
		{ 3,1,5,5, 3,1,3,5, 3,1,5,3, 3,1,3,3 },
		{ 5,3,1,3, 3,3,1,3, 3,3,1,3, 5,3,1,3 },
		{ 1,3,3,3, 1,3,3,3, 1,5,5,3, 1,5,3,3 },
		{ 3,1,5,3, 3,1,5,3, 3,1,3,3, 3,1,3,3 },
		{ 3,3,2,5, 3,3,1,5, 3,3,2,3, 3,3,1,3 }
	};

	for (int c = 0; c < 5; c++)
	{
		for (int i = 0; i < 0x2f0 / 2; i++)
		{
			int const x = (copies[c].base / 2) + i;

			src[x] = peel_5b(src[x], sel[c][x & 0x0f]);
		}
	}

	// the sixth copy holds the same code, so take it from the first
	for (int i = 0; i < 0x2f0 / 2; i++)
		src[(0x03df04 / 2) + i] = src[(0x03d000 / 2) + i];

	{
		address_space &space = m_maincpu->space(AS_PROGRAM);
		space.nop_write(0x200000, 0x201fff);
	}

	init_megadriv();

	m_vdp->stop_timers();
}

} // anonymous namespace


GAME( 1994, hshavoc, 0, hshavoc, hshavoc, hshavoc_state, init_hshavoc, ROT0, "Data East Corporation", "High Seas Havoc", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
