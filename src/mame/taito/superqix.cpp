// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni, Nicola Salmoria, Tomasz Slanina
/***************************************************************************

Prebillian        (c) 1986 Taito
Hot Smash         (c) 1987 Taito
Super Qix         (c) 1987 Taito
Perestroika Girls (c) 1994 Promat (hack of Super Qix)

driver by Mirko Buffoni, Nicola Salmoria, Tomasz Slanina

Super Qix is a later revision of the hardware, featuring a bitmap layer that
is not present in the earlier games. It also has two 8910, while the earlier
games have one 8910 + a sample player.

Notes:
- All versions of the hardware have four 64kx4 DRAMs near the video roms;
  the gfx hardware draws to this as a backing framebuffer for sprites and
  background layer during display. This ram is not tested, or even testable
  by the z80. See http://www.jammarcade.net/tag/super-qix/
  Super Qix: 6S and 6R are the BG framebuffer
             5P and 5M are the Sprite framebuffer

Super Qix:
- The sq07.ic108 ROM came from a bootleg where the 8751 MCU was replaced by an
  8031 MCU plus an external ROM (i.e. the sqixb1 romset). The 8031 ROM was bad
  (bit 3 was stuck high). It was originally recovered by carefully checking
  the disassembly, and this repair was later verified from another dump of an
  intact chip to be correct. The majority of the bootleg 8031 MCU code matches
  the deprotected sqix b03__03.l2 MCU code, implying the sq07.ic108 8031 MCU
  code ROM was derived from code dumped from an original Taito b03__03.l2 8751
  MCU somehow.
  The bootleg MCU code is different from the original b03__03.l2 MCU since
  an 8031 when running in external ROM mode cannot use ports 0 or 2, hence
  the code was extensively patched by the bootleggers to avoid use of those
  ports, by adding an additional multiplexer to port 1, and moving various
  read and write pins around.
  An important note about the sqixb1 bootleg pcb: the SOCKET on the pcb for
  sq07.ic108 is populated backwards compared to the way the ROM will fit into
  it! This is probably the cause of the bad bit 3 in the original dump in MAME
  (due to someone inserting and powering the chip backwards) and is definitely
  the cause of at least one other ROM failure during a repair. Be aware of
  this, if you find or own one of these PCBs!
  On certain pcbs the rom may be labeled just '7' rather than 'sq07'.
  The 8031 may have a capacitor or diode (unclear?) between vcc and p1.2 for an
  unknown reason. Some pcbs have it, some do not.

- The MCU sends some ID to the Z80 on startup, but the Z80 happily ignores it.
  This happens in all sets. There appears to be code that would check part of
  the MCU init sequence ($5973 onwards), but it doesn't seem to be called.

- sqixr0 (World/Japan V1.0) (and sqixb1 which is the exact same ROMs but the
  8751 MCU replaced with an 8031) has a bug with coin lockout: it is activated
  after inserting 10 coins instead of 9.
  This is fixed in World/Japan V1.1, V1.2 and the US set.
  In addition, the polarity of the coin lockout on V1.0 (and sqixb1) is
  flat-out reversed, so the pcb will not work in a standard JAMMA harness with
  coin lockouts without inverting JAMMA pins K and/or 9. The hack below on V1.0
  pcbs with the two wires connecting to IC 7H may have been a workaround which
  involved a customized JAMMA connector/harness. How exactly is unclear.

- All Taito Super Qix PCBS are part M6100237A, and have a wiring hack on top of
  component 7H (a 74LS86 Quad XOR gate):
  (reference: 74LS86 pins 4, 5 and 12 are 2A, 2B and 4A respectively, none are
  outputs)
  The V1.0 PCBs have two greenwires running on the back of the pcb, connected
  from 7H pins 4 and 5, to JAMMA pins e (GND) and d (unused) respectively.
  This implies there was a hack done on the JAMMA harness connector itself
  (possibly to invert the coin lockout value using one of the XOR gates
  at 7H (or perhaps 7H controls the coin lockouts themselves?)) but what
  exactly the hack does is unclear without further tracing.

  The V1.1, V1.2 and US PCBS have two resistors from VCC to GND forming a
  voltage divider on top of 7H, the resistor from VCC/Pin 14 to Common is
  22KOhms, the other resistor is also 22KOhms and seems to connect to
  GND/Pin 7. The center of the two resistors connects to one end of a 0.1uf
  capacitor and also to 7H pin 4, the other end of the capacitor connects to
  7H pin 12.
  This implies some sort of brief/reset pulse generation or filter on pin 12,
  or more likely some abuse of the TTL input hysteresis of the 74LS86 IC itself
  such that transitions of pin 12 cause transitions on pin 4 as well, or similar.
  Again, what exactly this accomplishes is unclear without further tracing.

- sqixb2 is a bootleg of sqixb1, with the MCU removed.

- Prebillian controls: (from the Japanese flyer):
  - pullout plunger for shot power (there's no on-screen power indicator in the game)
  - dial for aiming
  - button (fire M powerup, high score initials)


TODO:
- The way we generate NMI in sqix doesn't make much sense, but is a workaround
  for the slow gameplay you would otherwise get. Some interaction with vblank?

- I'm not sure about the NMI ack at 0418 in the original sqix, but the game hangs
  at the end of a game without it. Note that the bootleg replaces that call with
  something else. That something else is actually reading the system/Coin/Start
  inputs from 0418, which the MCU normally reads from its port 0, hence...
- Given the behavior of prebillian and hotsmash, I'm guessing 0418 resetting the
  NMI latch (i.e. NMI ACK) is correct. [LN]


Prebillian :
------------

PCB Layout (Prebillian)

 M6100211A
 -------------------------------------------------------------------
 |                    HM50464                                       |
 |  6                 HM50464                                       |
 |  5                 HM50464                               6116    |
 |  4                 HM50464                                       |
 |                                                                  |
 |                                                                  |
 |                                                               J  |
 |                                            68705P5 SW1(8)        |
 |               6264                                            A  |
 |                                              3     SW2(8)        |
 |                                                               M  |
 |                                                                  |
 |                                                               M  |
 |                                                                  |
 |                                   2                           A  |
 |                                                                  |
 |                                   1                              |
 |                                                                  |
 |                                   Z80B            AY-3-8910      |
 | 12MHz                                                            |
 --------------------------------------------------------------------

Notes:
       Vertical Sync: 60Hz
         Horiz. Sync: 15.67kHz
         Z80B Clock : 5.995MHz
     AY-3-8910 Clock: 1.499MHz



Hot (Vs) Smash :
----------------

Dips (not verified):

DSW1 stored @ $f236
76------ coin a
--54---- coin b
----3--- stored @ $f295 , tested @ $2a3b
------1- code @ $03ed, stored @ $f253 (flip screen)

DSW2 stored @ $f237
---4---- code @ $03b4, stored @ $f290
----32-- code @ $03d8, stored @ $f293 (3600/5400/2400/1200  -> bonus  ?)
------10 code @ $03be, stored @ $f291/92 (8,8/0,12/16,6/24,4 -> difficulty ? )

hotsmash notes for 408-41f area, related to above
code at z80:0070:
 set bit 3 at ram address f253 (was 0x00, now 0x08)
 read ram address f253 to 'a' register
 set bc to 0410, write 'a' register (0x08) to bc

code at z80:0093:
 set bc to 0418, read from bc and ignore result
 set bit 4 at ram address f253 (was 0x08, now 0x18)
 read ram address f253 to 'a' register
 set bc to 0410, write 'a' register (0x18) to bc


***************************************************************************/

#include "emu.h"
#include "superqix.h"

#include "cpu/z80/z80.h"
#include "screen.h"
#include "speaker.h"


SAMPLES_START_CB_MEMBER(hotsmash_state::pbillian_sh_start)
{
	// convert 8-bit unsigned samples to 8-bit signed
	m_samplebuf = std::make_unique<int16_t[]>(m_samples_region.length());
	for (unsigned i = 0; i < m_samples_region.length(); i++)
		m_samplebuf[i] = s8(m_samples_region[i] ^ 0x80) * 256;
}

void hotsmash_state::pbillian_sample_trigger_w(u8 data)
{
	//logerror("sample trigger write of %02x\n", data);

	// look for end of sample marker
	unsigned start = data << 7;
	unsigned end = start;
	while ((end < m_samples_region.length()) && (m_samples_region[end] != 0xff))
		end++;

	m_samples->start_raw(0, m_samplebuf.get() + start, end - start, (XTAL(12'000'000)/3072).value()); // needs verification, could be 2048 and 4096 alternating every sample
}

/**************************************************************************

Super Qix Z80 <-> 8751 communication

This is quite hackish, because the communication protocol is not very clear.

The Z80 acts this way:
- wait for 8910 #0 port B, bit 6 to be 0
- write command for MCU to 8910 #1 port B
- read port 0408
- wait for 8910 #0 port B, bit 6 to be 1
- read answer from MCU from 8910 #1 port B
- read port 0408

also, in other places it waits for 8910 #0 port B, bit 7 to be 0

The MCU acts this way:
- write FF to latch
- fiddle with port 1
- wait for IN2 bit 7 to be 1
- read command from latch
- process command
- fiddle with port 1
- write answer to latch
- wait for IN2 bit 7 to be 1

**************************************************************************/

int superqix_state::fromz80_semaphore_input_r()
{
	return (m_z80_has_written ? 1 : 0);
}

int superqix_state::frommcu_semaphore_input_r()
{
	return (m_mcu_has_written ? 1 : 0);
}

