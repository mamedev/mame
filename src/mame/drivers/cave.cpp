// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                              -= Cave Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)


Main  CPU    :  MC68000

Sound CPU    :  Z80 [Optional]

Sound Chips  :  YMZ280B or
                OKIM6295 x (1|2) + YM2203 / YM2151 [Optional]

Other        :  93C46 EEPROM


-----------------------------------------------------------------------------------------
Year + Game               License       PCB         Tilemaps        Sprites         Other
-----------------------------------------------------------------------------------------
94 Mazinger Z             Banpresto     BP943A      038 9335EX706   013 9341E7009   Z80
94 Power Instinct 2       Atlus         ATG02?      038 9429WX709   013 9341E7009   Z80 NMK 112
95 Gogetsuji Legends      Atlus         AT047G2-B   038 9429WX709   013 9341E7009   Z80 NMK 112
95 Metamoqester           Banpresto     BP947A      038 9437WX711   013 9346E7002   Z80
95 Sailor Moon            Banpresto     BP945A      038 9437WX711   013 9346E7002   Z80
95 Donpachi               Atlus         AT-C01DP-2  038 9429WX727   013 9347E7003   NMK 112
96 Air Gallet             Banpresto     BP962A      038 9437WX711   013 9346E7002   Z80
96 Hotdog Storm           Marble        ASCT9501    038 9341EX702   013             Z80
96 Pac-Slot               Namco         N-44 EM     038 9444WX010   013 9345E7006
96 Poka Poka Satan        Kato's        PPS-MAIN    038 9444WX010   013 9607EX013
97 Tekken Card World      Namco         EMG4        038 9701WX001   013 9651EX001
97 Dodonpachi             Atlus         AT-C03 D2   038 9341E7010   013 9338EX701
98 Dangun Feveron         Nihon System  CV01        038 9808WX003   013 9807EX004
98 ESP Ra.De.             Atlus         ATC04       038 9841WX002   013 9838EX002
98 Tekken Battle Scratch  Namco         EMG4        038 9748WX001   013
98 Uo Poko                Jaleco        CV02        038 9749WX001   013 9749EX004
99 Guwange                Atlus         ATC05       038 9919WX004   013
99 Gaia Crusaders         Noise Factory ?           038 9838WX003   013 9918EX008
99 Koro Koro Quest        Takumi        TUG-01B     038 9838WX004   013 9838EX004
99 Crusher Makochan       Takumi        TUG-01B     038 9838WX004   013 9838EX004
99 Tobikose! Jumpman      Namco         EMG4        038 9919WX007   013 9934WX002
01 Thunder Heroes         Primetek      ?           038 9838WX003   013 9918EX008
-----------------------------------------------------------------------------------------

To Do:

- Sprite lag in some games (e.g. metmqstr). The sprites chip probably
  generates interrupts (unknown_irq)

- Max sprite number is possibly less than 1024
  (ex: Boss explosion scene at most of cave shmups on real hardware)

- Tilemap scrolling issue in ppsatan right screen when flipped left
  screen at some scenes

- Most of videoreg functions aren't implemented

- Measure video timings

Stephh's notes (based on the games M68000 code and some tests) :

1) 'gaia'

  - Difficulty Dip Switch also affects "Bonus Life" Dip Switch


2) 'theroes'

  - This is a English/Chinese version, but from the manual, there might exist a English/Japanese one
  - Difficulty Dip Switch also affects "Bonus Life" Dip Switch
  - There are less degrees of difficulty in this version
  - DSW2 bit 5 effect remains unknown :
      * it is checked at address 0x008d16 at the beginning of each sub-level
      * it is checked at address 0x00c382 when you quickly push the joystick left or right twice
    Any info is welcome !

Versions known to exist but not dumped:
  Dodonpachi Campaign Version
     Reportedly only 3 ever made, one was given out as a prize to a high score contest winner.  The other two
     PCBs were shown running (and could be played) at a Cave fan show known as Cave Festival 2006. There are
     videos of the game being played floating around the internet and on YouTube. AKA DDP-CV or DDP BLUE ROM

***************************************************************************/

#include "emu.h"
#include "includes/cave.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/nmk112.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"
#include "sound/ymopm.h"
#include "sound/ymopn.h"
#include "sound/ymz280b.h"
#include "speaker.h"
#include <algorithm>

#include "ppsatan.lh"


/***************************************************************************


                        Interrupt Handling Routines


***************************************************************************/


/* Update the IRQ state based on all possible causes */
void cave_state::update_irq_state()
{
	if (m_vblank_irq || m_sound_irq || m_unknown_irq)
		m_maincpu->set_input_line(m_irq_level, ASSERT_LINE);
	else
		m_maincpu->set_input_line(m_irq_level, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(cave_state::vblank_end)
{
	if (m_kludge == 3)  /* mazinger metmqstr */
	{
		m_unknown_irq = 1;
		update_irq_state();
	}
	m_agallet_vblank_irq = 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(cave_state::vblank_start)
{
	m_vblank_irq = 1;
	update_irq_state();
	get_sprite_info(0);
	m_agallet_vblank_irq = 1;
	m_vblank_end_timer->adjust(attotime::from_usec(2000)); // 2000us : vblank time
}
TIMER_DEVICE_CALLBACK_MEMBER(cave_state::vblank_start_left)
{
	get_sprite_info(1);
}
TIMER_DEVICE_CALLBACK_MEMBER(cave_state::vblank_start_right)
{
	get_sprite_info(2);
}

/* Called once/frame to generate the VBLANK interrupt */
INTERRUPT_GEN_MEMBER(cave_state::interrupt)
{
	// 17376us : frame time
	m_int_timer->adjust(attotime::from_usec(17376 - m_time_vblank_irq));
}
INTERRUPT_GEN_MEMBER(cave_state::interrupt_ppsatan)
{
	m_int_timer->adjust      (attotime::from_usec(17376 - m_time_vblank_irq));
	m_int_timer_left->adjust (attotime::from_usec(17376 - m_time_vblank_irq));
	m_int_timer_right->adjust(attotime::from_usec(17376 - m_time_vblank_irq));
}

/* Called by the YMZ280B to set the IRQ state */
WRITE_LINE_MEMBER(cave_state::sound_irq_gen)
{
	m_sound_irq = (state != 0);
	update_irq_state();
}


/*  Level 1 irq routines:

    Game        |first read | bit==0->routine + |
                |offset:    | read this offset  |

    ddonpach    4,0         0 -> vblank + 4     1 -> rte    2 -> like 0     read sound
    dfeveron    0           0 -> vblank + 4     1 -> + 6    -               read sound
    uopoko      0           0 -> vblank + 4     1 -> + 6    -               read sound
    esprade     0           0 -> vblank + 4     1 -> rte    2 must be 0     read sound
    guwange     0           0 -> vblank + 6,4   1 -> + 6,4  2 must be 0     read sound
    mazinger    0           0 -> vblank + 4     rest -> scroll + 6
*/


/* Reads the cause of the interrupt and clears the state */

u16 cave_state::irq_cause_r(offs_t offset)
{
	u16 result = 0x0003;

	if (m_vblank_irq)
		result ^= 0x01;
	if (m_unknown_irq)
		result ^= 0x02;

	if (!machine().side_effects_disabled())
	{
		if (offset == 4/2)
			m_vblank_irq = 0;
		if (offset == 6/2)
			m_unknown_irq = 0;

		update_irq_state();
	}

/*
    sailormn and agallet wait for bit 2 of $b80001 to go 1 -> 0.
    It must happen once per frame as agallet uses this to show
    the copyright notice screen for ~8.5s
*/
	if (offset == 0)
	{
		result &= ~4;
		result |= (m_agallet_vblank_irq ? 0 : 4);
	}

	return result;
}

template<int Chip>
void cave_state::videoregs_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_videoregs[Chip][offset]);
	offset <<= 1;
	// offset 0x04 and 0x06 is offset related?
	// offset 0x0a is position mode toggle in bit 13-12 (separated for X and Y?), other bits used but unknown
	// offset 0x68 or 0x78 is commonly watchdog or DMA command?
	// offset 0x6c or 0x7c is encryption key or CRTC or something else?
	// offset 0x6e is commonly communication when sound CPU exists
	// other registers unknown
	if (offset != 0x00 &&
		offset != 0x02 &&
		offset != 0x04 &&
		offset != 0x06 &&
		offset != 0x08 &&
		offset != 0x0a &&
		offset != 0x68 &&
		offset != 0x6e &&
		offset != 0x78)
		logerror("%s: Unknown videoregs #%02X written %04X = %04X & %04X\n",
			machine().describe_context(), Chip, offset, data, mem_mask);
}


/***************************************************************************


                            Sound Handling Routines


***************************************************************************/

/*  We need a FIFO buffer for sailormn, where the inter-CPUs
    communication is *really* tight */

u8 cave_state::soundflags_r()
{
	// bit 2 is low: can read command (lo)
	// bit 3 is low: can read command (hi)
//  return  (m_sound_flag[0] ? 0 : 4) |
//          (m_sound_flag[1] ? 0 : 8) ;
return 0;
}

u16 cave_state::soundflags_ack_r()
{
	// bit 0 is low: can write command
	// bit 1 is low: can read answer
//  return  ((m_sound_flag[0] | m_sound_flag[1]) ? 1 : 0) |
//          (m_soundbuf_empty ? 0 : 2) ;

	return m_soundbuf_empty ? 2 : 0;
}

/* Main CPU: write a 16 bit sound latch and generate a NMI on the sound CPU */
void cave_state::sound_cmd_w(u16 data)
{
//  m_sound_flag[0] = 1;
//  m_sound_flag[1] = 1;
	m_soundlatch->write(data);
	m_maincpu->spin_until_time(attotime::from_usec(50));  // Allow the other cpu to reply
}

/* Sound CPU: read the low 8 bits of the 16 bit sound latch */
u8 cave_state::soundlatch_lo_r()
{
//  m_sound_flag[0] = 0;
	return m_soundlatch->read() & 0xff;
}

/* Sound CPU: read the high 8 bits of the 16 bit sound latch */
u8 cave_state::soundlatch_hi_r()
{
//  m_sound_flag[1] = 0;
	return m_soundlatch->read() >> 8;
}

/* Main CPU: read the latch written by the sound CPU (acknowledge) */
u16 cave_state::soundlatch_ack_r()
{
	if (!m_soundbuf_empty)
	{
		const u8 data = m_soundbuf_data[m_soundbuf_rptr];
		if (!machine().side_effects_disabled())
		{
			m_soundbuf_rptr = (m_soundbuf_rptr + 1) & 0x1f;
			m_soundbuf_empty = m_soundbuf_rptr == m_soundbuf_wptr;
		}
		return data;
	}
	else
	{
		logerror("%s: Sound Buffer 2 Underflow Error\n", machine().describe_context());
		return 0xff;
	}
}


/* Sound CPU: write latch for the main CPU (acknowledge) */
void cave_state::soundlatch_ack_w(u8 data)
{
	if (m_soundbuf_empty || (m_soundbuf_wptr != m_soundbuf_rptr))
	{
		m_soundbuf_data[m_soundbuf_wptr] = data;
		m_soundbuf_wptr = (m_soundbuf_wptr + 1) & 0x1f;
		m_soundbuf_empty = false;
	}
	else
		logerror("%s: Sound Buffer 2 Overflow Error\n", machine().describe_context());
}


/***************************************************************************


                                    EEPROM


***************************************************************************/

void cave_state::eeprom_w(u8 data)
{
	if (data & ~0xfe)
		logerror("%s: Unknown EEPROM bit written %04X\n", machine().describe_context(), data);

	machine().bookkeeping().coin_lockout_w(1, BIT(~data, 7));
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 6));
	machine().bookkeeping().coin_counter_w(1, BIT( data, 5));
	machine().bookkeeping().coin_counter_w(0, BIT( data, 4));

	// latch the bit
	m_eeprom->di_write(BIT(data, 3));

	// reset line asserted: reset.
	m_eeprom->cs_write(BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE);

	// clock line asserted: write latch or select next bit to read
	m_eeprom->clk_write(BIT(data, 2) ? ASSERT_LINE : CLEAR_LINE);
}

void cave_state::sailormn_eeprom_w(u8 data)
{
	sailormn_tilebank_w(BIT(data, 0));
	eeprom_w(data & ~0x01);
}

void cave_state::hotdogst_eeprom_w(u8 data)
{
	// latch the bit
	m_eeprom->di_write(BIT(data, 3));

	// reset line asserted: reset.
	m_eeprom->cs_write(BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE);

	// clock line asserted: write latch or select next bit to read
	m_eeprom->clk_write(BIT(data, 2) ? ASSERT_LINE : CLEAR_LINE);
}

void cave_state::ppsatan_eeprom_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (data & ~0x000f)
		logerror("%s: Unknown EEPROM bit written %04X\n",machine().describe_context(),data);

	if (ACCESSING_BITS_0_7)  // odd address
	{
		// bit 11?

		// latch the bit
		m_eeprom->di_write((data & 0x0400) >> 10);

		// reset line asserted: reset.
		m_eeprom->cs_write((data & 0x0100) ? CLEAR_LINE : ASSERT_LINE);

		// clock line asserted: write latch or select next bit to read
		m_eeprom->clk_write((data & 0x0200) ? ASSERT_LINE : CLEAR_LINE);
	}
}


void cave_state::guwange_eeprom_w(u8 data)
{
	if (data & ~0xef)
		logerror("%s: Unknown EEPROM bit written %04X\n",machine().describe_context(),data);

	machine().bookkeeping().coin_lockout_w(1, BIT(~data, 3));
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 2));
	machine().bookkeeping().coin_counter_w(1, BIT( data, 1));
	machine().bookkeeping().coin_counter_w(0, BIT( data, 0));

	// latch the bit
	m_eeprom->di_write(BIT(data, 7));

	// reset line asserted: reset.
	m_eeprom->cs_write(BIT(data, 5) ? ASSERT_LINE : CLEAR_LINE);

	// clock line asserted: write latch or select next bit to read
	m_eeprom->clk_write(BIT(data, 6) ? ASSERT_LINE : CLEAR_LINE);
}

/*  - No eeprom or lockouts */
void cave_state::gaia_coin_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
}

/*  - No coin lockouts
    - Writing 0xcf shouldn't send a 1 bit to the EEPROM   */
void cave_state::metmqstr_eeprom_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(1, BIT(data, 5));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 4));

	if (BIT(~data, 0))
	{
		// latch the bit
		m_eeprom->di_write(BIT(data, 3));

		// reset line asserted: reset.
		m_eeprom->cs_write(BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE);

		// clock line asserted: write latch or select next bit to read
		m_eeprom->clk_write(BIT(data, 2) ? ASSERT_LINE : CLEAR_LINE);
	}
}

/***************************************************************************


                            Memory Maps - Main CPU


***************************************************************************/

/*  Lines starting with an empty comment in the following MemoryReadAddress
     arrays are there for debug (e.g. the game does not read from those ranges
    AFAIK)  */

/***************************************************************************
                                Dangun Feveron
***************************************************************************/

void cave_state::dfeveron_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                                                // ROM
	map(0x100000, 0x10ffff).ram();                                                                                // RAM
	map(0x300000, 0x300003).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask16(0x00ff);   // YMZ280
	map(0x400000, 0x40ffff).ram().share("spriteram.0");                                                           // Sprites
	map(0x500000, 0x507fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_map));                                   // Layer 0
	map(0x600000, 0x607fff).m(m_tilemap[1], FUNC(tilemap038_device::vram_map));                                   // Layer 1
	map(0x708000, 0x708fff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0");              // Palette
	map(0x710000, 0x710bff).readonly();                                                                           // ?
	map(0x710c00, 0x710fff).ram();                                                                                // ?
	map(0x800000, 0x80007f).w(FUNC(cave_state::videoregs_w<0>)).share("videoregs.0");                             // Video Regs
	map(0x800000, 0x800007).r(FUNC(cave_state::irq_cause_r));                                                     // IRQ Cause
	map(0x900000, 0x900005).rw(m_tilemap[0], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 0 Control
	map(0xa00000, 0xa00005).rw(m_tilemap[1], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 1 Control
	map(0xb00000, 0xb00001).portr("IN0");                                                                         // Inputs
	map(0xb00002, 0xb00003).portr("IN1");                                                                         // Inputs + EEPROM
	map(0xc00000, 0xc00000).w(FUNC(cave_state::eeprom_w));                                                        // EEPROM
}


/***************************************************************************
                                Dodonpachi
***************************************************************************/

void cave_state::ddonpach_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                                                // ROM
	map(0x100000, 0x10ffff).ram();                                                                                // RAM
	map(0x300000, 0x300003).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask16(0x00ff);   // YMZ280
	map(0x400000, 0x40ffff).ram().share("spriteram.0");                                                           // Sprites
	map(0x500000, 0x507fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_map));                                   // Layer 0
	map(0x600000, 0x607fff).m(m_tilemap[1], FUNC(tilemap038_device::vram_map));                                   // Layer 1
	map(0x700000, 0x703fff).mirror(0x00c000).m(m_tilemap[2], FUNC(tilemap038_device::vram_8x8_map));              // Layer 2
	map(0x800000, 0x80007f).w(FUNC(cave_state::videoregs_w<0>)).share("videoregs.0");                             // Video Regs
	map(0x800000, 0x800007).r(FUNC(cave_state::irq_cause_r));                                                     // IRQ Cause
	map(0x900000, 0x900005).rw(m_tilemap[0], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 0 Control
	map(0xa00000, 0xa00005).rw(m_tilemap[1], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 1 Control
	map(0xb00000, 0xb00005).rw(m_tilemap[2], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 2 Control
	map(0xc00000, 0xc0ffff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0");              // Palette
	map(0xd00000, 0xd00001).portr("IN0");                                                                         // Inputs
	map(0xd00002, 0xd00003).portr("IN1");                                                                         // Inputs + EEPROM
	map(0xe00000, 0xe00000).w(FUNC(cave_state::eeprom_w));                                                        // EEPROM
}


/***************************************************************************
                                    Donpachi
***************************************************************************/

u16 cave_state::donpachi_videoregs_r(offs_t offset)
{
	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3: return irq_cause_r(offset);

		default:    return 0x0000;
	}
}

void cave_state::donpachi_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                                                                             // ROM
	map(0x100000, 0x10ffff).ram();                                                                                             // RAM
	map(0x200000, 0x207fff).m(m_tilemap[1], FUNC(tilemap038_device::vram_map));                                                // Layer 1
	map(0x300000, 0x307fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_map));                                                // Layer 0
	map(0x400000, 0x403fff).mirror(0x004000).m(m_tilemap[2], FUNC(tilemap038_device::vram_8x8_map));                           // Layer 2
	map(0x500000, 0x50ffff).ram().share("spriteram.0");                                                                        // Sprites
	map(0x600000, 0x600005).rw(m_tilemap[1], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w));              // Layer 1 Control
	map(0x700000, 0x700005).rw(m_tilemap[0], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w));              // Layer 0 Control
	map(0x800000, 0x800005).rw(m_tilemap[2], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w));              // Layer 2 Control
	map(0x900000, 0x90007f).rw(FUNC(cave_state::donpachi_videoregs_r), FUNC(cave_state::videoregs_w<0>)).share("videoregs.0"); // Video Regs
	map(0xa08000, 0xa08fff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0");                           // Palette
	map(0xb00000, 0xb00003).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write)).umask16(0x00ff);             // M6295 #0
	map(0xb00010, 0xb00013).rw("oki2", FUNC(okim6295_device::read), FUNC(okim6295_device::write)).umask16(0x00ff);             // M6295 #1
	map(0xb00020, 0xb0002f).w("nmk112", FUNC(nmk112_device::okibank_w)).umask16(0x00ff);                                       // Samples bank
	map(0xc00000, 0xc00001).portr("IN0");                                                                                      // Inputs
	map(0xc00002, 0xc00003).portr("IN1");                                                                                      // Inputs + EEPROM
	map(0xd00000, 0xd00000).w(FUNC(cave_state::eeprom_w));                                                                     // EEPROM
}


/***************************************************************************
                                    Esprade
***************************************************************************/

void cave_state::esprade_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                                                // ROM
	map(0x100000, 0x10ffff).ram();                                                                                // RAM
	map(0x300000, 0x300003).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask16(0x00ff);   // YMZ280
	map(0x400000, 0x40ffff).ram().share("spriteram.0");                                                           // Sprites
	map(0x500000, 0x507fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_map));                                   // Layer 0
	map(0x600000, 0x607fff).m(m_tilemap[1], FUNC(tilemap038_device::vram_map));                                   // Layer 1
	map(0x700000, 0x707fff).m(m_tilemap[2], FUNC(tilemap038_device::vram_map));                                   // Layer 2
	map(0x800000, 0x80007f).w(FUNC(cave_state::videoregs_w<0>)).share("videoregs.0");                             // Video Regs
	map(0x800000, 0x800007).r(FUNC(cave_state::irq_cause_r));                                                     // IRQ Cause
	map(0x900000, 0x900005).rw(m_tilemap[0], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 0 Control
	map(0xa00000, 0xa00005).rw(m_tilemap[1], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 1 Control
	map(0xb00000, 0xb00005).rw(m_tilemap[2], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 2 Control
	map(0xc00000, 0xc0ffff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0");              // Palette
	map(0xd00000, 0xd00001).portr("IN0");                                                                         // Inputs
	map(0xd00002, 0xd00003).portr("IN1");                                                                         // Inputs + EEPROM
	map(0xe00000, 0xe00000).w(FUNC(cave_state::eeprom_w));                                                        // EEPROM
}


/***************************************************************************
                                    Gaia Crusaders
***************************************************************************/

void cave_state::gaia_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                                                // ROM
	map(0x100000, 0x10ffff).ram();                                                                                // RAM
	map(0x300000, 0x300003).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask16(0x00ff);   // YMZ280
	map(0x400000, 0x40ffff).ram().share("spriteram.0");                                                           // Sprite
	map(0x500000, 0x507fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_map));                                   // Layer 0
	map(0x508000, 0x50ffff).ram();                                                                                // More Layer 0, Tested but not used?
	map(0x600000, 0x607fff).m(m_tilemap[1], FUNC(tilemap038_device::vram_map));                                   // Layer 1
	map(0x608000, 0x60ffff).ram();                                                                                // More Layer 1, Tested but not used?
	map(0x700000, 0x707fff).m(m_tilemap[2], FUNC(tilemap038_device::vram_map));                                   // Layer 2
	map(0x708000, 0x70ffff).ram();                                                                                // More Layer 2, Tested but not used?
	map(0x800000, 0x80007f).w(FUNC(cave_state::videoregs_w<0>)).share("videoregs.0");                             // Video Regs
	map(0x800000, 0x800007).r(FUNC(cave_state::irq_cause_r));                                                     // IRQ Cause
	map(0x900000, 0x900005).rw(m_tilemap[0], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 0 Control
	map(0xa00000, 0xa00005).rw(m_tilemap[1], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 1 Control
	map(0xb00000, 0xb00005).rw(m_tilemap[2], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 2 Control
	map(0xc00000, 0xc0ffff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0");              // Palette
	map(0xd00010, 0xd00011).portr("IN0");                                                                         // Inputs
	map(0xd00011, 0xd00011).w(FUNC(cave_state::gaia_coin_w));                                                     // Coin counter only
	map(0xd00012, 0xd00013).portr("IN1");                                                                         // Inputs
	map(0xd00014, 0xd00015).portr("DSW");                                                                         // Dips
	map(0xd00014, 0xd00015).w("watchdog", FUNC(watchdog_timer_device::reset16_w));                                // Watchdog?
}


/***************************************************************************
                                    Guwange
***************************************************************************/

void cave_state::guwange_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                                                // ROM
	map(0x200000, 0x20ffff).ram();                                                                                // RAM
	map(0x300000, 0x30007f).w(FUNC(cave_state::videoregs_w<0>)).share("videoregs.0");                             // Video Regs
	map(0x300000, 0x300007).r(FUNC(cave_state::irq_cause_r));                                                     // IRQ Cause
	map(0x400000, 0x40ffff).ram().share("spriteram.0");                                                           // Sprites
	map(0x500000, 0x507fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_map));                                   // Layer 0
	map(0x600000, 0x607fff).m(m_tilemap[1], FUNC(tilemap038_device::vram_map));                                   // Layer 1
	map(0x700000, 0x707fff).m(m_tilemap[2], FUNC(tilemap038_device::vram_map));                                   // Layer 2
	map(0x800000, 0x800003).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask16(0x00ff);   // YMZ280
	map(0x900000, 0x900005).rw(m_tilemap[0], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 0 Control
	map(0xa00000, 0xa00005).rw(m_tilemap[1], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 1 Control
	map(0xb00000, 0xb00005).rw(m_tilemap[2], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 2 Control
/**/map(0xc00000, 0xc0ffff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0");              // Palette
	map(0xd00010, 0xd00011).portr("IN0");                                                                         // Inputs
	map(0xd00011, 0xd00011).w(FUNC(cave_state::guwange_eeprom_w));                                                // EEPROM
	map(0xd00012, 0xd00013).portr("IN1");                                                                         // Inputs + EEPROM
//  map(0xd00012, 0xd00013).nopw();                                                                               // ?
//  map(0xd00014, 0xd00015).nopw();                                                                               // ? $800068 in dfeveron ? probably Watchdog
}


/***************************************************************************
                                Hotdog Storm
***************************************************************************/

void cave_state::hotdogst_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                                                // ROM
	map(0x300000, 0x30ffff).ram();                                                                                // RAM
	map(0x408000, 0x408fff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0");              // Palette
	map(0x880000, 0x887fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_map));                                   // Layer 0
	map(0x900000, 0x907fff).m(m_tilemap[1], FUNC(tilemap038_device::vram_map));                                   // Layer 1
	map(0x980000, 0x987fff).m(m_tilemap[2], FUNC(tilemap038_device::vram_map));                                   // Layer 2
	map(0xa80000, 0xa8007f).w(FUNC(cave_state::videoregs_w<0>)).share("videoregs.0");                             // Video Regs
	map(0xa80000, 0xa80007).r(FUNC(cave_state::irq_cause_r));                                                     // IRQ Cause
//  map(0xa8006e, 0xa8006f).r(FUNC(cave_state::soundlatch_ack_r));                                                // From Sound CPU
	map(0xa8006e, 0xa8006f).w(FUNC(cave_state::sound_cmd_w));                                                     // To Sound CPU
	map(0xb00000, 0xb00005).rw(m_tilemap[0], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 0 Control
	map(0xb80000, 0xb80005).rw(m_tilemap[1], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 1 Control
	map(0xc00000, 0xc00005).rw(m_tilemap[2], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 2 Control
	map(0xc80000, 0xc80001).portr("IN0");                                                                         // Inputs
	map(0xc80002, 0xc80003).portr("IN1");                                                                         // Inputs + EEPROM
	map(0xd00000, 0xd00000).w(FUNC(cave_state::hotdogst_eeprom_w));                                               // EEPROM
	map(0xd00002, 0xd00003).nopw();                                                                               // ???
	map(0xf00000, 0xf0ffff).ram().share("spriteram.0");                                                           // Sprites
}


/***************************************************************************
                               Koro Koro Quest
***************************************************************************/

void cave_state::show_leds()
{
#ifdef MAME_DEBUG
//  popmessage("led %04X eep %02X", m_leds[0], (m_leds[1] >> 8) & ~0x70);
#endif
}

void cave_state::korokoro_leds_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_leds[0]);

	m_led_outputs[0] = BIT(data, 15);
	m_led_outputs[1] = BIT(data, 14);
	m_led_outputs[2] = BIT(data, 12);    // square button
	m_led_outputs[3] = BIT(data, 11);    // round  button
//  machine().bookkeeping().coin_lockout_w(1, ~data & 0x0200);   // coin lockouts?
//  machine().bookkeeping().coin_lockout_w(0, ~data & 0x0100);

//  machine().bookkeeping().coin_counter_w(2, data & 0x0080);
//  machine().bookkeeping().coin_counter_w(1, data & 0x0020);
	machine().bookkeeping().coin_counter_w(0, data & 0x0010);

	m_led_outputs[5] = BIT(data, 3);
	m_led_outputs[6] = BIT(data, 2);
	m_led_outputs[7] = BIT(data, 1);
	m_led_outputs[8] = BIT(data, 0);

	show_leds();
}


void cave_state::korokoro_eeprom_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (data & ~0x7000)
	{
		logerror("%s: Unknown EEPROM bit written %04X\n",machine().describe_context(),data);
		COMBINE_DATA(&m_leds[1]);
		show_leds();
	}

	if (ACCESSING_BITS_8_15)  // even address
	{
		m_hopper = data & 0x0100;   // ???

		// latch the bit
		m_eeprom->di_write((data & 0x4000) >> 14);

		// reset line asserted: reset.
		m_eeprom->cs_write((data & 0x1000) ? ASSERT_LINE : CLEAR_LINE);

		// clock line asserted: write latch or select next bit to read
		m_eeprom->clk_write((data & 0x2000) ? ASSERT_LINE : CLEAR_LINE);
	}
}

READ_LINE_MEMBER(cave_state::korokoro_hopper_r)
{
	return m_hopper ? 1 : 0;
}


void cave_state::korokoro_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                                                         // ROM
	map(0x100000, 0x107fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_writeonly_map));                  // Layer 0
	map(0x140000, 0x140005).w(m_tilemap[0], FUNC(tilemap038_device::vregs_w));                             // Layer 0 Control
	map(0x180000, 0x187fff).writeonly().share("spriteram.0");                                              // Sprites
	map(0x1c0000, 0x1c007f).w(FUNC(cave_state::videoregs_w<0>)).share("videoregs.0");                      // Video Regs
	map(0x1c0000, 0x1c0007).r(FUNC(cave_state::irq_cause_r));                                              // IRQ Cause
	map(0x200000, 0x207fff).writeonly().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0"); // Palette
//  map(0x240000, 0x240003).r("ymz", FUNC(ymz280b_device::read)).umask16(0x00ff);                          // YMZ280
	map(0x240000, 0x240003).w("ymz", FUNC(ymz280b_device::write)).umask16(0x00ff);                         // YMZ280
	map(0x280000, 0x280001).portr("IN0");                                                                  // Inputs + ???
	map(0x280002, 0x280003).portr("IN1");                                                                  // Inputs + EEPROM
	map(0x280008, 0x280009).w(FUNC(cave_state::korokoro_leds_w));                                          // Leds
	map(0x28000a, 0x28000b).w(FUNC(cave_state::korokoro_eeprom_w));                                        // EEPROM
	map(0x28000c, 0x28000d).nopw();                                                                        // 0 (watchdog?)
	map(0x300000, 0x30ffff).ram();                                                                         // RAM
}

void cave_state::crusherm_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                                                         // ROM
	map(0x100000, 0x107fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_writeonly_map));                  // Layer 0
	map(0x140000, 0x140005).w(m_tilemap[0], FUNC(tilemap038_device::vregs_w));                             // Layer 0 Control
	map(0x180000, 0x187fff).writeonly().share("spriteram.0");                                              // Sprites
	map(0x200000, 0x207fff).writeonly().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0"); // Palette
	map(0x240000, 0x240003).w("ymz", FUNC(ymz280b_device::write)).umask16(0x00ff);                         // YMZ280
	map(0x280000, 0x280001).portr("IN0");                                                                  // Inputs + ???
	map(0x280002, 0x280003).portr("IN1");                                                                  // Inputs + EEPROM
	map(0x280008, 0x280009).w(FUNC(cave_state::korokoro_leds_w));                                          // Leds
	map(0x28000a, 0x28000b).w(FUNC(cave_state::korokoro_eeprom_w));                                        // EEPROM
	map(0x28000c, 0x28000d).nopw();                                                                        // 0 (watchdog?)
	map(0x300000, 0x30007f).w(FUNC(cave_state::videoregs_w<0>)).share("videoregs.0");                      // Video Regs
	map(0x300000, 0x300007).r(FUNC(cave_state::irq_cause_r));                                              // IRQ Cause
	map(0x340000, 0x34ffff).ram();                                                                         // RAM
}

/***************************************************************************
                                Mazinger Z
***************************************************************************/

void cave_state::mazinger_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                                                                // ROM
	map(0x100000, 0x10ffff).ram();                                                                                // RAM
	map(0x200000, 0x20ffff).ram().share("spriteram.0");                                                           // Sprites
	map(0x300000, 0x30007f).w(FUNC(cave_state::videoregs_w<0>)).share("videoregs.0");                             // Video Regs
	map(0x300000, 0x300007).r(FUNC(cave_state::irq_cause_r));                                                     // IRQ Cause
	map(0x300068, 0x300069).w("watchdog", FUNC(watchdog_timer_device::reset16_w));                                // Watchdog
	map(0x30006e, 0x30006f).rw(FUNC(cave_state::soundlatch_ack_r), FUNC(cave_state::sound_cmd_w));                // From Sound CPU
	map(0x400000, 0x403fff).mirror(0x004000).m(m_tilemap[1], FUNC(tilemap038_device::vram_8x8_map));              // Layer 1
	map(0x500000, 0x503fff).mirror(0x004000).m(m_tilemap[0], FUNC(tilemap038_device::vram_8x8_map));              // Layer 0
	map(0x600000, 0x600005).rw(m_tilemap[1], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 1 Control
	map(0x700000, 0x700005).rw(m_tilemap[0], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 0 Control
	map(0x800000, 0x800001).portr("IN0");                                                                         // Inputs
	map(0x800002, 0x800003).portr("IN1");                                                                         // Inputs + EEPROM
	map(0x900000, 0x900000).w(FUNC(cave_state::eeprom_w));                                                        // EEPROM
	map(0xc08000, 0xc0ffff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0");              // Palette
	map(0xd00000, 0xd7ffff).rom().region("user1", 0);                                                             // extra data ROM
}


/***************************************************************************
                                Metamoqester
***************************************************************************/

void cave_state::metmqstr_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                                                                // ROM
	map(0x100000, 0x17ffff).rom();                                                                                // ROM
	map(0x200000, 0x27ffff).rom();                                                                                // ROM
	map(0x408000, 0x408fff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0");              // Palette
	map(0x600000, 0x600001).r("watchdog", FUNC(watchdog_timer_device::reset16_r));                                // Watchdog?
	map(0x880000, 0x887fff).m(m_tilemap[2], FUNC(tilemap038_device::vram_map));                                   // Layer 2
	map(0x888000, 0x88ffff).ram();                                                                                //
	map(0x900000, 0x907fff).m(m_tilemap[1], FUNC(tilemap038_device::vram_map));                                   // Layer 1
	map(0x908000, 0x90ffff).ram();                                                                                //
	map(0x980000, 0x987fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_map));                                   // Layer 0
	map(0x988000, 0x98ffff).ram();                                                                                //
	map(0xa80000, 0xa8007f).w(FUNC(cave_state::videoregs_w<0>)).share("videoregs.0");                             // Video Regs
	map(0xa80000, 0xa80007).r(FUNC(cave_state::irq_cause_r));                                                     // IRQ Cause
	map(0xa80068, 0xa80069).w("watchdog", FUNC(watchdog_timer_device::reset16_w));                                // Watchdog?
	map(0xa8006c, 0xa8006d).r(FUNC(cave_state::soundflags_ack_r))/*.nopw()*/;                                     // Communication
	map(0xa8006e, 0xa8006f).rw(FUNC(cave_state::soundlatch_ack_r), FUNC(cave_state::sound_cmd_w));                // From Sound CPU
	map(0xb00000, 0xb00005).rw(m_tilemap[2], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 2 Control
	map(0xb80000, 0xb80005).rw(m_tilemap[1], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 1 Control
	map(0xc00000, 0xc00005).rw(m_tilemap[0], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 0 Control
	map(0xc80000, 0xc80001).portr("IN0");                                                                         // Inputs
	map(0xc80002, 0xc80003).portr("IN1");                                                                         // Inputs + EEPROM
	map(0xd00000, 0xd00000).w(FUNC(cave_state::metmqstr_eeprom_w));                                               // EEPROM
	map(0xf00000, 0xf0ffff).ram().share("spriteram.0");                                                           // Sprites
	// 0xf00000-0xf07fff Sprite RAM
	// 0xf08000-0xf0ffff Work RAM
}


/***************************************************************************
                               Poka Poka Satan
***************************************************************************/

void cave_state::ppsatan_io_mux_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_ppsatan_io_mux);
}

template<int Player>
u16 cave_state::ppsatan_touch_r()
{
	u8 ret_x = 0, ret_y = 0;

	u16 x = m_touch_x[Player]->read();
	u16 y = m_touch_y[Player]->read();

	if (x & 0x8000) // touching
	{
		x &= 0x7fff;

		// x
		int slot_x = floor( ((320.0f - 1 - x) - 12) / 20 );

		if (slot_x < 0)
			slot_x = 0;
		else if (slot_x > 14)
			slot_x = 14;

		if ( (m_ppsatan_io_mux & (1 << slot_x)) || ((m_ppsatan_io_mux << 13) & (1 << slot_x)) )
			ret_x |= 1 << (slot_x % 8);

		// y
		int slot_y = floor( ((224.0f - 1 - y) - 14) / 18 );

		if (slot_y < 0)
			slot_y = 0;
		else if (slot_y > 10)
			slot_y = 10;

		if ( ((m_ppsatan_io_mux >> 2) & (1 << slot_y)) || ((m_ppsatan_io_mux << 6) & (1 << slot_y)) )
			ret_y |= 1 << (slot_y % 6);

//      if (!Player)    popmessage("TOUCH %03x %03x -> %f -> %d", x, y, ((320.0f - 1 - x) - 12) / 20, slot_x);
	}

	return ret_x | (ret_y << 8);
}

void cave_state::ppsatan_out_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		machine().bookkeeping().coin_counter_w(0, data & 0x0001);

		m_led_outputs[0] = BIT(data, 4);
		m_led_outputs[1] = BIT(data, 5);
		m_led_outputs[2] = BIT(data, 6);
		m_led_outputs[3] = BIT(data, 7);
	}
	if (ACCESSING_BITS_8_15)
	{
		m_led_outputs[4] = BIT(data, 8);
		m_led_outputs[5] = BIT(data, 9);
		m_led_outputs[6] = BIT(data, 10);    // not tested in service mode
		m_led_outputs[7] = BIT(data, 11);    // not tested in service mode

		m_oki[0]->set_rom_bank((data & 0x8000) >> 15);
	}

//  popmessage("OUT %04x", data);
}

void cave_state::ppsatan_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();                                                                                // ROM
	map(0x040000, 0x04ffff).ram();                                                                                // RAM

	// Left Screen (Player 2)
	map(0x080000, 0x080005).rw(m_tilemap[1], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer Control
	map(0x100000, 0x107fff).m(m_tilemap[1], FUNC(tilemap038_device::vram_map));                                   // Layer
//  map(0x180000, 0x1803ff).ram()                                                                                 // Palette (Tilemaps)
//  map(0x187800, 0x188fff).ram().w(m_palette[1], FUNC(palette_device::write16)).share("palette.1");              // Palette (Sprites)
	map(0x180000, 0x188fff).ram().w(m_palette[1], FUNC(palette_device::write16)).share("palette.1");              // Palette
	map(0x1c0000, 0x1c7fff).ram().share("spriteram.1");                                                           // Sprites
	map(0x200000, 0x200001).portr("SYSTEM");                                                                      // DSW + (unused) EEPROM
	map(0x200000, 0x200001).w(FUNC(cave_state::ppsatan_out_w));                                                   // Outputs + OKI banking
	map(0x200002, 0x200003).rw(FUNC(cave_state::ppsatan_touch_r<1>), FUNC(cave_state::ppsatan_eeprom_w));         // Touch Screen + (unused) EEPROM
	map(0x200004, 0x200005).rw(FUNC(cave_state::ppsatan_touch_r<0>), FUNC(cave_state::ppsatan_io_mux_w));         // Touch Screen
	map(0x200006, 0x200007).nopw();                                                                               // Lev. 2 IRQ Ack?
	map(0x2c0000, 0x2c007f).w(FUNC(cave_state::videoregs_w<1>)).share("videoregs.1");                             // Video Regs
	map(0x2c0000, 0x2c0007).r(FUNC(cave_state::irq_cause_r));                                                     // IRQ Cause
	map(0x2c0068, 0x2c0069).w("watchdog", FUNC(watchdog_timer_device::reset16_w));                                // Watchdog

	map(0x300001, 0x300001).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));                // M6295

	// Right Screen (Player 1)
	map(0x480000, 0x480005).rw(m_tilemap[2], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer Control
	map(0x500000, 0x507fff).m(m_tilemap[2], FUNC(tilemap038_device::vram_map));                                   // Layer
//  map(0x580000, 0x5803ff).ram()                                                                                 // Palette (Tilemaps)
//  map(0x587800, 0x588fff).ram().w(m_palette[2], FUNC(palette_device::write16)).share("palette.2");              // Palette (Sprites)
	map(0x580000, 0x588fff).ram().w(m_palette[2], FUNC(palette_device::write16)).share("palette.2");              // Palette
	map(0x5c0000, 0x5c7fff).ram().share("spriteram.2");                                                           // Sprites
	map(0x6c0000, 0x6c007f).w(FUNC(cave_state::videoregs_w<2>)).share("videoregs.2");                             // Video Regs

	// Top Screen
	map(0x880000, 0x880005).rw(m_tilemap[0], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer Control
	map(0x900000, 0x907fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_map));                                   // Layer
//  map(0x980000, 0x9803ff).ram();                                                                                // Palette (Tilemaps)
//  map(0x987800, 0x988fff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0");              // Palette (Sprites)
	map(0x980000, 0x988fff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0");              // Palette
	map(0x9c0000, 0x9c7fff).ram().share("spriteram.0");                                                           // Sprites
	map(0xac0000, 0xac007f).w(FUNC(cave_state::videoregs_w<0>)).share("videoregs.0");                             // Video Regs
}


/***************************************************************************
                                Power Instinct 2
***************************************************************************/

u16 cave_state::pwrinst2_eeprom_r()
{
	return ~8 + ((m_eeprom->do_read() & 1) ? 8 : 0);
}

template<int Chip>
void cave_state::pwrinst2_vctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (offset == 4 / 2)
	{
		switch (data & 0x000f)
		{
			case 1: data = (data & ~0x000f) | 0;    break;
			case 2: data = (data & ~0x000f) | 1;    break;
			case 4: data = (data & ~0x000f) | 2;    break;
			default:
			case 8: data = (data & ~0x000f) | 3;    break;
		}
	}
	m_tilemap[Chip]->vregs_w(offset, data, mem_mask);
}

void cave_state::pwrinst2_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();                                                                                             // ROM
	map(0x400000, 0x40ffff).ram();                                                                                             // RAM
	map(0x500000, 0x500001).portr("IN0");                                                                                      // Inputs
	map(0x500002, 0x500003).portr("IN1");                                                                                      //
	map(0x600000, 0x6fffff).rom().region("user1", 0);                                                                          // extra data ROM space
	map(0x700000, 0x700000).w(FUNC(cave_state::eeprom_w));                                                                     // EEPROM
	map(0x800000, 0x807fff).m(m_tilemap[2], FUNC(tilemap038_device::vram_map));                                                // Layer 2
	map(0x880000, 0x887fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_map));                                                // Layer 0
	map(0x900000, 0x907fff).m(m_tilemap[1], FUNC(tilemap038_device::vram_map));                                                // Layer 1
	map(0x980000, 0x983fff).mirror(0x004000).m(m_tilemap[3], FUNC(tilemap038_device::vram_8x8_map));                           // Layer 3
	map(0xa00000, 0xa0ffff).ram().share("spriteram.0");                                                                        // Sprites
	map(0xa10000, 0xa1ffff).ram();                                                                                             // Sprites?
	map(0xa80000, 0xa8007f).rw(FUNC(cave_state::donpachi_videoregs_r), FUNC(cave_state::videoregs_w<0>)).share("videoregs.0"); // Video Regs
	map(0xb00000, 0xb00005).r(m_tilemap[2], FUNC(tilemap038_device::vregs_r)).w(FUNC(cave_state::pwrinst2_vctrl_w<2>));        // Layer 2 Control
	map(0xb80000, 0xb80005).r(m_tilemap[0], FUNC(tilemap038_device::vregs_r)).w(FUNC(cave_state::pwrinst2_vctrl_w<0>));        // Layer 0 Control
	map(0xc00000, 0xc00005).r(m_tilemap[1], FUNC(tilemap038_device::vregs_r)).w(FUNC(cave_state::pwrinst2_vctrl_w<1>));        // Layer 1 Control
	map(0xc80000, 0xc80005).r(m_tilemap[3], FUNC(tilemap038_device::vregs_r)).w(FUNC(cave_state::pwrinst2_vctrl_w<3>));        // Layer 3 Control
	map(0xd80000, 0xd80001).r(FUNC(cave_state::soundlatch_ack_r));                                                             // ? From Sound CPU
	map(0xe00000, 0xe00001).w(FUNC(cave_state::sound_cmd_w));                                                                  // To Sound CPU
	map(0xe80000, 0xe80001).r(FUNC(cave_state::pwrinst2_eeprom_r));                                                            // EEPROM
	map(0xf00000, 0xf04fff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0");                           // Palette
}


