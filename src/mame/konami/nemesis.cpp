// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************
This entire hardware series is generally called 'GX400'

    Bubble System           (various games) GX400 PWB(B) 200207F
    Twin Bee                (Game 412) PWB(B) 352473
    Gradius                 (Game 456) PWB(B) 352473
    Galactic Warriors       (Game 578) unknown board
    Konami GT               (Game 561) PWB(B) 352473
    RF2                     (Game 561) PWB(B) 352473
    Nemesis (Hacked?)       (Game 456) GX400 PWB(B) 201000A
    Nemesis (World?)        (Game 456) GX400 PWB(B) 201000A
    Salamander (Version D)  (Game 587) PWB(B) 201012A GX587
    Salamander (Version J)  (Game 587) PWB(B) 201012A GX587
    Lifeforce (US)          (Game 587) PWB(B) 201012A GX587
    Lifeforce (Japan)       (Game 587) PWB(B) 201012A GX587
    Black Panther           (Game 604) GX604 PWB(B) 201017A
    City Bomber (World)     (Game 787) PWB(B) 250102A
    City Bomber (Japan)     (Game 787) PWB(B) 250102A
    Kitten Kaboodle         (Game 712) PWB(B) 250102A
    Nyan Nyan Panic (Japan) (Game 712) PWB(B) 250102A
    Hyper Crash (Version D) (Game 790) GX790 PWB(B) 250093A
    Hyper Crash (Version C) (Game 790) GX790 PWB(B) 250093A

Most of these boards share the same bottom/gfx board, labeled 'GX400PWB // (A)200204B'
Black Panther uses a 'GX400PWB // (A)200204C' bottom/gfx board

driver by Bryan McPhail

Boards, from earliest to latest:
* GX400 PWB(B) 200207F - The Bubble System top board: (DATA VERIFIED THRU TRACING)
    Uses an 0x800 long block of shared SRAM at 0x000-0x7ff with the bubble MCU used for block transfers and boot
    Uses Program RAM (0x10000-0x1ffff), data uploaded from bubble cart
    Uses 8-bit RAM at (0x20000-0x27fff) on the lower half of the bus (upper half is ???)
    Uses Graphics RAM (0x30000-0x3ffff) data uploaded from bubble cart
    Uses Work RAM at 0x70000-0x73fff
    Uses an unknown SDIP64 'Bubble MCU' to handle all bubble access and refresh and system init; the bubble MCU
      uploads a 0x1e0 long BIOS/Bootloader to the shared ram at 0x000-0x800 and controls the 68k /RESET and /BR lines
      and only releases these lines after the bubble memory has warmed up and is ready.
    Has 4 Interrupts: ODD/EVEN frame, VBLANK, MCU done, and 220hz timer, through a priority encoder
    Has VLM5030
    VLM5030 voice data is at ram at Sound CPU 0x8000
    Sound CPU clocked at 1.789772MHz
    MainCPU can force NMI on Sound CPU, and sound NMI is also (optionally) tied to (VBLANK?)
* PWB(B) 352473 - The 'ROM-Gradius/ROM-Twinbee/ROM-RF2 board' (NOT VERIFIED YET)
    Uses a 0x1000 long 'BIOS/Bootloader ROM' at 0x0000-0x1000 which at least partially emulates the functionality
      of the bubble system BIOS/Bootloader
    Uses Program RAM (0x10000-0x1ffff), data uploaded from 0x80000 by the BIOS/Bootloader
    Uses 8-bit work RAM at (0x20000-0x27fff) on the lower half of the bus (upper half is ???)
    Uses Graphics RAM (0x30000-0x3ffff) data uploaded from 0x80000 by the BIOS/Bootloader
    Has 3 Interrupts: ODD/EVEN frame, VBLANK, and 220hz(?) timer, through a priority encoder
    Has VLM5030
    VLM5030 voice data is at ram at Sound CPU 0x8000
    Sound CPU clocked at 1.789772MHz
    Unknown whether MainCPU can force a sound NMI or not.
* Unknown board - The 'ROM-Gwarrior board' (NOT VERIFIED YET)
    Slightly different to the board above, see driver memory maps for details, exact differences are unknown
* GX400 PWB(B) 201000A - The 'Nemesis board' (FROM SCHEMATICS)
    We have schematics for this PCB, though they do not show the unpopulated hookup for the VLM5030
    Uses fixed roms at 0x00000-0x3ffff
    Has 2 Interrupts: ODD/EVEN frame, VBLANK, through a priority encoder
    Sound CPU clocked at 1.789772MHz
    Has a spot on the PCB for a VLM5030 and ROM(RAM?) but unpopulated and not shown on schematics
* PWB(B) 201012A GX587 - The 'Salamander board' (FROM SCHEMATICS)
    We have schematics for this PCB
    Uses fixed roms at 0x00000-0x7ffff
    Has 2 Interrupts: ODD/EVEN frame, VBLANK, and does away with the priority encoder in favor of implementing
     it using discrete logic gates
    Sound CPU clocked at 3.579545MHz
    Has VLM5030
    VLM5030 voice data is in 0x4000 of ROM
TODO: others.


TODO:
- exact cycles/scanlines for VBLANK and 256V int assert/clear need to be figured out and implemented.
- bubble system needs a delay (and auto-sound-nmi hookup) so the 'getting ready... 49...' countdown actually
  plays before the simulated MCU releases the 68k and the load (and morning music) begins.
- hcrash: Konami GT-type inputs doesn't work properly.
- gradiusb: still needs proper MCU emulation;

modified by Hau
03/27/2009
 spthx to Unagi,rassy,hina,nori,Tobikage,Tommy,Crimson,yasuken,cupmen,zoo

modified by hap
06/09/2012
 Special thx 2 Neusneus, Audrey Tautou, my water bottle, chair, sleepiness

Bubble System added 2019 ArcadeHacker/Bryan McPhail

Notes:
- blkpnthr:
There are sprite priority problems in upper part of the screen ,
they can only be noticed in 2nd and 4th level .
Enemy sprites are behind blue walls 2 level) or metal construction (4 )
but when they get close to top of the screen they go in front of them.
--
To display score, priority of upper part is always lower.
So this is the correct behavior of real hardware, not an emulation bug.
- hcrash:
The "overall ranking" sums up every play score by players, by looking up
initials

***************************************************************************/

#include "emu.h"
#include "nemesis.h"
#include "konamipt.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/adc0804.h"
#include "machine/gen_latch.h"
#include "machine/rescap.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "sound/k051649.h"
#include "sound/ymopm.h"
#include "sound/ymopl.h"
#include "speaker.h"

#include "konamigt.lh"


void nemesis_state::nemesis_vblank_irq(int state)
{
	if (state && m_irq_on)
		m_maincpu->set_input_line(1, HOLD_LINE);
}

void nemesis_state::blkpnthr_vblank_irq(int state)
{
	if (state && m_irq_on)
		m_maincpu->set_input_line(2, HOLD_LINE);
}

void nemesis_state::bubsys_vblank_irq(int state)
{
	if (state && m_irq_on)
		m_maincpu->set_input_line(4, HOLD_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(nemesis_state::bubsys_interrupt)
{
	// process these in priority order
	int scanline = param;
	m_scanline_counter++;
	if (m_scanline_counter >= 72)
	{
		m_scanline_counter = 0;

		// the int4 fires every 72 scanlines of a counter that is NOT reset by VBLANK, and acts as a sort of constant timer
		if (m_irq4_on)
			m_maincpu->set_input_line(4, HOLD_LINE);
	}

	// based on tracing, the VBLANK int rising edge is 16 full scanlines before the rising edge of the VSYNC pulse on CSYNC,
	// and the VBLANK int falling edge is 16 full scanlines after the falling edge of the VSYNC pulse on CSYNC. What we don't
	// know is where exactly "scanline 0" is within that block.
	// we know from traces of VBLANK vs 256V below (which is inverted the same cycle that the VBLANK int edge rises) that that
	// cycle must be the transition from scanline 255 to 256, so presumably the vblank area is 'after' the display lines of a
	// particular frame.
	// TODO: actually implement this. The behavior may differ in the (unused(?) and untested) 288 scanline mode, as well.
	if (scanline == 0 && m_irq2_on)
		m_maincpu->set_input_line(2, HOLD_LINE);

	if (scanline == 0 && m_irq1_on && (m_screen->frame_number() & 1) == 0)
	{
		// 'INT32' is tied to 256V, which is inverted exactly at the same time as the rising edge of the VBLANK int above in 256 scanline mode.
		// Its behavior in 288 scanline mode is unknown/untested.
		m_maincpu->set_input_line(1, ASSERT_LINE);
	}
	else if (scanline == 0 && m_irq1_on && (m_screen->frame_number() & 1) != 0)
		m_maincpu->set_input_line(1, CLEAR_LINE);

}

TIMER_DEVICE_CALLBACK_MEMBER(nemesis_state::konamigt_interrupt)
{
	int scanline = param;

	if (scanline == 240 && m_irq_on && (m_screen->frame_number() & 1) == 0)
		m_maincpu->set_input_line(1, HOLD_LINE);

	if (scanline == 0 && m_irq2_on)
		m_maincpu->set_input_line(2, HOLD_LINE);
}

// irq enables in reverse order than Salamander according to the irq routine contents
TIMER_DEVICE_CALLBACK_MEMBER(nemesis_state::hcrash_interrupt)
{
	int scanline = param;

	if (scanline == 240 && m_irq2_on) //&& (m_screen->frame_number() & 1) == 0)
		m_maincpu->set_input_line(1, HOLD_LINE);

	if (scanline == 0 && m_irq_on)
		m_maincpu->set_input_line(2, HOLD_LINE);
}


TIMER_DEVICE_CALLBACK_MEMBER(nemesis_state::gx400_interrupt)
{
	int scanline = param;

	if (scanline == 240 && m_irq1_on && (m_screen->frame_number() & 1) == 0)
		m_maincpu->set_input_line(1, HOLD_LINE);

	if (scanline == 0 && m_irq2_on)
		m_maincpu->set_input_line(2, HOLD_LINE);

	if (scanline == 120 && m_irq4_on)
		m_maincpu->set_input_line(4, HOLD_LINE);
}


void nemesis_state::irq_enable_w(int state)
{
	m_irq_on = state;
}

void nemesis_state::irq1_enable_w(int state)
{
	m_irq1_on = state;
}

void nemesis_state::irq2_enable_w(int state)
{
	m_irq2_on = state;
}

void nemesis_state::irq4_enable_w(int state)
{
	m_irq4_on = state;
}

void nemesis_state::coin1_lockout_w(int state)
{
	machine().bookkeeping().coin_lockout_w(0, state);
}

void nemesis_state::coin2_lockout_w(int state)
{
	machine().bookkeeping().coin_lockout_w(1, state);
}

void nemesis_state::sound_irq_w(int state)
{
	// This asserts the Z80 /irq pin by setting a 74ls74 latch; the Z80 pulses /IOREQ low during servicing of the interrupt,
	// which clears the latch automatically, so HOLD_LINE is correct in this case
	if (state)
		m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

void nemesis_state::sound_nmi_w(int state)
{
	// On Bubble System at least, this goes to an LS02 NOR before the Z80, whose other input is tied to ???, acting as an inverter.
	// Effectively, if the bit is 1, NMI is asserted, otherwise it is cleared. This is also cleared on reset.
	// The ??? input is likely either tied to VBLANK or 256V, or tied to one of those two through a 74ls74 enable latch, controlled
	// by something else (probably either the one of the two output/int enable latches of the 68k, or by exx0/exx7 address-latched
	// accesses from the sound z80, though technically it could be anything, even the /BS signal from the mcu to the 68k)
	// TODO: trace implement the other NMI source; without this, the 'getting ready' pre-bubble-ready countdown in bubble system cannot work,
	// since it requires a sequence of NMIs in order to function.
	m_audiocpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}

void nemesis_state::bubsys_mcu_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bubsys_control_ram[offset]);
	//logerror("bubsys_mcu_w (%08x) %d (%02x %02x %02x %02x)\n", m_maincpu->pc(), state, m_bubsys_control_ram[0], m_bubsys_control_ram[1], m_bubsys_control_ram[2], m_bubsys_control_ram[3]);

	if (offset==1)
	{
		// NOP?
		if (m_bubsys_control_ram[1]==0)
		{
		}
		// Read
		else if (m_bubsys_control_ram[1]==1)
		{
			// The MCU copies the requested page of bubble memory to 0xf00 of shared RAM
			int page = m_bubsys_control_ram[0] & 0x7ff;
			//int unknownBit = m_bubsys_control_ram[0] & 0x800;

			logerror("\tCopy page %02x to shared ram\n", page);

			const uint8_t *src = memregion("bubblememory")->base();
			memcpy(&m_bubsys_shared_ram[0xf00/2], src + page * 0x90, 0x80);

			// The last 2 bytes of the block are loaded into the control register
			m_bubsys_control_ram[0] = src[page * 0x90 + 0x80] | (src[page * 0x90 + 0x81]<<8);
			m_maincpu->set_input_line(5, HOLD_LINE); // This presumably gets asserted (under mcu control) whenever the MCU has completed a command
		}
		// Write?
		else if (m_bubsys_control_ram[1]==2)
		{
			logerror("Request to write to bubble memory?  %04x\n", m_bubsys_control_ram[0]);
		}
	}
	else
	{
		//logerror("bubsys_mcu_trigger_w (%08x) %d (%02x %02x %02x %02x)\n", m_maincpu->pc(), state, m_bubsys_control_ram[0], m_bubsys_control_ram[1], m_bubsys_control_ram[2], m_bubsys_control_ram[3]);
		// Not confirmed the clear happens here; clear is done by the MCU code itself, presumably some number of cycles after the assert.
		m_maincpu->set_input_line(5, CLEAR_LINE);
	}
}

uint16_t nemesis_state::gx400_sharedram_word_r(offs_t offset)
{
	return m_gx400_shared_ram[offset];
}

void nemesis_state::gx400_sharedram_word_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_gx400_shared_ram[offset] = data;
}


uint16_t nemesis_state::konamigt_input_word_r()
{
/*
    bit 0-7:   steering
    bit 8-9:   brake
    bit 10-11: unknown
    bit 12-15: accel
*/

	int data = ioport("IN3")->read();
	int data2 = ioport("WHEEL")->read();

	int ret=0x0000;

//  if (BIT(data, 4)) ret |= 0x0800;          // turbo/gear?
//  if (BIT(data, 7)) ret |= 0x0400;          // turbo?
	if (BIT(data, 5))
		ret |= 0x0300;          // brake        (0-3)

	if (BIT(data, 6))
		ret |= 0xf000;          // accel        (0-f)

	ret |= data2 & 0x7f;                    // steering wheel, not exactly sure if DIAL works ok.

	return ret;
}

void nemesis_state::selected_ip_w(uint8_t data)
{
	m_selected_ip = data;    // latch the value
}

uint8_t nemesis_state::selected_ip_r()
{
	switch (m_selected_ip & 0xf)
	{                                               // From WEC Le Mans Schems:
		case 0xc:  return ioport("ACCEL")->read();  // Accel - Schems: Accelevr
		case 0:    return ioport("ACCEL")->read();
		case 0xd:  return ioport("WHEEL")->read();  // Wheel - Schems: Handlevr
		case 1:    return ioport("WHEEL")->read();

		default: return ~0;
	}
}


void nemesis_state::nemesis_filter_w(offs_t offset, uint8_t data)
{
	int C1 = /* offset & 0x1000 ? 4700 : */ 0; // is this right? 4.7uF seems too large
	int C2 = offset & 0x0800 ? 33 : 0;         // 0.033uF = 33 nF
	m_filter1->filter_rc_set_RC(filter_rc_device::LOWPASS_3R, (AY8910_INTERNAL_RESISTANCE + 12000) / 3, 0, 0, CAP_N(C1)); // unused?
	m_filter2->filter_rc_set_RC(filter_rc_device::LOWPASS_3R, AY8910_INTERNAL_RESISTANCE + 1000, 10000, 0, CAP_N(C2));
	m_filter3->filter_rc_set_RC(filter_rc_device::LOWPASS_3R, AY8910_INTERNAL_RESISTANCE + 1000, 10000, 0, CAP_N(C2));
	m_filter4->filter_rc_set_RC(filter_rc_device::LOWPASS_3R, AY8910_INTERNAL_RESISTANCE + 1000, 10000, 0, CAP_N(C2));

	// konamigt also uses bits 0x0018, what are they for?
}

void nemesis_state::gx400_speech_w(offs_t offset, uint8_t data)
{
	// bit 3 falling edge: latch VLM data (databus is irrelevant for other writes)
	// bit 4 is also used (OE for VLM data?)
	if (BIT(~offset, 3) && BIT(m_gx400_speech_offset, 3))
		m_vlm->data_w(data);

	// bit 5: ST, bit 6: RST
	m_vlm->st(BIT(offset, 5));
	m_vlm->rst(BIT(offset, 6));

	m_gx400_speech_offset = offset;
}

void nemesis_state::salamand_speech_start_w(uint8_t data)
{
	m_vlm->rst(BIT(data, 0));
	m_vlm->st(BIT(data, 1));
	// bit 2 is OE for VLM data
}

uint8_t nemesis_state::salamand_speech_busy_r()
{
	return m_vlm->bsy();
}