TIMER_CALLBACK_MEMBER(superqix_state::z80_semaphore_assert_cb)
{
	/* if we're on a set with no mcu, namely sqixb2, perestro or perestrof,
	   do not assert the semaphore since at least a few checks in sqixb2 were
	   not patched out by the bootleggers, hence the semaphore flags must both
	   be hard-wired inactive on the pcb, or else it will never boot to the
	   title screen.
	   perestro and perestrof seem to completely ignore the semaphores.
	 */
	if (m_mcu.found()) m_z80_has_written = 1;
}

TIMER_CALLBACK_MEMBER(superqix_state::mcu_port2_w_cb)
{
	u8 const changed_m_port2 = m_port2_raw ^ param;
	m_port2_raw = param;
	// bit 0 = inverted CLK for 74ls174 @1J; normally active on rising edge, this is inverted first, hence active on the falling edge
	if (BIT(changed_m_port2, 0) && !BIT(m_port2_raw, 0))
	{
		// bit 1 = 74ls174@1J.d0 = coin cointer 1
		machine().bookkeeping().coin_counter_w(0, BIT(m_port2_raw, 1));

		// bit 2 = 74ls174@1J.d2 = coin counter 2
		machine().bookkeeping().coin_counter_w(1, BIT(m_port2_raw, 2));

		// bit 3 = 74ls174@1J.d3 = coin lockout
		machine().bookkeeping().coin_lockout_global_w(BIT(m_port2_raw, 3) ^ m_invert_coin_lockout);

		// bit 4 = 74ls174@1J.d5 = flip screen
		flip_screen_set(BIT(m_port2_raw, 4));

		// bit 5 = 74ls174@1J.d4 = Z80 /RESET
		m_maincpu->set_input_line(INPUT_LINE_RESET, BIT(m_port2_raw, 5) ? CLEAR_LINE : ASSERT_LINE);

		// bit 6 = 74ls174@1J.d1 = the mcu->z80 semaphore, visible un-inverted on AY-3-8910 #1 @3P Port B bit 6
		m_mcu_has_written = BIT(m_port2_raw, 6);
	}

	// bit 7 = TODO: PROBABLY resets the m_z80_has_written semaphore on falling edge (or level? this needs more tracing)
	if (BIT(changed_m_port2, 7) && !BIT(m_port2_raw, 7))
	{
		m_z80_has_written = 0;
	}
}

TIMER_CALLBACK_MEMBER(superqix_state::mcu_port3_w_cb)
{
	// the ay #2 iob bus and the mcu port 3 are literally directly connected together, so technically the result could be a binary AND of the two...
	m_from_mcu = param;
}

TIMER_CALLBACK_MEMBER(superqix_state::z80_ay1_sync_address_w_cb)
{
	m_ay1->address_w(param);
}


TIMER_CALLBACK_MEMBER(superqix_state::z80_ay2_iob_w_cb)
{
	// the ay #2 iob bus and the mcu port 3 are literally directly connected together, so technically the result could be a binary AND of the two...
	m_from_z80 = param;
}

void superqix_state::z80_ay1_sync_address_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(superqix_state::z80_ay1_sync_address_w_cb), this), data);
}

uint8_t superqix_state::z80_ay2_iob_r()
{
//  logerror("%0s: read mcu answer (%02x)\n",machine().describe_context(), m_from_mcu);
	return m_from_mcu;
}

uint8_t superqix_state::z80_semaphore_assert_r()
{
	if(!machine().side_effects_disabled())
	{
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(superqix_state::z80_semaphore_assert_cb), this));
	}
	return 0;
}

void superqix_state::z80_ay2_iob_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(superqix_state::z80_ay2_iob_w_cb), this), data);
}

void superqix_state::mcu_port3_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(superqix_state::mcu_port3_w_cb), this), data);
}

TIMER_CALLBACK_MEMBER(superqix_state::bootleg_mcu_port1_w_cb)
{
	// on the 8031 bootleg, the low 4 bits of port1 effectively control what would normally be port2 writes
	// the 4 high bits control a multiplexer for port3 input and a latch for port3 output
	// we assume the 'fake port2 writes' are absolutely identical to those of the real game
	// 76543210
	// |||||||\- new bit for 'fake port2 latch'
	// ||||\\\-- bit selected within 'fake port2 latch'
	// |||\----- port3 input is connected to DSW1 via transparent latch if this is low
	// ||\------ port3 input is connected to SYSTEM via transparent latch if this is low
	// |\------- port3 input is connected to AY2 iob via transparent latch if this is low
	// \-------- port3 output is latched to an octal latch (which drives AY2 iob) on the rising edge of this pin
	u8 const changed_m_bl_port1 = m_bl_port1 ^ param;
	m_bl_port1 = param;

	m_bl_fake_port2 &= ~(1<<((m_bl_port1&0xe)>>1)); // mask out the 'old bit'
	m_bl_fake_port2 |= ( BIT(m_bl_port1, 0) << ((m_bl_port1&0xe)>>1) ); // or in the 'new bit'

	if (BIT(changed_m_bl_port1, 7) && BIT(m_bl_port1, 7)) // on rising edge of p1.7
	{
		if ((m_bl_port1 & 0x70) != 0x70) logerror("WARNING: port3 out latched to m_from_mcu while port3 multiplexer set to a non-open-bus value!\n");
		m_from_mcu = m_bl_port3_out; // latch port3 out to ay2 iob bus
		//note we are not doing a synchronize here, because this callback is
		//already after a synchronize, and doing another one would be redundant
	}

	mcu_port2_w(m_bl_fake_port2); // finally write to port 2, which will do another synchronize
}

void superqix_state::bootleg_mcu_port1_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(superqix_state::bootleg_mcu_port1_w_cb), this), data);
}

uint8_t superqix_state::bootleg_mcu_port3_r()
{
	if ((m_bl_port1 & 0x10) == 0)
	{
		return ioport("DSW1")->read();
	}
	else if ((m_bl_port1 & 0x20) == 0)
	{
		return ioport("SYSTEM")->read();
	}
	else if ((m_bl_port1 & 0x40) == 0)
	{
		return m_from_z80;
	}
	// There are eight vertically mounted single resistors on the 8031 bootleg
	// pcb to (presumably, needs tracing) pull this bus high when no input is
	// selected.
	// It is possible that the value of m_from_mcu will be read here rather than
	// 0xff, the circuit of the bootleg pcb needs to be fully traced out to
	// prove this.
	return 0xff;
}

void superqix_state::bootleg_mcu_port3_w(uint8_t data)
{
	// unlike the 8751, the 8031 bootleg port3 does not directly connect to ay2
	// iob; there is a 74ls374 octal latch next to the 8031 which probably
	// connects between ay2 iob and port3, and the octal latch is clocked from
	// port3 by the rising edge of 8031 p1.7.
	m_bl_port3_out = data;
}

void superqix_state::mcu_port2_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(superqix_state::mcu_port2_w_cb), this), data);
}

uint8_t superqix_state::mcu_port3_r()
{
//  logerror("%s: read Z80 command %02x\n",machine().describe_context(), m_from_z80);
	return m_from_z80;
}


uint8_t superqix_state_base::nmi_ack_r()
{
	if(!machine().side_effects_disabled())
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
	if (m_mcu.found())
		return 0xff;
	else
		return ioport("SYSTEM")->read(); // only on the bootleg sets
}

uint8_t superqix_state::bootleg_in0_r()
{
	return bitswap<8>(ioport("DSW1")->read(), 0,1,2,3,4,5,6,7);
}

void superqix_state::bootleg_flipscreen_w(uint8_t data)
{
	flip_screen_set(~data & 1);
}


