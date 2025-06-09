// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                              -= Tetris Plus 2 =-

                    driver by   Luca Elia (l.elia@tin.it)


Main  CPU    :  TMP68HC000P-12

Video Chips  :  SS91022-03 9428XX001
                GS91022-04 9721PD008
                SS91022-05 9347EX002
                GS91022-05 048 9726HX002

Sound Chips  :  Yamaha YMZ280B-F

Other        :  XILINX XC5210 PQ240C X68710M AKJ9544
                XC7336 PC44ACK9633 A63458A
                NVRAM


To Do:

-   There is a 3rd unimplemented layer capable of rotation (not used by
    the game, can be tested in service mode).
-   Priority RAM is not taken into account.
-   The video system on this hardware is almost the same as the MegaSystem 32 board

Notes:

-   The Japan set of Tetris Plus 2 doesn't seem to have (or use) NVRAM. I can't
    enter a test mode or use the service coin either !?

***************************************************************************/
/***************************************************************************

 Jaleco's "Stepping Stage" Series

 A PC computer (Harddisk not dumped yet) + Two 68000 based board set.
 One 68000 drives 3 screens, another handles players input.

stepstag:

- 108070.w = 3 to pass boot tests
- 108688.w = 1 (after ram test) to enable all items in test mode

***************************************************************************/

#include "emu.h"
#include "tetrisp2.h"

#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"
#include "sound/okim6295.h"
#include "sound/ymz280b.h"

#include "screen.h"
#include "speaker.h"

#include "vjdash.lh"
#include "rocknms.lh"
#include "stepstag.lh"



/***************************************************************************


                                    Sound


***************************************************************************/

u16 rockn_state::rockn_adpcmbank_r()
{
	return ((m_rockn_adpcmbank & 0xf0ff) | (m_rockn_protectdata << 8));
}

void rockn_state::rockn_adpcmbank_w(u16 data)
{
	m_rockn_adpcmbank = data;
	m_ymzbank[0]->set_entry((data >> 2) & 0x07);
}

void rockn_state::rockn2_adpcmbank_w(u16 data)
{
	const uint8_t banktable[9][3]=
	{
		{  0,  1,  2 },     // bank $00
		{  3,  4,  5 },     // bank $04
		{  6,  7,  8 },     // bank $08
		{  9, 10, 11 },     // bank $0c
		{ 12, 13, 14 },     // bank $10
		{ 15, 16, 17 },     // bank $14
		{ 18, 19, 20 },     // bank $18
		{  0,  0,  0 },     // bank $1c
		{  0,  5, 14 },     // bank $20
	};

	m_rockn_adpcmbank = data;
	int bank = ((data & 0x003f) >> 2);

	if (bank > 8)
	{
		popmessage("!!!!! ADPCM BANK OVER:%01X (%04X) !!!!!", bank, data);
		bank = 0;
	}

	for (int i = 0; i < 3; i++)
		m_ymzbank[i]->set_entry(banktable[bank][i]);
}


u16 rockn_state::rockn_soundvolume_r()
{
	return 0xffff;
}

void rockn_state::rockn_soundvolume_w(u16 data)
{
	m_rockn_soundvolume = data;
	// TODO: unemulated
}


void nndmseal_state::nndmseal_sound_bank_w(u8 data)
{
	if (BIT(data, 2))
	{
		m_bank_lo = data & 0x03;

		m_okibank[0]->set_entry(m_bank_lo);
		m_okibank[1]->set_entry((m_bank_lo * 4) + m_bank_hi);

//      logerror("PC:%06X sound bank_lo = %02X\n",m_maincpu->pc(),m_bank_lo);
	}
	else
	{
		m_bank_hi = data & 0x03;
		m_okibank[1]->set_entry((m_bank_lo * 4) + m_bank_hi);

//      logerror("PC:%06X sound bank_hi = %02X\n",m_maincpu->pc(),m_bank_hi);
	}
}

/***************************************************************************


                                Protection


***************************************************************************/

u16 tetrisp2_state::tetrisp2_ip_1_word_r()
{
	return  (m_io_system->read() &  0xfcff) |
			(   machine().rand() & ~0xfcff) |
			(1 << (8 + (machine().rand() & 1)));
}


/***************************************************************************


                                    NVRAM


***************************************************************************/



/* The game only ever writes even bytes and reads odd bytes */
u16 tetrisp2_state::tetrisp2_nvram_r(offs_t offset)
{
	return  ((m_nvram[offset] >> 8) & 0x00ff) |
			((m_nvram[offset] << 8) & 0xff00);
}

void tetrisp2_state::nvram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_nvram[offset]);
}

u16 rockn_state::rockn_nvram_r(offs_t offset)
{
	return m_nvram[offset];
}


/***************************************************************************





***************************************************************************/


u16 rocknms_state::main2sub_r()
{
	return m_main2sub;
}

void rocknms_state::main2sub_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_main2sub = (data ^ 0xffff);
}

ioport_value rocknms_state::main2sub_status_r()
{
	return m_sub2main & 0x0003;
}

void rocknms_state::sub2main_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_sub2main = (data ^ 0xffff);
}


void tetrisp2_state::tetrisp2_coincounter_w(u16 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
}



/***************************************************************************


                                Memory Map


***************************************************************************/

void tetrisp2_state::tetrisp2_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                         // ROM
	map(0x100000, 0x103fff).ram().share(m_spriteram);           // Object RAM
	map(0x104000, 0x107fff).ram();                                                         // Spare Object RAM
	map(0x108000, 0x10ffff).ram();                                                         // Work RAM
	map(0x200000, 0x23ffff).rw(FUNC(tetrisp2_state::priority_r), FUNC(tetrisp2_state::priority_w));
	map(0x300000, 0x31ffff).ram().w(FUNC(tetrisp2_state::palette_w)).share(m_paletteram);        // Palette
	map(0x400000, 0x403fff).ram().w(FUNC(tetrisp2_state::vram_fg_w)).share(m_vram_fg);   // Foreground
	map(0x404000, 0x407fff).ram().w(FUNC(tetrisp2_state::vram_bg_w)).share(m_vram_bg);   // Background
	map(0x408000, 0x409fff).ram();                                                         // ???
	map(0x500000, 0x50ffff).ram();                                                         // Line
	map(0x600000, 0x60ffff).ram().w(FUNC(tetrisp2_state::vram_rot_w)).share(m_vram_rot); // Rotation
	map(0x650000, 0x651fff).ram().w(FUNC(tetrisp2_state::vram_rot_w));                              // Rotation (mirror)
	map(0x800000, 0x800003).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask16(0x00ff);   // Sound
	map(0x900000, 0x903fff).r(FUNC(tetrisp2_state::tetrisp2_nvram_r)).w(FUNC(tetrisp2_state::nvram_w)).share("nvram"); // NVRAM
	map(0x904000, 0x907fff).r(FUNC(tetrisp2_state::tetrisp2_nvram_r)).w(FUNC(tetrisp2_state::nvram_w));               // NVRAM (mirror)
	map(0xb00000, 0xb00001).w(FUNC(tetrisp2_state::tetrisp2_coincounter_w));                               // Coin Counter
	map(0xb20000, 0xb20001).nopw();                                                    // ???
	map(0xb40000, 0xb4000b).writeonly().share(m_scroll_fg);                     // Foreground Scrolling
	map(0xb40010, 0xb4001b).writeonly().share(m_scroll_bg);                     // Background Scrolling
	map(0xb4003e, 0xb4003f).nopw();                                                    // scr_size
	map(0xb60000, 0xb6002f).writeonly().share(m_rotregs);                       // Rotation Registers
	map(0xba0000, 0xba001f).m(m_sysctrl, FUNC(jaleco_ms32_sysctrl_device::amap));
	map(0xbe0000, 0xbe0001).nopr();                                                     // INT-level1 dummy read
	map(0xbe0002, 0xbe0003).portr("PLAYERS");                                        // Inputs
	map(0xbe0004, 0xbe0005).r(FUNC(tetrisp2_state::tetrisp2_ip_1_word_r));                                  // Inputs & protection
	map(0xbe0008, 0xbe0009).portr("DSW");                                            // Inputs
	map(0xbe000a, 0xbe000b).r("watchdog", FUNC(watchdog_timer_device::reset16_r));       // Watchdog
}


void nndmseal_state::nndmseal_coincounter_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		machine().bookkeeping().coin_counter_w(0, BIT( data, 0));
		//                  BIT(data, 2) ?
		machine().bookkeeping().coin_lockout_w(0, BIT(~data, 3));
	}
	if (ACCESSING_BITS_8_15)
	{
		m_leds[0] = BIT(data, 12);  // +
		m_leds[1] = BIT(data, 13);  // -
		m_leds[2] = BIT(data, 14);  // Cancel
		m_leds[3] = BIT(data, 15);  // OK
	}
//  popmessage("%04x",data);
}

void nndmseal_state::nndmseal_b20000_w(u16 data)
{
	// leds?
//  popmessage("%04x",data);
}

void nndmseal_state::nndmseal_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram().share(m_spriteram);   // Object RAM
	map(0x104000, 0x107fff).ram(); // Spare Object RAM
	map(0x108000, 0x10ffff).ram(); // Work RAM
	map(0x200000, 0x23ffff).rw(FUNC(nndmseal_state::priority_r), FUNC(nndmseal_state::priority_w));
	map(0x300000, 0x31ffff).ram().w(FUNC(nndmseal_state::palette_w)).share(m_paletteram);    // Palette
	map(0x400000, 0x403fff).ram().w(FUNC(nndmseal_state::vram_fg_w)).share(m_vram_fg);   // Foreground
	map(0x404000, 0x407fff).ram().w(FUNC(nndmseal_state::vram_bg_w)).share(m_vram_bg);   // Background

	map(0x408000, 0x409fff).ram(); // ???
	map(0x500000, 0x50ffff).ram(); // Line

	map(0x600000, 0x60ffff).ram().w(FUNC(nndmseal_state::vram_rot_w)).share(m_vram_rot); // Rotation
	map(0x650000, 0x651fff).ram().w(FUNC(nndmseal_state::vram_rot_w));  // Rotation (mirror)

	map(0x800000, 0x800003).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write)).umask16(0x00ff); // Sound

	map(0x900000, 0x903fff).rw(FUNC(nndmseal_state::tetrisp2_nvram_r), FUNC(nndmseal_state::nvram_w)).share("nvram"); // NVRAM

	map(0xb00000, 0xb00001).w(FUNC(nndmseal_state::nndmseal_coincounter_w));   // Coin Counter
	map(0xb20000, 0xb20001).w(FUNC(nndmseal_state::nndmseal_b20000_w));        // ???

	map(0xb40000, 0xb4000b).writeonly().share(m_scroll_fg); // Foreground Scrolling
	map(0xb40010, 0xb4001b).writeonly().share(m_scroll_bg); // Background Scrolling
	map(0xb4003e, 0xb4003f).nopw();    // scr_size

	map(0xb60000, 0xb6002f).writeonly().share(m_rotregs);   // Rotation Registers

	map(0xb80001, 0xb80001).w(FUNC(nndmseal_state::nndmseal_sound_bank_w));

	map(0xba0000, 0xba001f).m(m_sysctrl, FUNC(jaleco_ms32_sysctrl_device::amap));

	map(0xbe0000, 0xbe0001).nopr(); // INT-level1 dummy read
	map(0xbe0002, 0xbe0003).portr("BUTTONS");   // Inputs
	map(0xbe0004, 0xbe0005).portr("COINS");   // ""
	map(0xbe0006, 0xbe0007).portr("PRINT");   // ""
	map(0xbe0008, 0xbe0009).portr("DSW");   // ""

	map(0xbe000a, 0xbe000b).r("watchdog", FUNC(watchdog_timer_device::reset16_r));
}

void nndmseal_state::nndmseal_oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).bankr(m_okibank[0]);
	map(0x20000, 0x3ffff).bankr(m_okibank[1]);
}


void rockn_state::rockn1_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                         // ROM
	map(0x100000, 0x103fff).ram().share(m_spriteram);           // Object RAM
	map(0x104000, 0x107fff).ram();                                                         // Spare Object RAM
	map(0x108000, 0x10ffff).ram();                                                         // Work RAM
	map(0x200000, 0x23ffff).rw(FUNC(rockn_state::priority_r), FUNC(rockn_state::priority_w));
	map(0x300000, 0x31ffff).ram().w(FUNC(rockn_state::palette_w)).share(m_paletteram);        // Palette
	map(0x400000, 0x403fff).ram().w(FUNC(rockn_state::vram_fg_w)).share(m_vram_fg);   // Foreground
	map(0x404000, 0x407fff).ram().w(FUNC(rockn_state::vram_bg_w)).share(m_vram_bg);   // Background
	map(0x408000, 0x409fff).ram();                                                         // ???
	map(0x500000, 0x50ffff).ram();                                                         // Line
	map(0x600000, 0x60ffff).ram().w(FUNC(rockn_state::vram_rot_w)).share(m_vram_rot); // Rotation
	map(0x900000, 0x903fff).r(FUNC(rockn_state::rockn_nvram_r)).w(FUNC(rockn_state::nvram_w)).share("nvram");    // NVRAM
	map(0xa30000, 0xa30001).rw(FUNC(rockn_state::rockn_soundvolume_r), FUNC(rockn_state::rockn_soundvolume_w));         // Sound Volume
	map(0xa40000, 0xa40003).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask16(0x00ff);   // Sound
	map(0xa44000, 0xa44001).rw(FUNC(rockn_state::rockn_adpcmbank_r), FUNC(rockn_state::rockn_adpcmbank_w));             // Sound Bank
	map(0xa48000, 0xa48001).noprw();                                                         // YMZ280 Reset
	map(0xb00000, 0xb00001).w(FUNC(rockn_state::tetrisp2_coincounter_w));                               // Coin Counter
	map(0xb20000, 0xb20001).noprw();                                                         // ???
	map(0xb40000, 0xb4000b).writeonly().share(m_scroll_fg);                     // Foreground Scrolling
	map(0xb40010, 0xb4001b).writeonly().share(m_scroll_bg);                     // Background Scrolling
	map(0xb4003e, 0xb4003f).nopw();                                                    // scr_size
	map(0xb60000, 0xb6002f).writeonly().share(m_rotregs);                       // Rotation Registers
	map(0xba0000, 0xba001f).m(m_sysctrl, FUNC(jaleco_ms32_sysctrl_device::amap));
	map(0xbe0000, 0xbe0001).nopr();                                                     // INT-level1 dummy read
	map(0xbe0002, 0xbe0003).portr("PLAYERS");                                        // Inputs
	map(0xbe0004, 0xbe0005).portr("SYSTEM");                                         // Inputs
	map(0xbe0008, 0xbe0009).portr("DSW");                                            // Inputs
	map(0xbe000a, 0xbe000b).r("watchdog", FUNC(watchdog_timer_device::reset16_r));       // Watchdog
}

void rockn_state::rockn1_ymz_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom().region("ymz", 0);
	map(0x400000, 0xffffff).bankr(m_ymzbank[0]);
}


void rockn_state::rockn2_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                         // ROM
	map(0x100000, 0x103fff).ram().share(m_spriteram);           // Object RAM
	map(0x104000, 0x107fff).ram();                                                         // Spare Object RAM
	map(0x108000, 0x10ffff).ram();                                                         // Work RAM
	map(0x200000, 0x23ffff).rw(FUNC(rockn_state::priority_r), FUNC(rockn_state::priority_w));
	map(0x300000, 0x31ffff).ram().w(FUNC(rockn_state::palette_w)).share(m_paletteram);        // Palette
	map(0x500000, 0x50ffff).ram();                                                         // Line
	map(0x600000, 0x60ffff).ram().w(FUNC(rockn_state::vram_rot_w)).share(m_vram_rot); // Rotation
	map(0x800000, 0x803fff).ram().w(FUNC(rockn_state::vram_fg_w)).share(m_vram_fg);   // Foreground
	map(0x804000, 0x807fff).ram().w(FUNC(rockn_state::vram_bg_w)).share(m_vram_bg);   // Background
	map(0x808000, 0x809fff).ram();                                                         // ???
	map(0x900000, 0x903fff).r(FUNC(rockn_state::rockn_nvram_r)).w(FUNC(rockn_state::nvram_w)).share("nvram");    // NVRAM
	map(0xa30000, 0xa30001).rw(FUNC(rockn_state::rockn_soundvolume_r), FUNC(rockn_state::rockn_soundvolume_w));         // Sound Volume
	map(0xa40000, 0xa40003).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask16(0x00ff);   // Sound
	map(0xa44000, 0xa44001).rw(FUNC(rockn_state::rockn_adpcmbank_r), FUNC(rockn_state::rockn2_adpcmbank_w));            // Sound Bank
	map(0xa48000, 0xa48001).nopw();                                                    // YMZ280 Reset
	map(0xb00000, 0xb00001).w(FUNC(rockn_state::tetrisp2_coincounter_w));                               // Coin Counter
	map(0xb20000, 0xb20001).nopw();                                                    // ???
	map(0xb40000, 0xb4000b).writeonly().share(m_scroll_fg);                 // Foreground Scrolling
	map(0xb40010, 0xb4001b).writeonly().share(m_scroll_bg);                 // Background Scrolling
	map(0xb4003e, 0xb4003f).nopw();                                                    // scr_size
	map(0xb60000, 0xb6002f).writeonly().share(m_rotregs);                   // Rotation Registers
	map(0xba0000, 0xba001f).m(m_sysctrl, FUNC(jaleco_ms32_sysctrl_device::amap));
	map(0xbe0000, 0xbe0001).nopr();                                                     // INT-level1 dummy read
	map(0xbe0002, 0xbe0003).portr("PLAYERS");                                        // Inputs
	map(0xbe0004, 0xbe0005).portr("SYSTEM");                                         // Inputs
	map(0xbe0008, 0xbe0009).portr("DSW");                                            // Inputs
	map(0xbe000a, 0xbe000b).r("watchdog", FUNC(watchdog_timer_device::reset16_r));       // Watchdog
}

void rockn_state::rockn2_ymz_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom().region("ymz", 0);
	map(0x400000, 0x7fffff).bankr(m_ymzbank[0]);
	map(0x800000, 0xbfffff).bankr(m_ymzbank[1]);
	map(0xc00000, 0xffffff).bankr(m_ymzbank[2]);
}


void rocknms_state::rocknms_main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                         // ROM
	map(0x100000, 0x103fff).ram().share(m_spriteram);           // Object RAM
	map(0x104000, 0x107fff).ram();                                                         // Spare Object RAM
	map(0x108000, 0x10ffff).ram();                                                         // Work RAM
	map(0x200000, 0x23ffff).rw(FUNC(rocknms_state::priority_r), FUNC(rocknms_state::priority_w));
	map(0x300000, 0x31ffff).ram().w(FUNC(rocknms_state::palette_w)).share(m_paletteram);        // Palette
//  map(0x500000, 0x50ffff).ram();                                                         // Line
	map(0x600000, 0x60ffff).ram().w(FUNC(rocknms_state::vram_rot_w)).share(m_vram_rot); // Rotation
	map(0x800000, 0x803fff).ram().w(FUNC(rocknms_state::vram_fg_w)).share(m_vram_fg);   // Foreground
	map(0x804000, 0x807fff).ram().w(FUNC(rocknms_state::vram_bg_w)).share(m_vram_bg);   // Background
//  map(0x808000, 0x809fff).ram();                                                         // ???
	map(0x900000, 0x903fff).r(FUNC(rocknms_state::rockn_nvram_r)).w(FUNC(rocknms_state::nvram_w)).share("nvram");    // NVRAM
	map(0xa30000, 0xa30001).rw(FUNC(rocknms_state::rockn_soundvolume_r), FUNC(rocknms_state::rockn_soundvolume_w));         // Sound Volume
	map(0xa40000, 0xa40003).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask16(0x00ff);   // Sound
	map(0xa44000, 0xa44001).rw(FUNC(rocknms_state::rockn_adpcmbank_r), FUNC(rocknms_state::rockn_adpcmbank_w));             // Sound Bank
	map(0xa48000, 0xa48001).nopw();                                                    // YMZ280 Reset
	map(0xa00000, 0xa00001).w(FUNC(rocknms_state::main2sub_w));                                   // MAIN -> SUB Communication
	map(0xb00000, 0xb00001).w(FUNC(rocknms_state::tetrisp2_coincounter_w));                               // Coin Counter
	map(0xb20000, 0xb20001).nopw();                                                    // ???
	map(0xb40000, 0xb4000b).writeonly().share(m_scroll_fg);                     // Foreground Scrolling
	map(0xb40010, 0xb4001b).writeonly().share(m_scroll_bg);                     // Background Scrolling
	map(0xb4003e, 0xb4003f).nopw();                                                    // scr_size
	map(0xb60000, 0xb6002f).writeonly().share(m_rotregs);                       // Rotation Registers
	map(0xba0000, 0xba001f).m(m_sysctrl, FUNC(jaleco_ms32_sysctrl_device::amap));
	map(0xbe0000, 0xbe0001).nopr();                                                     // INT-level1 dummy read
	map(0xbe0002, 0xbe0003).portr("PLAYERS");
	map(0xbe0004, 0xbe0005).portr("SYSTEM");                                         // Inputs
	map(0xbe0008, 0xbe0009).portr("DSW");                                            // Inputs
	map(0xbe000a, 0xbe000b).r("watchdog", FUNC(watchdog_timer_device::reset16_r));       // Watchdog
}


void rocknms_state::rocknms_sub_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                         // ROM
	map(0x100000, 0x103fff).ram().share(m_spriteram2);      // Object RAM
	map(0x104000, 0x107fff).ram();                                                         // Spare Object RAM
	map(0x108000, 0x10ffff).ram();                                                         // Work RAM
	map(0x200000, 0x23ffff).ram().w(FUNC(rocknms_state::sub_priority_w)).share(m_sub_priority); // Priority
	map(0x300000, 0x31ffff).ram().w(FUNC(rocknms_state::sub_palette_w)).share(m_sub_paletteram);    // Palette
//  map(0x500000, 0x50ffff).ram();                                                         // Line
	map(0x600000, 0x60ffff).ram().w(FUNC(rocknms_state::sub_vram_rot_w)).share(m_sub_vram_rot); // Rotation
	map(0x800000, 0x803fff).ram().w(FUNC(rocknms_state::sub_vram_fg_w)).share(m_sub_vram_fg); // Foreground
	map(0x804000, 0x807fff).ram().w(FUNC(rocknms_state::sub_vram_bg_w)).share(m_sub_vram_bg); // Background
//  map(0x808000, 0x809fff).ram();                                                         // ???
	map(0x900000, 0x907fff).ram();                                                         // NVRAM
	map(0xa30000, 0xa30001).w(FUNC(rocknms_state::rockn_soundvolume_w));                                  // Sound Volume
	map(0xa40000, 0xa40003).w("ymz", FUNC(ymz280b_device::write)).umask16(0x00ff);             // Sound
	map(0xa44000, 0xa44001).w(FUNC(rocknms_state::rockn_adpcmbank_w));                                    // Sound Bank
	map(0xa48000, 0xa48001).nopw();                                                    // YMZ280 Reset
	map(0xb00000, 0xb00001).w(FUNC(rocknms_state::sub2main_w));                                   // MAIN <- SUB Communication
	map(0xb20000, 0xb20001).nopw();                                                    // ???
	map(0xb40000, 0xb4000b).writeonly().share(m_sub_scroll_fg);                 // Foreground Scrolling
	map(0xb40010, 0xb4001b).writeonly().share(m_sub_scroll_bg);                 // Background Scrolling
	map(0xb4003e, 0xb4003f).nopw();                                                    // scr_size
	map(0xb60000, 0xb6002f).writeonly().share(m_sub_rotregs);                       // Rotation Registers
	map(0xba0000, 0xba001f).m(m_sub_sysctrl, FUNC(jaleco_ms32_sysctrl_device::amap));