/***************************************************************************
                                Sailor Moon
***************************************************************************/

u16 cave_state::sailormn_input0_r()
{
//  watchdog_reset16_r(0, 0);    // written too rarely for mame.
	return m_io_in0->read();
}

void cave_state::sailormn_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                                                                // ROM
	map(0x100000, 0x10ffff).ram();                                                                                // RAM
	map(0x110000, 0x110001).ram();                                                                                // (agallet)
	map(0x200000, 0x3fffff).rom();                                                                                // ROM
	map(0x400000, 0x407fff).ram();                                                                                // (agallet)
	map(0x408000, 0x40bfff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0");              // Palette
	map(0x40c000, 0x40ffff).ram();                                                                                // (agallet)
	map(0x410000, 0x410001).ram();                                                                                // (agallet)
	map(0x500000, 0x50ffff).ram().share("spriteram.0");                                                           // Sprites
	map(0x510000, 0x510001).ram();                                                                                // (agallet)
	map(0x600000, 0x600001).r(FUNC(cave_state::sailormn_input0_r));                                               // Inputs + Watchdog!
	map(0x600002, 0x600003).portr("IN1");                                                                         // Inputs + EEPROM
	map(0x700000, 0x700000).w(FUNC(cave_state::sailormn_eeprom_w));                                               // EEPROM
	map(0x800000, 0x807fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_map));                                   // Layer 0
	map(0x880000, 0x887fff).m(m_tilemap[1], FUNC(tilemap038_device::vram_map));                                   // Layer 1
	map(0x900000, 0x907fff).m(m_tilemap[2], FUNC(tilemap038_device::vram_map));                                   // Layer 2
	map(0x908000, 0x908001).ram();                                                                                // (agallet)
	map(0xa00000, 0xa00005).rw(m_tilemap[0], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 0 Control
	map(0xa80000, 0xa80005).rw(m_tilemap[1], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 1 Control
	map(0xb00000, 0xb00005).rw(m_tilemap[2], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 2 Control
	map(0xb80000, 0xb8007f).w(FUNC(cave_state::videoregs_w<0>)).share("videoregs.0");                             // Video Regs
	map(0xb80000, 0xb80007).r(FUNC(cave_state::irq_cause_r));                                                     // IRQ Cause (bit 2 tested!)
	map(0xb8006c, 0xb8006d).r(FUNC(cave_state::soundflags_ack_r));                                                // Communication
	map(0xb8006e, 0xb8006f).r(FUNC(cave_state::soundlatch_ack_r));                                                // From Sound CPU
	map(0xb8006e, 0xb8006f).w(FUNC(cave_state::sound_cmd_w));                                                     // To Sound CPU
}


/***************************************************************************
                            Tekken Card World
***************************************************************************/

void cave_state::tekkencw_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                                                   // ROM
	map(0x100000, 0x10ffff).ram().share("nvram");                                                    // RAM (battery)
	map(0x200000, 0x20ffff).ram().share("spriteram.0");                                              // Sprites
	map(0x300000, 0x307fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_map));                      // Layer 0
	map(0x400000, 0x400001).portr("IN0");                                                            // Inputs + EEPROM + Hopper
	map(0x400002, 0x400003).portr("IN1");                                                            // Inputs
	map(0x500000, 0x500005).w(m_tilemap[0], FUNC(tilemap038_device::vregs_w));                       // Layer 0 Control
	map(0x600000, 0x60ffff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0"); // Palette
	map(0x700000, 0x70007f).w(FUNC(cave_state::videoregs_w<0>)).share("videoregs.0");                // Video Regs
	map(0x700000, 0x700007).r(FUNC(cave_state::irq_cause_r));                                        // IRQ Cause
	map(0x700068, 0x700069).w("watchdog", FUNC(watchdog_timer_device::reset16_w));                   // Watchdog
	map(0x800001, 0x800001).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));   // M6295
	map(0xc00001, 0xc00001).w(FUNC(cave_state::tjumpman_leds_w));                                    // Leds + Hopper
	map(0xe00001, 0xe00001).w(FUNC(cave_state::tjumpman_eeprom_w));                                  // EEPROM
}


/***************************************************************************
                          Tekken Battle Scratch
***************************************************************************/

void cave_state::tekkenbs_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                                                   // ROM
	map(0x100000, 0x10ffff).ram().share("nvram");                                                    // RAM (battery)
	map(0x200000, 0x20ffff).ram().share("spriteram.0");                                              // Sprites
	map(0x300000, 0x307fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_map));                      // Layer 0
	map(0x400000, 0x40ffff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0"); // Palette
	map(0x500000, 0x500005).w(m_tilemap[0], FUNC(tilemap038_device::vregs_w));                       // Layer 0 Control
	map(0x600000, 0x600001).portr("IN0");                                                            // Inputs + EEPROM + Hopper
	map(0x600002, 0x600003).portr("IN1");                                                            // Inputs
	map(0x700000, 0x70007f).w(FUNC(cave_state::videoregs_w<0>)).share("videoregs.0");                // Video Regs
	map(0x700000, 0x700007).r(FUNC(cave_state::irq_cause_r));                                        // IRQ Cause
	map(0x700068, 0x700069).w("watchdog", FUNC(watchdog_timer_device::reset16_w));                   // Watchdog
	map(0x800001, 0x800001).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));   // M6295
	map(0xc00001, 0xc00001).w(FUNC(cave_state::tjumpman_leds_w));                                    // Leds + Hopper
	map(0xe00001, 0xe00001).w(FUNC(cave_state::tjumpman_eeprom_w));                                  // EEPROM
}


/***************************************************************************
                            Tobikose! Jumpman
***************************************************************************/

void cave_state::tjumpman_eeprom_w(u8 data)
{
	if (data & ~0x38)
		logerror("%s: Unknown EEPROM bit written %04X\n",machine().describe_context(),data);

	// latch the bit
	m_eeprom->di_write(BIT(data, 5));

	// reset line asserted: reset.
	m_eeprom->cs_write(BIT(data, 3) ? ASSERT_LINE : CLEAR_LINE);

	// clock line asserted: write latch or select next bit to read
	m_eeprom->clk_write(BIT(data, 4) ? ASSERT_LINE : CLEAR_LINE);
}

void cave_state::tjumpman_leds_w(u8 data)
{
	m_led_outputs[0] = BIT(data, 0); // suru
	m_led_outputs[1] = BIT(data, 1); // shinai
	m_led_outputs[2] = BIT(data, 2); // payout
	m_led_outputs[3] = BIT(data, 3); // go
	m_led_outputs[4] = BIT(data, 4); // 1 bet
	m_led_outputs[5] = BIT(data, 5); // medal
	m_hopper = BIT(data, 6);  // hopper
	m_led_outputs[6] = BIT(data, 7); // 3 bet

//  popmessage("led %04X", data);
}

READ_LINE_MEMBER(cave_state::tjumpman_hopper_r)
{
	return (m_hopper && !(m_screen[0]->frame_number() % 10)) ? 0 : 1;
}

void cave_state::tjumpman_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                                                   // ROM
	map(0x100000, 0x10ffff).ram().share("nvram");                                                    // RAM (battery)
	map(0x200000, 0x20ffff).ram().share("spriteram.0");                                              // Sprites
	map(0x300000, 0x307fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_map));                      // Layer 0
	map(0x304000, 0x307fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_16x16_writeonly_map));      // Layer 0 - 16x16 tiles mapped here
	map(0x400000, 0x400005).w(m_tilemap[0], FUNC(tilemap038_device::vregs_w));                       // Layer 0 Control
	map(0x500000, 0x50ffff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0"); // Palette
	map(0x600000, 0x600001).portr("IN0");                                                            // Inputs + EEPROM + Hopper
	map(0x600002, 0x600003).portr("IN1");                                                            // Inputs
	map(0x700000, 0x70007f).w(FUNC(cave_state::videoregs_w<0>)).share("videoregs.0");                // Video Regs
	map(0x700000, 0x700007).r(FUNC(cave_state::irq_cause_r));                                        // IRQ Cause
	map(0x700068, 0x700069).w("watchdog", FUNC(watchdog_timer_device::reset16_w));                   // Watchdog
	map(0x800001, 0x800001).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));   // M6295
	map(0xc00001, 0xc00001).w(FUNC(cave_state::tjumpman_leds_w));                                    // Leds + Hopper
	map(0xe00001, 0xe00001).w(FUNC(cave_state::tjumpman_eeprom_w));                                  // EEPROM
}


/***************************************************************************
                                   Pac-Slot
***************************************************************************/

void cave_state::pacslot_leds_w(u8 data)
{
	m_led_outputs[0] = data & 0x0001; // pac-man
	m_led_outputs[1] = data & 0x0002; // ms. pac-man
	m_led_outputs[2] = data & 0x0004; // payout
	m_led_outputs[3] = data & 0x0008; // start
	m_led_outputs[4] = data & 0x0010; // bet
	m_led_outputs[5] = data & 0x0020; // medal
	m_hopper = data & 0x0040;  // hopper

//  popmessage("led %04X", data);
}

void cave_state::pacslot_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                                                   // ROM
	map(0x100000, 0x10ffff).ram().share("nvram");                                                    // RAM (battery)
	map(0x200000, 0x20ffff).ram().share("spriteram.0");                                              // Sprites
	map(0x300000, 0x307fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_map));                      // Layer 0
	map(0x400000, 0x40007f).w(FUNC(cave_state::videoregs_w<0>)).share("videoregs.0");                // Video Regs
	map(0x400000, 0x400007).r(FUNC(cave_state::irq_cause_r));                                        // IRQ Cause
	map(0x400068, 0x400069).w("watchdog", FUNC(watchdog_timer_device::reset16_w));                   // Watchdog
	map(0x500000, 0x500005).w(m_tilemap[0], FUNC(tilemap038_device::vregs_w));                       // Layer 0 Control
	map(0x600000, 0x60ffff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0"); // Palette
	map(0x700000, 0x700001).portr("IN0");                                                            // Inputs + EEPROM + Hopper
	map(0x700002, 0x700003).portr("IN1");                                                            // Inputs
	map(0x800001, 0x800001).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));   // M6295
	map(0xc00001, 0xc00001).w(FUNC(cave_state::pacslot_leds_w));                                     // Leds + Hopper
	map(0xe00001, 0xe00001).w(FUNC(cave_state::tjumpman_eeprom_w));                                  // EEPROM
}


/***************************************************************************
                                   Pac-Eight
***************************************************************************/

//TODO: leds need verifying

void cave_state::paceight_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                                                   // ROM
	map(0x100000, 0x10ffff).ram().share("nvram");                                                    // RAM (battery)
	map(0x200000, 0x20ffff).ram().share("spriteram.0");                                              // Sprites
	map(0x300000, 0x307fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_map));                      // Layer 0
	map(0x400000, 0x40ffff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0"); // Palette
	map(0x500000, 0x500001).portr("IN0");                                                            // Inputs + EEPROM + Hopper
	map(0x500002, 0x500003).portr("IN1");                                                            // Inputs
	map(0x600000, 0x600005).w(m_tilemap[0], FUNC(tilemap038_device::vregs_w));                       // Layer 0 Control
	map(0x700000, 0x70007f).w(FUNC(cave_state::videoregs_w<0>)).share("videoregs.0");                // Video Regs
	map(0x700000, 0x700007).r(FUNC(cave_state::irq_cause_r));                                        // IRQ Cause
	map(0x700068, 0x700069).w("watchdog", FUNC(watchdog_timer_device::reset16_w));                   // Watchdog
	map(0x800001, 0x800001).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));   // M6295
	map(0xc00000, 0xc00001).w(FUNC(cave_state::pacslot_leds_w));                                     // Leds + Hopper
	map(0xe00001, 0xe00001).w(FUNC(cave_state::tjumpman_eeprom_w));                                  // EEPROM
}


/***************************************************************************
                                   Pac-Carnival
***************************************************************************/

//TODO: leds need verifying

READ_LINE_MEMBER(cave_state::paccarn_bet4_r)
{
	return (m_io_bet->read() & 0x5) ? 1 : 0;
}

READ_LINE_MEMBER(cave_state::paccarn_bet8_r)
{
	return (m_io_bet->read() & 0x6) ? 1 : 0;
}

void cave_state::paccarn_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                                                   // ROM
	map(0x100000, 0x10ffff).ram().share("nvram");                                                    // RAM (battery)
	map(0x200000, 0x20ffff).ram().share("spriteram.0");                                              // Sprites
	map(0x300000, 0x307fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_map));                      // Layer 0
	map(0x400000, 0x400001).portr("IN0");                                                            // Inputs + EEPROM + Hopper
	map(0x400002, 0x400003).portr("IN1");                                                            // Inputs
	map(0x500000, 0x50ffff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0"); // Palette
	map(0x600000, 0x600005).w(m_tilemap[0], FUNC(tilemap038_device::vregs_w));                       // Layer 0 Control
	map(0x700000, 0x70007f).w(FUNC(cave_state::videoregs_w<0>)).share("videoregs.0");                // Video Regs
	map(0x700000, 0x700007).r(FUNC(cave_state::irq_cause_r));                                        // IRQ Cause
	map(0x700068, 0x700069).w("watchdog", FUNC(watchdog_timer_device::reset16_w));                   // Watchdog
	map(0x800001, 0x800001).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));   // M6295
	map(0xc00000, 0xc00001).w(FUNC(cave_state::pacslot_leds_w));                                     // Leds + Hopper
	map(0xe00001, 0xe00001).w(FUNC(cave_state::tjumpman_eeprom_w));                                  // EEPROM
}

/***************************************************************************
                                    Uo Poko
***************************************************************************/

void cave_state::uopoko_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                                                // ROM
	map(0x100000, 0x10ffff).ram();                                                                                // RAM
	map(0x300000, 0x300003).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask16(0x00ff);   // YMZ280
	map(0x400000, 0x40ffff).ram().share("spriteram.0");                                                           // Sprites
	map(0x500000, 0x507fff).m(m_tilemap[0], FUNC(tilemap038_device::vram_map));                                   // Layer 0
	map(0x600000, 0x60007f).w(FUNC(cave_state::videoregs_w<0>)).share("videoregs.0");                             // Video Regs
	map(0x600000, 0x600007).r(FUNC(cave_state::irq_cause_r));                                                     // IRQ Cause
	map(0x700000, 0x700005).rw(m_tilemap[0], FUNC(tilemap038_device::vregs_r), FUNC(tilemap038_device::vregs_w)); // Layer 0 Control
	map(0x800000, 0x80ffff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette.0");              // Palette
	map(0x900000, 0x900001).portr("IN0");                                                                         // Inputs
	map(0x900002, 0x900003).portr("IN1");                                                                         // Inputs + EEPROM
	map(0xa00000, 0xa00000).w(FUNC(cave_state::eeprom_w));                                                        // EEPROM
}



/***************************************************************************


                        Memory Maps - Sound CPU (Optional)


***************************************************************************/

template<int Mask>
void cave_state::z80_rombank_w(u8 data)
{
	if (data & ~Mask)
		logerror("%s: Z80 Bank %02X\n", machine().describe_context(), data);

	m_z80bank->set_entry(data & Mask);
}

template<int Mask>
void cave_state::oki1_bank_w(u8 data)
{
	int bank1 = (data >> 0) & Mask;
	int bank2 = (data >> 4) & Mask;
	m_okibank_lo[0]->set_entry(bank1);
	m_okibank_hi[0]->set_entry(bank2);
}

template<int Mask>
void cave_state::oki2_bank_w(u8 data)
{
	int bank1 = (data >> 0) & Mask;
	int bank2 = (data >> 4) & Mask;
	m_okibank_lo[1]->set_entry(bank1);
	m_okibank_hi[1]->set_entry(bank2);
}


void cave_state::oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).bankr("oki1_banklo");
	map(0x20000, 0x3ffff).bankr("oki1_bankhi");
}

void cave_state::oki2_map(address_map &map)
{
	map(0x00000, 0x1ffff).bankr("oki2_banklo");
	map(0x20000, 0x3ffff).bankr("oki2_bankhi");
}


/***************************************************************************
                                Hotdog Storm
***************************************************************************/

void cave_state::hotdogst_sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();             // ROM
	map(0x4000, 0x7fff).bankr("z80bank");  // ROM (Banked)
	map(0xe000, 0xffff).ram();             // RAM
}

void cave_state::hotdogst_sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(cave_state::z80_rombank_w<0x0f>));                                // ROM bank
	map(0x30, 0x30).r(FUNC(cave_state::soundlatch_lo_r));                                    // From Main CPU
	map(0x40, 0x40).r(FUNC(cave_state::soundlatch_hi_r));                                    // ""
	map(0x50, 0x51).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));      // YM2203
	map(0x60, 0x60).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));   // M6295
	map(0x70, 0x70).w(FUNC(cave_state::oki1_bank_w<0x3>));                                   // Samples bank
}


/***************************************************************************
                                Mazinger Z
***************************************************************************/

void cave_state::mazinger_sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();             // ROM
	map(0x4000, 0x7fff).bankr("z80bank");  // ROM (Banked)
	map(0xc000, 0xc7ff).ram();             // RAM
	map(0xf800, 0xffff).ram();             // RAM
}

void cave_state::mazinger_sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(cave_state::z80_rombank_w<0x07>));   // ROM bank
	map(0x10, 0x10).w(FUNC(cave_state::soundlatch_ack_w));      // To Main CPU
	map(0x30, 0x30).r(FUNC(cave_state::soundlatch_lo_r));       // From Main CPU
	map(0x50, 0x51).w("ymsnd", FUNC(ym2203_device::write));     // YM2203
	map(0x52, 0x53).r("ymsnd", FUNC(ym2203_device::read));      // YM2203
	map(0x70, 0x70).w("oki1", FUNC(okim6295_device::write));    // M6295
	map(0x74, 0x74).w(FUNC(cave_state::oki1_bank_w<0x3>));      // Samples bank
}


/***************************************************************************
                                Metamoqester
***************************************************************************/

void cave_state::metmqstr_sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();             // ROM
	map(0x4000, 0x7fff).bankr("z80bank");  // ROM (Banked)
	map(0xe000, 0xffff).ram();             // RAM
}

void cave_state::metmqstr_sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(cave_state::z80_rombank_w<0x0f>));                             // Rom Bank
	map(0x20, 0x20).r(FUNC(cave_state::soundflags_r));                                    // Communication
	map(0x30, 0x30).r(FUNC(cave_state::soundlatch_lo_r));                                 // From Main CPU
	map(0x40, 0x40).r(FUNC(cave_state::soundlatch_hi_r));                                 // ""
	map(0x50, 0x51).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));   // YM2151
	map(0x60, 0x60).w("oki1", FUNC(okim6295_device::write));                              // M6295 #0
	map(0x70, 0x70).w(FUNC(cave_state::oki1_bank_w<0x7>));                                // Samples Bank #0
	map(0x80, 0x80).w("oki2", FUNC(okim6295_device::write));                              // M6295 #1
	map(0x90, 0x90).w(FUNC(cave_state::oki2_bank_w<0x7>));                                // Samples Bank #1
}


/***************************************************************************
                                Power Instinct 2
***************************************************************************/

void cave_state::pwrinst2_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();             // ROM
	map(0x8000, 0xbfff).bankr("z80bank");  // ROM (Banked)
	map(0xe000, 0xffff).ram();             // RAM
}

void cave_state::pwrinst2_sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));  // M6295 #0
	map(0x08, 0x08).rw("oki2", FUNC(okim6295_device::read), FUNC(okim6295_device::write));  // M6295 #1
	map(0x10, 0x17).w("nmk112", FUNC(nmk112_device::okibank_w));                            // Samples bank
	map(0x40, 0x41).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));     // YM2203
	map(0x50, 0x50).w(FUNC(cave_state::soundlatch_ack_w));                                  // To Main CPU
//  map(0x51, 0x51).nopw();                                                                 // ?? volume
	map(0x60, 0x60).r(FUNC(cave_state::soundlatch_hi_r));                                   // From Main CPU
	map(0x70, 0x70).r(FUNC(cave_state::soundlatch_lo_r));                                   // ""
	map(0x80, 0x80).w(FUNC(cave_state::z80_rombank_w<0x07>));                               // ROM bank
}


/***************************************************************************
                                Sailor Moon
***************************************************************************/

void cave_state::sailormn_sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();                                 // ROM
	map(0x4000, 0x7fff).bankr("z80bank");                      // ROM (Banked)
	map(0xc000, 0xdfff).mirror(0x2000).ram();                  // RAM (8KB, mirrored)
}

void cave_state::sailormn_sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(cave_state::z80_rombank_w<0x1f>));                               // Rom Bank
	map(0x10, 0x10).w(FUNC(cave_state::soundlatch_ack_w));                                  // To Main CPU
	map(0x20, 0x20).r(FUNC(cave_state::soundflags_r));                                      // Communication
	map(0x30, 0x30).r(FUNC(cave_state::soundlatch_lo_r));                                   // From Main CPU
	map(0x40, 0x40).r(FUNC(cave_state::soundlatch_hi_r));                                   // ""
	map(0x50, 0x51).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));     // YM2151
	map(0x60, 0x60).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));  // M6295 #0
	map(0x70, 0x70).w(FUNC(cave_state::oki1_bank_w<0xf>));                                  // Samples Bank #0
	map(0x80, 0x80).rw("oki2", FUNC(okim6295_device::read), FUNC(okim6295_device::write));  // M6295 #1
	map(0xc0, 0xc0).w(FUNC(cave_state::oki2_bank_w<0xf>));                                  // Samples Bank #1
}



/***************************************************************************


                                Input Ports


***************************************************************************/

/*
    dfeveron config menu:
    101624.w -> 8,a6    preferences
    101626.w -> c,a6    (1:coin<<4|credit) <<8 | (2:coin<<4|credit)
*/