/***************************************************************************

 Hot Smash Z80 <-> 68705 protection interface

 High level commands; these commands are parsed by the MCU from the z80->mcu
 register when the MCU's /INT pin is activated, which seems to occur on a
 write to the Z80->MCU register by the Z80.

 MCU Commands Legend (hotsmash)
 0x00 - Reset MCU (jumps to reset vector; does not return anything or set mcu->z80 semaphore)
 0x01 - Read Spinner Position Counter for Player 1 (p1, bits 2 and 3 quadrature, counter range is clamped to 00-7f) OR Protection Read
        Protection reads are reads of a variable length of a rom extending from MCU rom 0x80 to 0xff, and are only every even byte;
        the strange values returned by the protection functions below are actually the raw rom offset in mcu rom where the reads will come from.
        The first byte of each read is checked if it is >= or < 0x32, if it is >= it is thrown out and the next byte is ignored. if it is <, it is thrown out,
        and the next byte is returned instead of reading spinner 1. In the case where the byte of the rom WOULD BE 0xFF, instead based on the LSB of the spinner
        counter value (effectively a random coin flip) 0x8A or 0x8B is returned. This protection value might actually be the ball speed or AI aggressiveness
        per level after a certain number of ball hits/points scored, as it always increases, to a limit.
 0x02 - Read Spinner Position Counter for Player 2 (p2, bits 3 and 2 quadrature, counter range is clamped to 00-7f)
 0x04 - Read dipswitch array sw1 and send to z80
 0x08 - Read dipswitch array sw2 and send to z80 and also write them to $3b
 0x20 - Reset quadrature counters to 0x38, clears protection read enable flag, return 0x38; unlike all other return values, this is not an offset into mcu rom.
 0x40 - Reset score and start 1P vs CPU game; returns number of points per game win based on sw2:3; clears counters and clears $3a
 0x41 - Reset score and start 2P game; returns number of points per game win based on sw2:3; clears counters and sets $3a bit 0
 0x80 - Increment score for CPU/P2, reset protection read suboffset and set protection read enable flag
        If in a 2P game, if p2 scored more than sw2.5?4:3 points, and won 2 matches, clear matches won by both players and return 0xb3
        If in a 2P game, if p2 scored more than sw2.5?4:3 points, and did not yet win 2 matches, return 0x9d
        If in a 2P game, if p2 did not yet score more than sw2.5?4:3 points, return 0x89
        If in a 1P game, if cpu scored more than sw2.5?4:3 points, return 0xee
        If in a 1P game, if cpu did not yet score more than sw2.5?4:3 points, return 0xd9
 0x81 - Increment score for P1, reset protection read suboffset and set protection read enable flag
        If in a 2P game, if p1 scored more than sw2.5?4:3 points, and won 2 matches, clear matches won by both players and return 0xa8
        If in a 2P game, if p1 scored more than sw2.5?4:3 points, and did not yet win 2 matches, return 0x92
        If in a 2P game, if p1 did not yet score more than sw2.5?4:3 points, return 0x80
        If in a 1P game, if p1 scored more than sw2.5?4:3 points, and won 2 matches, clear matches won by both players and return 0xe2
        If in a 1P game, if p1 scored more than sw2.5?4:3 points, and did not yet win 2 matches, return 0xe2
        If in a 1P game, if p1 did not yet score more than sw2.5?4:3 points, return 0xd0
 0x83 - Increment score for BOTH PLAYERS, reset protection read offset and set protection read enable flag, return 0xbe
 0x84 - Reset protection read suboffset and set protection read enable flag, return 0xc9
 0xF0 - Reset protection read suboffset, return 0xf0 (sent on mcu timeout from z80 side?)
 other - Echo (returns whatever the command byte was back to the z80 immediately)

 MCU Commands Detail:
 0x00 - Reset MCU (jumps to reset vector; does not return anything or set mcu->z80 semaphore)
 0x01 - Read Spinner Position Counter for Player 1 (p1, bits 2 and 3 quadrature, counter range is clamped to 00-7f) OR Protection Read
    if $31 has bit 1 set
    jump to 239
        increment $32
        load a with $34
        add $33 to a
        transfer a to x
        load a with $x
        if a is 0
        jump to 247
            jump to 204, see below
        if a < 0x32
        jump to 24a
            increment $33
            load a with $34
            add $33 to a
            transfer a to x
            load a with $x
            if a == 0xFF
            jump to 25b
                load x with 0x10
                load a with [x+1] which is 0x11 (spinner 1 value)
                store a (value of $11) into $2a
                if $2a has bit 0 set
                jump to 269
                    load a with 0x8B
                    store a (value of $34) into $2e
                    store $2e to the mcu->z80 latch
                otherwise
                    load a with 0x8A
                    store a (value of $34) into $2e
                    store $2e to the mcu->z80 latch
            otherwise jump to 22a <- I believe this path is the 'protection succeeded' path, as it allows an offset of the first 256 bytes of the mcu rom to be read...
                store accum into $2e
                store $2e to the mcu->z80 latch
        otherwise jump to 204, see below
    204:
        load x with 0x10
        load a with [x+1] which is 0x11 (spinner 1 value)
        store a (value of $11) into $2e
        store $2e to the mcu->z80 latch
 0x02 - Read Spinner Position Counter for Player 2 (p2, bits 3 and 2 quadrature, counter range is clamped to 00-7f)
    load x with 0x1a
    load accum with [x+1] which is 0x1b (spinner 2 value)
    store accum (value of $1b) into $2e
    store $2e to the mcu->z80 latch
 0x04 - Read dipswitch array sw1 and send to z80
 0x08 - Read dipswitch array sw2 and send to z80 and also write them to $3b
    same as command 0x04, but polls the other port, and also stores the result to $3b
 0x20 - Reset quadrature counters to 0x38, clears protection read enable flag, return 0x38
    load a with 0x38
    load x with 0x10
    store a to $11
    load x with 0x1a
    store a to $1b
    clear $31
    store a into $2e
    store $2e to the mcu->z80 latch
 0x40 Reset score and start 1P vs CPU game; returns number of points per game win based on sw2:3; clears counters and clears $3a
 0x41 Reset score and start 2P game; returns number of points per game win based on sw2:3; clears counters and sets $3a bit 0
    clear $32
    clear $35
    clear $36
    clear $37
    clear $38
    if $3b has bit 4 clear (number of points per game dipswitch is set to 3)
    jump to 29c
        store 0x03 in $39
        goto in all cases:
    otherwise (number of points per game dipswitch is set to 4)
        store 0x04 in $39
        goto in all cases:
    in all cases:
****if command was 0x40: clears $31, $3a,
****if command was 0x41: clears $31, sets bit 0x01 of $3a
    store accum (value of $39) into $2e
    store $2e to the mcu->z80 latch
 0x80 - Increment score for CPU/P2, reset protection read suboffset and set protection read enable flag
    if $3a has bit 0x01 set
    jump to 2ad
        set bit 1 of $31
        clear $33
        increment $36
        check if $36 == $39, if so
        jump to 2c2
            clear $36
            clear $35
            increment $38
            check if $38 == 0x02, if so
            jump to 2d7
                clear $38
                clear $37
                store 0xb3 into $34
                clear $32
                store a (value of $34) into $2e
                store $2e to the mcu->z80 latch
            otherwise
                store 0x9d into $34
                clear $32
                store a (value of $34) into $2e
                store $2e to the mcu->z80 latch
        otherwise
            store 0x89 into $34
            clear $32
            store a (value of $34) into $2e
            store $2e to the mcu->z80 latch
    otherwise jump to 31b <- we're in a 1p game
        set bit 1 of $31
        clear $33
        increment $36
        check if $36 == $39, if so
        jump to 330
            store 0xee into $34
            clear $32
            store a (value of $34) into $2e
            store $2e to the mcu->z80 latch
        otherwise
            store 0xd9 into $34
            clear $32
            store a (value of $34) into $2e
            store $2e to the mcu->z80 latch
 0x81 - Increment score for P1, reset protection read suboffset and set protection read enable flag
    if $3a has bit 0x01 set
    jump to 2e4
        set bit 0x01 of $31
        clear $33
        increment $35
        check if $35 == $39, if so
        jump to 2f9
            clear $36
            clear $35
            increment $37
            check if $37 == 0x02, if so
            jump to 30e
                clear $38
                clear $37
                store 0xA8 into $34
                clear $32
                store a (value of $34) into $2e
                store $2e to the mcu->z80 latch
            otherwise
                store 0x92 into $34
                clear $32
                store a (value of $34) into $2e
                store $2e to the mcu->z80 latch
        otherwise
            store 0x80 to $34
            clear $32
            store a (value of $34) into $2e
            store $2e to the mcu->z80 latch
    otherwise jump to 339
        set bit 0x01 of $31
        clear $33
        increment $35
        check if $35 == $39, if so
        jump to 34e
            clear $35
            clear $36
            increment $37
            check if $37 == 0x02
            if so jump to 363
                clear $37
                clear $38
                store 0xE2 into $34
                clear $32
                store a (value of $34) into $2e
                store $2e to the mcu->z80 latch
            otherwise
                store 0xE2 into $34
                clear $32
                store a (value of $34) into $2e
                store $2e to the mcu->z80 latch
        otherwise
            store 0xd0 to $34
            clear $32
            store a (value of $34) into $2e
            store $2e to the mcu->z80 latch
 0x83 - Increment score for BOTH PLAYERS, reset protection read offset and set protection read enable flag, return 0xbe
    increment $35
    increment $36
    store 0xbe to $34
    clear $32
    clear $33
    set bit 1 of $31
    store a (value of $34) into $2e
    store $2e to the mcu->z80 latch
 0x84 - Reset protection read suboffset and set protection read enable flag, return 0xc9
    store 0xc9 to $34
    clear $32
    clear $33
    set bit 1 of $31
    store a (value of $34) into $2e
    store $2e to the mcu->z80 latch
 0xF0 - Reset protection read suboffset, return 0xf0 (sent on mcu timeout from z80 side?)
    clear $32
    clear $33
    store a (0xF0) into $2e
    store $2e to the mcu->z80 latch
 other - Echo (returns whatever the command byte was back to the z80 immediately)

 MCU idle quadrature read loop starts at 165
 MCU reset vector is 120
 The block of code between 100 and 120 is unknown.

 MCU memory addresses known:
 10 - cleared by reset, holds the player 1 raw quadrature inputs as last read in bits 2 and 3
 11 - cleared by reset, holds the player 1 spinner position counter, clamped between 0x00 and 0x7f
 13 - cleared by reset, never used by mcu code (leftover from prebillian?)
 19 - cleared by reset, never used by mcu code (leftover from prebillian?)
 1a - cleared by reset, holds the player 2 raw quadrature inputs as last read in bits 3 and 2
 1b - cleared by reset, holds the player 2 spinner position counter, clamped between 0x00 and 0x7f
 1d - cleared by reset, never used by mcu code (leftover from prebillian?)
 21 - cleared by reset, never used by mcu code (leftover from prebillian?)
 23 - cleared by reset, never used by mcu code (leftover from prebillian?)
 24 - set to 0x10 by reset, never used by mcu code (leftover from prebillian?)
 2d ? definitely used, not sure where.
 2e - temporary storage for returned 'state' values for z80
 2f - cleared by reset, holds a copy of 10 or 1a, used for quadrature decode second read
 30 - cleared by reset, holds a copy of 10 or 1a, used for quadrature decode first read
 31 - bit 1: this is only set AFTER the first ball of the game has been played, and enables protection reads
 32 - some sort of running counter; this seems to increment once every mcu poll for spinner positions; may have been intended as some sort of protection watchdog, but doesn't seem to be read anywhere?
 33 - sub-offset for protection reads (full offset is created by adding 0x33 and 0x34, when protection read enable flag $31.1 is set); incremented twice per read
 34 - holds 'state' response for most commands, actually an offset into the rom at 0x80-0xFF and is the primary offset for protection reads
 35 - number of points scored by Player 1
 36 - number of points scored by Player 2/CPU
 37 - number of matches won by Player 1 in a 1P/CPU game or VS Game
 38 - number of matches won by Player 2 in a VS Game
 39 - number of points being played for in total (3 or 4, based on sw2:5 dipswitch)
 3a - bit 0: if set: 2P/VS game; if clear: 1P/CPU game
 3b - contents of dipswitch 2; bit 0x10 (switch 5?) affects the value loaded to 39

The Prebillian/Hotsmash hardware seems to be an evolution of the arkanoid hardware in regards to the mcu:
arkanoid:
Port A[7:0] <> bidir comms with z80
Port B[7:0] <- input MUX (where does the paddle select bit come from??? port a bit 0?)
PortC[0] <- m_z80_has_written
PortC[1] <- m_mcu_has_written
PortC[2] -> high - clear m_z80_has_written and deassert MCU /INT; low - allow m_from_z80 to be read at port A
PortC[3] -> high - latch port A contents into m_from_mcu and set m_mcu_has_written; low - do nothing.

hotsmash/prebillian:
PortA[] <- input MUX
PortB[] -> output MUX
PortC[3:0] -> select one of 8 MUX selects for m_porta_in and m_portb_out
PortC[4] -> activates m_porta_in latch (active low)

 *  Port C connections:
 *
 *  0-2 W  select I/O; inputs are read from port A, outputs are written to port B
 *         000  dsw A (I)
 *         001  dsw B (I)
 *         010  not used
 *         011  from Z80 (I)
 *         100  not used
 *         101  to Z80 (O)
 *         110  P1 dial input (I)
 *         111  P2 dial input (I)
 *  3   W  clocks the active latch (active low)
 *  4-7 W  nonexistent on 68705p5

 ***************************************************************************/