//  map(0xbe0000, 0xbe0001).nopr();                                                     // INT-level1 dummy read
	map(0xbe0002, 0xbe0003).rw(FUNC(rocknms_state::main2sub_r), FUNC(rocknms_state::sub2main_w));           // MAIN <-> SUB Communication
	map(0xbe000a, 0xbe000b).r("watchdog", FUNC(watchdog_timer_device::reset16_r));       // Watchdog
}


/***************************************************************************

                              Stepping Stage

***************************************************************************/

u16 stepstag_state::stepstag_soundvolume_r()
{
	const u8 vr1 = (64 - m_soundvr[0]->read()) << 2;
	const u8 vr2 = m_soundvr[1] ? (64 - m_soundvr[1]->read()) << 2 : 0; // Doesn't exist in Stepping Stage
	return (vr2 << 8) | vr1;
}

u16 stepstag_state::stepstag_coins_r()
{
	// bits 8 & 9?
	return  (m_io_coins->read() &  0xfcff) |
			(m_vj_upload_fini ? 0x300 : 0x100);
}

void stepstag_state::stepstag_b20000_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_vj_upload_idx++;

	if (m_vj_upload_idx >= 0xa1a8)  // 0x14350/2
		m_vj_upload_fini = true;
}

void stepstag_state::stepstag_b00000_w(u16 data)
{
	m_vj_upload_idx = 0;
	m_vj_upload_fini = false;
}

u16 stepstag_state::stepstag_sprite_status_status_r()
{
	// Guessed based on usage, but this is checked to make sure
	// the sprite chips on subboard are ready before using them
	return 3;
}

u16 stepstag_state::unknown_read_0xffff00()
{
	return machine().rand();
}

void stepstag_state::stepstag_soundlatch_word_w(u16 data)
{
	m_soundlatch->write(data);

	m_subcpu->set_input_line(M68K_IRQ_6, HOLD_LINE);

	machine().scheduler().perfect_quantum(attotime::from_usec(100));
}


void stepstag_state::stepstag_neon_w(offs_t offset, u16 data, u16 mem_mask)
{
//  1f60, 1c60, 0790, 0490, 0b60, 0860, 1390, 1090, 1f60, 1c60, 1390, 1090, 0b60, 0860, 0790, 0490, ...
	if (ACCESSING_BITS_0_7)
	{
		m_leds[32] = 0; //data & 0x0000; // Spot Lights?
		m_leds[33] = BIT(data, 4);
		m_leds[34] = BIT(data, 5);
		m_leds[35] = BIT(data, 6);
	}

	if (ACCESSING_BITS_8_15)
	{
		m_leds[40] = BIT(data, 8);
		m_leds[41] = BIT(data, 9);

		m_leds[42] = BIT(data, 10);
		m_leds[43] = BIT(data, 11);
		m_leds[44] = BIT(data, 12);
	}
}

void stepstag_state::stepstag_step_leds_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_leds[0] = BIT(data, 0); // P2 Front-Left
		m_leds[1] = BIT(data, 1); // P2 Front-Right
		m_leds[2] = BIT(data, 2); // P2 Left
		m_leds[3] = BIT(data, 3); // P2 Right
		m_leds[4] = BIT(data, 4); // P2 Back-Left
		m_leds[5] = BIT(data, 5); // P2 Back-Right
	}

	if (ACCESSING_BITS_8_15)
	{
		m_leds[6] = BIT(data, 8); // P1 Front-Left
		m_leds[7] = BIT(data, 9); // P1 Front-Right
		m_leds[8] = BIT(data, 10); // P1 Left
		m_leds[9] = BIT(data, 11); // P1 Right
		m_leds[10] = BIT(data, 12); // P1 Back-Left
		m_leds[11] = BIT(data, 13); // P1 Back-Right
	}
}

void stepstag_state::stepstag_button_leds_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		m_leds[17] = BIT(data, 8); // P2 Start
		m_leds[18] = BIT(data, 9); // P2 Left
		m_leds[19] = BIT(data, 10); // P2 Right
		m_leds[20] = BIT(data, 12); // P1 Start
		m_leds[21] = BIT(data, 13); // P1 Left
		m_leds[22] = BIT(data, 14); // P1 Right
	}
}

void stepstag_state::stepstag_spriteram1_updated_w(u16 data)
{
	// There's a timing issue with updating the spriteram buffers if the sprite buffers are used in place when
	// updating the screen because the data can be partially overwritten (cleared or updating) when a screen update
	// is performed before IRQ4 finishes.
	//
	// All 3 spriteram buffers get updated at the same time so I don't believe IRQ4 updates are directly linked to any
	// individual monitor. So there is probably another mechanism for triggering IRQ4 as noted in the field_cb timer callback.
	//
	// 0x880000/0x980000/0xa80000 is written to (always just 0) every time the update for that monitor is finished pushing to RAM,
	// so I think the intended method is for those registers to be used to signal that the sprite RAM is updated and/or to copy
	// the data elsewhere for rendering until the next sprite buffer update is pushed.
	//
	// The difference between the way the main PCB handles sprites vs the subboard PCB handles sprites is probably
	// because they used 3 FPGAs (+ a smaller core FPGA to tie them together?) for sprite rendering instead of the
	// Jaleco branded chips as seen on the main PCB.
	for (int i = 0; i < 0x400; i++)
		m_spriteram1_data[i] = m_spriteram1[i];
}

void stepstag_state::stepstag_spriteram2_updated_w(u16 data)
{
	for (int i = 0; i < 0x400; i++)
		m_spriteram2_data[i] = m_spriteram2[i];
}

void stepstag_state::stepstag_spriteram3_updated_w(u16 data)
{
	for (int i = 0; i < 0x400; i++)
		m_spriteram3_data[i] = m_spriteram3[i];
}

void stepstag_state::adv7176a_w(u16 data)
{
	// Used to configure the video encoder chips on the subboard.
	// Only meant to be used for debug output to help figure out the desired screen parameters for now.
	constexpr int STATE_IDLE = 0;
	constexpr int STATE_READ_SLAVEADDR = 1;
	constexpr int STATE_READ_SUBADDR = 2;
	constexpr int STATE_READ_DATA = 3;

	int clk = BIT(data, 0);
	int curbit = BIT(data, 1);

	if (m_adv7176a_sclock == 1 && clk == 0)
	{
		m_adv7176a_sclock = clk;
		m_adv7176a_sdata = curbit;
		return;
	}

	if (m_adv7176a_sclock == 0 && clk == 1)
	{
		if (m_adv7176a_shift < 8)
			m_adv7176a_byte |= curbit << (7 - m_adv7176a_shift);
		m_adv7176a_shift++;
	}

	if (m_adv7176a_sclock == 1 && clk == 1)
	{
		// Start bit and end bit are only recognized when the data line changes while the clock is high
		if (m_adv7176a_sdata == 1 && curbit == 0)
		{
			m_adv7176a_state = STATE_READ_SLAVEADDR;
			m_adv7176a_shift = m_adv7176a_byte = 0;
			logerror("[video encoder] start bit found\n");
		}
		else if (m_adv7176a_sdata == 0 && curbit == 1)
		{
			m_adv7176a_state = STATE_IDLE;
			m_adv7176a_shift = m_adv7176a_byte = 0;
			logerror("[video encoder] stop bit found\n\n");
		}
	}
	else if (m_adv7176a_state == STATE_READ_SLAVEADDR)
	{
		// Receive slave addr
		if (m_adv7176a_shift > 8)
		{
			logerror("[video encoder] slave addr %02x %c\n", BIT(m_adv7176a_byte, 0, 7), BIT(m_adv7176a_byte, 8) ? 'r' : 'w');
			m_adv7176a_shift = m_adv7176a_byte = 0;
			m_adv7176a_state = STATE_READ_SUBADDR;
		}
	}
	else if (m_adv7176a_state == STATE_READ_SUBADDR)
	{
		// Receive sub addr
		if (m_adv7176a_shift > 8)
		{
			m_adv7176a_subaddr = m_adv7176a_byte;
			m_adv7176a_shift = m_adv7176a_byte = 0;
			m_adv7176a_state = STATE_READ_DATA;
			logerror("[video encoder] addr %02x\n", m_adv7176a_subaddr);
		}
	}
	else if (m_adv7176a_state == STATE_READ_DATA)
	{
		// Receive data
		if (m_adv7176a_shift > 8)
		{
			logerror("[video encoder] data %02x\n", m_adv7176a_byte);

			if (m_adv7176a_subaddr == 0x00)
			{
				// Mode register 0
				auto const encode_mode_control = BIT(m_adv7176a_byte, 0, 2);
				auto const pedestal_control = BIT(m_adv7176a_byte, 2);
				auto const luminance_filter_control = BIT(m_adv7176a_byte, 3, 2);
				auto const rgb_sync = BIT(m_adv7176a_byte, 5);
				auto const output_control = BIT(m_adv7176a_byte, 6);

				logerror("Mode register 0\n");
				logerror("\tencode mode control %02x ", encode_mode_control);
				switch (encode_mode_control)
				{
					case 0b00: logerror("NTSC\n"); break;
					case 0b01: logerror("PAL (B, D, G, H, I)\n"); break;
					case 0b10: logerror("PAL (M)\n"); break;
					case 0b11: logerror("Reserved\n"); break;
				}

				logerror("\tpedestal control %02x %s\n", pedestal_control, pedestal_control ? "on" : "off");

				logerror("\tluminance filter control %02x ", luminance_filter_control);
				switch (luminance_filter_control)
				{
					case 0b00: logerror("Low pass filter (A)\n"); break;
					case 0b01: logerror("Notch filter\n"); break;
					case 0b10: logerror("Extended mode\n"); break;
					case 0b11: logerror("Low pass filter (B)\n"); break;
				}

				logerror("\trgb sync %02x %s\n", rgb_sync, rgb_sync ? "on" : "off");
				logerror("\toutput control %02x %s\n", output_control, output_control ? "RGB/YUV" : "YC");
			}
			else if (m_adv7176a_subaddr == 0x01)
			{
				// Mode register 1
				auto const interlaced_mode_control = BIT(m_adv7176a_byte, 0);
				auto const closed_captioning_field_control = BIT(m_adv7176a_byte, 1, 2);
				auto const dac_c_control = BIT(m_adv7176a_byte, 3);
				auto const dac_d_control = BIT(m_adv7176a_byte, 4);
				auto const dac_b_control = BIT(m_adv7176a_byte, 5);
				auto const dac_a_control = BIT(m_adv7176a_byte, 6);
				auto const color_bar_control = BIT(m_adv7176a_byte, 7);

				logerror("Mode register 1\n");
				logerror("\tinterlaced mode control %02x %s\n", interlaced_mode_control, interlaced_mode_control ? "non-interlaced" : "interlaced");
				logerror("\tclosed captioning field control %02x ", closed_captioning_field_control);
				switch (closed_captioning_field_control)
				{
					case 0b00: logerror("No data out\n"); break;
					case 0b01: logerror("Odd field only\n"); break;
					case 0b10: logerror("Even field only\n"); break;
					case 0b11: logerror("Data out (both fields)\n"); break;
				}

				logerror("\tdac a control %02x %s\n", dac_a_control, dac_a_control ? "power-down" : "normal");
				logerror("\tdac b control %02x %s\n", dac_b_control, dac_b_control ? "power-down" : "normal");
				logerror("\tdac c control %02x %s\n", dac_c_control, dac_c_control ? "power-down" : "normal");
				logerror("\tdac d control %02x %s\n", dac_d_control, dac_d_control ? "power-down" : "normal");
				logerror("\tcolor bar control %02x %s\n", color_bar_control, color_bar_control ? "enabled" : "disabled");
			}
			else if (m_adv7176a_subaddr >= 0x02 && m_adv7176a_subaddr <= 0x05)
			{
				// Mode register 1
				auto const r = 5 - m_adv7176a_subaddr;
				logerror("Subcarrier frequency register %d %02x\n", r, m_adv7176a_byte);
			}
			else if (m_adv7176a_subaddr == 0x07)
			{
				// Timing register 0
				auto const masterslave_control = BIT(m_adv7176a_byte, 0);
				auto const timing_mode = BIT(m_adv7176a_byte, 1, 2);
				auto const blank_control = BIT(m_adv7176a_byte, 3);
				auto const luma_delay_control = BIT(m_adv7176a_byte, 4, 2);
				auto const pixel_port_control = BIT(m_adv7176a_byte, 6);
				auto const timing_register_reset = BIT(m_adv7176a_byte, 7);

				logerror("Timing register 0\n");
				logerror("\tmaster/slave control %02x %s\n", masterslave_control, masterslave_control ? "master timing" : "slave timing");
				logerror("\ttiming mode selection %02x mode %d\n", timing_mode, timing_mode);
				logerror("\tblank control %02x %s\n", blank_control, blank_control ? "disable" : "enable");

				logerror("\tluma delay %02x ", luma_delay_control);
				switch (luma_delay_control)
				{
					case 0b00: logerror("0ns delay\n"); break;
					case 0b01: logerror("74ns delay\n"); break;
					case 0b10: logerror("148ns delay\n"); break;
					case 0b11: logerror("222ns delay\n"); break;
				}

				logerror("\tpixel port control %02x %s\n", pixel_port_control, pixel_port_control ? "16-bit" : "8-bit");
				logerror("\ttiming register reset %02x\n", timing_register_reset);
			}
			else if (m_adv7176a_subaddr == 0x0c)
			{
				// Timing register 1
				auto const hsync_width = BIT(m_adv7176a_byte, 0, 2);
				auto const hsync_to_vsync_delay = BIT(m_adv7176a_byte, 2, 2);
				auto const r = BIT(m_adv7176a_byte, 4, 2);
				auto const hsync_to_pixel_adjust = BIT(m_adv7176a_byte, 6, 2);

				logerror("Timing register 1\n");
				auto d = 1;
				switch (hsync_width)
				{
					case 0b00: d = 1; break;
					case 0b01: d = 4; break;
					case 0b10: d = 16; break;
					case 0b11: d = 128; break;
				}
				logerror("\thsync width %02x %d\n", hsync_width, d);
				switch (hsync_to_vsync_delay)
				{
					case 0b00: d = 0; break;
					case 0b01: d = 4; break;
					case 0b10: d = 8; break;
					case 0b11: d = 16; break;
				}
				logerror("\thsync to field/vsync delay %02x %d\n", hsync_to_vsync_delay, d);
				logerror("\ttr14-tr15 %02x\n", r);
				logerror("\thsync to pixel data adjustment %02x %lf\n", hsync_to_pixel_adjust);
			}
			else if (m_adv7176a_subaddr == 0x0d)
			{
				// Mode register 2
				auto const square_pixel_mode_control = BIT(m_adv7176a_byte, 0);
				auto const genlock_control = BIT(m_adv7176a_byte, 1, 2);
				auto const active_video_line_control = BIT(m_adv7176a_byte, 3);
				auto const chrominance_control = BIT(m_adv7176a_byte, 4);
				auto const burst_control = BIT(m_adv7176a_byte, 5);
				auto const rgb_yuv_control = BIT(m_adv7176a_byte, 6);
				auto const lower_power_control = BIT(m_adv7176a_byte, 7);

				logerror("Mode register 2\n");
				logerror("\tsquare pixel control %02x %s\n", square_pixel_mode_control, square_pixel_mode_control ? "enable" : "disable");
				logerror("\tgenlock control %02x ", genlock_control);
				switch (genlock_control)
				{
					case 0b00: case 0b10:
						logerror("Disable genlock\n"); break;
					case 0b01: logerror("Enable subcarrier reset pin\n"); break;
					case 0b11: logerror("Enable RTC pin\n"); break;
				}

				logerror("\tactive video line control %02x %s\n", active_video_line_control, active_video_line_control ? "ITU-R/SMPTE active line" : "720 pixels active line");
				logerror("\tchrominance control %02x %s\n", chrominance_control, chrominance_control ? "disable color" : "enable color");
				logerror("\tburst control %02x %s\n", burst_control, burst_control ? "disable burst" : "enable burst");
				logerror("\tRGB/YUV control %02x %s\n", rgb_yuv_control, rgb_yuv_control ? "YUV output" : "RGB output");
				logerror("\tlower power control %02x %s\n", lower_power_control, lower_power_control ? "enable" : "disabled");
			}
			else if (m_adv7176a_subaddr >= 0x0e && m_adv7176a_subaddr <= 0x11)
			{
				auto const r = 0x11 - m_adv7176a_subaddr;
				logerror("NTSC pedestal/PAL teletext register %d %02x\n", r, m_adv7176a_byte);
			}
			else if (m_adv7176a_subaddr == 0x12)
			{
				// Mode register 3
				auto const vbi_passthrough_control = BIT(m_adv7176a_byte, 1);
				auto const teletext_enable = BIT(m_adv7176a_byte, 4);
				auto const input_default_color = BIT(m_adv7176a_byte, 6);
				auto const dac_switching_control = BIT(m_adv7176a_byte, 7);

				logerror("Mode register 3\n");
				logerror("\tVBI passthrough control %02x %s\n", vbi_passthrough_control, vbi_passthrough_control ? "enable" : "disable");
				logerror("\tteletext enable %02x %s\n", teletext_enable, teletext_enable ? "enable" : "disable");
				logerror("\tinput default color %02x %s\n", input_default_color, input_default_color ? "black" : "input color");
				logerror("\tDAC switching control %02x dac a[%s] dac b[blue/comp/u] dac c[red/chroma/v] dac d[%s]\n", dac_switching_control, dac_switching_control ? "green/luma/y" : "composite", dac_switching_control ? "composite" : "green/luma/y");
			}
			else
			{
				logerror("Unknown subaddr! %02x\n", m_adv7176a_subaddr);
			}

			m_adv7176a_shift = m_adv7176a_byte = 0;
			m_adv7176a_state = 3;
		}
	}

	m_adv7176a_sclock = clk;
	m_adv7176a_sdata = curbit;
}

// Main CPU
void stepstag_state::stepstag_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x103fff).ram().share(m_spriteram);                                   // Object RAM
	map(0x108000, 0x10ffff).ram();                                                         // Work RAM
	map(0x200000, 0x23ffff).rw(FUNC(stepstag_state::priority_r), FUNC(stepstag_state::priority_w));
	map(0x300000, 0x31ffff).ram().w(FUNC(stepstag_state::palette_w)).share(m_paletteram);        // Palette
	map(0x400000, 0x403fff).ram().w(FUNC(stepstag_state::vram_fg_w)).share(m_vram_fg);           // Foreground
	map(0x404000, 0x407fff).ram().w(FUNC(stepstag_state::vram_bg_w)).share(m_vram_bg);           // Background
//  map(0x408000, 0x409fff).ram();                                                         // ???
	map(0x500000, 0x50ffff).ram();                                                         // Line
	map(0x600000, 0x60ffff).ram().w(FUNC(stepstag_state::vram_rot_w)).share(m_vram_rot);         // Rotation
	map(0x900000, 0x903fff).rw(FUNC(stepstag_state::rockn_nvram_r), FUNC(stepstag_state::nvram_w)).share("nvram"); // NVRAM
//  map(0x904000, 0x907fff).rw(FUNC(stepstag_state::rockn_nvram_r), FUNC(stepstag_state::nvram_w);                 // NVRAM (mirror)
	map(0xa00000, 0xa00001).nopr().w(FUNC(stepstag_state::stepstag_neon_w));  // Neon??
	map(0xa10000, 0xa10001).portr("RHYTHM").w(FUNC(stepstag_state::stepstag_step_leds_w));          // I/O
	map(0xa20000, 0xa20001).nopr().w(FUNC(stepstag_state::stepstag_button_leds_w));                    // I/O
	map(0xa30000, 0xa30001).r(FUNC(stepstag_state::stepstag_soundvolume_r)).nopw();         // Sound Volume
	map(0xa42000, 0xa42001).r(m_jaleco_vj_pc, FUNC(jaleco_vj_pc_device::response_r));
	map(0xa44000, 0xa44001).nopr();     // watchdog
	map(0xa48000, 0xa48001).w(m_jaleco_vj_pc, FUNC(jaleco_vj_pc_device::comm_w));
	// map(0xa4c000, 0xa4c001).noprw(); // Related to 0xa60000
	map(0xa50000, 0xa50001).r(m_soundlatch, FUNC(generic_latch_16_device::read)).w(FUNC(stepstag_state::stepstag_soundlatch_word_w));
	map(0xa60000, 0xa60003).w(m_jaleco_vj_pc, FUNC(jaleco_vj_pc_device::ymz_w));

	map(0xb00000, 0xb00001).w(FUNC(stepstag_state::stepstag_b00000_w));                                    // init xilinx uploading??
	map(0xb20000, 0xb20001).w(FUNC(stepstag_state::stepstag_b20000_w));                                    // 98343 interface board xilinx uploading?
	map(0xb40000, 0xb4000b).writeonly().share(m_scroll_fg);                             // Foreground Scrolling
	map(0xb40010, 0xb4001b).writeonly().share(m_scroll_bg);                             // Background Scrolling
	map(0xb4003e, 0xb4003f).ram();                                                         // scr_size
	map(0xb60000, 0xb6002f).writeonly().share(m_rotregs);                               // Rotation Registers
	map(0xba0000, 0xba001f).m(m_sysctrl, FUNC(jaleco_ms32_sysctrl_device::amap));
	map(0xbe0000, 0xbe0001).nopr();                                                     // INT-level1 dummy read
	map(0xbe0002, 0xbe0003).portr("BUTTONS");                                        // Inputs
	map(0xbe0004, 0xbe0005).r(FUNC(stepstag_state::stepstag_coins_r));                                      // Inputs & protection
	map(0xbe0008, 0xbe0009).portr("DSW");                                            // Inputs
	map(0xbe000a, 0xbe000b).r("watchdog", FUNC(watchdog_timer_device::reset16_r)).nopw();       // Watchdog
}

// Sub CPU (sprites)
void stepstag_state::stepstag_sub_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x20ffff).ram();

	map(0x300000, 0x33ffff).ram().w(FUNC(stepstag_state::palette_left_w)).share(m_vj_paletteram_l);
	map(0x400000, 0x43ffff).ram().w(FUNC(stepstag_state::palette_mid_w)).share(m_vj_paletteram_m);
	map(0x500000, 0x53ffff).ram().w(FUNC(stepstag_state::palette_right_w)).share(m_vj_paletteram_r);

	map(0x700000, 0x700001).w(m_jaleco_vj_pc, FUNC(jaleco_vj_pc_device::video_mix_w<0>));
	map(0x700002, 0x700003).w(m_jaleco_vj_pc, FUNC(jaleco_vj_pc_device::video_mix_w<1>));
	map(0x700004, 0x700005).w(m_jaleco_vj_pc, FUNC(jaleco_vj_pc_device::video_mix_w<2>));
	map(0x700006, 0x700007).w(m_jaleco_vj_pc, FUNC(jaleco_vj_pc_device::video_control_w));

	// left screen sprites
	map(0x800000, 0x8007ff).ram().share(m_spriteram1);      // Object RAM
	map(0x800800, 0x87ffff).ram();
	map(0x880000, 0x880001).w(FUNC(stepstag_state::stepstag_spriteram1_updated_w));
//  map(0x8c0000, 0x8c0001).nopw(); // cleared at boot

	// middle screen sprites
	map(0x900000, 0x9007ff).ram().share(m_spriteram2);      // Object RAM
	map(0x900800, 0x97ffff).ram();
	map(0x980000, 0x980001).w(FUNC(stepstag_state::stepstag_spriteram2_updated_w));