/* Most games use this */
static INPUT_PORTS_START( cave )
	PORT_START("IN0")   // Player 1
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(6)
	PORT_SERVICE_NO_TOGGLE( 0x0200, IP_ACTIVE_LOW )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )  // sw? exit service mode
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )  // sw? enter & exit service mode
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")   // Player 2
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(6)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0xf400, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/* Gaia Crusaders, no EEPROM. Has DIPS */
static INPUT_PORTS_START( gaia )
	PORT_INCLUDE( cave )

	PORT_MODIFY("IN0")  // Player 1 + 2
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_MODIFY("IN1")  // Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(6)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(6)
	PORT_SERVICE_NO_TOGGLE( 0x0004, IP_ACTIVE_LOW )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0fc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Language ) )         PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, "2 Coins/1 Credit (1 to continue)" )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, "150k/300k" ) PORT_CONDITION("DSW", 0xe000, EQUALS, 0xc000)
	PORT_DIPSETTING(      0x0400, "150k/350k" ) PORT_CONDITION("DSW", 0xe000, EQUALS, 0xa000)
	PORT_DIPSETTING(      0x0400, "150k/350k" ) PORT_CONDITION("DSW", 0xe000, EQUALS, 0xe000)
	PORT_DIPSETTING(      0x0400, "150k/400k" ) PORT_CONDITION("DSW", 0xe000, EQUALS, 0x6000)
	PORT_DIPSETTING(      0x0400, "150k/400k" ) PORT_CONDITION("DSW", 0xe000, EQUALS, 0x8000)
	PORT_DIPSETTING(      0x0400, "150k/400k" ) PORT_CONDITION("DSW", 0xe000, EQUALS, 0x2000)
	PORT_DIPSETTING(      0x0400, "200k/500k" ) PORT_CONDITION("DSW", 0xe000, EQUALS, 0x4000)
	PORT_DIPSETTING(      0x0400, "200k/500k" ) PORT_CONDITION("DSW", 0xe000, EQUALS, 0x0000)
	PORT_DIPNAME( 0x1800, 0x1800, "Damage" )                    PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x1800, "+0" )
	PORT_DIPSETTING(      0x1000, "+1" )
	PORT_DIPSETTING(      0x0800, "+2" )
	PORT_DIPSETTING(      0x0000, "+3" )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:6,7,8")
	PORT_DIPSETTING(      0xc000, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( Medium_Hard ) )
	PORT_DIPSETTING(      0x8000, "Hard 1" )
	PORT_DIPSETTING(      0x2000, "Hard 2" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( theroes )
	PORT_INCLUDE( gaia )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Language ) )         PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Chinese ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, "150k/300k" ) PORT_CONDITION("DSW", 0xc000, EQUALS, 0x8000)
	PORT_DIPSETTING(      0x0400, "150k/350k" ) PORT_CONDITION("DSW", 0xc000, EQUALS, 0xc000)
	PORT_DIPSETTING(      0x0400, "150k/400k" ) PORT_CONDITION("DSW", 0xc000, EQUALS, 0x4000)
	PORT_DIPSETTING(      0x0400, "200k/500k" ) PORT_CONDITION("DSW", 0xc000, EQUALS, 0x0000)
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Medium_Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
INPUT_PORTS_END


/* Normal layout but with 4 buttons */
static INPUT_PORTS_START( metmqstr )
	PORT_INCLUDE( cave )

	PORT_MODIFY("IN0")  // Player 1
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_MODIFY("IN1")  // Player 2
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
INPUT_PORTS_END

/* Different layout */
static INPUT_PORTS_START( guwange )
	PORT_START("IN0")   // Player 1 & 2
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("IN1")   // Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(6)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(6)
	PORT_SERVICE_NO_TOGGLE( 0x0004, IP_ACTIVE_LOW )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0xff70, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( korokoro )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(10)   // bit 0x0010 of leds (coin)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(10)   // bit 0x0020 of leds (does coin sound)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(10)   // bit 0x0080 of leds
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON1 )  // round  button (choose)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 )  // square button (select in service mode / medal out in game)
	PORT_BIT( 0x0fe0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE2 ) // service medal out?
	PORT_SERVICE( 0x2000, IP_ACTIVE_LOW )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 ) // service coin
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(cave_state, korokoro_hopper_r) // motor / hopper status ???

	PORT_START("IN1")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0xefff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( tekkencw )
	PORT_START("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 ) PORT_IMPULSE(10) // credits (impulse needed to coin up reliably)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME( DEF_STR( Yes ) ) PORT_CODE(KEYCODE_Y)    // suru ("do")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_NAME( "Bet" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_MEMBER(cave_state, tjumpman_hopper_r)

	PORT_START("IN1")
	PORT_CONFNAME( 0x08, 0x08, "Self Test" )
	PORT_CONFSETTING(    0x08, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2  ) PORT_NAME( DEF_STR( No ) ) PORT_CODE(KEYCODE_N)    // shinai ("not")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Action" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_IMPULSE(10)                                   // medal (impulse needed to coin up reliably)
	PORT_BIT( 0x87, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( tekkenbs )
	PORT_START("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 ) PORT_IMPULSE(10) // credits (impulse needed to coin up reliably)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_NAME( "Bet" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_MEMBER(cave_state, tjumpman_hopper_r)

	PORT_START("IN1")
	PORT_CONFNAME( 0x08, 0x08, "Self Test" )
	PORT_CONFSETTING(    0x08, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Start" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_IMPULSE(10) // medal (impulse needed to coin up reliably)
	PORT_BIT( 0x87, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( tjumpman )
	PORT_START("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x06, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER   ) PORT_NAME( DEF_STR( Yes ) ) PORT_CODE(KEYCODE_Y)    // suru ("do")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_NAME( "1 Bet" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_MEMBER(cave_state, tjumpman_hopper_r)

	PORT_START("IN1")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_CONFNAME( 0x08, 0x08, "Self Test" )
	PORT_CONFSETTING(    0x08, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME( DEF_STR( No ) ) PORT_CODE(KEYCODE_N)    // shinai ("not")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Go" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_IMPULSE(10)                                   // medal (impulse needed to coin up reliably)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "3 Bet" )
INPUT_PORTS_END


static INPUT_PORTS_START( pacslot )
	PORT_START("IN0")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW ) // must stay on during service mode
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 ) PORT_IMPULSE(10) // credits (impulse needed to coin up reliably)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER   ) PORT_NAME( "Pac-Man" ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_NAME( "Bet" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_READ_LINE_MEMBER(cave_state, tjumpman_hopper_r)

	PORT_START("IN1")
	PORT_CONFNAME( 0x08, 0x08, "Self Test" )
	PORT_CONFSETTING(    0x08, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME( "Ms. Pac-Man" ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_IMPULSE(10) // medal (impulse needed to coin up reliably)
	PORT_BIT( 0x87, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( paceight )
	PORT_INCLUDE( pacslot )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER   ) PORT_NAME( "Left" ) PORT_CODE(KEYCODE_Y)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME( "Right" ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Max Bet" )
INPUT_PORTS_END

static INPUT_PORTS_START( paccarn )
	PORT_START("IN0")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(10) // credits (impulse needed to coin up reliably)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(cave_state, paccarn_bet4_r)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Bet 2" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(cave_state, tjumpman_hopper_r)

	PORT_START("IN1")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_CONFNAME( 0x08, 0x08, "Self Test" )
	PORT_CONFSETTING(    0x08, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(cave_state, paccarn_bet8_r)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Bet 3" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(10) // medal (impulse needed to coin up reliably)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	// holding together Bet 4 and Bet 8 activates Bet 12 in IO Test Mode
	PORT_START("BET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME( "Bet 4" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME( "Bet 8" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME( "Bet 12" )
INPUT_PORTS_END

static INPUT_PORTS_START( ppsatan )
	PORT_START("SYSTEM")   // $200000
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1    )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 ) // service coin
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 ) // advance in service mode
	PORT_BIT( 0x0072, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0300, "1 Coin/1 1P-Game (2 Coins/1 2P-Game)" )
	PORT_DIPSETTING(      0x0100, "2 Coins/1 1P-Game (3 Coins/1 2P-Game)" )
	PORT_DIPSETTING(      0x0200, "2 Coins/1 1P-Game (4 Coins/1 2P-Game)" )
	PORT_DIPSETTING(      0x0000, "2 Coins/1 1P-Game (4 Coins/1 2P-Game) (duplicate)" )
	PORT_DIPUNKNOWN(0x0400, 0x0400)                             PORT_DIPLOCATION("SW1:3")
	PORT_DIPUNKNOWN(0x0800, 0x0800)                             PORT_DIPLOCATION("SW1:4")
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )    // 15 hits
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )  // 20 hits
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )    // 25 hits
	PORT_DIPSETTING(      0x2000, "Hard (duplicate)" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:7")   // Jingle after "warning" screen (every 3 demo loops)
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_SERVICE(0x8000, IP_ACTIVE_LOW)                         PORT_DIPLOCATION("SW1:8")

	PORT_START("TOUCH1_X")
	PORT_BIT( 0x7fff, 0x20, IPT_LIGHTGUN_X ) PORT_PLAYER(1) PORT_MINMAX(0x000, 0x140-1) PORT_CROSSHAIR(X, 284.0/320.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(8)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_START("TOUCH1_Y")
	PORT_BIT( 0xffff, 0x18, IPT_LIGHTGUN_Y ) PORT_PLAYER(1) PORT_MINMAX(0x000,  0xe0-1) PORT_CROSSHAIR(Y, 188.0/224.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(8)

	PORT_START("TOUCH2_X")
	PORT_BIT( 0x7fff, 0x20, IPT_LIGHTGUN_X ) PORT_PLAYER(2) PORT_MINMAX(0x000, 0x140-1) PORT_CROSSHAIR(X, 284.0/320.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(8)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_START("TOUCH2_Y")
	PORT_BIT( 0xffff, 0x18, IPT_LIGHTGUN_Y ) PORT_PLAYER(2) PORT_MINMAX(0x000,  0xe0-1) PORT_CROSSHAIR(Y, 188.0/224.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(8)
INPUT_PORTS_END


/***************************************************************************


                            Graphics Layouts


***************************************************************************/

// 6bpp tiles are accessible only 0x400 colors

// 8x8x6 tiles (in a 8x8x8 layout)
static const gfx_layout layout_8x8x6 =
{
	8,8,
	RGN_FRAC(1,1),
	6,
	{STEP2(8,1),STEP4(0,1)},
	{0*4,1*4,4*4,5*4,8*4,9*4,12*4,13*4},
	{STEP8(0,8*8)},
	8*8*8
};

// 8x8x6 tiles (4 bits in one rom, 2 bits in the other,
// unpacked in 2 pages of 4 bits)
static const gfx_layout layout_8x8x6_2 =
{
	8,8,
	RGN_FRAC(1,2),
	6,
	{RGN_FRAC(1,2)+2,RGN_FRAC(1,2)+3, STEP4(0,1)},
	{STEP8(0,4)},
	{STEP8(0,4*8)},
	8*8*4
};

// 8x8x8 tiles
static const gfx_layout layout_8x8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{STEP4(8,1),STEP4(0,1)},
	{0*4,1*4,4*4,5*4,8*4,9*4,12*4,13*4},
	{STEP8(0,8*8)},
	8*8*8
};

// 16~x16~x4 Zooming Sprites - decode each 16 pixel lines
static const gfx_layout layout_sprites =
{
	16,1,
	RGN_FRAC(1,1),
	4,
	{STEP4(0,1)},
	{1*4,0*4,3*4,2*4,5*4,4*4,7*4,6*4,9*4,8*4,11*4,10*4,13*4,12*4,15*4,14*4},
	{0},
	16*4
};

// 16~x16~x4 Non-zooming Sprites - decode each 16 pixel lines
static const gfx_layout layout_sprites_msb =
{
	16,1,
	RGN_FRAC(1,1),
	4,
	{STEP4(0,1)},
	{STEP16(0,4)},
	{0},
	16*4
};

// esprade, guwange uses 8bpp sprites instead 4bpp
static const gfx_layout layout_sprites_8bpp =
{
	16,1,
	RGN_FRAC(1,1),
	8,
	{STEP4(0,1), STEP4(4*4,1)},
	{STEP4(4*4*2*0+3*4,-4),STEP4(4*4*2*1+3*4,-4),STEP4(4*4*2*2+3*4,-4),STEP4(4*4*2*3+3*4,-4)},
	{0},
	16*8
};

static GFXDECODE_START( gfx_common_spr )
	GFXDECODE_ENTRY( "sprites0", 0, layout_sprites, 0x0000, 0x40 )
GFXDECODE_END

/***************************************************************************
                                Dangun Feveron
***************************************************************************/

static GFXDECODE_START( gfx_dfeveron )
	GFXDECODE_ENTRY( "layer0", 0, gfx_8x8x4_packed_msb, 0x0400, 0x40 ) // [0] Layer 0
	GFXDECODE_ENTRY( "layer1", 0, gfx_8x8x4_packed_msb, 0x0400, 0x40 ) // [1] Layer 1
GFXDECODE_END

/***************************************************************************
                                Dodonpachi
***************************************************************************/

// different gfx layout
static GFXDECODE_START( gfx_ddonpach_spr )
	GFXDECODE_ENTRY( "sprites0", 0, layout_sprites_msb, 0x0000, 0x40 )
GFXDECODE_END

static GFXDECODE_START( gfx_ddonpach )
	GFXDECODE_ENTRY( "layer0", 0, gfx_8x8x4_packed_msb, 0x4000, 0x40 ) // [0] Layer 0
	GFXDECODE_ENTRY( "layer1", 0, gfx_8x8x4_packed_msb, 0x4000, 0x40 ) // [1] Layer 1
	GFXDECODE_ENTRY( "layer2", 0, layout_8x8x8,         0x4000, 0x40 ) // [2] Layer 2
GFXDECODE_END

/***************************************************************************
                                Donpachi
***************************************************************************/

static GFXDECODE_START( gfx_donpachi )
	GFXDECODE_ENTRY( "layer0", 0, gfx_8x8x4_packed_msb, 0x0400, 0x40 ) // [0] Layer 0
	GFXDECODE_ENTRY( "layer1", 0, gfx_8x8x4_packed_msb, 0x0400, 0x40 ) // [1] Layer 1
	GFXDECODE_ENTRY( "layer2", 0, gfx_8x8x4_packed_msb, 0x0400, 0x40 ) // [2] Layer 2
GFXDECODE_END

/***************************************************************************
                                Esprade
***************************************************************************/

static GFXDECODE_START( gfx_esprade_spr )
	GFXDECODE_ENTRY( "sprites0", 0, layout_sprites_8bpp, 0x0000, 0x40 )
GFXDECODE_END

static GFXDECODE_START( gfx_esprade )
	GFXDECODE_ENTRY( "layer0", 0, layout_8x8x8, 0x4000, 0x40 ) // [0] Layer 0
	GFXDECODE_ENTRY( "layer1", 0, layout_8x8x8, 0x4000, 0x40 ) // [1] Layer 1
	GFXDECODE_ENTRY( "layer2", 0, layout_8x8x8, 0x4000, 0x40 ) // [2] Layer 2
GFXDECODE_END

/***************************************************************************
                                Hotdog Storm
***************************************************************************/

static GFXDECODE_START( gfx_hotdogst )
	GFXDECODE_ENTRY( "layer0", 0, gfx_8x8x4_packed_msb, 0x0000, 0x40 ) // [0] Layer 0
	GFXDECODE_ENTRY( "layer1", 0, gfx_8x8x4_packed_msb, 0x0000, 0x40 ) // [1] Layer 1
	GFXDECODE_ENTRY( "layer2", 0, gfx_8x8x4_packed_msb, 0x0000, 0x40 ) // [2] Layer 2
GFXDECODE_END

/***************************************************************************
                                Koro Koro Quest
***************************************************************************/

// different sprite base palette
static GFXDECODE_START( gfx_korokoro_spr )
	GFXDECODE_ENTRY( "sprites0", 0, layout_sprites, 0x3c00, 0x40 )
GFXDECODE_END

static GFXDECODE_START( gfx_korokoro )
	GFXDECODE_ENTRY( "layer0", 0, gfx_8x8x4_packed_msb, 0x0400, 0x40 ) // [0] Layer 0
GFXDECODE_END

/***************************************************************************
                                Mazinger Z
***************************************************************************/

static GFXDECODE_START( gfx_mazinger )
	GFXDECODE_ENTRY( "layer0", 0, gfx_8x8x4_packed_msb, 0x0000, 0x40 ) // [0] Layer 0
	GFXDECODE_ENTRY( "layer1", 0, layout_8x8x6,         0x0400, 0x10 ) // [1] Layer 1
GFXDECODE_END

/***************************************************************************
                               Poka Poka Satan
***************************************************************************/

static GFXDECODE_START( gfx_ppsatan_0 )
	GFXDECODE_ENTRY( "layer0", 0, gfx_8x8x4_packed_msb, 0x0000, 0x40 ) // [0] Layer 0
GFXDECODE_END

static GFXDECODE_START( gfx_ppsatan_spr_1 )
	GFXDECODE_ENTRY( "sprites1", 0, layout_sprites, 0x3c00, 0x40 )
GFXDECODE_END

static GFXDECODE_START( gfx_ppsatan_1 )
	GFXDECODE_ENTRY( "layer1", 0, gfx_8x8x4_packed_msb, 0x0000, 0x40 ) // [1] Layer 1
GFXDECODE_END

static GFXDECODE_START( gfx_ppsatan_spr_2 )
	GFXDECODE_ENTRY( "sprites2", 0, layout_sprites, 0x3c00, 0x40 )
GFXDECODE_END

static GFXDECODE_START( gfx_ppsatan_2 )
	GFXDECODE_ENTRY( "layer2", 0, gfx_8x8x4_packed_msb, 0x0000, 0x40 ) // [2] Layer 2
GFXDECODE_END

/***************************************************************************
                                Power Instinct 2
***************************************************************************/

// expanded sprite color space
static GFXDECODE_START( gfx_pwrinst2_spr )
	GFXDECODE_ENTRY( "sprites0", 0, layout_sprites_msb, 0x0000, 0x80 )
GFXDECODE_END

static GFXDECODE_START( gfx_pwrinst2 )
	GFXDECODE_ENTRY( "layer0", 0, gfx_8x8x4_packed_msb, 0x0800, 0x40 ) // [0] Layer 0
	GFXDECODE_ENTRY( "layer1", 0, gfx_8x8x4_packed_msb, 0x1000, 0x40 ) // [1] Layer 1
	GFXDECODE_ENTRY( "layer2", 0, gfx_8x8x4_packed_msb, 0x1800, 0x40 ) // [2] Layer 2
	GFXDECODE_ENTRY( "layer3", 0, gfx_8x8x4_packed_msb, 0x2000, 0x40 ) // [3] Layer 3
GFXDECODE_END


/***************************************************************************
                                Sailor Moon
***************************************************************************/

static GFXDECODE_START( gfx_sailormn )
	GFXDECODE_ENTRY( "layer0", 0, gfx_8x8x4_packed_msb, 0x0400, 0x40 ) // [0] Layer 0
	GFXDECODE_ENTRY( "layer1", 0, gfx_8x8x4_packed_msb, 0x0800, 0x40 ) // [1] Layer 1
	GFXDECODE_ENTRY( "layer2", 0, layout_8x8x6_2,       0x0c00, 0x10 ) // [2] Layer 2
GFXDECODE_END


/***************************************************************************
                                Uo Poko
***************************************************************************/

static GFXDECODE_START( gfx_uopoko )
	GFXDECODE_ENTRY( "layer0", 0, layout_8x8x8, 0x4000, 0x40 ) // [0] Layer 0
GFXDECODE_END


/***************************************************************************


                                Machine Drivers


***************************************************************************/

void cave_state::machine_start()
{
	m_led_outputs.resolve();
	m_vblank_end_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(cave_state::vblank_end), this));

	save_item(NAME(m_soundbuf_wptr));
	save_item(NAME(m_soundbuf_rptr));
	save_item(NAME(m_soundbuf_data));
	save_item(NAME(m_soundbuf_empty));

	save_item(NAME(m_vblank_irq));
	save_item(NAME(m_sound_irq));
	save_item(NAME(m_unknown_irq));
	save_item(NAME(m_agallet_vblank_irq));
}

void cave_state::machine_reset()
{
	std::fill(std::begin(m_soundbuf_data), std::end(m_soundbuf_data), 0);
	m_soundbuf_wptr = 0;
	m_soundbuf_rptr = 0;
	m_soundbuf_empty = true;

	m_vblank_irq = 0;
	m_sound_irq = 0;
	m_unknown_irq = 0;
	m_agallet_vblank_irq = 0;
}

void cave_state::add_base_config(machine_config &config, int layer)
{
	M68000(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_vblank_int("screen.0", FUNC(cave_state::interrupt));

	TIMER(config, m_int_timer).configure_generic(FUNC(cave_state::vblank_start));

	SCREEN(config, m_screen[0], SCREEN_TYPE_RASTER);
	m_screen[0]->set_refresh_hz(15625/271.5);
	m_screen[0]->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen[0]->set_size(320, 240);
	m_screen[0]->set_visarea(0, 320-1, 0, 240-1);
	m_screen[0]->set_screen_update(FUNC(cave_state::screen_update));

	PALETTE(config, m_palette[0], palette_device::BLACK).set_format(palette_device::xGRB_555, 0x8000);

	GFXDECODE(config, m_spr_gfxdecode[0], m_palette[0], gfx_common_spr);

	for (int i = 0; i < layer; i++)
	{
		TMAP038(config, m_tilemap[i]);
		m_tilemap[i]->set_gfxdecode_tag(m_gfxdecode[0]);
		m_tilemap[i]->set_gfx(i);
	}
}

void cave_state::add_ymz(machine_config &config)
{
	// TODO: all PCB versions using mono, on a YMZ chip as well? Sounds very unlikely, verify on all flavours.
	SPEAKER(config, "mono").front_center();

	ymz280b_device &ymz(YMZ280B(config, "ymz", 16.9344_MHz_XTAL));
	ymz.irq_handler().set(FUNC(cave_state::sound_irq_gen));
	ymz.add_route(ALL_OUTPUTS, "mono", 1.0);
}

/***************************************************************************
                                Dangun Feveron
***************************************************************************/

void cave_state::dfeveron(machine_config &config)
{
	add_base_config(config, 2);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &cave_state::dfeveron_map);

	EEPROM_93C46_16BIT(config, m_eeprom);

	/* video hardware */
	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_dfeveron);
	m_palette[0]->set_entries(0x1000/2);

	MCFG_VIDEO_START_OVERRIDE(cave_state,spr_4bpp)

	/* sound hardware */
	add_ymz(config);
}


/***************************************************************************
                                Dodonpachi
***************************************************************************/

void cave_state::ddonpach(machine_config &config)
{
	add_base_config(config, 3);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &cave_state::ddonpach_map);

	EEPROM_93C46_16BIT(config, m_eeprom);

	/* video hardware */
	m_spr_gfxdecode[0]->set_info(gfx_ddonpach_spr);
	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_ddonpach);

	MCFG_VIDEO_START_OVERRIDE(cave_state,spr_8bpp)

	/* sound hardware */
	add_ymz(config);
}


/***************************************************************************
                                    Donpachi
***************************************************************************/

void cave_state::donpachi(machine_config &config)
{
	add_base_config(config, 3);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &cave_state::donpachi_map);

	EEPROM_93C46_16BIT(config, m_eeprom);

	/* video hardware */
	m_spr_gfxdecode[0]->set_info(gfx_ddonpach_spr);
	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_donpachi);
	m_palette[0]->set_entries(0x1000/2);

	MCFG_VIDEO_START_OVERRIDE(cave_state,spr_4bpp)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki[0], 4.224_MHz_XTAL/4, okim6295_device::PIN7_HIGH); // pin 7 not verified
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 1.60);

	OKIM6295(config, m_oki[1], 4.224_MHz_XTAL/2, okim6295_device::PIN7_HIGH); // pin 7 not verified
	m_oki[1]->add_route(ALL_OUTPUTS, "mono", 1.0);

	nmk112_device &nmk112(NMK112(config, "nmk112", 0));
	nmk112.set_rom0_tag("oki1");
	nmk112.set_rom1_tag("oki2");
	nmk112.set_page_mask(1 << 0);    // chip #0 (music) is not paged
}


/***************************************************************************
                                Esprade
***************************************************************************/

void cave_state::esprade(machine_config &config)
{
	add_base_config(config, 3);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &cave_state::esprade_map);

	EEPROM_93C46_16BIT(config, m_eeprom);

	/* video hardware */
	m_spr_gfxdecode[0]->set_info(gfx_esprade_spr);
	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_esprade);

	MCFG_VIDEO_START_OVERRIDE(cave_state,spr_8bpp)

	/* sound hardware */
	add_ymz(config);
}


/***************************************************************************
                                    Gaia Crusaders
***************************************************************************/

void cave_state::gaia(machine_config &config)
{
	add_base_config(config, 3);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &cave_state::gaia_map);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	m_screen[0]->set_visarea(0, 320-1, 0, 224-1);

	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_esprade);

	MCFG_VIDEO_START_OVERRIDE(cave_state,spr_8bpp)

	/* sound hardware */
	add_ymz(config);
}


/***************************************************************************
                                    Guwange
***************************************************************************/

void cave_state::guwange(machine_config &config)
{
	add_base_config(config, 3);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &cave_state::guwange_map);

	EEPROM_93C46_16BIT(config, m_eeprom);

	/* video hardware */
	m_spr_gfxdecode[0]->set_info(gfx_esprade_spr);
	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_esprade);

	MCFG_VIDEO_START_OVERRIDE(cave_state,spr_8bpp)

	/* sound hardware */
	add_ymz(config);
}

/***************************************************************************
                                Hotdog Storm
***************************************************************************/

void cave_state::hotdogst(machine_config &config)
{
	add_base_config(config, 3);

	/* basic machine hardware */
	m_maincpu->set_clock(32_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cave_state::hotdogst_map);

	Z80(config, m_audiocpu, 32_MHz_XTAL/8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &cave_state::hotdogst_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &cave_state::hotdogst_sound_portmap);

	EEPROM_93C46_16BIT(config, m_eeprom);

	/* video hardware */
	m_screen[0]->set_size(384, 240);
	m_screen[0]->set_visarea(0, 384-1, 0, 240-1);

	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_hotdogst);
	m_palette[0]->set_entries(0x1000/2);

	MCFG_VIDEO_START_OVERRIDE(cave_state,spr_4bpp)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_16(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2203_device &ymsnd(YM2203(config, "ymsnd", 32_MHz_XTAL/8));
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(0, "mono", 0.20);
	ymsnd.add_route(1, "mono", 0.20);
	ymsnd.add_route(2, "mono", 0.20);
	ymsnd.add_route(3, "mono", 0.80);

	OKIM6295(config, m_oki[0], 32_MHz_XTAL/16, okim6295_device::PIN7_HIGH); // pin 7 not verified
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 1.0);
	m_oki[0]->set_addrmap(0, &cave_state::oki_map);
}


/***************************************************************************
                               Koro Koro Quest
***************************************************************************/

void cave_state::korokoro(machine_config &config)
{
	add_base_config(config, 1);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &cave_state::korokoro_map);

	EEPROM_93C46_16BIT(config, m_eeprom);

	/* video hardware */
	m_screen[0]->set_visarea(0, 320-1-2, 0, 240-1-1);

	m_spr_gfxdecode[0]->set_info(gfx_korokoro_spr);
	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_korokoro);
	m_palette[0]->set_entries(0x8000/2);

	MCFG_VIDEO_START_OVERRIDE(cave_state,spr_4bpp)

	/* sound hardware */
	add_ymz(config);
}

void cave_state::crusherm(machine_config &config)
{
	korokoro(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &cave_state::crusherm_map);
}


/***************************************************************************
                                Mazinger Z
***************************************************************************/

void cave_state::mazinger(machine_config &config)
{
	add_base_config(config, 2);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &cave_state::mazinger_map);

	Z80(config, m_audiocpu, 4_MHz_XTAL); // Bidirectional communication
	m_audiocpu->set_addrmap(AS_PROGRAM, &cave_state::mazinger_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &cave_state::mazinger_sound_portmap);

	WATCHDOG_TIMER(config, "watchdog").set_time(attotime::from_seconds(3));  /* a guess, and certainly wrong */

	EEPROM_93C46_16BIT(config, m_eeprom);

	/* video hardware */
	m_screen[0]->set_size(384, 240);
	m_screen[0]->set_visarea(0, 384-1, 0, 240-1);

	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_mazinger);
	m_palette[0]->set_entries(0x8000/2);

	MCFG_VIDEO_START_OVERRIDE(cave_state,spr_4bpp)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_16(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2203_device &ymsnd(YM2203(config, "ymsnd", 4_MHz_XTAL));
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(0, "mono", 0.20);
	ymsnd.add_route(1, "mono", 0.20);
	ymsnd.add_route(2, "mono", 0.20);
	ymsnd.add_route(3, "mono", 0.60);

	OKIM6295(config, m_oki[0], 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 2.0);
	m_oki[0]->set_addrmap(0, &cave_state::oki_map);
}


/***************************************************************************
                                Metamoqester
***************************************************************************/

void cave_state::metmqstr(machine_config &config)
{
	add_base_config(config, 3);

	/* basic machine hardware */
	m_maincpu->set_clock(32_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cave_state::metmqstr_map);

	Z80(config, m_audiocpu, 32_MHz_XTAL / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &cave_state::metmqstr_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &cave_state::metmqstr_sound_portmap);

	WATCHDOG_TIMER(config, "watchdog").set_time(attotime::from_seconds(3));  /* a guess, and certainly wrong */

	EEPROM_93C46_16BIT(config, m_eeprom);

	/* video hardware */
	m_screen[0]->set_size(0x200, 240);
	m_screen[0]->set_visarea(0x7d, 0x7d + 0x180-1, 0, 240-1);

	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_donpachi);
	m_palette[0]->set_entries(0x1000/2);

	MCFG_VIDEO_START_OVERRIDE(cave_state,spr_4bpp)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_16(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 16_MHz_XTAL / 4));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.6);

	OKIM6295(config, m_oki[0], 32_MHz_XTAL / 16, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.5);
	m_oki[0]->set_addrmap(0, &cave_state::oki_map);

	OKIM6295(config, m_oki[1], 32_MHz_XTAL / 16, okim6295_device::PIN7_HIGH);
	m_oki[1]->add_route(ALL_OUTPUTS, "mono", 0.5);
	m_oki[1]->set_addrmap(0, &cave_state::oki2_map);
}


/***************************************************************************
                                   Pac-Slot
***************************************************************************/

void cave_state::pacslot(machine_config &config)
{
	add_base_config(config, 1);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* basic machine hardware */
	m_maincpu->set_clock(28_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cave_state::pacslot_map);

	WATCHDOG_TIMER(config, "watchdog").set_time(attotime::from_seconds(3));  /* a guess, and certainly wrong */

	EEPROM_93C46_16BIT(config, m_eeprom, eeprom_serial_streaming::ENABLE);

	/* video hardware */
	m_screen[0]->set_size(0x200, 240);
	m_screen[0]->set_visarea(0x80, 0x80 + 0x140-1, 0, 240-1);

	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_uopoko);

	MCFG_VIDEO_START_OVERRIDE(cave_state,spr_8bpp)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki[0], 28_MHz_XTAL / 28, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 1.0);

	// oki2 chip is present but its rom socket is unpopulated
	OKIM6295(config, m_oki[1], 28_MHz_XTAL / 28, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki[1]->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void cave_state::paceight(machine_config &config)
{
	pacslot(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &cave_state::paceight_map);
}

void cave_state::paccarn(machine_config &config)
{
	pacslot(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &cave_state::paccarn_map);
}

/***************************************************************************
                               Poka Poka Satan
***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER( cave_state::timer_lev2_cb )
{
	m_maincpu->set_input_line(M68K_IRQ_2, HOLD_LINE);   // ppsatan: read touch screens
}

void cave_state::ppsatan(machine_config &config)
{
	add_base_config(config, 1);

	/* basic machine hardware */
	m_maincpu->set_vblank_int("screen.0", FUNC(cave_state::interrupt_ppsatan));
	m_maincpu->set_addrmap(AS_PROGRAM, &cave_state::ppsatan_map);

	WATCHDOG_TIMER(config, "watchdog").set_time(attotime::from_seconds(1));  /* a guess, and certainly wrong */

	EEPROM_93C46_16BIT(config, m_eeprom);

	TIMER(config, "timer_lev2").configure_periodic(FUNC(cave_state::timer_lev2_cb), attotime::from_hz(60));

	/* video hardware */
	m_screen[0]->set_visarea(0, 320-1, 0, 224-1);
	m_screen[0]->set_screen_update(FUNC(cave_state::screen_update_ppsatan_top));
	subdevice<timer_device>("int_timer")->configure_generic(FUNC(cave_state::vblank_start));

	SCREEN(config, m_screen[1], SCREEN_TYPE_RASTER);
	m_screen[1]->set_refresh_hz(15625/271.5);
	m_screen[1]->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen[1]->set_size(320, 240);
	m_screen[1]->set_visarea(0, 320-1, 0, 224-1);
	m_screen[1]->set_screen_update(FUNC(cave_state::screen_update_ppsatan_left));
	TIMER(config, "int_timer_left").configure_generic(FUNC(cave_state::vblank_start_left));

	SCREEN(config, m_screen[2], SCREEN_TYPE_RASTER);
	m_screen[2]->set_refresh_hz(15625/271.5);
	m_screen[2]->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen[2]->set_size(320, 240);
	m_screen[2]->set_visarea(0, 320-1, 0, 224-1);
	m_screen[2]->set_screen_update(FUNC(cave_state::screen_update_ppsatan_right));
	TIMER(config, "int_timer_right").configure_generic(FUNC(cave_state::vblank_start_right));

	m_spr_gfxdecode[0]->set_info(gfx_korokoro_spr);
	GFXDECODE(config, m_spr_gfxdecode[1], m_palette[1], gfx_ppsatan_spr_1);
	GFXDECODE(config, m_spr_gfxdecode[2], m_palette[2], gfx_ppsatan_spr_2);
	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_ppsatan_0);
	GFXDECODE(config, m_gfxdecode[1], m_palette[1], gfx_ppsatan_1);
	GFXDECODE(config, m_gfxdecode[2], m_palette[2], gfx_ppsatan_2);

	m_tilemap[0]->set_xoffs(2, 0);

	TMAP038(config, m_tilemap[1]);
	m_tilemap[1]->set_gfxdecode_tag(m_gfxdecode[1]);
	m_tilemap[1]->set_gfx(0);
	m_tilemap[1]->set_xoffs(1, 0);

	TMAP038(config, m_tilemap[2]);
	m_tilemap[2]->set_gfxdecode_tag(m_gfxdecode[2]);
	m_tilemap[2]->set_gfx(0);
	m_tilemap[2]->set_xoffs(0, -57);

	m_palette[0]->set_entries(0x9000/2);
	PALETTE(config, m_palette[1], palette_device::BLACK).set_format(palette_device::xGRB_555, 0x9000/2);
	PALETTE(config, m_palette[2], palette_device::BLACK).set_format(palette_device::xGRB_555, 0x9000/2);

	config.set_default_layout(layout_ppsatan);

	MCFG_VIDEO_START_OVERRIDE(cave_state,ppsatan)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki[0], 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 2.0);
}


/***************************************************************************
                                Power Instinct 2
***************************************************************************/

/*  X1 = 12 MHz, X2 = 28 MHz, X3 = 16 MHz. OKI: / 165 mode A ; / 132 mode B */