/**************************************************************************

 Prebillian MCU info

Seems to act like an older version of hotsmash mcu code, the quadrature code is much messier here than in hotsmash

 MCU Commands Legend (prebillian)
 0x00 - Reset MCU
 0x01 - Read Plunger Position Counter for Player 1 or 2 (p1 plunger, bits UNKNOWN (2 and 3?) quadrature) OR (p2 plunger, bits UNKNOWN (0 and 1?) quadrature); counter range is 00-FF???
 0x02 - Read Spinner Position Counter for Player 1 or 2 (p1 spinner, bits UNKNOWN (3 and 2?) quadrature) OR (p2 spinner, bits UNKNOWN (0 and 1?) quadrature); counter range is 00-FF and wraps
 0x04 - Read dipswitch array sw1 and send to z80
 0x08 - Read dipswitch array sw2 and send to z80
 0x80 - Set commands 00 and 01 to return player 1 controls, return nothing
 0x81 - Set commands 00 and 01 to return player 2 controls, return nothing
 other - do nothing, return nothing
 Disabled/dead code MCU commands (can be enabled by patching MCU rom 0x1BA to 0x9D)
  0x03 - return mcu timer, and latch the current command (0x03) (or another byte if you write one VERY fast) to add to an accumulator
  0x0A - return the accumulator from command 0x03
  0x13 - return currently selected player number (bit0=0 for player 1, bit0=1 for player 2; upper 7 bits are a counter of how many times more or less command 80 or 81 was run; 80 increments, 81 decrements)
  0x10 - protection scramble; immediately latch the current command (0x10) (or another byte if you write one VERY fast) and do some rotates and scrambling of the value an XORing it against the prior value, and return it. This is affected by the carry flag if something else set it.

**************************************************************************/

/*
 * This wrapper routine is necessary because the dial is not connected to an
 * hardware counter as usual, but the DIR and CLOCK inputs are directly
 * connected to the 68705 which acts as a counter.
 * both hotsmash and prebillian have two dials connected this way.
 * on hotsmash only, the second player dial has the DIR and CLOCK inputs swapped
 * prebillian also has two plungers, which are connected via a standard quadrature hookup.
 * though the plungers are spring-loaded and return to one extreme when released.
 * prebillian also has a launch button which will instantly launch the ball;
 * Whether this is a secondary trigger at the innermost position of the plunger
 * (in case the quadrature is fouled and/or the plunger is slammed inward), a separate
 * panel button, or a debug button left over from development is up to debate.
 */

int hotsmash_state::read_inputs(int player) // if called with player=1, we're mux port 7, otherwise mux port 6
{
	// get the new position and adjust the result
	// dials use DIR and CLOCK?
	int const newpos_dial = m_dials[player]->read();
	// get the launch button state
	int const launchbtn_state = m_launchbtns[player]->read()&1;
	if (newpos_dial != m_dial_oldpos[player])
	{
		m_dial_sign[player] = ((newpos_dial - m_dial_oldpos[player]) & 0x80) >> 7;
		m_dial_oldpos[player] = newpos_dial;
	}
	// plungers use a plain old quadrature
	// quad1 = plunger bit 1
	// quad2 = plunger bit 0 XOR plunger bit 1
	int const newpos_plunger = m_plungers[player]->read();

	if ((player == 0) || (m_invert_p2_spinner == false))
		return (launchbtn_state<<4 | ((m_dial_oldpos[player] & 1) << 2) | (m_dial_sign[player] << 3) | (newpos_plunger&2) | ((newpos_plunger^(newpos_plunger>>1))&1) );
	else    // (player == 1) && (m_invert_p2_spinner == true)
		return (launchbtn_state<<4 | ((m_dial_oldpos[player] & 1) << 3) | (m_dial_sign[player] << 2) | (newpos_plunger&2) | ((newpos_plunger^(newpos_plunger>>1))&1) );
}

u8 hotsmash_state::hotsmash_68705_porta_r()
{
	return m_porta_in;
}

void hotsmash_state::hotsmash_68705_portb_w(u8 data)
{
	m_portb_out = data;
}

void hotsmash_state::hotsmash_68705_portc_w(u8 data)
{
	u8 const changed_m_portc_out = m_portc_out ^ data;
	m_portc_out = data;
	//logerror("%s: MCU setting MUX port to %d\n", machine().describe_context(), m_portc_out & 0x07);
	// maybe on the RISING edge of the latch bit, the semaphores are updated, like TaitoSJ?
	/*if (BIT(changed_m_portc_out, 3) && BIT(m_portc_out, 3))
	{
	    switch (m_portc_out & 0x07)
	    {
	    case 0x03:
	        m_z80_has_written = 0;
	        break;
	    case 0x05:
	        m_mcu_has_written = 1;
	        break;
	    default:
	        break;
	    }
	}*/
	// on the falling edge of the latch bit, update port A and (if applicable) m_portb_out latches
	if (BIT(changed_m_portc_out, 3) && !BIT(m_portc_out, 3))
	{
		switch (m_portc_out & 0x07)
		{
		case 0x0:   // dsw A
		case 0x1:   // dsw B
			m_porta_in = m_dsw[m_portc_out & 0x01]->read();
			break;

		case 0x3:   // Read command from Z80 to MCU, the z80->mcu semaphore is cleared on the rising edge
			//logerror("%s: command %02x read by MCU; Z80HasWritten: %d (and will be 0 after this); MCUHasWritten: %d\n",machine().describe_context(), m_from_z80, m_z80_has_written, m_mcu_has_written);
			m_mcu->set_input_line(M68705_IRQ_LINE, CLEAR_LINE);
			m_porta_in = m_from_z80;
			m_z80_has_written = 0;
			break;

		case 0x5:   // latch response from MCU to Z80; the mcu->z80 semaphore is set on the rising edge
			m_from_mcu = m_portb_out;
			//logerror("%s: response %02x written by MCU; Z80HasWritten: %d; MCUHasWritten: %d (and will be 1 after this)\n",machine().describe_context(), m_from_mcu, m_z80_has_written, m_mcu_has_written);
			m_mcu_has_written = 1;
			m_porta_in = 0xff;
			break;

		case 0x6:
		case 0x7:
			m_porta_in = read_inputs(m_portc_out & 0x01);
			break;

		default: // cases 2 and 4 presumably latch open bus/0xFF; implication from the superqix bootleg is that reading port 4 may clear the m_mcu_has_written flag, but the hotsmash MCU never touches it. Needs hardware tests/tracing to prove.
			logerror("%s: MCU attempted to read mux port %d which is invalid!\n", machine().describe_context(), m_portc_out & 0x07);
			m_porta_in = 0xff;
			break;
		}
		//if ((m_portC_out & 0x07) < 6) logerror("%s: MCU latched %02x from mux input %d m_portA_in\n", machine().describe_context(), m_portA_in, m_portC_out & 0x07);
	}
}

void hotsmash_state::hotsmash_z80_mcu_w(u8 data)
{
	m_from_z80 = data;
	//if ((m_from_z80 != 0x04) && (m_from_z80 != 0x08))
	//  logerror("%s: z80 write to MCU %02x; Z80HasWritten: %d (and will be 1 after this); MCUHasWritten: %d\n",machine().describe_context(), m_from_z80, m_z80_has_written, m_mcu_has_written);
	m_z80_has_written = 1; // set the semaphore, and assert interrupt on the mcu
	machine().scheduler().perfect_quantum(attotime::from_usec(250)); //boost the interleave temporarily, or the game will crash.
	m_mcu->set_input_line(M68705_IRQ_LINE, ASSERT_LINE);
}