uint8_t nemesis_state::nemesis_portA_r()
{
/*
   bit 0-3:   timer
   bit 4 6:   unused (always high)
   bit 5:     vlm5030 busy
   bit 7:     unused by this software version. Bubble Memory version uses this bit (TODO: verify this?)
*/
	int res = (m_audiocpu->total_cycles() / 512) & 0x0f;

	res |= 0xd0;

	if (m_vlm != nullptr && m_vlm->bsy())
		res |= 0x20;

	return res;
}

void nemesis_state::city_sound_bank_w(uint8_t data)
{
	int bank_A = (data & 0x03);
	int bank_B = ((data >> 2) & 0x03);
	m_k007232->set_bank(bank_A, bank_B);
}


void nemesis_state::nemesis_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x040000, 0x04ffff).ram().w(FUNC(nemesis_state::nemesis_charram_word_w)).share("charram");
	map(0x050000, 0x0503ff).ram().share("xscroll1");
	map(0x050400, 0x0507ff).ram().share("xscroll2");
	map(0x050800, 0x050eff).ram();
	map(0x050f00, 0x050f7f).ram().share("yscroll2");
	map(0x050f80, 0x050fff).ram().share("yscroll1");
	map(0x051000, 0x051fff).ram();
	map(0x052000, 0x052fff).ram().w(FUNC(nemesis_state::nemesis_videoram1_word_w)).share("videoram1");       /* VRAM */
	map(0x053000, 0x053fff).ram().w(FUNC(nemesis_state::nemesis_videoram2_word_w)).share("videoram2");
	map(0x054000, 0x054fff).ram().w(FUNC(nemesis_state::nemesis_colorram1_word_w)).share("colorram1");
	map(0x055000, 0x055fff).ram().w(FUNC(nemesis_state::nemesis_colorram2_word_w)).share("colorram2");
	map(0x056000, 0x056fff).ram().share("spriteram");
	map(0x05a000, 0x05afff).ram().w(FUNC(nemesis_state::nemesis_palette_word_w)).share("paletteram");
	map(0x05c001, 0x05c001).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x05c400, 0x05c401).portr("DSW0");
	map(0x05c402, 0x05c403).portr("DSW1");
	map(0x05c800, 0x05c801).w("watchdog", FUNC(watchdog_timer_device::reset16_w));   /* probably */
	map(0x05cc00, 0x05cc01).portr("IN0");
	map(0x05cc02, 0x05cc03).portr("IN1");
	map(0x05cc04, 0x05cc05).portr("IN2");
	map(0x05cc06, 0x05cc07).portr("TEST");
	map(0x05e000, 0x05e00f).w("outlatch", FUNC(ls259_device::write_d0)).umask16(0xff00);
	map(0x05e000, 0x05e00f).w("intlatch", FUNC(ls259_device::write_d0)).umask16(0x00ff);
	map(0x060000, 0x067fff).ram();         /* WORK RAM */
}

void nemesis_state::gx400_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();     /* ROM BIOS */
	map(0x010000, 0x01ffff).ram();
	map(0x020000, 0x027fff).rw(FUNC(nemesis_state::gx400_sharedram_word_r), FUNC(nemesis_state::gx400_sharedram_word_w));
	map(0x030000, 0x03ffff).ram().w(FUNC(nemesis_state::nemesis_charram_word_w)).share("charram");
	map(0x050000, 0x0503ff).ram().share("xscroll1");
	map(0x050400, 0x0507ff).ram().share("xscroll2");
	map(0x050800, 0x050eff).ram();
	map(0x050f00, 0x050f7f).ram().share("yscroll2");
	map(0x050f80, 0x050fff).ram().share("yscroll1");
	map(0x051000, 0x051fff).ram();
	map(0x052000, 0x052fff).ram().w(FUNC(nemesis_state::nemesis_videoram1_word_w)).share("videoram1");       /* VRAM */
	map(0x053000, 0x053fff).ram().w(FUNC(nemesis_state::nemesis_videoram2_word_w)).share("videoram2");
	map(0x054000, 0x054fff).ram().w(FUNC(nemesis_state::nemesis_colorram1_word_w)).share("colorram1");
	map(0x055000, 0x055fff).ram().w(FUNC(nemesis_state::nemesis_colorram2_word_w)).share("colorram2");
	map(0x056000, 0x056fff).ram().share("spriteram");
	map(0x057000, 0x057fff).ram();             /* needed for twinbee */
	map(0x05a000, 0x05afff).ram().w(FUNC(nemesis_state::nemesis_palette_word_w)).share("paletteram");
	map(0x05c001, 0x05c001).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x05c402, 0x05c403).portr("DSW0");
	map(0x05c404, 0x05c405).portr("DSW1");
	map(0x05c406, 0x05c407).portr("TEST");
	map(0x05c800, 0x05c801).w("watchdog", FUNC(watchdog_timer_device::reset16_w));   /* probably */
	map(0x05cc00, 0x05cc01).portr("IN0");
	map(0x05cc02, 0x05cc03).portr("IN1");
	map(0x05cc04, 0x05cc05).portr("IN2");
	map(0x05e000, 0x05e00f).w("outlatch", FUNC(ls259_device::write_d0)).umask16(0xff00);
	map(0x05e000, 0x05e00f).w("intlatch", FUNC(ls259_device::write_d0)).umask16(0x00ff);
	map(0x060000, 0x07ffff).ram();         /* WORK RAM */
	map(0x080000, 0x0bffff).rom();
}

void nemesis_state::bubsys_map(address_map &map)
{
	map(0x000000, 0x000fff).ram().share("bubsys_shared"); /* Shared with MCU */
	map(0x010000, 0x01ffff).ram(); /* PROGRAM RAM */
	map(0x020000, 0x027fff).rw(FUNC(nemesis_state::gx400_sharedram_word_r), FUNC(nemesis_state::gx400_sharedram_word_w));
	map(0x030000, 0x03ffff).ram().w(FUNC(nemesis_state::nemesis_charram_word_w)).share("charram");
	map(0x040000, 0x040007).ram().w(FUNC(nemesis_state::bubsys_mcu_w)).share("bubsys_control"); // Shared with MCU
	map(0x050000, 0x0503ff).ram().share("xscroll1");
	map(0x050400, 0x0507ff).ram().share("xscroll2");
	map(0x050800, 0x050eff).ram();
	map(0x050f00, 0x050f7f).ram().share("yscroll2");
	map(0x050f80, 0x050fff).ram().share("yscroll1");
	map(0x051000, 0x051fff).ram();
	map(0x052000, 0x052fff).ram().w(FUNC(nemesis_state::nemesis_videoram1_word_w)).share("videoram1");       /* VRAM */
	map(0x053000, 0x053fff).ram().w(FUNC(nemesis_state::nemesis_videoram2_word_w)).share("videoram2");
	map(0x054000, 0x054fff).ram().w(FUNC(nemesis_state::nemesis_colorram1_word_w)).share("colorram1");
	map(0x055000, 0x055fff).ram().w(FUNC(nemesis_state::nemesis_colorram2_word_w)).share("colorram2");
	map(0x056000, 0x056fff).ram().share("spriteram");
	map(0x057000, 0x057fff).ram();
	map(0x05a000, 0x05afff).ram().w(FUNC(nemesis_state::nemesis_palette_word_w)).share("paletteram");
	map(0x05c001, 0x05c001).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x05c402, 0x05c403).portr("DSW0");
	map(0x05c404, 0x05c405).portr("DSW1");
	map(0x05c406, 0x05c407).portr("TEST");
	map(0x05c800, 0x05c801).w("watchdog", FUNC(watchdog_timer_device::reset16_w));   /* probably */
	map(0x05cc00, 0x05cc01).portr("IN0");
	map(0x05cc02, 0x05cc03).portr("IN1");
	map(0x05cc04, 0x05cc05).portr("IN2");
	map(0x05e000, 0x05e00f).w("outlatch", FUNC(ls259_device::write_d0)).umask16(0xff00);
	map(0x05e000, 0x05e00f).w("intlatch", FUNC(ls259_device::write_d0)).umask16(0x00ff);
	map(0x070000, 0x073fff).ram();  /* WORK RAM */
	map(0x078000, 0x07ffff).rom();  /* Empty diagnostic ROM slot */
}

void nemesis_state::konamigt_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x040000, 0x04ffff).ram().w(FUNC(nemesis_state::nemesis_charram_word_w)).share("charram");
	map(0x050000, 0x0503ff).ram().share("xscroll1");
	map(0x050400, 0x0507ff).ram().share("xscroll2");
	map(0x050800, 0x050eff).ram();
	map(0x050f00, 0x050f7f).ram().share("yscroll2");
	map(0x050f80, 0x050fff).ram().share("yscroll1");
	map(0x051000, 0x051fff).ram();
	map(0x052000, 0x052fff).ram().w(FUNC(nemesis_state::nemesis_videoram1_word_w)).share("videoram1");       /* VRAM */
	map(0x053000, 0x053fff).ram().w(FUNC(nemesis_state::nemesis_videoram2_word_w)).share("videoram2");
	map(0x054000, 0x054fff).ram().w(FUNC(nemesis_state::nemesis_colorram1_word_w)).share("colorram1");
	map(0x055000, 0x055fff).ram().w(FUNC(nemesis_state::nemesis_colorram2_word_w)).share("colorram2");
	map(0x056000, 0x056fff).ram().share("spriteram");
	map(0x05a000, 0x05afff).ram().w(FUNC(nemesis_state::nemesis_palette_word_w)).share("paletteram");
	map(0x05c001, 0x05c001).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x05c400, 0x05c401).portr("DSW0");
	map(0x05c402, 0x05c403).portr("DSW1");
	map(0x05c800, 0x05c801).w("watchdog", FUNC(watchdog_timer_device::reset16_w));   /* probably */
	map(0x05cc00, 0x05cc01).portr("IN0");
	map(0x05cc02, 0x05cc03).portr("IN1");
	map(0x05cc04, 0x05cc05).portr("IN2");
	map(0x05cc06, 0x05cc07).portr("TEST");
	map(0x05e000, 0x05e00f).w("outlatch", FUNC(ls259_device::write_d0)).umask16(0xff00);
	map(0x05e000, 0x05e00f).w("intlatch", FUNC(ls259_device::write_d0)).umask16(0x00ff);
	map(0x060000, 0x067fff).ram();         /* WORK RAM */
	map(0x070000, 0x070001).r(FUNC(nemesis_state::konamigt_input_word_r));
}

void nemesis_state::rf2_gx400_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();     /* ROM BIOS */
	map(0x010000, 0x01ffff).ram();
	map(0x020000, 0x027fff).rw(FUNC(nemesis_state::gx400_sharedram_word_r), FUNC(nemesis_state::gx400_sharedram_word_w));
	map(0x030000, 0x03ffff).ram().w(FUNC(nemesis_state::nemesis_charram_word_w)).share("charram");
	map(0x050000, 0x0503ff).ram().share("xscroll1");
	map(0x050400, 0x0507ff).ram().share("xscroll2");
	map(0x050800, 0x050eff).ram();
	map(0x050f00, 0x050f7f).ram().share("yscroll2");
	map(0x050f80, 0x050fff).ram().share("yscroll1");
	map(0x051000, 0x051fff).ram();
	map(0x052000, 0x052fff).ram().w(FUNC(nemesis_state::nemesis_videoram1_word_w)).share("videoram1");       /* VRAM */
	map(0x053000, 0x053fff).ram().w(FUNC(nemesis_state::nemesis_videoram2_word_w)).share("videoram2");
	map(0x054000, 0x054fff).ram().w(FUNC(nemesis_state::nemesis_colorram1_word_w)).share("colorram1");
	map(0x055000, 0x055fff).ram().w(FUNC(nemesis_state::nemesis_colorram2_word_w)).share("colorram2");
	map(0x056000, 0x056fff).ram().share("spriteram");
	map(0x05a000, 0x05afff).ram().w(FUNC(nemesis_state::nemesis_palette_word_w)).share("paletteram");
	map(0x05c001, 0x05c001).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x05c402, 0x05c403).portr("DSW0");
	map(0x05c404, 0x05c405).portr("DSW1");
	map(0x05c406, 0x05c407).portr("TEST");
	map(0x05c800, 0x05c801).w("watchdog", FUNC(watchdog_timer_device::reset16_w));   /* probably */
	map(0x05cc00, 0x05cc01).portr("IN0");
	map(0x05cc02, 0x05cc03).portr("IN1");
	map(0x05cc04, 0x05cc05).portr("IN2");
	map(0x05e000, 0x05e00f).w("outlatch", FUNC(ls259_device::write_d0)).umask16(0xff00);
	map(0x05e000, 0x05e00f).w("intlatch", FUNC(ls259_device::write_d0)).umask16(0x00ff);
	map(0x060000, 0x067fff).ram();         /* WORK RAM */
	map(0x070000, 0x070001).r(FUNC(nemesis_state::konamigt_input_word_r));
	map(0x080000, 0x0bffff).rom();
}


void nemesis_state::sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0xa000, 0xafff).w(m_k005289, FUNC(k005289_device::ld1_w));
	map(0xc000, 0xcfff).w(m_k005289, FUNC(k005289_device::ld2_w));
	map(0xe001, 0xe001).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xe003, 0xe003).w(m_k005289, FUNC(k005289_device::tg1_w));
	map(0xe004, 0xe004).w(m_k005289, FUNC(k005289_device::tg2_w));
	map(0xe005, 0xe005).w("ay2", FUNC(ay8910_device::address_w));
	map(0xe006, 0xe006).w("ay1", FUNC(ay8910_device::address_w));
	map(0xe007, 0xe007).w(FUNC(nemesis_state::nemesis_filter_w));
	map(0xe086, 0xe086).r("ay1", FUNC(ay8910_device::data_r));
	map(0xe106, 0xe106).w("ay1", FUNC(ay8910_device::data_w));
	map(0xe205, 0xe205).r("ay2", FUNC(ay8910_device::data_r));
	map(0xe405, 0xe405).w("ay2", FUNC(ay8910_device::data_w));
}

void nemesis_state::gx400_sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x7fff).ram().share("gx400_shared");
	map(0x8000, 0x87ff).ram().share("voiceram");
	map(0xa000, 0xafff).w(m_k005289, FUNC(k005289_device::ld1_w));
	map(0xc000, 0xcfff).w(m_k005289, FUNC(k005289_device::ld2_w));
	map(0xe000, 0xe000).select(0x78).w(FUNC(nemesis_state::gx400_speech_w));
	map(0xe001, 0xe001).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xe003, 0xe003).w(m_k005289, FUNC(k005289_device::tg1_w));
	map(0xe004, 0xe004).w(m_k005289, FUNC(k005289_device::tg2_w));
	map(0xe005, 0xe005).w("ay2", FUNC(ay8910_device::address_w));
	map(0xe006, 0xe006).w("ay1", FUNC(ay8910_device::address_w));
	map(0xe007, 0xe007).select(0x1ff8).w(FUNC(nemesis_state::nemesis_filter_w));
	map(0xe086, 0xe086).r("ay1", FUNC(ay8910_device::data_r));
	map(0xe106, 0xe106).w("ay1", FUNC(ay8910_device::data_w));
	map(0xe205, 0xe205).r("ay2", FUNC(ay8910_device::data_r));
	map(0xe405, 0xe405).w("ay2", FUNC(ay8910_device::data_w));
}

// gx400 voice data is not in a ROM but in sound RAM at $8000
void nemesis_state::gx400_vlm_map(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x000, 0x7ff).ram().share("voiceram");
}

/******************************************************************************/

void nemesis_state::salamand_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x087fff).ram();
	map(0x090000, 0x091fff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).umask16(0x00ff).share("palette");
	map(0x0a0000, 0x0a0001).w(FUNC(nemesis_state::salamand_control_port_word_w));     /* irq enable, flipscreen, etc. */
	map(0x0c0001, 0x0c0001).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x0c0002, 0x0c0003).portr("DSW0");
	map(0x0c0004, 0x0c0005).w("watchdog", FUNC(watchdog_timer_device::reset16_w));   /* probably */
	map(0x0c2000, 0x0c2001).portr("IN0");    /* Coins, start buttons, test mode */
	map(0x0c2002, 0x0c2003).portr("IN1");
	map(0x0c2004, 0x0c2005).portr("IN2");
	map(0x0c2006, 0x0c2007).portr("DSW1");
	map(0x100000, 0x100fff).ram().w(FUNC(nemesis_state::nemesis_videoram2_word_w)).share("videoram2");       /* VRAM */
	map(0x101000, 0x101fff).ram().w(FUNC(nemesis_state::nemesis_videoram1_word_w)).share("videoram1");
	map(0x102000, 0x102fff).ram().w(FUNC(nemesis_state::nemesis_colorram2_word_w)).share("colorram2");
	map(0x103000, 0x103fff).ram().w(FUNC(nemesis_state::nemesis_colorram1_word_w)).share("colorram1");
	map(0x120000, 0x12ffff).ram().w(FUNC(nemesis_state::nemesis_charram_word_w)).share("charram");
	map(0x180000, 0x180fff).ram().share("spriteram");       /* more sprite ram ??? */
	map(0x190000, 0x1903ff).ram().share("xscroll2");
	map(0x190400, 0x1907ff).ram().share("xscroll1");
	map(0x190800, 0x190eff).ram();
	map(0x190f00, 0x190f7f).ram().share("yscroll1");
	map(0x190f80, 0x190fff).ram().share("yscroll2");
	map(0x191000, 0x191fff).ram();
}