void cave_state::pwrinst2(machine_config &config)
{
	add_base_config(config, 4);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &cave_state::pwrinst2_map);

	Z80(config, m_audiocpu, 16_MHz_XTAL / 2);    /* 8 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &cave_state::pwrinst2_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &cave_state::pwrinst2_sound_portmap);

	EEPROM_93C46_16BIT(config, m_eeprom);

	/* video hardware */
	m_screen[0]->set_size(0x200, 240);
	m_screen[0]->set_visarea(0x70, 0x70 + 0x140-1, 0, 240-1);

	m_spr_gfxdecode[0]->set_info(gfx_pwrinst2_spr);
	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_pwrinst2);
	m_palette[0]->set_entries(0x5000/2);

	MCFG_VIDEO_START_OVERRIDE(cave_state,spr_4bpp)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_16(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2203_device &ym2203(YM2203(config, "ymsnd", 16_MHz_XTAL / 4));
	ym2203.irq_handler().set_inputline("audiocpu", 0);
	ym2203.add_route(0, "mono", 0.40);
	ym2203.add_route(1, "mono", 0.40);
	ym2203.add_route(2, "mono", 0.40);
	ym2203.add_route(3, "mono", 0.80);

	OKIM6295(config, m_oki[0], 3_MHz_XTAL, okim6295_device::PIN7_LOW);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.80);

	OKIM6295(config, m_oki[1], 3_MHz_XTAL, okim6295_device::PIN7_LOW);
	m_oki[1]->add_route(ALL_OUTPUTS, "mono", 1.00);

	nmk112_device &nmk112(NMK112(config, "nmk112", 0));
	nmk112.set_rom0_tag("oki1");
	nmk112.set_rom1_tag("oki2");
}


/***************************************************************************
                        Sailor Moon / Air Gallet
***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER( cave_state::sailormn_startup )
{
	m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

MACHINE_RESET_MEMBER(cave_state,sailormn)
{
	machine_reset();
	m_startup->adjust(attotime::from_usec(1000), 0, attotime::zero);
}

void cave_state::sailormn(machine_config &config)
{
	add_base_config(config, 3);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &cave_state::sailormn_map);

	// could be a wachdog, but if it is then our watchdog address is incorrect as there are periods where the game doesn't write it.
	TIMER(config, m_startup).configure_generic(FUNC(cave_state::sailormn_startup));

	Z80(config, m_audiocpu, 8_MHz_XTAL); // Bidirectional Communication
	m_audiocpu->set_addrmap(AS_PROGRAM, &cave_state::sailormn_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &cave_state::sailormn_sound_portmap);

//  config.set_maximum_quantum(attotime::from_hz(600));

	MCFG_MACHINE_RESET_OVERRIDE(cave_state,sailormn)
	EEPROM_93C46_16BIT(config, m_eeprom);

	/* video hardware */
	m_screen[0]->set_size(320+1, 240);
	m_screen[0]->set_visarea(0+1, 320+1-1, 0, 240-1);

	/* Layer 2 (8x8) needs to be handled differently */
	m_tilemap[2]->set_tile_callback(FUNC(cave_state::sailormn_get_banked_code)); /* Layer 2 has 1 banked ROM */

	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_sailormn); // 4 bit sprites, 6 bit tiles
	m_palette[0]->set_entries(0x4000/2);

	MCFG_VIDEO_START_OVERRIDE(cave_state,spr_4bpp)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_16(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 16_MHz_XTAL / 4));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.30);

	OKIM6295(config, m_oki[0], 2112000, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 1.0);
	m_oki[0]->set_addrmap(0, &cave_state::oki_map);

	OKIM6295(config, m_oki[1], 2112000, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki[1]->add_route(ALL_OUTPUTS, "mono", 1.0);
	m_oki[1]->set_addrmap(0, &cave_state::oki2_map);


}


/***************************************************************************
                            Tekken Card World
***************************************************************************/

void cave_state::tekkencw(machine_config &config)
{
	add_base_config(config, 1);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* basic machine hardware */
	m_maincpu->set_clock(28_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cave_state::tekkencw_map);

	WATCHDOG_TIMER(config, "watchdog").set_time(attotime::from_seconds(3));  /* a guess, and certainly wrong */

	EEPROM_93C46_16BIT(config, m_eeprom, eeprom_serial_streaming::ENABLE);

	/* video hardware */
	m_screen[0]->set_size(0x200, 240);
	m_screen[0]->set_visarea(0x80, 0x80 + 0x140-1, 0, 240-1);

	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_uopoko);

	MCFG_VIDEO_START_OVERRIDE(cave_state,spr_8bpp)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki[0], 28_MHz_XTAL / 28, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 1.0);

	// oki2 chip spot and rom socket are both unpopulated
}

void cave_state::tekkenbs(machine_config &config)
{
	tekkencw(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &cave_state::tekkenbs_map);
}


/***************************************************************************
                            Tobikose! Jumpman
***************************************************************************/

void cave_state::tjumpman(machine_config &config)
{
	add_base_config(config, 1);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* basic machine hardware */
	m_maincpu->set_clock(28_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cave_state::tjumpman_map);

	WATCHDOG_TIMER(config, "watchdog").set_time(attotime::from_seconds(3));  /* a guess, and certainly wrong */

	EEPROM_93C46_16BIT(config, m_eeprom, eeprom_serial_streaming::ENABLE);

	/* video hardware */
	m_screen[0]->set_size(0x200, 240);
	m_screen[0]->set_visarea(0x80, 0x80 + 0x140-1, 0, 240-1);

	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_uopoko);

	MCFG_VIDEO_START_OVERRIDE(cave_state,spr_8bpp)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki[0], 28_MHz_XTAL / 28, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 1.0);

	// oki2 chip spot and rom socket are both unpopulated
}


/***************************************************************************
                                Uo Poko
***************************************************************************/

void cave_state::uopoko(machine_config &config)
{
	add_base_config(config, 1);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &cave_state::uopoko_map);

	EEPROM_93C46_16BIT(config, m_eeprom);

	/* video hardware */
	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_uopoko);

	MCFG_VIDEO_START_OVERRIDE(cave_state,spr_8bpp)

	/* sound hardware */
	add_ymz(config);
}


/***************************************************************************


                                ROMs Loading


***************************************************************************/

void cave_state::unpack_sprites(int chip)
{
	gfx_element *gfx = m_spr_gfxdecode[chip]->gfx(0);
	m_sprite_gfx_mask[chip] = 1;
	const u32 needed = gfx->elements() * gfx->height() * gfx->width();
	while (m_sprite_gfx_mask[chip] < needed)
	{
		m_sprite_gfx_mask[chip] <<= 1;
	}
	m_sprite_gfx[chip] = make_unique_clear<u8[]>(m_sprite_gfx_mask[chip]);

	u8 *dst = m_sprite_gfx[chip].get();
	for (int e = 0; e < gfx->elements(); e++)
	{
		const u8 *data = gfx->get_data(e);
		for (int y = 0; y < gfx->height(); y++)
		{
			const u8 *datatmp = data;
			for (int x = 0; x < gfx->width(); x++)
			{
				*dst++ = *datatmp++;
			}
			data += gfx->rowbytes();
		}
	}
	m_sprite_gfx_mask[chip]--;
}


/***************************************************************************

                                Air Gallet

Banpresto
Runs on identical board to Sailor Moon (several sockets unpopulated)

PCB: BP945A (overstamped with BP962A)
CPU: TMP68HC000P16 (68000, 64 pin DIP)
SND: Z84C0008PEC (Z80, 40 pin DIP), OKI M6295 x 2, YM2151, YM3012
OSC: 28.000MHz, 16.000MHz
RAM: 62256 x 8, NEC 424260 x 2, 6264 x 5

Other Chips:
SGS Thomson ST93C46CB1 (EEPROM)
PALS (same as Sailor Moon, not dumped):
      18CV8 label SMBG
      18CV8 label SMZ80
      18CV8 label SMCPU
      GAL16V8 (located near BP962A.U47)

GFX:  038 9437WX711 (176 pin PQFP)
      038 9437WX711 (176 pin PQFP)
      038 9437WX711 (176 pin PQFP)
      013 9346E7002 (240 pin PQFP)

On PCB near JAMMA connector is a small push button to access test mode.

ROMS:
BP962A.U9   27C040      Sound Program
BP962A.U45  27C240      Main Program
BP962A.U47  23C16000    Sound
BP962A.U48  23C16000    Sound
BP962A.U53  23C16000    GFX
BP962A.U54  23C16000    GFX
BP962A.U57  23C16000    GFX
BP962A.U65  23C16000    GFX
BP962A.U76  23C16000    GFX
BP962A.U77  23C16000    GFX

***************************************************************************/

#define ROMS_AGALLET_COMMON \
	ROM_REGION( 0x80000, "audiocpu", 0 ) \
	ROM_LOAD( "bp962a.u9",  0x00000, 0x80000, CRC(06caddbe) SHA1(6a3cc50558ba19a31b21b7f3ec6c6e2846244ff1) ) \
	\
	ROM_REGION( 0x400000, "sprites0", 0 ) \
	ROM_LOAD( "bp962a.u76", 0x000000, 0x200000, CRC(858da439) SHA1(33a3d2a3ec3fa3364b00e1e43b405e5030a5b2a3) ) \
	ROM_LOAD( "bp962a.u77", 0x200000, 0x200000, CRC(ea2ba35e) SHA1(72487f21d44fe7be9a98068ce7f57a43c132945f) ) \
	\
	ROM_REGION( 0x100000, "layer0", 0 ) \
	ROM_LOAD( "bp962a.u53", 0x000000, 0x100000, CRC(fcd9a107) SHA1(169b94db8389e7d47d4d77f36907a62c30fea727) ) \
	ROM_CONTINUE(           0x000000, 0x100000             ) \
	\
	ROM_REGION( 0x200000, "layer1", 0 ) \
	ROM_LOAD( "bp962a.u54", 0x000000, 0x200000, CRC(0cfa3409) SHA1(17107e26762ef7e3b902fb29a6d7bc534a4d09aa) ) \
	\
	ROM_REGION( (1*0x200000)*2, "layer2", 0 )   \
	\
	ROM_LOAD( "bp962a.u57", 0x000000, 0x200000, CRC(6d608957) SHA1(15f6e8346f5f95eb229505b1b4666dabeb810ee8) ) \
	\
	ROM_LOAD( "bp962a.u65", 0x200000, 0x100000, CRC(135fcf9a) SHA1(2e8c89c2627bbdef160d96724d07883fb2fa1a57) ) \
	ROM_CONTINUE(           0x200000, 0x100000             ) \
	\
	ROM_REGION( 0x200000, "oki1", 0 ) \
	ROM_LOAD( "bp962a.u48", 0x000000, 0x200000, CRC(ae00a1ce) SHA1(5e8c74df0ac77efb3080406870856f958be14f79) ) \
	\
	ROM_REGION( 0x200000, "oki2", 0 )   \
	ROM_LOAD( "bp962a.u47", 0x000000, 0x200000, CRC(6d4e9737) SHA1(81c7ecdfc2d38d0b35e26745866f6672f566f936) )


// these roms were dumped from a board set to Taiwanese region.
#define ROMS_AGALLET \
	ROM_REGION( 0x400000, "maincpu", 0 ) \
	ROM_LOAD16_WORD_SWAP( "bp962a.u45", 0x000000, 0x080000, CRC(24815046) SHA1(f5eeae60b923ae850b335e7898a2760407631d8b) ) \
	ROMS_AGALLET_COMMON

/* the regions differ only in the EEPROM, hence the macro above - all EEPROMs are Factory Defaulted */
ROM_START( agallet )
	ROMS_AGALLET

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "agallet_europe.nv", 0x0000, 0x0080, CRC(ec38bf65) SHA1(cb8d9eacc0cf55a0c6b187e6673e3354554314b5) )
ROM_END

ROM_START( agalletu )
	ROMS_AGALLET

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "agallet_usa.nv", 0x0000, 0x0080, CRC(72e65056) SHA1(abf1a86df01064d9d5d8c418e8367817319ec335) )
ROM_END

ROM_START( agalletj )
	ROMS_AGALLET

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "agallet_japan.nv", 0x0000, 0x0080, CRC(0753f547) SHA1(aabb987470406b8729894108bc4d050f7200917d) )
ROM_END

ROM_START( agalletk )
	ROMS_AGALLET

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "agallet_korea.nv", 0x0000, 0x0080, CRC(7f41c253) SHA1(50793d4da0ad6eb590941d26a729a1cf4b3c25c2) )
ROM_END

ROM_START( agallett ) // the dumped board was this region
	ROMS_AGALLET

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "agallet_taiwan.nv", 0x0000, 0x0080, CRC(0af46742) SHA1(37b704c4c573b2aabd6f016e9e8dd458f95148f7) )
ROM_END

ROM_START( agalleth )
	ROMS_AGALLET

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "agallet_hongkong.nv", 0x0000, 0x0080, CRC(998d1a74) SHA1(13e7e27a18417949d49e97d521781fc0feeef792) )
ROM_END

// these roms were dumped from a board set to the Japanese region.
#define ROMS_AGALLETA \
	ROM_REGION( 0x400000, "maincpu", 0 ) \
	ROM_LOAD16_WORD_SWAP( "u45", 0x000000, 0x080000, CRC(2cab18b0) SHA1(5e779b74d8520cb482697b5efba4746854e7c9fe) ) \
	ROMS_AGALLET_COMMON

/* the regions differ only in the EEPROM, hence the macro above - all EEPROMs are Factory Defaulted */
ROM_START( agalleta )
	ROMS_AGALLETA

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "agallet_europe.nv", 0x0000, 0x0080, CRC(ec38bf65) SHA1(cb8d9eacc0cf55a0c6b187e6673e3354554314b5) )
ROM_END

ROM_START( agalletau )
	ROMS_AGALLETA

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "agallet_usa.nv", 0x0000, 0x0080, CRC(72e65056) SHA1(abf1a86df01064d9d5d8c418e8367817319ec335) )
ROM_END

ROM_START( agalletaj ) // the dumped board was this region
	ROMS_AGALLETA

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "agallet_japan.nv", 0x0000, 0x0080, CRC(0753f547) SHA1(aabb987470406b8729894108bc4d050f7200917d) )
ROM_END

ROM_START( agalletak )
	ROMS_AGALLETA

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "agallet_korea.nv", 0x0000, 0x0080, CRC(7f41c253) SHA1(50793d4da0ad6eb590941d26a729a1cf4b3c25c2) )
ROM_END

ROM_START( agalletat )
	ROMS_AGALLETA

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "agallet_taiwan.nv", 0x0000, 0x0080, CRC(0af46742) SHA1(37b704c4c573b2aabd6f016e9e8dd458f95148f7) )
ROM_END

ROM_START( agalletah )
	ROMS_AGALLETA

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "agallet_hongkong.nv", 0x0000, 0x0080, CRC(998d1a74) SHA1(13e7e27a18417949d49e97d521781fc0feeef792) )
ROM_END

/***************************************************************************

              Fever SOS (International) / Dangun Feveron (Japan)

Board:  CV01
OSC:    28.0, 16.0, 16.9 MHz

***************************************************************************/