u8 hotsmash_state::hotsmash_z80_mcu_r()
{
	if(!machine().side_effects_disabled())
	{
		//if ((m_from_z80 != 0x04) && (m_from_z80 != 0x08))
		//  logerror("%s: z80 read from MCU %02x; Z80HasWritten: %d; MCUHasWritten: %d (and will be 0 after this)\n",machine().describe_context(), m_from_mcu, m_z80_has_written, m_mcu_has_written);
		m_mcu_has_written = 0;
	}
	// return the last value the 68705 wrote, but do not mark that we've read it
	return m_from_mcu;
}

ioport_value hotsmash_state::pbillian_semaphore_input_r()
{
	ioport_value res = 0;
	// bit 0x40 is PROBABLY latch 1 on 74ls74.7c, is high if m_z80_has_written is clear
	if (!m_z80_has_written)
		res |= 0x01;

	// bit 0x80 is PROBABLY latch 2 on 74ls74.7c, is high if m_mcu_has_written is clear
	// prebillian code at 0x6771 will wait in a loop reading ay port E forever and waiting
	// for bit 7 to be clear before it will read from the mcu
	if (!m_mcu_has_written)
		res |= 0x02;
	return res;
}


void superqix_state_base::machine_init_common()
{

	// commmon 68705/8751/HLE
	save_item(NAME(m_mcu_has_written));
	save_item(NAME(m_z80_has_written));
	save_item(NAME(m_from_mcu));
	save_item(NAME(m_from_z80));

	//general machine stuff
	save_item(NAME(m_invert_coin_lockout));
	save_item(NAME(m_invert_p2_spinner));
	save_item(NAME(m_nmi_mask));

	// superqix specific stuff, TODO: should be moved to superqix_state below
	save_item(NAME(m_gfxbank));
	save_item(NAME(m_show_bitmap));
	// the following are saved in VIDEO_START_MEMBER(superqix_state,superqix):
	//save_item(NAME(*m_fg_bitmap[0]));
	//save_item(NAME(*m_fg_bitmap[1]));

	m_z80_has_written = false;
}

void superqix_state::machine_init_common()
{
	superqix_state_base::machine_init_common();

	// 8031 and/or 8751 MCU related
	save_item(NAME(m_bl_port1));
	save_item(NAME(m_bl_fake_port2));
	save_item(NAME(m_port2_raw));
	save_item(NAME(m_bl_port3_out));
}

void hotsmash_state::machine_init_common()
{
	superqix_state_base::machine_init_common();

	// 68705 related
	save_item(NAME(m_portb_out));
	save_item(NAME(m_portc_out));

	// spinner quadrature stuff
	save_item(NAME(m_dial_oldpos));
	save_item(NAME(m_dial_sign));
}

MACHINE_RESET_MEMBER(superqix_state, superqix)
{
	if (m_mcu.found()) // mcu sets only
	{
		// on reset, the mcu is reset, and the mcu p2 latch is explicitly cleared by the reset generator;
		// the act of clearing this latch asserts the z80 reset, and the mcu must clear it itself by writing
		// to the p2 latch with bit 5 set.
		m_port2_raw = 0x01; // force the following function into latching a zero write by having bit 0 falling edge
		mcu_port2_w(0x00);
		m_mcu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	}
}

void superqix_state::machine_start()
{
	/* configure the banks */
	membank("bank1")->configure_entries(0, 4, memregion("maincpu")->base() + 0x10000, 0x4000);

	machine_init_common();
}

void hotsmash_state::machine_start()
{
	/* configure the banks */
	membank("bank1")->configure_entries(0, 2, memregion("maincpu")->base() + 0x10000, 0x4000);

	machine_init_common();
}


void superqix_state_base::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	// the following four ranges are part of a single 6264 64Kibit SRAM chip, called 'VRAM' in POST
	map(0xe000, 0xe0ff).ram().share("spriteram");
	map(0xe100, 0xe7ff).ram();
	map(0xe800, 0xefff).ram().w(FUNC(superqix_state_base::superqix_videoram_w)).share("videoram");
	map(0xf000, 0xffff).ram();
}

void hotsmash_state::pbillian_port_map(address_map &map)
{ // used by both pbillian and hotsmash
	map(0x0000, 0x01ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette"); // 6116 sram near the jamma connector, "COLOR RAM" during POST
	//map(0x0200, 0x03ff).ram(); // looks like leftover crap from a dev board which had double the color ram? zeroes written here, never read.
	map(0x0401, 0x0401).r(m_ay1, FUNC(ay8910_device::data_r)); // ay i/o ports connect to "SYSTEM" and "BUTTONS" inputs which includes mcu semaphore flags
	map(0x0402, 0x0403).w(m_ay1, FUNC(ay8910_device::data_address_w));
	map(0x0408, 0x0408).rw(FUNC(hotsmash_state::hotsmash_z80_mcu_r), FUNC(hotsmash_state::hotsmash_z80_mcu_w));
	map(0x0410, 0x0410).w(FUNC(hotsmash_state::pbillian_0410_w)); /* Coin Counters, ROM bank, NMI enable, Flipscreen */
	map(0x0418, 0x0418).r(FUNC(hotsmash_state::nmi_ack_r));
	map(0x0419, 0x0419).nopw(); // ??? is this a watchdog, or something else? manual reset of mcu semaphores? manual nmi TRIGGER? used by prebillian
	map(0x041a, 0x041a).w(FUNC(hotsmash_state::pbillian_sample_trigger_w));
	map(0x041b, 0x041b).nopr();  // input related? but probably not used, may be 'sample has stopped playing' flag? used by prebillian
}

void hotsmash_state::pbillianb_port_map(address_map &map)
{
	pbillian_port_map(map);
	map(0x0408, 0x0408).unmaprw(); // no MCU on PCB, still reads/writes here
	map(0x0c00, 0x0c00).portr("DSW1");
	map(0x0c01, 0x0c01).portr("DSW2");
	map(0x0c06, 0x0c06).portr("CONTROLS");
	map(0x0c07, 0x0c07).portr("CONTROLS2");
}

void superqix_state::sqix_port_map(address_map &map)
{
	map(0x0000, 0x00ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x0401, 0x0401).r(m_ay1, FUNC(ay8910_device::data_r));
	map(0x0402, 0x0402).w(m_ay1, FUNC(ay8910_device::data_w));
	map(0x0403, 0x0403).w(FUNC(superqix_state::z80_ay1_sync_address_w)); // sync on address write, so semaphores are accurately read
	map(0x0405, 0x0405).r(m_ay2, FUNC(ay8910_device::data_r));
	map(0x0406, 0x0407).w(m_ay2, FUNC(ay8910_device::data_address_w));
	map(0x0408, 0x0408).r(FUNC(superqix_state::z80_semaphore_assert_r));
	map(0x0410, 0x0410).w(FUNC(superqix_state::superqix_0410_w));  /* ROM bank, NMI enable, tile bank, bitmap bank */
	map(0x0418, 0x0418).r(FUNC(superqix_state::nmi_ack_r));
	// following two ranges are made of two 64kx4 4464 DRAM chips at 9L and 9M, "GRAPHICS RAM" or "GRP BIT" if there is an error in POST
	map(0x0800, 0x77ff).ram().w(FUNC(superqix_state::superqix_bitmapram_w)).share("bitmapram");
	map(0x8800, 0xf7ff).ram().w(FUNC(superqix_state::superqix_bitmapram2_w)).share("bitmapram2");
	//map(0xf970, 0xfa6f).ram(); // this is probably a portion of the remainder of the chips at 9L and 9M which isn't used or tested for graphics ram
}

void superqix_state::sqix_8031_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("mcu", 0); // external program ROM
}