//  map(0x9c0000, 0x9c0001).nopw(); // cleared at boot

	// right screen sprites
	map(0xa00000, 0xa007ff).ram().share(m_spriteram3);      // Object RAM
	map(0xa00800, 0xa7ffff).ram();
	map(0xa80000, 0xa80001).w(FUNC(stepstag_state::stepstag_spriteram3_updated_w));
//  map(0xac0000, 0xac0001).nopw(); // cleared at boot

	// The code for PC comms fully exists in VJ but it doesn't appear to ever be called
	//map(0xa42000, 0xa42001).r(m_jaleco_vj_pc, FUNC(jaleco_vj_pc_device::response_r));
	//map(0xa44000, 0xa44001).nopr();     // watchdog
	//map(0xa48000, 0xa48001).w(m_jaleco_vj_pc, FUNC(jaleco_vj_pc_device::comm_w));

	map(0xb00000, 0xb00001).rw(m_soundlatch, FUNC(generic_latch_16_device::read), FUNC(generic_latch_16_device::write));

	// A write here likely locks the sprite RAM buffers for batch updating.
	// Stepping Stage and VJ both will set this register to 1, clear 0x800000-0x880000
	// and also the ranges for spriteram2 and spriteram3, then set this register back to 0.
	map(0xc00000, 0xc00001).r(FUNC(stepstag_state::stepstag_sprite_status_status_r)).nopw();

	map(0xd00000, 0xd00001).nopr(); // watchdog
	map(0xf00000, 0xf00001).w(FUNC(stepstag_state::adv7176a_w));
	map(0xffff00, 0xffff01).r(FUNC(stepstag_state::unknown_read_0xffff00));
}


void stepstag_state::vjdash_map(address_map &map)
{
	stepstag_map(map);
}


/***************************************************************************


                                Input Ports


***************************************************************************/

/***************************************************************************
                            Tetris Plus 2 (World)
***************************************************************************/

static INPUT_PORTS_START( tetrisp2 )

	PORT_START("PLAYERS") /*$be0002.w*/
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM") /*$be0004.w*/
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2   )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_CUSTOM  ) /* ?*/
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_CUSTOM  ) /* ?*/
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") //$be0008.w
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Vs Mode Rounds" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Japanese ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPNAME( 0x1000, 0x1000, "F.B.I Logo" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Voice" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

INPUT_PORTS_END


/***************************************************************************
                            Tetris Plus 2 (Japan)

    The code for checking the "service mode" and "free play"
    DSWs is (deliberately?) bugged in this set

***************************************************************************/

static INPUT_PORTS_START( tetrisp2j )
	PORT_INCLUDE(tetrisp2)

	PORT_MODIFY("DSW")  // $be0008.w
	PORT_DIPNAME( 0x0040, 0x0040, "Unknown 1-7" ) PORT_DIPLOCATION("SW1:7") /* Free Play in "World" set */
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Unknown 1-8" ) PORT_DIPLOCATION("SW1:8") /* Test Mode in "World" set */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Japanese ) )
	PORT_DIPSETTING(      0x0000, "English (buggy, breaks game)" )
	PORT_DIPNAME( 0x1000, 0x1000, "Unknown 2-5" ) PORT_DIPLOCATION("SW2:5") /* F.B.I. Logo in "World" set */
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************
                            Nandemo Seal Iinkai
***************************************************************************/

static INPUT_PORTS_START( nndmseal )

	PORT_START("BUTTONS") // be0002.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 )  PORT_NAME( "OK" )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON4 )  PORT_NAME( "Cancel" )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 )  PORT_NAME( "-" )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON2 )  PORT_NAME( "+" )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINS") // be0004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE2 ) // (keep pressed during boot for test mode)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") // be0008.w
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Time?" )
	PORT_DIPSETTING(      0x0000, "35" )
	PORT_DIPSETTING(      0x0040, "45" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0f00, "0" )
	PORT_DIPSETTING(      0x0e00, "1" )
	PORT_DIPSETTING(      0x0d00, "2" )
	PORT_DIPSETTING(      0x0c00, "3" )
	PORT_DIPSETTING(      0x0b00, "4" )
	PORT_DIPSETTING(      0x0a00, "5" )
	PORT_DIPSETTING(      0x0900, "6" )
	PORT_DIPSETTING(      0x0800, "7" )
	PORT_DIPSETTING(      0x0700, "8" )
	PORT_DIPSETTING(      0x0600, "9" )
	PORT_DIPSETTING(      0x0500, "a" )
	PORT_DIPSETTING(      0x0400, "b" )
	PORT_DIPSETTING(      0x0300, "c" )
	PORT_DIPSETTING(      0x0200, "d" )
	PORT_DIPSETTING(      0x0100, "e" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )

	PORT_START("PRINT") // be0006.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Print 1?") // Press both to print (and alternate with ok too).
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Print 2?") // Hold them for some seconds to bring up a "caution" message.
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH,IPT_CUSTOM )  // ?
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END


/***************************************************************************
                            Rock'n Tread (Japan)
***************************************************************************/


static INPUT_PORTS_START( rockn )
	PORT_START("PLAYERS")   //$be0002.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    //$be0004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")   //$be0008.w
	PORT_DIPNAME( 0x0001, 0x0001, "DIPSW 1-1") // All these used to be marked 'Cheat', can't think why.
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "DIPSW 1-2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "DIPSW 1-3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "DIPSW 1-4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "DIPSW 1-5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "DIPSW 1-6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "DIPSW 1-7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "DIPSW 1-8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME(0x0100, 0x0100, "DIPSW 2-1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(0x0200, 0x0200, "DIPSW 2-2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0400, 0x0400, "DIPSW 2-3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0800, 0x0800, "DIPSW 2-4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x1000, 0x1000, "DIPSW 2-5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x2000, 0x2000, "DIPSW 2-6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x4000, 0x4000, "DIPSW 2-7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x8000, 0x8000, "DIPSW 2-8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( rocknms )
	PORT_START("PLAYERS")   // IN0 - $be0002.w
	PORT_BIT( 0x0003, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(rocknms_state::main2sub_status_r)) // MAIN -> SUB Communication
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_START("SYSTEM")    // IN1 - $be0004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")   //$be0008.w
	PORT_DIPNAME( 0x0001, 0x0001, "DIPSW 1-1") // All these used to be marked 'Cheat', can't think why.
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "DIPSW 1-2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "DIPSW 1-3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "DIPSW 1-4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "DIPSW 1-5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "DIPSW 1-6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "DIPSW 1-7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "DIPSW 1-8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME(0x0100, 0x0100, "DIPSW 2-1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(0x0200, 0x0200, "DIPSW 2-2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0400, 0x0400, "DIPSW 2-3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0800, 0x0800, "DIPSW 2-4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x1000, 0x1000, "DIPSW 2-5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x2000, 0x2000, "DIPSW 2-6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x4000, 0x4000, "DIPSW 2-7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x8000, 0x8000, "DIPSW 2-8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************
                              Stepping Stage
***************************************************************************/

static INPUT_PORTS_START( stepstag )
	PORT_START("BUTTONS") // $be0002.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START2 )                           // P2 start (middle)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)    // P2 left
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)    // P2 right
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )                           // P1 start (middle)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)    // P1 left
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)    // P1 right
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINS") // $be0004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN   )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN   )
	PORT_SERVICE( 0x0010, IP_ACTIVE_LOW )               // service mode
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_SERVICE1 )    // service coin
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_COIN1    )    // coin
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_CUSTOM  )    // ?
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_CUSTOM  )    // ?
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN  )

	PORT_START("RHYTHM") // $a10000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Top-Left")  PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Top-Right") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Mid-Left")  PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Mid-Right") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Btm-Left")  PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Btm-Right") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Top-Left")  PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Top-Right") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Mid-Left")  PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Mid-Right") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Btm-Left")  PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 Btm-Right") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") // $be0008.w
	PORT_DIPNAME( 0x0001, 0x0001, "DIPSW 1-1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "DIPSW 1-2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "DIPSW 1-3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "DIPSW 1-4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "DIPSW 1-5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "DIPSW 1-6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "DIPSW 1-7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "DIPSW 1-8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, "DIPSW 2-1: >>2, &40")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "DIPSW 2-2: >>4, &20")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "DIPSW 2-3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "DIPSW 2-4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "DIPSW 2-5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "DIPSW 2-6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "DIPSW 2-7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "DIPSW 2-8: ?")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SOUND_VR1")
	PORT_ADJUSTER( 64, "Sound Volume" ) PORT_MINMAX( 1, 64 )
INPUT_PORTS_END

static INPUT_PORTS_START( vjdash )
	PORT_START("BUTTONS") // $be0002.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)    // P2 up
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)    // P2 down
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)    // P2 left
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)    // P2 right
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1)    // P1 up
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1)    // P1 down
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1)    // P1 left
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)    // P1 right
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINS") // $be0004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_SERVICE( 0x0010, IP_ACTIVE_LOW )           // service mode
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 ) // service coin
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1    ) // coin
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_CUSTOM  ) // ?
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_CUSTOM ) // ?
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("RHYTHM") // $a10000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Top-Left")      PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Top-Right")     PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Mid-Left")      PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Mid-Right")     PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Btm-Left")      PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P2 Btm-Right")     PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START2  )      // P2 start
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 SOUND KEY 1")   PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 SOUND KEY 2")   PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 SOUND KEY 3")   PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 MONITOR KEY 1") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 MONITOR KEY 2") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("P1 MONITOR KEY 3") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1  )      // P1 start
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") // $be0008.w
	PORT_DIPNAME( 0x001f, 0x001f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3,4,5")
	PORT_DIPSETTING(      0x001f, "P1 2 coins, P2 4 coins, 1 coin continue" )
	PORT_DIPSETTING(      0x001e, "P1/P2 1 coin, 1 coin continue" )
	PORT_DIPSETTING(      0x001d, "P1 1 coins, P2 2 coins, 1 coin continue" )
	PORT_DIPSETTING(      0x001c, "P1 2 coins, P2 3 coins, 1 coin continue" )
	PORT_DIPSETTING(      0x001b, "P1 2 coins, P2 4 coins, 1 coin continue" )
	PORT_DIPSETTING(      0x001a, "P1/P2 2 coins, 1 coin continue" )
	PORT_DIPSETTING(      0x0019, "P1/P2 2 coins, 2 coins continue" )
	PORT_DIPSETTING(      0x0018, "P1 2 coins, P2 3 coins, 2 coins continue" )
	PORT_DIPSETTING(      0x0017, "P1/P2 3 coins, 1 coin continue" )
	PORT_DIPSETTING(      0x0016, "P1/P2 3 coins, 2 coins continue" )
	PORT_DIPSETTING(      0x0015, "P1/P2 3 coins, 3 coins continue" )
	PORT_DIPSETTING(      0x0014, "P1 3 coins, P2 4 coins, 1 coin continue" )
	PORT_DIPSETTING(      0x0013, "P1 3 coins, P2 4 coins, 2 coins continue" )
	PORT_DIPSETTING(      0x0012, "P1 3 coins, P2 4 coins, 3 coins continue" )
	PORT_DIPSETTING(      0x0011, "P1 3 coins, P2 6 coins, 1 coin continue" )
	PORT_DIPSETTING(      0x0010, "P1 3 coins, P2 6 coins, 2 coins continue" )
	PORT_DIPSETTING(      0x000f, "P1 3 coins, P2 6 coins, 3 coins continue" )
	PORT_DIPSETTING(      0x000e, "P1 4 coins, P2 8 coins, 2 coins continue" )
	PORT_DIPSETTING(      0x000d, "P1 4 coins, P2 8 coins, 3 coins continue" )
	PORT_DIPSETTING(      0x000c, "P1 4 coins, P2 8 coins, 4 coins continue" )
	PORT_DIPSETTING(      0x000b, "P1 5 coins, P2 10 coins, 3 coins continue" )
	PORT_DIPSETTING(      0x000a, "P1 5 coins, P2 10 coins, 4 coins continue" )
	PORT_DIPSETTING(      0x0009, "P1 5 coins, P2 10 coins, 5 coins continue" )
	PORT_DIPSETTING(      0x0008, "P1 6 coins, P2 12 coins, 4 coins continue" )
	PORT_DIPSETTING(      0x0007, "P1 6 coins, P2 12 coins, 5 coins continue" )
	PORT_DIPSETTING(      0x0006, "P1 6 coins, P2 12 coins, 6 coins continue" )
	PORT_DIPSETTING(      0x0005, "P1 7 coins, P2 14 coins, 5 coins continue" )
	PORT_DIPSETTING(      0x0004, "P1 7 coins, P2 14 coins, 6 coins continue" )
	PORT_DIPSETTING(      0x0003, "P1 7 coins, P2 14 coins, 7 coins continue" )
	PORT_DIPSETTING(      0x0002, "P1 8 coins, P2 16 coins, 6 coins continue" )
	PORT_DIPSETTING(      0x0001, "P1 8 coins, P2 16 coins, 7 coins continue" )
	PORT_DIPSETTING(      0x0000, "P1 8 coins, P2 16 coins, 8 coins continue" )
	PORT_DIPNAME( 0x0020, 0x0020, "DIPSW 1-6") PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "DIPSW 1-7") PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0700, 0x0100, "Volume Level") PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0600, "0" )
	PORT_DIPSETTING(      0x0500, "1" )
	PORT_DIPSETTING(      0x0700, "2" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPSETTING(      0x0300, "4" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPSETTING(      0x0100, "6" )
	PORT_DIPSETTING(      0x0000, "7" )
	PORT_DIPNAME( 0x0800, 0x0800, "DIPSW 2-4") PORT_DIPLOCATION("SW2:4") // Unused?
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "DIPSW 2-5") PORT_DIPLOCATION("SW2:5") // Unused?
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x6000, 0x4000, "Volume") PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(      0x6000, "MAX" ) // 0
	PORT_DIPSETTING(      0x4000, "MID" ) // 64
	PORT_DIPSETTING(      0x2000, "MIN" ) // 128
	PORT_DIPSETTING(      0x0000, "OFF" ) // 256
	PORT_DIPNAME( 0x8000, 0x8000, "DIPSW 2-8") PORT_DIPLOCATION("SW2:8") // Unused?
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SOUND_VR1")
	PORT_ADJUSTER( 64, "Sound VR.1" ) PORT_MINMAX( 1, 64 )

	PORT_START("SOUND_VR2")
	PORT_ADJUSTER( 64, "Sound VR.2 (Woofer)" ) PORT_MINMAX( 1, 64 )
INPUT_PORTS_END


/***************************************************************************


                            Graphics Layouts


***************************************************************************/

static GFXDECODE_START( gfx_tetrisp2 )
	GFXDECODE_ENTRY( "tiles_bg", 0, gfx_16x16x8_raw, 0x1000, 0x10 ) // [1] Background
	GFXDECODE_ENTRY( "tiles_rot", 0, gfx_16x16x8_raw, 0x2000, 0x10 ) // [2] Rotation
	GFXDECODE_ENTRY( "tiles_fg", 0, gfx_8x8x8_raw,   0x6000, 0x10 ) // [3] Foreground
GFXDECODE_END

static GFXDECODE_START( gfx_rocknms_sub )
	GFXDECODE_ENTRY( "sub_tiles_bg", 0, gfx_16x16x8_raw, 0x1000, 0x10 ) // [1] Background
	GFXDECODE_ENTRY( "sub_tiles_rot", 0, gfx_16x16x8_raw, 0x2000, 0x10 ) // [2] Rotation
	GFXDECODE_ENTRY( "sub_tiles_fg", 0, gfx_8x8x8_raw,   0x6000, 0x10 ) // [3] Foreground
GFXDECODE_END

/***************************************************************************


                                Machine Drivers


***************************************************************************/

void nndmseal_state::machine_start()
{
	tetrisp2_state::machine_start();

	m_okibank[0]->configure_entries(0, 4, memregion("oki")->base(), 0x80000);
	m_okibank[1]->configure_entries(0, 16, memregion("oki")->base(), 0x20000);
	m_okibank[0]->set_entry(0);
	m_okibank[1]->set_entry(0);

	m_bank_hi = 0;
	m_bank_lo = 0;

	save_item(NAME(m_bank_lo));
	save_item(NAME(m_bank_hi));
}

void rockn_state::machine_start()
{
	tetrisp2_state::machine_start();

	save_item(NAME(m_rockn_protectdata));
	save_item(NAME(m_rockn_adpcmbank));
	save_item(NAME(m_rockn_soundvolume));
}

void rocknms_state::machine_start()
{
	rockn_state::machine_start();

	save_item(NAME(m_main2sub));
	save_item(NAME(m_sub2main));
}

void rockn_state::init_rockn()
{
	m_ymzbank[0]->configure_entries(0, 8, memregion("ymz")->base() + 0x400000, 0xc00000);
	m_ymzbank[0]->set_entry(0);
	m_rockn_protectdata = 1;
}

void rockn_state::init_rockn2()
{
	m_ymzbank[0]->configure_entries(0, 21, memregion("ymz")->base() + 0x400000, 0x400000);
	m_ymzbank[1]->configure_entries(0, 21, memregion("ymz")->base() + 0x400000, 0x400000);
	m_ymzbank[2]->configure_entries(0, 21, memregion("ymz")->base() + 0x400000, 0x400000);
	m_ymzbank[0]->set_entry(0);
	m_ymzbank[1]->set_entry(1);
	m_ymzbank[2]->set_entry(2);
	m_rockn_protectdata = 2;
}

void rocknms_state::init_rocknms()
{
	m_ymzbank[0]->configure_entries(0, 8, memregion("ymz")->base() + 0x400000, 0xc00000);
	m_ymzbank[0]->set_entry(0);
	m_rockn_protectdata = 3;
}

void rockn_state::init_rockn3()
{
	m_ymzbank[0]->configure_entries(0, 21, memregion("ymz")->base() + 0x400000, 0x400000);
	m_ymzbank[1]->configure_entries(0, 21, memregion("ymz")->base() + 0x400000, 0x400000);
	m_ymzbank[2]->configure_entries(0, 21, memregion("ymz")->base() + 0x400000, 0x400000);
	m_ymzbank[0]->set_entry(0);
	m_ymzbank[1]->set_entry(1);
	m_ymzbank[2]->set_entry(2);
	m_rockn_protectdata = 4;
}

void tetrisp2_state::field_irq_w(int state)
{
	// irq1 is valid on all games but tetrisp2, but always masked by SR?
	m_maincpu->set_input_line(1, (state) ? ASSERT_LINE : CLEAR_LINE);
}

void tetrisp2_state::vblank_irq_w(int state)
{
	m_maincpu->set_input_line(2, (state) ? ASSERT_LINE : CLEAR_LINE);
}

void tetrisp2_state::timer_irq_w(int state)
{
	m_maincpu->set_input_line(4, (state) ? ASSERT_LINE : CLEAR_LINE);
}

void tetrisp2_state::sound_reset_line_w(int state)
{
	logerror("%s: sound_reset_line_w %d but no CPU to reset?\n", machine().describe_context(), state);
}

void tetrisp2_state::setup_main_sysctrl(machine_config &config, const XTAL clock)
{
	JALECO_MS32_SYSCTRL(config, m_sysctrl, clock, m_screen);
	m_sysctrl->flip_screen_cb().set(FUNC(tetrisp2_state::flipscreen_w));
	m_sysctrl->vblank_cb().set(FUNC(tetrisp2_state::vblank_irq_w));
	m_sysctrl->field_cb().set(FUNC(tetrisp2_state::field_irq_w));
	m_sysctrl->prg_timer_cb().set(FUNC(tetrisp2_state::timer_irq_w));
	m_sysctrl->sound_reset_cb().set(FUNC(tetrisp2_state::sound_reset_line_w));
}

void tetrisp2_state::setup_main_sprite(machine_config &config, const XTAL clock)
{
	JALECO_MEGASYSTEM32_SPRITE(config, m_sprite, clock);
	m_sprite->set_palette(m_palette);
	m_sprite->set_color_base(0);
	m_sprite->set_color_entries(16);
	m_sprite->set_zoom(false);
}

void tetrisp2_state::tetrisp2(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12_MHz_XTAL); // 12MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &tetrisp2_state::tetrisp2_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 8);    /* guess */

	/* video hardware */
	constexpr XTAL pixel_clock = XTAL(48'000'000)/8;
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(pixel_clock, 384, 0, 320, 263, 0, 224); // default CRTC setup
	m_screen->set_screen_update(FUNC(tetrisp2_state::screen_update_tetrisp2));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tetrisp2);
	PALETTE(config, m_palette).set_entries(0x8000);

	setup_main_sprite(config, pixel_clock);
	setup_main_sysctrl(config, XTAL(48'000'000));

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	ymz280b_device &ymz(YMZ280B(config, "ymz", XTAL(16'934'400))); // 16.9344MHz
	ymz.add_route(0, "speaker", 1.0, 0);
	ymz.add_route(1, "speaker", 1.0, 1);
}


void nndmseal_state::nndmseal(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(12'000'000)); // 12MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &nndmseal_state::nndmseal_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	// An odd one: it uses the faster dot clock divider setting
	// but they replaced the xtal to a OSC1(42.9545MHz), I guess they compensated to not go out of ~60 Hz
	constexpr XTAL pixel_clock = XTAL(42'954'545)/6;
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(pixel_clock, 455, 0, 384, 262, 0, 240);
	m_screen->set_screen_update(FUNC(nndmseal_state::screen_update_tetrisp2));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tetrisp2);
	PALETTE(config, m_palette).set_entries(0x8000);

	setup_main_sprite(config, pixel_clock);
	setup_main_sysctrl(config, XTAL(42'954'545));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(2'000'000), okim6295_device::PIN7_HIGH));
	oki.set_addrmap(0, &nndmseal_state::nndmseal_oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 1.0); // 2MHz
}


void rockn_state::rockn(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(12'000'000)); // 12MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &rockn_state::rockn1_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	constexpr XTAL pixel_clock = XTAL(48'000'000)/8;
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(pixel_clock, 384, 0, 320, 263, 0, 224);
	m_screen->set_screen_update(FUNC(rockn_state::screen_update_rockntread));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tetrisp2);
	PALETTE(config, m_palette).set_entries(0x8000);

	setup_main_sprite(config, pixel_clock);
	setup_main_sysctrl(config, XTAL(48'000'000));

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	ymz280b_device &ymz(YMZ280B(config, "ymz", XTAL(16'934'400))); // 16.9344MHz
	ymz.set_addrmap(0, &rockn_state::rockn1_ymz_map);
	ymz.add_route(0, "speaker", 1.0, 0);
	ymz.add_route(1, "speaker", 1.0, 1);
}


void rockn_state::rockn2(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(12'000'000)); // 12MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &rockn_state::rockn2_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	constexpr XTAL pixel_clock = XTAL(48'000'000)/8;
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(pixel_clock, 384, 0, 320, 263, 0, 224);
	m_screen->set_screen_update(FUNC(rockn_state::screen_update_rockntread));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tetrisp2);
	PALETTE(config, m_palette).set_entries(0x8000);

	setup_main_sprite(config, pixel_clock);
	setup_main_sysctrl(config, XTAL(48'000'000));

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	ymz280b_device &ymz(YMZ280B(config, "ymz", XTAL(16'934'400))); // 16.9344MHz
	ymz.set_addrmap(0, &rockn_state::rockn2_ymz_map);
	ymz.add_route(0, "speaker", 1.0, 0);
	ymz.add_route(1, "speaker", 1.0, 1);
}

void rocknms_state::sub_field_irq_w(int state)
{
	m_subcpu->set_input_line(1, (state) ? ASSERT_LINE : CLEAR_LINE);
}

void rocknms_state::sub_vblank_irq_w(int state)
{
	m_subcpu->set_input_line(2, (state) ? ASSERT_LINE : CLEAR_LINE);
}

void rocknms_state::sub_timer_irq_w(int state)
{
	m_subcpu->set_input_line(4, (state) ? ASSERT_LINE : CLEAR_LINE);
}