ROM_START( dfeveron )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "cv01-u34.bin", 0x000000, 0x080000, CRC(be87f19d) SHA1(595239245df3835cdf5a99a6c62480465558d8d3) )
	ROM_LOAD16_BYTE( "cv01-u33.bin", 0x000001, 0x080000, CRC(e53a7db3) SHA1(ddced29f78dc3cc89038757b6577ba2ba0d8b041) )

	ROM_REGION( 0x800000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD( "cv01-u25.bin", 0x000000, 0x400000, CRC(a6f6a95d) SHA1(e1eb45cb5d0e6163edfd9d830633b913fb53c6ca) )
	ROM_LOAD( "cv01-u26.bin", 0x400000, 0x400000, CRC(32edb62a) SHA1(3def74e1316b80cc25a8c3ac162cd7bcb8cc807c) )

	ROM_REGION( 0x200000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "cv01-u50.bin", 0x000000, 0x200000, CRC(7a344417) SHA1(828bd8f95d2fcc34407e17629ccafc904a4ea12d) )

	ROM_REGION( 0x200000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "cv01-u49.bin", 0x000000, 0x200000, CRC(d21cdda7) SHA1(cace4650de580c3c4a037f1f5c32bfc1846b383c) )

	ROM_REGION( 0x400000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "cv01-u19.bin", 0x000000, 0x400000, CRC(5f5514da) SHA1(53f27364aee544572a82649c9ff29bacc642b732) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-dfeveron.bin", 0x0000, 0x0080, CRC(c3174959) SHA1(29b5c94722756481e4f84bfd75dee15fdb5c8cf7) )
ROM_END

/*

Fever SOS

  The program code checks for 0x05 & 0x19 at the 17th & 18th byte in the EEPROM.  Therefore
  you cannot convert a Dangun Feveron over to a Fever SOS by changing the 2 program roms

Jumper JP1:
INT Version - 2 & 3
JAP Version - 1 & 2

However there are more differences:

U4:
INT Version - 013 9838EX003
JAP Version - 013 9807EX004 (The second set of numbers are manufacture day codes)

UA2 & UB2:
INT Version - 038 9838WX001
JAP Version - 038 9808WX003 (The second set of numbers are manufacture day codes)

TA8030S (Beside SW1)
INT Version  - NOT MOUNTED
JAP Version - TA8030S (WatchDog Timer, might be controlled by JP1)

U47 & U48 - Differ
U38 & U37 - Differ - These chips are Static RAM

It actually looks like the international version is older than
the Japanese version PCB wise, but the software date is 98/09/25
and mine is 98/09/17!

The famous full extent of the JAM is inside the image but so is
"full extent" of the LAW. There are also other version strings
inside the same image look here...

          NOTICE
  THIS GAME IS FOR USE IN
                KOREA ONLY
            HONG KONG ONLY
               TAIWAN ONLY
       SOUTHEAST ASIA ONLY
               EUROPE ONLY
                U.S.A ONLY
                JAPAN ONLY
SALES, EXPORT OR OPERATION
OUTSIDE THIS COUNTRY MAY BE
CONSTRUED AS COPYRIGHT AND
TRADEMARK INFRINGEMENT AND
IS STRICTLY PROHIBITED.
VIOLATOR AND SUBJECT TO
SEVERE PENALTIES AND WILL
BE PROSECUTED TO THE FULL
EXTENT OF THE JAM.
              98/09/10 VER.

Look at the version date!

          NOTICE
THIS GAME MAY NOT BE SOLD,
EXPORTED OR OPERATED
WITHOUTPROOF OF LEGAL CONSENT
BY CAVE CO.,LTD.
VIOLATION OF THESE TERMS WILL
RESULT IN COPYRIGHT AND
TRADEMARK INFRINGEMENT,AND IS
STRICTLY PROHIBITED.
VIOLATORS ARE SUBJECT TO
SEVERE PENALTIES AND WILL BE
PROSECUTED TO THE FULL EXTENT
OF THE LAW GOVERNED BY THE
COUNTRY OF ORIGIN.
                 98/09/25 VER

This is from Fever SOS image! Both version strings are present!

The PCB is also different, UD's PCB does not have the Cave logo and
the CV01 marker in the lower left corner of the PCB.

There is some "engrish" story inside the UD image but this is NOT
present in the japanese images...

*/

ROM_START( feversos )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "cv01-u34.sos", 0x000000, 0x080000, CRC(24ef3ce6) SHA1(42799eebbb2686a837b8972aec684143deadca59) )
	ROM_LOAD16_BYTE( "cv01-u33.sos", 0x000001, 0x080000, CRC(64ff73fd) SHA1(7fc3a8469cec2361d373a4dac4a547c13ca5f709) )

	ROM_REGION( 0x800000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD( "cv01-u25.bin", 0x000000, 0x400000, CRC(a6f6a95d) SHA1(e1eb45cb5d0e6163edfd9d830633b913fb53c6ca) )
	ROM_LOAD( "cv01-u26.bin", 0x400000, 0x400000, CRC(32edb62a) SHA1(3def74e1316b80cc25a8c3ac162cd7bcb8cc807c) )

	ROM_REGION( 0x200000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "cv01-u50.bin", 0x000000, 0x200000, CRC(7a344417) SHA1(828bd8f95d2fcc34407e17629ccafc904a4ea12d) )

	ROM_REGION( 0x200000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "cv01-u49.bin", 0x000000, 0x200000, CRC(d21cdda7) SHA1(cace4650de580c3c4a037f1f5c32bfc1846b383c) )

	ROM_REGION( 0x400000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "cv01-u19.bin", 0x000000, 0x400000, CRC(5f5514da) SHA1(53f27364aee544572a82649c9ff29bacc642b732) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-feversos.bin", 0x0000, 0x0080, CRC(d80303aa) SHA1(8580f7c2223c72614516d800a98465e362c333ef) )
ROM_END

/***************************************************************************

                                Dodonpachi (Japan)

PCB:    AT-C03 D2
CPU:    MC68000-16
Sound:  YMZ280B
OSC:    28.0000MHz
        16.0000MHz
        16.9MHz (16.9344MHz?)

***************************************************************************/

ROM_START( ddonpach )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "b1.u27", 0x000000, 0x080000, CRC(b5cdc8d3) SHA1(58757b50e21a27e500a82c03f62cf02a85389926) )
	ROM_LOAD16_BYTE( "b2.u26", 0x000001, 0x080000, CRC(6bbb063a) SHA1(e5de64b9c3efc0a38a2e0e16b78ee393bff63558) )

	ROM_REGION( 0x800000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD16_WORD_SWAP( "u50.bin", 0x000000, 0x200000, CRC(14b260ec) SHA1(33bda210302428d5500115d0c7a839cdfcb67d17) )
	ROM_LOAD16_WORD_SWAP( "u51.bin", 0x200000, 0x200000, CRC(e7ba8cce) SHA1(ad74a6b7d53760b19587c4a6dbea937daa7e87ce) )
	ROM_LOAD16_WORD_SWAP( "u52.bin", 0x400000, 0x200000, CRC(02492ee0) SHA1(64d9cc64a4ad189a8b03cf6a749ddb732b4a0014) )
	ROM_LOAD16_WORD_SWAP( "u53.bin", 0x600000, 0x200000, CRC(cb4c10f0) SHA1(a622e8bd0c938b5d38b392b247400b744d8be288) )

	ROM_REGION( 0x200000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "u60.bin", 0x000000, 0x200000, CRC(903096a7) SHA1(a243e903fef7c4a7b71383263e82e42acd869261) )

	ROM_REGION( 0x200000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "u61.bin", 0x000000, 0x200000, CRC(d89b7631) SHA1(a66bb4955ca58fab8973ca37a0f971e9a67ce017) )

	ROM_REGION( 0x200000, "layer2", 0 ) /* Layer 2 */
	ROM_LOAD( "u62.bin", 0x000000, 0x200000, CRC(292bfb6b) SHA1(11b385991ee990eb5ef36e136b988802b5f90fa4) )

	ROM_REGION( 0x400000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "u6.bin", 0x000000, 0x200000, CRC(9dfdafaf) SHA1(f5cb450cdc78a20c3a74c6dac05c9ac3cba08327) )
	ROM_LOAD( "u7.bin", 0x200000, 0x200000, CRC(795b17d5) SHA1(cbfc29f1df9600c82e0fdae00edd00da5b73e14c) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-ddonpach.bin", 0x0000, 0x0080, CRC(315fb546) SHA1(7f597107d1610fc286413e0e93c794c80c0c554f) )
ROM_END


ROM_START( ddonpachj )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "u27.bin", 0x000000, 0x080000, CRC(2432ff9b) SHA1(fbc826c30553f6553ead40b312b73c049e8f4bf6) )
	ROM_LOAD16_BYTE( "u26.bin", 0x000001, 0x080000, CRC(4f3a914a) SHA1(ae98eba049f1462aa1145f6959b9f9a32c97278f) )

	ROM_REGION( 0x800000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD16_WORD_SWAP( "u50.bin", 0x000000, 0x200000, CRC(14b260ec) SHA1(33bda210302428d5500115d0c7a839cdfcb67d17) )
	ROM_LOAD16_WORD_SWAP( "u51.bin", 0x200000, 0x200000, CRC(e7ba8cce) SHA1(ad74a6b7d53760b19587c4a6dbea937daa7e87ce) )
	ROM_LOAD16_WORD_SWAP( "u52.bin", 0x400000, 0x200000, CRC(02492ee0) SHA1(64d9cc64a4ad189a8b03cf6a749ddb732b4a0014) )
	ROM_LOAD16_WORD_SWAP( "u53.bin", 0x600000, 0x200000, CRC(cb4c10f0) SHA1(a622e8bd0c938b5d38b392b247400b744d8be288) )

	ROM_REGION( 0x200000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "u60.bin", 0x000000, 0x200000, CRC(903096a7) SHA1(a243e903fef7c4a7b71383263e82e42acd869261) )

	ROM_REGION( 0x200000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "u61.bin", 0x000000, 0x200000, CRC(d89b7631) SHA1(a66bb4955ca58fab8973ca37a0f971e9a67ce017) )

	ROM_REGION( 0x200000, "layer2", 0 ) /* Layer 2 */
	ROM_LOAD( "u62.bin", 0x000000, 0x200000, CRC(292bfb6b) SHA1(11b385991ee990eb5ef36e136b988802b5f90fa4) )

	ROM_REGION( 0x400000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "u6.bin", 0x000000, 0x200000, CRC(9dfdafaf) SHA1(f5cb450cdc78a20c3a74c6dac05c9ac3cba08327) )
	ROM_LOAD( "u7.bin", 0x200000, 0x200000, CRC(795b17d5) SHA1(cbfc29f1df9600c82e0fdae00edd00da5b73e14c) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-ddonpach.bin", 0x0000, 0x0080, CRC(315fb546) SHA1(7f597107d1610fc286413e0e93c794c80c0c554f) )
ROM_END


ROM_START( ddonpacha )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "arrange_u27.bin", 0x000000, 0x080000, CRC(44b899ae) SHA1(798ec437d861b94fcd90c99a7015dd420887c788) )
	ROM_LOAD16_BYTE( "arrange_u26.bin", 0x000001, 0x080000, CRC(727a09a8) SHA1(91876386855f19e8a3d8d1df71dfe9b3d98e9ea9) )

	ROM_REGION( 0x800000, "sprites0", 0 )       /* Sprites: * 2 */
	ROM_LOAD16_WORD_SWAP( "u50.bin", 0x000000, 0x200000, CRC(14b260ec) SHA1(33bda210302428d5500115d0c7a839cdfcb67d17) )
	ROM_LOAD16_WORD_SWAP( "arrange_u51.bin", 0x200000, 0x200000, CRC(0f3e5148) SHA1(3016f4d075940feae691389606cd2aa7ac53849e) )
	ROM_LOAD16_WORD_SWAP( "u52.bin", 0x400000, 0x200000, CRC(02492ee0) SHA1(64d9cc64a4ad189a8b03cf6a749ddb732b4a0014) )
	ROM_LOAD16_WORD_SWAP( "u53.bin", 0x600000, 0x200000, CRC(cb4c10f0) SHA1(a622e8bd0c938b5d38b392b247400b744d8be288) )

	ROM_REGION( 0x200000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "u60.bin", 0x000000, 0x200000, CRC(903096a7) SHA1(a243e903fef7c4a7b71383263e82e42acd869261) )

	ROM_REGION( 0x200000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "u61.bin", 0x000000, 0x200000, CRC(d89b7631) SHA1(a66bb4955ca58fab8973ca37a0f971e9a67ce017) )

	ROM_REGION( 0x200000, "layer2", 0 ) /* Layer 2 */
	ROM_LOAD( "arrange_u62.bin", 0x000000, 0x200000, CRC(42e4c6c5) SHA1(4d282f7592f5fc5e11839c57f39cae20b8422aa1) )

	ROM_REGION( 0x400000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "u6.bin", 0x000000, 0x200000, CRC(9dfdafaf) SHA1(f5cb450cdc78a20c3a74c6dac05c9ac3cba08327) )
	ROM_LOAD( "u7.bin", 0x200000, 0x200000, CRC(795b17d5) SHA1(cbfc29f1df9600c82e0fdae00edd00da5b73e14c) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-ddonpach.bin", 0x0000, 0x0080, CRC(2df16438) SHA1(4881b70589a97e2420feb6d6e6737273beeff303) )
ROM_END

/***************************************************************************

                                Donpachi

Known versions:

USA        Version 1.12 1995/05/2x
Korea      Version 1.12 1995/05/2x
Hong Kong  Version 1.10 1995/05/17
Japan      Version 1.01 1995/05/11

BOARD #:      AT-C01DP-2
CPU:          TMP68HC000-16
VOICE:        M6295 x2
OSC:          28.000/16.000/4.220MHz
EEPROM:       ATMEL 93C46
CUSTOM:       ATLUS 8647-01 013
              038 9429WX727 x3
              NMK 112 (M6295 sample ROM banking)

---------------------------------------------------
 filenames          devices       kind
---------------------------------------------------
 PRG.U29            27C4096       68000 main prg.
 U58.BIN            27C020        gfx   data
 ATDP.U32           57C8200       M6295 data
 ATDP.U33           57C16200      M6295 data
 ATDP.U44           57C16200      gfx   data
 ATDP.U45           57C16200      gfx   data
 ATDP.U54           57C8200       gfx   data
 ATDP.U57           57C8200       gfx   data

 USA Version
----------------------------------------------------
 prgu.U29           27C4002       68000 Main Program
 text.u58           27C2001       Labeled as "TEXT"

***************************************************************************/

ROM_START( donpachi )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "prgu.u29",     0x00000, 0x80000, CRC(89c36802) SHA1(7857c726cecca5a4fce282e0d2b873774d2c1b1d) )

	ROM_REGION( 0x400000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD16_WORD_SWAP( "atdp.u44", 0x000000, 0x200000, CRC(7189e953) SHA1(53adbe6ea5e01ecb48575e9db82cc3d0dc8a3726) )
	ROM_LOAD16_WORD_SWAP( "atdp.u45", 0x200000, 0x200000, CRC(6984173f) SHA1(625dd6674adeb206815855b8b6a1fba79ed5c4cd) )

	ROM_REGION( 0x100000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "atdp.u54", 0x000000, 0x100000, CRC(6bda6b66) SHA1(6472e6706505bac17484fb8bf4e8922ced4adf63) )

	ROM_REGION( 0x100000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "atdp.u57", 0x000000, 0x100000, CRC(0a0e72b9) SHA1(997e8253777e7acca5a1c0c4026e78eecc122d5d) )

	ROM_REGION( 0x040000, "layer2", 0 ) /* Text / Character Layer */
	ROM_LOAD( "text.u58", 0x000000, 0x040000, CRC(5dba06e7) SHA1(f9dab7f6c732a683fddb4cae090a875b3962332b) )

	ROM_REGION( 0x240000, "oki1", 0 )   /* OKIM6295 #1 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "atdp.u33", 0x040000, 0x200000, CRC(d749de00) SHA1(64a0acc23eb2515e7d0459f0289919e083c63afc) )

	ROM_REGION( 0x340000, "oki2", 0 )   /* OKIM6295 #2 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "atdp.u32", 0x040000, 0x100000, CRC(0d89fcca) SHA1(e16ed15fa5e72537822f7b37e83ccfed0fa87338) )
	ROM_LOAD( "atdp.u33", 0x140000, 0x200000, CRC(d749de00) SHA1(64a0acc23eb2515e7d0459f0289919e083c63afc) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-donpachi.u10", 0x0000, 0x0080, CRC(315fb546) SHA1(7f597107d1610fc286413e0e93c794c80c0c554f) ) /* ATMEL 93C46 */

	ROM_REGION( 0x0155, "pal", 0 )
	ROM_LOAD( "peel18cv8p-15.u18", 0x0000, 0x0155, CRC(3f4787e9) SHA1(fc7da25c9f36c9cbc6ba5a7314c4828d405d1261) ) /* PEEL18CV8P-15 */
ROM_END

ROM_START( donpachij )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "prg.u29",     0x00000, 0x80000, CRC(6be14af6) SHA1(5b1158071f160efeded816ae4c4edca1d00d6e05) )

	ROM_REGION( 0x400000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD16_WORD_SWAP( "atdp.u44", 0x000000, 0x200000, CRC(7189e953) SHA1(53adbe6ea5e01ecb48575e9db82cc3d0dc8a3726) )
	ROM_LOAD16_WORD_SWAP( "atdp.u45", 0x200000, 0x200000, CRC(6984173f) SHA1(625dd6674adeb206815855b8b6a1fba79ed5c4cd) )

	ROM_REGION( 0x100000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "atdp.u54", 0x000000, 0x100000, CRC(6bda6b66) SHA1(6472e6706505bac17484fb8bf4e8922ced4adf63) )

	ROM_REGION( 0x100000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "atdp.u57", 0x000000, 0x100000, CRC(0a0e72b9) SHA1(997e8253777e7acca5a1c0c4026e78eecc122d5d) )

	ROM_REGION( 0x040000, "layer2", 0 ) /* Text / Character Layer */
	ROM_LOAD( "u58.bin", 0x000000, 0x040000, CRC(285379ff) SHA1(b9552edcec29ddf4b552800b145c398b94117ab0) )

	ROM_REGION( 0x240000, "oki1", 0 )   /* OKIM6295 #1 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "atdp.u33", 0x040000, 0x200000, CRC(d749de00) SHA1(64a0acc23eb2515e7d0459f0289919e083c63afc) )

	ROM_REGION( 0x340000, "oki2", 0 )   /* OKIM6295 #2 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "atdp.u32", 0x040000, 0x100000, CRC(0d89fcca) SHA1(e16ed15fa5e72537822f7b37e83ccfed0fa87338) )
	ROM_LOAD( "atdp.u33", 0x140000, 0x200000, CRC(d749de00) SHA1(64a0acc23eb2515e7d0459f0289919e083c63afc) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-donpachi.bin", 0x0000, 0x0080, CRC(315fb546) SHA1(7f597107d1610fc286413e0e93c794c80c0c554f) )

	ROM_REGION( 0x0155, "pal", 0 )
	ROM_LOAD( "peel18cv8p-15.u18", 0x0000, 0x0155, CRC(3f4787e9) SHA1(fc7da25c9f36c9cbc6ba5a7314c4828d405d1261) ) /* PEEL18CV8P-15 */
ROM_END

ROM_START( donpachikr )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "prgk.u26",    0x00000, 0x80000, CRC(bbaf4c8b) SHA1(0f9d42c8c4c5b69e3d39bf768bc4b663f66b4f36) )

	ROM_REGION( 0x400000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD16_WORD_SWAP( "atdp.u44", 0x000000, 0x200000, CRC(7189e953) SHA1(53adbe6ea5e01ecb48575e9db82cc3d0dc8a3726) )
	ROM_LOAD16_WORD_SWAP( "atdp.u45", 0x200000, 0x200000, CRC(6984173f) SHA1(625dd6674adeb206815855b8b6a1fba79ed5c4cd) )

	ROM_REGION( 0x100000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "atdp.u54", 0x000000, 0x100000, CRC(6bda6b66) SHA1(6472e6706505bac17484fb8bf4e8922ced4adf63) )

	ROM_REGION( 0x100000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "atdp.u57", 0x000000, 0x100000, CRC(0a0e72b9) SHA1(997e8253777e7acca5a1c0c4026e78eecc122d5d) )

	ROM_REGION( 0x040000, "layer2", 0 ) /* Text / Character Layer */
	ROM_LOAD( "text.u58", 0x000000, 0x040000, CRC(5dba06e7) SHA1(f9dab7f6c732a683fddb4cae090a875b3962332b) )

	ROM_REGION( 0x240000, "oki1", 0 )   /* OKIM6295 #1 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "atdp.u33", 0x040000, 0x200000, CRC(d749de00) SHA1(64a0acc23eb2515e7d0459f0289919e083c63afc) )

	ROM_REGION( 0x340000, "oki2", 0 )   /* OKIM6295 #2 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "atdp.u32", 0x040000, 0x100000, CRC(0d89fcca) SHA1(e16ed15fa5e72537822f7b37e83ccfed0fa87338) )
	ROM_LOAD( "atdp.u33", 0x140000, 0x200000, CRC(d749de00) SHA1(64a0acc23eb2515e7d0459f0289919e083c63afc) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-donpachi.bin", 0x0000, 0x0080, CRC(315fb546) SHA1(7f597107d1610fc286413e0e93c794c80c0c554f) )

	ROM_REGION( 0x0155, "pal", 0 )
	ROM_LOAD( "peel18cv8p-15.u18", 0x0000, 0x0155, CRC(3f4787e9) SHA1(fc7da25c9f36c9cbc6ba5a7314c4828d405d1261) ) /* PEEL18CV8P-15 */
ROM_END

ROM_START( donpachihk )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "37.u29",    0x00000, 0x80000, CRC(71f39f30) SHA1(08a028208f21c073d450a29061604f27775786a8) )

	ROM_REGION( 0x400000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD16_WORD_SWAP( "atdp.u44", 0x000000, 0x200000, CRC(7189e953) SHA1(53adbe6ea5e01ecb48575e9db82cc3d0dc8a3726) )
	ROM_LOAD16_WORD_SWAP( "atdp.u45", 0x200000, 0x200000, CRC(6984173f) SHA1(625dd6674adeb206815855b8b6a1fba79ed5c4cd) )

	ROM_REGION( 0x100000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "atdp.u54", 0x000000, 0x100000, CRC(6bda6b66) SHA1(6472e6706505bac17484fb8bf4e8922ced4adf63) )

	ROM_REGION( 0x100000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "atdp.u57", 0x000000, 0x100000, CRC(0a0e72b9) SHA1(997e8253777e7acca5a1c0c4026e78eecc122d5d) )

	ROM_REGION( 0x040000, "layer2", 0 ) /* Text / Character Layer */
	ROM_LOAD( "u58.bin", 0x000000, 0x040000, CRC(285379ff) SHA1(b9552edcec29ddf4b552800b145c398b94117ab0) )

	ROM_REGION( 0x240000, "oki1", 0 )   /* OKIM6295 #1 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "atdp.u33", 0x040000, 0x200000, CRC(d749de00) SHA1(64a0acc23eb2515e7d0459f0289919e083c63afc) )

	ROM_REGION( 0x340000, "oki2", 0 )   /* OKIM6295 #2 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "atdp.u32", 0x040000, 0x100000, CRC(0d89fcca) SHA1(e16ed15fa5e72537822f7b37e83ccfed0fa87338) )
	ROM_LOAD( "atdp.u33", 0x140000, 0x200000, CRC(d749de00) SHA1(64a0acc23eb2515e7d0459f0289919e083c63afc) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-donpachi.bin", 0x0000, 0x0080, CRC(315fb546) SHA1(7f597107d1610fc286413e0e93c794c80c0c554f) )

	ROM_REGION( 0x0155, "pal", 0 )
	ROM_LOAD( "peel18cv8p-15.u18", 0x0000, 0x0155, CRC(3f4787e9) SHA1(fc7da25c9f36c9cbc6ba5a7314c4828d405d1261) ) /* PEEL18CV8P-15 */
ROM_END

/*
    When you press the 2p start button, it pauses the game (music still plays).
    Pressing the 1p start button unpauses the game.
    If you press both 1p start and 2p start at the same time, the game lets you play in slow motion (music still plays normally).

    This was on the label of the ROM chip: 国内撮影用
*/

ROM_START( donpachijs )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "prg.u29",     0x00000, 0x80000, CRC(810dbd42) SHA1(703a5aec90b595a1c5a679ab165643119ba6b2f3) )

	ROM_REGION( 0x400000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD16_WORD_SWAP( "atdp.u44", 0x000000, 0x200000, CRC(7189e953) SHA1(53adbe6ea5e01ecb48575e9db82cc3d0dc8a3726) )
	ROM_LOAD16_WORD_SWAP( "atdp.u45", 0x200000, 0x200000, CRC(6984173f) SHA1(625dd6674adeb206815855b8b6a1fba79ed5c4cd) )

	ROM_REGION( 0x100000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "atdp.u54", 0x000000, 0x100000, CRC(6bda6b66) SHA1(6472e6706505bac17484fb8bf4e8922ced4adf63) )

	ROM_REGION( 0x100000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "atdp.u57", 0x000000, 0x100000, CRC(0a0e72b9) SHA1(997e8253777e7acca5a1c0c4026e78eecc122d5d) )

	ROM_REGION( 0x040000, "layer2", 0 ) /* Text / Character Layer */
	ROM_LOAD( "u58.bin", 0x000000, 0x040000, CRC(285379ff) SHA1(b9552edcec29ddf4b552800b145c398b94117ab0) )

	ROM_REGION( 0x240000, "oki1", 0 )   /* OKIM6295 #1 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "atdp.u33", 0x040000, 0x200000, CRC(d749de00) SHA1(64a0acc23eb2515e7d0459f0289919e083c63afc) )

	ROM_REGION( 0x340000, "oki2", 0 )   /* OKIM6295 #2 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "atdp.u32", 0x040000, 0x100000, CRC(0d89fcca) SHA1(e16ed15fa5e72537822f7b37e83ccfed0fa87338) )
	ROM_LOAD( "atdp.u33", 0x140000, 0x200000, CRC(d749de00) SHA1(64a0acc23eb2515e7d0459f0289919e083c63afc) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-donpachi.u10", 0x0000, 0x0080, CRC(315fb546) SHA1(7f597107d1610fc286413e0e93c794c80c0c554f) ) /* ATMEL 93C46 */

	ROM_REGION( 0x0155, "pal", 0 )
	ROM_LOAD( "peel18cv8p-15.u18", 0x0000, 0x0155, CRC(3f4787e9) SHA1(fc7da25c9f36c9cbc6ba5a7314c4828d405d1261) ) /* PEEL18CV8P-15 */
ROM_END

/***************************************************************************

                                ESP Ra.De.

ATC04
OSC:    28.0, 16.0, 16.9 MHz

***************************************************************************/

ROM_START( esprade )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "u42.int", 0x000000, 0x080000, CRC(3b510a73) SHA1(ab1666eb826cb4a71588d86831dd18a2ef1c2a33) )
	ROM_LOAD16_BYTE( "u41.int", 0x000001, 0x080000, CRC(97c1b649) SHA1(37a56b7b9662219a356aee3f4b5cbb774ac4950e) )

	ROM_REGION( 0x1000000, "sprites0", 0 )       /* Sprites */
	ROM_LOAD32_WORD_SWAP( "esp_u63.u63", 0x000000, 0x400000, CRC(2f2fe92c) SHA1(9519e365248bcec8419786eabb16fe4aae299af5) )
	ROM_LOAD32_WORD_SWAP( "esp_u64.u64", 0x000002, 0x400000, CRC(491a3da4) SHA1(53549a2bd3edc7b5e73fb46e1421b156bb0c190f) )
	ROM_LOAD32_WORD_SWAP( "esp_u65.u65", 0x800000, 0x400000, CRC(06563efe) SHA1(94e72da1f542b4e0525b4b43994242816b43dbdc) )
	ROM_LOAD32_WORD_SWAP( "esp_u66.u66", 0x800002, 0x400000, CRC(7bbe4cfc) SHA1(e77d0ed7a11b5abca1df8a0eb20ac9360cf79e76) )

	ROM_REGION( 0x800000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "esp_u54.u54", 0x000000, 0x400000, CRC(e7ca6936) SHA1(b7f5ab67071a1d9dd3d2c1cd2304d9cdad68850c) )
	ROM_LOAD( "esp_u55.u55", 0x400000, 0x400000, CRC(f53bd94f) SHA1(d0a74fb3d36fe522ef075e5ae44a9980da8abe2f) )

	ROM_REGION( 0x800000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "esp_u52.u52", 0x000000, 0x400000, CRC(e7abe7b4) SHA1(e98da45497e1aaf0d6ab352ec3e43c7438ed792a) )
	ROM_LOAD( "esp_u53.u53", 0x400000, 0x400000, CRC(51a0f391) SHA1(8b7355cbad119f4e1add14e5cd5e343ec6706104) )

	ROM_REGION( 0x400000, "layer2", 0 ) /* Layer 2 */
	ROM_LOAD( "esp_u51.u51", 0x000000, 0x400000, CRC(0b9b875c) SHA1(ef05447cd8565ae24bb71db42342724622ad1e3e) )

	ROM_REGION( 0x400000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "esp_u19.u19", 0x000000, 0x400000, CRC(f54b1cab) SHA1(34d70bb5798de85d892c062001d9ac1d6604fd9f) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-esprade.bin", 0x0000, 0x0080, CRC(315fb546) SHA1(7f597107d1610fc286413e0e93c794c80c0c554f) )
ROM_END

ROM_START( espradej )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "u42_ver.2", 0x000000, 0x080000, CRC(75d03c42) SHA1(1c176185b6f1531752b633a97f705ffa0cfeb5ad) )
	ROM_LOAD16_BYTE( "u41_ver.2", 0x000001, 0x080000, CRC(734b3ef0) SHA1(f584227b85c347d62d5f179445011ce0f607bcfd) )

	ROM_REGION( 0x1000000, "sprites0", 0 )       /* Sprites */
	ROM_LOAD32_WORD_SWAP( "esp_u63.u63", 0x000000, 0x400000, CRC(2f2fe92c) SHA1(9519e365248bcec8419786eabb16fe4aae299af5) )
	ROM_LOAD32_WORD_SWAP( "esp_u64.u64", 0x000002, 0x400000, CRC(491a3da4) SHA1(53549a2bd3edc7b5e73fb46e1421b156bb0c190f) )
	ROM_LOAD32_WORD_SWAP( "esp_u65.u65", 0x800000, 0x400000, CRC(06563efe) SHA1(94e72da1f542b4e0525b4b43994242816b43dbdc) )
	ROM_LOAD32_WORD_SWAP( "esp_u66.u66", 0x800002, 0x400000, CRC(7bbe4cfc) SHA1(e77d0ed7a11b5abca1df8a0eb20ac9360cf79e76) )

	ROM_REGION( 0x800000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "esp_u54.u54", 0x000000, 0x400000, CRC(e7ca6936) SHA1(b7f5ab67071a1d9dd3d2c1cd2304d9cdad68850c) )
	ROM_LOAD( "esp_u55.u55", 0x400000, 0x400000, CRC(f53bd94f) SHA1(d0a74fb3d36fe522ef075e5ae44a9980da8abe2f) )

	ROM_REGION( 0x800000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "esp_u52.u52", 0x000000, 0x400000, CRC(e7abe7b4) SHA1(e98da45497e1aaf0d6ab352ec3e43c7438ed792a) )
	ROM_LOAD( "esp_u53.u53", 0x400000, 0x400000, CRC(51a0f391) SHA1(8b7355cbad119f4e1add14e5cd5e343ec6706104) )

	ROM_REGION( 0x400000, "layer2", 0 ) /* Layer 2 */
	ROM_LOAD( "esp_u51.u51", 0x000000, 0x400000, CRC(0b9b875c) SHA1(ef05447cd8565ae24bb71db42342724622ad1e3e) )

	ROM_REGION( 0x400000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "esp_u19.u19", 0x000000, 0x400000, CRC(f54b1cab) SHA1(34d70bb5798de85d892c062001d9ac1d6604fd9f) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-esprade.bin", 0x0000, 0x0080, CRC(315fb546) SHA1(7f597107d1610fc286413e0e93c794c80c0c554f) )
ROM_END

ROM_START( espradejo )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "u42.bin", 0x000000, 0x080000, CRC(0718c7e5) SHA1(c7d1f30bd2ef363cad15b6918f9980312a15809a) )
	ROM_LOAD16_BYTE( "u41.bin", 0x000001, 0x080000, CRC(def30539) SHA1(957ad0b06f06689ae71393572592f6b8f818603a) )

	ROM_REGION( 0x1000000, "sprites0", 0 )       /* Sprites */
	ROM_LOAD32_WORD_SWAP( "esp_u63.u63", 0x000000, 0x400000, CRC(2f2fe92c) SHA1(9519e365248bcec8419786eabb16fe4aae299af5) )
	ROM_LOAD32_WORD_SWAP( "esp_u64.u64", 0x000002, 0x400000, CRC(491a3da4) SHA1(53549a2bd3edc7b5e73fb46e1421b156bb0c190f) )
	ROM_LOAD32_WORD_SWAP( "esp_u65.u65", 0x800000, 0x400000, CRC(06563efe) SHA1(94e72da1f542b4e0525b4b43994242816b43dbdc) )
	ROM_LOAD32_WORD_SWAP( "esp_u66.u66", 0x800002, 0x400000, CRC(7bbe4cfc) SHA1(e77d0ed7a11b5abca1df8a0eb20ac9360cf79e76) )

	ROM_REGION( 0x800000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "esp_u54.u54", 0x000000, 0x400000, CRC(e7ca6936) SHA1(b7f5ab67071a1d9dd3d2c1cd2304d9cdad68850c) )
	ROM_LOAD( "esp_u55.u55", 0x400000, 0x400000, CRC(f53bd94f) SHA1(d0a74fb3d36fe522ef075e5ae44a9980da8abe2f) )

	ROM_REGION( 0x800000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "esp_u52.u52", 0x000000, 0x400000, CRC(e7abe7b4) SHA1(e98da45497e1aaf0d6ab352ec3e43c7438ed792a) )
	ROM_LOAD( "esp_u53.u53", 0x400000, 0x400000, CRC(51a0f391) SHA1(8b7355cbad119f4e1add14e5cd5e343ec6706104) )

	ROM_REGION( 0x400000, "layer2", 0 ) /* Layer 2 */
	ROM_LOAD( "esp_u51.u51", 0x000000, 0x400000, CRC(0b9b875c) SHA1(ef05447cd8565ae24bb71db42342724622ad1e3e) )

	ROM_REGION( 0x400000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "esp_u19.u19", 0x000000, 0x400000, CRC(f54b1cab) SHA1(34d70bb5798de85d892c062001d9ac1d6604fd9f) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-esprade.bin", 0x0000, 0x0080, CRC(315fb546) SHA1(7f597107d1610fc286413e0e93c794c80c0c554f) )
ROM_END


/***************************************************************************

                                Gaia Crusaders

Noise Factory, 1999

PCB Layout
----------

|------------------------------------------------|
|   YAC516    YMZ280B      XC9536      68000     |
|          16MHz                       PRG2   PAL|
|                          TC51832     PRG1      |
|     SND3     SND2        TC51832   28.322MHz   |
|              SND1        62256     16MHz       |
|                          62256                 |
|J 62256 62256 62256 62256 62256 62256           |
|A                                 KM416C256     |
|M                                      KM416C256|
|M     -------------------  ---------------      |
|A     |     |     |     |  |             | 62256|
|      |     |     |     |  |             |      |
| DSW1 |     |     |     |  |013 9918EX008| 62256|
|      |038 9838WX003(x3)|  |             |      |
|      -------------------  ---------------      |
| DSW2                                           |
|                    XC9536          OBJ2        |
|                                                |
|       BG2     BG3    BG1           OBJ1        |
|                                                |
|------------------------------------------------|

Notes:
      68000 clock  : 16.000MHz
      YMZ280B clock: 16.000MHz
      VSync        : 58Hz
      HSync        : 15.40kHz

***************************************************************************/

ROM_START( gaia )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "prg1.127", 0x000000, 0x080000, CRC(47b904b2) SHA1(58b9b55f59cf00f70b690a0371096e86f4d723c2) )
	ROM_LOAD16_BYTE( "prg2.128", 0x000001, 0x080000, CRC(469b7794) SHA1(502f855c51005a866900b19c3a0a170d9ea02392) )

	ROM_REGION( 0x800000, "sprites0", 0 )  /* Sprites */
	ROM_LOAD( "obj1.736", 0x000000, 0x400000, CRC(f4f84e5d) SHA1(8f445dd7a5c8a996939c211e5aec5742121a6e7e) )
	ROM_LOAD( "obj2.738", 0x400000, 0x400000, CRC(15c2a9ce) SHA1(631eb2968395be86ef2403733e7d4ec769a013b9) )

	ROM_REGION( 0x400000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "bg1.989", 0x000000, 0x400000, CRC(013a693d) SHA1(2cc5be6f47c13febed942e1c3167946efedc5f9b) )

	ROM_REGION( 0x400000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "bg2.995", 0x000000, 0x400000, CRC(783cc62f) SHA1(8b6e4212688b53be5ecc29ff2d41fd43e7d0a420) )

	ROM_REGION( 0x400000, "layer2", 0 ) /* Layer 2 */
	ROM_LOAD( "bg3.998", 0x000000, 0x400000, CRC(bcd61d1c) SHA1(660a3b02a8c39e1117b00d0ad06f73221fef4ce8) )

	ROM_REGION( 0xc00000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "snd1.447", 0x000000, 0x400000, CRC(92770a52) SHA1(81f6835e1b45eb0f367e4586fdda92466f02edb9) )
	ROM_LOAD( "snd2.454", 0x400000, 0x400000, CRC(329ae1cf) SHA1(0c5e5074a5d8f4fb85ab4893bc953f192dcb301a) )
	ROM_LOAD( "snd3.455", 0x800000, 0x400000, CRC(4048d64e) SHA1(5e4ec6d37e70484e2fcd04188385e79ef0b53026) )
ROM_END

/*
Thunder Heroes
Primetek Investments Ltd. , 2001

A quasi-clone, remake or continuation of Gaia Crusaders but is clearly a different game.

PCB Layout
----------

|------------------------------------------------------|
| 4558  YAC516   YMZ280B     XILINX                    |
| 4558   16MHz               XC9536    68000      PAL  |
|                SND2                                  |
|   VOL  SND3                TC51832   EPM0            |
|                SND1                                  |
|                            TC51832   EPM1            |
|                    58257           28.322MHz         |
|J                   58257                16MHz        |
|A   58257  58257  58257  58257  58257                 |
|M  DSW1                      58257   M514260  M514260 |
|M                                                     |
|A  DSW2 |------|  |------|  |------|  |--------|58257 |
|        | 038  |  | 038  |  | 038  |  |  013   |      |
|        |      |  |      |  |      |  |        |58257 |
|        |------|  |------|  |------|  |        |      |
|                                      |--------|      |
|                         XILINX            OBJ2       |
|                         XC9536                       |
|       BG2       BG3       BG1             OBJ1       |
|------------------------------------------------------|
Notes:
      68000 clock 16.00MHz
      YMZ280B clock 16.000MHz
      HSync 15.4kHz
      VSync 58Hz
      038/013 = Same video chips used on some Banpresto games
*/

ROM_START( theroes )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "t-hero-epm1.u0127", 0x000000, 0x080000, CRC(09db7195) SHA1(6aa5aa80e3b74e405ed8f1b9b801ce4367756986) )
	ROM_LOAD16_BYTE( "t-hero-epm0.u0129", 0x000001, 0x080000, CRC(2d4e3310) SHA1(7c3284a2adc7943db50933a209d037422f87f80b) )

	ROM_REGION( 0x800000, "sprites0", 0 )  /* Sprites */
	ROM_LOAD( "t-hero-obj1.u0736", 0x000000, 0x400000, CRC(35090f7c) SHA1(035e6c12a87d9c7241eea34fc7e2170bec842acc) )
	ROM_LOAD( "t-hero-obj2.u0738", 0x400000, 0x400000, CRC(71605108) SHA1(6070c26d8f22fafc81d97cacfef96ae652e355d0) )

	ROM_REGION( 0x400000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "t-hero-bg1.u0999", 0x000000, 0x400000, CRC(47b0fb40) SHA1(a7217b3d805b4255c589821cdadd9b190cada525) )

	ROM_REGION( 0x400000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "t-hero-bg2.u0995", 0x000000, 0x400000, CRC(b16237a1) SHA1(66aed2c5036492a17d20de90333e172a6f117851) )

	ROM_REGION( 0x400000, "layer2", 0 ) /* Layer 2 */
	ROM_LOAD( "t-hero-bg3.u0998", 0x000000, 0x400000, CRC(08eb5604) SHA1(3d32966708c73198272c40e6ddc680bf4c7919eb) )

	ROM_REGION( 0xc00000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "crvsaders-snd1.u0447", 0x000000, 0x400000, CRC(92770a52) SHA1(81f6835e1b45eb0f367e4586fdda92466f02edb9) )
	ROM_LOAD( "crvsaders-snd2.u0454", 0x400000, 0x400000, CRC(329ae1cf) SHA1(0c5e5074a5d8f4fb85ab4893bc953f192dcb301a) )
	ROM_LOAD( "t-hero-snd3.u0455",    0x800000, 0x400000, CRC(52b0b2c0) SHA1(6e96698905391c21a4fedd60e2768734b58add4e) )
ROM_END


/***************************************************************************

                                Guwange (Japan)

PCB:    ATC05
CPU:    MC68000-16
Sound:  YMZ280B
OSC:    28.0000MHz
        16.0000MHz
        16.9MHz

***************************************************************************/

ROM_START( guwange )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "gu-u0127.bin", 0x000000, 0x080000, CRC(f86b5293) SHA1(f8b1cd77cc25328d5010889850e4b86c27d9e396) )
	ROM_LOAD16_BYTE( "gu-u0129.bin", 0x000001, 0x080000, CRC(6c0e3b93) SHA1(aaad6569b9a7b6f9a315062f9fedfc95851c1bc6) )

	ROM_REGION( 0x2000000, "sprites0", 0 )       /* Sprites */
	ROM_LOAD32_WORD_SWAP( "u083.bin", 0x0000000, 0x800000, CRC(adc4b9c4) SHA1(3f9fb004e19187bbfa87ddfe8cfc69740656a1bd) )
	ROM_LOAD32_WORD_SWAP( "u082.bin", 0x0000002, 0x800000, CRC(3d75876c) SHA1(705b8c2dbdc31e9516f429969f87988beec796d7) )
	ROM_LOAD32_WORD_SWAP( "u086.bin", 0x1000000, 0x400000, CRC(188e4f81) SHA1(626074d81782a6de0b52406331b4b8561d3e36f5) )
	ROM_RELOAD(                       0x1800000, 0x400000 )
	ROM_LOAD32_WORD_SWAP( "u085.bin", 0x1000002, 0x400000, CRC(a7d5659e) SHA1(10abac022ebe106a3ca7186ff18ca2757f903033) )
	ROM_RELOAD(                       0x1800002, 0x400000 )
//  sprite bug fix?
//  ROM_FILL(                    0x1800000, 0x800000, 0xff )

	ROM_REGION( 0x800000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "u101.bin", 0x000000, 0x800000, CRC(0369491f) SHA1(ca6b1345506f13a17c9bace01637d1f61a278644) )

	ROM_REGION( 0x400000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "u10102.bin", 0x000000, 0x400000, CRC(e28d6855) SHA1(7001a6e298c6a1fcceb79586bf5f4bf0f30027f6) )

	ROM_REGION( 0x400000, "layer2", 0 ) /* Layer 2 */
	ROM_LOAD( "u10103.bin", 0x000000, 0x400000, CRC(0fe91b8e) SHA1(8b71ebeef5e4d2b00fdaaab97776d74e1c96dc59) )

	ROM_REGION( 0x400000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "u0462.bin", 0x000000, 0x400000, CRC(b3d75691) SHA1(71d8dae92be1542a3cff50efeec0bf3c14ab59f5) )

	ROM_REGION( 0x0004, "plds", 0 )
	ROM_LOAD( "atc05-1.bin", 0x0000, 0x0001, NO_DUMP ) /* GAL16V8D-15LP located at U159 */
	ROM_LOAD( "u0259.bin",   0x0000, 0x0001, NO_DUMP ) /* XC9536-15PC44C Located at U0249. (Chip label different then label silk screened onto the board.) */
	ROM_LOAD( "u108.bin",    0x0000, 0x0001, NO_DUMP ) /* XC9536-15PC44C Located at U108. */
	ROM_LOAD( "u084.bin",    0x0000, 0x0001, NO_DUMP ) /* XC9536-15PC44C Located at U084. */

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-guwange.bin", 0x0000, 0x0080, CRC(c3174959) SHA1(29b5c94722756481e4f84bfd75dee15fdb5c8cf7) )
ROM_END

ROM_START( guwanges )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "gu-u0127b.bin", 0x000000, 0x080000, CRC(64667d2e) SHA1(a5893eb38e309e2bced4a46559f02850ab39afe7) )
	ROM_LOAD16_BYTE( "gu-u0129b.bin", 0x000001, 0x080000, CRC(a99c6b6c) SHA1(614a3cd1de9b325f73e461eaf250ff9cf773f4a5) )

	ROM_REGION( 0x2000000, "sprites0", 0 )       /* Sprites */
	ROM_LOAD32_WORD_SWAP( "u083.bin", 0x0000000, 0x800000, CRC(adc4b9c4) SHA1(3f9fb004e19187bbfa87ddfe8cfc69740656a1bd) )
	ROM_LOAD32_WORD_SWAP( "u082.bin", 0x0000002, 0x800000, CRC(3d75876c) SHA1(705b8c2dbdc31e9516f429969f87988beec796d7) )
	ROM_LOAD32_WORD_SWAP( "u086.bin", 0x1000000, 0x400000, CRC(188e4f81) SHA1(626074d81782a6de0b52406331b4b8561d3e36f5) )
	ROM_RELOAD(                       0x1800000, 0x400000 )
	ROM_LOAD32_WORD_SWAP( "u085.bin", 0x1000002, 0x400000, CRC(a7d5659e) SHA1(10abac022ebe106a3ca7186ff18ca2757f903033) )
	ROM_RELOAD(                       0x1800002, 0x400000 )
//  sprite bug fix?
//  ROM_FILL(                    0x1800000, 0x800000, 0xff )

	ROM_REGION( 0x800000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "u101.bin", 0x000000, 0x800000, CRC(0369491f) SHA1(ca6b1345506f13a17c9bace01637d1f61a278644) )

	ROM_REGION( 0x400000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "u10102.bin", 0x000000, 0x400000, CRC(e28d6855) SHA1(7001a6e298c6a1fcceb79586bf5f4bf0f30027f6) )

	ROM_REGION( 0x400000, "layer2", 0 ) /* Layer 2 */
	ROM_LOAD( "u10103.bin", 0x000000, 0x400000, CRC(0fe91b8e) SHA1(8b71ebeef5e4d2b00fdaaab97776d74e1c96dc59) )

	ROM_REGION( 0x400000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "u0462.bin", 0x000000, 0x400000, CRC(b3d75691) SHA1(71d8dae92be1542a3cff50efeec0bf3c14ab59f5) )

	ROM_REGION( 0x0004, "plds", 0 )
	ROM_LOAD( "atc05-1.bin", 0x0000, 0x0001, NO_DUMP ) /* GAL16V8D-15LP located at U159 */
	ROM_LOAD( "u0259.bin",   0x0000, 0x0001, NO_DUMP ) /* XC9536-15PC44C Located at U0249. (Chip label different then label silk screened onto the board.) */
	ROM_LOAD( "u108.bin",    0x0000, 0x0001, NO_DUMP ) /* XC9536-15PC44C Located at U108. */
	ROM_LOAD( "u084.bin",    0x0000, 0x0001, NO_DUMP ) /* XC9536-15PC44C Located at U084. */

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-guwange.bin", 0x0000, 0x0080, CRC(c3174959) SHA1(29b5c94722756481e4f84bfd75dee15fdb5c8cf7) )
ROM_END


/***************************************************************************

Hotdog Storm
Marble 1996

+------------------------------------------------------+
|       6295   MP1     MP2        6264 6264  LED 68257 |
|            GAL      68257                      68257 |
|  VOL    Y3014 Z80               +--------+           |
|LA4460N     YM2203               |        |       9 8 |
|                                 |  013   |       P P |
|                                 |        |       M M |
|                     68257       +--------+           |
|           68000P12  68257                            |
|J                                                     |
|A 93C46     MP3        +------+  +------+  +------+   |
|M           MP4        | 038  |  | 038  |  | 038  |   |
|M                      |      |  |      |  |      |   |
|A                      +------+  +------+  +------+   |
|                                                      |
|                             4  4      4  4      4  4 |
|                 6264     5  6  6   6  6  6   7  6  6 |
| P1 P2           6264     P  2  2   P  2  2   P  2  2 |
|                    32MHz M  6  6   M  6  6   M  6  6 |
+------------------------------------------------------+

BOARD #:      ASCT9501
CPU:          MC68HC00P12, Z0840006PSC
Sound:        M6295, YM2203C + Y3014B
              LA4460N Sanyo High Gain 51dB, 12W AF Power Amplifier
OSC:          32.000MHz
EEPROM:       ATMEL 93C46
CUSTOM:       038 9341EX702 x3
              013 ?????
P1 & P2       Pushbuttons for Reset & Service
VOL           Volume pot
LED           Power indicator LED

Have seen bootleg boards without the standard JAMMA "key" slot and with a
small daughter card that splits MP8 & MP9 into two roms each:
+-------------+
| 8 9 8 9   ::|
| P P P P   ::|
| M M M M   ::|
+-------------+

***************************************************************************/

ROM_START( hotdogst )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "mp3.u29", 0x00000, 0x80000, CRC(1f4e5479) SHA1(5c3d7b36b1eda4c87c53e4f7cf89951cc5bcc871) )
	ROM_LOAD16_BYTE( "mp4.u28", 0x00001, 0x80000, CRC(6f1c3c4b) SHA1(ab4e4d9b2ef74a2eefda718e120bef05fd0346ff) )

	ROM_REGION( 0x40000, "audiocpu", 0 )    /* Z80 code */
	ROM_LOAD( "mp2.u19", 0x00000, 0x40000, CRC(ff979ebe) SHA1(4cb80086cfdc69a321c7f75455cef89e20488b76) )   // FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x400000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD( "mp9.u55", 0x000000, 0x200000, CRC(258d49ec) SHA1(f39e30c82d8f680f248e1eb59d7c5acb479fa277) )
	ROM_LOAD( "mp8.u54", 0x200000, 0x200000, CRC(bdb4d7b8) SHA1(0dd490988aa84b0e9a21ade5fd606b03eca13f6c) )

	ROM_REGION( 0x80000, "layer0", 0 )  /* Layer 0 */
	ROM_LOAD( "mp7.u56", 0x00000, 0x80000, CRC(87c21c50) SHA1(fc0eea79abdd96edb4fa2c7047aaa728ef838234) )

	ROM_REGION( 0x80000, "layer1", 0 )  /* Layer 1 */
	ROM_LOAD( "mp6.u61", 0x00000, 0x80000, CRC(4dafb288) SHA1(4756259adfe49ba42cde25e7902655b0f0731a6c) )

	ROM_REGION( 0x80000, "layer2", 0 )  /* Layer 2 */
	ROM_LOAD( "mp5.u64", 0x00000, 0x80000, CRC(9b26458c) SHA1(acef62422fa3f92e6ca1eba0ee6fb914cd1ee190) )

	ROM_REGION( 0x80000, "oki1", 0 ) /* Samples */
	ROM_LOAD( "mp1.u65", 0x00000, 0x80000, CRC(4868be1b) SHA1(32b8234b19fdbe07fa5057fa7965e36807e35e77) )   // 1xxxxxxxxxxxxxxxxxx = 0xFF, 4 x 0x20000

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-hotdogst.u14", 0x0000, 0x0080, CRC(12b4f934) SHA1(5b28d8fbd78869db78ce49e541a9d65558841966) )
ROM_END


/***************************************************************************

Koro Koro Quest
Takumi, 1999

Hardware is kind of Banpresto-ish

TUG-01B
MP-001
|--------------------------------------------------|
|        NJM4560   ROM.U0130                       |
|                                     68000        |
|                                                  |
|     YAC516                       16MHz       PAL |
|                                  PAL             |
|                PAL    PAL                        |
|M                 62256                   3V_BATT |
|A    93C46        62256                           |
|H                                                 |
|J                 28MHz  |---------|              |
|O        |------|        |  013    |              |
|N        | 038  |        |  9838E  |     M5M44260 |
|G        | 9838W| 62256  |  X004   |     M5M44260 |
|5        | X004 | 62256  |         |              |
|6        |------|        |---------|              |
|  YMZ280B                                 62256   |
|  16.9344MHz                              62256   |
|                                                  |
|                     PAL                          |
|                                                  |
|ROM.U1186                                 X       |
|                                                  |
|   X       ROM.U1060   ROM.U1051      ROM.U1066   |
|--------------------------------------------------|

 PCB Number - TUG-01B MP001-00175
 68000-16 + 16MHZ OSC
 YMZ280B + YAC516-M + Xtal 16.9344MHz
 93C46 EEPROM
 Custom - 013 9838EX004 (QFP240), 038 9838WX004 (QFP144) + OSC 28MHz
 RAM - 62256 (x8), M5M44260 (x2)
 3volt battery
 GAL16V8H (x5)

***************************************************************************/