static INPUT_PORTS_START( pbillian )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x18, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                    PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "10/20/300K Points" )
	PORT_DIPSETTING(    0x00, "10/30/500K Points" )
	PORT_DIPSETTING(    0x08, "20/30/400K Points" )
	PORT_DIPSETTING(    0x04, "30/40/500K Points" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM") // ay port B (register F)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // hblank?
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("BUTTONS") // ay port A (register E)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )     // N/C
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )    // P1 fire (M powerup) + high score initials
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     // N/C
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL  // P2 fire (M powerup) + high score initials
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(hotsmash_state, pbillian_semaphore_input_r)  // Z80 and MCU Semaphores

	PORT_START("PLUNGER1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(16)

	PORT_START("DIAL1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(20) PORT_KEYDELTA(8)

	PORT_START("LAUNCH1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PLUNGER2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(16) PORT_COCKTAIL

	PORT_START("DIAL2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(20) PORT_KEYDELTA(8) PORT_COCKTAIL

	PORT_START("LAUNCH2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END

static INPUT_PORTS_START( pbillianb )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x18, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                    PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "10/20/300K Points" )
	PORT_DIPSETTING(    0x00, "10/30/500K Points" )
	PORT_DIPSETTING(    0x08, "20/30/400K Points" )
	PORT_DIPSETTING(    0x04, "30/40/500K Points" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM") // ay port B (register F)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // hblank?
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("CONTROLS") // 0xc06: both players in upright, player 1 in cocktail
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("CONTROLS2") // 0xc07: player 2 in cocktail
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS") // ay port A (register E)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )     // N/C
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )    // P1 fire (M powerup) + high score initials
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )     // N/C
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL  // P2 fire (M powerup) + high score initials
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Z80 and MCU Semaphores for the parent, needs to be high in this MCU-less bootleg, too

INPUT_PORTS_END

static INPUT_PORTS_START( hotsmash )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Difficulty vs. CPU" )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Difficulty vs. 2P" )
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, "Points per game" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM") // ay port B (register F)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )//$49c
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )//$42d
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // hblank?
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("BUTTONS") // ay port A (register E)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) // p1 button 2, unused on this game?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL  // p2 button 2, unused on this game?
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(hotsmash_state, pbillian_semaphore_input_r)  // Z80 and MCU Semaphores

	PORT_START("PLUNGER1")  // plunger isn't present on hotsmash though the pins exist for it
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_PLAYER(1)

	PORT_START("DIAL1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(15) PORT_KEYDELTA(30) PORT_CENTERDELTA(0) PORT_PLAYER(1)

	PORT_START("LAUNCH1")  // launch button isn't present on hotsmash
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PLUNGER2")  // plunger isn't present on hotsmash though the pins exist for it
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_PLAYER(2)

	PORT_START("DIAL2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(15) PORT_KEYDELTA(30) PORT_CENTERDELTA(0) PORT_PLAYER(2)

	PORT_START("LAUNCH2")  // launch button isn't present on hotsmash
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_PLAYER(2)

INPUT_PORTS_END


static INPUT_PORTS_START( superqix )
	PORT_START("DSW1")  /* DSW1 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Freeze" )                    PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START("DSW2")  /* DSW2 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "20000 50000" )
	PORT_DIPSETTING(    0x0c, "30000 100000" )
	PORT_DIPSETTING(    0x04, "50000 100000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0xc0, 0xc0, "Fill Area" )                 PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, "70%" )
	PORT_DIPSETTING(    0xc0, "75%" )
	PORT_DIPSETTING(    0x40, "80%" )
	PORT_DIPSETTING(    0x00, "85%" )

	PORT_START("SYSTEM") /* Port 0 of MCU, on bootlegs this is readable by z80 at io 0x0418 (nmi ack read port) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) // JAMMA #16
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) // JAMMA #T
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 ) // JAMMA #17
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 ) // JAMMA #U
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 ) // JAMMA #R ("Service")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT ) // JAMMA #S ("Tilt")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 ) // JAMMA #15 ("Test")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(superqix_state, fromz80_semaphore_input_r)  // 74ls74 @C2 pin 8 (/Q2), this is the z80->mcu semaphore

	PORT_START("P1") /* AY-3-8910 #1 @3P Port A */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY // JAMMA #18
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY // JAMMA #19
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY // JAMMA #20
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY // JAMMA #21
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) // JAMMA #22
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) // JAMMA #23
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")   /* ??? where does this come from?  */
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW ) // ??? where does this come from?

	PORT_START("P2") /* AY-3-8910 #1 @3P Port B */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL // JAMMA #V
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL // JAMMA #W
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL // JAMMA #X
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL // JAMMA #Y
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL // JAMMA #Z
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL // JAMMA #a
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(superqix_state, frommcu_semaphore_input_r) // 74ls174 @1J pin 5 (Q1), this is the mcu->z80 semaphore
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(superqix_state, fromz80_semaphore_input_r) // 74ls74 @C2 pin 8 (/Q2), this is the z80->mcu semaphore

INPUT_PORTS_END



static const gfx_layout pbillian_charlayout =
{
	8,8,
	0x800,  /* doesn't use the whole ROM space */
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
			32*8+0*4, 32*8+1*4, 32*8+2*4, 32*8+3*4, 32*8+4*4, 32*8+5*4, 32*8+6*4, 32*8+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	128*8
};


static GFXDECODE_START( gfx_pbillian )
	GFXDECODE_ENTRY( "gfx1", 0, pbillian_charlayout, 16*16, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,            0, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_sqix )
	GFXDECODE_ENTRY( "gfx1", 0x00000, gfx_8x8x4_packed_msb,   0, 16 )    /* Chars */
	GFXDECODE_ENTRY( "gfx2", 0x00000, gfx_8x8x4_packed_msb,   0, 16 )    /* Background tiles */
	GFXDECODE_ENTRY( "gfx3", 0x00000, spritelayout,           0, 16 )    /* Sprites */
GFXDECODE_END