void rocknms_state::sub_sound_reset_line_w(int state)
{
	logerror("%s: sound_reset_line_w %d on sub CPU but no CPU to reset?\n", machine().describe_context(), state);
}

void rocknms_state::rocknms(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(12'000'000)); // 12MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &rocknms_state::rocknms_main_map);

	M68000(config, m_subcpu, XTAL(12'000'000)); // 12MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &rocknms_state::rocknms_sub_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */

	config.set_default_layout(layout_rocknms);

	constexpr XTAL pixel_clock = XTAL(48'000'000)/8;
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_orientation(ROT0);
	m_screen->set_raw(pixel_clock, 384, 0, 320, 263, 0, 224);
	m_screen->set_screen_update(FUNC(rocknms_state::screen_update_top));

	SCREEN(config, m_sub_screen, SCREEN_TYPE_RASTER);
	m_sub_screen->set_orientation(ROT270);
	m_sub_screen->set_raw(pixel_clock, 384, 0, 320, 263, 0, 224);
	m_sub_screen->set_screen_update(FUNC(rocknms_state::screen_update_bottom));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tetrisp2);
	PALETTE(config, m_palette).set_entries(0x8000);

	GFXDECODE(config, m_sub_gfxdecode, m_sub_palette, gfx_rocknms_sub);
	PALETTE(config, m_sub_palette).set_entries(0x8000);

	setup_main_sprite(config, pixel_clock);

	JALECO_MEGASYSTEM32_SPRITE(config, m_sub_sprite, pixel_clock); // 48MHz for video?
	m_sub_sprite->set_palette(m_sub_palette);
	m_sub_sprite->set_color_base(0);
	m_sub_sprite->set_color_entries(16);
	m_sub_sprite->set_zoom(false);

	setup_main_sysctrl(config, XTAL(48'000'000));

	JALECO_MS32_SYSCTRL(config, m_sub_sysctrl, XTAL(48'000'000), m_sub_screen);
	m_sub_sysctrl->flip_screen_cb().set(FUNC(rocknms_state::sub_flipscreen_w));
	m_sub_sysctrl->vblank_cb().set(FUNC(rocknms_state::sub_vblank_irq_w));
	m_sub_sysctrl->field_cb().set(FUNC(rocknms_state::sub_field_irq_w));
	m_sub_sysctrl->prg_timer_cb().set(FUNC(rocknms_state::sub_timer_irq_w));
	m_sub_sysctrl->sound_reset_cb().set(FUNC(rocknms_state::sub_sound_reset_line_w));

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	ymz280b_device &ymz(YMZ280B(config, "ymz", XTAL(16'934'400))); // 16.9344MHz
	ymz.set_addrmap(0, &rocknms_state::rockn1_ymz_map);
	ymz.add_route(0, "speaker", 1.0, 0);
	ymz.add_route(1, "speaker", 1.0, 1);
}

void stepstag_state::field_cb(int state)
{
	// TODO: pinpoint the exact source, translate to configure_scanline if necessary
	// irq 4 is definitely a 30 Hz-ish here as well,
	// except we have a multi-screen arrangement setup and no way to pinpoint source
	// Based on PCB pics there appears to be a trace coming out of BLANK on the video encoder
	// chip closest to the CPU (and only that chip?) so I suspect that it's tied to that,
	// but it's not possible to 100% confirm it from the pics alone.
	if ((m_rscreen->frame_number() & 1) == 0)
		m_subcpu->set_input_line(4, (state) ? ASSERT_LINE : CLEAR_LINE);
}

void stepstag_state::setup_non_sysctrl_screen(machine_config &config, screen_device *screen, const XTAL xtal)
{
	// Values based on parameters set by video decoder chip initialization registers
	// VJ is interlaced (confirmed on real hardware)
	// Stepping Stage is non-interlaced (based on registers)
	screen->set_raw(xtal, 858, 0, 352, 525, 0, 240);
}

void stepstag_state::stepstag(machine_config &config)
{
	// 3 screens come from RGB headers off subboard

	M68000(config, m_maincpu, XTAL(12'000'000)); // unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &stepstag_state::stepstag_map);

	constexpr XTAL mainxtal = XTAL(48'000'000); // on main PCB, OSC1
	constexpr XTAL subxtal = XTAL(54'000'000); // on sub PCB, OSC1
	constexpr XTAL sub_pixel_clock = subxtal/2;

	M68000(config, m_subcpu, subxtal/3);
	m_subcpu->set_addrmap(AS_PROGRAM, &stepstag_state::stepstag_sub_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	lscreen.set_orientation(ROT270);
	setup_non_sysctrl_screen(config, &lscreen, sub_pixel_clock);
	lscreen.set_screen_update(FUNC(stepstag_state::screen_update_stepstag_left));

	screen_device &mscreen(SCREEN(config, "mscreen", SCREEN_TYPE_RASTER));
	mscreen.set_orientation(ROT0);
	setup_non_sysctrl_screen(config, &mscreen, sub_pixel_clock);
	mscreen.set_screen_update(FUNC(stepstag_state::screen_update_stepstag_mid));

	SCREEN(config, m_rscreen, SCREEN_TYPE_RASTER);
	m_rscreen->set_orientation(ROT270);
	setup_non_sysctrl_screen(config, m_rscreen, sub_pixel_clock);
	m_rscreen->set_screen_update(FUNC(stepstag_state::screen_update_stepstag_right));
	m_rscreen->screen_vblank().set(FUNC(stepstag_state::field_cb));

	MCFG_VIDEO_START_OVERRIDE(stepstag_state, stepstag)
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tetrisp2);
	PALETTE(config, m_palette).set_entries(0x8000);

	PALETTE(config, m_vj_palette_l).set_entries(0x8000);
	PALETTE(config, m_vj_palette_m).set_entries(0x8000);
	PALETTE(config, m_vj_palette_r).set_entries(0x8000);

	JALECO_MEGASYSTEM32_SPRITE(config, m_sprite, mainxtal/8); // unknown
	m_sprite->set_palette(m_palette);
	m_sprite->set_color_base(0);
	m_sprite->set_color_entries(16);
	m_sprite->set_zoom(false);

	// (left screen, vertical in stepping stage)
	JALECO_MEGASYSTEM32_SPRITE(config, m_vj_sprite_l, sub_pixel_clock); // unknown
	m_vj_sprite_l->set_palette(m_vj_palette_l);
	m_vj_sprite_l->set_color_base(0);
	m_vj_sprite_l->set_color_entries(0x80);
	m_vj_sprite_l->set_zoom(false);
	m_vj_sprite_l->set_yuv(true);

	// (mid screen, horizontal)
	JALECO_MEGASYSTEM32_SPRITE(config, m_vj_sprite_m, sub_pixel_clock); // unknown
	m_vj_sprite_m->set_palette(m_vj_palette_m);
	m_vj_sprite_m->set_color_base(0);
	m_vj_sprite_m->set_color_entries(0x80);
	m_vj_sprite_m->set_zoom(false);
	m_vj_sprite_m->set_yuv(true);

	// (right screen, vertical in stepping stage)
	JALECO_MEGASYSTEM32_SPRITE(config, m_vj_sprite_r, sub_pixel_clock); // unknown
	m_vj_sprite_r->set_palette(m_vj_palette_r);
	m_vj_sprite_r->set_color_base(0);
	m_vj_sprite_r->set_color_entries(0x80);
	m_vj_sprite_r->set_zoom(false);
	m_vj_sprite_r->set_yuv(true);

	// All video for Stepping Stage comes from subboard's 3 RGB headers so this screen isn't needed
	// but jaleco_ms32_sysctrl is built such that it requires a screen to work.
	// TODO: Refactor jaleco_ms32_sysctrl so it doesn't need a dummy screen
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(mainxtal/8, 384, 0, 320, 263, 0, 224);
	m_screen->set_screen_update(FUNC(stepstag_state::screen_update_nop));
	setup_main_sysctrl(config, mainxtal);

	config.set_default_layout(layout_stepstag);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_16(config, m_soundlatch);

	JALECO_VJ_PC(config, m_jaleco_vj_pc, 0);
	m_jaleco_vj_pc->set_steppingstage_mode(true);
	m_jaleco_vj_pc->add_route(0, "speaker", 1.0, 0);
	m_jaleco_vj_pc->add_route(1, "speaker", 1.0, 1);
}

void stepstag_state::vjdash(machine_config &config)    // 4 Screens
{
	// Bottom screen comes directly off JAMMA harness
	// 3 top screens come from RGB headers off subboard

	M68000(config, m_maincpu, XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &stepstag_state::vjdash_map);

	constexpr XTAL mainxtal = XTAL(48'000'000); // on main PCB, OSC1
	constexpr XTAL subxtal = XTAL(54'000'000); // on sub PCB, OSC1
	constexpr XTAL main_pixel_clock = mainxtal/8;
	constexpr XTAL sub_pixel_clock = subxtal/2;

	M68000(config, m_subcpu, subxtal/3); // divider unknown
	m_subcpu->set_addrmap(AS_PROGRAM, &stepstag_state::stepstag_sub_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(main_pixel_clock, 384, 0, 320, 263, 0, 224);
	m_screen->set_screen_update(FUNC(stepstag_state::screen_update_vjdash_main));
	m_screen->set_palette(m_palette);

	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	setup_non_sysctrl_screen(config, &lscreen, sub_pixel_clock);
	lscreen.set_screen_update(FUNC(stepstag_state::screen_update_vjdash_left));

	screen_device &mscreen(SCREEN(config, "mscreen", SCREEN_TYPE_RASTER));
	setup_non_sysctrl_screen(config, &mscreen, sub_pixel_clock);
	mscreen.set_screen_update(FUNC(stepstag_state::screen_update_vjdash_mid));

	SCREEN(config, m_rscreen, SCREEN_TYPE_RASTER);
	setup_non_sysctrl_screen(config, m_rscreen, sub_pixel_clock);
	m_rscreen->set_screen_update(FUNC(stepstag_state::screen_update_vjdash_right));
	m_rscreen->screen_vblank().set(FUNC(stepstag_state::field_cb));

	MCFG_VIDEO_START_OVERRIDE(stepstag_state, stepstag)
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tetrisp2);
	PALETTE(config, m_palette).set_entries(0x8000);

	PALETTE(config, m_vj_palette_l).set_entries(0x8000);
	PALETTE(config, m_vj_palette_m).set_entries(0x8000);
	PALETTE(config, m_vj_palette_r).set_entries(0x8000);

	// (left screen, horizontal)
	JALECO_MEGASYSTEM32_SPRITE(config, m_vj_sprite_l, sub_pixel_clock); // unknown
	m_vj_sprite_l->set_palette(m_vj_palette_l);
	m_vj_sprite_l->set_color_base(0);
	m_vj_sprite_l->set_color_entries(0x80);
	m_vj_sprite_l->set_zoom(false);
	m_vj_sprite_l->set_yuv(true);

	// (mid screen, horizontal)
	JALECO_MEGASYSTEM32_SPRITE(config, m_vj_sprite_m, sub_pixel_clock); // unknown
	m_vj_sprite_m->set_palette(m_vj_palette_m);
	m_vj_sprite_m->set_color_base(0);
	m_vj_sprite_m->set_color_entries(0x80);
	m_vj_sprite_m->set_zoom(false);
	m_vj_sprite_m->set_yuv(true);

	// (right screen, horizontal)
	JALECO_MEGASYSTEM32_SPRITE(config, m_vj_sprite_r, sub_pixel_clock); // unknown
	m_vj_sprite_r->set_palette(m_vj_palette_r);
	m_vj_sprite_r->set_color_base(0);
	m_vj_sprite_r->set_color_entries(0x80);
	m_vj_sprite_r->set_zoom(false);
	m_vj_sprite_r->set_yuv(true);

	setup_main_sprite(config, main_pixel_clock);
	setup_main_sysctrl(config, mainxtal); // unknown, controls game speed including sync of charts/keysounds to BGM

	config.set_default_layout(layout_vjdash);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_16(config, m_soundlatch);

	JALECO_VJ_PC(config, m_jaleco_vj_pc, 0);
	m_jaleco_vj_pc->set_steppingstage_mode(false);
	m_jaleco_vj_pc->add_route(0, "speaker", 1.0, 0);
	m_jaleco_vj_pc->add_route(1, "speaker", 1.0, 1);
}

void stepstag_state::machine_start()
{
	rockn_state::machine_start();

	m_spriteram1_data = std::make_unique<uint16_t[]>(0x400);
	m_spriteram2_data = std::make_unique<uint16_t[]>(0x400);
	m_spriteram3_data = std::make_unique<uint16_t[]>(0x400);

	m_adv7176a_shift = 0;
	m_adv7176a_byte = 0;
	m_adv7176a_sclock = 1;
	m_adv7176a_sdata = 1;
	m_adv7176a_state = 0;
	m_adv7176a_subaddr = 0;

	save_pointer(NAME(m_spriteram1_data), 0x400);
	save_pointer(NAME(m_spriteram2_data), 0x400);
	save_pointer(NAME(m_spriteram3_data), 0x400);
	save_item(NAME(m_adv7176a_shift));
	save_item(NAME(m_adv7176a_byte));
	save_item(NAME(m_adv7176a_sclock));
	save_item(NAME(m_adv7176a_sdata));
	save_item(NAME(m_adv7176a_state));
	save_item(NAME(m_adv7176a_subaddr));
}

void stepstag_state::machine_reset()
{
	tetrisp2_state::machine_reset();

	for (int i = 0; i < 0x400; i++)
		m_spriteram1_data[i] = m_spriteram2_data[i] = m_spriteram3_data[i] = 0;

	m_adv7176a_shift = 0;
	m_adv7176a_byte = 0;
	m_adv7176a_sclock = 1;
	m_adv7176a_sdata = 1;
	m_adv7176a_state = 0;
	m_adv7176a_subaddr = 0;
}


/***************************************************************************


                                ROMs Loading


***************************************************************************/


/***************************************************************************

                            Tetris Plus 2 (World & Japan sets)

(c)1997 Jaleco / The Tetris Company

TP-97222
96019 EB-00-20117-0
MDK332V-0

BRIEF HARDWARE OVERVIEW

CPU:    Toshiba TMP68HC000P-12
Sound:  Yamaha YMZ280B-F & Yamaha  YAC516-M
OSC:    12.000MHz
        48.0000MHz
        16.9344MHz
DIPS:   Two 8-way dipswitches

Listing of custom chips:

IC38    JALECO SS91022-03 9428XX001
IC31    JALECO SS91022-05 9347EX002
IC32    JALECO GS91022-05    048  9726HX002
IC30    JALECO GS91022-04 9721PD008
IC39    XILINX XC5210 PQ240C X68710M AKJ9544
IC49    XILINX XC7336 PC44ACK9633 A63458A

World set:
  Supports English or Japanese language
  Optional showing of US FBI Logo
  Full Test Mode via dipswitch
  EEPROM used

Japan Set:
  Japanese language only
  No optional FBI Logo
  No way to enter a Test Mode
  No support for EEPROM

***************************************************************************/

ROM_START( tetrisp2 ) /* Version 2.8 */
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "tet2_4_ver2.8.ic59", 0x000000, 0x080000, CRC(e67f9c51) SHA1(d8b2937699d648267b163c7c3f591426877f3701) )
	ROM_LOAD16_BYTE( "tet2_1_ver2.8.ic65", 0x000001, 0x080000, CRC(5020a4ed) SHA1(9c0f02fe3700761771ac026a2e375144e86e5eb7) )

	ROM_REGION( 0x800000, "sprite", 0 )   /* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "96019-01.9", 0x000000, 0x400000, CRC(06f7dc64) SHA1(722c51b707b9854c0293afdff18b27ec7cae6719) )
	ROM_LOAD32_WORD( "96019-02.8", 0x000002, 0x400000, CRC(3e613bed) SHA1(038b5e43fa3d69654107c8093126eeb2e8fa4ddc) )

	/* If t2p_m01&2 from this board were correctly read, since they hold the same data of the above but with swapped halves, it
	       means they had to invert the top bit of the "page select" register in the sprite's hardware on this board! */

	ROM_REGION( 0x800000, "tiles_bg", 0 )   /* 16x16x8 (Background) */
	ROM_LOAD( "96019-06.13", 0x000000, 0x400000, CRC(16f7093c) SHA1(2be77c6a692c5d762f5553ae24e8c415ab194cc6) )
	//ROM_LOAD( "96019-04.6",  0x400000, 0x100000, CRC(b849dec9) SHA1(fa7ac00fbe587a74c3fb8c74a0f91f7afeb8682f) )

	ROM_REGION( 0x100000, "tiles_rot", 0 )   /* 16x16x8 (Rotation) */
	ROM_COPY( "tiles_bg",        0x400000, 0x000000, 0x100000 )
	ROM_LOAD( "96019-04.6",  0x000000, 0x100000, CRC(b849dec9) SHA1(fa7ac00fbe587a74c3fb8c74a0f91f7afeb8682f) )

	ROM_REGION( 0x080000, "tiles_fg", 0 )   /* 8x8x8 (Foreground) */
	ROM_LOAD( "tetp2-10.ic27", 0x000000, 0x080000, CRC(34dd1bad) SHA1(9bdf1dde11f82839676400de5dd7acb06ea8cdb2) )   // 11111xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "96019-07.7", 0x000000, 0x400000, CRC(a8a61954) SHA1(86c3db10b348ba1f44ff696877b8b20845fa53de) )
ROM_END

ROM_START( tetrisp2a ) /* Version 2.7 */
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "tet2_4_ver2.7.ic59", 0x000000, 0x080000, CRC(3070bfde) SHA1(ba4f69961411fb7d64bcdf83763322e8ff097a59) )
	ROM_LOAD16_BYTE( "tet2_1_ver2.7.ic65", 0x000001, 0x080000, CRC(fe3eb1d2) SHA1(7ab27de254a0701a5ba8879456db794e871e6d69) )

	ROM_REGION( 0x800000, "sprite", 0 )   /* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "96019-01.9", 0x000000, 0x400000, CRC(06f7dc64) SHA1(722c51b707b9854c0293afdff18b27ec7cae6719) )
	ROM_LOAD32_WORD( "96019-02.8", 0x000002, 0x400000, CRC(3e613bed) SHA1(038b5e43fa3d69654107c8093126eeb2e8fa4ddc) )

	/* If t2p_m01&2 from this board were correctly read, since they hold the same data of the above but with swapped halves, it
	       means they had to invert the top bit of the "page select" register in the sprite's hardware on this board! */

	ROM_REGION( 0x800000, "tiles_bg", 0 )   /* 16x16x8 (Background) */
	ROM_LOAD( "96019-06.13", 0x000000, 0x400000, CRC(16f7093c) SHA1(2be77c6a692c5d762f5553ae24e8c415ab194cc6) )
	//ROM_LOAD( "96019-04.6",  0x400000, 0x100000, CRC(b849dec9) SHA1(fa7ac00fbe587a74c3fb8c74a0f91f7afeb8682f) )

	ROM_REGION( 0x100000, "tiles_rot", 0 )   /* 16x16x8 (Rotation) */
	ROM_COPY( "tiles_bg",        0x400000, 0x000000, 0x100000 )
	ROM_LOAD( "96019-04.6",  0x000000, 0x100000, CRC(b849dec9) SHA1(fa7ac00fbe587a74c3fb8c74a0f91f7afeb8682f) )

	ROM_REGION( 0x080000, "tiles_fg", 0 )   /* 8x8x8 (Foreground) */
	ROM_LOAD( "tetp2-10.ic27", 0x000000, 0x080000, CRC(34dd1bad) SHA1(9bdf1dde11f82839676400de5dd7acb06ea8cdb2) )   // 11111xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "96019-07.7", 0x000000, 0x400000, CRC(a8a61954) SHA1(86c3db10b348ba1f44ff696877b8b20845fa53de) )
ROM_END

ROM_START( tetrisp2j ) /* Version 2.2 */
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "tet2_4_ver2.2.ic59", 0x000000, 0x080000, CRC(5bfa32c8) SHA1(55fb2872695fcfbad13f5c0723302e72da69e44a) )
	ROM_LOAD16_BYTE( "tet2_1_ver2.2.ic65", 0x000001, 0x080000, CRC(919116d0) SHA1(3e1c0fd4c9175b2900a4717fbb9e8b591c5f534d) )

	ROM_REGION( 0x800000, "sprite", 0 )   /* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "96019-01.9", 0x000000, 0x400000, CRC(06f7dc64) SHA1(722c51b707b9854c0293afdff18b27ec7cae6719) )
	ROM_LOAD32_WORD( "96019-02.8", 0x000002, 0x400000, CRC(3e613bed) SHA1(038b5e43fa3d69654107c8093126eeb2e8fa4ddc) )

	ROM_REGION( 0x400000, "tiles_bg", 0 )   /* 16x16x8 (Background) */
	ROM_LOAD( "96019-06.13", 0x000000, 0x400000, CRC(16f7093c) SHA1(2be77c6a692c5d762f5553ae24e8c415ab194cc6) )

	ROM_REGION( 0x100000, "tiles_rot", 0 )   /* 16x16x8 (Rotation) */
	ROM_LOAD( "96019-04.6",  0x000000, 0x100000, CRC(b849dec9) SHA1(fa7ac00fbe587a74c3fb8c74a0f91f7afeb8682f) )

	ROM_REGION( 0x080000, "tiles_fg", 0 )   /* 8x8x8 (Foreground) */
	ROM_LOAD( "tetp2-10.ic27", 0x000000, 0x080000, CRC(34dd1bad) SHA1(9bdf1dde11f82839676400de5dd7acb06ea8cdb2) )   // 11111xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "96019-07.7", 0x000000, 0x400000, CRC(a8a61954) SHA1(86c3db10b348ba1f44ff696877b8b20845fa53de) )
ROM_END

ROM_START( tetrisp2ja ) /* Version 2.1 */
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	// yes, only one rom of the interleaved pair differs to the 2.2 revision?!
	ROM_LOAD16_BYTE( "tet2_ic4_ver2.1.ic59", 0x000000, 0x080000, CRC(5bfa32c8) SHA1(55fb2872695fcfbad13f5c0723302e72da69e44a) )
	ROM_LOAD16_BYTE( "tet2_ic1_ver2.1.ic65", 0x000001, 0x080000, CRC(5b5f8377) SHA1(75e17d628a1fd6da5616eea3e1e137f000824f14) )

	ROM_REGION( 0x800000, "sprite", 0 )   /* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "96019-01.9", 0x000000, 0x400000, CRC(06f7dc64) SHA1(722c51b707b9854c0293afdff18b27ec7cae6719) )
	ROM_LOAD32_WORD( "96019-02.8", 0x000002, 0x400000, CRC(3e613bed) SHA1(038b5e43fa3d69654107c8093126eeb2e8fa4ddc) )

	ROM_REGION( 0x400000, "tiles_bg", 0 )   /* 16x16x8 (Background) */
	ROM_LOAD( "96019-06.13", 0x000000, 0x400000, CRC(16f7093c) SHA1(2be77c6a692c5d762f5553ae24e8c415ab194cc6) )

	ROM_REGION( 0x100000, "tiles_rot", 0 )   /* 16x16x8 (Rotation) */
	ROM_LOAD( "96019-04.6",  0x000000, 0x100000, CRC(b849dec9) SHA1(fa7ac00fbe587a74c3fb8c74a0f91f7afeb8682f) )

	ROM_REGION( 0x080000, "tiles_fg", 0 )   /* 8x8x8 (Foreground) */
	ROM_LOAD( "tetp2-10.ic27", 0x000000, 0x080000, CRC(34dd1bad) SHA1(9bdf1dde11f82839676400de5dd7acb06ea8cdb2) )   // 11111xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "96019-07.7", 0x000000, 0x400000, CRC(a8a61954) SHA1(86c3db10b348ba1f44ff696877b8b20845fa53de) )