ROM_START( korokoro )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "mp-001_ver07.u0130", 0x000000, 0x080000, CRC(86c7241f) SHA1(c9f0ab63c4fe36df1300445e9bb0d5c6a1bb733f) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x180000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD( "mp-001_ver01.u1066", 0x000000, 0x100000, CRC(c5c6af7e) SHA1(13ac26fd703672a01d629be4e5efe9fb8720a4fb) )
	ROM_LOAD( "mp-001_ver01.u1051", 0x100000, 0x080000, CRC(fe5e28e8) SHA1(44da1a7d813b149f9bae351bbcbd0bc2d4c70e10) )  // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "mp-001_ver01.u1060", 0x000000, 0x100000, CRC(ec9cf9d8) SHA1(32fa7120e30c14e484de3b3a9c93efe3654d43c8) )

	ROM_REGION( 0x100000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "mp-001_ver01.u1186", 0x000000, 0x100000, CRC(d16e7c5d) SHA1(1f825ace3ed2e23c8d3212320c4645d3d52214c7) )
ROM_END

ROM_START( crusherm )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "mp-003ver01.u0130", 0x000000, 0x080000, CRC(a4f56e6b) SHA1(1d3af7602c48a6b6c76c376dbc8ad3823b56868a) )

	ROM_REGION( 0x200000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD( "mp-003ver01.u1067", 0x000000, 0x100000, CRC(268a4921) SHA1(8bb818466616051af01680b381af53b8b6a18428) )
	ROM_LOAD( "mp-003ver01.u1066", 0x100000, 0x100000, CRC(79e77a6e) SHA1(9d03dd083769851d628ba6b3d77cfde9603e74f4) )

	ROM_REGION( 0x100000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "mp-003ver01.u1060", 0x000000, 0x100000, CRC(7661893e) SHA1(d51645c96247b039214393ba5eae7357144dfd65) )

	ROM_REGION( 0x200000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "mp-003ver01.u1186", 0x000000, 0x100000, CRC(c3aeb745) SHA1(1bb8ab0512a9a9b0d3ce15f90b49cda431fb14eb) )
	ROM_LOAD( "mp-003ver01.u1187", 0x100000, 0x100000, CRC(d9312497) SHA1(a349cfdbcad96701a74f06394e87f0e0614e115d) )
ROM_END

/***************************************************************************

                                Mazinger Z

Banpresto 1994

U63               038               62256
                  9335EX706         62256
3664                            62256  62256
3664                                U924      32MHz
                                    U24
U60               038             68000
                  9335EX706
3664                                U21   YM2203  92E422
3664                                Z80
                                    3664
                  013
                  9341E7009
U56
U55

62256 62256      514260  514260     U64         M6295

***************************************************************************/

#define ROMS_MAZINGER \
	ROM_REGION( 0x80000, "maincpu", 0 ) \
	ROM_LOAD16_WORD_SWAP( "mzp-0.u24", 0x00000, 0x80000, CRC(43a4279f) SHA1(2c17eb31040bb7f1554bc1c9a968eec5e72af097) ) \
	\
	ROM_REGION16_BE( 0x80000, "user1", 0 ) \
	ROM_LOAD16_WORD_SWAP( "mzp-1.924", 0x00000, 0x80000, CRC(db40acba) SHA1(797a3046b6ab33773c5c4d6bb6d045ea60c1eb45) ) \
	\
	ROM_REGION( 0x20000, "audiocpu", 0 ) \
	ROM_LOAD( "mzs.u21", 0x00000, 0x20000, CRC(c5b4f7ed) SHA1(01f3cd1dd4045029260544e0e1c15dd08817012e) ) \
	\
	ROM_REGION( 0x400000, "sprites0", ROMREGION_ERASEFF ) \
	ROM_LOAD( "bp943a-2.u56", 0x000000, 0x200000, CRC(97e13959) SHA1(c30b1093aacebafefcae701af767dd36fc55fac7) ) \
	ROM_LOAD( "bp943a-3.u55", 0x200000, 0x080000, CRC(9c4957dd) SHA1(e775605a01b6cadc318855ac046dad03c4fc5bb4) ) \
	\
	ROM_REGION( 0x200000, "layer0", 0 ) \
	ROM_LOAD( "bp943a-1.u60", 0x000000, 0x200000, CRC(46327415) SHA1(679d26caefa975569198fac550105c370e2be00d) ) \
	\
	ROM_REGION( 0x200000, "layer1", 0 ) \
	ROM_LOAD( "bp943a-0.u63", 0x000000, 0x200000, CRC(c1fed98a) SHA1(c276505f80a49b129862966a19db507f97153e45) ) \
	\
	ROM_REGION( 0x080000, "oki1", 0 ) \
	ROM_LOAD( "bp943a-4.u64", 0x000000, 0x080000, CRC(3fc7f29a) SHA1(feb21b918243c0a03dfa4a80cc80b86be4f62680) )

/* the regions differ only in the EEPROM, hence the macro above - all EEPROMs are Factory Defaulted */
ROM_START( mazinger )
	ROMS_MAZINGER

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "mazinger_world.nv", 0x0000, 0x0080, CRC(4f6225c6) SHA1(ed8e1c3ca9b961778cd317deb0dd8a0143eaab4f) )
ROM_END

ROM_START( mazingerj )
	ROMS_MAZINGER

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "mazinger_japan.nv", 0x0000, 0x0080, CRC(f84a2a45) SHA1(2e8ad10994bba9a3952889ed0ec3bcaca9b41d03) )
ROM_END

/***************************************************************************

             Metamoqester (World) / Oni - The Ninja Master (Japan)

(C) 1995 Banpresto

PCB: BP947A
CPU: MC68HC000P16 (68000, 64 pin DIP)
SND: Z0840008PSC (Z80, 40 pin DIP), AD-65 x 2 (= OKI M6295), YM2151, CY5002 (= YM3012)
OSC: 32.000 MHz
RAM: LGS GM76C88ALFW-15 x 9 (28 pin SOP), LGS GM71C4260AJ70 x 2 (40 pin SOJ)
     Hitachi HM62256LFP-12T x 2 (40 pin SOJ)

Other Chips:
AT93C46 (EEPROM)
PAL (not dumped, located near 68000): ATF16V8 x 1

GFX:  (Same GFX chips as "Sailor Moon")

      038 9437WX711 (176 pin PQFP)
      038 9437WX711 (176 pin PQFP)
      038 9437WX711 (176 pin PQFP)
      013 9346E7002 (240 pin PQFP)

On PCB near JAMMA connector is a small push button labelled SW1 to access test mode.

ROMS:
BP947A.U37  16M Mask    \ Oki Samples
BP947A.U42  16M Mask    /

BP947A.U46  16M Mask    \
BP947A.U47  16M Mask    |
BP947A.U48  16M Mask    |
BP947A.U49  16M Mask    | GFX
BP947A.U50  16M Mask    |
BP947A.U51  16M Mask    |
BP947A.U52  16M Mask    /

BP947A.U20  27C020        Sound PRG

BP947A.U25  27C240      \
BP947A.U28  27C240      | Main PRG
BP947A.U29  27C240      /

***************************************************************************/