void nemesis_state::blkpnthr_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x081fff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).umask16(0x00ff).share("palette");
	map(0x090000, 0x097fff).ram();
	map(0x0a0000, 0x0a0001).ram().w(FUNC(nemesis_state::salamand_control_port_word_w));     /* irq enable, flipscreen, etc. */
	map(0x0c0001, 0x0c0001).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x0c0002, 0x0c0003).portr("DSW0");
	map(0x0c0004, 0x0c0005).w("watchdog", FUNC(watchdog_timer_device::reset16_w));   /* probably */
	map(0x0c2000, 0x0c2001).portr("IN0");    /* Coins, start buttons, test mode */
	map(0x0c2002, 0x0c2003).portr("IN1");
	map(0x0c2004, 0x0c2005).portr("IN2");
	map(0x0c2006, 0x0c2007).portr("DSW1");
	map(0x100000, 0x100fff).ram().w(FUNC(nemesis_state::nemesis_colorram1_word_w)).share("colorram1").mirror(0x4000); /* VRAM */
	map(0x101000, 0x101fff).ram().w(FUNC(nemesis_state::nemesis_colorram2_word_w)).share("colorram2").mirror(0x4000);
	map(0x102000, 0x102fff).ram().w(FUNC(nemesis_state::nemesis_videoram1_word_w)).share("videoram1");
	map(0x103000, 0x103fff).ram().w(FUNC(nemesis_state::nemesis_videoram2_word_w)).share("videoram2");
	map(0x120000, 0x12ffff).ram().w(FUNC(nemesis_state::nemesis_charram_word_w)).share("charram");
	map(0x180000, 0x1803ff).ram().share("xscroll1");
	map(0x180400, 0x1807ff).ram().share("xscroll2");
	map(0x180800, 0x180eff).ram();
	map(0x180f00, 0x180f7f).ram().share("yscroll2");
	map(0x180f80, 0x180fff).ram().share("yscroll1");
	map(0x181000, 0x181fff).ram();
	map(0x190000, 0x190fff).ram().share("spriteram");       /* more sprite ram ??? */
}

void nemesis_state::citybomb_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x080000, 0x087fff).ram();
	map(0x0e0000, 0x0e1fff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).umask16(0x00ff).share("palette");
	map(0x0f0000, 0x0f0001).portr("DSW1");
	map(0x0f0002, 0x0f0003).portr("IN2");
	map(0x0f0004, 0x0f0005).portr("IN1");
	map(0x0f0006, 0x0f0007).portr("IN0");    /* Coins, start buttons, test mode */
	map(0x0f0008, 0x0f0009).portr("DSW0");
	map(0x0f0011, 0x0f0011).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x0f0018, 0x0f0019).w("watchdog", FUNC(watchdog_timer_device::reset16_w));   /* probably */
	map(0x0f0021, 0x0f0021).rw("adc", FUNC(adc0804_device::read), FUNC(adc0804_device::write));
	map(0x0f8000, 0x0f8001).w(FUNC(nemesis_state::salamand_control_port_word_w));     /* irq enable, flipscreen, etc. */
	map(0x100000, 0x1bffff).rom();
	map(0x200000, 0x20ffff).ram().w(FUNC(nemesis_state::nemesis_charram_word_w)).share("charram");
	map(0x210000, 0x210fff).ram().w(FUNC(nemesis_state::nemesis_videoram1_word_w)).share("videoram1");       /* VRAM */
	map(0x211000, 0x211fff).ram().w(FUNC(nemesis_state::nemesis_videoram2_word_w)).share("videoram2");
	map(0x212000, 0x212fff).ram().w(FUNC(nemesis_state::nemesis_colorram1_word_w)).share("colorram1");
	map(0x213000, 0x213fff).ram().w(FUNC(nemesis_state::nemesis_colorram2_word_w)).share("colorram2");
	map(0x300000, 0x3003ff).ram().share("xscroll1");
	map(0x300400, 0x3007ff).ram().share("xscroll2");
	map(0x300800, 0x300eff).ram();
	map(0x300f00, 0x300f7f).ram().share("yscroll2");
	map(0x300f80, 0x300fff).ram().share("yscroll1");
	map(0x301000, 0x301fff).ram();
	map(0x310000, 0x310fff).ram().share("spriteram");       /* more sprite ram ??? */
}

void nemesis_state::nyanpani_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x040000, 0x047fff).ram();
	map(0x060000, 0x061fff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).umask16(0x00ff).share("palette");
	map(0x070000, 0x070001).portr("DSW1");
	map(0x070002, 0x070003).portr("IN2");
	map(0x070004, 0x070005).portr("IN1");
	map(0x070006, 0x070007).portr("IN0");    /* Coins, start buttons, test mode */
	map(0x070008, 0x070009).portr("DSW0");
	map(0x070011, 0x070011).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x070018, 0x070019).w("watchdog", FUNC(watchdog_timer_device::reset16_w));   /* probably */
	map(0x078000, 0x078001).w(FUNC(nemesis_state::salamand_control_port_word_w));     /* irq enable, flipscreen, etc. */
	map(0x100000, 0x13ffff).rom();
	map(0x200000, 0x200fff).ram().w(FUNC(nemesis_state::nemesis_videoram1_word_w)).share("videoram1");       /* VRAM */
	map(0x201000, 0x201fff).ram().w(FUNC(nemesis_state::nemesis_videoram2_word_w)).share("videoram2");
	map(0x202000, 0x202fff).ram().w(FUNC(nemesis_state::nemesis_colorram1_word_w)).share("colorram1");
	map(0x203000, 0x203fff).ram().w(FUNC(nemesis_state::nemesis_colorram2_word_w)).share("colorram2");
	map(0x210000, 0x21ffff).ram().w(FUNC(nemesis_state::nemesis_charram_word_w)).share("charram");
	map(0x300000, 0x300fff).ram().share("spriteram");       /* more sprite ram ??? */
	map(0x310000, 0x3103ff).ram().share("xscroll1");
	map(0x310400, 0x3107ff).ram().share("xscroll2");
	map(0x310800, 0x310eff).ram();
	map(0x310f00, 0x310f7f).ram().share("yscroll2");
	map(0x310f80, 0x310fff).ram().share("yscroll1");
	map(0x311000, 0x311fff).ram();
}

void nemesis_state::sal_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xb000, 0xb00d).rw(m_k007232, FUNC(k007232_device::read), FUNC(k007232_device::write));
	map(0xc000, 0xc001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xd000, 0xd000).w(m_vlm, FUNC(vlm5030_device::data_w));
	map(0xe000, 0xe000).r(FUNC(nemesis_state::salamand_speech_busy_r));
	map(0xf000, 0xf000).w(FUNC(nemesis_state::salamand_speech_start_w));
}

void nemesis_state::salamand_vlm_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x3fff).rom();
}

void nemesis_state::blkpnthr_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xb000, 0xb00d).rw(m_k007232, FUNC(k007232_device::read), FUNC(k007232_device::write));
	map(0xc000, 0xc001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
}

void nemesis_state::city_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9800, 0x98ff).m("k051649", FUNC(k051649_device::scc_map));
	map(0xa000, 0xa001).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0xb000, 0xb00d).rw(m_k007232, FUNC(k007232_device::read), FUNC(k007232_device::write));
	map(0xc000, 0xc000).w(FUNC(nemesis_state::city_sound_bank_w)); /* 7232 bankswitch */
	map(0xd000, 0xd000).r("soundlatch", FUNC(generic_latch_8_device::read));
}

/******************************************************************************/

void nemesis_state::hcrash_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x040000, 0x05ffff).rom();
	map(0x080000, 0x083fff).ram();
	map(0x090000, 0x091fff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).umask16(0x00ff).share("palette");
	map(0x0a0000, 0x0a0001).w(FUNC(nemesis_state::salamand_control_port_word_w));     /* irq enable, flipscreen, etc. */
	map(0x0c0001, 0x0c0001).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x0c0002, 0x0c0003).portr("DSW0");
	map(0x0c0004, 0x0c0005).portr("DSW1");
	map(0x0c0006, 0x0c0007).portr("TEST");
	map(0x0c0008, 0x0c0009).w("watchdog", FUNC(watchdog_timer_device::reset16_w));   /* watchdog probably */
	map(0x0c000a, 0x0c000b).portr("IN0");
	map(0x0c2000, 0x0c2001).r(FUNC(nemesis_state::konamigt_input_word_r)); /* Konami GT control */
	map(0x0c2800, 0x0c280f).w("intlatch", FUNC(ls259_device::write_d0)).umask16(0x00ff); // ???
	map(0x0c4000, 0x0c4001).portr("IN1");
	map(0x0c4001, 0x0c4001).w(FUNC(nemesis_state::selected_ip_w));
	map(0x0c4003, 0x0c4003).rw("adc", FUNC(adc0804_device::read), FUNC(adc0804_device::write));    /* WEC Le Mans 24 control */
	map(0x100000, 0x100fff).ram().w(FUNC(nemesis_state::nemesis_videoram2_word_w)).share("videoram2");       /* VRAM */
	map(0x101000, 0x101fff).ram().w(FUNC(nemesis_state::nemesis_videoram1_word_w)).share("videoram1");
	map(0x102000, 0x102fff).ram().w(FUNC(nemesis_state::nemesis_colorram2_word_w)).share("colorram2");
	map(0x103000, 0x103fff).ram().w(FUNC(nemesis_state::nemesis_colorram1_word_w)).share("colorram1");
	map(0x120000, 0x12ffff).ram().w(FUNC(nemesis_state::nemesis_charram_word_w)).share("charram");
	map(0x180000, 0x180fff).ram().share("spriteram");
	map(0x190000, 0x1903ff).ram().share("xscroll2");
	map(0x190400, 0x1907ff).ram().share("xscroll1");
	map(0x190800, 0x190eff).ram();
	map(0x190f00, 0x190f7f).ram().share("yscroll1");
	map(0x190f80, 0x190fff).ram().share("yscroll2");
	map(0x191000, 0x191fff).ram();
}

/******************************************************************************/

static INPUT_PORTS_START( nemesis )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)    // power-up
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)    // shoot
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)    // missile
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW0")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), DEF_STR( None ), SW1)
	/* "None" = coin slot B disabled */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "50k and every 100k" )
	PORT_DIPSETTING(    0x10, "30k" )
	PORT_DIPSETTING(    0x08, "50k" )
	PORT_DIPSETTING(    0x00, "100k" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("TEST")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( nemesuk )
	PORT_INCLUDE( nemesis )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "20k and every 70k" )
	PORT_DIPSETTING(    0x10, "30k and every 80k" )
	PORT_DIPSETTING(    0x08, "20k" )
	PORT_DIPSETTING(    0x00, "30k" )
INPUT_PORTS_END


/* This needs to be sorted */
static INPUT_PORTS_START( konamigt )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Gear Shift") PORT_TOGGLE
	PORT_BIT( 0xef, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Brake")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Accelerator")
//  PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 )

	PORT_START("DSW0")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW1")
	PORT_BIT( 0x4f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("TEST")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_BIT( 0xfa, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("WHEEL")    /* Wheel (360deg) */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(5)
INPUT_PORTS_END


/* This needs to be sorted */
static INPUT_PORTS_START( rf2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* don't change */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_BUTTON3 ) /* gear (0-7) */
	PORT_BIT( 0x8f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )
//  PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 )

	PORT_START("DSW0")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW1")
	PORT_BIT( 0x4f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("TEST")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_BIT( 0xfa, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("WHEEL")    /* Wheel (360deg) */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(5)
INPUT_PORTS_END


static INPUT_PORTS_START( gwarrior )
	PORT_START("IN0")
	KONAMI8_SYSTEM_UNK

	PORT_START("IN1")
	KONAMI8_B123(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	KONAMI8_B123(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "30k 100k 200k 400k" )
	PORT_DIPSETTING(    0x10, "40k 120k 240k 480k" )
	PORT_DIPSETTING(    0x08, "50k 150k 300k 600k" )
	PORT_DIPSETTING(    0x00, "100k 200k 400k 800k" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("TEST")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Players ) )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( twinbee )
	PORT_START("IN0")
	KONAMI8_SYSTEM_UNK

	PORT_START("IN1")
	KONAMI8_B12_UNK(1)

	PORT_START("IN2")
	KONAMI8_B12_UNK(2)

	PORT_START("DSW0")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "20k 100k" )
	PORT_DIPSETTING(    0x10, "30k 120k" )
	PORT_DIPSETTING(    0x08, "40k 140k" )
	PORT_DIPSETTING(    0x00, "50k 160k" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("TEST")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Players ) )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( gradius )
	PORT_START("IN0")
	KONAMI8_SYSTEM_UNK

	PORT_START("IN1")
	KONAMI8_B132(1) // button1 = power-up, button3 = shoot, button2 = missile
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	KONAMI8_B132(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), DEF_STR( None ), SW1)
	/* "None" = coin slot B disabled */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "20k and every 70k" )
	PORT_DIPSETTING(    0x10, "30k and every 80k" )
	PORT_DIPSETTING(    0x08, "20k only" )
	PORT_DIPSETTING(    0x00, "30k only" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("TEST")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( salamand )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW3:2" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW3:3" )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "Disabled" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, "Coin Slot(s)" )          PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x18, 0x00, "Max Credit(s)" )         PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "9" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( lifefrcj )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW3:2" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW3:3" )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)    // power-up
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)    // shoot
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)    // missile
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW0")
	KONAMI_COINAGE_LOC("Invalid", "Invalid", SW1)
	/* "Invalid" = both coin slots disabled */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x04, 0x00, "Coin Slot(s)" )          PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "70k and every 200k" )
	PORT_DIPSETTING(    0x10, "100k and every 300k" )
	PORT_DIPSETTING(    0x08, "70k only" )
	PORT_DIPSETTING(    0x00, "100k only" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( blkpnthr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x80, "Continue" )              PORT_DIPLOCATION("SW3:2,3")
	PORT_DIPSETTING(    0xc0, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, "2 Areas" )
	PORT_DIPSETTING(    0x40, "3 Areas" )
	PORT_DIPSETTING(    0x00, "4 Areas" )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW3:4" )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW0")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), DEF_STR( None ), SW1)
	/* "None" = coin slot B disabled */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "50k 100k" )
	PORT_DIPSETTING(    0x10, "20k 50k" )
	PORT_DIPSETTING(    0x08, "30k 70k" )
	PORT_DIPSETTING(    0x00, "80k 150k" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( citybomb )
	PORT_START("IN0")
	KONAMI8_SYSTEM_10
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Upright Controls" )       PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW3:3" )

	PORT_START("IN1")
	KONAMI8_B123(1)
	PORT_DIPNAME( 0x80, 0x80, "Device Type" )           PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x00, "Handle" )
	PORT_DIPSETTING(    0x80, DEF_STR( Joystick ) )

	PORT_START("IN2")
	KONAMI8_B123(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("adc", adc0804_device, intr_r)

	PORT_START("DSW0")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* "Invalid" = both coin slots disabled */

	PORT_START("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x10, "Qualify" )               PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "Long" )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, "Short" )
	PORT_DIPSETTING(    0x00, "Very Short" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	/* WEC Le Mans 24 specific control? */
	PORT_START("ACCEL")     /* Accelerator */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

	PORT_START("WHEEL")     /* Steering Wheel */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)
INPUT_PORTS_END

static INPUT_PORTS_START( nyanpani )
	PORT_START("IN0")
	KONAMI8_SYSTEM_10
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW3:2" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW3:3" )

	PORT_START("IN1")
	KONAMI8_B123_UNK(1)

	PORT_START("IN2")
	KONAMI8_B123_UNK(2)

	PORT_START("DSW0")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* "Invalid" = both coin slots disabled */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( hcrash )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CONDITION("DSW1", 0x03, EQUALS, 0x02)        // only in WEC Le Mans 24 cabinets
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_CONDITION("DSW1", 0x03, NOTEQUALS, 0x02) // player 2?
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM )   // must be 0 otherwise game freezes when using WEC Le Mans 24 cabinet
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("adc", adc0804_device, intr_r)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* "Invalid" = both coin slots disabled */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "Konami GT without brake" )
	PORT_DIPSETTING(    0x02, "WEC Le Mans 24 Upright" )
	PORT_DIPSETTING(    0x01, "Konami GT with brake" )
	// 0x00 WEC Le Mans 24 Upright again
	PORT_BIT( 0x1c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("TEST")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Quantity of Initials" )  PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "7" )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPNAME( 0x08, 0x08, "Speed Unit" )            PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, "km/h" )
	PORT_DIPSETTING(    0x00, "M.P.H." )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	/* Konami GT specific control */
	PORT_START("PADDLE")
	PORT_BIT( 0x7f, 0x40, IPT_PADDLE ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CONDITION("DSW1", 0x03, NOTEQUALS, 0x02)

	PORT_START("IN3")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CONDITION("DSW1", 0x03, EQUALS, 0x01)        // only in Konami GT cabinet with brake
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_CONDITION("DSW1", 0x03, NOTEQUALS, 0x01)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )
//  PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 )

	/* WEC Le Mans 24 specific control */
	PORT_START("ACCEL")     /* Accelerator */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0,0x80) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_CONDITION("DSW1", 0x03, EQUALS, 0x02)

	PORT_START("WHEEL")     /* Steering Wheel */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_CONDITION("DSW1", 0x03, EQUALS, 0x02)