ROM_END

/***************************************************************************

Nandemo Seal Iinkai
(c)1996 I'max/Jaleco

Mainboard:
NS-96205 9601
EB-00-20110-0

Daughterboard:
NS-96206A
96017
EB-00-20111-1


CPU: TMP68000P-12

Sound: M6295 (on daughterboard)

OSC: 12.000MHz (X1)
     42.9545MHz(OSC1)
     2.000MHz(X1 on daughterboard. silk print says 4MHz)

Custom chips:
GS91022-04
GS91022-05
SS91022-03
SS91022-05

PLDs:
PS96017-01 (XILINX XC7336)
96017-02 (18CV8P)
96017-03 (18CV8P)
96017-04 (18CV8P on daughterboard)

Others:
CR2032 battery
Centronics printer port

ROMs:(all ROMs are on daughterboard)
1.1 - Programs (TMS 27C020)
3.3 /
(actual label is "Cawaii 1 Ver1.1" & "Cawaii 3 Ver1.1")

MR97006-01.2 (42pin mask ROM, read as 16M, byte mode)
MR97006-02.5 (same as above)
MR97001-01.6 (same as above)
MR96017-04.9 (same as above, samples)

MR97006-04.8 (32pin mask ROM, read as 8M)


dumped by sayu
--- Team Japump!!! ---

***************************************************************************/

ROM_START( nndmseal )
	ROM_REGION( 0x80000, "maincpu", 0 )     // 68000 Code
	ROM_LOAD16_BYTE( "cawaii 1 ver1.3.1", 0x00000, 0x40000, CRC(c48ea4d9) SHA1(11a3510d6db293c8b48805959e1b353dd9388d98) )    // 1xxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "cawaii 3 ver1.3.3", 0x00001, 0x40000, CRC(a8b85eb6) SHA1(753372f6301bf1f802707ec59e825253ee3c1e79) )    // 1xxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "sprite", ROMREGION_ERASE )    /* 8x8x8 (Sprites) */
/* This game doesn't use sprites, but the region needs to be a valid size for at least one sprite 'page' for the init to work. */

	ROM_REGION( 0x400000, "tiles_bg", 0 )   // 16x16x8 (Background)
	ROM_LOAD( "mr97006-02.5", 0x000000, 0x200000, CRC(4793f84e) SHA1(05acba6cc8a527a6050af79a460b08c4676287aa) )
	ROM_LOAD( "mr97001-01.6", 0x200000, 0x200000, CRC(dd648e8a) SHA1(7036ab30d0ea179c59d74c1fbe4372968722ec0f) )

	ROM_REGION( 0x200000, "tiles_rot", 0 )   // 16x16x8 (Rotation)
	ROM_LOAD( "mr97006-01.2", 0x000000, 0x200000, CRC(32283485) SHA1(14ccd25389b97825d9a727809c3a1de803687c16) )

	ROM_REGION( 0x100000, "tiles_fg", 0 )   // 8x8x8 (Foreground)
	ROM_LOAD( "mr97006-04.8", 0x000000, 0x100000, CRC(6726a25b) SHA1(4ea49c014477229eaf9de4a0b9bf83021b82c095) )

	ROM_REGION( 0x200000, "oki", 0 )  // Samples
	ROM_LOAD( "mr96017-04.9", 0x000000, 0x200000, CRC(c2e7b444) SHA1(e2b9d3d94720d01beff1108ef3dfbff805ddd1fd) )
ROM_END

ROM_START( nndmseal11 )
	ROM_REGION( 0x80000, "maincpu", 0 )     // 68000 Code
	ROM_LOAD16_BYTE( "cawaii 1 ver1.1.1", 0x00000, 0x40000, CRC(45acea25) SHA1(f2f2e78be261c3d8c0145a639bc3771f0588401d) )    // 1xxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "cawaii 3 ver1.1.3", 0x00001, 0x40000, CRC(0754d96a) SHA1(1da44994e8bcfd8832755e298c0125b38cfdd16e) )    // 1xxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "sprite", ROMREGION_ERASE )    /* 8x8x8 (Sprites) */
/* This game doesn't use sprites, but the region needs to be a valid size for at least one sprite 'page' for the init to work. */

	ROM_REGION( 0x400000, "tiles_bg", 0 )   // 16x16x8 (Background)
	ROM_LOAD( "mr97006-02.5", 0x000000, 0x200000, CRC(4793f84e) SHA1(05acba6cc8a527a6050af79a460b08c4676287aa) )
	ROM_LOAD( "mr97001-01.6", 0x200000, 0x200000, CRC(dd648e8a) SHA1(7036ab30d0ea179c59d74c1fbe4372968722ec0f) )

	ROM_REGION( 0x200000, "tiles_rot", 0 )   // 16x16x8 (Rotation)
	ROM_LOAD( "mr97006-01.2", 0x000000, 0x200000, CRC(32283485) SHA1(14ccd25389b97825d9a727809c3a1de803687c16) )

	ROM_REGION( 0x100000, "tiles_fg", 0 )   // 8x8x8 (Foreground)
	ROM_LOAD( "mr97006-04.8", 0x000000, 0x100000, CRC(6726a25b) SHA1(4ea49c014477229eaf9de4a0b9bf83021b82c095) )

	ROM_REGION( 0x200000, "oki", 0 )  // Samples
	ROM_LOAD( "mr96017-04.9", 0x000000, 0x200000, CRC(c2e7b444) SHA1(e2b9d3d94720d01beff1108ef3dfbff805ddd1fd) )
ROM_END

/***************************************************************************

Nandemo Seal Iinkai (Astro Boy ver.)
(c)1996 1997 I'max/Jaleco

Jaleco game similar to Nandemo Seal.
It sat at the back of an arcade repair workshop for about 15 years.
There's an external sound board. No ROMs on it but there's an NEC D78P014 MCU(?)
and an Altera FPGA and other sound related stuff, plus a Sony CXD1178Q (RGBDAC?)

The game boots here to a message screen and complains and won't go further.
Probably the external sound PCB is required to boot up.
It's missing the special cable so I can't connect it.

Mainboard:
NS-96205

Daughterboard:
NS-96206A

Sound: M6295 (on daughterboard)

ROMs:(all ROMs are on daughterboard)
1.1 - Programs (TMS 27C020)
3.3 /
(actual label is "Cawaii 1 Ver 1.0" & "Cawaii 3 Ver 1.0")
MR97006-01.2 (42pin mask ROM, read as 16M, byte mode)
IC6 is not populated.
IC2 and IC5 have a 0-ohm resistor wired to pin32 (A20 on a 32MBit ROM)
IC5 has it set so pin32 is tied to ground. So IC5 is 16MBit.
IC2 has it set so it goes to some logic on the main board (pin 12 of a 74LS244).
IC2 is dumped as 32M with both halves dumped in byte mode, 00's stripped and then interleaved.

***************************************************************************/

ROM_START( nndmseala )
	ROM_REGION( 0x80000, "maincpu", 0 )     // 68000 Code
	ROM_LOAD16_BYTE( "1.ic1", 0x00000, 0x40000, CRC(4eab8565) SHA1(07cdf00b60e19339188cbcd9d8e96a683b114f3e) )
	ROM_LOAD16_BYTE( "3.ic3", 0x00001, 0x40000, CRC(054ba50f) SHA1(11e3c5a6199955d6501ee72a5af62d17440fc306) )

	ROM_REGION( 0x100000, "sprite", ROMREGION_ERASE )    /* 8x8x8 (Sprites) */
/* This game doesn't use sprites, but the region needs to be a valid size for at least one sprite 'page' for the init to work. */

	ROM_REGION( 0x200000, "tiles_bg", 0 )   // 16x16x8 (Background)
	ROM_LOAD( "mr97032-02.ic5", 0x000000, 0x200000, CRC(460f16bd) SHA1(cdc4efa9897060d2ae3b21915dba68661e76ec03) )

	ROM_REGION( 0x400000, "tiles_rot", 0 )   // 16x16x8 (Rotation)
	ROM_LOAD( "mr97032-01.ic2", 0x000000, 0x400000, CRC(18c1a394) SHA1(491a2eb190efb5684f5eddb317adacd55afa727c) )

	ROM_REGION( 0x100000, "tiles_fg", 0 )   // 8x8x8 (Foreground)
	ROM_LOAD( "mr97032-03.ic8", 0x000000, 0x100000, CRC(5678a378) SHA1(306a3238590fa6e274e3c2ad334f5f210738dd7d) )

	ROM_REGION( 0x200000, "oki", 0 )  // Samples
	ROM_LOAD( "mr97016-04.ic9", 0x000000, 0x200000, CRC(f421232b) SHA1(d9cdc911566e795e6968d4b349c008b47132bea3) )
ROM_END

ROM_START( nndmsealb ) // NS-96205 96017 + NS-97211 97005
	ROM_REGION( 0x80000, "maincpu", 0 )     // 68000 Code
	ROM_LOAD16_BYTE( "1-ver1.1.ic1", 0x00000, 0x40000, CRC(b0845ef3) SHA1(a73918b400cb2e7c2e6869c385e5da8aacec6466) )
	ROM_LOAD16_BYTE( "3-ver1.1.ic3", 0x00001, 0x40000, CRC(405a51f6) SHA1(718accae0104089180fe6ceaf6f1fc3d92c275eb) )

	ROM_REGION( 0x100000, "sprite", ROMREGION_ERASE )    // 8x8x8 (Sprites)
// This game doesn't use sprites, but the region needs to be a valid size for at least one sprite 'page' for the init to work.

	ROM_REGION( 0x200000, "tiles_bg", 0 )   // 16x16x8 (Background)
	ROM_LOAD( "mr98009-02.ic6", 0x000000, 0x200000, CRC(94f1a8ba) SHA1(a9b627d27aac7c548977d0f799b1d7389801c8d8) )

	ROM_REGION( 0x400000, "tiles_rot", 0 )   // 16x16x8 (Rotation)
	ROM_LOAD( "mr98009-01.ic2", 0x000000, 0x400000, CRC(b3451bd0) SHA1(84c879b417c2042810f310ff366da7790afd1f81) )

	ROM_REGION( 0x100000, "tiles_fg", 0 )   // 8x8x8 (Foreground)
	ROM_LOAD( "mr98009-03.ic10", 0x000000, 0x100000, CRC(f0574f06) SHA1(42ff312dde90406b0aa355ff455765b12bbd6e6c) )

	ROM_REGION( 0x200000, "oki", 0 )  // Samples
	ROM_LOAD( "mr97016-04.ic12", 0x000000, 0x200000, CRC(f421232b) SHA1(d9cdc911566e795e6968d4b349c008b47132bea3) )
ROM_END

ROM_START( nndmsealc ) // NS-96205 96017 + NS-96206A 96017
	ROM_REGION( 0x80000, "maincpu", 0 )     // 68000 Code
	ROM_LOAD16_BYTE( "ver1.0.ic1", 0x00000, 0x40000, CRC(cd75ae3f) SHA1(654c79b91d863eabb11adc56a729628cc6e6b567) )
	ROM_LOAD16_BYTE( "ver1.0.ic3", 0x00001, 0x40000, CRC(42a0eb1c) SHA1(8c3a773233e051a8706444e5584aa9fa0aad7189) )

	ROM_REGION( 0x100000, "sprite", ROMREGION_ERASE )    // 8x8x8 (Sprites)
// This game doesn't use sprites, but the region needs to be a valid size for at least one sprite 'page' for the init to work.

	ROM_REGION( 0x400000, "tiles_bg", 0 )   // 16x16x8 (Background)
	ROM_LOAD( "mr98058-02.ic7", 0x000000, 0x400000, CRC(9d8401e3) SHA1(4b113f47c94b113d680c5420cdd5709e5b93b069) )

	ROM_REGION( 0x400000, "tiles_rot", 0 )   // 16x16x8 (Rotation)
	ROM_LOAD( "mr98058-01.ic2", 0x000000, 0x400000, CRC(a10848f0) SHA1(e86ed4fe85658046572903c6903e6ac9d885b85c) )

	ROM_REGION( 0x100000, "tiles_fg", 0 )   // 8x8x8 (Foreground)
	ROM_LOAD( "mr98058-03.ic10", 0x000000, 0x100000, CRC(4696609b) SHA1(6810483426b4c94e5d8b6fff4cbbbf3d953e26a7) )

	ROM_REGION( 0x200000, "oki", 0 )  // Samples
	ROM_LOAD( "mr97016-04.ic16", 0x000000, 0x200000, CRC(f421232b) SHA1(d9cdc911566e795e6968d4b349c008b47132bea3) )
ROM_END


/***************************************************************************

                            Rock'n Tread 1 (Japan)
                            Rock'n Tread 2 (Japan)
                            Rock'n MegaSession (Japan)
                            Rock'n 3 (Japan)
                            Rock'n 4 (Japan)

(c)1999-2000 Jaleco

Main board marked in copper and silk screen:

    JALECO VJ-98344
    98053 EB-00-20120-0
    MADE IN JAPAN

CPU:    TMP68HC000P-12
Sound:  YMZ280B-F / YAC516-M (on sound rom board)
OSC:    12.0000MHz (next to 68000)
        48.0000MHz
        16.9344MHz (on sound rom board)
DIPS:   Two 8-way dipswitches
BAT:    CR-2032

Custom: SS91022-03
        GS91022-04
        GS91022-05
        SS91022-05

Other:  Sigma XILINX XCS30
        Sigma XILINX XC9536 (socketed and stamped SL4)


PCB Layout for sound rom board (from Rock'n 3):

JALECO 99004 SL-99352 EB-00-20128-0
+------------------------------------------------------+
|                                                      |
| mr99029-02 mr99029-10 mr99029-18 mr99029-01  IC36*   |
| mr99029-03 mr99029-11 mr99029-19  IC29*      IC37*   |
| mr99029-04 mr99029-12 mr99029-20                     |
| mr99029-05 mr99029-13 mr99029-21                     |
| mr99029-06 mr99029-14  IC23*        YMZ280B-F        |
| mr99029-07 mr99029-15  IC24*      16.9344MHz         |
| mr99029-08 mr99029-16  IC25*        XC9572           |
| mr99029-09 mr99029-17  IC26*                         |
|                       YAC516-M                       |
|   CN1        CN2                       CN4           |
+-||||||||---||||||||----------------------------------+

Notes:
  All chip sockets are marked as 27C3200 (read as 27C322)
  Chips are OKI M72C3252C2 mounted on Jaleco PCB EB-00-40051-0 42 pin converters

* Unpopulated sockets

Sound chips: Yamaha YMZ280B-F & Yamaha YAC516-M
 Other chip: Sigma XILINX XC9572
        OSC: 16.9344MHz

  CN1 - 8 pin header
  CN2 - 8 pin header
  CN4 - 34 pin dual row ribbon connection

***************************************************************************/

/***  Rock 'n' Tread  ***/

ROM_START( rockn )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "rock_n_1_vj-98344_1.bin", 0x000001, 0x80000, CRC(4cf79e58) SHA1(f50e596d43c9ab2072ae0476169eee2a8512fd8d) )
	ROM_LOAD16_BYTE( "rock_n_1_vj-98344_4.bin", 0x000000, 0x80000, CRC(caa33f79) SHA1(8ccff67091dac5ad871cae6cdb31e1fc37c1a4c2) )

	ROM_REGION( 0x400000, "sprite", 0 )   /* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "rock_n_1_vj-98344_8.bin", 0x000002, 0x200000, CRC(fa3f6f9c) SHA1(586dcc690a1a4aa7c97932ad496382def6a074a4) )
	ROM_LOAD32_WORD( "rock_n_1_vj-98344_9.bin", 0x000000, 0x200000, CRC(3d12a688) SHA1(356b2ea81d960838b604c5a17cc77e79fb0e40ce) )

	ROM_REGION( 0x200000, "tiles_bg", 0 )   /* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "rock_n_1_vj-98344_13.bin", 0x000000, 0x200000, CRC(261b99a0) SHA1(7b3c768ae9d7429e2559fe32c1a4ff220d727e7e) )

	ROM_REGION( 0x100000, "tiles_rot", 0 )   /* 16x16x8 (Rotation) */
	ROM_LOAD( "rock_n_1_vj-98344_6.bin", 0x000000, 0x100000, CRC(5551717f) SHA1(64943a9a68ad4074f3f5128d7796e4f03baa14d5) )

	ROM_REGION( 0x080000, "tiles_fg", 0 )   /* 8x8x8 (Foreground) */
	ROM_LOAD( "rock_n_1_vj-98344_10.bin", 0x000000, 0x080000, CRC(918663a8) SHA1(aedacb741c986ef8159385cfef866cb7e3ef6cb6) )

	/* from the bootleg set, are they right for this? */
	ROM_REGION( 0x6400000, "ymz", ROMREGION_ERASEFF )   /* Samples */
	ROM_LOAD( "sound00", 0x0000000, 0x0400000, CRC(c354f753) SHA1(bf538c02e2162a93d8c6793a1211e21480156223)  ) // COMMON AREA
	ROM_LOAD( "sound01", 0x0400000, 0x0400000, CRC(5b42999e) SHA1(376c773f292eae8b75db11bad3cb6ec5fe48392e)  ) // bank 0
	ROM_LOAD( "sound02", 0x0800000, 0x0400000, CRC(8306f302) SHA1(8c0437d7ab8d74d4d15f4a641d30602e39cdd99d)  ) // bank 0
	ROM_LOAD( "sound03", 0x0c00000, 0x0400000, CRC(3fda842c) SHA1(2b9e7c548b689bab491237e36a2dcf4782a81d79)  ) // bank 0
	ROM_LOAD( "sound04", 0x1000000, 0x0400000, CRC(86d4f289) SHA1(908490ab0cf8d33cf3e127f71edee3bece70b86d)  ) // bank 1
	ROM_LOAD( "sound05", 0x1400000, 0x0400000, CRC(f8dbf47d) SHA1(f19f7ae26e3b8af17a4e66e6722dd2f5c36d33f8)  ) // bank 1
	ROM_LOAD( "sound06", 0x1800000, 0x0400000, CRC(525aff97) SHA1(b18e5bdf67d3a89f39c59f4f9bd3bb608dacc7f7)  ) // bank 1
	ROM_LOAD( "sound07", 0x1c00000, 0x0400000, CRC(5bd8bb95) SHA1(3b33c42778f7d50ca1513d37e7bc4a4efcc3cf82)  ) // bank 2
	ROM_LOAD( "sound08", 0x2000000, 0x0400000, CRC(304c1643) SHA1(0be090077e00d4b9abce2fac4821c630b6a40f22)  ) // bank 2
	ROM_LOAD( "sound09", 0x2400000, 0x0400000, CRC(78c22c56) SHA1(eb48d188d25538a1d381ca760f8e98096ee12bfe)  ) // bank 2
	ROM_LOAD( "sound10", 0x2800000, 0x0400000, CRC(d5e8d8a5) SHA1(df7db3c8b110ce1aa85e627537afb744c98877bd)  ) // bank 3
	ROM_LOAD( "sound11", 0x2c00000, 0x0400000, CRC(569ef4dd) SHA1(777f8a3aef741655555364d00a1eaa472ac4b922)  ) // bank 3
	ROM_LOAD( "sound12", 0x3000000, 0x0400000, CRC(aae8d59c) SHA1(ccca1f511ce0ea8d452f3b1d24350b5cee402ad2)  ) // bank 3
	ROM_LOAD( "sound13", 0x3400000, 0x0400000, CRC(9ec1459b) SHA1(10e08a47636dec431cdb8e105cf61287fe9c6637)  ) // bank 4
	ROM_LOAD( "sound14", 0x3800000, 0x0400000, CRC(b26f9a81) SHA1(0d1c8e382eb5877f9a748ff289be97cbdb73b0cc)  ) // bank 4
ROM_END

ROM_START( rockna )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "rock_n_1_vj-98344_1", 0x000001, 0x80000, CRC(6078fa48) SHA1(e98c1a1abf026f2d5b5035ccbc9d412a08ca1f02) )
	ROM_LOAD16_BYTE( "rock_n_1_vj-98344_4", 0x000000, 0x80000, CRC(c8310bd0) SHA1(1efee954cc94b668b7d9f28a099b8d1c83d3093f) )

	ROM_REGION( 0x400000, "sprite", 0 )   /* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "rock_n_1_vj-98344_8.bin", 0x000002, 0x200000, CRC(fa3f6f9c) SHA1(586dcc690a1a4aa7c97932ad496382def6a074a4) )
	ROM_LOAD32_WORD( "rock_n_1_vj-98344_9.bin", 0x000000, 0x200000, CRC(3d12a688) SHA1(356b2ea81d960838b604c5a17cc77e79fb0e40ce) )

	ROM_REGION( 0x200000, "tiles_bg", 0 )   /* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "rock_n_1_vj-98344_13.bin", 0x000000, 0x200000, CRC(261b99a0) SHA1(7b3c768ae9d7429e2559fe32c1a4ff220d727e7e) )

	ROM_REGION( 0x100000, "tiles_rot", 0 )   /* 16x16x8 (Rotation) */
	ROM_LOAD( "rock_n_1_vj-98344_6.bin", 0x000000, 0x100000, CRC(5551717f) SHA1(64943a9a68ad4074f3f5128d7796e4f03baa14d5) )

	ROM_REGION( 0x080000, "tiles_fg", 0 )   /* 8x8x8 (Foreground) */
	ROM_LOAD( "rock_n_1_vj-98344_10.bin", 0x000000, 0x080000, CRC(918663a8) SHA1(aedacb741c986ef8159385cfef866cb7e3ef6cb6) )

	ROM_REGION( 0x6400000, "ymz", ROMREGION_ERASEFF )   /* Samples */
	ROM_LOAD( "sound00", 0x0000000, 0x0400000, CRC(c354f753) SHA1(bf538c02e2162a93d8c6793a1211e21480156223)  ) // COMMON AREA
	ROM_LOAD( "sound01", 0x0400000, 0x0400000, CRC(5b42999e) SHA1(376c773f292eae8b75db11bad3cb6ec5fe48392e)  ) // bank 0
	ROM_LOAD( "sound02", 0x0800000, 0x0400000, CRC(8306f302) SHA1(8c0437d7ab8d74d4d15f4a641d30602e39cdd99d)  ) // bank 0
	ROM_LOAD( "sound03", 0x0c00000, 0x0400000, CRC(3fda842c) SHA1(2b9e7c548b689bab491237e36a2dcf4782a81d79)  ) // bank 0
	ROM_LOAD( "sound04", 0x1000000, 0x0400000, CRC(86d4f289) SHA1(908490ab0cf8d33cf3e127f71edee3bece70b86d)  ) // bank 1
	ROM_LOAD( "sound05", 0x1400000, 0x0400000, CRC(f8dbf47d) SHA1(f19f7ae26e3b8af17a4e66e6722dd2f5c36d33f8)  ) // bank 1
	ROM_LOAD( "sound06", 0x1800000, 0x0400000, CRC(525aff97) SHA1(b18e5bdf67d3a89f39c59f4f9bd3bb608dacc7f7)  ) // bank 1
	ROM_LOAD( "sound07", 0x1c00000, 0x0400000, CRC(5bd8bb95) SHA1(3b33c42778f7d50ca1513d37e7bc4a4efcc3cf82)  ) // bank 2
	ROM_LOAD( "sound08", 0x2000000, 0x0400000, CRC(304c1643) SHA1(0be090077e00d4b9abce2fac4821c630b6a40f22)  ) // bank 2
	ROM_LOAD( "sound09", 0x2400000, 0x0400000, CRC(78c22c56) SHA1(eb48d188d25538a1d381ca760f8e98096ee12bfe)  ) // bank 2
	ROM_LOAD( "sound10", 0x2800000, 0x0400000, CRC(d5e8d8a5) SHA1(df7db3c8b110ce1aa85e627537afb744c98877bd)  ) // bank 3
	ROM_LOAD( "sound11", 0x2c00000, 0x0400000, CRC(569ef4dd) SHA1(777f8a3aef741655555364d00a1eaa472ac4b922)  ) // bank 3
	ROM_LOAD( "sound12", 0x3000000, 0x0400000, CRC(aae8d59c) SHA1(ccca1f511ce0ea8d452f3b1d24350b5cee402ad2)  ) // bank 3
	ROM_LOAD( "sound13", 0x3400000, 0x0400000, CRC(9ec1459b) SHA1(10e08a47636dec431cdb8e105cf61287fe9c6637)  ) // bank 4
	ROM_LOAD( "sound14", 0x3800000, 0x0400000, CRC(b26f9a81) SHA1(0d1c8e382eb5877f9a748ff289be97cbdb73b0cc)  ) // bank 4