void hotsmash_state::vblank_irq(int state)
{
	if (state && m_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

INTERRUPT_GEN_MEMBER(superqix_state::sqix_timer_irq)
{
	if (m_nmi_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}



void hotsmash_state::pbillian(machine_config &config)
{
	Z80(config, m_maincpu, XTAL(12'000'000)/2); /* 6 MHz, ROHM Z80B */
	m_maincpu->set_addrmap(AS_PROGRAM, &hotsmash_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &hotsmash_state::pbillian_port_map);

	m68705p5_device &mcu(M68705P5(config, m_mcu, XTAL(12'000'000)/4)); /* 3mhz???? */
	mcu.porta_r().set(FUNC(hotsmash_state::hotsmash_68705_porta_r));
	mcu.portb_w().set(FUNC(hotsmash_state::hotsmash_68705_portb_w));
	mcu.portc_w().set(FUNC(hotsmash_state::hotsmash_68705_portc_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(hotsmash_state::screen_update_pbillian));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(hotsmash_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pbillian);
	PALETTE(config, m_palette).set_format(1, &hotsmash_state::BBGGRRII, 512);

	SPEAKER(config, "mono").front_center();

	AY8910(config, m_ay1, XTAL(12'000'000)/8); // AY-3-8910A
	m_ay1->port_a_read_callback().set_ioport("BUTTONS");
	m_ay1->port_b_read_callback().set_ioport("SYSTEM");
	m_ay1->add_route(ALL_OUTPUTS, "mono", 0.30);

	SAMPLES(config, m_samples);
	m_samples->set_channels(1);
	m_samples->set_samples_start_callback(FUNC(hotsmash_state::pbillian_sh_start));
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void hotsmash_state::pbillianb(machine_config &config)
{
	pbillian(config);

	m_maincpu->set_addrmap(AS_IO, &hotsmash_state::pbillianb_port_map);

	config.device_remove("mcu");
}

void superqix_state::sqix(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(12'000'000)/2); /* Z80B, 12 MHz / 2 (6 MHz), verified from pcb tracing */
	m_maincpu->set_addrmap(AS_PROGRAM, &superqix_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &superqix_state::sqix_port_map);
	m_maincpu->set_periodic_int(FUNC(superqix_state::sqix_timer_irq), attotime::from_hz(4*60)); /* ??? */

	i8751_device &mcu(I8751(config, m_mcu, XTAL(12'000'000)/2)); /* i8751-88, 12 MHz / 2 (6 MHz), verified from pcb tracing */
	mcu.port_in_cb<0>().set_ioport("SYSTEM");
	mcu.port_in_cb<1>().set_ioport("DSW1");
	mcu.port_out_cb<2>().set(FUNC(superqix_state::mcu_port2_w));
	mcu.port_in_cb<3>().set(FUNC(superqix_state::mcu_port3_r));
	mcu.port_out_cb<3>().set(FUNC(superqix_state::mcu_port3_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(superqix_state::screen_update_superqix));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sqix);
	PALETTE(config, m_palette).set_format(1, &superqix_state::BBGGRRII, 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY8910(config, m_ay1, XTAL(12'000'000)/8); // AY-3-8910A @3P, analog outputs directly tied together
	m_ay1->set_flags(AY8910_SINGLE_OUTPUT);
	m_ay1->port_a_read_callback().set_ioport("P1");
	m_ay1->port_b_read_callback().set_ioport("P2");
	m_ay1->add_route(ALL_OUTPUTS, "mono", 0.25);

	AY8910(config, m_ay2, XTAL(12'000'000)/8); // AY-3-8910A @3M, analog outputs directly tied together
	m_ay2->set_flags(AY8910_SINGLE_OUTPUT);
	m_ay2->port_a_read_callback().set_ioport("DSW2");
	m_ay2->port_b_read_callback().set(FUNC(superqix_state::z80_ay2_iob_r));
	m_ay2->port_b_write_callback().set(FUNC(superqix_state::z80_ay2_iob_w));
	m_ay2->add_route(ALL_OUTPUTS, "mono", 0.25);
}


void superqix_state::sqix_8031(machine_config &config)
{
	sqix(config);

	i8031_device &mcu(I8031(config.replace(), m_mcu, XTAL(12'000'000)/2)); /* p8031ah, clock not verified */
	mcu.set_addrmap(AS_PROGRAM, &superqix_state::sqix_8031_map);
	mcu.port_out_cb<1>().set(FUNC(superqix_state::bootleg_mcu_port1_w));
	mcu.port_in_cb<3>().set(FUNC(superqix_state::bootleg_mcu_port3_r));
	mcu.port_out_cb<3>().set(FUNC(superqix_state::bootleg_mcu_port3_w));
}

void superqix_state::sqix_nomcu(machine_config &config)
{
	sqix(config);

	config.device_remove("mcu");

	m_ay2->port_b_read_callback().set(FUNC(superqix_state::bootleg_in0_r));
	m_ay2->port_b_write_callback().set_nop();
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

/* Prebillian pcbs do not have the usual Taito letter-number pair code on the
labels on the roms/MCU, they only have the mitsubishi electric logo and a
single number.
The PCB has a label which says "M6100211A // プレビリアン" (PuReBiRiAN)
*/
ROM_START( pbillian )
	ROM_REGION( 0x018000, "maincpu", 0 )
	ROM_LOAD( "mitsubishi__electric__1.m5l27256k.6bc",  0x00000, 0x08000, CRC(d379fe23) SHA1(e147a9151b1cdeacb126d9713687bd0aa92980ac) )
	ROM_LOAD( "mitsubishi__electric__2.m5l27128k.6d",  0x14000, 0x04000, CRC(1af522bc) SHA1(83e002dc831bfcedbd7096b350c9b34418b79674) )

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "mitsubishi__electric__7.mc68705p5s.7k", 0x0000, 0x0800, CRC(03de0c74) SHA1(ee2bc8be9bab9557c6776b996b85ed6f32300b47) )

	ROM_REGION( 0x8000, "samples", 0 )
	ROM_LOAD( "mitsubishi__electric__3.m5l27256k.7h",  0x0000, 0x08000, CRC(3f9bc7f1) SHA1(0b0c2ec3bea6a7f3fc6c0c8b750318f3f9ec3d1f) )

	ROM_REGION( 0x018000, "gfx1", 0 )
	ROM_LOAD( "mitsubishi__electric__4.m5l27256k.1n",  0x00000, 0x08000, CRC(9c08a072) SHA1(25f31fcf72216cf42528b07ad8c09113aa69861a) )
	ROM_LOAD( "mitsubishi__electric__5.m5l27256k.1r",  0x08000, 0x08000, CRC(2dd5b83f) SHA1(b05e3a008050359d0207757b9cbd8cee87abc697) )
	ROM_LOAD( "mitsubishi__electric__6.m5l27256k.1t",  0x10000, 0x08000, CRC(33b855b0) SHA1(5a1df4f82fc0d6f78883b759fd61f395942645eb) )
ROM_END

ROM_START( pbillianb ) // EV-97 PCB
	ROM_REGION( 0x018000, "maincpu", 0 )
	ROM_LOAD( "1",  0x00000, 0x08000, CRC(cd8e34f0) SHA1(6598a594b4e305a6139d03e6d1d564c093da713a) ) // different
	ROM_LOAD( "2",  0x10000, 0x08000, CRC(e60a2cb0) SHA1(949bb7ff5df36ee10d910a33f595e6a90350304e) ) // 2nd half identical to the corresponding ROM of the parent

	ROM_REGION( 0x8000, "samples", 0 ) // identical to the parent
	ROM_LOAD( "3",  0x0000, 0x08000, CRC(3f9bc7f1) SHA1(0b0c2ec3bea6a7f3fc6c0c8b750318f3f9ec3d1f) )

	ROM_REGION( 0x018000, "gfx1", 0 ) // only 5 differs from the parent
	ROM_LOAD( "4",  0x00000, 0x08000, CRC(9c08a072) SHA1(25f31fcf72216cf42528b07ad8c09113aa69861a) )
	ROM_LOAD( "5",  0x08000, 0x08000, CRC(63f3437b) SHA1(a2234019bac27256692d4b059d39caaddfb9fb08) )
	ROM_LOAD( "6",  0x10000, 0x08000, CRC(33b855b0) SHA1(5a1df4f82fc0d6f78883b759fd61f395942645eb) )
ROM_END

ROM_START( hotsmash )
	ROM_REGION( 0x018000, "maincpu", 0 )
	ROM_LOAD( "b18-04",  0x00000, 0x08000, CRC(981bde2c) SHA1(ebcc901a036cde16b33d534d423500d74523b781) )

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "b18-06.mcu", 0x0000, 0x0800, CRC(67c0920a) SHA1(23a294892823d1d9216ea8ddfa9df1c8af149477) ) // has valid reset vector and int vector in it, SWI and TIMER vectors are NOPs

	ROM_REGION( 0x8000, "samples", 0 )
	ROM_LOAD( "b18-05",  0x0000, 0x08000, CRC(dab5e718) SHA1(6cf6486f283f5177dfdc657b1627fbfa3f0743e8) )

	ROM_REGION( 0x018000, "gfx1", 0 )
	ROM_LOAD( "b18-01",  0x00000, 0x08000, CRC(870a4c04) SHA1(a029108bcda40755c8320d2ee297f42d816aa7c0) )
	ROM_LOAD( "b18-02",  0x08000, 0x08000, CRC(4e625cac) SHA1(2c21b32240eaada9a5f909a2ec5b335372c8c994) )
	ROM_LOAD( "b18-03",  0x14000, 0x04000, CRC(1c82717d) SHA1(6942c8877e24ac51ed71036e771a1655d82f3491) )
ROM_END

ROM_START( sqix ) // It is unclear what this set fixes vs 1.1 below, but the 'rug pattern' on the bitmap test during POST has the left edge entirely black, unlike v1.0 or v1.1, but like sqixu
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b03__01-2.ef3",  0x00000, 0x08000, CRC(5ded636b) SHA1(827954001b4617b3bd439be75094d8dca06ea32b) )
	ROM_LOAD( "b03__02.h3",     0x10000, 0x10000, CRC(9c23cb64) SHA1(7e04cb18cabdc0031621162cbc228cd95875a022) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* I8751 code */
	ROM_LOAD( "b03__03.l2",     0x00000, 0x1000, CRC(f0c3af2b) SHA1(6dce2175011b5c8d0f1bce433c53979841d5d1a4) ) /* B03 // 03 C8751-88 MCU, verified from deprotected part */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "b03__04.s8",    0x00000, 0x08000, CRC(f815ef45) SHA1(4189d455b6ccf3ae922d410fb624c4665203febf) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "taito_sq-iu3__lh231041__sharp_japan__8709_d.p8",    0x00000, 0x20000, CRC(b8d0c493) SHA1(ef5d62ef3835c7ae088a7aa98945f747130fe0ec) ) /* Sharp LH231041 28 pin 128K x 8bit mask rom */

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "b03__05.t8",    0x00000, 0x10000, CRC(df326540) SHA1(1fe025edcd38202e24c4e1005f478b6a88533453) )
ROM_END

ROM_START( sqixr1 ) // This set has the coin lockout polarity inverted, and also fixes the 10 vs 9 lockout bug
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b03__01-1.ef3",  0x00000, 0x08000, CRC(ad614117) SHA1(c461f00a2aecde1bc3860c15a3c31091b14665a2) )
	ROM_LOAD( "b03__02.h3",     0x10000, 0x10000, CRC(9c23cb64) SHA1(7e04cb18cabdc0031621162cbc228cd95875a022) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* I8751 code */
	ROM_LOAD( "b03__03.l2",     0x00000, 0x1000, CRC(f0c3af2b) SHA1(6dce2175011b5c8d0f1bce433c53979841d5d1a4) ) /* B03 // 03 C8751-88 MCU, verified from deprotected part */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "b03__04.s8",    0x00000, 0x08000, CRC(f815ef45) SHA1(4189d455b6ccf3ae922d410fb624c4665203febf) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "taito_sq-iu3__lh231041__sharp_japan__8709_d.p8",    0x00000, 0x20000, CRC(b8d0c493) SHA1(ef5d62ef3835c7ae088a7aa98945f747130fe0ec) ) /* Sharp LH231041 28 pin 128K x 8bit mask rom */

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "b03__05.t8",    0x00000, 0x10000, CRC(df326540) SHA1(1fe025edcd38202e24c4e1005f478b6a88533453) )
ROM_END

ROM_START( sqixr0 ) // This set is older than the above two: it has the coin lockout only trigger after 10 coins (causing the last coin to be lost), and the coin lockout polarity is not inverted
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b03__01.ef3",    0x00000, 0x08000, CRC(0888b7de) SHA1(de3e4637436de185f43d2ad4186d4cfdcd4d33d9) )
	ROM_LOAD( "b03__02.h3",     0x10000, 0x10000, CRC(9c23cb64) SHA1(7e04cb18cabdc0031621162cbc228cd95875a022) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* I8751 code */
	ROM_LOAD( "b03__03.l2",     0x00000, 0x1000, CRC(f0c3af2b) SHA1(6dce2175011b5c8d0f1bce433c53979841d5d1a4) ) /* B03 // 03 C8751-88 MCU, verified from deprotected part */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "b03__04.s8",    0x00000, 0x08000, CRC(f815ef45) SHA1(4189d455b6ccf3ae922d410fb624c4665203febf) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "taito_sq-iu3__lh231041__sharp_japan__8709_d.p8",    0x00000, 0x20000, CRC(b8d0c493) SHA1(ef5d62ef3835c7ae088a7aa98945f747130fe0ec) ) /* Sharp LH231041 28 pin 128K x 8bit mask rom */

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "b03__05.t8",    0x00000, 0x10000, CRC(df326540) SHA1(1fe025edcd38202e24c4e1005f478b6a88533453) )
ROM_END

ROM_START( sqixu )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b03__06.ef3",   0x00000, 0x08000, CRC(4f59f7af) SHA1(6ea627ea8505cf8d1a5a1350258180c61fbd1ed9) )
	ROM_LOAD( "b03__07.h3",    0x10000, 0x10000, CRC(4c417d4a) SHA1(de46551da1b27312dca40240a210e77595cf9dbd) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* I8751 code */
	ROM_LOAD( "b03__08.l2",    0x00000, 0x01000, CRC(7c338c0f) SHA1(b91468c881641f807067835b2dd490cd3e3c577e) ) /* B03 // 08 C8751-88 MCU, verified from deprotected part, 3 bytes different from B03 // 03 */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "b03__04.s8",    0x00000, 0x08000, CRC(f815ef45) SHA1(4189d455b6ccf3ae922d410fb624c4665203febf) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "taito_sq-iu3__lh231041__sharp_japan__8709_d.p8",    0x00000, 0x20000, CRC(b8d0c493) SHA1(ef5d62ef3835c7ae088a7aa98945f747130fe0ec) ) /* Sharp LH231041 28 pin 128K x 8bit mask rom */

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "b03__09.t8",    0x00000, 0x10000, CRC(69d2a84a) SHA1(b461d8a01f73c6aaa4aac85602c688c111bdca5d) )
ROM_END