INPUT_PORTS_END

static INPUT_PORTS_START( bubsys )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)    // power-up
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)    // shoot
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)    // missile
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), DEF_STR( None ), SW1)
	/* "None" = coin slot B disabled */

	PORT_START("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("TEST")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Players ) )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( gradiusb )
	PORT_INCLUDE( bubsys )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "20k and every 70k" )
	PORT_DIPSETTING(    0x10, "30k and every 80k" )
	PORT_DIPSETTING(    0x08, "20k only" )
	PORT_DIPSETTING(    0x00, "30k only" )

	PORT_MODIFY("TEST")
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
INPUT_PORTS_END

static INPUT_PORTS_START( twinbeeb )
	PORT_INCLUDE( bubsys )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "20k 100k" )
	PORT_DIPSETTING(    0x10, "30k 120k" )
	PORT_DIPSETTING(    0x08, "40k 140k" )
	PORT_DIPSETTING(    0x00, "50k 160k" )
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the two bitplanes are merged in the same nibble */
	{ STEP8(0, 4) },
	{ STEP8(0, 4*8) },
	4*8*8
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the two bitplanes are merged in the same nibble */
	{ STEP16(0, 4) },
	{ STEP16(0, 4*16) },
	4*16*16
};

static const gfx_layout spritelayout3216 =
{
	32,16,  /* 32*16 sprites */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the two bitplanes are merged in the same nibble */
	{ STEP32(0, 4) },
	{ STEP16(0, 4*32) },
	4*32*16
};

static const gfx_layout spritelayout1632 =
{
	16,32,  /* 16*32 sprites */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the two bitplanes are merged in the same nibble */
	{ STEP16(0, 4) },
	{ STEP32(0, 4*16) },
	4*16*32
};

static const gfx_layout spritelayout3232 =
{
	32,32,  /* 32*32 sprites */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the two bitplanes are merged in the same nibble */
	{ STEP32(0, 4) },
	{ STEP32(0, 4*32) },
	4*32*32
};

static const gfx_layout spritelayout816 =
{
	8,16,   /* 8*16 sprites */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the two bitplanes are merged in the same nibble */
	{ STEP8(0, 4) },
	{ STEP16(0, 4*8) },
	4*8*16
};

static const gfx_layout spritelayout168 =
{
	16,8,   /* 16*8 sprites */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the two bitplanes are merged in the same nibble */
	{ STEP16(0, 4) },
	{ STEP8(0, 4*16) },
	4*16*8
};

static const uint32_t spritelayout6464_xoffset[64] = { STEP64(0, 4) };

static const uint32_t spritelayout6464_yoffset[64] = { STEP64(0, 4*64) };

static const gfx_layout spritelayout6464 =
{
	64,64,  /* 64*64 sprites */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	4*64*64,
	spritelayout6464_xoffset,
	spritelayout6464_yoffset
};

static GFXDECODE_START( gfx_nemesis )
	GFXDECODE_RAM( "charram", 0x0, charlayout,   0, 0x80 )
	GFXDECODE_RAM( "charram", 0x0, spritelayout, 0, 0x80 )
	GFXDECODE_RAM( "charram", 0x0, spritelayout3216, 0, 0x80 )
	GFXDECODE_RAM( "charram", 0x0, spritelayout816, 0, 0x80 )
	GFXDECODE_RAM( "charram", 0x0, spritelayout3232, 0, 0x80 )
	GFXDECODE_RAM( "charram", 0x0, spritelayout1632, 0, 0x80 )
	GFXDECODE_RAM( "charram", 0x0, spritelayout168, 0, 0x80 )
	GFXDECODE_RAM( "charram", 0x0, spritelayout6464, 0, 0x80 )
GFXDECODE_END

/******************************************************************************/

void nemesis_state::volume_callback(uint8_t data)
{
	m_k007232->set_volume(0, (data >> 4) * 0x11, 0);
	m_k007232->set_volume(1, 0, (data & 0x0f) * 0x11);
}

/******************************************************************************/

void nemesis_state::machine_start()
{
	save_item(NAME(m_irq_on));
	save_item(NAME(m_irq1_on));
	save_item(NAME(m_irq2_on));
	save_item(NAME(m_irq4_on));
	save_item(NAME(m_scanline_counter));
	save_item(NAME(m_gx400_irq1_cnt));
	save_item(NAME(m_gx400_speech_offset));
	save_item(NAME(m_selected_ip));
	save_item(NAME(m_tilemap_flip));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_irq_port_last));
}

void nemesis_state::machine_reset()
{
	m_irq_on = 0;
	m_gx400_irq1_cnt = 0;
	m_gx400_speech_offset = 0;
	m_scanline_counter = 0;
	m_selected_ip = 0;

	m_flipscreen = 0;
	m_tilemap_flip = 0;
	m_irq_port_last = 0;
}

void nemesis_state::set_screen_raw_params(machine_config &config)
{
	// 60.606060 Hz for 256x224
	m_screen->set_raw(XTAL(18432000.0)/3,384,0,256,264,2*8,30*8);
}

void nemesis_state::nemesis(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 18432000/2); /* 9.216 MHz? */
//  14318180/2, /* From schematics, should be accurate */
	m_maincpu->set_addrmap(AS_PROGRAM, &nemesis_state::nemesis_map);

	Z80(config, m_audiocpu, 14318180/8); /* 1.7897725MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &nemesis_state::sound_map); /* fixed */

	ls259_device &outlatch(LS259(config, "outlatch")); // 13J
	outlatch.q_out_cb<0>().set(FUNC(nemesis_state::coin1_lockout_w));
	outlatch.q_out_cb<0>().append(FUNC(nemesis_state::coin2_lockout_w));
	outlatch.q_out_cb<2>().set(FUNC(nemesis_state::sound_irq_w));

	ls259_device &intlatch(LS259(config, "intlatch")); // 11K
	intlatch.q_out_cb<0>().set(FUNC(nemesis_state::irq_enable_w));
	intlatch.q_out_cb<2>().set(FUNC(nemesis_state::gfx_flipx_w));
	intlatch.q_out_cb<3>().set(FUNC(nemesis_state::gfx_flipy_w));

	WATCHDOG_TIMER(config, "watchdog", 0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(nemesis_state::screen_update_nemesis));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(nemesis_state::nemesis_vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_nemesis);
	PALETTE(config, m_palette).set_entries(2048);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ay8910_device &ay1(AY8910(config, "ay1", 14318180/8));
	ay1.set_flags(AY8910_LEGACY_OUTPUT | AY8910_SINGLE_OUTPUT);
	ay1.port_a_read_callback().set(FUNC(nemesis_state::nemesis_portA_r));
	ay1.add_route(ALL_OUTPUTS, "filter1", 0.20);

	ay8910_device &ay2(AY8910(config, "ay2", 14318180/8));
	ay2.port_a_write_callback().set(m_k005289, FUNC(k005289_device::control_A_w));
	ay2.port_b_write_callback().set(m_k005289, FUNC(k005289_device::control_B_w));
	ay2.add_route(0, "filter2", 1.00);
	ay2.add_route(1, "filter3", 1.00);
	ay2.add_route(2, "filter4", 1.00);

	FILTER_RC(config, m_filter1);
	m_filter1->add_route(ALL_OUTPUTS, "mono", 1.0);
	FILTER_RC(config, m_filter2);
	m_filter2->add_route(ALL_OUTPUTS, "mono", 1.0);
	FILTER_RC(config, m_filter3);
	m_filter3->add_route(ALL_OUTPUTS, "mono", 1.0);
	FILTER_RC(config, m_filter4);
	m_filter4->add_route(ALL_OUTPUTS, "mono", 1.0);

	K005289(config, m_k005289, 3579545);
	m_k005289->add_route(ALL_OUTPUTS, "mono", 0.35);
}