ROM_END

ROM_START( rockn2 )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "rock_n_2_vj-98344_1_v1.0", 0x000001, 0x80000, CRC(854b5a45) SHA1(91496bc511fef1d552d2bd00b82d2470eae94528) )
	ROM_LOAD16_BYTE( "rock_n_2_vj-98344_4_v1.0", 0x000000, 0x80000, CRC(4665bbd2) SHA1(3562c67b81a32d178a8bcb872e676bf7284855d7) )

	ROM_REGION( 0x400000, "sprite", 0 )   /* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "rock_n_2_vj-98344_8_v1.0", 0x000002, 0x200000, CRC(673ce2c2) SHA1(6c0a13de386b02a7f3a86e8128374938ede2525c) )
	ROM_LOAD32_WORD( "rock_n_2_vj-98344_9_v1.0", 0x000000, 0x200000, CRC(9d3968cf) SHA1(11c96e7685ab8c1b416396238ec5c12e7819385f) )

	ROM_REGION( 0x200000, "tiles_bg", 0 )   /* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "rock_n_2_vj-98344_13_v1.0", 0x000000, 0x200000, CRC(e35c55b3) SHA1(a18367c28befc3f71823f1d4ab2126ad6f8a28fc)  )

	ROM_REGION( 0x200000, "tiles_rot", 0 )   /* 16x16x8 (Rotation) */
	ROM_LOAD( "rock_n_2_vj-98344_6_v1.0", 0x000000, 0x200000, CRC(241d7449) SHA1(9fcc2d128d7be273836460313c0e73c81e33c9cb)  )

	ROM_REGION( 0x080000, "tiles_fg", 0 )   /* 8x8x8 (Foreground) */
	ROM_LOAD( "rock_n_2_vj-98344_10_v1.0", 0x000000, 0x080000, CRC(ae74d5b3) SHA1(07aa6ee540a783e3f2a8710a7095d922cff1d443)  )

	ROM_REGION( 0x6400000, "ymz", ROMREGION_ERASEFF )   /* Samples */
	ROM_LOAD( "sound00", 0x0000000, 0x0400000, CRC(4e9611a3) SHA1(2a9b1d5afc0ea9a3285f9fc6b49a1c3abd8cd2a5)  ) // COMMON AREA
	ROM_LOAD( "sound01", 0x0400000, 0x0400000, CRC(ec600f13) SHA1(151cb0a16782c8bba223d0f6881b80c1e43bc9bc)  ) // bank 0
	ROM_LOAD( "sound02", 0x0800000, 0x0400000, CRC(8306f302) SHA1(8c0437d7ab8d74d4d15f4a641d30602e39cdd99d)  ) // bank 0
	ROM_LOAD( "sound03", 0x0c00000, 0x0400000, CRC(3fda842c) SHA1(2b9e7c548b689bab491237e36a2dcf4782a81d79)  ) // bank 0
	ROM_LOAD( "sound04", 0x1000000, 0x0400000, CRC(86d4f289) SHA1(908490ab0cf8d33cf3e127f71edee3bece70b86d)  ) // bank 1
	ROM_LOAD( "sound05", 0x1400000, 0x0400000, CRC(f8dbf47d) SHA1(f19f7ae26e3b8af17a4e66e6722dd2f5c36d33f8)  ) // bank 1
	ROM_LOAD( "sound06", 0x1800000, 0x0400000, CRC(06f7bd63) SHA1(d8b27212ebba99f5129483550aeac5b86ff2a1d2)  ) // bank 1
	ROM_LOAD( "sound07", 0x1c00000, 0x0400000, CRC(22f042f6) SHA1(649bdf43dd698150992e68b23fd758bca56c615b)  ) // bank 2
	ROM_LOAD( "sound08", 0x2000000, 0x0400000, CRC(dd294d8e) SHA1(49d889d341ab6167d9741340eb27902923b6cb42)  ) // bank 2
	ROM_LOAD( "sound09", 0x2400000, 0x0400000, CRC(8fedee6e) SHA1(540c01d1c5f410abb1f86f33a5a532208946cb7c)  ) // bank 2
	ROM_LOAD( "sound10", 0x2800000, 0x0400000, CRC(01292f11) SHA1(da88b14bf8df34e7574cf8c9f5dd385db13ab34c)  ) // bank 3
	ROM_LOAD( "sound11", 0x2c00000, 0x0400000, CRC(20dc76ba) SHA1(078397f2de54d4ca91035dce11419ac0d934fbfa)  ) // bank 3
	ROM_LOAD( "sound12", 0x3000000, 0x0400000, CRC(11fff0bc) SHA1(2767fcc3a5d3200750b011c97a83073719a9325f)  ) // bank 3
	ROM_LOAD( "sound13", 0x3400000, 0x0400000, CRC(2367dd18) SHA1(b58f757ce4c832c5462637f4e08d7be511ca0c96)  ) // bank 4
	ROM_LOAD( "sound14", 0x3800000, 0x0400000, CRC(75ced8c0) SHA1(fda17464767be073a36c117f5212411b66197dd9)  ) // bank 4
	ROM_LOAD( "sound15", 0x3c00000, 0x0400000, CRC(aeaca380) SHA1(1c389911aa766abec389b1c79a1542759ac58b9f)  ) // bank 4
	ROM_LOAD( "sound16", 0x4000000, 0x0400000, CRC(21d50e32) SHA1(24eaceb7c0b868b6e8fc16b403dae2427e422bf6)  ) // bank 5
	ROM_LOAD( "sound17", 0x4400000, 0x0400000, CRC(de785a2a) SHA1(1f5ae46ac9476a31a431ce0f0cf124e1c8c930a6)  ) // bank 5
	ROM_LOAD( "sound18", 0x4800000, 0x0400000, CRC(18cabb1e) SHA1(c769820e2e84eff0e4ce956236656ae757e3299c)  ) // bank 5
	ROM_LOAD( "sound19", 0x4c00000, 0x0400000, CRC(33c89e53) SHA1(7d216f5db6b30c9b05a9a77030498ff68ae6fbad)  ) // bank 6
	ROM_LOAD( "sound20", 0x5000000, 0x0400000, CRC(89c1b088) SHA1(9b4118815959a5fb65b2a293015f592a46f4126f)  ) // bank 6
	ROM_LOAD( "sound21", 0x5400000, 0x0400000, CRC(13db74bd) SHA1(ab87438bbac97d46b1b8195b61dca1d72172a621)  ) // bank 6
//  ROM_LOAD( "sound22", 0x5800000, 0x0400000 ) // bank 7 ( ** unpopulated **  -  IC24)
//  ROM_LOAD( "sound23", 0x5c00000, 0x0400000 ) // bank 7 ( ** unpopulated **  -  IC25)
//  ROM_LOAD( "sound24", 0x6000000, 0x0400000 ) // bank 7 ( ** unpopulated **  -  IC26)
ROM_END

/***************************************************************************

Jaleco Rock'n 3

Labeled on the chips like:

    Rock'n 3
    VJ-98344
    10
    Ver 1.0


ROM ID       Label                    Rom type
----------------------------------------------
IC19    Rock'n 3 VJ-98344 10 Ver 1.0    27C040
IC59    Rock'n 3 VJ-98344 4  Ver 1.0    27C040
IC65    Rock'n 3 VJ-98344 1  Ver 1.0    27C040

IC10    Rock'n 3 VJ-98344 13 Ver 1.0    27C160
IC33    Rock'n 3 VJ-98344 9  Ver 1.0    27C160
IC32    Rock'n 3 VJ-98344 8  Ver 1.0    27C160
IC38    Rock'n 3 VJ-98344 6  Ver 1.0    27C160

IC57    PS96019-01              ICT 18CV8P PAL
IC14    PS96019-02              ICT 18CV8P PAL
IC58    PS96019-04              ICT 18CV8P PAL

IC43   (no label) XILINX 17S30PC  Serial Config rom

***************************************************************************/

ROM_START( rockn3 )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "rock_n_3_vj-98344_1_v1.0", 0x000001, 0x80000, CRC(abc6ab4a) SHA1(2f1983b95cd9e42d709edac5613b1f0b450df4ba) ) /* IC65 (alt PCB number 1) */
	ROM_LOAD16_BYTE( "rock_n_3_vj-98344_4_v1.0", 0x000000, 0x80000, CRC(3ecba46e) SHA1(64ff5b7932a8d8dc01c649b9dcc1d55cf1e43387) ) /* IC59 (alt PCB number 4) */

	ROM_REGION( 0x400000, "sprite", 0 )   /* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "rock_n_3_vj-98344_8_v1.0", 0x000002, 0x200000, CRC(468bf696) SHA1(d58e399ff876ab0f4ef52aaa85d86d72db307b6a) ) /* IC32 (alt PCB number 8) */
	ROM_LOAD32_WORD( "rock_n_3_vj-98344_9_v1.0", 0x000000, 0x200000, CRC(8a61fc18) SHA1(4e895a2014e711d044ed5d8bff8a91766f14b307) ) /* IC33 (alt PCB number 9) */

	ROM_REGION( 0x200000, "tiles_bg", 0 )   /* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "rock_n_3_vj-98344_13_v1.0", 0x000000, 0x200000, CRC(e01bf471) SHA1(4485c71770bdb8800ded4afb37814c2d287b78be)  ) /* IC10 (alt PCB number 13) */

	ROM_REGION( 0x200000, "tiles_rot", 0 )   /* 16x16x8 (Rotation) */
	ROM_LOAD( "rock_n_3_vj-98344_6_v1.0", 0x000000, 0x200000, CRC(4e146de5) SHA1(5971cbb91da5fde652786d82d0143197518bad9b)  ) /* IC38 (alt PCB number 6) */

	ROM_REGION( 0x080000, "tiles_fg", 0 )   /* 8x8x8 (Foreground) */
	ROM_LOAD( "rock_n_3_vj-98344_10_v1.0", 0x000000, 0x080000, CRC(8100039e) SHA1(e07b1e2f3cbcb1c086edd628d20423ecd4f74860)  ) /* IC19 (alt PCB number 10) */

	ROM_REGION( 0x6400000, "ymz", ROMREGION_ERASEFF )   /* Samples - On separate ROM board with YMZ280B-F sound chip */
	ROM_LOAD( "mr99029-01.ic28", 0x0000000, 0x0400000, CRC(e2f69042) SHA1(deb361a53ed6a9033e21c2f805f327cc3e9b11c6)  ) // COMMON AREA  (alt PCB number 25)
	// BANK AREA (unpopulated IC29, IC36 & IC37 (alt PCB numbers 26, 27 & 28 repectively)
	ROM_LOAD( "mr99029-02.ic1",  0x0400000, 0x0400000, CRC(b328b18f) SHA1(22edebcabd6c8ed65d8c9e501621991d404c430d)  ) // bank 0 (alt PCB number 1)
	ROM_LOAD( "mr99029-03.ic2",  0x0800000, 0x0400000, CRC(f46438e3) SHA1(718f54fc0e3689f5ab29bef2ec13eb2aa9b117fc)  ) // bank 0 (alt PCB number 2)
	ROM_LOAD( "mr99029-04.ic3",  0x0c00000, 0x0400000, CRC(b979e887) SHA1(10852ceb1b9e24fb87cf9339bc9fb4ae066a1221)  ) // bank 0 (alt PCB number 3)
	ROM_LOAD( "mr99029-05.ic4",  0x1000000, 0x0400000, CRC(0bb2c212) SHA1(4f8ab3c96c3e1aa337a3fe871cffc04ec603f8c0)  ) // bank 1 (alt PCB number 4)
	ROM_LOAD( "mr99029-06.ic5",  0x1400000, 0x0400000, CRC(3116e437) SHA1(f1b06592a6f0eba92eb4511d3ca03a3bb51e8c9d)  ) // bank 1 (alt PCB number 5)
	ROM_LOAD( "mr99029-07.ic6",  0x1800000, 0x0400000, CRC(26b37ef6) SHA1(f7090f3ec81f0c651c53d460b476e63f52dd06dc)  ) // bank 1 (alt PCB number 6)
	ROM_LOAD( "mr99029-08.ic7",  0x1c00000, 0x0400000, CRC(1dd3f4e3) SHA1(8474e00b962368164c717e5fe2e926852f3b4426)  ) // bank 2 (alt PCB number 7)
	ROM_LOAD( "mr99029-09.ic8",  0x2000000, 0x0400000, CRC(a1b03d67) SHA1(95f89a37e97d62706e15fd5571ff2e70dd98fee2)  ) // bank 2 (alt PCB number 8)
	ROM_LOAD( "mr99029-10.ic10", 0x2400000, 0x0400000, CRC(35107aac) SHA1(d56a66e15c46c33cf6c9c28edf48b730b681d21a)  ) // bank 2 (alt PCB number 9)
	ROM_LOAD( "mr99029-11.ic11", 0x2800000, 0x0400000, CRC(059ec592) SHA1(205210af558eb7e8e1399b2a506ef0285c5feda3)  ) // bank 3 (alt PCB number 10)
	ROM_LOAD( "mr99029-12.ic12", 0x2c00000, 0x0400000, CRC(84d4badb) SHA1(fc20f97a008f000a49e7cadd559789516643704a)  ) // bank 3 (alt PCB number 11)
	ROM_LOAD( "mr99029-13.ic13", 0x3000000, 0x0400000, CRC(4527a9b7) SHA1(a73ebece5c84bf14f8d25bbd869b7b43b1fcd042)  ) // bank 3 (alt PCB number 12)
	ROM_LOAD( "mr99029-14.ic14", 0x3400000, 0x0400000, CRC(bfa4b7ce) SHA1(4100f2deabb8994e8e3ff897a1db13693ab64c11)  ) // bank 4 (alt PCB number 13)
	ROM_LOAD( "mr99029-15.ic15", 0x3800000, 0x0400000, CRC(a2ccd2ce) SHA1(fc6325219f7b8e68c22a129f5ec4e900e326fb9d)  ) // bank 4 (alt PCB number 14)
	ROM_LOAD( "mr99029-16.ic16", 0x3c00000, 0x0400000, CRC(95baf678) SHA1(f7b39a3379f16df0560a22d4f42165ebbe05cebe)  ) // bank 4 (alt PCB number 15)
	ROM_LOAD( "mr99029-17.ic17", 0x4000000, 0x0400000, CRC(5883c84b) SHA1(54aec4e1e2f5edc198aebc4788caf5062f9a5b6c)  ) // bank 5 (alt PCB number 16)
	ROM_LOAD( "mr99029-18.ic19", 0x4400000, 0x0400000, CRC(f92098ce) SHA1(9b13cd37ad5d7baf36b20218c4bced956084ec45)  ) // bank 5 (alt PCB number 17)
	ROM_LOAD( "mr99029-19.ic20", 0x4800000, 0x0400000, CRC(dbb2c228) SHA1(f7cd24026236e2c616376c695b9e986cc221f36d)  ) // bank 5 (alt PCB number 18)
	ROM_LOAD( "mr99029-20.ic21", 0x4c00000, 0x0400000, CRC(9efdae1c) SHA1(6158a1804fbaa9ce27ae7e12cfda5f49084b4998)  ) // bank 6 (alt PCB number 19)
	ROM_LOAD( "mr99029-21.ic22", 0x5000000, 0x0400000, CRC(5f301b83) SHA1(e24e85c43a62871360545aa42dfa439045334b79)  ) // bank 6 (alt PCB number 20)
//  ROM_LOAD( "ic23",            0x5400000, 0x0400000 ) // bank 6 ( ** unpopulated **  -  alt PCB number 21)
//  ROM_LOAD( "ic24",            0x5800000, 0x0400000 ) // bank 7 ( ** unpopulated **  -  alt PCB number 22)
//  ROM_LOAD( "ic25",            0x5c00000, 0x0400000 ) // bank 7 ( ** unpopulated **  -  alt PCB number 23)
//  ROM_LOAD( "ic26",            0x6000000, 0x0400000 ) // bank 7 ( ** unpopulated **  -  alt PCB number 24)
ROM_END

ROM_START( rockn4 ) /* Prototype */
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "rock_n_4_vj-98344_1.bin", 0x000001, 0x80000, CRC(c666caea) SHA1(57018de40d71fe214a6b5cc33c8ad5e88622d010) )
	ROM_LOAD16_BYTE( "rock_n_4_vj-98344_4.bin", 0x000000, 0x80000, CRC(cc94e557) SHA1(d38abed04239d9eecf1b1be7a9f765a1b7aa0d8d) )

	ROM_REGION( 0x400000, "sprite", 0 )   /* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "rock_n_4_vj-98344_8.bin", 0x000002, 0x200000, CRC(5eeae537) SHA1(6bb8c658a2985c3919f0590a0147eead995c01c9) )
	ROM_LOAD32_WORD( "rock_n_4_vj-98344_9.bin", 0x000000, 0x200000, CRC(3fedddc9) SHA1(4bd8f402ecf8e6255326927e825179fa6d300e73) )

	ROM_REGION( 0x200000, "tiles_bg", 0 )   /* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "rock_n_4_vj-98344_13.bin", 0x000000, 0x200000, CRC(ead41e79) SHA1(9c24b1e52b6ed43d5b5a1caf48f2974b8fa61f4a)  )

	ROM_REGION( 0x200000, "tiles_rot", 0 )   /* 16x16x8 (Rotation) */
	ROM_LOAD( "rock_n_4_vj-98344_6.bin", 0x000000, 0x200000, CRC(eb16fc67) SHA1(5be40f2c9a5693785268eafcfcf348f147533463)  )

	ROM_REGION( 0x100000, "tiles_fg", 0 )   /* 8x8x8 (Foreground) */
	ROM_LOAD( "rock_n_4_vj-98344_10.bin", 0x000000, 0x100000, CRC(37d50259) SHA1(fd02f98a981470c47889f0b2f813ce59373a4b42)  )

	ROM_REGION( 0x6400000, "ymz", ROMREGION_ERASEFF )   /* Samples */
	ROM_LOAD( "sound00", 0x0000000, 0x0400000, CRC(918ea8eb) SHA1(0cd82859634635b6ce49db36fb91ed3365a101eb)  ) // COMMON AREA
	ROM_LOAD( "sound01", 0x0400000, 0x0400000, CRC(c548e51e) SHA1(4fe1e35c9ed4366dce98b4f4c00f94e202ef15dc)  ) // bank 0
	ROM_LOAD( "sound02", 0x0800000, 0x0400000, CRC(ffda0253) SHA1(9b8ae98accc2f72a1cd881086f89e647e4904ad9)  ) // bank 0
	ROM_LOAD( "sound03", 0x0c00000, 0x0400000, CRC(1f813af5) SHA1(a72d842e39b9fc955a2fc6721673b34b1b591e4a)  ) // bank 0
	ROM_LOAD( "sound04", 0x1000000, 0x0400000, CRC(035c4ff3) SHA1(9290c49244dc45ad5d6543775c5f2cc507e54e77)  ) // bank 1
	ROM_LOAD( "sound05", 0x1400000, 0x0400000, CRC(0f01f7b0) SHA1(e0c6daa1606dd5aaac59a7ae75d76e937e9c0151)  ) // bank 1
	ROM_LOAD( "sound06", 0x1800000, 0x0400000, CRC(31574b1c) SHA1(a08b50b4c4f2be32892b7534f2192101f8af6762)  ) // bank 1
	ROM_LOAD( "sound07", 0x1c00000, 0x0400000, CRC(388e2c91) SHA1(493ef760858a82cbc38de59c4db3f273c0ddfdfb)  ) // bank 2
	ROM_LOAD( "sound08", 0x2000000, 0x0400000, CRC(6e7e3f23) SHA1(4b9b959f79254d0633f1c4324b7ee6a17e222308)  ) // bank 2
	ROM_LOAD( "sound09", 0x2400000, 0x0400000, CRC(39fa512f) SHA1(d07426bc74492496756b67b8ded1b507726720c7)  ) // bank 2
ROM_END