/* this is a bootleg with an 8031+external rom in place of the 8751 of the
   original board; The MCU code is extensively hacked to avoid use of ports 0
   and 2, which are used as the rom data and address buses, using a multiplexed
   latch on the other ports instead. This bootleg MCU is based on a dump of the
   original b03__03.l2 code, obtained by the pirates through unknown means.
   Barring the bootleg MCU, the actual rom set is an exact copy of sqixr0 above. */
ROM_START( sqixb1 ) // formerly 'sqixa'
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "sq01.97",       0x00000, 0x08000, CRC(0888b7de) SHA1(de3e4637436de185f43d2ad4186d4cfdcd4d33d9) ) // == b03__01.ef3
	ROM_LOAD( "b03__02.h3",     0x10000, 0x10000, CRC(9c23cb64) SHA1(7e04cb18cabdc0031621162cbc228cd95875a022) ) // actual label is something different on the bootleg

	ROM_REGION( 0x01000, "mcu", 0 ) /* I8031 code */
	ROM_LOAD( "sq07.ic108",     0x00000, 0x1000, CRC(d11411fb) SHA1(31183f433596c4d2503c01f6dc8d91024f2cf5de) ) // actual label is something different on the bootleg

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "b03__04.s8",    0x00000, 0x08000, CRC(f815ef45) SHA1(4189d455b6ccf3ae922d410fb624c4665203febf) ) // actual label is something different on the bootleg

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "b03-03",       0x00000, 0x10000, CRC(6e8b6a67) SHA1(c71117cc880a124c46397c446d1edc1cbf681200) ) /* == 1st half of taito_sq-iu3__lh231041__sharp_japan__8709_d.p8, fake label */
	ROM_LOAD( "b03-06",       0x10000, 0x10000, CRC(38154517) SHA1(703ad4cfe54a4786c67aedcca5998b57f39fd857) ) /* == 2nd half of taito_sq-iu3__lh231041__sharp_japan__8709_d.p8, fake label */

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "b03__05.t8",    0x00000, 0x10000, CRC(df326540) SHA1(1fe025edcd38202e24c4e1005f478b6a88533453) ) // actual label is something different on the bootleg
ROM_END

ROM_START( sqixb2 ) // this bootleg set has been extensively hacked to avoid using the MCU at all, though a few checks for the semaphore flags were never patched out
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cpu.2",         0x00000, 0x08000, CRC(682e28e3) SHA1(fe9221d26d7397be5a0fc8fdc51672b5924f3cf2) )
	ROM_LOAD( "b03__02.h3",     0x10000, 0x10000, CRC(9c23cb64) SHA1(7e04cb18cabdc0031621162cbc228cd95875a022) ) // actual label is something different on the bootleg

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "b03__04.s8",    0x00000, 0x08000, CRC(f815ef45) SHA1(4189d455b6ccf3ae922d410fb624c4665203febf) ) // actual label is something different on the bootleg

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "b03-03",       0x00000, 0x10000, CRC(6e8b6a67) SHA1(c71117cc880a124c46397c446d1edc1cbf681200) ) /* == 1st half of taito_sq-iu3__lh231041__sharp_japan__8709_d.p8, fake label */
	ROM_LOAD( "b03-06",       0x10000, 0x10000, CRC(38154517) SHA1(703ad4cfe54a4786c67aedcca5998b57f39fd857) ) /* == 2nd half of taito_sq-iu3__lh231041__sharp_japan__8709_d.p8, fake label */

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "b03__05.t8",    0x00000, 0x10000, CRC(df326540) SHA1(1fe025edcd38202e24c4e1005f478b6a88533453) ) // actual label is something different on the bootleg
ROM_END

ROM_START( perestrof )
	ROM_REGION( 0x20000, "maincpu", 0 )
	/* 0x8000 - 0x10000 in the rom is empty anyway */
	ROM_LOAD( "rom1.bin",        0x00000, 0x20000, CRC(0cbf96c1) SHA1(cf2b1367887d1b8812a56aa55593e742578f220c) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "rom4.bin",       0x00000, 0x10000, CRC(c56122a8) SHA1(1d24b2f0358e14aca5681f92175869224584a6ea) ) /* both halves identical */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "rom2.bin",       0x00000, 0x20000, CRC(36f93701) SHA1(452cb23efd955c6c155cef2b1b650e253e195738) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "rom3.bin",       0x00000, 0x10000, CRC(00c91d5a) SHA1(fdde56d3689a47e6bfb296e442207b93b887ec7a) )
ROM_END

ROM_START( perestro )
	ROM_REGION( 0x20000, "maincpu", 0 )
	/* 0x8000 - 0x10000 in the rom is empty anyway */
	ROM_LOAD( "rom1.bin",        0x00000, 0x20000, CRC(0cbf96c1) SHA1(cf2b1367887d1b8812a56aa55593e742578f220c) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "rom4.bin",       0x00000, 0x10000, CRC(c56122a8) SHA1(1d24b2f0358e14aca5681f92175869224584a6ea) ) /* both halves identical */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "rom2.bin",       0x00000, 0x20000, CRC(36f93701) SHA1(452cb23efd955c6c155cef2b1b650e253e195738) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "rom3a.bin",       0x00000, 0x10000, CRC(7a2a563f) SHA1(e3654091b858cc80ec1991281447fc3622a0d4f9) )
ROM_END

void superqix_state_base::init_sqix()
{
	m_invert_coin_lockout = true;
}

void superqix_state_base::init_sqixr0()
{
	m_invert_coin_lockout = false;
}

void superqix_state_base::init_perestro()
{
	uint8_t *src;
	int len;
	uint8_t temp[16];
	int i,j;

	/* decrypt program code; the address lines are shuffled around in a non-trivial way */
	src = memregion("maincpu")->base();
	len = memregion("maincpu")->bytes();
	for (i = 0;i < len;i += 16)
	{
		memcpy(temp,&src[i],16);
		for (j = 0;j < 16;j++)
		{
			static const int convtable[16] =
			{
				0xc, 0x9, 0xb, 0xa,
				0x8, 0xd, 0xf, 0xe,
				0x4, 0x1, 0x3, 0x2,
				0x0, 0x5, 0x7, 0x6
			};

			src[i+j] = temp[convtable[j]];
		}
	}

	/* decrypt gfx ROMs; simple bit swap on the address lines */
	src = memregion("gfx1")->base();
	len = memregion("gfx1")->bytes();
	for (i = 0;i < len;i += 16)
	{
		memcpy(temp,&src[i],16);
		for (j = 0;j < 16;j++)
		{
			src[i+j] = temp[bitswap<8>(j,7,6,5,4,3,2,0,1)];
		}
	}

	src = memregion("gfx2")->base();
	len = memregion("gfx2")->bytes();
	for (i = 0;i < len;i += 16)
	{
		memcpy(temp,&src[i],16);
		for (j = 0;j < 16;j++)
		{
			src[i+j] = temp[bitswap<8>(j,7,6,5,4,0,1,2,3)];
		}
	}

	src = memregion("gfx3")->base();
	len = memregion("gfx3")->bytes();
	for (i = 0;i < len;i += 16)
	{
		memcpy(temp,&src[i],16);
		for (j = 0;j < 16;j++)
		{
			src[i+j] = temp[bitswap<8>(j,7,6,5,4,1,0,3,2)];
		}
	}
}

void superqix_state_base::init_pbillian()
{
	m_invert_p2_spinner = false;
}

void superqix_state_base::init_hotsmash()
{
	m_invert_p2_spinner = true;
}

/*    YEAR  NAME       PARENT    MACHINE     INPUT      CLASS           INIT           ROT    COMPANY                             FULLNAME */
GAME( 1986, pbillian,  0,        pbillian,   pbillian,  hotsmash_state, init_pbillian, ROT0,  "Kaneko / Taito",                   "Prebillian", MACHINE_SUPPORTS_SAVE )
GAME( 1987, pbillianb, pbillian, pbillianb,  pbillianb, hotsmash_state, init_pbillian, ROT0,  "bootleg (Game Corp.)",             "Prebillian (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, hotsmash,  0,        pbillian,   hotsmash,  hotsmash_state, init_hotsmash, ROT90, "Kaneko / Taito",                   "Vs. Hot Smash", MACHINE_SUPPORTS_SAVE )
GAME( 1987, sqix,      0,        sqix,       superqix,  superqix_state, init_sqix,     ROT90, "Kaneko / Taito",                   "Super Qix (World/Japan, V1.2)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, sqixr1,    sqix,     sqix,       superqix,  superqix_state, init_sqix,     ROT90, "Kaneko / Taito",                   "Super Qix (World/Japan, V1.1)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, sqixr0,    sqix,     sqix,       superqix,  superqix_state, init_sqixr0,   ROT90, "Kaneko / Taito",                   "Super Qix (World/Japan, V1.0)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, sqixu,     sqix,     sqix,       superqix,  superqix_state, init_sqix,     ROT90, "Kaneko / Taito (Romstar License)", "Super Qix (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, sqixb1,    sqix,     sqix_8031,  superqix,  superqix_state, init_sqixr0,   ROT90, "bootleg",                          "Super Qix (bootleg of V1.0, 8031 MCU)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, sqixb2,    sqix,     sqix_nomcu, superqix,  superqix_state, init_sqix,     ROT90, "bootleg",                          "Super Qix (bootleg, No MCU)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, perestro,  0,        sqix_nomcu, superqix,  superqix_state, init_perestro, ROT90, "Promat",                           "Perestroika Girls", MACHINE_SUPPORTS_SAVE )
GAME( 1993, perestrof, perestro, sqix_nomcu, superqix,  superqix_state, init_perestro, ROT90, "Promat (Fuuki license)",           "Perestroika Girls (Fuuki license)", MACHINE_SUPPORTS_SAVE )