ROM_START( metmqstr )
	ROM_REGION( 0x280000, "maincpu", 0 )        /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "bp947a.u25", 0x000000, 0x80000, CRC(0a5c3442) SHA1(684b79912dedc103f45c42fdebf9983e091b1308) )
	ROM_LOAD16_WORD_SWAP( "bp947a.u28", 0x100000, 0x80000, CRC(8c55decf) SHA1(76c6ce4c8e621273258d31ceb9ec4442fcf1a393) )
	ROM_LOAD16_WORD_SWAP( "bp947a.u29", 0x200000, 0x80000, CRC(cf0f3f3b) SHA1(49a3c0e7536edd53bbf09353e43e9166d736b3f4) )

	ROM_REGION( 0x40000, "audiocpu", 0 )        /* Z80 code */
	ROM_LOAD( "bp947a.u20",  0x00000, 0x40000, CRC(a4a36170) SHA1(ae55094518bd968ea0d04613a133c1421e412012) )

	ROM_REGION( 0x800000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD( "bp947a.u49", 0x000000, 0x200000, CRC(09749531) SHA1(6deeed2712241611ec3202c49a66beed28698af8) )
	ROM_LOAD( "bp947a.u50", 0x200000, 0x200000, CRC(19cea8b2) SHA1(87fb29458074f0e4852237e0184b8b3b44b0eb29) )
	ROM_LOAD( "bp947a.u51", 0x400000, 0x200000, CRC(c19bed67) SHA1(ac664a15512c0e8c8b701833aede95f53cd46a45) )
	ROM_LOAD( "bp947a.u52", 0x600000, 0x200000, CRC(70c64875) SHA1(1c20ab100ccfdf42c97a25e4deb9041b83f5ca8d) )

	ROM_REGION( 0x100000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "bp947a.u48", 0x000000, 0x100000, CRC(04ff6a3d) SHA1(7187db436f7a2ab59a3f5c6ab297b3d740e20f1d) )  // FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(           0x000000, 0x100000             )

	ROM_REGION( 0x100000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "bp947a.u47", 0x000000, 0x100000, CRC(0de42827) SHA1(05d452ca11a31f941cb8a9b0cbb0b59c6b0cbdcb) )  // FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(           0x000000, 0x100000             )

	ROM_REGION( 0x100000, "layer2", 0 ) /* Layer 2 */
	ROM_LOAD( "bp947a.u46", 0x000000, 0x100000, CRC(0f9c906e) SHA1(03872e8be28637df66373bddb04ed91de4f9db75) )  // FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(           0x000000, 0x100000             )

	ROM_REGION( 0x100000, "oki1", 0 )   /* OKIM6295 #1 Samples */
	ROM_LOAD( "bp947a.u42", 0x000000, 0x100000, CRC(2ce8ff2a) SHA1(8ef8c5b7d4a0e60c980c2962e75f7977faafa311) )  // FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(           0x000000, 0x100000             )

	ROM_REGION( 0x100000, "oki2", 0 )   /* OKIM6295 #2 Samples */
	ROM_LOAD( "bp947a.u37", 0x000000, 0x100000, CRC(c3077c8f) SHA1(0a76316a81b7de78279b859549eb5161a721ac71) )  // FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(           0x000000, 0x100000             )
ROM_END

ROM_START( nmaster )
	ROM_REGION( 0x280000, "maincpu", 0 )        /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "bp947a_n.u25",0x000000, 0x80000, CRC(748cc514) SHA1(11d882e77a539407c314f087386e50d691a6bc0b) )
	ROM_LOAD16_WORD_SWAP( "bp947a.u28" , 0x100000, 0x80000, CRC(8c55decf) SHA1(76c6ce4c8e621273258d31ceb9ec4442fcf1a393) )
	ROM_LOAD16_WORD_SWAP( "bp947a.u29",  0x200000, 0x80000, CRC(cf0f3f3b) SHA1(49a3c0e7536edd53bbf09353e43e9166d736b3f4) )

	ROM_REGION( 0x40000, "audiocpu", 0 )        /* Z80 code */
	ROM_LOAD( "bp947a.u20",  0x00000, 0x40000, CRC(a4a36170) SHA1(ae55094518bd968ea0d04613a133c1421e412012) )

	ROM_REGION( 0x800000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD( "bp947a.u49", 0x000000, 0x200000, CRC(09749531) SHA1(6deeed2712241611ec3202c49a66beed28698af8) )
	ROM_LOAD( "bp947a.u50", 0x200000, 0x200000, CRC(19cea8b2) SHA1(87fb29458074f0e4852237e0184b8b3b44b0eb29) )
	ROM_LOAD( "bp947a.u51", 0x400000, 0x200000, CRC(c19bed67) SHA1(ac664a15512c0e8c8b701833aede95f53cd46a45) )
	ROM_LOAD( "bp947a.u52", 0x600000, 0x200000, CRC(70c64875) SHA1(1c20ab100ccfdf42c97a25e4deb9041b83f5ca8d) )

	ROM_REGION( 0x100000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "bp947a.u48", 0x000000, 0x100000, CRC(04ff6a3d) SHA1(7187db436f7a2ab59a3f5c6ab297b3d740e20f1d) )  // FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(           0x000000, 0x100000             )

	ROM_REGION( 0x100000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "bp947a.u47", 0x000000, 0x100000, CRC(0de42827) SHA1(05d452ca11a31f941cb8a9b0cbb0b59c6b0cbdcb) )  // FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(           0x000000, 0x100000             )

	ROM_REGION( 0x100000, "layer2", 0 ) /* Layer 2 */
	ROM_LOAD( "bp947a.u46", 0x000000, 0x100000, CRC(0f9c906e) SHA1(03872e8be28637df66373bddb04ed91de4f9db75) )  // FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(           0x000000, 0x100000             )

	ROM_REGION( 0x100000, "oki1", 0 )   /* OKIM6295 #1 Samples */
	ROM_LOAD( "bp947a.u42", 0x000000, 0x100000, CRC(2ce8ff2a) SHA1(8ef8c5b7d4a0e60c980c2962e75f7977faafa311) )  // FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(           0x000000, 0x100000             )

	ROM_REGION( 0x100000, "oki2", 0 )   /* OKIM6295 #2 Samples */
	ROM_LOAD( "bp947a.u37", 0x000000, 0x100000, CRC(c3077c8f) SHA1(0a76316a81b7de78279b859549eb5161a721ac71) )  // FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(           0x000000, 0x100000             )
ROM_END


/***************************************************************************

  Pac-Slot by Namco, 1996 (according to http://pacman.com/ja/museum/index.html)
  Namco N-44 EM VIDEO platform, PCB A0442

  TMP 68HC000P-16

  013 9345E7006
  038 9444WX010

  OKI M6295 x 2

  Battery
  93C46 EEPROM (at U24)

  28MHz XTAL

***************************************************************************/

ROM_START( pacslot )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pa1-mprob.u41", 0x00000, 0x80000, CRC(56281370) SHA1(b75a7c5997adac14486cef7be4e41d113c86021f) )

	ROM_REGION( 0x100000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD16_BYTE( "pa1-obj0.u52", 0x00000, 0x80000, CRC(bf9232ce) SHA1(9a887a964e9a75e16c59dcf217c664404e74cc2a) )
	ROM_LOAD16_BYTE( "pa1-obj1.u53", 0x00001, 0x80000, CRC(6eb76a04) SHA1(66c8e36bee4439c203a02b30898e4f741205d681) )

	ROM_REGION( 0x80000, "layer0", 0 )  /* Layer 0 */
	ROM_LOAD16_BYTE( "pa1-cha0.u60", 0x00000, 0x40000, CRC(314b51a6) SHA1(eef102c4f0c0e0f668a7cf228cd4fbe45b2ce45f) )
	ROM_LOAD16_BYTE( "pa1-cha1.u61", 0x00001, 0x40000, CRC(f7a2c846) SHA1(3b505a7a3c7f30e6cd87803f5ae7e962205fc1f0) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* OKIM6295 #1 Samples */
	ROM_LOAD( "pa1-voi0.u27", 0x00000, 0x40000, CRC(e3e623e1) SHA1(396accc7f7384277b700f019b5083def8a48ccd7) )

	ROM_REGION( 0x40000, "oki2", ROMREGION_ERASE00 )    /* OKIM6295 #2 Samples */
	// empty ROM socket

	ROM_REGION( 0x117 * 3, "plds", 0 )
	ROM_LOAD( "n44u1a.u1",   0x117*0, 0x117, NO_DUMP )  // GAL16V8B-15LP (Protected)
	ROM_LOAD( "n44u3a.u3",   0x117*1, 0x117, NO_DUMP )  // GAL16V8B-15LP (Protected)
	ROM_LOAD( "n44u51a.u51", 0x117*2, 0x117, CRC(3c5e9bc5) SHA1(b4e04c4fa91ff33542b73971f67e71d13e24c5ec) )  // GAL16V8B-15LP (Protected, dumped from the paceight PCB)
ROM_END


/***************************************************************************

  Pac-Eight by Namco, 1996 (according to http://pacman.com/ja/museum/index.html)
  Namco N-44 EM VIDEO platform, PCB C0348

  TMP 68HC000P-16

  013 9341E7002
  038 9635WY003

  OKI M6295 x 2

  Battery
  93C46 EEPROM (at U24)

  28MHz XTAL

***************************************************************************/

ROM_START( paceight )
	ROM_REGION( 0x80000, "maincpu", 0 )        /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pae1-mpro.u41", 0x00000, 0x80000, CRC(bb026f97) SHA1(70d48f05275c64b25f37f03206219ef3ee9c0ee2) ) // 27c240

	ROM_REGION( 0x100000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD16_BYTE( "pae1-obj0.u52", 0x00000, 0x80000, CRC(2cd99155) SHA1(146ed2b3f2763232a60e6b238a16067d3ccfa959) ) // 27c040
	ROM_LOAD16_BYTE( "pae1-obj1.u53", 0x00001, 0x80000, CRC(9ae2685b) SHA1(5eed5f00d28d803358c8ffaf42c4979af23a0a8c) ) // ""

	ROM_REGION( 0x80000, "layer0", 0 )  /* Layer 0 */
	ROM_LOAD16_BYTE( "pae1-cha0.u60", 0x00000, 0x40000, CRC(757263e3) SHA1(668060e9e209752474f48362752a3f819ff82d72) ) // 27c020? not readable
	ROM_LOAD16_BYTE( "pae1-cha1.u61", 0x00001, 0x40000, CRC(0396d241) SHA1(79382805fa4486d8dae792f9afc0f02aee1bbb33) ) // ""

	ROM_REGION( 0x40000, "oki1", 0 )    /* OKIM6295 #1 Samples */
	ROM_LOAD( "pae1-vo10.u27", 0x00000, 0x40000, CRC(0be7b94f) SHA1(4179e2ab2d2d1df0cc6cfd71e277ea114578f147) ) // 27c? not readable

	ROM_REGION( 0x40000, "oki2", ROMREGION_ERASE00 )    /* OKIM6295 #2 Samples */
	// empty ROM socket

	ROM_REGION( 0x117 * 3, "plds", 0 )
	ROM_LOAD( "n44u1c.u1",   0x117*0, 0x117, CRC(903fc2d8) SHA1(becbae356efde873225ef64af462d9702aac03f0) )  // GAL16V8B-15LP
	ROM_LOAD( "n44u3c.u3",   0x117*1, 0x117, CRC(72201412) SHA1(6ad7d22e612e27343eac5c38f00d548df644d52c) )  // GAL16V8B-15LP
	ROM_LOAD( "n44u51a.u51", 0x117*2, 0x117, CRC(3c5e9bc5) SHA1(b4e04c4fa91ff33542b73971f67e71d13e24c5ec) )  // GAL16V8B-15LP
ROM_END

/***************************************************************************

  Pac-Carnival by Namco, 1996
  Namco N-44 EM VIDEO platform, PCB B0445

  TMP 68HC000P-16

  013 9341E7005
  038 9429WX704

  OKI M6295 x 2

  Battery
  93C46 EEPROM (at U24)

  28MHz XTAL

***************************************************************************/

ROM_START( paccarn )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "pl1-mpr0b.u41", 0x00000, 0x80000, CRC(ef6b08ea) SHA1(61fe4db433a154233c4e1efd248ad51ba1d265d0) )

	ROM_REGION( 0x100000, "sprites0", 0 )
	ROM_LOAD16_BYTE( "pl1-obj0.u52", 0x00000, 0x80000, CRC(1dd7c292) SHA1(e78d4be35c24616a1954910039e381869246a246) )
	ROM_LOAD16_BYTE( "pl1-obj1.u53", 0x00001, 0x80000, CRC(7b9935d0) SHA1(ef05230689124d0999ba9e472968e04d6e75c405) )

	ROM_REGION( 0x80000, "layer0", 0 )
	ROM_LOAD16_BYTE( "pl1-cha0.u60", 0x00000, 0x40000, CRC(7977662e) SHA1(5aaa69ffaa62b20196a7b638232716b7c6391490) )
	ROM_LOAD16_BYTE( "pl1-cha1.u61", 0x00001, 0x40000, CRC(2150eafa) SHA1(e744a0861ee8a76c2bb24f681df38966df5dc9f0) )

	ROM_REGION( 0x40000, "oki1", 0 )
	ROM_LOAD( "pl1-voi0.u27", 0x00000, 0x40000, CRC(e22b87e3) SHA1(83a6952fb0f695bb74cc5a3a1e24a916b6de9c8e) )

	ROM_REGION( 0x40000, "oki2", ROMREGION_ERASE00 )
	// empty ROM socket

	ROM_REGION( 0x117 * 3, "plds", 0 ) // all protected
	ROM_LOAD( "n44u1b.u1",   0x117*0, 0x117, NO_DUMP )
	ROM_LOAD( "n44u3b.u3",   0x117*1, 0x117, NO_DUMP )
	ROM_LOAD( "n44u51a.u51", 0x117*2, 0x117, NO_DUMP )
ROM_END

/***************************************************************************

  Poka Poka Satan - wack-a-mole game with one frontal upright screen and two
                    table-top touch screens to bang on with plastic "hammers"

  PPS-MAIN (Sticker: 96. 4. 5 017)

  TMP 68HC000P-16

  013 9607EX013 x 3
  038 9444WX010 x 3

  OKI M6295

  DSW8
  BR93LC46 EEPROM

  16MHz XTAL
  28MHz XTAL

***************************************************************************/

ROM_START( ppsatan )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASEFF )        /* 68000 Code */
	ROM_LOAD16_BYTE( "66a5.u79", 0x00000, 0x20000, CRC(60efeed3) SHA1(72095cef77065a8f1089273050f60a2e99582cf1) ) // checksum = 60D5 (OK?). 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "43b1.u61", 0x00001, 0x20000, CRC(f14e6287) SHA1(75c0465780a10ec8f533349b008f0d489bf362a5) ) // checksum = 43B1 (OK).  1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD16_BYTE( "ver1.0.u27", 0x00000, 0x80000, CRC(d1b02639) SHA1(19bbcf951a6ace91da72af9232f3d808afa8416c) )
	ROM_LOAD16_BYTE( "ver1.0.u17", 0x00001, 0x80000, CRC(c66730ca) SHA1(75c18c80c1d2ced69edd4f013685c4eaf015049c) )

	ROM_REGION( 0x200000, "sprites1", 0 )        /* Sprites: * 2 */
	ROM_LOAD16_BYTE( "ver1.0.u13", 0x00000, 0x80000, CRC(24c31e01) SHA1(c2c96bdd0a2a764ac0e1c8d64334d0ab76c46aa5) )
	ROM_LOAD16_BYTE( "ver1.0.u19", 0x00001, 0x80000, CRC(ffbc6284) SHA1(05a735f3193218d32ad253c5abe21e1d00d1a5ca) )

	ROM_REGION( 0x200000, "sprites2", 0 )        /* Sprites: * 2 */
	ROM_LOAD16_BYTE( "ver1.0.u15", 0x00000, 0x80000, CRC(24c31e01) SHA1(c2c96bdd0a2a764ac0e1c8d64334d0ab76c46aa5) )
	ROM_LOAD16_BYTE( "ver1.0.u23", 0x00001, 0x80000, CRC(ffbc6284) SHA1(05a735f3193218d32ad253c5abe21e1d00d1a5ca) )

	ROM_REGION( 0x80000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "ver1.0.u57", 0x00000, 0x80000, CRC(5faa697a) SHA1(308ea0a4dee7510b3bdd1b3b3c0a86c6508df40b) ) // 1xxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x80000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "ver1.0.u49", 0x00000, 0x80000, CRC(f21787b0) SHA1(e29ffcf948ef55f8ee11949903e5a363e6c4fa44) ) // 1xxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x80000, "layer2", 0 ) /* Layer 2 */
	ROM_LOAD( "ver1.0.u53", 0x00000, 0x80000, CRC(f21787b0) SHA1(e29ffcf948ef55f8ee11949903e5a363e6c4fa44) ) // 1xxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x80000, "oki1", 0 ) /* Samples */
	ROM_LOAD( "7a1f.u83", 0x000000, 0x80000, CRC(2ae77933) SHA1(a5eb0915813efd538b0812a6bbd5239b4b203f4a) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "u90.u90",   0x000, 0x117, CRC(dae5e82a) SHA1(718ad537a917a0fdc3ef3c2307f04499a0029451) )
	ROM_LOAD( "u91.u91",   0x000, 0x117, CRC(28382623) SHA1(9fd4b4964f71807ae35a92bd5e81d6d1a5fdf469) )
	ROM_LOAD( "u92.u92",   0x000, 0x117, CRC(40b16ace) SHA1(1cdf39743713109e9510aeb6323df2bedfddbeb1) )
	ROM_LOAD( "u94.u94",   0x000, 0x117, CRC(eefe343a) SHA1(c7627d20711c3bf00f5d498d67ff43c8a0962a23) )
	ROM_LOAD( "u95.u95",   0x000, 0x117, CRC(b1a78112) SHA1(65a4da294c74ce4d36ceead8284b79be6cbb6379) )
	ROM_LOAD( "u111.u111", 0x000, 0x117, CRC(7f93cbdd) SHA1(7432b21c83249ab53d8f37e0c6fffd28f8de4ef3) )
ROM_END


/***************************************************************************

           Power Instinct 2 (USA) / Gouketsuji Ichizoku 2 (Japan)

(c)1994 Atlus
CPU: 68000, Z80
Sound: YM2203, AR17961 (x2)
Custom: NMK 112 (M6295 sample ROM banking), Atlus 8647-01  013, 038 (x4)
X1 = 12 MHz
X2 = 28 MHz
X3 = 16 MHz

***************************************************************************/

ROM_START( pwrinst2 )
	ROM_REGION( 0x200000, "maincpu", 0 )        /* 68000 code */
	ROM_LOAD16_BYTE( "g02.u45", 0x000000, 0x80000, CRC(7b33bc43) SHA1(a68eb94e679f03c354932b8c5cd1bb2922fec0aa) )
	ROM_LOAD16_BYTE( "g02.u44", 0x000001, 0x80000, CRC(8f6f6637) SHA1(024b12c0fe40e27c79e38bd7601a9183a62d75fd) )
	ROM_LOAD16_BYTE( "g02.u43", 0x100000, 0x80000, CRC(178e3d24) SHA1(926234f4196a5d5e3bd1438abbf73355f2c65b06) )
	ROM_LOAD16_BYTE( "g02.u42", 0x100001, 0x80000, CRC(a0b4ee99) SHA1(c6df4aa2543b04d8bda7683f503e5eb763e506af) )

	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* 68000 extra data roms */
	/* not used */

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Z80 code */
	ROM_LOAD( "g02.u3a", 0x00000, 0x20000, CRC(ebea5e1e) SHA1(4d3af9e5f29d0c1b26563f51250039c9e8bd3735) )

	ROM_REGION( 0xe00000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD( "g02.u61", 0x000000, 0x200000, CRC(91e30398) SHA1(2b59a5e40bed2a988382054fe30d92808dad3348) )
	ROM_LOAD( "g02.u62", 0x200000, 0x200000, CRC(d9455dd7) SHA1(afa69fe9a540cd78b8cfecf09cffa1401c01141a) )
	ROM_LOAD( "g02.u63", 0x400000, 0x200000, CRC(4d20560b) SHA1(ceaee8cf0b69cc366b95ddcb689a5594d79e5114) )
	ROM_LOAD( "g02.u64", 0x600000, 0x200000, CRC(b17b9b6e) SHA1(fc6213d8322cda4c7f653e2d7d6d314ce84c97b7) )
	ROM_LOAD( "g02.u65", 0x800000, 0x200000, CRC(08541878) SHA1(138cf077a49a26440a3da1bdc2c399a208359e57) )
	ROM_LOAD( "g02.u66", 0xa00000, 0x200000, CRC(becf2a36) SHA1(f8b386d0292b1dc745b7253a3df51d1aa8d5e9db) )
	ROM_LOAD( "g02.u67", 0xc00000, 0x200000, CRC(52fe2b8b) SHA1(dd50aa62f7db995e28f47de9b3fb749aeeaaa5b0) )

	ROM_REGION( 0x200000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "g02.u78", 0x000000, 0x200000, CRC(1eca63d2) SHA1(538942b43301f950e3d5139461331c54dc90129d) )

	ROM_REGION( 0x100000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "g02.u81", 0x000000, 0x100000, CRC(8a3ff685) SHA1(4a59ec50ec4470453374fe10f76d3e894494b49f) )

	ROM_REGION( 0x100000, "layer2", 0 ) /* Layer 2 */
	ROM_LOAD( "g02.u89", 0x000000, 0x100000, CRC(373e1f73) SHA1(ec1ae9fab37eee41be8e1bc6dad03809b62fdbce) )

	ROM_REGION( 0x080000, "layer3", 0 ) /* Layer 3 */
	ROM_LOAD( "g02.82a", 0x000000, 0x080000, CRC(4b3567d6) SHA1(d3e14783b312d2bea9722a8e3c22bcec81e26166) )

	ROM_REGION( 0x440000, "oki1", 0 )   /* OKIM6295 #1 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "g02.u53", 0x040000, 0x200000, CRC(c4bdd9e0) SHA1(a938a831e789ddf6f3cc5f3e5f3877ec7bd62d4e) )
	ROM_LOAD( "g02.u54", 0x240000, 0x200000, CRC(1357d50e) SHA1(433766177ce9d6933f90de85ba91bfc6d8d5d664) )

	ROM_REGION( 0x440000, "oki2", 0 )   /* OKIM6295 #2 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "g02.u55", 0x040000, 0x200000, CRC(2d102898) SHA1(bd81f4cd2ba100707db0c5bb1419f0b23c998574) )
	ROM_LOAD( "g02.u56", 0x240000, 0x200000, CRC(9ff50dda) SHA1(1121685e387c20e228032f2b0f5cbb606376fc15) )
ROM_END

ROM_START( pwrinst2j )
	ROM_REGION( 0x200000, "maincpu", 0 )        /* 68000 code */
	ROM_LOAD16_BYTE( "g02j.u45", 0x000000, 0x80000, CRC(42d0abd7) SHA1(c58861d43c4539ccc8b2f93eabc56aab37d3aa34))
	ROM_LOAD16_BYTE( "g02j.u44", 0x000001, 0x80000, CRC(362b7af3) SHA1(2d15611530cef76f0f9c82ee0411966079ae19c3))
	ROM_LOAD16_BYTE( "g02j.u43", 0x100000, 0x80000, CRC(c94c596b) SHA1(ee755a344f769e3ed05d8ca57f517b9e8c02f22e) )
	ROM_LOAD16_BYTE( "g02j.u42", 0x100001, 0x80000, CRC(4f4c8270) SHA1(1fa964f5646bd1d078e3661c21e191b0789c05c9) )

	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* 68000 extra data roms */
	/* not used */

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Z80 code */
	ROM_LOAD( "g02j.u3a", 0x00000, 0x20000, CRC(eead01f1) SHA1(0ced6755e471e0303fe397b3d54a5c799762ebd8) )

	ROM_REGION( 0xe00000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD( "g02.u61", 0x000000, 0x200000, CRC(91e30398) SHA1(2b59a5e40bed2a988382054fe30d92808dad3348) )
	ROM_LOAD( "g02.u62", 0x200000, 0x200000, CRC(d9455dd7) SHA1(afa69fe9a540cd78b8cfecf09cffa1401c01141a) )
	ROM_LOAD( "g02.u63", 0x400000, 0x200000, CRC(4d20560b) SHA1(ceaee8cf0b69cc366b95ddcb689a5594d79e5114) )
	ROM_LOAD( "g02.u64", 0x600000, 0x200000, CRC(b17b9b6e) SHA1(fc6213d8322cda4c7f653e2d7d6d314ce84c97b7) )
	ROM_LOAD( "g02.u65", 0x800000, 0x200000, CRC(08541878) SHA1(138cf077a49a26440a3da1bdc2c399a208359e57) )
	ROM_LOAD( "g02.u66", 0xa00000, 0x200000, CRC(becf2a36) SHA1(f8b386d0292b1dc745b7253a3df51d1aa8d5e9db) )
	ROM_LOAD( "g02.u67", 0xc00000, 0x200000, CRC(52fe2b8b) SHA1(dd50aa62f7db995e28f47de9b3fb749aeeaaa5b0) )

	ROM_REGION( 0x200000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "g02.u78", 0x000000, 0x200000, CRC(1eca63d2) SHA1(538942b43301f950e3d5139461331c54dc90129d) )

	ROM_REGION( 0x100000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "g02.u81", 0x000000, 0x100000, CRC(8a3ff685) SHA1(4a59ec50ec4470453374fe10f76d3e894494b49f) )

	ROM_REGION( 0x100000, "layer2", 0 ) /* Layer 2 */
	ROM_LOAD( "g02.u89", 0x000000, 0x100000, CRC(373e1f73) SHA1(ec1ae9fab37eee41be8e1bc6dad03809b62fdbce) )

	ROM_REGION( 0x080000, "layer3", 0 ) /* Layer 3 */
	ROM_LOAD( "g02j.82a", 0x000000, 0x080000, CRC(3be86fe1) SHA1(313bfe5fb8dc5fee4462db259738e079759f9390) )

	ROM_REGION( 0x440000, "oki1", 0 )   /* OKIM6295 #1 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "g02.u53", 0x040000, 0x200000, CRC(c4bdd9e0) SHA1(a938a831e789ddf6f3cc5f3e5f3877ec7bd62d4e) )
	ROM_LOAD( "g02.u54", 0x240000, 0x200000, CRC(1357d50e) SHA1(433766177ce9d6933f90de85ba91bfc6d8d5d664) )

	ROM_REGION( 0x440000, "oki2", 0 )   /* OKIM6295 #2 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "g02.u55", 0x040000, 0x200000, CRC(2d102898) SHA1(bd81f4cd2ba100707db0c5bb1419f0b23c998574) )
	ROM_LOAD( "g02.u56", 0x240000, 0x200000, CRC(9ff50dda) SHA1(1121685e387c20e228032f2b0f5cbb606376fc15) )
ROM_END

/*

Power Instinct Legends (US) / Gouketsuji Ichizoku Saikyou Densetsu (Japan)
Atlus, 1995

PCB Layout
----------

AT047G2-B ATLUS
|---------------------------------------------------------------|
|LM324 M6295  G02_U53          Z80  16MHz 28MHz 12MHz  TA8030S  |
|VOL          G02_U54 |------| SOUND_U3                TEXT_U82 |
|      M6295  G02_U55 |NMK112|   6264         6264              |
|uPC2505      G02_U56 |      |                        |------|  |
|      4558           |------|                6264    |038   |  |
|     Y3014   YM2203    PAL                           |9429WX|  |
|                                            ATGS_U89 |------|  |
|J       TEST_SW  62256                                         |
|A    93C46       62256                       6264    |------|  |
|M            |----SUB-BOARD-----|                    |038   |  |
|M    |---|   |*P  P *P  P *P *P |      PAL   6264    |9429WX|  |
|A    |   |   | R  R  R  R  R  R |                    |------|  |
|     | 6 |   | 1  O  1  O  1  1 |           ATGS_U81           |
|     | 8 |   | 2  G  2  G  2  2 |                    |------|  |
|     | 0 |   | U  U  U  U  U  U |            6264    |038   |  |
|     | 0 |   | 2  4  4  4  3  5 |                    |9429WX|  |
|     | 0 |   |    5     4       |62256       6264    |------|  |
|     |   |   |------------------|62256                         |
|     |---|     PAL            |-------|     ATGS_U78 |------|  |
|--------|                     |8647-01|              |038   |  |
|*ATGS_U1|                     |013    |    KM416C256 |9429WX|  |
|        |                     |9341E70|              |------|  |
|        |G02_U66    G02_U63   |-------|                6264    |
|        |  G02_U65    G02_U62   62256      KM416C256           |
|*ATGS_U2|    G02_U64    G02_U61 62256                  6264    |
|--------|------------------------------------------------------|
Notes:
      ROMs marked with * are located on a plug-in sub board
      68000 clock - 16.000MHz
      Z80 clock   - 8.000MHz [16/2]
      6295 clocks - 3.000MHz [12/4], sample rate = 3000000 / 165
      YM2203 clock- 4.000MHz [16/4]
      VSync       - 57.5Hz
      HSync       - 15.23kHz

      ROMs -
            U3       : 27C1001 EPROM
            U82      : 27C040 EPROM
            PR12*    : 27C040 EPROMs
            PROG*    : 27C040 EPROMs
            ALL other ROMs are soldered-in 16M 42 pin mask ROM (read as 27C160)
*/

ROM_START( plegends )
	ROM_REGION( 0x200000, "maincpu", 0 )        /* 68000 code */
	ROM_LOAD16_BYTE( "d12.u45", 0x000000, 0x80000, CRC(ed8a2e3d) SHA1(0a09c58cd8a726189cd7679d06343e0b8c3de945) )
	ROM_LOAD16_BYTE( "d13.u44", 0x000001, 0x80000, CRC(25821731) SHA1(7c6ece92b36dc7eb489879d9ae3e8af9380b9f62) )
	ROM_LOAD16_BYTE( "d14.u2",  0x100000, 0x80000, CRC(c2cb1402) SHA1(78e70915ca32b97c22605a304dc8611e1fe01ae9) ) /* Contains text strings */
	ROM_LOAD16_BYTE( "d16.u3",  0x100001, 0x80000, CRC(50a1c63e) SHA1(5a8431a81aa61034e67141944b9e7cf97842773a) ) /* Contains text strings */

	ROM_REGION16_BE( 0x100000, "user1", 0 ) /* 68000 extra data roms */
	ROM_LOAD16_BYTE( "d15.u4",  0x000000, 0x80000, CRC(6352cec0) SHA1(a54d55b8d642e438158268d0d41880b6589e48e2) )
	ROM_LOAD16_BYTE( "d17.u5",  0x000001, 0x80000, CRC(7af810d8) SHA1(5e24f78a228809a001f3f3372c1b32ea05070e17) )

	ROM_REGION( 0x40000, "audiocpu", 0 )        /* Z80 code */
	ROM_LOAD( "d19.u3", 0x00000, 0x40000, CRC(47598459) SHA1(4e9dcfebfbd160230768965e8c6e5ed446c1aa7b) ) /* Same as sound.u3 below, but twice the size? */

	ROM_REGION( 0x1000000, "sprites0", 0 )       /* Sprites: * 2 */
	ROM_LOAD( "g02.u61", 0x000000, 0x200000, CRC(91e30398) SHA1(2b59a5e40bed2a988382054fe30d92808dad3348) )
	ROM_LOAD( "g02.u62", 0x200000, 0x200000, CRC(d9455dd7) SHA1(afa69fe9a540cd78b8cfecf09cffa1401c01141a) )
	ROM_LOAD( "g02.u63", 0x400000, 0x200000, CRC(4d20560b) SHA1(ceaee8cf0b69cc366b95ddcb689a5594d79e5114) )
	ROM_LOAD( "g02.u64", 0x600000, 0x200000, CRC(b17b9b6e) SHA1(fc6213d8322cda4c7f653e2d7d6d314ce84c97b7) )
	ROM_LOAD( "g02.u65", 0x800000, 0x200000, CRC(08541878) SHA1(138cf077a49a26440a3da1bdc2c399a208359e57) )
	ROM_LOAD( "g02.u66", 0xa00000, 0x200000, CRC(becf2a36) SHA1(f8b386d0292b1dc745b7253a3df51d1aa8d5e9db) )
	ROM_LOAD( "atgs.u1", 0xc00000, 0x200000, CRC(aa6f34a9) SHA1(00de85de1b413bd2c46931c13365f8556b50b634) ) /* US version's rom labeled "sp6_u67-1" */
	ROM_LOAD( "atgs.u2", 0xe00000, 0x200000, CRC(553eda27) SHA1(5b9126f966f0c64b3ac7c06526064d71e4df60c5) ) /* US version's rom labeled "sp6_u67-2" */

	ROM_REGION( 0x200000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "atgs.u78", 0x000000, 0x200000, CRC(16710ecb) SHA1(6277f7f6095457df649932550b04242e5853ec5e) ) /* US version's rom labeled "bg0_u78" */

	ROM_REGION( 0x200000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "atgs.u81", 0x000000, 0x200000, CRC(cb2aca91) SHA1(869f0f2db35c45ec90b74d33d521cbb598e60a3f) ) /* US version's rom labeled "bg1_u81" */

	ROM_REGION( 0x200000, "layer2", 0 ) /* Layer 2 */
	ROM_LOAD( "atgs.u89", 0x000000, 0x200000, CRC(65f45a0f) SHA1(b7f4b56308dcdc144100d0a92d91255459a320a4) ) /* US version's rom labeled "bg2_u89" */

	ROM_REGION( 0x080000, "layer3", 0 ) /* Layer 3 */
	ROM_LOAD( "text.u82", 0x000000, 0x080000, CRC(f57333ea) SHA1(409d8005ffcf91943e4a743b2434ce425f5bdc36) ) /* US version's rom labeled "d20" */

	ROM_REGION( 0x440000, "oki1", 0 )   /* OKIM6295 #1 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "g02.u53", 0x040000, 0x200000, CRC(c4bdd9e0) SHA1(a938a831e789ddf6f3cc5f3e5f3877ec7bd62d4e) )
	ROM_LOAD( "g02.u54", 0x240000, 0x200000, CRC(1357d50e) SHA1(433766177ce9d6933f90de85ba91bfc6d8d5d664) )

	ROM_REGION( 0x440000, "oki2", 0 )   /* OKIM6295 #2 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "g02.u55", 0x040000, 0x200000, CRC(2d102898) SHA1(bd81f4cd2ba100707db0c5bb1419f0b23c998574) )
	ROM_LOAD( "g02.u56", 0x240000, 0x200000, CRC(9ff50dda) SHA1(1121685e387c20e228032f2b0f5cbb606376fc15) )
ROM_END

ROM_START( plegendsj )
	ROM_REGION( 0x200000, "maincpu", 0 )        /* 68000 code */
	ROM_LOAD16_BYTE( "prog.u45", 0x000000, 0x80000, CRC(94f53db2) SHA1(34c671f160cfcb7d46cc964731ff2b77dc0be928) )
	ROM_LOAD16_BYTE( "prog.u44", 0x000001, 0x80000, CRC(db0ad756) SHA1(9c1510491cdc9442062ee3bd8a1bb93f00d33d97) )
	ROM_LOAD16_BYTE( "pr12.u2",  0x100000, 0x80000, CRC(0e202559) SHA1(217a8e47d5c679aff02ca43de1641230e4f78b01) ) /* Contains text in Japanese */
	ROM_LOAD16_BYTE( "pr12.u3",  0x100001, 0x80000, CRC(54742f21) SHA1(fae7bb7381478eb077f0409acd521f77417aa968) ) /* Contains text in Japanese */

	ROM_REGION16_BE( 0x100000, "user1", 0 ) /* 68000 extra data roms */
	ROM_LOAD16_BYTE( "d15.u4",  0x000000, 0x80000, CRC(6352cec0) SHA1(a54d55b8d642e438158268d0d41880b6589e48e2) )
	ROM_LOAD16_BYTE( "d17.u5",  0x000001, 0x80000, CRC(7af810d8) SHA1(5e24f78a228809a001f3f3372c1b32ea05070e17) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Z80 code */
	ROM_LOAD( "sound.u3", 0x00000, 0x20000, CRC(36f71520) SHA1(11d0a059ddba3e1aa4c54ccdde7b3f5c7bde482f) )

	ROM_REGION( 0x1000000, "sprites0", 0 )       /* Sprites: * 2 */
	ROM_LOAD( "g02.u61", 0x000000, 0x200000, CRC(91e30398) SHA1(2b59a5e40bed2a988382054fe30d92808dad3348) )
	ROM_LOAD( "g02.u62", 0x200000, 0x200000, CRC(d9455dd7) SHA1(afa69fe9a540cd78b8cfecf09cffa1401c01141a) )
	ROM_LOAD( "g02.u63", 0x400000, 0x200000, CRC(4d20560b) SHA1(ceaee8cf0b69cc366b95ddcb689a5594d79e5114) )
	ROM_LOAD( "g02.u64", 0x600000, 0x200000, CRC(b17b9b6e) SHA1(fc6213d8322cda4c7f653e2d7d6d314ce84c97b7) )
	ROM_LOAD( "g02.u65", 0x800000, 0x200000, CRC(08541878) SHA1(138cf077a49a26440a3da1bdc2c399a208359e57) )
	ROM_LOAD( "g02.u66", 0xa00000, 0x200000, CRC(becf2a36) SHA1(f8b386d0292b1dc745b7253a3df51d1aa8d5e9db) )
	ROM_LOAD( "atgs.u1", 0xc00000, 0x200000, CRC(aa6f34a9) SHA1(00de85de1b413bd2c46931c13365f8556b50b634) ) /* US version's rom labeled "sp6_u67-1" */
	ROM_LOAD( "atgs.u2", 0xe00000, 0x200000, CRC(553eda27) SHA1(5b9126f966f0c64b3ac7c06526064d71e4df60c5) ) /* US version's rom labeled "sp6_u67-2" */

	ROM_REGION( 0x200000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "atgs.u78", 0x000000, 0x200000, CRC(16710ecb) SHA1(6277f7f6095457df649932550b04242e5853ec5e) ) /* US version's rom labeled "bg0_u78" */

	ROM_REGION( 0x200000, "layer1", 0 ) /* Layer 1 */
	ROM_LOAD( "atgs.u81", 0x000000, 0x200000, CRC(cb2aca91) SHA1(869f0f2db35c45ec90b74d33d521cbb598e60a3f) ) /* US version's rom labeled "bg1_u81" */

	ROM_REGION( 0x200000, "layer2", 0 ) /* Layer 2 */
	ROM_LOAD( "atgs.u89", 0x000000, 0x200000, CRC(65f45a0f) SHA1(b7f4b56308dcdc144100d0a92d91255459a320a4) ) /* US version's rom labeled "bg2_u89" */

	ROM_REGION( 0x080000, "layer3", 0 ) /* Layer 3 */
	ROM_LOAD( "text.u82", 0x000000, 0x080000, CRC(f57333ea) SHA1(409d8005ffcf91943e4a743b2434ce425f5bdc36) ) /* US version's rom labeled "d20" */

	ROM_REGION( 0x440000, "oki1", 0 )   /* OKIM6295 #1 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "g02.u53", 0x040000, 0x200000, CRC(c4bdd9e0) SHA1(a938a831e789ddf6f3cc5f3e5f3877ec7bd62d4e) )
	ROM_LOAD( "g02.u54", 0x240000, 0x200000, CRC(1357d50e) SHA1(433766177ce9d6933f90de85ba91bfc6d8d5d664) )

	ROM_REGION( 0x440000, "oki2", 0 )   /* OKIM6295 #2 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "g02.u55", 0x040000, 0x200000, CRC(2d102898) SHA1(bd81f4cd2ba100707db0c5bb1419f0b23c998574) )
	ROM_LOAD( "g02.u56", 0x240000, 0x200000, CRC(9ff50dda) SHA1(1121685e387c20e228032f2b0f5cbb606376fc15) )
ROM_END


/***************************************************************************

                         Pretty Soldier Sailor Moon

(C) 1995 Banpresto
PCB: BP945A
CPU: TMP68HC000P16 (68000, 64 pin DIP)
SND: Z84C0008PEC (Z80, 40 pin DIP), OKI M6295 x 2, YM2151, YM3012
OSC: 28.000MHz, 16.000MHz
RAM: NEC 43256 x 8, NEC 424260 x 2, Sanyo LC3664 x 5

Other Chips:
SGS Thomson ST93C46CB1 (EEPROM?)
PALS (not dumped):
      18CV8 label SMBG
      18CV8 label SMZ80
      18CV8 label SMCPU
      GAL16V8 (located near BPSM-U47)

GFX:  038 9437WX711 (176 pin PQFP)
      038 9437WX711 (176 pin PQFP)
      038 9437WX711 (176 pin PQFP)
      013 9346E7002 (240 pin PQFP)

On PCB near JAMMA connector is a small push button to access test mode.

ROMS:
BP945A.U9   27C040      Sound Program
BP945A.U45  27C240      Main Program
BPSM.U46    23C16000    Main Program?
BPSM.U47    23C4000     Sound?
BPSM.U48    23C16000    Sound?
BPSM.U53    23C16000    GFX
BPSM.U54    23C16000    GFX
BPSM.U57    23C16000    GFX
BPSM.U58    23C16000    GFX
BPSM.U59    23C16000    GFX
BPSM.U60    23C16000    GFX
BPSM.U61    23C16000    GFX
BPSM.U62    23C16000    GFX
BPSM.U63    23C16000    GFX
BPSM.U64    23C16000    GFX
BPSM.U65    23C16000    GFX
BPSM.U76    23C16000    GFX
BPSM.U77    23C16000    GFX

***************************************************************************/


#define ROMS_SAILORMN \
	ROM_REGION( 0x400000, "maincpu", 0 ) \
	ROM_LOAD16_WORD_SWAP( "bpsm945a.u45", 0x000000, 0x080000, CRC(898c9515) SHA1(0fe8d7f13f5cfe2f6e79a0a21b2e8e7e70e65c4b) ) \
	ROM_LOAD16_WORD_SWAP( "bpsm.u46",     0x200000, 0x200000, CRC(32084e80) SHA1(0ac503190d95009620b5ad7e7e0e63324f6fa4eb) ) \
	\
	ROM_REGION( 0x80000, "audiocpu", 0 ) \
	ROM_LOAD( "bpsm945a.u9",  0x00000, 0x80000, CRC(438de548) SHA1(81a0ca1cd662e2017aa980da162d39cfd0a19f14) ) \
	\
	ROM_REGION( 0x400000, "sprites0", 0 ) \
	ROM_LOAD( "bpsm.u76", 0x000000, 0x200000, CRC(a243a5ba) SHA1(3a32d685e53e0b75977f7acb187cf414a50c7f8b) ) \
	ROM_LOAD( "bpsm.u77", 0x200000, 0x200000, CRC(5179a4ac) SHA1(ceb8d3d889aae885debb2c9cf2263f60be3f1212) ) \
	\
	ROM_REGION( 0x200000, "layer0", 0 ) \
	ROM_LOAD( "bpsm.u53", 0x000000, 0x200000, CRC(b9b15f83) SHA1(8c574c97d38fb9e2889648c8d677b171e80a4229) ) \
	\
	ROM_REGION( 0x200000, "layer1", 0 ) \
	ROM_LOAD( "bpsm.u54", 0x000000, 0x200000, CRC(8f00679d) SHA1(4ea412f8ecdb9fd46f2d1378809919d1a62fcc2b) ) \
	\
	ROM_REGION( (5*0x200000)*2, "layer2", 0 ) \
	\
	ROM_LOAD( "bpsm.u57", 0x000000, 0x200000, CRC(86be7b63) SHA1(6b7d3d41fb1e4045c765b3cc98304464d91e6e3d) ) \
	ROM_LOAD( "bpsm.u58", 0x200000, 0x200000, CRC(e0bba83b) SHA1(9e1434814efd9321b2e5210b995d2fe66cca37dd) ) \
	ROM_LOAD( "bpsm.u62", 0x400000, 0x200000, CRC(a1e3bfac) SHA1(4528887d57e519df8dd60b2392db4c175c57b239) ) \
	ROM_LOAD( "bpsm.u61", 0x600000, 0x200000, CRC(6a014b52) SHA1(107c687479b59c455fc514cd61d290853c95ad9a) ) \
	ROM_LOAD( "bpsm.u60", 0x800000, 0x200000, CRC(992468c0) SHA1(3c66cc08313a9a326badc44f53a98cdfe0643da4) ) \
	\
	ROM_LOAD( "bpsm.u65", 0xa00000, 0x200000, CRC(f60fb7b5) SHA1(72cb8908cd687a330e14657664cd35037a52c39e) ) \
	ROM_LOAD( "bpsm.u64", 0xc00000, 0x200000, CRC(6559d31c) SHA1(bf688123a4beff625652cc1844bf0dc192f5c90f) ) \
	ROM_LOAD( "bpsm.u63", 0xe00000, 0x100000, CRC(d57a56b4) SHA1(e039b336887b66eba4e0630a3cb04cbd8fe14073) ) \
	ROM_CONTINUE(         0xe00000, 0x100000             ) \
	\
	ROM_REGION( 0x200000, "oki1", 0 ) \
	ROM_LOAD( "bpsm.u48", 0x000000, 0x200000, CRC(498e4ed1) SHA1(28d45a41702d9e5af4e214c1800b2e513ec84d51) ) \
	\
	ROM_REGION( 0x200000, "oki2", 0 ) \
	ROM_LOAD( "bpsm.u47", 0x000000, 0x080000, CRC(0f2901b9) SHA1(ebd3e9e39e8d2bc91688dac19b99548a28b4733c) ) \
	ROM_RELOAD(           0x080000, 0x080000             ) \
	ROM_RELOAD(           0x100000, 0x080000             ) \
	ROM_RELOAD(           0x180000, 0x080000             )
/* the regions differ only in the EEPROM, hence the macro above - all EEPROMs are Factory Defaulted */
ROM_START( sailormn )
	ROMS_SAILORMN

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_europe.nv", 0x0000, 0x0080, CRC(59a7dc50) SHA1(6b116bdfbde42192b01678cb0b9bab0f2e56fd28) )
ROM_END

ROM_START( sailormnu )
	ROMS_SAILORMN

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_usa.nv", 0x0000, 0x0080, CRC(3915abe3) SHA1(1b8d3b8c65cf2298939c27607ec52630c017c7ea) )
ROM_END

ROM_START( sailormnj )
	ROMS_SAILORMN

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_japan.nv", 0x0000, 0x0080, CRC(ea03c30a) SHA1(2afc71f932674e34fc4491db0e2027e0371569fc) )
ROM_END

ROM_START( sailormnk )
	ROMS_SAILORMN

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_korea.nv", 0x0000, 0x0080, CRC(0e7de398) SHA1(b495bf43d8596a0dc9843c74fc04fd21499bd115) )
ROM_END

ROM_START( sailormnt )
	ROMS_SAILORMN

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_taiwan.nv", 0x0000, 0x0080, CRC(6c7e8c2a) SHA1(68ef4e6593e4c12e6488a20dcc6dda920b01de67) )
ROM_END

ROM_START( sailormnh )
	ROMS_SAILORMN

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_hongkong.nv", 0x0000, 0x0080, CRC(4d24c874) SHA1(93b87ef18cff98e4f6a60234692e7a9a9c8e7552) )
ROM_END


#define ROMS_SAILORMNN \
	ROM_REGION( 0x400000, "maincpu", 0 ) \
	ROM_LOAD16_WORD_SWAP( "smprg.u45",    0x000000, 0x080000, CRC(234f1152) SHA1(8fc6d4a8995d550862d328011d3357c09334f0fa) ) \
	ROM_LOAD16_WORD_SWAP( "bpsm.u46",     0x200000, 0x200000, CRC(32084e80) SHA1(0ac503190d95009620b5ad7e7e0e63324f6fa4eb) ) \
	\
	ROM_REGION( 0x80000, "audiocpu", 0 ) \
	ROM_LOAD( "bpsm945a.u9",  0x00000, 0x80000, CRC(438de548) SHA1(81a0ca1cd662e2017aa980da162d39cfd0a19f14) ) \
	\
	ROM_REGION( 0x400000, "sprites0", 0 ) \
	ROM_LOAD( "bpsm.u76", 0x000000, 0x200000, CRC(a243a5ba) SHA1(3a32d685e53e0b75977f7acb187cf414a50c7f8b) ) \
	ROM_LOAD( "bpsm.u77", 0x200000, 0x200000, CRC(5179a4ac) SHA1(ceb8d3d889aae885debb2c9cf2263f60be3f1212) ) \
	\
	ROM_REGION( 0x200000, "layer0", 0 ) \
	ROM_LOAD( "bpsm.u53", 0x000000, 0x200000, CRC(b9b15f83) SHA1(8c574c97d38fb9e2889648c8d677b171e80a4229) ) \
	\
	ROM_REGION( 0x200000, "layer1", 0 ) \
	ROM_LOAD( "bpsm.u54", 0x000000, 0x200000, CRC(8f00679d) SHA1(4ea412f8ecdb9fd46f2d1378809919d1a62fcc2b) ) \
	\
	ROM_REGION( (5*0x200000)*2, "layer2", 0 ) \
	\
	ROM_LOAD( "bpsm.u57", 0x000000, 0x200000, CRC(86be7b63) SHA1(6b7d3d41fb1e4045c765b3cc98304464d91e6e3d) ) \
	ROM_LOAD( "bpsm.u58", 0x200000, 0x200000, CRC(e0bba83b) SHA1(9e1434814efd9321b2e5210b995d2fe66cca37dd) ) \
	ROM_LOAD( "bpsm.u62", 0x400000, 0x200000, CRC(a1e3bfac) SHA1(4528887d57e519df8dd60b2392db4c175c57b239) ) \
	ROM_LOAD( "bpsm.u61", 0x600000, 0x200000, CRC(6a014b52) SHA1(107c687479b59c455fc514cd61d290853c95ad9a) ) \
	ROM_LOAD( "bpsm.u60", 0x800000, 0x200000, CRC(992468c0) SHA1(3c66cc08313a9a326badc44f53a98cdfe0643da4) ) \
	\
	ROM_LOAD( "bpsm.u65", 0xa00000, 0x200000, CRC(f60fb7b5) SHA1(72cb8908cd687a330e14657664cd35037a52c39e) ) \
	ROM_LOAD( "bpsm.u64", 0xc00000, 0x200000, CRC(6559d31c) SHA1(bf688123a4beff625652cc1844bf0dc192f5c90f) ) \
	ROM_LOAD( "bpsm.u63", 0xe00000, 0x100000, CRC(d57a56b4) SHA1(e039b336887b66eba4e0630a3cb04cbd8fe14073) ) \
	ROM_CONTINUE(         0xe00000, 0x100000             ) \
	\
	ROM_REGION( 0x200000, "oki1", 0 ) \
	ROM_LOAD( "bpsm.u48", 0x000000, 0x200000, CRC(498e4ed1) SHA1(28d45a41702d9e5af4e214c1800b2e513ec84d51) ) \
	\
	ROM_REGION( 0x200000, "oki2", 0 ) \
	ROM_LOAD( "bpsm.u47", 0x000000, 0x080000, CRC(0f2901b9) SHA1(ebd3e9e39e8d2bc91688dac19b99548a28b4733c) ) \
	ROM_RELOAD(           0x080000, 0x080000             ) \
	ROM_RELOAD(           0x100000, 0x080000             ) \
	ROM_RELOAD(           0x180000, 0x080000             )
/* the regions differ only in the EEPROM, hence the macro above - all EEPROMs are Factory Defaulted */
ROM_START( sailormnn )
	ROMS_SAILORMNN

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_europe.nv", 0x0000, 0x0080, CRC(59a7dc50) SHA1(6b116bdfbde42192b01678cb0b9bab0f2e56fd28) )
ROM_END

ROM_START( sailormnnu )
	ROMS_SAILORMNN

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_usa.nv", 0x0000, 0x0080, CRC(3915abe3) SHA1(1b8d3b8c65cf2298939c27607ec52630c017c7ea) )
ROM_END

ROM_START( sailormnnj )
	ROMS_SAILORMNN

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_japan.nv", 0x0000, 0x0080, CRC(ea03c30a) SHA1(2afc71f932674e34fc4491db0e2027e0371569fc) )
ROM_END

ROM_START( sailormnnk )
	ROMS_SAILORMNN

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_korea.nv", 0x0000, 0x0080, CRC(0e7de398) SHA1(b495bf43d8596a0dc9843c74fc04fd21499bd115) )
ROM_END

ROM_START( sailormnnt )
	ROMS_SAILORMNN

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_taiwan.nv", 0x0000, 0x0080, CRC(6c7e8c2a) SHA1(68ef4e6593e4c12e6488a20dcc6dda920b01de67) )
ROM_END

ROM_START( sailormnnh )
	ROMS_SAILORMNN

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_hongkong.nv", 0x0000, 0x0080, CRC(4d24c874) SHA1(93b87ef18cff98e4f6a60234692e7a9a9c8e7552) )
ROM_END


#define ROMS_SAILORMNO \
	ROM_REGION( 0x400000, "maincpu", 0 ) \
	ROM_LOAD16_WORD_SWAP( "smprg.u45",    0x000000, 0x080000, CRC(97837ab4) SHA1(bf5a8cf3fbb942c2bb74c3b93312d9018ae6e2fd) ) \
	ROM_LOAD16_WORD_SWAP( "bpsm.u46",     0x200000, 0x200000, CRC(32084e80) SHA1(0ac503190d95009620b5ad7e7e0e63324f6fa4eb) ) \
	\
	ROM_REGION( 0x80000, "audiocpu", 0 ) \
	ROM_LOAD( "bpsm945a.u9",  0x00000, 0x80000, CRC(438de548) SHA1(81a0ca1cd662e2017aa980da162d39cfd0a19f14) ) \
	\
	ROM_REGION( 0x400000, "sprites0", 0 ) \
	ROM_LOAD( "bpsm.u76", 0x000000, 0x200000, CRC(a243a5ba) SHA1(3a32d685e53e0b75977f7acb187cf414a50c7f8b) ) \
	ROM_LOAD( "bpsm.u77", 0x200000, 0x200000, CRC(5179a4ac) SHA1(ceb8d3d889aae885debb2c9cf2263f60be3f1212) ) \
	\
	ROM_REGION( 0x200000, "layer0", 0 ) \
	ROM_LOAD( "bpsm.u53", 0x000000, 0x200000, CRC(b9b15f83) SHA1(8c574c97d38fb9e2889648c8d677b171e80a4229) ) \
	\
	ROM_REGION( 0x200000, "layer1", 0 ) \
	ROM_LOAD( "bpsm.u54", 0x000000, 0x200000, CRC(8f00679d) SHA1(4ea412f8ecdb9fd46f2d1378809919d1a62fcc2b) ) \
	\
	ROM_REGION( (5*0x200000)*2, "layer2", 0 ) \
	\
	ROM_LOAD( "bpsm.u57", 0x000000, 0x200000, CRC(86be7b63) SHA1(6b7d3d41fb1e4045c765b3cc98304464d91e6e3d) ) \
	ROM_LOAD( "bpsm.u58", 0x200000, 0x200000, CRC(e0bba83b) SHA1(9e1434814efd9321b2e5210b995d2fe66cca37dd) ) \
	ROM_LOAD( "bpsm.u62", 0x400000, 0x200000, CRC(a1e3bfac) SHA1(4528887d57e519df8dd60b2392db4c175c57b239) ) \
	ROM_LOAD( "bpsm.u61", 0x600000, 0x200000, CRC(6a014b52) SHA1(107c687479b59c455fc514cd61d290853c95ad9a) ) \
	ROM_LOAD( "bpsm.u60", 0x800000, 0x200000, CRC(992468c0) SHA1(3c66cc08313a9a326badc44f53a98cdfe0643da4) ) \
	\
	ROM_LOAD( "bpsm.u65", 0xa00000, 0x200000, CRC(f60fb7b5) SHA1(72cb8908cd687a330e14657664cd35037a52c39e) ) \
	ROM_LOAD( "bpsm.u64", 0xc00000, 0x200000, CRC(6559d31c) SHA1(bf688123a4beff625652cc1844bf0dc192f5c90f) ) \
	ROM_LOAD( "bpsm.u63", 0xe00000, 0x100000, CRC(d57a56b4) SHA1(e039b336887b66eba4e0630a3cb04cbd8fe14073) ) \
	ROM_CONTINUE(         0xe00000, 0x100000             ) \
	\
	ROM_REGION( 0x200000, "oki1", 0 ) \
	ROM_LOAD( "bpsm.u48", 0x000000, 0x200000, CRC(498e4ed1) SHA1(28d45a41702d9e5af4e214c1800b2e513ec84d51) ) \
	\
	ROM_REGION( 0x200000, "oki2", 0 ) \
	ROM_LOAD( "bpsm.u47", 0x000000, 0x080000, CRC(0f2901b9) SHA1(ebd3e9e39e8d2bc91688dac19b99548a28b4733c) ) \
	ROM_RELOAD(           0x080000, 0x080000             ) \
	ROM_RELOAD(           0x100000, 0x080000             ) \
	ROM_RELOAD(           0x180000, 0x080000             )
/* the regions differ only in the EEPROM, hence the macro above - all EEPROMs are Factory Defaulted */
ROM_START( sailormno )
	ROMS_SAILORMNO

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_europe.nv", 0x0000, 0x0080, CRC(59a7dc50) SHA1(6b116bdfbde42192b01678cb0b9bab0f2e56fd28) )
ROM_END

ROM_START( sailormnou )
	ROMS_SAILORMNO

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_usa.nv", 0x0000, 0x0080, CRC(3915abe3) SHA1(1b8d3b8c65cf2298939c27607ec52630c017c7ea) )
ROM_END

ROM_START( sailormnoj )
	ROMS_SAILORMNO

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_japan.nv", 0x0000, 0x0080, CRC(ea03c30a) SHA1(2afc71f932674e34fc4491db0e2027e0371569fc) )
ROM_END

ROM_START( sailormnok )
	ROMS_SAILORMNO

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_korea.nv", 0x0000, 0x0080, CRC(0e7de398) SHA1(b495bf43d8596a0dc9843c74fc04fd21499bd115) )
ROM_END

ROM_START( sailormnot )
	ROMS_SAILORMNO

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_taiwan.nv", 0x0000, 0x0080, CRC(6c7e8c2a) SHA1(68ef4e6593e4c12e6488a20dcc6dda920b01de67) )
ROM_END

ROM_START( sailormnoh )
	ROMS_SAILORMNO

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_hongkong.nv", 0x0000, 0x0080, CRC(4d24c874) SHA1(93b87ef18cff98e4f6a60234692e7a9a9c8e7552) )
ROM_END


/***************************************************************************

  Tekken Card World by Namco, 1997
  Namco EMG4 platform, PCB 8824960101 (sticker: D 0049)

  TMP 68HC000P-16

  013 9651EX001
  038 9701WX001

  OKI M6295 (the second OKI location is unpopulated)

  3V Button Battery
  93C46 EEPROM (at U24)

  28MHz XTAL

***************************************************************************/

ROM_START( tekkencw )
	ROM_REGION( 0x80000, "maincpu", 0 )        /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mpr0.u41", 0x00000, 0x80000, CRC(5b8919f3) SHA1(580298b6dc36527ab69889c848acab97726a6cc6) ) // 27c240

	ROM_REGION( 0x100000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD16_BYTE( "obj0.u52", 0x00000, 0x80000, CRC(6d3c0c76) SHA1(92f9c9beae222a2c2a3242f812030e08036c9963) ) // 27c040
	ROM_LOAD16_BYTE( "obj1.u53", 0x00001, 0x80000, CRC(8069b731) SHA1(9f0409c28466503092b74f635602962d9f127de8) ) // ""

	ROM_REGION( 0x80000, "layer0", 0 )  /* Layer 0 */
	ROM_LOAD16_BYTE( "cha0.u60", 0x00000, 0x40000, CRC(2a245ade) SHA1(7217017975c88c3edea613152ee6f2158f8777d7) ) // 27c020
	ROM_LOAD16_BYTE( "cha1.u61", 0x00001, 0x40000, CRC(43f62cce) SHA1(aa12ed0ccb94115ff8f9acf17850e1186c68bcf9) ) // ""

	ROM_REGION( 0x40000, "oki1", 0 )    /* OKIM6295 #1 Samples */
	ROM_LOAD( "voi0.u27", 0x00000, 0x40000, CRC(3bcd9b7d) SHA1(7ecb47127733187f385a75b9db655e35c249de18) ) // 27c020

	ROM_REGION( 0x117 * 2, "plds", 0 )
	ROM_LOAD( "n44u1d.u1", 0x117*0, 0x117, NO_DUMP )   // GAL16V8D-15LP
	ROM_LOAD( "n44u3a.u3", 0x117*1, 0x117, NO_DUMP )   // GAL16V8D-15LP
ROM_END


/***************************************************************************

  Tekken Battle Scratch by Namco, 1998
  Namco EMG4 platform, PCB 8824960101 (sticker: D 0880)

  TMP 68HC000P-16

  013 9????????
  038 9748WX001

  OKI M6295 (the second OKI location is unpopulated)

  3V Button Battery
  93C46 EEPROM (at U24)

  28MHz XTAL

***************************************************************************/

ROM_START( tekkenbs )
	ROM_REGION( 0x80000, "maincpu", 0 )        /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "tbs1_mpr-0a.u41", 0x00000, 0x80000, CRC(625487d3) SHA1(6bdc0f0f9877eeb1041f8f5b0d44e41b83ddcc76) ) // 27c4002

	ROM_REGION( 0x100000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD16_BYTE( "tbs1_obj-0a.u52", 0x00000, 0x80000, CRC(a870481b) SHA1(644370e10b197832ee828b22e43f114d40740432) ) // 27c4001
	ROM_LOAD16_BYTE( "tbs1_obj-1a.u53", 0x00001, 0x80000, CRC(73d8f520) SHA1(70ab5abeeaf0b3f5a263a7ece21d000a27148994) ) // ""

	ROM_REGION( 0x100000, "layer0", 0 )  /* Layer 0 */
	ROM_LOAD16_BYTE( "tbs1_cha-0a.u60", 0x00000, 0x80000, CRC(73e5c069) SHA1(5e4e8a0bc1fdf57e4cdf7075704dc0b60d9629e3) ) // 27c4001
	ROM_LOAD16_BYTE( "tbs1_cha-1a.u61", 0x00001, 0x80000, CRC(f41d3f2f) SHA1(d44f1506110fe9b7ef74ca05874146526ddaf020) ) // ""

	ROM_REGION( 0x40000, "oki1", 0 )    /* OKIM6295 #1 Samples */
	ROM_LOAD( "tbs1_voi-0a.u27", 0x00000, 0x40000, CRC(bdccb92e) SHA1(7efcce4028fe492891e6f47b266d68a22dbe4c63) ) // 27c2001

	ROM_REGION( 0x117 * 2, "plds", 0 )
	ROM_LOAD( "n44u1e.u1", 0x117*0, 0x117, NO_DUMP )   // GAL16V8D-15LP
	ROM_LOAD( "n44u3e.u3", 0x117*1, 0x117, NO_DUMP )   // GAL16V8D-15LP
ROM_END


/***************************************************************************

  Tobikose! Jumpman by Namco, 1999
  Namco EMG4 platform, PCB TJ0476

  TMP 68HC000P-16

  013 9934WX002
  038 9919WX007

  OKI M6295

  Battery
  93C46 EEPROM? (at U24)

  28MHz XTAL

***************************************************************************/

ROM_START( tjumpman )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "tj1_mpr-0c.u41", 0x00000, 0x80000, CRC(de3030b8) SHA1(5f2165ea039c34cab605ebddc0b61eadc47b1532) )

	ROM_REGION( 0x100000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD16_BYTE( "tj1_obj-0a.u52", 0x00000, 0x80000, CRC(b42cf8e8) SHA1(9ed7fb3574ed163a81f34a0d8cfa7a4661439932) )
	ROM_LOAD16_BYTE( "tj1_obj-1a.u53", 0x00001, 0x80000, CRC(5f0124d7) SHA1(4d9cfa464159998c176a178c668273d128dedff8) )

	ROM_REGION( 0x80000, "layer0", 0 )  /* Layer 0 */
	ROM_LOAD16_BYTE( "tj1_cha-0a.u60", 0x00000, 0x40000, CRC(8aa08a38) SHA1(92b4df72fb8a833bb686ea478811243c5b868470) )
	ROM_LOAD16_BYTE( "tj1_cha-1a.u61", 0x00001, 0x40000, CRC(50072c82) SHA1(f8823e5a865afb8824cafd3b6483e2b6250ee77f) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* OKIM6295 #1 Samples */
	ROM_LOAD( "tj1_voi-0a.u27", 0x00000, 0x40000, CRC(b5693aae) SHA1(8887ae98030cb5d184e3d57d2b4e48bf1e76a232) )

	ROM_REGION( 0x117 * 2, "plds", 0 )
	ROM_LOAD( "n44u1g.u1", 0x117*0, 0x117, CRC(e226ec18) SHA1(c14098e06413d6fc88926e31538d496ef7314903) )   // GAL16V8D-15LP
	ROM_LOAD( "n44u3b.u3", 0x117*1, 0x117, CRC(4cd79750) SHA1(cfb3331cd8bb2eaaf5d2a80ae76a5a15ae92d379) )   // GAL16V8D-15LP
ROM_END


/***************************************************************************

                             Puzzle Uo Poko
Board: CV02
OSC:    28.0, 16.0, 16.9 MHz

PCB found with hand written labels in Japanese (translated):

Program (O)        Program (E)
Eigo  U25          Eigo  U26
C553  AOU          ECB7  AOU

Eigo means English and AOU is Amusement Operators Union.  This board looks
to be an AOU show board.  The data contents remained the same for the
actual International production release.

***************************************************************************/

ROM_START( uopoko )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "u26.int", 0x000000, 0x080000, CRC(b445c9ac) SHA1(4dda1c6e19de629ea4d9061560c32a9f0deabd53) )
	ROM_LOAD16_BYTE( "u25.int", 0x000001, 0x080000, CRC(a1258482) SHA1(7f4adc4a6d069032aaf3d93eb60fde16b59483f8) )

	ROM_REGION( 0x400000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD( "cave_cv-02_u33.u33", 0x000000, 0x400000, CRC(5d142ad2) SHA1(f26abcf7a625a322b83df44fbd6e852bfb03663c) ) /* mask ROM */

	ROM_REGION( 0x400000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "cave_cv-02_u49.u49", 0x000000, 0x400000, CRC(12fb11bb) SHA1(953df1b16b5c9a6c3eb2fdebec4669a879270e73) ) /* mask ROM */

	ROM_REGION( 0x200000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "cave_cv-02_u4.u4", 0x000000, 0x200000, CRC(a2d0d755) SHA1(f8493ef7f367f3dc2a229ba785ac67bc5c2c54c0) ) /* mask ROM */

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-uopoko.bin", 0x0000, 0x0080, CRC(f4a24b95) SHA1(4043f0ffed24e38b4f7dbe1a5a4a9e79bdde7dfd) )
ROM_END

ROM_START( uopokoj )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "u26.bin", 0x000000, 0x080000, CRC(e7eec050) SHA1(cf3a77741029f96dbbec5ca7217a1723e4233cff) )
	ROM_LOAD16_BYTE( "u25.bin", 0x000001, 0x080000, CRC(68cb6211) SHA1(a6db0bc2e3e54b6992a44b7d52395917e66db49b) )

	ROM_REGION( 0x400000, "sprites0", 0 )        /* Sprites: * 2 */
	ROM_LOAD( "cave_cv-02_u33.u33", 0x000000, 0x400000, CRC(5d142ad2) SHA1(f26abcf7a625a322b83df44fbd6e852bfb03663c) ) /* mask ROM */

	ROM_REGION( 0x400000, "layer0", 0 ) /* Layer 0 */
	ROM_LOAD( "cave_cv-02_u49.u49", 0x000000, 0x400000, CRC(12fb11bb) SHA1(953df1b16b5c9a6c3eb2fdebec4669a879270e73) ) /* mask ROM */

	ROM_REGION( 0x200000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "cave_cv-02_u4.u4", 0x000000, 0x200000, CRC(a2d0d755) SHA1(f8493ef7f367f3dc2a229ba785ac67bc5c2c54c0) ) /* mask ROM */

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-uopoko.bin", 0x0000, 0x0080, CRC(f4a24b95) SHA1(4043f0ffed24e38b4f7dbe1a5a4a9e79bdde7dfd) )
ROM_END


/***************************************************************************


    Drivers Init Routines - Rom decryption/unpacking, global vars etc.


***************************************************************************/

/* Tiles are 6 bit, 4 bits stored in one rom, 2 bits in the other.
   Expand the 2 bit part into a 4 bit layout, so we can decode it */
void cave_state::sailormn_unpack_tiles(int chip)
{
	const u32 len=   m_tileregion[chip]->bytes();
	u8 *rgn      =   m_tileregion[chip]->base();
	u8 *src      =   rgn + (len/4)*3 - 1;
	u8 *dst      =   rgn + (len/4)*4 - 2;

	while (src <= dst)
	{
		u8 data = src[0];

		dst[0] = ((data & 0x03) << 4) + ((data & 0x0c) >> 2);
		dst[1] = ((data & 0x30) >> 0) + ((data & 0xc0) >> 6);

		src -= 1;
		dst -= 2;
	}
}

void cave_state::init_cave()
{
	m_spritetype[0] = TYPE_ZOOM; // Normal sprites
	m_kludge = 0;
	m_time_vblank_irq = 100;

	m_irq_level = 1;
}

void cave_state::init_z80_bank()
{
	u8 *ROM = m_z80region->base();
	u32 max = m_z80region->bytes() / 0x4000;
	m_z80bank->configure_entries(0, max, &ROM[0x00000], 0x4000);
}

void cave_state::init_oki_bank(int chip)
{
	u8 *ROM = m_okiregion[chip]->base();
	u32 max = m_okiregion[chip]->bytes() / 0x20000;
	m_okibank_lo[chip]->configure_entries(0, max, &ROM[0x00000], 0x20000);
	m_okibank_hi[chip]->configure_entries(0, max, &ROM[0x00000], 0x20000);
}

void cave_state::init_agallet()
{
	init_cave();

	init_z80_bank();
	init_oki_bank(0);
	init_oki_bank(1);

	sailormn_unpack_tiles(2);

	unpack_sprites(0);
}

void cave_state::init_dfeveron()
{
	init_cave();

	unpack_sprites(0);
	m_kludge = 2;
}

void cave_state::init_feversos()
{
	init_cave();

	unpack_sprites(0);
	m_kludge = 2;
}

void cave_state::init_ddonpach()
{
	init_cave();

	unpack_sprites(0);
	m_spritetype[0] = TYPE_NOZOOM;    // "different" sprites (no zooming?)
	m_time_vblank_irq = 90;

	/* 4bpp but Only first 16 colors are used in palette index for Layer 0, 1. */
	m_gfxdecode[0]->gfx(0)->set_granularity(256);
	m_gfxdecode[0]->gfx(1)->set_granularity(256);
}

void cave_state::init_donpachi()
{
	init_cave();

	unpack_sprites(0);
	m_spritetype[0] = TYPE_NOZOOM;    // "different" sprites (no zooming?)
	m_time_vblank_irq = 90;
}

void cave_state::init_esprade()
{
	init_cave();

	unpack_sprites(0);
	m_time_vblank_irq = 2000;   /**/

#if 0       //ROM PATCH
	{
		u16 *rom = (u16 *)memregion("maincpu")->base();
		rom[0x118A/2] = 0x4e71;         //palette fix   118A: 5548              SUBQ.W  #2,A0       --> NOP
	}
#endif
}

void cave_state::init_gaia()
{
	init_cave();

	/* No EEPROM */

	unpack_sprites(0);
	m_time_vblank_irq = 2000;   /**/
}

void cave_state::init_guwange()
{
	init_cave();

	unpack_sprites(0);
	m_time_vblank_irq = 2000;   /**/
}

void cave_state::init_hotdogst()
{
	init_cave();

	init_z80_bank();
	init_oki_bank(0);

	unpack_sprites(0);
	m_time_vblank_irq = 2000;   /**/
}

void cave_state::init_mazinger()
{
	u8 *src = m_spriteregion[0]->base();
	const u32 len = m_spriteregion[0]->bytes();

	init_cave();

	init_z80_bank();
	init_oki_bank(0);

	/* decrypt sprites */
	std::vector<u8> buffer(len);
	{
		for (int i = 0; i < len; i++)
			buffer[i ^ 0xdf88] = src[(i & ~0xffffff) | bitswap<24>(i,23,22,21,20,19,9,7,3,15,4,17,14,18,2,16,5,11,8,6,13,1,10,12,0)];
		std::copy(buffer.begin(), buffer.end(), &src[0]);
	}

	unpack_sprites(0);
	m_kludge = 3;
	m_time_vblank_irq = 2100;
}

void cave_state::init_metmqstr()
{
	init_cave();

	init_z80_bank();
	init_oki_bank(0);
	init_oki_bank(1);

	unpack_sprites(0);
	m_kludge = 3;
	m_time_vblank_irq = 17376;
}

void cave_state::init_ppsatan()
{
	init_cave();

	unpack_sprites(0);
	unpack_sprites(1);
	unpack_sprites(2);

	m_time_vblank_irq = 2000;   /**/

	m_ppsatan_io_mux = 0;
	save_item(NAME(m_ppsatan_io_mux));
}

void cave_state::init_pwrinst2j()
{
	u8 *src = m_spriteregion[0]->base();
	const u32 len = m_spriteregion[0]->bytes();

	init_cave();

	init_z80_bank();

	std::vector<u8> buffer(len);
	{
		for (u32 i = 0; i < len; i++)
		{
			u32 j = (i & ~0xffffff) | bitswap<24>(i,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7, 2,4,6,1,5,3, 0);
			if(((j & 6) == 0) || ((j & 6) == 6))
				j ^= 6;
			buffer[j ^ 7] = src[i];
		}

		std::copy(buffer.begin(), buffer.end(), &src[0]);
	}

	unpack_sprites(0);
	m_spritetype[0] = TYPE_NOZOOM | TYPE_ISPWRINST2;
	m_kludge = 4;
	m_time_vblank_irq = 2000;   /**/
}

void cave_state::init_pwrinst2()
{
	/* this patch fixes on of the moves, why is it needed? is the rom bad or is there another
	   problem? does the Japan set need it or not? */
	init_pwrinst2j();

#if 1       //ROM PATCH
	{
		u16 *rom = (u16 *)memregion("maincpu")->base();
		rom[0xd46c / 2] = 0xd482;           // kurara dash fix  0xd400 -> 0xd482
	}
#endif

}

void cave_state::init_sailormn()
{
	u8 *src = m_spriteregion[0]->base();
	const u32 len = m_spriteregion[0]->bytes();

	init_cave();

	init_z80_bank();
	init_oki_bank(0);
	init_oki_bank(1);

	/* decrypt sprites */
	std::vector<u8> buffer(len);
	{
		for (int i = 0; i < len; i++)
			buffer[i ^ 0x950c4] = src[(i & ~0xffffff) | bitswap<24>(i,23,22,21,20,15,10,12,6,11,1,13,3,16,17,2,5,14,7,18,8,4,19,9,0)];
		std::copy(buffer.begin(), buffer.end(), &src[0]);
	}

	sailormn_unpack_tiles(2);

	unpack_sprites(0);
	m_kludge = 1;
	m_time_vblank_irq = 2000;

	m_sailormn_tilebank = 0;
	save_item(NAME(m_sailormn_tilebank));
}

void cave_state::init_tjumpman()
{
	init_cave();

	unpack_sprites(0);
	m_kludge = 3;
	m_time_vblank_irq = 17376;

	m_hopper = 0;
	save_item(NAME(m_hopper));
}

void cave_state::init_uopoko()
{
	init_cave();

	unpack_sprites(0);
	m_kludge = 2;
	m_time_vblank_irq = 2000;   /**/
}

void cave_state::init_korokoro()
{
	init_cave();

	m_irq_level = 2;

	unpack_sprites(0);
	m_time_vblank_irq = 2000;   /**/

	m_leds[0] = 0;
	m_leds[1] = 0;
	m_hopper = 0;
	save_item(NAME(m_leds));
	save_item(NAME(m_hopper));
}

/***************************************************************************


                                Game Drivers


***************************************************************************/

GAME( 1994, pwrinst2,   0,        pwrinst2, metmqstr, cave_state, init_pwrinst2,  ROT0,   "Atlus",                                  "Power Instinct 2 (US, Ver. 94.04.08)",         MACHINE_SUPPORTS_SAVE )
GAME( 1994, pwrinst2j,  pwrinst2, pwrinst2, metmqstr, cave_state, init_pwrinst2j, ROT0,   "Atlus",                                  "Gouketsuji Ichizoku 2 (Japan, Ver. 94.04.08)", MACHINE_SUPPORTS_SAVE )

// Version/Date string is stored at 68000 ROM 0x1200-0x121f
// The EEPROM determines the region, program roms are the same between sets
GAME( 1994, mazinger,   0,        mazinger, cave,     cave_state, init_mazinger,  ROT90,  "Banpresto / Dynamic Pl. Toei Animation", "Mazinger Z (World)", MACHINE_SUPPORTS_SAVE ) // 1994/06/27 08:00
GAME( 1994, mazingerj,  mazinger, mazinger, cave,     cave_state, init_mazinger,  ROT90,  "Banpresto / Dynamic Pl. Toei Animation", "Mazinger Z (Japan)", MACHINE_SUPPORTS_SAVE ) // 1994/06/27 08:00

// Version/Date string is stored at 68000 ROM 0x400-0x41f
GAME( 1995, donpachi,   0,        donpachi, cave,     cave_state, init_donpachi,  ROT270, "Cave (Atlus license)",                   "DonPachi (US)",                     MACHINE_SUPPORTS_SAVE ) // Ver.1.12 1995/05/2x XXXXX
GAME( 1995, donpachij,  donpachi, donpachi, cave,     cave_state, init_donpachi,  ROT270, "Cave (Atlus license)",                   "DonPachi (Japan)",                  MACHINE_SUPPORTS_SAVE ) // Ver.1.01 1995/05/11
GAME( 1995, donpachijs, donpachi, donpachi, cave,     cave_state, init_donpachi,  ROT270, "Cave (Atlus license)",                   "DonPachi (Japan, Satsuei version)", MACHINE_SUPPORTS_SAVE ) // Ver.1.01 1995/05/11
GAME( 1995, donpachikr, donpachi, donpachi, cave,     cave_state, init_donpachi,  ROT270, "Cave (Atlus license)",                   "DonPachi (Korea)",                  MACHINE_SUPPORTS_SAVE ) // Ver.1.12 1995/05/2x XXXXX
GAME( 1995, donpachihk, donpachi, donpachi, cave,     cave_state, init_donpachi,  ROT270, "Cave (Atlus license)",                   "DonPachi (Hong Kong)",              MACHINE_SUPPORTS_SAVE ) // Ver.1.10 1995/05/17  asia

GAME( 1995, metmqstr,   0,        metmqstr, metmqstr, cave_state, init_metmqstr,  ROT0,   "Banpresto / Pandorabox",                 "Metamoqester (World)",           MACHINE_SUPPORTS_SAVE )
GAME( 1995, nmaster,    metmqstr, metmqstr, metmqstr, cave_state, init_metmqstr,  ROT0,   "Banpresto / Pandorabox",                 "Oni - The Ninja Master (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1995, plegends,   0,        pwrinst2, metmqstr, cave_state, init_pwrinst2j, ROT0,   "Atlus",                                  "Gogetsuji Legends (US, Ver. 95.06.20)",                       MACHINE_SUPPORTS_SAVE )
GAME( 1995, plegendsj,  plegends, pwrinst2, metmqstr, cave_state, init_pwrinst2j, ROT0,   "Atlus",                                  "Gouketsuji Gaiden - Saikyou Densetsu (Japan, Ver. 95.06.20)", MACHINE_SUPPORTS_SAVE )

// The EEPROM determines the region, program roms are the same between sets
GAME( 1995, sailormn,   0,        sailormn, cave,     cave_state, init_sailormn,  ROT0,   "Gazelle (Banpresto license)",            "Pretty Soldier Sailor Moon (Version 95/03/22B, Europe)",     MACHINE_SUPPORTS_SAVE )
GAME( 1995, sailormnu,  sailormn, sailormn, cave,     cave_state, init_sailormn,  ROT0,   "Gazelle (Banpresto license)",            "Pretty Soldier Sailor Moon (Version 95/03/22B, USA)",        MACHINE_SUPPORTS_SAVE )
GAME( 1995, sailormnj,  sailormn, sailormn, cave,     cave_state, init_sailormn,  ROT0,   "Gazelle (Banpresto license)",            "Bishoujo Senshi Sailor Moon (Version 95/03/22B, Japan)",     MACHINE_SUPPORTS_SAVE )
GAME( 1995, sailormnk,  sailormn, sailormn, cave,     cave_state, init_sailormn,  ROT0,   "Gazelle (Banpresto license)",            "Pretty Soldier Sailor Moon (Version 95/03/22B, Korea)",      MACHINE_SUPPORTS_SAVE )
GAME( 1995, sailormnt,  sailormn, sailormn, cave,     cave_state, init_sailormn,  ROT0,   "Gazelle (Banpresto license)",            "Bishoujo Senshi Sailor Moon (Version 95/03/22B, Taiwan)",    MACHINE_SUPPORTS_SAVE )
GAME( 1995, sailormnh,  sailormn, sailormn, cave,     cave_state, init_sailormn,  ROT0,   "Gazelle (Banpresto license)",            "Bishoujo Senshi Sailor Moon (Version 95/03/22B, Hong Kong)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, sailormnn,  sailormn, sailormn, cave,     cave_state, init_sailormn,  ROT0,   "Gazelle (Banpresto license)",            "Pretty Soldier Sailor Moon (Version 95/03/22, Europe)",      MACHINE_SUPPORTS_SAVE )
GAME( 1995, sailormnnu, sailormn, sailormn, cave,     cave_state, init_sailormn,  ROT0,   "Gazelle (Banpresto license)",            "Pretty Soldier Sailor Moon (Version 95/03/22, USA)",         MACHINE_SUPPORTS_SAVE )
GAME( 1995, sailormnnj, sailormn, sailormn, cave,     cave_state, init_sailormn,  ROT0,   "Gazelle (Banpresto license)",            "Bishoujo Senshi Sailor Moon (Version 95/03/22, Japan)",      MACHINE_SUPPORTS_SAVE )
GAME( 1995, sailormnnk, sailormn, sailormn, cave,     cave_state, init_sailormn,  ROT0,   "Gazelle (Banpresto license)",            "Pretty Soldier Sailor Moon (Version 95/03/22, Korea)",       MACHINE_SUPPORTS_SAVE )
GAME( 1995, sailormnnt, sailormn, sailormn, cave,     cave_state, init_sailormn,  ROT0,   "Gazelle (Banpresto license)",            "Bishoujo Senshi Sailor Moon (Version 95/03/22, Taiwan)",     MACHINE_SUPPORTS_SAVE )
GAME( 1995, sailormnnh, sailormn, sailormn, cave,     cave_state, init_sailormn,  ROT0,   "Gazelle (Banpresto license)",            "Bishoujo Senshi Sailor Moon (Version 95/03/22, Hong Kong)",  MACHINE_SUPPORTS_SAVE )
GAME( 1995, sailormno,  sailormn, sailormn, cave,     cave_state, init_sailormn,  ROT0,   "Gazelle (Banpresto license)",            "Pretty Soldier Sailor Moon (Version 95/03/21, Europe)",      MACHINE_SUPPORTS_SAVE )
GAME( 1995, sailormnou, sailormn, sailormn, cave,     cave_state, init_sailormn,  ROT0,   "Gazelle (Banpresto license)",            "Pretty Soldier Sailor Moon (Version 95/03/21, USA)",         MACHINE_SUPPORTS_SAVE )
GAME( 1995, sailormnoj, sailormn, sailormn, cave,     cave_state, init_sailormn,  ROT0,   "Gazelle (Banpresto license)",            "Bishoujo Senshi Sailor Moon (Version 95/03/21, Japan)",      MACHINE_SUPPORTS_SAVE )
GAME( 1995, sailormnok, sailormn, sailormn, cave,     cave_state, init_sailormn,  ROT0,   "Gazelle (Banpresto license)",            "Pretty Soldier Sailor Moon (Version 95/03/21, Korea)",       MACHINE_SUPPORTS_SAVE )
GAME( 1995, sailormnot, sailormn, sailormn, cave,     cave_state, init_sailormn,  ROT0,   "Gazelle (Banpresto license)",            "Bishoujo Senshi Sailor Moon (Version 95/03/21, Taiwan)",     MACHINE_SUPPORTS_SAVE )
GAME( 1995, sailormnoh, sailormn, sailormn, cave,     cave_state, init_sailormn,  ROT0,   "Gazelle (Banpresto license)",            "Bishoujo Senshi Sailor Moon (Version 95/03/21, Hong Kong)",  MACHINE_SUPPORTS_SAVE )

// The EEPROM determines the region, program roms are the same between sets
GAME( 1996, agallet,    0,        sailormn, cave,     cave_state, init_agallet,   ROT270, "Gazelle (Banpresto license)",            "Air Gallet (Europe)",    MACHINE_SUPPORTS_SAVE )
GAME( 1996, agalletu,   agallet,  sailormn, cave,     cave_state, init_agallet,   ROT270, "Gazelle (Banpresto license)",            "Air Gallet (USA)",       MACHINE_SUPPORTS_SAVE )
GAME( 1996, agalletj,   agallet,  sailormn, cave,     cave_state, init_agallet,   ROT270, "Gazelle (Banpresto license)",            "Akuu Gallet (Japan)",    MACHINE_SUPPORTS_SAVE )
GAME( 1996, agalletk,   agallet,  sailormn, cave,     cave_state, init_agallet,   ROT270, "Gazelle (Banpresto license)",            "Air Gallet (Korea)",     MACHINE_SUPPORTS_SAVE )
GAME( 1996, agallett,   agallet,  sailormn, cave,     cave_state, init_agallet,   ROT270, "Gazelle (Banpresto license)",            "Air Gallet (Taiwan)",    MACHINE_SUPPORTS_SAVE )
GAME( 1996, agalleth,   agallet,  sailormn, cave,     cave_state, init_agallet,   ROT270, "Gazelle (Banpresto license)",            "Air Gallet (Hong Kong)", MACHINE_SUPPORTS_SAVE )
// this set appears to be older, there is some kind of reset circuit / watchdog circuit check on startup, the same check exists in the above set but the code skips over it so presumably it was removed
// to avoid boards simply hanging on a black screen if the circuit didn't fire.
GAME( 1996, agalleta,   agallet,  sailormn, cave,     cave_state, init_agallet,   ROT270, "Gazelle (Banpresto license)",            "Air Gallet (older, Europe)",    MACHINE_SUPPORTS_SAVE )
GAME( 1996, agalletau,  agallet,  sailormn, cave,     cave_state, init_agallet,   ROT270, "Gazelle (Banpresto license)",            "Air Gallet (older, USA)",       MACHINE_SUPPORTS_SAVE )
GAME( 1996, agalletaj,  agallet,  sailormn, cave,     cave_state, init_agallet,   ROT270, "Gazelle (Banpresto license)",            "Akuu Gallet (older, Japan)",    MACHINE_SUPPORTS_SAVE )
GAME( 1996, agalletak,  agallet,  sailormn, cave,     cave_state, init_agallet,   ROT270, "Gazelle (Banpresto license)",            "Air Gallet (older, Korea)",     MACHINE_SUPPORTS_SAVE )
GAME( 1996, agalletat,  agallet,  sailormn, cave,     cave_state, init_agallet,   ROT270, "Gazelle (Banpresto license)",            "Air Gallet (older, Taiwan)",    MACHINE_SUPPORTS_SAVE )
GAME( 1996, agalletah,  agallet,  sailormn, cave,     cave_state, init_agallet,   ROT270, "Gazelle (Banpresto license)",            "Air Gallet (older, Hong Kong)", MACHINE_SUPPORTS_SAVE )

// 68000 ROM string 0x328e-32b5 has 1993 copyright and publisher string, it's planned release date but cancelled?
GAME( 1996, hotdogst,   0,        hotdogst, cave,     cave_state, init_hotdogst,  ROT90,  "Marble (Ace International license)",     "Hotdog Storm (Korea)", MACHINE_SUPPORTS_SAVE )

GAME( 1996, pacslot,    0,        pacslot,  pacslot,  cave_state, init_tjumpman,  ROT0,   "Namco",                                  "Pac-Slot",     MACHINE_SUPPORTS_SAVE )
GAME( 1996, paceight,   0,        paceight, paceight, cave_state, init_tjumpman,  ROT0,   "Namco",                                  "Pac-Eight",    MACHINE_SUPPORTS_SAVE )
GAME( 1996, paccarn,    0,        paccarn,  paccarn,  cave_state, init_tjumpman,  ROT0,   "Namco",                                  "Pac-Carnival", MACHINE_SUPPORTS_SAVE )

GAME( 1996, ppsatan,    0,        ppsatan,  ppsatan,  cave_state, init_ppsatan,   ROT0,   "Kato Seisakujo Co., Ltd.",               "Poka Poka Satan (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )

GAME( 1997, tekkencw,   0,        tekkencw, tekkencw, cave_state, init_tjumpman,  ROT0,   "Namco",                                  "Tekken Card World",     MACHINE_SUPPORTS_SAVE )
GAME( 1998, tekkenbs,   0,        tekkenbs, tekkenbs, cave_state, init_tjumpman,  ROT0,   "Namco",                                  "Tekken Battle Scratch", MACHINE_SUPPORTS_SAVE )

// Version/Date string at 68000 ROM 0x400-0x41f is leftover from donpachi (DONPACHI Ver.1.12 1995/05/2x XXXXX)
GAME( 1997, ddonpach,   0,        ddonpach, cave,     cave_state, init_ddonpach,  ROT270, "Cave (Atlus license)",                   "DoDonPachi (World, 1997 2/ 5 Master Ver.)", MACHINE_SUPPORTS_SAVE )
GAME( 1997, ddonpachj,  ddonpach, ddonpach, cave,     cave_state, init_ddonpach,  ROT270, "Cave (Atlus license)",                   "DoDonPachi (Japan, 1997 2/ 5 Master Ver.)", MACHINE_SUPPORTS_SAVE )
// NOT an official CAVE release, but several PCBs have been converted to it and used on location.
GAME( 2012, ddonpacha,  ddonpach, ddonpach, cave,     cave_state, init_ddonpach,  ROT270, "hack (trap15)",                          "DoDonPachi (2012/02/12 Arrange Ver. 1.1) (hack)",  MACHINE_SUPPORTS_SAVE )

// Version/Date string is stored at 68000 ROM 0x400-0x427
GAME( 1998, dfeveron,   feversos, dfeveron, cave,     cave_state, init_dfeveron,  ROT270, "Cave (Nihon System license)",            "Dangun Feveron (Japan, 98/09/17 VER.)", MACHINE_SUPPORTS_SAVE ) // ca005 Ver0.01 Thu Sep 17 18:40:02 1998
GAME( 1998, feversos,   0,        dfeveron, cave,     cave_state, init_feversos,  ROT270, "Cave (Nihon System license)",            "Fever SOS (World, 98/09/25 VER)",       MACHINE_SUPPORTS_SAVE ) // ca005 Ver0.01 Fri Sep 25 10:10:15 1998

// Version/Date string at 68000 ROM 0x400-0x41f is leftover from donpachi (DONPACHI Ver.1.12 1995/05/2x XXXXX)
GAME( 1998, esprade,    0,        esprade,  cave,     cave_state, init_esprade,   ROT270, "Cave (Atlus license)",                   "ESP Ra.De. (World, 1998 4/22 International Ver.)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, espradej,   esprade,  esprade,  cave,     cave_state, init_esprade,   ROT270, "Cave (Atlus license)",                   "ESP Ra.De. (Japan, 1998 4/21 Master Ver.)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, espradejo,  esprade,  esprade,  cave,     cave_state, init_esprade,   ROT270, "Cave (Atlus license)",                   "ESP Ra.De. (Japan, 1998 4/14 Master Ver.)", MACHINE_SUPPORTS_SAVE )

// Date string is stored at 68000 ROM 0x100-0x1ff
GAME( 1998, uopoko,     0,        uopoko,   cave,     cave_state, init_uopoko,    ROT0,   "Cave (Jaleco license)",                  "Puzzle Uo Poko (World)", MACHINE_SUPPORTS_SAVE ) // CAVE 1998.FEB.06 UOPOKO CV-02-00 OVER SEA
GAME( 1998, uopokoj,    uopoko,   uopoko,   cave,     cave_state, init_uopoko,    ROT0,   "Cave (Jaleco license)",                  "Puzzle Uo Poko (Japan)", MACHINE_SUPPORTS_SAVE ) // CAVE 1998.FEB.06 UOPOKO CV-02-00 JAPAN

// Version/Date string at 68000 ROM 0x400-0x41f is leftover from donpachi (DONPACHI Ver.1.12 1995/05/2x XXXXX)
GAME( 1999, guwange,    0,        guwange,  guwange,  cave_state, init_guwange,   ROT270, "Cave (Atlus license)",                   "Guwange (Japan, 1999 6/24 Master Ver 16:55)",  MACHINE_SUPPORTS_SAVE )
GAME( 1999, guwanges,   guwange,  guwange,  guwange,  cave_state, init_guwange,   ROT270, "Cave (Atlus license)",                   "Guwange (Japan, 2000 7/ 7 Special Ver 13:22)", MACHINE_SUPPORTS_SAVE )

GAME( 1999, gaia,       0,        gaia,     gaia,     cave_state, init_gaia,      ROT0,   "Noise Factory",                          "Gaia Crusaders", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND ) // cuts out occasionally

GAME( 1999, korokoro,   0,        korokoro, korokoro, cave_state, init_korokoro,  ROT0,   "Takumi",                                 "Koro Koro Quest (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1999, crusherm,   0,        crusherm, korokoro, cave_state, init_korokoro,  ROT0,   "Takumi",                                 "Crusher Makochan (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1999, tjumpman,   0,        tjumpman, tjumpman, cave_state, init_tjumpman,  ROT0,   "Namco",                                  "Tobikose! Jumpman", MACHINE_SUPPORTS_SAVE )

GAME( 2001, theroes,    0,        gaia,     theroes,  cave_state, init_gaia,      ROT0,   "Primetek Investments",                   "Thunder Heroes", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND ) // cuts out occasionally