ROM_START( rocknms )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "mast_prg1", 0x000001, 0x80000, CRC(c36674f8) SHA1(8aeb19fcd6f786c9d76a72abee4b607d29fb7d56) )
	ROM_LOAD16_BYTE( "mast_prg0", 0x000000, 0x80000, CRC(69382065) SHA1(2d528c2954556d440e790db209a2e3563580296a) )

	ROM_REGION( 0x100000, "sub", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "slav_prg1", 0x000001, 0x80000, CRC(769e2245) SHA1(5e6b5456fb213da887be4ef3739685360f6fdae5) )
	ROM_LOAD16_BYTE( "slav_prg0", 0x000000, 0x80000, CRC(55b8df65) SHA1(7744e7a75904174843fc6e3d54324839c6cf104d) )

	ROM_REGION( 0x0800000, "sprite", 0 )  /* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "mast_spr1", 0x000002, 0x400000, CRC(520152dc) SHA1(619a55352c0dab914f6188d66272a24495b5d1d4)  )
	ROM_LOAD32_WORD( "mast_spr0", 0x000000, 0x400000, CRC(1caad02a) SHA1(00c3fc849d1f633874fee30f7d0caf0c62735c50)  )

	ROM_REGION( 0x200000, "tiles_bg", 0 )   /* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "mast_back", 0x000000, 0x200000, CRC(1ca30e3f) SHA1(763c9dd287c186b6ca8ecb88c3ce29d68fea9179)  )

	ROM_REGION( 0x200000, "tiles_rot", 0 )   /* 16x16x8 (Rotation) */
	ROM_LOAD( "mast_rot", 0x000000, 0x200000, CRC(1f29b622) SHA1(aab6aafb98fa732266675daa63dc4c0d2084bcbd)  )

	ROM_REGION( 0x080000, "tiles_fg", 0 )   /* 8x8x8 (Foreground) */
	ROM_LOAD( "mast_front", 0x000000, 0x080000, CRC(a4717579) SHA1(cf28c0f19713ebf9f8fd5d55d654c1cd2e8cd73d)  )

	ROM_REGION( 0x800000, "sub_sprite", 0 )   /* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "slav_spr1", 0x000002, 0x400000, CRC(3f8124b0) SHA1(c9ab89f559551d2298d28e107b2d44d312e53216) )
	ROM_LOAD32_WORD( "slav_spr0", 0x000000, 0x400000, CRC(48a7f5b1) SHA1(4724856bde3cf975efc3be407b60693a69a39365) )

	ROM_REGION( 0x200000, "sub_tiles_bg", 0 )   /* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "slav_back", 0x000000, 0x200000, CRC(f0a28e32) SHA1(517b98dee6ec201bab02a3c81b0937ed462a626e) )

	ROM_REGION( 0x200000, "sub_tiles_rot", 0 )   /* 16x16x8 (Rotation) */
	ROM_LOAD( "slav_rot", 0x000000, 0x200000, CRC(0bab21f4) SHA1(afd3f32d7bb99b3f566b302fce11059ae8788715) )

	ROM_REGION( 0x080000, "sub_tiles_fg", 0 )   /* 8x8x8 (Foreground) */
	ROM_LOAD( "slav_front", 0x000000, 0x080000,  CRC(b65734a7) SHA1(80190e260ed32cb3355f0604722b85eb659483d0) )

	ROM_REGION( 0x6400000, "ymz", ROMREGION_ERASEFF )   /* Samples */
	ROM_LOAD( "sound00", 0x0000000, 0x0400000, CRC(8bafae71) SHA1(db74accd4bc1bfeb4a3341a0fd572b81287f1278)  ) // COMMON AREA
	ROM_LOAD( "sound01", 0x0400000, 0x0400000, CRC(eec0589b) SHA1(f54c1c7e7741100a1398ebd45aef4755171d9965)  ) // bank 0
	ROM_LOAD( "sound02", 0x0800000, 0x0400000, CRC(564aa972) SHA1(b19e960fd79647e5bcca509982c9887decb92bc6)  ) // bank 0
	ROM_LOAD( "sound03", 0x0c00000, 0x0400000, CRC(940302d0) SHA1(b28c2bb1a9b8cea0b6963ffa5d3ac26d90b0bffc)  ) // bank 0
	ROM_LOAD( "sound04", 0x1000000, 0x0400000, CRC(766db7f8) SHA1(41cfcac2e8d4307f75c56d57431b841e6d64b23c)  ) // bank 1
	ROM_LOAD( "sound05", 0x1400000, 0x0400000, CRC(3a3002f9) SHA1(27b24b8a34a0b919e051e81a10e87aa300b11d8f)  ) // bank 1
	ROM_LOAD( "sound06", 0x1800000, 0x0400000, CRC(06b04df9) SHA1(4bfc7c05843b4533f238f5360230cb71d7a66d56)  ) // bank 1
	ROM_LOAD( "sound07", 0x1c00000, 0x0400000, CRC(da74305e) SHA1(9dfb744f36ac8b3661006921dc482e941711f389)  ) // bank 2
	ROM_LOAD( "sound08", 0x2000000, 0x0400000, CRC(b5a0aa48) SHA1(2deb2c1c97c259f5e79e9dc3cd8859548549a189)  ) // bank 2
	ROM_LOAD( "sound09", 0x2400000, 0x0400000, CRC(0fd4a088) SHA1(5c1ea8a14dee7ee885ce0c86fb463741599db44d)  ) // bank 2
	ROM_LOAD( "sound10", 0x2800000, 0x0400000, CRC(33c89e53) SHA1(7d216f5db6b30c9b05a9a77030498ff68ae6fbad)  ) // bank 3
	ROM_LOAD( "sound11", 0x2c00000, 0x0400000, CRC(f9256a3f) SHA1(a3ec0845497d349c97222a1f986c252c8ca781e7)  ) // bank 3
	ROM_LOAD( "sound12", 0x3000000, 0x0400000, CRC(b0a09f3e) SHA1(d2e37eb935d7ef7e887ff79a49bc11da11c31f3c)  ) // bank 3
	ROM_LOAD( "sound13", 0x3400000, 0x0400000, CRC(d5cee673) SHA1(85194c73c43b69bccbcc895f147d5251bb039c2a)  ) // bank 4
	ROM_LOAD( "sound14", 0x3800000, 0x0400000, CRC(b394aa8a) SHA1(68541d5d98e2d59d6a3096f0c10b74b6f5803722)  ) // bank 4
	ROM_LOAD( "sound15", 0x3c00000, 0x0400000, CRC(6c791501) SHA1(8c67f070651493d6f7a2ef7b8a5f9e12c0181f67)  ) // bank 4
	ROM_LOAD( "sound16", 0x4000000, 0x0400000, CRC(fe80159e) SHA1(b6a980d4f62dfeaa6f51a99518aa4d483fe338e5)  ) // bank 5
	ROM_LOAD( "sound17", 0x4400000, 0x0400000, CRC(142c1159) SHA1(dfabbe69119c84040d6368561e93514ce7bb91db)  ) // bank 5
	ROM_LOAD( "sound18", 0x4800000, 0x0400000, CRC(cc595d85) SHA1(5f725771d79e71d62b64bb18e2a51b839a6e4c7f)  ) // bank 5
	ROM_LOAD( "sound19", 0x4c00000, 0x0400000, CRC(82b085a3) SHA1(5a5f2ed90d659bbad710c23b9df2a7dbb3c9acfe)  ) // bank 6
	ROM_LOAD( "sound20", 0x5000000, 0x0400000, CRC(dd5e9680) SHA1(5a2826641ad75757ce4a583e0ea901d54d20ffca)  ) // bank 6
ROM_END


/***************************************************************************

 VJ Visual & Music Slap

 vjdasha dump is incomplete, missing roms filled in using vjdash

    JALECO VJ-98342
    98053 EB-00-20122-0
    MADE IN JAPAN
    ---------------------
    TMP68HC000P-12
    3x Analog Devices ADV7176AKS video encoder
    6x 40-pin connectors for Qtaro device (2 cables per Qtaro board)
    3x 5-pin headers (R, G, B, black, white wires connected to each header)
    3x XILINX XCS30 PQ240CKN9825 A2016280A 3C
    3x XILINX 17S30PC One-Time Programmable Configuration PROM
    1x XILINX XCS05 VQ100CKN9845 A2015738A 3C
    1x XILINX 17S05PC One-Time Programmable Configuration PROM
    15x CY7C109-20VC 128k x 8 SRAM (5 for each XCS30)
    2x CY7C199-15PC 32k x 8 SRAM
    OSC1 54.0000 MHz

    34-pin IDC cable connects main board to sub board and then sub board to PC (via the ISA card), chained using same cable


    JALECO VJ-98346
    98053 EB-00-20123-0
    MADE IN JAPAN
    4CH AMP (x2)
    ---------------------
    ------------------------------------------
    |                    ?                   |
    |                                        |
    |    IC     IC     IC            FUSE    |
    |                                        |
    | P1 P2 P3 P4 CN1 CN2 CN3 CN4    CN5     |
    ------------------------------------------

    P1, P2, P3, P4 - Small screwing potentiometers
    CN1 - Audio signal input
    CN2, CN3 - Unpopulated, 3 pads in a triangle formation.
                One of the PCBs is completely unused but the other uses resistors to bridge the pads.
                TODO: Verify if resistors are a quirk for this specific machine or not.
    CN4 - Audio signal output
    CN5 - Power input
    ICs - SOIC8 (TODO: verify what chip this is)


    JALECO VJ-98348
    98053 EB-00-20126-0
    MADE IN JAPAN
    ---------------------
    ------------------------------------------------------------------
    |                                     23  24  25  26  27  28     |
    | CN3 15  16  17  18  19  20  21  22                         CN4 |
    |   IC16                                                         |
    |                                                      IC31      |
    |     1   2   3   4   5   6   7   8   9   10                     |
    | CN1                                         11  12  13  14 CN2 |
    |    IC1                                                         |
    ------------------------------------------------------------------

    IC1, IC16, IC31 - Texas Instruments SN74AS138N
    CN1, CN2, CN3, CN4 - 50-pin headers (connects to VJ-98342)
    1 - MR98053-05
    2 - MR98053-06
    3 - MR98053-07
    4 - MR98053-08
    5 - MR98053-09
    6 - MR98053-10
    7 - MR98053-11
    8 - MR98053-14
    9 - MR98053-16
    10 - MR98053-13
    11 - VJ 11 Ver1.1 sub program ROM, M27C4001-10F1
    12 - Unpopulated
    13 - Unpopulated
    14 - VJ 14 Ver1.1 sub program ROM, M27C4001-10F1
    15 - MR98053-05
    16 - MR98053-06
    17 - MR98053-07
    18 - MR98053-08
    19 - MR98053-09
    20 - MR98053-10
    21 - MR98053-C0
    22 - MR98053-C1
    23 - MR98053-10
    24 - MR98053-09
    25 - MR98053-08
    26 - MR98053-07
    27 - MR98053-06
    28 - MR98053-05

***************************************************************************/

ROM_START( vjslap )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "rom4.ic59", 0x00000, 0x80000, CRC(b6e16738) SHA1(53d12effd176b48b60c193530537b0b726c547b9) )
	ROM_LOAD16_BYTE( "rom1.ic65", 0x00001, 0x80000, CRC(1db8b380) SHA1(249c5ca0296258c9fbb82237995dabe51bd98a09) )

	ROM_REGION( 0x100000, "sub", 0 ) // 68000
	ROM_LOAD16_BYTE( "vj11_ver1.1.ic12", 0x00000, 0x80000, CRC(141e9969) SHA1(5148708312faa63669d3e86ece22ff14d0938455) )
	ROM_LOAD16_BYTE( "vj14_ver1.1.ic15", 0x00001, 0x80000, CRC(d32e862b) SHA1(1430008beb65f201937c22c9c4c9d811c89247cc) )

	ROM_REGION( 0x2000000, "sprite_l", 0 ) // left screen sprites
	ROM_LOAD( "mr98053-05.ic2", 0x000000, 0x400000, CRC(97da6668) SHA1(23b957184716776462eab235ce316e0f2a56f4bd) )
	ROM_LOAD( "mr98053-06.ic3", 0x400000, 0x400000, CRC(8ef6be1b) SHA1(836e907c0c00dcc74a9a62f3f5d9f25cf46bea60) )
	ROM_LOAD( "mr98053-07.ic4", 0x800000, 0x400000, CRC(801c7396) SHA1(51df041c982c5b8dcee7f593bb3be2a329b68399) )
	ROM_LOAD( "mr98053-08.ic5", 0xc00000, 0x400000, CRC(09ca77e3) SHA1(b56c82d516069612f5eb452faff1eb68665436b8) )
	ROM_LOAD( "mr98053-09.ic6", 0x1000000, 0x400000, CRC(80586e56) SHA1(7b60f87ccb9f2dd0b332d387b964706c93629536) )
	ROM_LOAD( "mr98053-10.ic7", 0x1400000, 0x400000, CRC(077e922f) SHA1(8baac5250618494eb030b7cd1f3515710eb1842c) )
	ROM_LOAD( "mr98053-11.ic8", 0x1800000, 0x400000, CRC(911b64ab) SHA1(2fb67d623402efa6ea23c9a945525a1cb5644eb9) )
	ROM_LOAD( "mr98053-14.ic9", 0x1c00000, 0x400000, CRC(a79228fc) SHA1(4e3993e73ce4f2400a6e571a7be874db124c273e) )

	ROM_REGION( 0x2000000, "sprite_m", 0 ) // middle screen sprites
	ROM_LOAD( "mr98053-05.ic2",  0x000000, 0x400000, CRC(97da6668) SHA1(23b957184716776462eab235ce316e0f2a56f4bd) )
	ROM_LOAD( "mr98053-06.ic3",  0x400000, 0x400000, CRC(8ef6be1b) SHA1(836e907c0c00dcc74a9a62f3f5d9f25cf46bea60) )
	ROM_LOAD( "mr98053-07.ic4",  0x800000, 0x400000, CRC(801c7396) SHA1(51df041c982c5b8dcee7f593bb3be2a329b68399) )
	ROM_LOAD( "mr98053-08.ic5",  0xc00000, 0x400000, CRC(09ca77e3) SHA1(b56c82d516069612f5eb452faff1eb68665436b8) )
	ROM_LOAD( "mr98053-09.ic6",  0x1000000, 0x400000, CRC(80586e56) SHA1(7b60f87ccb9f2dd0b332d387b964706c93629536) )
	ROM_LOAD( "mr98053-10.ic7",  0x1400000, 0x400000, CRC(077e922f) SHA1(8baac5250618494eb030b7cd1f3515710eb1842c) )
	ROM_LOAD( "mr98053-c0.ic23", 0x1800000, 0x400000, CRC(0d4148b3) SHA1(ac515c53ce91e24dd4dc46191281926a3bc9f74a) )
	ROM_LOAD( "mr98053-c1.ic24", 0x1c00000, 0x400000, CRC(510374ae) SHA1(ba48b69874dfde6329b8206f87b833bacbfdd7b5) )

	ROM_REGION( 0x2000000, "sprite_r", 0 ) // right screen sprites
	ROM_LOAD( "mr98053-05.ic2",  0x000000, 0x400000, CRC(97da6668) SHA1(23b957184716776462eab235ce316e0f2a56f4bd) )
	ROM_LOAD( "mr98053-06.ic3",  0x400000, 0x400000, CRC(8ef6be1b) SHA1(836e907c0c00dcc74a9a62f3f5d9f25cf46bea60) )
	ROM_LOAD( "mr98053-07.ic4",  0x800000, 0x400000, CRC(801c7396) SHA1(51df041c982c5b8dcee7f593bb3be2a329b68399) )
	ROM_LOAD( "mr98053-08.ic5",  0xc00000, 0x400000, CRC(09ca77e3) SHA1(b56c82d516069612f5eb452faff1eb68665436b8) )
	ROM_LOAD( "mr98053-09.ic6",  0x1000000, 0x400000, CRC(80586e56) SHA1(7b60f87ccb9f2dd0b332d387b964706c93629536) )
	ROM_LOAD( "mr98053-10.ic7",  0x1400000, 0x400000, CRC(077e922f) SHA1(8baac5250618494eb030b7cd1f3515710eb1842c) )
	ROM_LOAD( "mr98053-13.ic11", 0x1800000, 0x400000, CRC(a38af3a1) SHA1(ce7b2d7518f9de050293f3b9a073a1cedbc444fa) )
	ROM_LOAD( "mr98053-16.ic10", 0x1c00000, 0x400000, CRC(9c7f5964) SHA1(4e8d0a14c2459774204a8b24ea23a65520d3fc29) )

	ROM_REGION( 0x080000, "tiles_fg", 0 )   /* 8x8x8 (Foreground) */
	ROM_LOAD( "vj10_ver1.0.ic27", 0x000000, 0x080000, CRC(c143b7e4) SHA1(055699a18aa3529bb252dca391cf3f1e19f9ebe8) )

	ROM_REGION( 0x800000, "sprite", 0 )   /* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "obj-o.ic40", 0x000002, 0x400000, CRC(eaa927f1) SHA1(84742aecc1f9e40c289c87319255001cb701949f)  )
	ROM_LOAD32_WORD( "obj-e.ic41", 0x000000, 0x400000, CRC(a6c1e41b) SHA1(157af81a70604bee194c9b24f5b74774b3e7eff3)  )

	ROM_REGION( 0x400000, "tiles_bg", 0 )   /* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "bg.ic14", 0x000000, 0x200000, CRC(45f045ed) SHA1(196a41c71f3e579ff5c43ca75f5473a0597333b3) )

	ROM_REGION( 0x400000, "tiles_rot", 0 )   /* 16x16x8 (Rotation) */
	ROM_LOAD( "mr98053-04.ic36", 0x000000, 0x200000, CRC(4c69de30) SHA1(5f758498abb87f86f428193413c8e06bb4024725) )

	ROM_REGION( 0x001000, "gal", ROMREGION_ERASE )  // ICT GAL
	ROM_LOAD( "98053-09.ic58", 0x000000, 3553, CRC(10a443a6) SHA1(fa0950d2b089a34d4b6a039e4a9e8c458dd8e157) )

	ROM_REGION( 0x010000, "xilinx", ROMREGION_ERASE )  // XILINX CPLD
	ROM_LOAD( "15c.ic49", 0x000000, 38807, CRC(60d50907) SHA1(c5a837b3105ba15fcec103154c8c4d00924974e1) )

	DISK_REGION( "jaleco_vj_pc:pci:07.1:ide1:0:hdd" )
	DISK_IMAGE( "vj_ver1", 0, BAD_DUMP SHA1(bf5c70fba13186854ff0b7eafab07dd527aac663) ) // macOS infected the HDD with a ".Spotlight-V100" folder before it could be copied
ROM_END

ROM_START( vjdash )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "vjdash_pro4_ver1.0.ic59", 0x00000, 0x80000, CRC(136df9a6) SHA1(855cea28359256c8399501aa8c4dea63e0c48b5a) )
	ROM_LOAD16_BYTE( "vjdash_pro1_ver1.0.ic65", 0x00001, 0x80000, CRC(19ebd931) SHA1(3d9be64fcb73abfc33ee03801921db6e05b30485) )

	ROM_REGION( 0x100000, "sub", 0 ) // 68000
	ROM_LOAD16_BYTE( "vjdash_pro11_ver1.0.ic12", 0x00000, 0x80000, CRC(025e0396) SHA1(27e0bb792975a7375be9f58f80aa97f997066ec8) )
	ROM_LOAD16_BYTE( "vjdash_pro14_ver1.0.ic15", 0x00001, 0x80000, CRC(5a60cc88) SHA1(9413bdc58984c32b40f37538bdff3eed0b203cda) )

	ROM_REGION( 0x2000000, "sprite_l", 0 ) // left screen sprites
	ROM_LOAD( "mr98053-05.ic2", 0x000000, 0x400000, CRC(97da6668) SHA1(23b957184716776462eab235ce316e0f2a56f4bd) )
	ROM_LOAD( "mr98053-06.ic3", 0x400000, 0x400000, CRC(8ef6be1b) SHA1(836e907c0c00dcc74a9a62f3f5d9f25cf46bea60) )
	ROM_LOAD( "mr98053-07.ic4", 0x800000, 0x400000, CRC(801c7396) SHA1(51df041c982c5b8dcee7f593bb3be2a329b68399) )
	ROM_LOAD( "mr98053-08.ic5", 0xc00000, 0x400000, CRC(09ca77e3) SHA1(b56c82d516069612f5eb452faff1eb68665436b8) )
	ROM_LOAD( "mr98053-09.ic6", 0x1000000, 0x400000, CRC(80586e56) SHA1(7b60f87ccb9f2dd0b332d387b964706c93629536) )
	ROM_LOAD( "mr98053-10.ic7", 0x1400000, 0x400000, CRC(077e922f) SHA1(8baac5250618494eb030b7cd1f3515710eb1842c) )
	ROM_LOAD( "mr98053-11.ic8", 0x1800000, 0x400000, CRC(911b64ab) SHA1(2fb67d623402efa6ea23c9a945525a1cb5644eb9) )
	ROM_LOAD( "mr98053-14.ic9", 0x1c00000, 0x400000, CRC(a79228fc) SHA1(4e3993e73ce4f2400a6e571a7be874db124c273e) )

	ROM_REGION( 0x2000000, "sprite_m", 0 ) // middle screen sprites
	ROM_LOAD( "mr98053-05.ic2",  0x000000, 0x400000, CRC(97da6668) SHA1(23b957184716776462eab235ce316e0f2a56f4bd) )
	ROM_LOAD( "mr98053-06.ic3",  0x400000, 0x400000, CRC(8ef6be1b) SHA1(836e907c0c00dcc74a9a62f3f5d9f25cf46bea60) )
	ROM_LOAD( "mr98053-07.ic4",  0x800000, 0x400000, CRC(801c7396) SHA1(51df041c982c5b8dcee7f593bb3be2a329b68399) )
	ROM_LOAD( "mr98053-08.ic5",  0xc00000, 0x400000, CRC(09ca77e3) SHA1(b56c82d516069612f5eb452faff1eb68665436b8) )
	ROM_LOAD( "mr98053-09.ic6",  0x1000000, 0x400000, CRC(80586e56) SHA1(7b60f87ccb9f2dd0b332d387b964706c93629536) )
	ROM_LOAD( "mr98053-10.ic7",  0x1400000, 0x400000, CRC(077e922f) SHA1(8baac5250618494eb030b7cd1f3515710eb1842c) )
	ROM_LOAD( "mr98053-c0.ic23", 0x1800000, 0x400000, CRC(0d4148b3) SHA1(ac515c53ce91e24dd4dc46191281926a3bc9f74a) )
	ROM_LOAD( "mr98053-c1.ic24", 0x1c00000, 0x400000, CRC(510374ae) SHA1(ba48b69874dfde6329b8206f87b833bacbfdd7b5) )

	ROM_REGION( 0x2000000, "sprite_r", 0 ) // right screen sprites
	ROM_LOAD( "mr98053-05.ic2",  0x000000, 0x400000, CRC(97da6668) SHA1(23b957184716776462eab235ce316e0f2a56f4bd) )
	ROM_LOAD( "mr98053-06.ic3",  0x400000, 0x400000, CRC(8ef6be1b) SHA1(836e907c0c00dcc74a9a62f3f5d9f25cf46bea60) )
	ROM_LOAD( "mr98053-07.ic4",  0x800000, 0x400000, CRC(801c7396) SHA1(51df041c982c5b8dcee7f593bb3be2a329b68399) )
	ROM_LOAD( "mr98053-08.ic5",  0xc00000, 0x400000, CRC(09ca77e3) SHA1(b56c82d516069612f5eb452faff1eb68665436b8) )
	ROM_LOAD( "mr98053-09.ic6",  0x1000000, 0x400000, CRC(80586e56) SHA1(7b60f87ccb9f2dd0b332d387b964706c93629536) )
	ROM_LOAD( "mr98053-10.ic7",  0x1400000, 0x400000, CRC(077e922f) SHA1(8baac5250618494eb030b7cd1f3515710eb1842c) )
	ROM_LOAD( "mr98053-13.ic11", 0x1800000, 0x400000, CRC(a38af3a1) SHA1(ce7b2d7518f9de050293f3b9a073a1cedbc444fa) )
	ROM_LOAD( "mr98053-16.ic10", 0x1c00000, 0x400000, CRC(9c7f5964) SHA1(4e8d0a14c2459774204a8b24ea23a65520d3fc29) )

	ROM_REGION( 0x080000, "tiles_fg", 0 )   /* 8x8x8 (Foreground) */
	ROM_LOAD( "vjdash_ver1.0.ic27", 0x000000, 0x080000, CRC(f3cff858) SHA1(9277e5fb3494f7afb7f3911792d1c68b2b1b147e) )

	ROM_REGION( 0x800000, "sprite", 0 )   /* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "vjdash_8.ic40", 0x000002, 0x400000, CRC(798bc4a4) SHA1(5aca6495e48a1d72bf2993ff26f0462d78acc88b) )
	ROM_LOAD32_WORD( "vjdash_9.ic41", 0x000000, 0x400000, CRC(f2489f68) SHA1(d98f08dd0fa3448e266afaf6971aa5b39686d4f4) )

	ROM_REGION( 0x400000, "tiles_bg", 0 )   /* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "bg.ic14", 0x000000, 0x200000, CRC(45f045ed) SHA1(196a41c71f3e579ff5c43ca75f5473a0597333b3) )

	ROM_REGION( 0x400000, "tiles_rot", 0 )   /* 16x16x8 (Rotation) */
	ROM_LOAD( "mr98053-04.ic36", 0x000000, 0x200000, CRC(4c69de30) SHA1(5f758498abb87f86f428193413c8e06bb4024725) )

	ROM_REGION( 0x001000, "gal", 0 )  // ICT GAL
	ROM_LOAD( "98053-09.ic58", 0x000000, 3553, CRC(10a443a6) SHA1(fa0950d2b089a34d4b6a039e4a9e8c458dd8e157) )

	ROM_REGION( 0x010000, "xilinx", 0 )  // XILINX CPLD
	ROM_LOAD( "15c.ic49", 0x000000, 38807, CRC(60d50907) SHA1(c5a837b3105ba15fcec103154c8c4d00924974e1) )

	DISK_REGION( "jaleco_vj_pc:pci:07.1:ide1:0:hdd" )
	DISK_IMAGE( "vj_ver1", 0, BAD_DUMP SHA1(bf5c70fba13186854ff0b7eafab07dd527aac663) )
ROM_END