void nemesis_state::gx400(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 18432000/2); /* 9.216MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &nemesis_state::gx400_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(nemesis_state::gx400_interrupt), "screen", 0, 1);

	Z80(config, m_audiocpu, 14318180/8);        /* 1.7897725MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &nemesis_state::gx400_sound_map);

	ls259_device &outlatch(LS259(config, "outlatch"));
	outlatch.q_out_cb<0>().set(FUNC(nemesis_state::coin1_lockout_w));
	outlatch.q_out_cb<1>().set(FUNC(nemesis_state::coin2_lockout_w));
	outlatch.q_out_cb<2>().set(FUNC(nemesis_state::sound_irq_w));
	outlatch.q_out_cb<7>().set(FUNC(nemesis_state::irq4_enable_w)); // ??

	ls259_device &intlatch(LS259(config, "intlatch"));
	intlatch.q_out_cb<0>().set(FUNC(nemesis_state::irq2_enable_w));
	intlatch.q_out_cb<1>().set(FUNC(nemesis_state::irq1_enable_w));
	intlatch.q_out_cb<2>().set(FUNC(nemesis_state::gfx_flipx_w));
	intlatch.q_out_cb<3>().set(FUNC(nemesis_state::gfx_flipy_w));

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(nemesis_state::screen_update_nemesis));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline("audiocpu", INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_nemesis);
	PALETTE(config, m_palette).set_entries(2048);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ay8910_device &ay1(AY8910(config, "ay1", 14318180/8));
	ay1.set_flags(AY8910_LEGACY_OUTPUT | AY8910_SINGLE_OUTPUT);
	ay1.port_a_read_callback().set(FUNC(nemesis_state::nemesis_portA_r));
	ay1.add_route(ALL_OUTPUTS, "filter1", 0.20);

	ay8910_device &ay2(AY8910(config, "ay2", 14318180/8));
	ay2.port_a_write_callback().set(m_k005289, FUNC(k005289_device::control_A_w));
	ay2.port_b_write_callback().set(m_k005289, FUNC(k005289_device::control_B_w));
	ay2.add_route(0, "filter2", 1.00);
	ay2.add_route(1, "filter3", 1.00);
	ay2.add_route(2, "filter4", 1.00);

	FILTER_RC(config, m_filter1);
	m_filter1->add_route(ALL_OUTPUTS, "mono", 1.0);
	FILTER_RC(config, m_filter2);
	m_filter2->add_route(ALL_OUTPUTS, "mono", 1.0);
	FILTER_RC(config, m_filter3);
	m_filter3->add_route(ALL_OUTPUTS, "mono", 1.0);
	FILTER_RC(config, m_filter4);
	m_filter4->add_route(ALL_OUTPUTS, "mono", 1.0);

	K005289(config, m_k005289, 3579545);
	m_k005289->add_route(ALL_OUTPUTS, "mono", 0.35);

	VLM5030(config, m_vlm, 3579545);
	m_vlm->set_addrmap(0, &nemesis_state::gx400_vlm_map);
	m_vlm->add_route(ALL_OUTPUTS, "mono", 0.70);
}

void nemesis_state::konamigt(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 18432000/2); /* 9.216 MHz? */
	m_maincpu->set_addrmap(AS_PROGRAM, &nemesis_state::konamigt_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(nemesis_state::konamigt_interrupt), "screen", 0, 1);

	Z80(config, m_audiocpu, 14318180/8);        /* 1.7897725MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &nemesis_state::sound_map);

	ls259_device &outlatch(LS259(config, "outlatch"));
	outlatch.q_out_cb<0>().set(FUNC(nemesis_state::coin2_lockout_w));
	outlatch.q_out_cb<1>().set(FUNC(nemesis_state::coin1_lockout_w));
	outlatch.q_out_cb<2>().set(FUNC(nemesis_state::sound_irq_w));

	ls259_device &intlatch(LS259(config, "intlatch"));
	intlatch.q_out_cb<0>().set(FUNC(nemesis_state::irq2_enable_w));
	intlatch.q_out_cb<1>().set(FUNC(nemesis_state::irq_enable_w));
	intlatch.q_out_cb<2>().set(FUNC(nemesis_state::gfx_flipx_w));
	intlatch.q_out_cb<3>().set(FUNC(nemesis_state::gfx_flipy_w));

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(nemesis_state::screen_update_nemesis));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_nemesis);
	PALETTE(config, m_palette).set_entries(2048);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ay8910_device &ay1(AY8910(config, "ay1", 14318180/8));
	ay1.set_flags(AY8910_LEGACY_OUTPUT | AY8910_SINGLE_OUTPUT);
	ay1.port_a_read_callback().set(FUNC(nemesis_state::nemesis_portA_r));
	ay1.add_route(ALL_OUTPUTS, "filter1", 0.20);

	ay8910_device &ay2(AY8910(config, "ay2", 14318180/8));
	ay2.port_a_write_callback().set(m_k005289, FUNC(k005289_device::control_A_w));
	ay2.port_b_write_callback().set(m_k005289, FUNC(k005289_device::control_B_w));
	ay2.add_route(0, "filter2", 1.00);
	ay2.add_route(1, "filter3", 1.00);
	ay2.add_route(2, "filter4", 1.00);

	FILTER_RC(config, m_filter1);
	m_filter1->add_route(ALL_OUTPUTS, "mono", 1.0);
	FILTER_RC(config, m_filter2);
	m_filter2->add_route(ALL_OUTPUTS, "mono", 1.0);
	FILTER_RC(config, m_filter3);
	m_filter3->add_route(ALL_OUTPUTS, "mono", 1.0);
	FILTER_RC(config, m_filter4);
	m_filter4->add_route(ALL_OUTPUTS, "mono", 1.0);

	K005289(config, m_k005289, 3579545);
	m_k005289->add_route(ALL_OUTPUTS, "mono", 0.60);
}

void nemesis_state::rf2_gx400(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 18432000/2); /* 9.216MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &nemesis_state::rf2_gx400_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(nemesis_state::gx400_interrupt), "screen", 0, 1);

	Z80(config, m_audiocpu, 14318180/8); /* 1.7897725MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &nemesis_state::gx400_sound_map);

	ls259_device &outlatch(LS259(config, "outlatch"));
	outlatch.q_out_cb<0>().set(FUNC(nemesis_state::coin1_lockout_w));
	outlatch.q_out_cb<1>().set(FUNC(nemesis_state::coin2_lockout_w));
	outlatch.q_out_cb<2>().set(FUNC(nemesis_state::sound_irq_w));
	outlatch.q_out_cb<7>().set(FUNC(nemesis_state::irq4_enable_w)); // ??

	ls259_device &intlatch(LS259(config, "intlatch"));
	intlatch.q_out_cb<0>().set(FUNC(nemesis_state::irq2_enable_w));
	intlatch.q_out_cb<1>().set(FUNC(nemesis_state::irq1_enable_w));
	intlatch.q_out_cb<2>().set(FUNC(nemesis_state::gfx_flipx_w));
	intlatch.q_out_cb<3>().set(FUNC(nemesis_state::gfx_flipy_w));

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(nemesis_state::screen_update_nemesis));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline("audiocpu", INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_nemesis);
	PALETTE(config, m_palette).set_entries(2048);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ay8910_device &ay1(AY8910(config, "ay1", 14318180/8));
	ay1.set_flags(AY8910_LEGACY_OUTPUT | AY8910_SINGLE_OUTPUT);
	ay1.port_a_read_callback().set(FUNC(nemesis_state::nemesis_portA_r));
	ay1.add_route(ALL_OUTPUTS, "filter1", 0.20);

	ay8910_device &ay2(AY8910(config, "ay2", 14318180/8));
	ay2.port_a_write_callback().set(m_k005289, FUNC(k005289_device::control_A_w));
	ay2.port_b_write_callback().set(m_k005289, FUNC(k005289_device::control_B_w));
	ay2.add_route(0, "filter2", 1.00);
	ay2.add_route(1, "filter3", 1.00);
	ay2.add_route(2, "filter4", 1.00);

	FILTER_RC(config, m_filter1);
	m_filter1->add_route(ALL_OUTPUTS, "mono", 1.0);
	FILTER_RC(config, m_filter2);
	m_filter2->add_route(ALL_OUTPUTS, "mono", 1.0);
	FILTER_RC(config, m_filter3);
	m_filter3->add_route(ALL_OUTPUTS, "mono", 1.0);
	FILTER_RC(config, m_filter4);
	m_filter4->add_route(ALL_OUTPUTS, "mono", 1.0);

	K005289(config, m_k005289, 3579545);
	m_k005289->add_route(ALL_OUTPUTS, "mono", 0.60);

	VLM5030(config, m_vlm, 3579545);
	m_vlm->set_addrmap(0, &nemesis_state::gx400_vlm_map);
	m_vlm->add_route(ALL_OUTPUTS, "mono", 0.70);
}

void nemesis_state::salamand(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 18432000/2); /* 9.216MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &nemesis_state::salamand_map);

	Z80(config, m_audiocpu, 3579545); /* 3.579545 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &nemesis_state::sal_sound_map);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(nemesis_state::screen_update_nemesis));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(nemesis_state::nemesis_vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_nemesis);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);
	m_palette->set_membits(8);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, "soundlatch");

	VLM5030(config, m_vlm, 3579545);
	m_vlm->set_addrmap(0, &nemesis_state::salamand_vlm_map);
	m_vlm->add_route(ALL_OUTPUTS, "lspeaker", 2.50);
	m_vlm->add_route(ALL_OUTPUTS, "rspeaker", 2.50);

	K007232(config, m_k007232, 3579545);
	m_k007232->port_write().set(FUNC(nemesis_state::volume_callback));
	m_k007232->add_route(ALL_OUTPUTS, "lspeaker", 0.08);
	m_k007232->add_route(ALL_OUTPUTS, "rspeaker", 0.08);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 3579545));
//  ymsnd.irq_handler().set_inputline(m_audiocpu, 0); ... Interrupts _are_ generated, I wonder where they go
	ymsnd.add_route(0, "rspeaker", 1.2); // reversed according to MT #4565
	ymsnd.add_route(1, "lspeaker", 1.2);
}

void nemesis_state::blkpnthr(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 18432000/2); /* 9.216 MHz? */
	m_maincpu->set_addrmap(AS_PROGRAM, &nemesis_state::blkpnthr_map);

	Z80(config, m_audiocpu, 3579545); /* 3.579545 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &nemesis_state::blkpnthr_sound_map);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(nemesis_state::screen_update_nemesis));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(nemesis_state::blkpnthr_vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_nemesis);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);
	m_palette->set_membits(8);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, "soundlatch");

	K007232(config, m_k007232, 3579545);
	m_k007232->port_write().set(FUNC(nemesis_state::volume_callback));
	m_k007232->add_route(ALL_OUTPUTS, "lspeaker", 0.10);
	m_k007232->add_route(ALL_OUTPUTS, "rspeaker", 0.10);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 3579545));
//  ymsnd.irq_handler().set_inputline(m_audiocpu, 0); ... Interrupts _are_ generated, I wonder where they go
	ymsnd.add_route(0, "lspeaker", 1.0);
	ymsnd.add_route(1, "rspeaker", 1.0);
}

void nemesis_state::citybomb(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 18432000/2); /* 9.216 MHz? */
	m_maincpu->set_addrmap(AS_PROGRAM, &nemesis_state::citybomb_map);

	Z80(config, m_audiocpu, 3579545); /* 3.579545 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &nemesis_state::city_sound_map);

	adc0804_device &adc(ADC0804(config, "adc", RES_K(10), CAP_P(150)));
	adc.vin_callback().set(FUNC(nemesis_state::selected_ip_r));

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(nemesis_state::screen_update_nemesis));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(nemesis_state::nemesis_vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_nemesis);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);
	m_palette->set_membits(8);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	K007232(config, m_k007232, 3579545);
	m_k007232->port_write().set(FUNC(nemesis_state::volume_callback));
	m_k007232->add_route(ALL_OUTPUTS, "mono", 0.30);

	ym3812_device &ym3812(YM3812(config, "ymsnd", 3579545));
//  ym3812.irq_handler().set_inputline("audiocpu", 0); ... Interrupts _are_ generated, I wonder where they go
	ym3812.add_route(ALL_OUTPUTS, "mono", 1.0);

	k051649_device &k051649(K051649(config, "k051649", 3579545));
	k051649.add_route(ALL_OUTPUTS, "mono", 0.38);
}

void nemesis_state::nyanpani(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 18432000/2); /* 9.216 MHz? */
	m_maincpu->set_addrmap(AS_PROGRAM, &nemesis_state::nyanpani_map);

	Z80(config, m_audiocpu, 3579545); /* 3.579545 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &nemesis_state::city_sound_map);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(nemesis_state::screen_update_nemesis));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(nemesis_state::nemesis_vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_nemesis);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);
	m_palette->set_membits(8);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	K007232(config, m_k007232, 3579545);
	m_k007232->port_write().set(FUNC(nemesis_state::volume_callback));
	m_k007232->add_route(ALL_OUTPUTS, "mono", 0.30);

	ym3812_device &ym3812(YM3812(config, "ymsnd", 3579545));
//  ym3812.irq_handler().set_inputline("audiocpu", 0); ... Interrupts _are_ generated, I wonder where they go
	ym3812.add_route(ALL_OUTPUTS, "mono", 1.0);

	k051649_device &k051649(K051649(config, "k051649", 3579545));
	k051649.add_route(ALL_OUTPUTS, "mono", 0.38);
}

void nemesis_state::hcrash(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 18432000/3); /* 6.144MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &nemesis_state::hcrash_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(nemesis_state::hcrash_interrupt), "screen", 0, 1);

	Z80(config, m_audiocpu, 14318180/4); /* 3.579545 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &nemesis_state::sal_sound_map);

	adc0804_device &adc(ADC0804(config, "adc", 640000)); // unknown clock (doesn't seem to be R/C here)
	adc.vin_callback().set(FUNC(nemesis_state::selected_ip_r));

	ls259_device &intlatch(LS259(config, "intlatch"));
	intlatch.q_out_cb<0>().set_nop(); // ?
	intlatch.q_out_cb<1>().set(FUNC(nemesis_state::irq2_enable_w)); // or at 0x0c2804 ?
	intlatch.q_out_cb<2>().set_nop(); // ?

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(nemesis_state::screen_update_nemesis));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_nemesis);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);
	m_palette->set_membits(8);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, "soundlatch");

	VLM5030(config, m_vlm, 3579545);
	m_vlm->set_addrmap(0, &nemesis_state::salamand_vlm_map);
	m_vlm->add_route(ALL_OUTPUTS, "lspeaker", 2.00);
	m_vlm->add_route(ALL_OUTPUTS, "rspeaker", 2.00);

	K007232(config, m_k007232, 3579545);
	m_k007232->port_write().set(FUNC(nemesis_state::volume_callback));
	m_k007232->add_route(ALL_OUTPUTS, "lspeaker", 0.10);
	m_k007232->add_route(ALL_OUTPUTS, "rspeaker", 0.10);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 3579545));
//  ymsnd.irq_handler().set_inputline(m_audiocpu, 0); ... Interrupts _are_ generated, I wonder where they go
	ymsnd.add_route(0, "lspeaker", 0.50);
	ymsnd.add_route(1, "rspeaker", 0.50);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( nemesis )
	ROM_REGION( 0x40000, "maincpu", 0 )    /* 4 * 64k for code and rom */
	ROM_LOAD16_BYTE( "456-d01.12a",   0x00000, 0x8000, CRC(35ff1aaa) SHA1(2879a5d2ff7dca217fe5cd40be871878294c491f) )
	ROM_LOAD16_BYTE( "456-d05.12c",   0x00001, 0x8000, CRC(23155faa) SHA1(08c73c669b3a5275353cbfcbe58ced92d93244a7) )
	ROM_LOAD16_BYTE( "456-d02.13a",   0x10000, 0x8000, CRC(ac0cf163) SHA1(8b1a46c3ad102fe78cf099425e108d09dafd0955) )
	ROM_LOAD16_BYTE( "456-d06.13c",   0x10001, 0x8000, CRC(023f22a9) SHA1(0b9096b9cfcc3ed273de04c93227ab24c63513e8) )
	ROM_LOAD16_BYTE( "456-d03.14a",   0x20000, 0x8000, CRC(8cefb25f) SHA1(876b1974ca76ca89f8b8ea45b4ba9ec82d7c7228) )
	ROM_LOAD16_BYTE( "456-d07.14c",   0x20001, 0x8000, CRC(d50b82cb) SHA1(71e9fbe51272788e2ef6f150c7bff59ac8c28f1d) )
	ROM_LOAD16_BYTE( "456-d04.15a",   0x30000, 0x8000, CRC(9ca75592) SHA1(04388f2874faa54dd2cabfec4d6ce3e8d164cbcc) )
	ROM_LOAD16_BYTE( "456-d08.15c",   0x30001, 0x8000, CRC(03c0b7f5) SHA1(4eb31bcbd2ee66afe4158308351a57589c5a1e4e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound */
	ROM_LOAD(      "456-d09.9c",   0x00000, 0x4000, CRC(26bf9636) SHA1(009dcbf18ea6230fc75a72232bd4fc29ad28dbf0) )

	ROM_REGION( 0x0200,  "k005289", 0 )      /* 2x 256 byte for 0005289 wavetable data */
	ROM_LOAD(      "400-a01.fse",  0x00000, 0x0100, CRC(5827b1e8) SHA1(fa8cf5f868cfb08bce203baaebb6c4055ee2a000) )
	ROM_LOAD(      "400-a02.fse",  0x00100, 0x0100, CRC(2f44f970) SHA1(7ab46f9d5d587665782cefc623b8de0124a6d38a) )
ROM_END

ROM_START( nemesisuk )
	ROM_REGION( 0x40000, "maincpu", 0 )    /* 4 * 64k for code and rom */
	ROM_LOAD16_BYTE( "456-e01.12a",   0x00000, 0x8000, CRC(e1993f91) SHA1(6759bb9ba0ce28ad4d7f61b824a7d0fe43215bdc) )
	ROM_LOAD16_BYTE( "456-e05.12c",   0x00001, 0x8000, CRC(c9761c78) SHA1(bfd63517efa820a05a0d9a908dd0917cd0d01b77) )
	ROM_LOAD16_BYTE( "456-e02.13a",   0x10000, 0x8000, CRC(f6169c4b) SHA1(047a204fbcf8c24eca2db7197d4297e5a28c2b42) )
	ROM_LOAD16_BYTE( "456-e06.13c",   0x10001, 0x8000, CRC(af58c548) SHA1(a15725c14b6e7840c84ab2bd4cf3668bbaf35abf) )
	ROM_LOAD16_BYTE( "456-e03.14a",   0x20000, 0x8000, CRC(8cefb25f) SHA1(876b1974ca76ca89f8b8ea45b4ba9ec82d7c7228) ) /* Labeled "E03" but same as above set */
	ROM_LOAD16_BYTE( "456-e07.14c",   0x20001, 0x8000, CRC(d50b82cb) SHA1(71e9fbe51272788e2ef6f150c7bff59ac8c28f1d) ) /* Labeled "E07" but same as above set */
	ROM_LOAD16_BYTE( "456-e04.15a",   0x30000, 0x8000, CRC(322423d0) SHA1(6106b607132a09193353f339d06032a13b1e3de8) )
	ROM_LOAD16_BYTE( "456-e08.15c",   0x30001, 0x8000, CRC(eb656266) SHA1(2f4abea282d30775f7a25747eb41bfd8d5299967) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound */
	ROM_LOAD(      "456-b09.9c",   0x00000, 0x4000, CRC(26bf9636) SHA1(009dcbf18ea6230fc75a72232bd4fc29ad28dbf0) ) /* Labeled "B09" but same as above set */

	ROM_REGION( 0x0200,  "k005289", 0 )      /* 2x 256 byte for 0005289 wavetable data */
	ROM_LOAD(      "400-a01.fse",  0x00000, 0x0100, CRC(5827b1e8) SHA1(fa8cf5f868cfb08bce203baaebb6c4055ee2a000) )
	ROM_LOAD(      "400-a02.fse",  0x00100, 0x0100, CRC(2f44f970) SHA1(7ab46f9d5d587665782cefc623b8de0124a6d38a) )
ROM_END

ROM_START( konamigt )
	ROM_REGION( 0x40000, "maincpu", 0 )    /* 4 * 64k for code and rom */
	ROM_LOAD16_BYTE( "561-c01.12a",   0x00000, 0x8000, CRC(56245bfd) SHA1(12579ae0031c172d42b766f5a801ef479148105e) )
	ROM_LOAD16_BYTE( "561-c05.12c",   0x00001, 0x8000, CRC(8d651f44) SHA1(0d057ce063dd19c0a708cffa413511b367206682) )
	ROM_LOAD16_BYTE( "561-c02.13a",   0x10000, 0x8000, CRC(3407b7cb) SHA1(1df834a47e3b4cabc79ece4cd90e05e5df68df9a) )
	ROM_LOAD16_BYTE( "561-c06.13c",   0x10001, 0x8000, CRC(209942d4) SHA1(953321eeed88086dee3a9f8cd596191f19260b3a) )
	ROM_LOAD16_BYTE( "561-b03.14a",   0x20000, 0x8000, CRC(aef7df48) SHA1(04d3e79e8fa0e332d92738094933069bcdbdfeab) )
	ROM_LOAD16_BYTE( "561-b07.14c",   0x20001, 0x8000, CRC(e9bd6250) SHA1(507b72c7e5f8fb7b6feb357ec522e814e25f2cc1) )
	ROM_LOAD16_BYTE( "561-b04.15a",   0x30000, 0x8000, CRC(94bd4bd7) SHA1(314b537ba97dec1a91dcfc5deeb1dd9f7bb4a930) )
	ROM_LOAD16_BYTE( "561-b08.15c",   0x30001, 0x8000, CRC(b7236567) SHA1(7626d70262a0acff36357877a5e7c9ed3f45415e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound */
	ROM_LOAD(       "561-b09.9c",  0x00000, 0x4000, CRC(539d0c49) SHA1(4c16b07fbd876b6445fc0ec49c3ad5ab1a92cbf6) )

	ROM_REGION( 0x0200,  "k005289", 0 )      /* 2x 256 byte for 0005289 wavetable data */
	ROM_LOAD(      "400-a01.fse",  0x00000, 0x0100, CRC(5827b1e8) SHA1(fa8cf5f868cfb08bce203baaebb6c4055ee2a000) )
	ROM_LOAD(      "400-a02.fse",  0x00100, 0x0100, CRC(2f44f970) SHA1(7ab46f9d5d587665782cefc623b8de0124a6d38a) )
ROM_END

ROM_START( rf2 )
	ROM_REGION( 0xc0000, "maincpu", 0 )    /* 5 * 64k for code and rom */
	ROM_LOAD16_BYTE( "400-a06.15l",  0x00000, 0x08000, CRC(b99d8cff) SHA1(18e277827a534bab2b3b8b81e51d886b8382d435) )
	ROM_LOAD16_BYTE( "400-a04.10l",  0x00001, 0x08000, CRC(d02c9552) SHA1(ec0aaa093541dab98412c11f666161cd558c383a) )
	ROM_LOAD16_BYTE( "561-a07.17l",  0x80000, 0x20000, CRC(ed6e7098) SHA1(a28f2846b091b5bc333088054451d7b6d7f6458e) )
	ROM_LOAD16_BYTE( "561-a05.12l",  0x80001, 0x20000, CRC(dfe04425) SHA1(0817992aeeba140feba1417c265b794f096936d9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound */
	ROM_LOAD(      "400-e03.5l",   0x00000, 0x02000, CRC(a5a8e57d) SHA1(f4236770093392dec3f76835a5766e9b3ed64e2e) )

	ROM_REGION( 0x0200,  "k005289", 0 )      /* 2x 256 byte for 0005289 wavetable data */
	ROM_LOAD(      "400-a01.fse",  0x00000, 0x0100, CRC(5827b1e8) SHA1(fa8cf5f868cfb08bce203baaebb6c4055ee2a000) )
	ROM_LOAD(      "400-a02.fse",  0x00100, 0x0100, CRC(2f44f970) SHA1(7ab46f9d5d587665782cefc623b8de0124a6d38a) )
ROM_END

ROM_START( twinbee )
	ROM_REGION( 0xc0000, "maincpu", 0 )    /* 5 * 64k for code and rom */
	ROM_LOAD16_BYTE( "400-a06.15l",  0x00000, 0x08000, CRC(b99d8cff) SHA1(18e277827a534bab2b3b8b81e51d886b8382d435) )
	ROM_LOAD16_BYTE( "400-a04.10l",  0x00001, 0x08000, CRC(d02c9552) SHA1(ec0aaa093541dab98412c11f666161cd558c383a) )
	ROM_LOAD16_BYTE( "412-a07.17l",  0x80000, 0x20000, CRC(d93c5499) SHA1(4555b9232ce86192360ea5b5092643ff51446aa0) )
	ROM_LOAD16_BYTE( "412-a05.12l",  0x80001, 0x20000, CRC(2b357069) SHA1(409cf3aa174f5d7dc5efc8b8b1c925fcb677fc98) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound */
	ROM_LOAD(      "400-e03.5l",   0x00000, 0x02000, CRC(a5a8e57d) SHA1(f4236770093392dec3f76835a5766e9b3ed64e2e) )

	ROM_REGION( 0x0200,  "k005289", 0 )      /* 2x 256 byte for 0005289 wavetable data */
	ROM_LOAD(      "400-a01.fse",  0x00000, 0x0100, CRC(5827b1e8) SHA1(fa8cf5f868cfb08bce203baaebb6c4055ee2a000) )
	ROM_LOAD(      "400-a02.fse",  0x00100, 0x0100, CRC(2f44f970) SHA1(7ab46f9d5d587665782cefc623b8de0124a6d38a) )
ROM_END

ROM_START( gradius )
	ROM_REGION( 0xc0000, "maincpu", 0 )    /* 5 * 64k for code and rom */
	ROM_LOAD16_BYTE( "400-a06.15l",  0x00000, 0x08000, CRC(b99d8cff) SHA1(18e277827a534bab2b3b8b81e51d886b8382d435) )
	ROM_LOAD16_BYTE( "400-a04.10l",  0x00001, 0x08000, CRC(d02c9552) SHA1(ec0aaa093541dab98412c11f666161cd558c383a) )
	ROM_LOAD16_BYTE( "456-a07.17l",  0x80000, 0x20000, CRC(92df792c) SHA1(aec916f70af92a2d6476d7a36ba9be265890f9aa) )
	ROM_LOAD16_BYTE( "456-a05.12l",  0x80001, 0x20000, CRC(5cafb263) SHA1(7cd12c695ec6ef4d5785ce218911961fc3528e95) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound */
	ROM_LOAD(      "400-e03.5l",   0x00000, 0x2000, CRC(a5a8e57d) SHA1(f4236770093392dec3f76835a5766e9b3ed64e2e) )

	ROM_REGION( 0x0200,  "k005289", 0 )      /* 2x 256 byte for 0005289 wavetable data */
	ROM_LOAD(      "400-a01.fse",  0x00000, 0x0100, CRC(5827b1e8) SHA1(fa8cf5f868cfb08bce203baaebb6c4055ee2a000) )
	ROM_LOAD(      "400-a02.fse",  0x00100, 0x0100, CRC(2f44f970) SHA1(7ab46f9d5d587665782cefc623b8de0124a6d38a) )
ROM_END

ROM_START( gwarrior )
	ROM_REGION( 0xc0000, "maincpu", 0 )    /* 5 * 64k for code and rom */
	ROM_LOAD16_BYTE( "400-a06.15l",  0x00000, 0x08000, CRC(b99d8cff) SHA1(18e277827a534bab2b3b8b81e51d886b8382d435) )
	ROM_LOAD16_BYTE( "400-a04.10l",  0x00001, 0x08000, CRC(d02c9552) SHA1(ec0aaa093541dab98412c11f666161cd558c383a) )
	ROM_LOAD16_BYTE( "578-a07.17l",  0x80000, 0x20000, CRC(0aedacb5) SHA1(bf8e4b443df37e021a86e1fe76683113977a1a76) )
	ROM_LOAD16_BYTE( "578-a05.12l",  0x80001, 0x20000, CRC(76240e2e) SHA1(3f4086972fa655704ec6480fa3012c3e8999d8ab) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound */
	ROM_LOAD(      "400-e03.5l",   0x00000, 0x02000, CRC(a5a8e57d) SHA1(f4236770093392dec3f76835a5766e9b3ed64e2e) )

	ROM_REGION( 0x0200,  "k005289", 0 )      /* 2x 256 byte for 0005289 wavetable data */
	ROM_LOAD(      "400-a01.fse",  0x00000, 0x0100, CRC(5827b1e8) SHA1(fa8cf5f868cfb08bce203baaebb6c4055ee2a000) )
	ROM_LOAD(      "400-a02.fse",  0x00100, 0x0100, CRC(2f44f970) SHA1(7ab46f9d5d587665782cefc623b8de0124a6d38a) )
ROM_END

ROM_START( salamand )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "587-d02.18b",  0x00000, 0x10000, CRC(a42297f9) SHA1(7c974779e438eae649b39b36f6f6d24847099a6e) )
	ROM_LOAD16_BYTE( "587-d05.18c",  0x00001, 0x10000, CRC(f9130b0a) SHA1(925ea65c13fc87fc59f893cc0ead2c82fd0bed6f) )
	ROM_LOAD16_BYTE( "587-c03.17b",  0x40000, 0x20000, CRC(e5caf6e6) SHA1(f5df4fbc43cfa6e2866558c99dd95ba8dc89dc7a) ) /* Mask rom */
	ROM_LOAD16_BYTE( "587-c06.17c",  0x40001, 0x20000, CRC(c2f567ea) SHA1(0c38fea53f3d4a9ae0deada5669deca4be8c9fd3) ) /* Mask rom */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound */
	ROM_LOAD(      "587-d09.11j",      0x00000, 0x08000, CRC(5020972c) SHA1(04c752c3b7fd850a8a51ecd230b39e6edde9dd7e) )

	ROM_REGION( 0x04000, "vlm", 0 )    /* VLM5030 data */
	ROM_LOAD(      "587-d08.8g",       0x00000, 0x04000, CRC(f9ac6b82) SHA1(3370fc3a7f82e922e19d54afb3bca7b07fa4aa9a) )

	ROM_REGION( 0x20000, "k007232", 0 )    /* 007232 data */
	ROM_LOAD(      "587-c01.10a",      0x00000, 0x20000, CRC(09fe0632) SHA1(4c3b29c623d70bbe8a938a0beb4638912c46fb6a) ) /* Mask rom */
ROM_END

ROM_START( salamandj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "587-j02.18b",  0x00000, 0x10000, CRC(f68ee99a) SHA1(aec1f4720abe2529120ae711daa9e7e7d966b351) )
	ROM_LOAD16_BYTE( "587-j05.18c",  0x00001, 0x10000, CRC(72c16128) SHA1(6921445caa0b1121e483c9c62c17aad8aa42cc18) )
	ROM_LOAD16_BYTE( "587-c03.17b",  0x40000, 0x20000, CRC(e5caf6e6) SHA1(f5df4fbc43cfa6e2866558c99dd95ba8dc89dc7a) ) /* Mask rom */
	ROM_LOAD16_BYTE( "587-c06.17c",  0x40001, 0x20000, CRC(c2f567ea) SHA1(0c38fea53f3d4a9ae0deada5669deca4be8c9fd3) ) /* Mask rom */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound */
	ROM_LOAD(      "587-d09.11j",      0x00000, 0x08000, CRC(5020972c) SHA1(04c752c3b7fd850a8a51ecd230b39e6edde9dd7e) )

	ROM_REGION( 0x04000, "vlm", 0 )    /* VLM5030 data */
	ROM_LOAD(      "587-d08.8g",       0x00000, 0x04000, CRC(f9ac6b82) SHA1(3370fc3a7f82e922e19d54afb3bca7b07fa4aa9a) )

	ROM_REGION( 0x20000, "k007232", 0 )    /* 007232 data */
	ROM_LOAD(      "587-c01.10a",      0x00000, 0x20000, CRC(09fe0632) SHA1(4c3b29c623d70bbe8a938a0beb4638912c46fb6a) ) /* Mask rom */
ROM_END

// Original Konami PCBs PWB (B) 201012A GX587 + GX400PWD(A)200204C
ROM_START( salamandt )
	ROM_REGION( 0x80000, "maincpu", 0 )  // Same program ROMs content as 'salamand', but with smaller ROMs
	ROM_LOAD16_BYTE( "5_27512.18b",  0x00000, 0x10000, CRC(a42297f9) SHA1(7c974779e438eae649b39b36f6f6d24847099a6e) )
	ROM_LOAD16_BYTE( "6_27512.18c",  0x00001, 0x10000, CRC(f9130b0a) SHA1(925ea65c13fc87fc59f893cc0ead2c82fd0bed6f) )
	ROM_LOAD16_BYTE( "10_27512.17b", 0x40000, 0x10000, CRC(b83e8724) SHA1(69707a98ac3e15f240b24812ff639cafd99c306d) )
	ROM_LOAD16_BYTE( "4_27512.17c",  0x40001, 0x10000, CRC(a6ef6dc4) SHA1(639113b506fce4318d4b79efd8717232c31dd748) )
	ROM_LOAD16_BYTE( "9_27512.17b",  0x60000, 0x10000, CRC(b4d2fec9) SHA1(b095e003d86f6d5f486a1d9a4cf02572ff20edeb) )
	ROM_LOAD16_BYTE( "3_27512.17c",  0x60001, 0x10000, CRC(6ea59643) SHA1(cb2ba819a601eb417eb67d50568126269fa613a4) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // 64k for sound
	ROM_LOAD( "2_tmm24256ap.11j",    0x00000, 0x08000, CRC(7eb8bb88) SHA1(8eb3063b0f4b0775b80f25cc6588661c0e61227d) ) // Unique for this set

	ROM_REGION( 0x04000, "vlm", 0 )      // VLM5030 data
	ROM_LOAD( "1_27128.8g",          0x00000, 0x04000, CRC(f9ac6b82) SHA1(3370fc3a7f82e922e19d54afb3bca7b07fa4aa9a) )

	ROM_REGION( 0x20000, "k007232", 0 )  // 007232 data
	ROM_LOAD( "8_27512.10a",         0x00000, 0x10000, CRC(cf477da4) SHA1(92867b3488228a802f0176139040c4eaf2b92700) )
	ROM_LOAD( "7_27512.10a",         0x10000, 0x10000, CRC(52384e79) SHA1(06d7003d8746287d95710dc6b52556aa642ccc83) )

	ROM_REGION( 0x20, "pld", 0 )
	ROM_LOAD( "007366_pal8l14a.19d", 0x00000, 0x00020, CRC(77304735) SHA1(1f9dc7b78d4f7a40e7886d106d07b6349abaaa57) )
ROM_END

ROM_START( lifefrce )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "587-l02.18b",  0x00000, 0x10000, CRC(4a44da18) SHA1(8e76bc2b9c48bfc65664fb6ee4d1d33622ee1eb8) )
	ROM_LOAD16_BYTE( "587-l05.18c",  0x00001, 0x10000, CRC(2f8c1cbd) SHA1(aa309d509be69f315e50047abff42d9b30334e1d) )
	ROM_LOAD16_BYTE( "6107.17b",     0x40000, 0x20000, CRC(e5caf6e6) SHA1(f5df4fbc43cfa6e2866558c99dd95ba8dc89dc7a) ) /* Mask rom */
	ROM_LOAD16_BYTE( "6108.17c",     0x40001, 0x20000, CRC(c2f567ea) SHA1(0c38fea53f3d4a9ae0deada5669deca4be8c9fd3) ) /* Mask rom */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound */
	ROM_LOAD(      "587-k09.11j",    0x00000, 0x08000, CRC(2255fe8c) SHA1(6ee35575a15f593642b29020857ec466094ef495) )

	ROM_REGION( 0x04000, "vlm", 0 )    /* VLM5030 data */
	ROM_LOAD(      "587-k08.8g",     0x00000, 0x04000, CRC(7f0e9b41) SHA1(c9fc2723fac55691dfbb4cf9b3c472a42efa97c9) )

	ROM_REGION( 0x20000, "k007232", 0 )    /* 007232 data */
	ROM_LOAD(      "6106.10a",       0x00000, 0x20000, CRC(09fe0632) SHA1(4c3b29c623d70bbe8a938a0beb4638912c46fb6a) ) /* Mask rom */
ROM_END

ROM_START( lifefrcej )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "587-n02.18b",  0x00000, 0x10000, CRC(235dba71) SHA1(f3a0092a7d002436253054953e36d0865ce95b80) )
	ROM_LOAD16_BYTE( "587-n05.18c",  0x00001, 0x10000, CRC(054e569f) SHA1(e810f7e3e762875e2e71e4356997257e1bbe0da1) )
	ROM_LOAD16_BYTE( "587-n03.17b",  0x40000, 0x20000, CRC(9041f850) SHA1(d62b8c3132916a4053cb282448b2404ac0143e01) )
	ROM_LOAD16_BYTE( "587-n06.17c",  0x40001, 0x20000, CRC(fba8b6aa) SHA1(5ef861b89b7a89c9d70355e09621b106baa5c1e7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound */
	ROM_LOAD(      "587-n09.11j",  0x00000, 0x08000, CRC(e8496150) SHA1(c7d40b6dc56849dfd8d080f1aaebad36c88d93df) )

	ROM_REGION( 0x04000, "vlm", 0 )    /* VLM5030 data */
	ROM_LOAD(      "587-n08.8g",  0x00000, 0x04000, CRC(7f0e9b41) SHA1(c9fc2723fac55691dfbb4cf9b3c472a42efa97c9) BAD_DUMP ) // TODO: verify if contents are different from K08

	ROM_REGION( 0x20000, "k007232", 0 )    /* 007232 data */
	ROM_LOAD(      "587-c01.10a", 0x00000, 0x20000, CRC(09fe0632) SHA1(4c3b29c623d70bbe8a938a0beb4638912c46fb6a) ) /* Mask rom */
ROM_END

ROM_START( blkpnthr )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "604-f02.18b",  0x00000, 0x10000, CRC(487bf8da) SHA1(43b01599a1e3f82972d597a7a92bdd4ce1343847) )
	ROM_LOAD16_BYTE( "604-f05.18c",  0x00001, 0x10000, CRC(b08f8ca2) SHA1(ca3b17709a86abdcfa0034ccb4ff8d0afc84558f) )
	ROM_LOAD16_BYTE( "604-c03.17b",  0x40000, 0x20000, CRC(815bc3b0) SHA1(ee643b9af5906d12b1d621996503c2e28d93a207) )
	ROM_LOAD16_BYTE( "604-c06.17c",  0x40001, 0x20000, CRC(4af6bf7f) SHA1(bf6d128670dda1f30cbf72cb82b61bf6ddfcde60) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound */
	ROM_LOAD(      "604-a08.11j",  0x00000, 0x08000, CRC(aff88a2b) SHA1(7080add63deab5755606759a218dea9105df4819) )

	ROM_REGION( 0x20000, "k007232", 0 )    /* 007232 data */
	ROM_LOAD(      "604-a01.10a",  0x00000, 0x20000, CRC(eceb64a6) SHA1(028157d336770fe4ca17c24476d62a790255898a) )
ROM_END

ROM_START( citybomb )
	ROM_REGION( 0x1c0000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "787-g10.15k",  0x000000, 0x10000, CRC(26207530) SHA1(ccb5e4ca472aad11cf308973d6a020d3af22a134) )
	ROM_LOAD16_BYTE( "787-g09.15h",  0x000001, 0x10000, CRC(ce7de262) SHA1(73ab58c057113ffffb633c314fa383e65236d423) )
	ROM_LOAD16_BYTE( "787-g08.15f",  0x100000, 0x20000, CRC(6242ef35) SHA1(16fd4478d54117bbf09792e22c786622ca5049bb) )
	ROM_LOAD16_BYTE( "787-g07.15d",  0x100001, 0x20000, CRC(21be5e9e) SHA1(37fdf6d6fe3e84e897f2d908afdfc47e8d4a9265) )
	ROM_LOAD16_BYTE( "787-e06.14f",  0x140000, 0x20000, CRC(c251154a) SHA1(7c6a1f862ddf64a604164b85e4a04bb01e2966a7) )
	ROM_LOAD16_BYTE( "787-e05.14d",  0x140001, 0x20000, CRC(0781e22d) SHA1(03a998ee47194960af4dde2bf553359fe8a3aee7) )
	ROM_LOAD16_BYTE( "787-g04.13f",  0x180000, 0x20000, CRC(137cf39f) SHA1(39cfd25c45d824cabc3641fd39eb77c98d32ec9b) )
	ROM_LOAD16_BYTE( "787-g03.13d",  0x180001, 0x20000, CRC(0cc704dc) SHA1(b0c3991393cdb6a75461597d51452bfa08955081) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound */
	ROM_LOAD(      "787-e02.4h",  0x00000, 0x08000, CRC(f4591e46) SHA1(c17c1a24bf1866fbba388521a4b7ea0975bda587) )

	ROM_REGION( 0x80000, "k007232", 0 )    /* 007232 data */
	ROM_LOAD(      "787-e01.1k",  0x00000, 0x80000, CRC(edc34d01) SHA1(b1465d1a7364a7cebc14b96cd01dc78e57975972) )
ROM_END

ROM_START( citybombj )
	ROM_REGION( 0x1c0000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "787-h10.15k",  0x000000, 0x10000, CRC(66fecf69) SHA1(5881ec019ef6228a693af5c9f6c26e05bdee3846) )
	ROM_LOAD16_BYTE( "787-h09.15h",  0x000001, 0x10000, CRC(a0e29468) SHA1(78971da14a748ade6ea94770080a393c7617b97d) )
	ROM_LOAD16_BYTE( "787-g08.15f",  0x100000, 0x20000, CRC(6242ef35) SHA1(16fd4478d54117bbf09792e22c786622ca5049bb) )
	ROM_LOAD16_BYTE( "787-g07.15d",  0x100001, 0x20000, CRC(21be5e9e) SHA1(37fdf6d6fe3e84e897f2d908afdfc47e8d4a9265) )
	ROM_LOAD16_BYTE( "787-e06.14f",  0x140000, 0x20000, CRC(c251154a) SHA1(7c6a1f862ddf64a604164b85e4a04bb01e2966a7) )
	ROM_LOAD16_BYTE( "787-e05.14d",  0x140001, 0x20000, CRC(0781e22d) SHA1(03a998ee47194960af4dde2bf553359fe8a3aee7) )
	ROM_LOAD16_BYTE( "787-g04.13f",  0x180000, 0x20000, CRC(137cf39f) SHA1(39cfd25c45d824cabc3641fd39eb77c98d32ec9b) )
	ROM_LOAD16_BYTE( "787-g03.13d",  0x180001, 0x20000, CRC(0cc704dc) SHA1(b0c3991393cdb6a75461597d51452bfa08955081) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound */
	ROM_LOAD(      "787-e02.4h",  0x00000, 0x08000, CRC(f4591e46) SHA1(c17c1a24bf1866fbba388521a4b7ea0975bda587) )

	ROM_REGION( 0x80000, "k007232", 0 )    /* 007232 data */
	ROM_LOAD(      "787-e01.1k",  0x00000, 0x80000, CRC(edc34d01) SHA1(b1465d1a7364a7cebc14b96cd01dc78e57975972) )
ROM_END

ROM_START( kittenk )
	ROM_REGION( 0x140000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "712-b10.15k",  0x000000, 0x10000, CRC(8267cb2b) SHA1(63c4ebef834850eff379141b8eb0fafbdcf26d0e) )
	ROM_LOAD16_BYTE( "712-b09.15h",  0x000001, 0x10000, CRC(eb41cfa5) SHA1(d481e63faea098625a42613c13f82fec310a7c62) )
	ROM_LOAD16_BYTE( "712-b08.15f",  0x100000, 0x20000, CRC(e6d71611) SHA1(89fced4074c491c211fea908f08be94595c57f31) )
	ROM_LOAD16_BYTE( "712-b07.15d",  0x100001, 0x20000, CRC(30f75c9f) SHA1(0cbc247ff37800dd3275d2ff23a63ed19ec4cef2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound */
	ROM_LOAD(      "712-e02.4h",  0x00000, 0x08000, CRC(ba76f310) SHA1(cc2164a9617493d1b3b8ac67430f9eb26fd987d2) )

	ROM_REGION( 0x80000, "k007232", 0 )    /* 007232 data */
	ROM_LOAD(      "712-b01.1k",  0x00000, 0x80000, CRC(f65b5d95) SHA1(12701be68629844720cd16af857ce38ef06af61c) )
ROM_END

ROM_START( nyanpani )
	ROM_REGION( 0x140000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "712-j10.15k",  0x000000, 0x10000, CRC(924b27ec) SHA1(019279349b1be45ba46e57ef8f21d79a1b115d7b) )
	ROM_LOAD16_BYTE( "712-j09.15h",  0x000001, 0x10000, CRC(a9862ea1) SHA1(84e481eb6159889d54d0dfe4c31399ab06e13bb7) )
	ROM_LOAD16_BYTE( "712-b08.15f",  0x100000, 0x20000, CRC(e6d71611) SHA1(89fced4074c491c211fea908f08be94595c57f31) )
	ROM_LOAD16_BYTE( "712-b07.15d",  0x100001, 0x20000, CRC(30f75c9f) SHA1(0cbc247ff37800dd3275d2ff23a63ed19ec4cef2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound */
	ROM_LOAD(      "712-e02.4h",  0x00000, 0x08000, CRC(ba76f310) SHA1(cc2164a9617493d1b3b8ac67430f9eb26fd987d2) )

	ROM_REGION( 0x80000, "k007232", 0 )    /* 007232 data */
	ROM_LOAD(      "712-b01.1k",  0x00000, 0x80000, CRC(f65b5d95) SHA1(12701be68629844720cd16af857ce38ef06af61c) )
ROM_END

/*

Hyper Crash
Konami, 1987

PCB Layout
----------

GX790 PWB(B) 250093A
|----------------------------------------------------------------------|
|VOL-L      CN3    CN5     790C01.M10                                  |
|VOL-R      CN4                                                        |
|    MB3722        UPC324          6264   790C02.S9  790C03.T9        |-|
|                                                                     | |
|J      ADC0804    UPC324                                             | |
|A                                                                    | |
|M                     UPC324                                         | |
|M                                 6264   790C05.S7  790C06.T7        | |
|A                         007232                                     | |
|                                                                     |-|
|                                                                      |
|                      YM3012                                          |
|                                3.579545MHz                           |
|                          YM2151                                      |
|                                                                      |
|                                   Z80           68000                |
|                VLM5030                                              |-|
|                     790C08.J4                                       | |
|1                                                                    | |
|8         AN6914                                                     | |
|W         AN6914                                                     | |
|A                                                                    | |
|Y                DSW3(4)                                             | |
|                                  790C09.N2   007593                 |-|
|   CN7           DSW2(8)  DSW1(8)                                     |
|   CN8      CN9                   6116                                |
|----------------------------------------------------------------------|
Notes:
      68000 CPU clock - 6.144MHz [18.432/3]
      Z80 clock     - 3.579545MHz
      YM2151 clock  - 3.579545MHz
      VLM5030 clock - 3.579545MHz
      007232 clock  - 3.579545MHz
      CN3/CN4 - 4 pin plug/jumper for stereo/mono output selection
      CN5 - Right speaker output connection
      CN7 - 4 pin steering connector
      CN8 - 4 pin accelerate/brake connection
      CN9 - 8 pin connection labelled 'ACTION SEAT'
      VSync - 60Hz
      HSync - 15.52kHz
      Konami Custom ICs -
                          007232 (SDIP64)
                          007593 (custom ceramic flat pack with 56 legs)
      ROMs -
             790C02/05 - Fujitsu 27C512 OTP EPROM (DIP28)
             790C03/06 - Fujitsu 27C256 EPROM (DIP28)
             790C01    - Toshiba 571001 (in socket adapter to DIP28 pins on PCB)
                         Actual socket on PCB is wired for 28 pin 1M MaskROM
             790C09    - Fujitsu 27C256 EPROM (DIP28)
             790C08    - Fujitsu 27C256 EPROM (DIP28)
                         Note! PCB is wired for 27C128, top half of EPROM is blank.


GX400PWB(A)200204C
|----------------------------------------------------------------------|
|                                                                      |
|  4416 4416 4416 4416         6264           0005292                  |
|                                                                     |-|
|  4416 4416 4416 4416         6264                                   | |
|                                                                     | |
|                                                                     | |
|                                                                     | |
|                                                                     | |
|                                                                     | |
|                                                                     |-|
|                                6264                                  |
|   0005294   0005290    0005293              0005291   6116  6116     |
|                                                                      |
|                                                                      |
|                                                                      |
|                                                                      |
|                                                                     |-|
|                                                                     | |
|                                                                     | |
|                                                                     | |
|                                                                     | |
|4164 4164 4164 4164                                                  | |
|4164 4164 4164 4164                                                  | |
|                                      6116                           |-|
|4164 4164 4164 4164      0005295                                      |
|4164 4164 4164 4164                                  18.432MHz        |
|----------------------------------------------------------------------|
Notes:
      4416 - 16K x4 DRAM
      4164 - 64K x1 DRAM
      6264 - 8K x8 SRAM
      6116 - 2K x8 SRAM
      Konami Custom ICs -
                         0005290 (SDIP64)
                         0005291 (ZIP64)
                         0005292(SDIP64)
                         0005293 (SDIP64), also stamped 'TC15G014AP-0019'
                         0005294 (ZIP64)
                         0005295 (SDIP64)
*/

ROM_START( hcrash )
	ROM_REGION( 0x140000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "790-d03.t9",   0x00000, 0x08000, CRC(10177dce) SHA1(e46f75e3206eff5299e08e5258e67b68efc4c20c) )
	ROM_LOAD16_BYTE( "790-d06.t7",   0x00001, 0x08000, CRC(fca5ab3e) SHA1(2ad335cf25a86fe38c190e2e0fe101ea161eb81d) )
	ROM_LOAD16_BYTE( "790-c02.s9",   0x40000, 0x10000, CRC(8ae6318f) SHA1(b3205df1103a69eef34c5207e567a27a5fee5660) )
	ROM_LOAD16_BYTE( "790-c05.s7",   0x40001, 0x10000, CRC(c214f77b) SHA1(c5754c3da2a3820d8d06f8ff171be6c2aea92ecc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound */
	ROM_LOAD( "790-c09.n2",   0x00000, 0x8000, CRC(a68a8cce) SHA1(a54966b9cbbe37b2be6a2276ee09c81452d9c0ca) )

	ROM_REGION( 0x08000, "vlm", 0 )  /* VLM5030 data */
	ROM_LOAD( "790-c08.j4",   0x04000, 0x04000, CRC(cfb844bc) SHA1(43b7adb6093e707212204118087ef4f79b0dbc1f) )
	ROM_CONTINUE(             0x00000, 0x04000 ) /* Board is wired for 27C128, top half of EPROM is blank */

	ROM_REGION( 0x20000, "k007232", 0 )  /* 007232 data */
	ROM_LOAD( "790-c01.m10",  0x00000, 0x20000, CRC(07976bc3) SHA1(9341ac6084fbbe17c4e7bbefade9a3f1dec3f132) )
ROM_END

ROM_START( hcrashc )
	ROM_REGION( 0x140000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "790-c03.t9",   0x00000, 0x08000, CRC(d98ec625) SHA1(ddec88b0babd1c538fe5055adec73b537d637d3e) )
	ROM_LOAD16_BYTE( "790-c06.t7",   0x00001, 0x08000, CRC(1d641a86) SHA1(d20ae01565d04db62d5687546c19d87c8e26248c) )
	ROM_LOAD16_BYTE( "790-c02.s9",   0x40000, 0x10000, CRC(8ae6318f) SHA1(b3205df1103a69eef34c5207e567a27a5fee5660) )
	ROM_LOAD16_BYTE( "790-c05.s7",   0x40001, 0x10000, CRC(c214f77b) SHA1(c5754c3da2a3820d8d06f8ff171be6c2aea92ecc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound */
	ROM_LOAD( "790-c09.n2",   0x00000, 0x8000, CRC(a68a8cce) SHA1(a54966b9cbbe37b2be6a2276ee09c81452d9c0ca) )

	ROM_REGION( 0x08000, "vlm", 0 )  /* VLM5030 data */
	ROM_LOAD( "790-c08.j4",   0x04000, 0x04000, CRC(cfb844bc) SHA1(43b7adb6093e707212204118087ef4f79b0dbc1f) )
	ROM_CONTINUE(             0x00000, 0x04000 ) /* Board is wired for 27C128, top half of EPROM is blank */

	ROM_REGION( 0x20000, "k007232", 0 )  /* 007232 data */
	ROM_LOAD( "790-c01.m10",  0x00000, 0x20000, CRC(07976bc3) SHA1(9341ac6084fbbe17c4e7bbefade9a3f1dec3f132) )
ROM_END



GAME(  1985, nemesis,   0,        nemesis,   nemesis,  nemesis_state, empty_init, ROT0,   "Konami",                  "Nemesis (ROM version)",         MACHINE_SUPPORTS_SAVE )
GAME(  1985, nemesisuk, nemesis,  nemesis,   nemesuk,  nemesis_state, empty_init, ROT0,   "Konami",                  "Nemesis (World?, ROM version)", MACHINE_SUPPORTS_SAVE )
GAMEL( 1985, konamigt,  0,        konamigt,  konamigt, nemesis_state, empty_init, ROT0,   "Konami",                  "Konami GT",                     MACHINE_SUPPORTS_SAVE, layout_konamigt )
GAME(  1985, rf2,       konamigt, rf2_gx400, rf2,      nemesis_state, empty_init, ROT0,   "Konami",                  "Konami RF2 - Red Fighter",      MACHINE_SUPPORTS_SAVE )
GAME(  1985, twinbee,   0,        gx400,     twinbee,  nemesis_state, empty_init, ROT90,  "Konami",                  "TwinBee (ROM version)",         MACHINE_SUPPORTS_SAVE )
GAME(  1985, gradius,   nemesis,  gx400,     gradius,  nemesis_state, empty_init, ROT0,   "Konami",                  "Gradius (Japan, ROM version)",  MACHINE_SUPPORTS_SAVE )
GAME(  1985, gwarrior,  0,        gx400,     gwarrior, nemesis_state, empty_init, ROT0,   "Konami",                  "Galactic Warriors",             MACHINE_SUPPORTS_SAVE )
GAME(  1986, salamand,  0,        salamand,  salamand, nemesis_state, empty_init, ROT0,   "Konami",                  "Salamander (version D)",        MACHINE_SUPPORTS_SAVE )
GAME(  1986, salamandj, salamand, salamand,  salamand, nemesis_state, empty_init, ROT0,   "Konami",                  "Salamander (version J)",        MACHINE_SUPPORTS_SAVE )
GAME(  1986, salamandt, salamand, salamand,  salamand, nemesis_state, empty_init, ROT0,   "Konami (Tecfri license)", "Salamander (Tecfri license)",   MACHINE_SUPPORTS_SAVE )
GAME(  1986, lifefrce,  salamand, salamand,  salamand, nemesis_state, empty_init, ROT0,   "Konami",                  "Lifeforce (US)",                MACHINE_SUPPORTS_SAVE )
GAME(  1987, lifefrcej, salamand, salamand,  lifefrcj, nemesis_state, empty_init, ROT0,   "Konami",                  "Lifeforce (Japan)",             MACHINE_SUPPORTS_SAVE )
GAME(  1987, blkpnthr,  0,        blkpnthr,  blkpnthr, nemesis_state, empty_init, ROT0,   "Konami",                  "Black Panther",                 MACHINE_SUPPORTS_SAVE )
GAME(  1987, citybomb,  0,        citybomb,  citybomb, nemesis_state, empty_init, ROT270, "Konami",                  "City Bomber (World)",           MACHINE_SUPPORTS_SAVE )
GAME(  1987, citybombj, citybomb, citybomb,  citybomb, nemesis_state, empty_init, ROT270, "Konami",                  "City Bomber (Japan)",           MACHINE_SUPPORTS_SAVE )
GAME(  1987, hcrash,    0,        hcrash,    hcrash,   nemesis_state, empty_init, ROT0,   "Konami",                  "Hyper Crash (version D)",       MACHINE_SUPPORTS_SAVE )
GAME(  1987, hcrashc,   hcrash,   hcrash,    hcrash,   nemesis_state, empty_init, ROT0,   "Konami",                  "Hyper Crash (version C)",       MACHINE_SUPPORTS_SAVE )
GAME(  1988, kittenk,   0,        nyanpani,  nyanpani, nemesis_state, empty_init, ROT0,   "Konami",                  "Kitten Kaboodle",               MACHINE_SUPPORTS_SAVE )
GAME(  1988, nyanpani,  kittenk,  nyanpani,  nyanpani, nemesis_state, empty_init, ROT0,   "Konami",                  "Nyan Nyan Panic (Japan)",       MACHINE_SUPPORTS_SAVE )

/*

Konami Bubble System
Konami, 1985

A 68000/Z80-based arcade system PCB with an additional Bubble Memory Cassette
containing the game data which can be changed easily. The data in the Bubble
Cassette can be corrupted if subjected to magnetic interference.
The bottom PCB appears to be exactly the same as used on Nemesis hardware.

The boot sequence is....
On power up, displays a blue screen containing some junk pixels and speech
says....
"Presented By Konami"
then...
"Getting Ready..... Fifty"
"Getting Ready..... Forty Nine"
etc, until about 30, then the screen displays some orange text 'WARMING UP NOW' on
a black background and numbers that count down from 99 to 0, and below that text
'PRESENTED BY KONAMI". A tune also plays while the numbers count down.
When the counter reaches 0 the game boots.


The harness pinout matches Scramble with 3 additional wires....
-12V = A15 (pin 15 solder side)
1P Fire 3 = A12 (pin 12 solder side)
2P Fire 3 = B15 (pin 15 parts side)


PCB Layouts
-----------

Top PCB

GX400PWB(B)200207E                            Bubble Memory Cassette (above PCB)
|----------------------------------------------------------||----------------------|
|    400A1.2B  4066 UPC324 MB3761      |-------------------||--------------------| |
| 400A2.1B         AN6914 VOL          |                   ||   2128             ||-|
|                                      |                   \/                    || |
|            AY3-8910   LA4460         |          68000         2128             || |
|            AY3-8910                  |0005297                                  || |
|-|                                    |                                         || |
  |                                    |                                         || |
|-|  0005289                           |                                         ||-|
|                                      |                                         | |
|               14.31818MHz            |                                         | |
|1                                     |                                         | |
|8         DSW3                        |                                         | |
|W              3.579545MHz            |                        4416 4416 4416 4416|
|A         DSW2                        |                                         | |
|Y                                     |                        4416 4416 4416 4416|
|          DSW1         400B03.8G      |                                         | |
|-|                           Z80A   *2|                                         ||-|
  |        VLM5030                  |-||                                         || |
|-|                                 | ||                                         || |
|                                   | ||                                         || |
|          4416                2128 |12MHz                                       || |
|                                   | ||                         2128            || |
|          4416                     |-||                                         ||-|
|                          *1          |-------------------------2128------------| |
|--------------------|------------|------------------------------------------------|
                     |6264    6264|
                     |------------|
Notes:
      *1 - Small plug-in PCB containing two 8kx8 SRAMs. PCB number GX456 PWB(C)400327
      *2 - Bubble Cassette connector
      68000 - clock 9.216MHz [18.432/2]
      Z80A - clock 3.579545MHz
      VLM5030 - clock 1.7897725MHz [3.579545/2]
      AY3-8910 - clock 3.579545MHz
      400A1.2B / 400A2.1B - Texas Instruments TBP24S10 Bipolar PROMs
                            Connected to 0005289 (wavetable data)
      400B03.8G - 2764 EPROM
      2128 - 2kx8 SRAM
      6264 - 8kx8 SRAM
      4416 - 16kx4 DRAM
      VSync - 60Hz
      HSync - 15.52kHz

      Custom Chips - 0005289 (DIP42, wavetable sound chip), 0005297 (SDIP64, ULA)


Bottom PCB

GX400PWB(A)200204C
|----------------------------------------------------------------------------------|
|                                                                                  |
|                                      6264                                       |-|
|                                                   0005292                       | |
|                                                                                 | |
|                                      6264                                       | |
|                                                                                 | |
|                                                                                 | |
|                                                                                 |-|
|                                                                                  |
|                                                                  2128   2128     |
|                                                                                  |
|              0005290                 6264              0005291                   |
|  0005294                   0005293                                               |
|                                                                                  |
|                                                                                  |
|                                                                                  |
|                                                                                 |-|
|                                                                                 | |
|                                                                                 | |
|                                                                                 | |
|4164 4164 4164 4164                             2128                             | |
|4164 4164 4164 4164                                                              | |
|4164 4164 4164 4164                                                              |-|
|4164 4164 4164 4164         0005295                            18.432MHz          |
|----------------------------------------------------------------------------------|
Notes:
      4164 - 64kx1 DRAM
      2128 - 2kx8 SRAM
      6264 - 8kx8 SRAM

      Konami custom chips -
      0005290 - SDIP64 package
      0005291 - 64-pin Quad-In-Line (Spider-legs) package (possibly manufactured by Rockwell?)
      0005292 - SDIP64 package
      0005293 - SDIP64 package (manufactured by Toshiba, marked TC15G0144AP)
      0005294 - 64-pin Quad-In-Line (Spider-legs) package (possibly manufactured by Rockwell?)
      0005295 - SDIP64 package


Bubble Cassette
---------------
The bubble cassette PCB is housed in a metal box. The PCB
is about half the size of the box.

     |-------------------------|
     |C271C   MB3908   MB3908  |
     |C271C   MB3908   MB3908  |
     |74LS03                   |
     |     |-------| |-------| |
     |     |   F   | |   F   | |
     |     |       | |       | |
     |C2501|       | |       | |
     |     |-------| |-------| |
     |      RE65G      RE65G   |
 |---|      25Ohms     25Ohms  |
 |   |                         |
 |   |MB466 MB466   MB466 MB466|
 | *1|      MB3910     MB3910  |
 |   |            74LS32       |
 |---|        12000kHz MB14506 |
     |-----|------------|------|
           |-----*2-----|
Notes:
      All IC's shown
      F - Fujitsu bubble memory. No part number given. Memory size unknown.
          One stamped '4612125, with sticker 'KN - #01'
          the other is stamped '3801105, with sticker 'KN - #01'
          DIP24 package. Both have Fujitsu manufacturer symbol
     *1 - Small plug-in PCB containing capacitors and resistors.
     *2 - Connector joining to main PCB


Controls
--------
2x 8-way joystick with 3 buttons each player


DIPs
----

DSW1
Default = *
                1   2   3   4
|-------------|---|---|---|---|
|COIN1  1C 1P*|OFF|OFF|OFF|OFF|
|       1C 2P |ON |OFF|OFF|OFF|
|       1C 3P |OFF|ON |OFF|OFF|
|       1C 4P |ON |ON |OFF|OFF|
|       1C 5P |OFF|OFF|ON |OFF|
|       1C 6P |ON |OFF|ON |OFF|
|       1C 7P |OFF|ON |ON |OFF|
|       2C 1P |ON |ON |ON |OFF|
|       2C 3P |OFF|OFF|OFF|ON |
|       2C 5P |ON |OFF|OFF|ON |
|       3C 1P |OFF|ON |OFF|ON |
|       3C 2P |ON |ON |OFF|ON |
|       3C 4P |OFF|OFF|ON |ON |
|       4C 1P |ON |OFF|ON |ON |
|       4C 3P |OFF|ON |ON |ON |
|    Freeplay |ON |ON |ON |ON |
|-------------|---|---|---|---|

                5   6   7   8
|-------------|---|---|---|---|
|COIN2  1C 1P*|OFF|OFF|OFF|OFF|
|       1C 2P |ON |OFF|OFF|OFF|
|       1C 3P |OFF|ON |OFF|OFF|
|       1C 4P |ON |ON |OFF|OFF|
|       1C 5P |OFF|OFF|ON |OFF|
|       1C 6P |ON |OFF|ON |OFF|
|       1C 7P |OFF|ON |ON |OFF|
|       2C 1P |ON |ON |ON |OFF|
|       2C 3P |OFF|OFF|OFF|ON |
|       2C 5P |ON |OFF|OFF|ON |
|       3C 1P |OFF|ON |OFF|ON |
|       3C 2P |ON |ON |OFF|ON |
|       3C 4P |OFF|OFF|ON |ON |
|       4C 1P |ON |OFF|ON |ON |
|       4C 3P |OFF|ON |ON |ON |
|  Invalidity |ON |ON |ON |ON |
|-------------|---|---|---|---|

DSW2
Default = *
             1   2   3   4   5   6   7   8
|----------|---|---|---|---|---|---|---|---|
|LIVES   3*|OFF|OFF|   |   |   |   |   |   |
|        4 |ON |OFF|   |   |   |   |   |   |
|        5 |OFF|ON |   |   |   |   |   |   |
|        7 |ON |ON |   |   |   |   |   |   |
|----------|---|---|---|---|---|---|---|---|
|CABINET   UPRIGHT*|OFF|   |   |   |   |   |
|            TABLE |ON |   |   |   |   |   |
|------------------|---|---|---|---|---|---|
|BONUS 1ST/2ND         |   |   |   |   |   |
|          20000/70000 |OFF|OFF|   |   |   |
|          30000/80000*|ON |OFF|   |   |   |
|          20000/NONE  |OFF|ON |   |   |   |
|          30000/NONE  |ON |ON |   |   |   |
|----------------------|---|---|---|---|---|
|DIFFICULTY               EASY |OFF|OFF|   |
|                       NORMAL*|ON |OFF|   |
|                    DIFFICULT |OFF|ON |   |
|                  V.DIFFICULT |ON |ON |   |
|------------------------------|---|---|---|
|DEMO SOUND                        OFF |OFF|
|                                  ON* |ON |
|--------------------------------------|---|

DSW3
Default = *
             1   2   3
|----------|---|---|---|
|SCREEN    |   |   |   |
|   NORMAL*|OFF|   |   |
|     FLIP |ON |   |   |
|----------|---|---|---|
|CONTROLS          |   |
|   SINGLE UPRIGHT*|OFF|
|     DUAL UPRIGHT |ON |
|------------------|---|
|MODE         GAME*|OFF|
|             TEST |ON |
|------------------|---|
Manual says SW4, 5, 6, 7 & 8 not used, leave off

Interrupt source info from ArcadeHacker:
74LS147 @ 17E
PIN1 INPUT 4 -> 14H 74LS74 PIN 5
PIN2 INPUT 5 -> MCU PIN  31
PIN3 INPUT 6 -> VCC
PIN4 INPUT 7 -> VCC
PIN5 INPUT 8 -> VCC
PIN6 OUTPUT C -> 68K IPL2
PIN7 OUTPUT B -> 68K IPL1
PIN8 GND
PIN9 OUTPUT A -> 68K IPL0
PIN10 INPUT 9 -> VCC
PIN11 INPUT 1 -> 18E 74LS74 PIN 5
PIN12 INPUT 2 -> 18E 74LS74 PIN 9
PIN13 INPUT 3 -> VCC
PIN14 OUTPUT D -> N.C.
PIN15 N.C.
PIN16 VCC

*/

void nemesis_state::bubsys(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 18432000/2); /* 9.216MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &nemesis_state::bubsys_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(nemesis_state::bubsys_interrupt), "screen", 0, 1);

	Z80(config, m_audiocpu, 14318180/8); /* 1.7897725MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &nemesis_state::gx400_sound_map);

	ls259_device &outlatch(LS259(config, "outlatch"));
	outlatch.q_out_cb<0>().set(FUNC(nemesis_state::coin1_lockout_w));
	outlatch.q_out_cb<1>().set(FUNC(nemesis_state::coin2_lockout_w));
	outlatch.q_out_cb<2>().set(FUNC(nemesis_state::sound_irq_w));
	outlatch.q_out_cb<4>().set(FUNC(nemesis_state::sound_nmi_w));
	outlatch.q_out_cb<7>().set(FUNC(nemesis_state::irq4_enable_w));

	ls259_device &intlatch(LS259(config, "intlatch"));
	intlatch.q_out_cb<0>().set(FUNC(nemesis_state::irq2_enable_w));
	intlatch.q_out_cb<1>().set(FUNC(nemesis_state::irq1_enable_w));
	intlatch.q_out_cb<2>().set(FUNC(nemesis_state::gfx_flipx_w));
	intlatch.q_out_cb<3>().set(FUNC(nemesis_state::gfx_flipy_w));

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(nemesis_state::screen_update_nemesis));
	m_screen->set_palette(m_palette);
	// TODO: This is supposed to be gated by something on bubble system, unclear what.
	// it should only be active while the bubble memory is warming up, and disabled after
	// the bubble mcu 'releases' the 68k from reset.
	//m_screen->screen_vblank().set_inputline("audiocpu", INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_nemesis);
	PALETTE(config, m_palette).set_entries(2048);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ay8910_device &ay1(AY8910(config, "ay1", 14318180/8));
	ay1.set_flags(AY8910_LEGACY_OUTPUT | AY8910_SINGLE_OUTPUT);
	ay1.port_a_read_callback().set(FUNC(nemesis_state::nemesis_portA_r));
	ay1.add_route(ALL_OUTPUTS, "filter1", 0.20);

	ay8910_device &ay2(AY8910(config, "ay2", 14318180/8));
	ay2.port_a_write_callback().set(m_k005289, FUNC(k005289_device::control_A_w));
	ay2.port_b_write_callback().set(m_k005289, FUNC(k005289_device::control_B_w));
	ay2.add_route(0, "filter2", 1.00);
	ay2.add_route(1, "filter3", 1.00);
	ay2.add_route(2, "filter4", 1.00);

	FILTER_RC(config, m_filter1);
	m_filter1->add_route(ALL_OUTPUTS, "mono", 1.0);
	FILTER_RC(config, m_filter2);
	m_filter2->add_route(ALL_OUTPUTS, "mono", 1.0);
	FILTER_RC(config, m_filter3);
	m_filter3->add_route(ALL_OUTPUTS, "mono", 1.0);
	FILTER_RC(config, m_filter4);
	m_filter4->add_route(ALL_OUTPUTS, "mono", 1.0);

	K005289(config, m_k005289, 3579545);
	m_k005289->add_route(ALL_OUTPUTS, "mono", 0.35);

	VLM5030(config, m_vlm, 3579545);
	m_vlm->set_addrmap(0, &nemesis_state::gx400_vlm_map);
	m_vlm->add_route(ALL_OUTPUTS, "mono", 0.70);
}



ROM_START( bubsys )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD( "boot.bin", 0x0000, 0x1e0, CRC(f0774fc2) SHA1(84fade54e025f170d983200a86c1ed96ef1a9ed3) )

	ROM_REGION( 0x49000, "bubblememory", ROMREGION_ERASE00 )

	ROM_REGION( 0x1000, "mcu", ROMREGION_ERASE00 ) /* Fujitsu MCU, unknown type */
	ROM_LOAD( "mcu", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound */
	ROM_LOAD( "400b03.8g",   0x00000, 0x2000, CRC(85c2afc5) SHA1(387842d02d50d0d78a27270e7267af19555b9e63) )

	ROM_REGION( 0x0200,  "k005289", 0 )      /* 2x 256 byte for 0005289 wavetable data */
	ROM_LOAD( "400a1.2b", 0x000, 0x100, CRC(5827b1e8) SHA1(fa8cf5f868cfb08bce203baaebb6c4055ee2a000) )
	ROM_LOAD( "400a2.1b", 0x100, 0x100, CRC(2f44f970) SHA1(7ab46f9d5d587665782cefc623b8de0124a6d38a) )
ROM_END

ROM_START( gradiusb )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD( "boot.bin", 0x0000, 0x1e0, CRC(f0774fc2) SHA1(84fade54e025f170d983200a86c1ed96ef1a9ed3) )

	ROM_REGION( 0x48360, "bubblememory", 0 )
	/* The Gradius cartridge contains 0x807 pages of 130 bytes each */
	ROM_LOAD16_WORD_SWAP( "gradius.bin", 0x000, 0x48360, CRC(f83b9607) SHA1(53493c2d5b0e66dd6b75865abf0982ee50c01a6f) )

	ROM_REGION( 0x1000, "mcu", ROMREGION_ERASE00 ) /* Fujitsu MCU, unknown type */
	ROM_LOAD( "mcu", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound */
	ROM_LOAD( "400b03.8g",   0x00000, 0x2000, CRC(85c2afc5) SHA1(387842d02d50d0d78a27270e7267af19555b9e63) )

	ROM_REGION( 0x0200,  "k005289", 0 )      /* 2x 256 byte for 0005289 wavetable data */
	ROM_LOAD( "400a1.2b", 0x000, 0x100, CRC(5827b1e8) SHA1(fa8cf5f868cfb08bce203baaebb6c4055ee2a000) )
	ROM_LOAD( "400a2.1b", 0x100, 0x100, CRC(2f44f970) SHA1(7ab46f9d5d587665782cefc623b8de0124a6d38a) )
ROM_END

ROM_START( twinbeeb )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD( "boot.bin", 0x000, 0x1e0, CRC(ee6e93d7) SHA1(7302c08a726a760f59d6837be8fd10bbd1f79da0) )

	ROM_REGION( 0x806*0x90, "bubblememory", ROMREGION_ERASE00 )
//  ROM_LOAD16_WORD_SWAP( "bubble_twinbeeb", 0x000, 0x48360, CRC(21599cf5) SHA1(7eb068e10134d5c66f7f90f6d6b265353b7bd8be) ) // re-encoded data

	ROM_REGION( 0x806*0x80, "bubblememory_temp", 0 )
	ROM_LOAD( "twinbee.bin", 0x00000, 0x40300, CRC(4d396a0a) SHA1(ee922a1bd7062c0fcf358f5079cca6424aadc975) )

	ROM_REGION( 0x1000, "mcu", ROMREGION_ERASE00 ) // Fujitsu MCU
	ROM_LOAD( "mcu", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "400-e03.5l",   0x00000, 0x02000, CRC(a5a8e57d) SHA1(f4236770093392dec3f76835a5766e9b3ed64e2e) )

	ROM_REGION( 0x0200,  "k005289", 0 )
	ROM_LOAD( "400-a01.fse",  0x00000, 0x0100, CRC(5827b1e8) SHA1(fa8cf5f868cfb08bce203baaebb6c4055ee2a000) )
	ROM_LOAD( "400-a02.fse",  0x00100, 0x0100, CRC(2f44f970) SHA1(7ab46f9d5d587665782cefc623b8de0124a6d38a) )
ROM_END

void nemesis_state::bubsys_init()
{
	/*
	    The MCU is the master of the system and controls the /RESET and /BS lines of the 68000.
	    At boot the MCU asserts /RESET and /BS of the 68000 and waits for the bubble memory to warm up.
	    During this period, the Audio CPU is running and speaking the "Getting ready... Fifty..."
	    countdown via the vlm5030. Once the bubble memory is ready, the MCU copies the 68000 boot program
	    to shared RAM which takes 30.65 milliseconds then releases /RESET and /BS so the 68000 starts execution.

	    As the MCU is not dumped we effectively start the simulation at the point the 68000
	    is released, and manually copy the boot program to 68000 address space.

	    TODO: add a 'delay' (configurable) to simulate the bubble memory 'warming up' and only release the 68k after this is done.
	*/

	const uint8_t *src = memregion("maincpu")->base();
	memcpy(m_bubsys_shared_ram, src, 0x1e0);

	/*
	    The MCU sets this flag once the boot program is copied.  The 68000 will reset
	    if the value is not correct. Presumably this was done for safety in case somehow the
	    68000 was released from reset when the MCU wasn't yet ready.
	*/
	m_bubsys_control_ram[3]=0x240;
}


void nemesis_state::bubsys_twinbeeb_init()
{
	// the twinbee bubble data is in a stripped down, predecoded state already, why?
	// this reencodes it to something the loading code can actually use

	uint8_t *src = memregion("bubblememory_temp")->base();
	uint8_t *dst = memregion("bubblememory")->base();

	for (int i = 0; i < 0x806; i++)
	{
		[[maybe_unused]] uint16_t crc = 0;

		int sourcebase = i * 0x80;
		int destbase = i * 0x90;

		for (int j = 0; j < 0x80; j++)
		{
			uint8_t dat = src[sourcebase + j];
			dst[destbase + j + 0] |= (dat >> 6) & 0x03;
			dst[destbase + j + 1] |= (dat << 2) & 0xfc;

			crc += dat;
		}

		for (int j = 0; j < 0x82; j += 2)
		{
			uint8_t temp1 = dst[destbase + j + 0];
			dst[destbase + j + 0] = dst[destbase + j + 1];
			dst[destbase + j + 1] = temp1;
		}

		dst[destbase+0x83] = i >> 8;
		dst[destbase+0x82] = i & 0xff;
	}

	bubsys_init();
}

GAME( 1985, bubsys,   0,         bubsys,    bubsys,   nemesis_state, bubsys_init, ROT0,   "Konami", "Bubble System BIOS", MACHINE_IS_BIOS_ROOT )
GAME( 1985, gradiusb, bubsys,    bubsys,    gradiusb, nemesis_state, bubsys_init, ROT0,   "Konami", "Gradius (Bubble System)", MACHINE_UNEMULATED_PROTECTION )
GAME( 1985, twinbeeb, bubsys,    bubsys,    twinbeeb, nemesis_state, bubsys_twinbeeb_init, ROT90,   "Konami", "TwinBee (Bubble System)", MACHINE_UNEMULATED_PROTECTION )
// Bubble System RF2
// Bubble System Galactic Warriors
// Bubble System Attack Rush