ROM_START( vjdasha )
	// This version has some spelling fixes and some data tables shifted around in the code but there's nothing to actually
	// differentiate the versions like a revision number in-game anywhere.
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "vjdash4_ver1.2.ic59", 0x00000, 0x80000, CRC(f7cf8d62) SHA1(8a1bf3a4eb431b71262d9dda47caa0ba0a0127f6) )
	ROM_LOAD16_BYTE( "vjdash1_ver1.2.ic65", 0x00001, 0x80000, CRC(6d01bef5) SHA1(1f27a82cd583451b32f14967d8db00448543f948) )

	ROM_REGION( 0x100000, "sub", 0 ) // 68000
	ROM_LOAD16_BYTE( "vjdash_pro11_ver1.0.ic12", 0x00000, 0x80000, CRC(025e0396) SHA1(27e0bb792975a7375be9f58f80aa97f997066ec8) BAD_DUMP ) // From vjdash set, may be different on a real Dash Ver 1.2 board
	ROM_LOAD16_BYTE( "vjdash_pro14_ver1.0.ic15", 0x00001, 0x80000, CRC(5a60cc88) SHA1(9413bdc58984c32b40f37538bdff3eed0b203cda) BAD_DUMP )

	ROM_REGION( 0x2000000, "sprite_l", 0 ) // left screen sprites
	ROM_LOAD( "mr98053-05.ic2", 0x000000, 0x400000, CRC(97da6668) SHA1(23b957184716776462eab235ce316e0f2a56f4bd) BAD_DUMP )
	ROM_LOAD( "mr98053-06.ic3", 0x400000, 0x400000, CRC(8ef6be1b) SHA1(836e907c0c00dcc74a9a62f3f5d9f25cf46bea60) BAD_DUMP )
	ROM_LOAD( "mr98053-07.ic4", 0x800000, 0x400000, CRC(801c7396) SHA1(51df041c982c5b8dcee7f593bb3be2a329b68399) BAD_DUMP )
	ROM_LOAD( "mr98053-08.ic5", 0xc00000, 0x400000, CRC(09ca77e3) SHA1(b56c82d516069612f5eb452faff1eb68665436b8) BAD_DUMP )
	ROM_LOAD( "mr98053-09.ic6", 0x1000000, 0x400000, CRC(80586e56) SHA1(7b60f87ccb9f2dd0b332d387b964706c93629536) BAD_DUMP )
	ROM_LOAD( "mr98053-10.ic7", 0x1400000, 0x400000, CRC(077e922f) SHA1(8baac5250618494eb030b7cd1f3515710eb1842c) BAD_DUMP )
	ROM_LOAD( "mr98053-11.ic8", 0x1800000, 0x400000, CRC(911b64ab) SHA1(2fb67d623402efa6ea23c9a945525a1cb5644eb9) BAD_DUMP )
	ROM_LOAD( "mr98053-14.ic9", 0x1c00000, 0x400000, CRC(a79228fc) SHA1(4e3993e73ce4f2400a6e571a7be874db124c273e) BAD_DUMP )

	ROM_REGION( 0x2000000, "sprite_m", 0 ) // middle screen sprites
	ROM_LOAD( "mr98053-05.ic2",  0x000000, 0x400000, CRC(97da6668) SHA1(23b957184716776462eab235ce316e0f2a56f4bd) BAD_DUMP )
	ROM_LOAD( "mr98053-06.ic3",  0x400000, 0x400000, CRC(8ef6be1b) SHA1(836e907c0c00dcc74a9a62f3f5d9f25cf46bea60) BAD_DUMP )
	ROM_LOAD( "mr98053-07.ic4",  0x800000, 0x400000, CRC(801c7396) SHA1(51df041c982c5b8dcee7f593bb3be2a329b68399) BAD_DUMP )
	ROM_LOAD( "mr98053-08.ic5",  0xc00000, 0x400000, CRC(09ca77e3) SHA1(b56c82d516069612f5eb452faff1eb68665436b8) BAD_DUMP )
	ROM_LOAD( "mr98053-09.ic6",  0x1000000, 0x400000, CRC(80586e56) SHA1(7b60f87ccb9f2dd0b332d387b964706c93629536) BAD_DUMP )
	ROM_LOAD( "mr98053-10.ic7",  0x1400000, 0x400000, CRC(077e922f) SHA1(8baac5250618494eb030b7cd1f3515710eb1842c) BAD_DUMP )
	ROM_LOAD( "mr98053-c0.ic23", 0x1800000, 0x400000, CRC(0d4148b3) SHA1(ac515c53ce91e24dd4dc46191281926a3bc9f74a) BAD_DUMP )
	ROM_LOAD( "mr98053-c1.ic24", 0x1c00000, 0x400000, CRC(510374ae) SHA1(ba48b69874dfde6329b8206f87b833bacbfdd7b5) BAD_DUMP )

	ROM_REGION( 0x2000000, "sprite_r", 0 ) // right screen sprites
	ROM_LOAD( "mr98053-05.ic2",  0x000000, 0x400000, CRC(97da6668) SHA1(23b957184716776462eab235ce316e0f2a56f4bd) BAD_DUMP )
	ROM_LOAD( "mr98053-06.ic3",  0x400000, 0x400000, CRC(8ef6be1b) SHA1(836e907c0c00dcc74a9a62f3f5d9f25cf46bea60) BAD_DUMP )
	ROM_LOAD( "mr98053-07.ic4",  0x800000, 0x400000, CRC(801c7396) SHA1(51df041c982c5b8dcee7f593bb3be2a329b68399) BAD_DUMP )
	ROM_LOAD( "mr98053-08.ic5",  0xc00000, 0x400000, CRC(09ca77e3) SHA1(b56c82d516069612f5eb452faff1eb68665436b8) BAD_DUMP )
	ROM_LOAD( "mr98053-09.ic6",  0x1000000, 0x400000, CRC(80586e56) SHA1(7b60f87ccb9f2dd0b332d387b964706c93629536) BAD_DUMP )
	ROM_LOAD( "mr98053-10.ic7",  0x1400000, 0x400000, CRC(077e922f) SHA1(8baac5250618494eb030b7cd1f3515710eb1842c) BAD_DUMP )
	ROM_LOAD( "mr98053-13.ic11", 0x1800000, 0x400000, CRC(a38af3a1) SHA1(ce7b2d7518f9de050293f3b9a073a1cedbc444fa) BAD_DUMP )
	ROM_LOAD( "mr98053-16.ic10", 0x1c00000, 0x400000, CRC(9c7f5964) SHA1(4e8d0a14c2459774204a8b24ea23a65520d3fc29) BAD_DUMP )

	ROM_REGION( 0x080000, "tiles_fg", 0 )   /* 8x8x8 (Foreground) */
	ROM_LOAD( "vjdash_ver1.0.ic27", 0x000000, 0x080000, CRC(f3cff858) SHA1(9277e5fb3494f7afb7f3911792d1c68b2b1b147e) )

	ROM_REGION( 0x800000, "sprite", 0 )   /* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "vjdash_8.ic40", 0x000002, 0x400000, CRC(798bc4a4) SHA1(5aca6495e48a1d72bf2993ff26f0462d78acc88b) BAD_DUMP ) // From vjdash set, may be different on a real Dash Ver 1.2 board
	ROM_LOAD32_WORD( "vjdash_9.ic41", 0x000000, 0x400000, CRC(f2489f68) SHA1(d98f08dd0fa3448e266afaf6971aa5b39686d4f4) BAD_DUMP )

	ROM_REGION( 0x400000, "tiles_bg", 0 )   /* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "mr98053-03.ic14", 0x000000, 0x200000, CRC(0bd32084) SHA1(2fcac3019ebedc54b83b08f527aa968ce6d48617) )

	ROM_REGION( 0x400000, "tiles_rot", 0 )   /* 16x16x8 (Rotation) */
	ROM_LOAD( "mr98053-04.ic36", 0x000000, 0x200000, CRC(4c69de30) SHA1(5f758498abb87f86f428193413c8e06bb4024725) )

	ROM_REGION( 0x001000, "gal", 0 )  // ICT GAL
	ROM_LOAD( "98053-09.ic58", 0x000000, 3553, CRC(10a443a6) SHA1(fa0950d2b089a34d4b6a039e4a9e8c458dd8e157) )

	ROM_REGION( 0x010000, "xilinx", 0 )  // XILINX CPLD
	ROM_LOAD( "15c.ic49", 0x000000, 38807, CRC(60d50907) SHA1(c5a837b3105ba15fcec103154c8c4d00924974e1) )

	DISK_REGION( "jaleco_vj_pc:pci:07.1:ide1:0:hdd" )
	DISK_IMAGE( "vj_ver1", 0, SHA1(bf5c70fba13186854ff0b7eafab07dd527aac663) BAD_DUMP )
ROM_END

/***************************************************************************

 Stepping Stage Special

 dump is incomplete, these are leftovers from an upgrade
***************************************************************************/

ROM_START( stepstag )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "vj98344ver11.4", 0x00000, 0x80000, CRC(391ca913) SHA1(2cc329aa6419f8a0d7e0fb8a9f4c2b8ca25197b3) )
	ROM_LOAD16_BYTE( "vj98344ver11.1", 0x00001, 0x80000, CRC(aedcb225) SHA1(f167c390e79ffbf7c019c326384ae656ae8b7d13) )

	ROM_REGION( 0x100000, "sub", 0 ) // 68000
	ROM_LOAD16_BYTE( "vj98348ver11.11", 0x00000, 0x80000, CRC(29b7f848) SHA1(c4d89e5c9be622b2d9038c359a5f65ce0dd461b0) )
	ROM_LOAD16_BYTE( "vj98348ver11.14", 0x00001, 0x80000, CRC(e3314c6c) SHA1(61b0e9f9d0126d9f475304866a03cfa21701d9aa) )

	ROM_REGION( 0x1000000, "sprite_l", 0 )  // left screen sprites (vertical)
	ROM_LOAD( "mr99001-01", 0x000000, 0x400000, CRC(aa92cebf) SHA1(2ccc0d2ef9bc92c27f0a625819154bbcf9cfde0c) )  // VERTICAL
	ROM_LOAD( "mr99001-02", 0x400000, 0x400000, CRC(12c65d86) SHA1(7fe5853fa3ba086f8da15702b126eb13c6ea30a9) )  // VERTICAL
	ROM_LOAD( "s.s.s._vj-98348_3_pr99021-01",  0x800000, 0x400000, CRC(e0fbc6f1) SHA1(7ca4507702f3f81bb9de3f9b5d270d379e439633) )  // VERTICAL

	ROM_REGION( 0x1800000, "sprite_m", 0 ) // middle screen sprites (horizontal)
	ROM_LOAD( "mr99001-03", 0x0000000, 0x400000, CRC(40fee0df) SHA1(94c3567e82f8039b3169bf4dcb1fcd9e39c6eb27) ) // HORIZONTAL TRUSTED
	ROM_LOAD( "mr99001-04", 0x0400000, 0x400000, CRC(d6837981) SHA1(56709d73304f0b186c70844ae96f73400b541609) ) // HORIZONTAL TRUSTED
	ROM_LOAD( "mr99001-05", 0x0800000, 0x400000, CRC(3958473b) SHA1(12279a587263290945744b22aafb80460eea77f7) ) // HORIZONTAL TRUSTED
	ROM_LOAD( "mr99001-06", 0x0c00000, 0x400000, CRC(cfa27c93) SHA1(a0837877736e8e898f3acc64bc87ee0cc4d9f243) ) // HORIZONTAL
	ROM_LOAD( "s.s.s._vj-98348_19_pr99021-02", 0x1000000, 0x400000, CRC(2d98da1a) SHA1(b09375fa1b4b2e0794632d6e237459009f40310d) )  // HORIZONTAL TRUSTED

	ROM_REGION( 0x1000000, "sprite_r", 0 )  // right screen sprites (vertical)
	ROM_LOAD( "mr99001-01", 0x000000, 0x400000, CRC(aa92cebf) SHA1(2ccc0d2ef9bc92c27f0a625819154bbcf9cfde0c) )  // VERTICAL
	ROM_LOAD( "mr99001-02", 0x400000, 0x400000, CRC(12c65d86) SHA1(7fe5853fa3ba086f8da15702b126eb13c6ea30a9) )  // VERTICAL
	// rom _26_ seems a bad dump of rom _3_, overwrite it:
	ROM_LOAD( "s.s.s._vj-98348_26_pr99021-01", 0x800000, 0x400000, BAD_DUMP CRC(fefb3777) SHA1(df624e105ab1dea52317e318ad29caa02b900788) )  // VERTICAL
	ROM_LOAD( "s.s.s._vj-98348_3_pr99021-01",  0x800000, 0x400000, CRC(e0fbc6f1) SHA1(7ca4507702f3f81bb9de3f9b5d270d379e439633) )           // VERTICAL

	ROM_REGION( 0x400000, "tiles_fg", 0 ) // foreground tiles
	ROM_LOAD( "vjdash_ver1.0.ic27", 0x000000, 0x080000, BAD_DUMP CRC(f3cff858) SHA1(9277e5fb3494f7afb7f3911792d1c68b2b1b147e) )

	ROM_REGION( 0x400000, "sprite", ROMREGION_ERASE )   /* 8x8x8 (Sprites) */

	ROM_REGION( 0x400000, "tiles_bg", ROMREGION_ERASE )   /* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "stepstag_scroll", 0x000000, 0x400000, NO_DUMP )

	ROM_REGION( 0x400000, "tiles_rot", ROMREGION_ERASE )   /* 16x16x8 (Rotation) */
	ROM_LOAD( "stepstag_rott", 0x000000, 0x400000, NO_DUMP )

	DISK_REGION( "jaleco_vj_pc:pci:07.1:ide1:0:hdd" )
	DISK_IMAGE( "stepstag", 0, NO_DUMP )
ROM_END

/***************************************************************************

 Stepping 3 Superior

 dump is incomplete, these are leftovers from an upgrade
***************************************************************************/

ROM_START( step3 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASE00 ) // 68000
	ROM_LOAD16_BYTE( "vj98344_step3.4", 0x00000, 0x80000, NO_DUMP )
	ROM_LOAD16_BYTE( "vj98344_step3.1", 0x00001, 0x80000, NO_DUMP )
	ROM_FILL( 6, 1, 0x01 )
	ROM_FILL( 0x100, 1, 0x60 )
	ROM_FILL( 0x101, 1, 0xfe )

	ROM_REGION( 0x100000, "sub", 0 ) // 68000
	ROM_LOAD16_BYTE( "vj98348_step3_11_v1.1", 0x00000, 0x80000, CRC(9c36aef5) SHA1(bbac48c2c7949a6f8a6ec83515e94a343c88d1b6) )
	ROM_LOAD16_BYTE( "vj98348_step3_14_v1.1", 0x00001, 0x80000, CRC(b86be557) SHA1(49dbd6ef1c50adcf3386d5423da8ae7685649c46) )

	ROM_REGION( 0x1800000, "sprite_l", 0 )  // left screen sprites (vertical)
	ROM_LOAD( "mr9930-01.ic2",         0x0000000, 0x400000, CRC(9e3e054e) SHA1(06a4fa76cb83dbe9d565d5ccd0a5ecc5067887c9) )  // sprites VERTICAL
	ROM_LOAD( "mr9930-02.ic3",         0x0400000, 0x400000, CRC(b23c29f4) SHA1(a7b10a3a9af43db319baf8633bb3728120960923) )  // 8x8 VERTICAL
	ROM_LOAD( "mr9930-03.ic4",         0x0800000, 0x400000, CRC(9a5d070f) SHA1(b4668b4f299033140a2c56499cc2712ba111cb57) )  // 8x8 VERTICAL
	ROM_LOAD( "vj98348_step3_4_v1.1",  0x0c00000, 0x400000, CRC(dec612df) SHA1(acb86bb90c1cc61c7db3e022c69a5ff0611ffbae) )  // 8x8 HORIZONTAL

	ROM_REGION( 0x1800000, "sprite_m", 0 )   // middle screen sprites (horizontal)
	ROM_LOAD( "mr99030-04.ic17", 0x000000, 0x400000, CRC(3eac3591) SHA1(3b294e94af23fd92fdf51d2c9c43f60d2ebd1688) )  // 8x8 HORIZONTAL
	ROM_LOAD( "mr99030-05.ic18", 0x400000, 0x400000, CRC(dea7b8d6) SHA1(d7d98675eb3998a8057929f90aa340c1e5f6a617) )  // 8x8 HORIZONTAL
	ROM_LOAD( "mr99030-06.ic19", 0x800000, 0x400000, CRC(71489d79) SHA1(0398a354c2588e3974cb76a331e46165db6af06d) )  // 8x8 HORIZONTAL
	ROM_LOAD( "vj98348_step3_18_v1.1", 0x0c00000, 0x400000, CRC(bc92f0a0) SHA1(49c08de7a898a27972d4209709ddf447c5dca36a) )  // 8x8 VERTICAL

	ROM_REGION( 0x1800000, "sprite_r", 0 )  // right screen sprites (vertical)
	ROM_LOAD( "mr9930-01.ic30",        0x0000000, 0x400000, CRC(9e3e054e) SHA1(06a4fa76cb83dbe9d565d5ccd0a5ecc5067887c9) )  // sprites? VERTICAL
	ROM_LOAD( "mr9930-02.ic29",        0x0400000, 0x400000, CRC(b23c29f4) SHA1(a7b10a3a9af43db319baf8633bb3728120960923) )  // 8x8 VERTICAL
	ROM_LOAD( "mr9930-03.ic28",        0x0800000, 0x400000, CRC(9a5d070f) SHA1(b4668b4f299033140a2c56499cc2712ba111cb57) )  // 8x8 VERTICAL
	ROM_LOAD( "vj98348_step3_25_v1.1", 0x0c00000, 0x400000, CRC(dec612df) SHA1(acb86bb90c1cc61c7db3e022c69a5ff0611ffbae) )  // 8x8 VERTICAL

	ROM_REGION( 0x400000, "tiles_fg", 0 ) // foreground tiles
	ROM_LOAD( "vjdash_ver1.0.ic27", 0x000000, 0x080000, BAD_DUMP CRC(f3cff858) SHA1(9277e5fb3494f7afb7f3911792d1c68b2b1b147e) )

	ROM_REGION( 0x400000, "sprite", ROMREGION_ERASE )   /* 8x8x8 (Sprites) */

	ROM_REGION( 0x400000, "tiles_bg", ROMREGION_ERASE )   /* 16x16x8 (Background) */

	ROM_REGION( 0x400000, "tiles_rot", ROMREGION_ERASE )   /* 16x16x8 (Rotation) */

	DISK_REGION( "jaleco_vj_pc:pci:07.1:ide1:0:hdd" )
	DISK_IMAGE( "step3", 0, SHA1(926a32998c837f7ba45d07db243c43c1f9d46d6a) )
ROM_END


/***************************************************************************


                                Game Drivers


***************************************************************************/

//    YEAR, NAME,      PARENT,   MACHINE,  INPUT,     STATE,          INIT,         MONITOR, COMPANY,                       FULLNAME,                               FLAGS
GAME( 1997, tetrisp2,  0,        tetrisp2, tetrisp2,  tetrisp2_state, empty_init,   ROT0,                      "Jaleco / The Tetris Company", "Tetris Plus 2 (World, V2.8)",               MACHINE_SUPPORTS_SAVE )
GAME( 1997, tetrisp2a, tetrisp2, tetrisp2, tetrisp2,  tetrisp2_state, empty_init,   ROT0,                      "Jaleco / The Tetris Company", "Tetris Plus 2 (World, V2.7)",               MACHINE_SUPPORTS_SAVE )
GAME( 1997, tetrisp2j, tetrisp2, tetrisp2, tetrisp2j, tetrisp2_state, empty_init,   ROT0,                      "Jaleco / The Tetris Company", "Tetris Plus 2 (Japan, V2.2)",               MACHINE_SUPPORTS_SAVE )
GAME( 1997, tetrisp2ja,tetrisp2, tetrisp2, tetrisp2j, tetrisp2_state, empty_init,   ROT0,                      "Jaleco / The Tetris Company", "Tetris Plus 2 (Japan, V2.1)",               MACHINE_SUPPORTS_SAVE )

GAME( 1997, nndmseal,  0,        nndmseal, nndmseal,  nndmseal_state, empty_init,   ROT0 | ORIENTATION_FLIP_X, "I'Max / Jaleco",              "Nandemo Seal Iinkai (ver 1.3)",             MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1997, nndmseal11,nndmseal, nndmseal, nndmseal,  nndmseal_state, empty_init,   ROT0 | ORIENTATION_FLIP_X, "I'Max / Jaleco",              "Nandemo Seal Iinkai (ver 1.1)",             MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1997, nndmseala, nndmseal, nndmseal, nndmseal,  nndmseal_state, empty_init,   ROT0 | ORIENTATION_FLIP_X, "I'Max / Jaleco",              "Nandemo Seal Iinkai (Astro Boy ver. 1.0?)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // version guessed
GAME( 1997, nndmsealb, nndmseal, nndmseal, nndmseal,  nndmseal_state, empty_init,   ROT0 | ORIENTATION_FLIP_X, "I'Max / Jaleco",              "Nandemo Seal Iinkai (Astro Boy ver. 1.1)",  MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // appears to have one more mode than the one above, ver taken from PRG ROM labels
GAME( 1997, nndmsealc, nndmseal, nndmseal, nndmseal,  nndmseal_state, empty_init,   ROT0 | ORIENTATION_FLIP_X, "I'Max / Jaleco",              "Nandemo Seal Iinkai (alternate ver 1.0)",   MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // only shows Jaleco copyright even though I'Max is in strings in ROMs. Ver taken from PRG ROM labels

GAME( 1999, rockn,     0,        rockn,    rockn,     rockn_state,    init_rockn,   ROT270,                    "Jaleco",                      "Rock'n Tread (Japan)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1999, rockna,    rockn,    rockn,    rockn,     rockn_state,    init_rockn,   ROT270,                    "Jaleco",                      "Rock'n Tread (Japan, alternate)",           MACHINE_SUPPORTS_SAVE )
GAME( 1999, rockn2,    0,        rockn2,   rockn,     rockn_state,    init_rockn2,  ROT270,                    "Jaleco",                      "Rock'n Tread 2 (Japan)",                    MACHINE_SUPPORTS_SAVE )
GAME( 1999, rocknms,   0,        rocknms,  rocknms,   rocknms_state,  init_rocknms, ROT0,                      "Jaleco",                      "Rock'n MegaSession (Japan)",                MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, rockn3,    0,        rockn2,   rockn,     rockn_state,    init_rockn3,  ROT270,                    "Jaleco",                      "Rock'n 3 (Japan)",                          MACHINE_SUPPORTS_SAVE )
GAME( 2000, rockn4,    0,        rockn2,   rockn,     rockn_state,    init_rockn3,  ROT270,                    "Jaleco / PCCWJ",              "Rock'n 4 (Japan, prototype)",               MACHINE_SUPPORTS_SAVE )

GAME( 1999, vjslap,    0,        vjdash,   vjdash,    stepstag_state, empty_init,   ROT0,                      "Jaleco",                      "VJ: Visual & Music Slap",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, vjdash,    vjslap,   vjdash,   vjdash,    stepstag_state, empty_init,   ROT0,                      "Jaleco",                      "VJ Dash (ver 1.0)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, vjdasha,   vjslap,   vjdash,   vjdash,    stepstag_state, empty_init,   ROT0,                      "Jaleco",                      "VJ Dash (Ver 1.2)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

// Undumped:
// - Stepping Stage <- the original Game
// - Stepping Stage 2 Supreme
// Dumped (partially):
GAME( 1999, stepstag,  0,        stepstag, stepstag,  stepstag_state, empty_init,   ROT0,                      "Jaleco",                      "Stepping Stage Special",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, step3,     0,        stepstag, stepstag,  stepstag_state, empty_init,   ROT0,                      "Jaleco",                      "Stepping 3 Superior",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
