// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/***************************************************************************

                          -= Newer Seta Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)


CPU    :    TMP68301*
            or ColdFire + H8/3007 + PIC12C508 (for EVA2 & EVA3 PCBs)

Video  :    DX-101 or X1-020 (for P0-113A & P0-121A PCBs, compatible?)
            DX-102 x3

Sound  :    X1-010
            or OKI M9810 (for EVA2 & EVA3 PCBs)

OSC    :    50.00000MHz
            32.53047MHz

*   The Toshiba TMP68301 is a 68HC000 + serial I/O, parallel I/O,
    3 timers, address decoder, wait generator, interrupt controller,
    all integrated in a single chip.

-------------------------------------------------------------------------------------------
Ordered by Board        Year    Game                                    By
-------------------------------------------------------------------------------------------
P-FG01-1 (also P0-113A) 1995    Guardians / Denjin Makai II             Banpresto
P0-113A                 1994    Mobile Suit Gundam EX Revue             Banpresto
P0-121A ; 2MP1-E00 (Ss) 1996    TelePachi Fever Lion                    Sunsoft
P0-123A                 1996    Wakakusamonogatari Mahjong Yonshimai    Maboroshi Ware
P0-125A ; KE (Namco)    1996    Kosodate Quiz My Angel                  Namco
P0-130B ; M-133 (Namco) 1997    Star Audition                           Namco
P0-136A ; KL (Namco)    1997    Kosodate Quiz My Angel 2                Namco
P-FG-02                 1997    Reel'N Quake                            <unknown>
P-FG-03                 ????    Endless Riches                          E.N.Tiger
P0-140B                 2000    Funcube                                 Namco
P0-140B                 2000    Namco Stars                             Namco
P0-142A                 1999    Puzzle De Bowling                       MOSS / Nihon System
P0-142A + extra parts   2000    Penguin Brothers / A-Blast              Subsino
B0-003A (or B0-003B)    2000    Deer Hunting USA                        Sammy
B0-003A (or B0-003B)    2001    Turkey Hunting USA                      Sammy
B0-006B                 2001-2  Funcube 2 - 5                           Namco
B0-010A                 2001    Wing Shooting Championship              Sammy
B0-010A                 2002    Trophy Hunting - Bear & Moose           Sammy
P0-145-1                2002    Trophy Hunting - Bear & Moose (test)    Sammy
-------------------------------------------------------------------------------------------

TODO:

- Proper emulation of the ColdFire CPU, in a core file.
- improvements to Flip screen / Zooming support. (Flip Screen is often done with 'negative zoom value')
- Fix some graphics imperfections (e.g. color depth selection, "tilemap" sprites) [all done? - NS]
- I added a kludge involving a -0x10 yoffset, this fixes the lifeline in myangel.
  I didn't find a better way to do it without breaking pzlbowl's title screen.
- Background color is not verified
- Device-fy video chip and split according to hardware (i.e. NamcoEVA2 and 3).

gundamex:
- slowdowns, music tempo is incorrect

mj4simai:
- test mode doesn't work correctly, the grid is OK but when you press a key to go to the
  next screen (input test) it stays up a second and then drops back into the game

myangel:
- some gfx at the end of the game (rays just before fireworks, and the border during
  the wedding) have wrong colors. You can see the rays red, green and yellow because
  that's how the palette is preinitialized by MAME, but the game never sets up those
  palette entries. The game selects color depth "1", whose meaning is uncertain, and
  color code 0 so I see no way to point to a different section of palette RAM.
- there are glitches in the bg horizontal scroll in the wedding sequence at the end of
  the game. It looks like "scrollx" should be delayed one frame wrt "xoffs".
- there's a 4 pixel gap at the top of the title screen since clipping was reimplemented.

myangel2:
- before each level, the background image is shown with completely wrong colors. It
  corrects itself when the level starts.

wschampb:
- dumps of the program ROMs matched the hand written checksum for each chip, but
  the boot screen reports NG for both ROMs. - Is this correct and a bug from the
  original release? Is that why the next bug fix release is v1.01? IE: such a
  a minor increase in the version number.

funcube series:
- Hacked to run, as they use a ColdFire CPU.
- Pay-out key causes "unknown error" after coin count reaches 0.

***************************************************************************/

#include "emu.h"
#include "seta2.h"

#include "cpu/m68000/mcf5206e.h"
#include "machine/mcf5206e.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/watchdog.h"

#include "diserial.h"
#include "speaker.h"

#define LOG_IO      (1U << 1)
#define LOG_DEBUG   (1U << 2)

#define LOG_ALL     (LOG_IO)

#define VERBOSE (0)
#include "logmacro.h"

#define LOGIO(...)     LOGMASKED(LOG_IO, __VA_ARGS__)
#define LOGDEBUG(...)  LOGMASKED(LOG_DEBUG, __VA_ARGS__)


/***************************************************************************


                            Memory Maps - Main CPU


***************************************************************************/

void seta2_state::machine_start()
{
	if (memregion("x1snd") != nullptr)
	{
		uint32_t const max = memregion("x1snd")->bytes() / 0x20000;
		for (int i = 0; i < 8; i++)
		{
			if (m_x1_bank[i] != nullptr)
			{
				uint32_t ind = 0;
				while (ind < 256)
				{
					m_x1_bank[i]->configure_entries(ind, max, memregion("x1snd")->base(), 0x20000); // TODO : Mirrored?
					ind += max;
				}
			}
		}
	}

	m_leds.resolve();
	m_lamps.resolve();
}

void seta2_state::sound_bank_w(offs_t offset, uint8_t data)
{
	m_x1_bank[offset & 7]->set_entry(data);
}

void seta2_state::x1_map(address_map &map)
{
	map(0x00000, 0x1ffff).bankr(m_x1_bank[0]);
	map(0x20000, 0x3ffff).bankr(m_x1_bank[1]);
	map(0x40000, 0x5ffff).bankr(m_x1_bank[2]);
	map(0x60000, 0x7ffff).bankr(m_x1_bank[3]);
	map(0x80000, 0x9ffff).bankr(m_x1_bank[4]);
	map(0xa0000, 0xbffff).bankr(m_x1_bank[5]);
	map(0xc0000, 0xdffff).bankr(m_x1_bank[6]);
	map(0xe0000, 0xfffff).bankr(m_x1_bank[7]);
}


/***************************************************************************
                                Guardians
***************************************************************************/

void seta2_state::grdians_lockout_w(uint8_t data)
{
	// initially 0, then either $25 (coin 1) or $2a (coin 2)
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));   // or 0x04
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));   // or 0x08
	//LOGIO("%04X\n", data & 0xff);
}

void seta2_state::grdians_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();                                                                 // ROM
	map(0x200000, 0x20ffff).ram();                                                                 // RAM
	map(0x304000, 0x30ffff).ram();                                                                 // ? seems tile data
	map(0x600000, 0x600001).portr("DSW1");                                                         // DSW 1
	map(0x600002, 0x600003).portr("DSW2");                                                         // DSW 2
	map(0x700000, 0x700001).portr("P1");                                                           // P1
	map(0x700002, 0x700003).portr("P2");                                                           // P2
	map(0x700004, 0x700005).portr("SYSTEM");                                                       // Coins
	map(0x70000c, 0x70000d).r("watchdog", FUNC(watchdog_timer_device::reset16_r));
	map(0x800001, 0x800001).w(FUNC(seta2_state::grdians_lockout_w));
	map(0xb00000, 0xb03fff).rw("x1snd", FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w)); // Sound
	map(0xc00000, 0xc3ffff).ram().w(FUNC(seta2_state::spriteram_w)).share(m_spriteram);            // Sprites
	map(0xc40000, 0xc4ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    // Palette
	map(0xc50000, 0xc5ffff).ram();                                                                 // cleared
	map(0xc60000, 0xc6003f).ram().w(FUNC(seta2_state::vregs_w)).share(m_vregs);                    // Video Registers
	map(0xe00010, 0xe0001f).w(FUNC(seta2_state::sound_bank_w)).umask16(0x00ff);                    // Samples Banks
}

/***************************************************************************
                        Mobile Suit Gundam EX Revue
***************************************************************************/

uint16_t seta2_state::gundamex_eeprom_r()
{
	return (m_eeprom->do_read() & 1) << 3;
}

void seta2_state::gundamex_eeprom_w(uint16_t data)
{
	m_eeprom->clk_write(BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->di_write(BIT(data, 0));
	m_eeprom->cs_write(BIT(data, 2) ? ASSERT_LINE : CLEAR_LINE);
}

void seta2_state::gundamex_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();                                                                 // ROM
	map(0x200000, 0x20ffff).ram();                                                                 // RAM
	map(0x500000, 0x57ffff).rom();                                                                 // ROM
	map(0x600000, 0x600001).portr("DSW1");                                                         // DSW 1
	map(0x600002, 0x600003).portr("DSW2");                                                         // DSW 2
	map(0x700000, 0x700001).portr("P1");                                                           // P1
	map(0x700002, 0x700003).portr("P2");                                                           // P2
	map(0x700004, 0x700005).portr("SYSTEM");                                                       // Coins
	map(0x700008, 0x700009).portr("IN0");                                                          // P1
	map(0x70000a, 0x70000b).portr("IN1");                                                          // P2
	map(0x70000c, 0x70000d).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x800000, 0x800001).w(FUNC(seta2_state::grdians_lockout_w));
	map(0xb00000, 0xb03fff).rw("x1snd", FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w)); // Sound
	map(0xc00000, 0xc3ffff).ram().share(m_spriteram);                                              // Sprites
	map(0xc40000, 0xc4ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    // Palette
	map(0xc50000, 0xc5ffff).ram();                                                                 // cleared
	map(0xc60000, 0xc6003f).ram().w(FUNC(seta2_state::vregs_w)).share(m_vregs);                    // Video Registers
	map(0xe00010, 0xe0001f).w(FUNC(seta2_state::sound_bank_w)).umask16(0x00ff);                    // Samples Banks
}


/***************************************************************************
                      Wakakusamonogatari Mahjong Yonshimai
***************************************************************************/

void mj4simai_state::machine_start()
{
	seta2_state::machine_start();
	save_item(NAME(m_keyboard_row));
}

template <unsigned Which>
uint16_t mj4simai_state::mj4simai_key_r()
{
	uint16_t result = 0xffff;
	if (BIT(m_keyboard_row, 0)) result &= m_keys[Which][0]->read();
	if (BIT(m_keyboard_row, 1)) result &= m_keys[Which][1]->read();
	if (BIT(m_keyboard_row, 2)) result &= m_keys[Which][2]->read();
	if (BIT(m_keyboard_row, 3)) result &= m_keys[Which][3]->read();
	if (BIT(m_keyboard_row, 4)) result &= m_keys[Which][4]->read();
	return result;
}

void mj4simai_state::mj4simai_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();                                                                 // ROM
	map(0x200000, 0x20ffff).ram();                                                                 // RAM
	map(0x600000, 0x600001).r(FUNC(mj4simai_state::mj4simai_key_r<0>));                            // P1
	map(0x600002, 0x600003).r(FUNC(mj4simai_state::mj4simai_key_r<1>));                            // P2
	map(0x600005, 0x600005).lw8(NAME([this] (u8 data) { m_keyboard_row = data; }));                // select keyboard row to read
	map(0x600006, 0x600007).r("watchdog", FUNC(watchdog_timer_device::reset16_r));
	map(0x600100, 0x600101).portr("SYSTEM");
	map(0x600200, 0x600201).nopw();                                                                // LEDs? Coins?
	map(0x600300, 0x600301).portr("DSW1");                                                         // DSW 1
	map(0x600302, 0x600303).portr("DSW2");                                                         // DSW 2
	map(0x600300, 0x60030f).w(FUNC(mj4simai_state::sound_bank_w)).umask16(0x00ff);                 // Samples Banks
	map(0xb00000, 0xb03fff).rw("x1snd", FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w)); // Sound
	map(0xc00000, 0xc3ffff).ram().share(m_spriteram);                                              // Sprites
	map(0xc40000, 0xc4ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    // Palette
	map(0xc60000, 0xc6003f).ram().w(FUNC(mj4simai_state::vregs_w)).share(m_vregs);                 // Video Registers
}


/***************************************************************************
                            Kosodate Quiz My Angel
***************************************************************************/

void seta2_state::myangel_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();                                                                 // ROM
	map(0x200000, 0x20ffff).ram();                                                                 // RAM
	map(0x700000, 0x700001).portr("P1");                                                           // P1
	map(0x700002, 0x700003).portr("P2");                                                           // P2
	map(0x700004, 0x700005).portr("SYSTEM");                                                       // Coins
	map(0x700006, 0x700007).r("watchdog", FUNC(watchdog_timer_device::reset16_r));
	map(0x700200, 0x700201).nopw();                                                                // LEDs? Coins?
	map(0x700300, 0x700301).portr("DSW1");                                                         // DSW 1
	map(0x700302, 0x700303).portr("DSW2");                                                         // DSW 2
	map(0x700310, 0x70031f).w(FUNC(seta2_state::sound_bank_w)).umask16(0x00ff);                    // Samples Banks
	map(0xb00000, 0xb03fff).rw("x1snd", FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w)); // Sound
	map(0xc00000, 0xc3ffff).ram().share(m_spriteram);                                              // Sprites
	map(0xc40000, 0xc4ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    // Palette
	map(0xc60000, 0xc6003f).ram().w(FUNC(seta2_state::vregs_w)).share(m_vregs);                    // Video Registers
}


/***************************************************************************
                            Kosodate Quiz My Angel 2
***************************************************************************/

void seta2_state::myangel2_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();                                                                 // ROM
	map(0x200000, 0x20ffff).ram();                                                                 // RAM
	map(0x600000, 0x600001).portr("P1");                                                           // P1
	map(0x600002, 0x600003).portr("P2");                                                           // P2
	map(0x600004, 0x600005).portr("SYSTEM");                                                       // Coins
	map(0x600006, 0x600007).r("watchdog", FUNC(watchdog_timer_device::reset16_r));
	map(0x600200, 0x600201).nopw();                                                                // LEDs? Coins?
	map(0x600300, 0x600301).portr("DSW1");                                                         // DSW 1
	map(0x600302, 0x600303).portr("DSW2");                                                         // DSW 2
	map(0x600300, 0x60030f).w(FUNC(seta2_state::sound_bank_w)).umask16(0x00ff);                    // Samples Banks
	map(0xb00000, 0xb03fff).rw("x1snd", FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w)); // Sound
	map(0xd00000, 0xd3ffff).ram().share(m_spriteram);                                              // Sprites
	map(0xd40000, 0xd4ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    // Palette
	map(0xd60000, 0xd6003f).ram().w(FUNC(seta2_state::vregs_w)).share(m_vregs);                    // Video Registers
}


/***************************************************************************
                                Puzzle De Bowling
***************************************************************************/

/*  The game checks for a specific value read from the ROM region.
    The offset to use is stored in RAM at address 0x20BA16 */
uint16_t seta2_state::pzlbowl_protection_r(address_space &space)
{
	const uint32_t address = (space.read_word(0x20ba16) << 16) | space.read_word(0x20ba18);
	return memregion("maincpu")->base()[address - 2];
}

uint8_t seta2_state::pzlbowl_coins_r()
{
	return m_in_system->read() | (machine().rand() & 0x80);
}

void seta2_state::pzlbowl_coin_counter_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 4));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 5));
}

void seta2_state::pzlbowl_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                                 // ROM
	map(0x200000, 0x20ffff).ram();                                                                 // RAM
	map(0x400300, 0x400301).portr("DSW1");                                                         // DSW 1
	map(0x400302, 0x400303).portr("DSW2");                                                         // DSW 2
	map(0x400300, 0x40030f).w(FUNC(seta2_state::sound_bank_w)).umask16(0x00ff);                    // Samples Banks
	map(0x500000, 0x500001).portr("P1");                                                           // P1
	map(0x500002, 0x500003).portr("P2");                                                           // P2
	map(0x500005, 0x500005).rw(FUNC(seta2_state::pzlbowl_coins_r), FUNC(seta2_state::pzlbowl_coin_counter_w));   // Coins + Protection?
	map(0x500006, 0x500007).r("watchdog", FUNC(watchdog_timer_device::reset16_r));
	map(0x700000, 0x700001).r(FUNC(seta2_state::pzlbowl_protection_r));                            // Protection
	map(0x800000, 0x83ffff).ram().share(m_spriteram);                                              // Sprites
	map(0x840000, 0x84ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    // Palette
	map(0x860000, 0x86003f).ram().w(FUNC(seta2_state::vregs_w)).share(m_vregs);                    // Video Registers
	map(0x900000, 0x903fff).rw("x1snd", FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w)); // Sound
}


/***************************************************************************
                            Penguin Bros
***************************************************************************/

void seta2_state::penbros_base_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x20ffff).ram();
	map(0x210000, 0x21ffff).ram(); // zeroed at startup, then never written again on originals, used on the bootleg
	map(0x220000, 0x22ffff).ram(); // zeroed at startup, then never written again
	map(0x230000, 0x23ffff).ram(); // zeroed at startup, then never written again on originals, used on the bootleg
	map(0x600000, 0x600001).portr("P1");
	map(0x600002, 0x600003).portr("P2");
	map(0x600004, 0x600005).portr("SYSTEM");
	map(0x600005, 0x600005).w(FUNC(seta2_state::pzlbowl_coin_counter_w));
	map(0x600006, 0x600007).r("watchdog", FUNC(watchdog_timer_device::reset16_r));
	map(0xa00000, 0xa03fff).rw("x1snd", FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));
	map(0xb00000, 0xb3ffff).ram().share("spriteram");
	map(0xb40000, 0xb4ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
}

void seta2_state::penbros_map(address_map &map)
{
	penbros_base_map(map);
	map(0x300000, 0x30ffff).ram();
	map(0x500300, 0x500301).portr("DSW1");
	map(0x500302, 0x500303).portr("DSW2");
	map(0x500300, 0x50030f).w(FUNC(seta2_state::sound_bank_w)).umask16(0x00ff);
	map(0xb60000, 0xb6003f).ram().w(FUNC(seta2_state::vregs_w)).share(m_vregs);
}

void seta2_state::ablastb_map(address_map &map)
{
	penbros_base_map(map);
	map(0x508300, 0x508301).portr("DSW1");
	map(0x508302, 0x508303).portr("DSW2");
	// TODO: Is there samples banking like in the original?
}


/***************************************************************************
                              Reel'N Quake
***************************************************************************/

void seta2_state::reelquak_leds_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		// bit 0 - start
		// bit 1 - small
		// bit 2 - bet
		// bit 3 - big
		// bit 4 - double up
		// bit 5 - collect
		// bit 6 - bet cancel
		for (int i = 0; i <= 6; i++)
			m_leds[i] = BIT(data, i);
	}
	if (ACCESSING_BITS_8_15)
	{
		m_dispenser->motor_w(BIT(data, 8)); // ticket dispenser
	}

	//LOGIO("LED %04X\n", data);
}

void seta2_state::reelquak_coin_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));  // coin in
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));  // coin in
	machine().bookkeeping().coin_counter_w(2, BIT(data, 2));  // pay out
	machine().bookkeeping().coin_counter_w(3, BIT(data, 3));  // key in
	// BIT(data, 4)); // Sound IRQ Ack.? 1->0
	// BIT(data, 5)); // Vblank IRQ.? 1
	//LOGIO("COIN %04X\n", data & 0xff);
}

void seta2_state::reelquak_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                                 // ROM
	map(0x200000, 0x20ffff).ram();                                                                 // RAM
	map(0x300000, 0x303fff).ram().share("nvram");                                                  // NVRAM (Battery Backed)
	map(0x400000, 0x400001).portr("P1");                                                           // P1
	map(0x400002, 0x400003).portr("TICKET");                                                       // Tickets
	map(0x400004, 0x400005).portr("SYSTEM");                                                       // Coins
	map(0x400006, 0x400007).r("watchdog", FUNC(watchdog_timer_device::reset16_r));
	map(0x400201, 0x400201).w(FUNC(seta2_state::reelquak_coin_w));                                 // Coin Counters / IRQ Ack
	map(0x400300, 0x400301).portr("DSW1");                                                         // DSW 1
	map(0x400302, 0x400303).portr("DSW2");                                                         // DSW 2
	map(0x400300, 0x40030f).w(FUNC(seta2_state::sound_bank_w)).umask16(0x00ff);                    // Samples Banks
	map(0xb00000, 0xb03fff).rw("x1snd", FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w)); // Sound
	map(0xc00000, 0xc3ffff).ram().share(m_spriteram);                                              // Sprites
	map(0xc40000, 0xc4ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    // Palette
	map(0xc60000, 0xc6003f).ram().w(FUNC(seta2_state::vregs_w)).share(m_vregs);                    // Video Registers
}


/***************************************************************************
                                Namco Stars
***************************************************************************/

// To be done:
void seta2_state::namcostr_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                              // ROM
	map(0x200000, 0x20ffff).ram();                                              // RAM
	map(0xc00000, 0xc3ffff).ram().share(m_spriteram);                           // Sprites
	map(0xc60000, 0xc6003f).ram().w(FUNC(seta2_state::vregs_w)).share(m_vregs); // Video Registers
}


/***************************************************************************
                            Sammy Outdoor Shooting
***************************************************************************/

void seta2_state::samshoot_coin_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT( data, 4));
	machine().bookkeeping().coin_counter_w(1, BIT( data, 5));

	// Are these connected? They are set in I/O test
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 6));
	machine().bookkeeping().coin_lockout_w(1, BIT(~data, 7));
	//LOGIO("%04x\n",data);
}

void seta2_state::samshoot_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x200000, 0x20ffff).ram();
	map(0x300000, 0x30ffff).ram().share("nvram");

	map(0x400000, 0x400001).portr("DSW1");                                                         // DSW 1
	map(0x400002, 0x400003).portr("BUTTONS");                                                      // Buttons

	map(0x400300, 0x40030f).w(FUNC(seta2_state::sound_bank_w)).umask16(0x00ff);                    // Samples Banks

	map(0x500000, 0x500001).portr("GUN1");                                                         // P1
	map(0x580000, 0x580001).portr("GUN2");                                                         // P2

	map(0x700000, 0x700001).portr("TRIGGER");                                                      // Trigger
	map(0x700002, 0x700003).portr("PUMP");                                                         // Pump
	map(0x700004, 0x700005).portr("COIN");                                                         // Coins
	map(0x700005, 0x700005).w(FUNC(seta2_state::samshoot_coin_w));                                 // Coins
	map(0x700006, 0x700007).r("watchdog", FUNC(watchdog_timer_device::reset16_r));                 // Watchdog?

	map(0x800000, 0x83ffff).ram().share(m_spriteram);                                              // Sprites
	map(0x840000, 0x84ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    // Palette
	map(0x860000, 0x86003f).ram().w(FUNC(seta2_state::vregs_w)).share(m_vregs);                    // Video Registers

	map(0x900000, 0x903fff).rw("x1snd", FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w)); // Sound
}


/***************************************************************************
                              Star Audition
***************************************************************************/

// Outputs

void staraudi_state::staraudi_debug_outputs()
{
	//LOGDEBUG("L1: %04X L2: %04X CAM: %04X\n", m_lamps1, m_lamps2, m_cam);
}

void staraudi_state::lamps1_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_lamps1);
	m_leds[0] = BIT(data, 0); // Lamp 1 |
	m_leds[1] = BIT(data, 1); // Lamp 2 |- Camera Lamps
	m_leds[2] = BIT(data, 2); // Lamp 3 |
	// BIT(data, 3) );  // Degauss
	staraudi_debug_outputs();
}

void staraudi_state::lamps2_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_lamps2);
	// BIT(data, 5) ); // ? Always On
	m_leds[3] = BIT(data, 6); // 2P Switch Lamp
	m_leds[4] = BIT(data, 7); // 1P Switch Lamp
	staraudi_debug_outputs();
}

void staraudi_state::camera_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_cam);
	// BIT(data, 0) ); // ? Always On
	// BIT(data, 1) ); // ? Print Test
	// BIT(data, 3) ); // Camera On (Test Mode)
	// BIT(data, 5) ); // ?
	staraudi_debug_outputs();
}

// Tile RAM

#define TILE0 (0x7c000)
#define TILERAM(offset) ((uint16_t*)(memregion("sprites")->base() + TILE0 * 8*8 + (offset * 2 / 0x20000) * 2 + ((offset * 2) % 0x20000) / 2 * 8))

uint16_t staraudi_state::tileram_r(offs_t offset)
{
	return *TILERAM(offset);
}

void staraudi_state::tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(TILERAM(offset));
	int tile = TILE0 + ((offset * 2) % 0x20000) / (8*2);
	for (int i = 0; m_gfxdecode->gfx(i); ++i)
		m_gfxdecode->gfx(i)->mark_dirty(tile);
}

void staraudi_state::staraudi_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();                             // ROM
	map(0x200000, 0x23ffff).ram();                             // RAM

	map(0x400000, 0x45ffff).rw(FUNC(staraudi_state::tileram_r), FUNC(staraudi_state::tileram_w)).share("tileram"); // Tile RAM

//  map(0x500000, 0x53ffff).ram();                             // Camera RAM (r8g8)
//  map(0x540000, 0x57ffff).ram();                             // Camera RAM (00b8)
	map(0x500000, 0x57ffff).ram().share(m_rgbram);

	map(0x600001, 0x600001).w(FUNC(staraudi_state::camera_w));        // Camera Outputs

	map(0x700000, 0x700001).portr("P1");                 // P1
	map(0x700002, 0x700003).portr("P2");                 // P2
	map(0x700004, 0x700005).portr("SYSTEM");             // Coins
	map(0x700006, 0x700007).rw("watchdog", FUNC(watchdog_timer_device::reset16_r), FUNC(watchdog_timer_device::reset16_w));

	map(0x700101, 0x700101).w(FUNC(staraudi_state::lamps1_w));        // Lamps 1
	map(0x700180, 0x70018f).rw(m_rtc, FUNC(upd4992_device::read), FUNC(upd4992_device::write)).umask16(0x00ff);
	map(0x700201, 0x700201).w(FUNC(staraudi_state::lamps2_w));        // Lamps 2
	map(0x700300, 0x700301).portr("DSW1");               // DSW 1
	map(0x700302, 0x700303).portr("DSW2");               // DSW 2
	map(0x700300, 0x70030f).w(FUNC(staraudi_state::sound_bank_w));             // Samples Banks

	map(0x800000, 0x9fffff).rw(m_flash, FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));

	map(0xb00000, 0xb03fff).rw("x1snd", FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));   // Sound
	map(0xc00000, 0xc3ffff).ram().share("spriteram");       // Sprites
	map(0xc40000, 0xc4ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    // Palette
	map(0xc50000, 0xc5ffff).ram();                             // cleared
	map(0xc60000, 0xc6003f).ram().w(FUNC(staraudi_state::vregs_w)).share(m_vregs);  // Video Registers
}


/***************************************************************************
                            TelePachi Fever Lion
***************************************************************************/

void seta2_state::telpacfl_lamp1_w(uint8_t data)
{
	for (int i = 0; i <= 7; i++)
		m_lamps[i] = BIT(data, i);

	//LOGIO("LAMP1 %04X\n", data);
}

void seta2_state::telpacfl_lamp2_w(uint8_t data)
{
	m_lamps[8] = BIT(data, 0); // on/off lamp (throughout)
	m_lamps[9] = BIT(data, 1); // bet lamp
	m_lamps[10] = BIT(data, 2); // payout lamp
	m_dispenser->motor_w(BIT(data, 3)); // coin out motor
	machine().bookkeeping().coin_counter_w(0, BIT(data, 4)); // coin out counter
	// BIT(data, 5) ); // on credit increase

	//LOGIO("LAMP2 %04X\n", data);
}

void seta2_state::telpacfl_lockout_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(1, BIT( data, 1)); // 100yen in
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 2)); // coin blocker
	machine().bookkeeping().coin_lockout_w(1, BIT(~data, 3)); // 100yen blocker
	// bits 0x30 ?

	//LOGIO("LOCK %04X\n", data);
}

void seta2_state::telpacfl_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                                 // ROM
	map(0x200000, 0x20ffff).ram();                                                                 // RAM
	map(0x300000, 0x303fff).ram().share("nvram");                                                  // NVRAM (Battery Backed)
	map(0x600000, 0x600001).portr("DSW1");                                                         // DSW 1
	map(0x600002, 0x600003).portr("DSW2");                                                         // DSW 2
	map(0x700000, 0x700001).portr("COIN");                                                         // Coin
	map(0x700002, 0x700003).portr("P1");                                                           // P1 + Dispenser
	map(0x700004, 0x700005).portr("SERVICE");                                                      // Service
	map(0x700006, 0x700007).portr("UNKNOWN");                                                      // (unused?)
	map(0x700009, 0x700009).w(FUNC(seta2_state::telpacfl_lamp1_w));                                // Lamps
	map(0x70000d, 0x70000d).w(FUNC(seta2_state::telpacfl_lamp2_w));                                // ""
	map(0x800001, 0x800001).w(FUNC(seta2_state::telpacfl_lockout_w));                              // Coin Blockers
	map(0x900000, 0x903fff).rw("x1snd", FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w)); // Sound
	map(0xb00000, 0xb3ffff).ram().share(m_spriteram);                                              // Sprites
	map(0xb40000, 0xb4ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    // Palette
	map(0xb60000, 0xb6003f).ram().w(FUNC(seta2_state::vregs_w)).share(m_vregs);                    // Video Registers
	map(0xd00006, 0xd00007).r("watchdog", FUNC(watchdog_timer_device::reset16_r));
//  map(0xe00000, 0xe00001).w(FUNC(seta2_state::));
	map(0xe00010, 0xe0001f).w(FUNC(seta2_state::sound_bank_w)).umask16(0x00ff);                    // Samples Banks
}


/***************************************************************************
                               Funcube series
***************************************************************************/

// Touchscreen

class funcube_touchscreen_device : public device_t,
									public device_serial_interface
{
public:
	funcube_touchscreen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	auto tx_cb() { return m_tx_cb.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void tra_complete() override;
	virtual void tra_callback() override;

	TIMER_CALLBACK_MEMBER(read_buttons);

private:
	devcb_write_line m_tx_cb;
	required_ioport m_x;
	required_ioport m_y;
	required_ioport m_btn;

	uint8_t m_button_state;
	int m_serial_pos;
	uint8_t m_serial[4];
};

DEFINE_DEVICE_TYPE(FUNCUBE_TOUCHSCREEN, funcube_touchscreen_device, "funcube_touchscreen", "Funcube Touchscreen")

static INPUT_PORTS_START( funcube_touchscreen )
	PORT_START("touch_btn")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME( "Touch Screen" )

	PORT_START("touch_x")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_X ) PORT_MINMAX(0,0x5c+1) PORT_CROSSHAIR(X, -(1.0 * 0x05d/0x5c), -1.0/0x5c, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(5) PORT_REVERSE

	PORT_START("touch_y")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_Y ) PORT_MINMAX(0,0x46+1) PORT_CROSSHAIR(Y, -(0xf0-8.0)/0xf0*0x047/0x46, -1.0/0x46, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(5) PORT_REVERSE
INPUT_PORTS_END

funcube_touchscreen_device::funcube_touchscreen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, FUNCUBE_TOUCHSCREEN, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_tx_cb(*this),
	m_x(*this, "touch_x"),
	m_y(*this, "touch_y"),
	m_btn(*this, "touch_btn")
{
}

ioport_constructor funcube_touchscreen_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(funcube_touchscreen);
}

void funcube_touchscreen_device::device_start()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_tra_rate(9600);
	m_button_state = 0x00;
	emu_timer *tm = timer_alloc(FUNC(funcube_touchscreen_device::read_buttons), this);
	tm->adjust(attotime::from_ticks(1, clock()), 0, attotime::from_ticks(1, clock()));

	save_item(NAME(m_button_state));
	save_item(NAME(m_serial_pos));
	save_item(NAME(m_serial));
}

void funcube_touchscreen_device::device_reset()
{
	m_serial_pos = 0;
	memset(m_serial, 0, sizeof(m_serial));
	m_tx_cb(1);
}

TIMER_CALLBACK_MEMBER(funcube_touchscreen_device::read_buttons)
{
	const uint8_t button_state = m_btn->read();
	if (m_button_state != button_state)
	{
		m_button_state = button_state;
		m_serial[0] = button_state ? 0xfe : 0xfd;
		m_serial[1] = m_x->read();
		m_serial[2] = m_y->read();
		m_serial[3] = 0xff;
		m_serial_pos = 0;
		transmit_register_setup(m_serial[m_serial_pos++]);
	}
}

void funcube_touchscreen_device::tra_complete()
{
	if (m_serial_pos != 4)
		transmit_register_setup(m_serial[m_serial_pos++]);
}

void funcube_touchscreen_device::tra_callback()
{
	m_tx_cb(transmit_register_get_data_bit());
}


// Main CPU

uint32_t funcube_state::debug_r()
{
	uint32_t ret = m_in_debug->read();

	// This bits let you move the crosshair in the inputs / touch panel test with a joystick
	if (!(m_screen->frame_number() % 3))
		ret |= 0x3f;

	return ret;
}

void funcube_state::funcube_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom();
	map(0x00200000, 0x0020ffff).ram();

	map(0x00400000, 0x00400003).r(FUNC(funcube_state::debug_r));
	map(0x00400004, 0x00400007).r("watchdog", FUNC(watchdog_timer_device::reset32_r)).nopw();

	map(0x00500001, 0x00500001).rw(m_oki, FUNC(okim9810_device::read_status), FUNC(okim9810_device::write_command));
	map(0x00500003, 0x00500003).w(m_oki, FUNC(okim9810_device::write_tmp_register));

	map(0x00800000, 0x0083ffff).rw(FUNC(funcube_state::spriteram_r), FUNC(funcube_state::spriteram_w));
	map(0x00840000, 0x0084ffff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");  // Palette
	map(0x00860000, 0x0086003f).rw(FUNC(funcube_state::vregs_r), FUNC(funcube_state::vregs_w));

	map(0x00c00000, 0x00c002ff).rw(FUNC(funcube_state::nvram_r), FUNC(funcube_state::nvram_w)).umask32(0x00ff00ff);

	map(0xf0000000, 0xf00001ff).rw("maincpu_onboard", FUNC(mcf5206e_peripheral_device::seta2_coldfire_regs_r), FUNC(mcf5206e_peripheral_device::seta2_coldfire_regs_w)); // technically this can be moved with MBAR
	map(0xffffe000, 0xffffffff).ram();    // SRAM
}

void funcube_state::funcube2_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom();
	map(0x00200000, 0x0020ffff).ram();

	map(0x00500000, 0x00500003).r(FUNC(funcube_state::debug_r));
	map(0x00500004, 0x00500007).r("watchdog", FUNC(watchdog_timer_device::reset32_r)).nopw();

	map(0x00600001, 0x00600001).rw(m_oki, FUNC(okim9810_device::read_status), FUNC(okim9810_device::write_command));
	map(0x00600003, 0x00600003).w(m_oki, FUNC(okim9810_device::write_tmp_register));

	map(0x00800000, 0x0083ffff).rw(FUNC(funcube_state::spriteram_r), FUNC(funcube_state::spriteram_w));
	map(0x00840000, 0x0084ffff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x00860000, 0x0086003f).rw(FUNC(funcube_state::vregs_r), FUNC(funcube_state::vregs_w));

	map(0x00c00000, 0x00c002ff).rw(FUNC(funcube_state::nvram_r), FUNC(funcube_state::nvram_w)).umask32(0x00ff00ff);

	map(0xf0000000, 0xf00001ff).rw("maincpu_onboard", FUNC(mcf5206e_peripheral_device::seta2_coldfire_regs_r), FUNC(mcf5206e_peripheral_device::seta2_coldfire_regs_w)); // technically this can be moved with MBAR
	map(0xffffe000, 0xffffffff).ram();    // SRAM
}

// Sub CPU

void funcube_state::funcube_sub_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x200000, 0x20017f).rw(FUNC(funcube_state::nvram_r), FUNC(funcube_state::nvram_w)).umask16(0xffff);
}




// Simulate coin drop through two sensors

static constexpr XTAL FUNCUBE_SUB_CPU_CLOCK = XTAL(14'745'600);

uint8_t funcube_state::coins_r()
{
	uint8_t ret = m_in_switch->read();
	uint8_t coin_bit0 = 1; // active low
	uint8_t coin_bit1 = 1;

	const uint8_t hopper_bit = (m_hopper_motor && !(m_screen->frame_number() % 20)) ? 1 : 0;

	const uint64_t coin_total_cycles = FUNCUBE_SUB_CPU_CLOCK.value() / (1000/10);

	if (m_coin_start_cycles)
	{
		const uint64_t elapsed = m_sub->total_cycles() - m_coin_start_cycles;

		if (elapsed < coin_total_cycles/2)
			coin_bit0 = 0;
		else if (elapsed < coin_total_cycles)
			coin_bit1 = 0;
		else
		{
			if (!machine().side_effects_disabled())
				m_coin_start_cycles = 0;
		}
	}
	else
	{
		if (!machine().side_effects_disabled())
		{
			if (!(ret & 1))
				m_coin_start_cycles = m_sub->total_cycles();
		}
	}

	return (ret & ~7) | (hopper_bit << 2) | (coin_bit1 << 1) | coin_bit0;
}

void funcube_state::funcube_debug_outputs()
{
	//LOGDEBUG("LED: %02x OUT: %02x\n", m_funcube_leds, m_outputs);
}

void funcube_state::leds_w(uint8_t data)
{
	m_funcube_leds = data;

	m_leds[0] = BIT(~data, 0); // win lamp (red)
	m_leds[1] = BIT(~data, 1); // win lamp (green)

	// Set in a moving pattern: 0111 -> 1011 -> 1101 -> 1110
	m_leds[2] = BIT(~data, 4);
	m_leds[3] = BIT(~data, 5);
	m_leds[4] = BIT(~data, 6);
	m_leds[5] = BIT(~data, 7);

	funcube_debug_outputs();
}

uint8_t funcube_state::outputs_r()
{
	// Bits 1,2,3 read
	return m_outputs;
}

void funcube_state::outputs_w(uint8_t data)
{
	m_outputs = data;

	// Bits 0,1,3 written

	// Bit 0: hopper motor
	m_hopper_motor = BIT(~data, 0);

	// Bit 1: high on pay out

	// Bit 3: low after coining up, blinks on pay out
	m_leds[6] = BIT(~data, 3);

	funcube_debug_outputs();
}

uint8_t funcube_state::battery_r()
{
	return m_in_battery->read() ? 0x40 : 0x00;
}


/***************************************************************************

                                Input Ports

***************************************************************************/

/***************************************************************************
                        Mobile Suit Gundam EX Revue
***************************************************************************/

static INPUT_PORTS_START( gundamex )
	PORT_START("DSW1")  // $600000.w
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0006, 0x0006, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW1:4" ) // Listed as "Unused"
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Freeze" ) PORT_DIPLOCATION("SW1:6")  // Listed as "Unused"
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Show Targets" ) PORT_DIPLOCATION("SW1:7") // Listed as "Unused"
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")  // $600002.w
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0000, "3 Coins/5 Credits" )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Debug Mode" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")    // $700000.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")    // $700002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    // $700004.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) //jumper pad
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Language ) )          //jumper pad
	PORT_DIPSETTING(      0x0020, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japanese ) )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")   // $700008.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")   // $70000a.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************
                                Guardians
***************************************************************************/

static INPUT_PORTS_START( grdians )
	PORT_START("DSW1")  // $600000.w
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy )    )  // 0
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal )  )  // 1
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard )    )  // 2
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )  // 3
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, "300000" )
	PORT_DIPSETTING(      0x0000, "500000" )
	PORT_DIPNAME( 0x0008, 0x0008, "Title" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, "Guardians" )
	PORT_DIPSETTING(      0x0000, "Denjin Makai II" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0020, "1" )
	PORT_DIPSETTING(      0x0030, "2" )
	PORT_DIPSETTING(      0x0010, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_SERVICE_DIPLOC(  0x0040, IP_ACTIVE_LOW, "SW1:7" ) // NOTE: Test mode shows player 3 & 4 controls, but it's a two player game
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")  // $600002.w
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x00f0, 0x00f0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(      0x0020, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x00f0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x00d0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x00b0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0090, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")    // $700000.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")    // $700002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    // $700004.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                      Wakakusamonogatari Mahjong Yonshimai
***************************************************************************/

static INPUT_PORTS_START( mj4simai )
	PORT_START("DSW1")  // $600300.w
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Tumo Pin" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")  // $600302.w
	PORT_DIPNAME( 0x0007, 0x0004, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0004, "0" )
	PORT_DIPSETTING(      0x0003, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0001, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPSETTING(      0x0007, "5" )
	PORT_DIPSETTING(      0x0006, "6" )
	PORT_DIPSETTING(      0x0005, "7" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0010, 0x0000, "Select Girl" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0020, 0x0000, "Com Put" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    // $600100.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_KEY0")   // $600000(0)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_KEY1")   // $600000(1)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("P1_KEY2")   // $600000(2)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_KEY3")   // $600000(3)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_KEY4")   // $600000(4)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2_KEY0")   // $600000(0)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2_KEY1")   // $600000(1)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("P2_KEY2")   // $600000(2)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2_KEY3")   // $600000(3)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2_KEY4")   // $600000(4)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                            Kosodate Quiz My Angel
***************************************************************************/

static INPUT_PORTS_START( myangel )
	PORT_START("DSW1")  // $700300.w
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0002, "SW1:2" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW1:3" ) // Listed as "Unused"
	PORT_DIPNAME( 0x0008, 0x0008, "Increase Lives While Playing" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0020, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0010, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")  // $700302.w
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" ) // Listed as "Unused"
	PORT_DIPNAME( 0x0080, 0x0080, "Push Start To Freeze (Cheat)") PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1") //$700000.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2") //$700002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM") //$700004.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                            Kosodate Quiz My Angel 2
***************************************************************************/

static INPUT_PORTS_START( myangel2 )
	PORT_START("DSW1") //$600300.w
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0002, "SW1:2" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW1:3" ) // Listed as "Unused"
	PORT_DIPNAME( 0x0008, 0x0008, "Increase Lives While Playing" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0020, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0010, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT(     0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2") //$600302.w
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" ) // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW2:8" ) // Listed as "Unused"
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1") //$600000.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2") //$600002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM") //$600004.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Puzzle De Bowling
***************************************************************************/

static INPUT_PORTS_START( pzlbowl )
	PORT_START("DSW1") //$400300.w
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0030, DEF_STR( Easiest ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( Easier ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Winning Rounds (Player VS Player)" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0040, "1" )
	PORT_DIPSETTING(      0x00c0, "2" )     // This setting is not defined in the manual
	PORT_DIPSETTING(      0x0080, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2") //$400302.w
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_2C ) )
//  PORT_DIPSETTING(      0x0002, DEF_STR( 1C_1C ) )        // This setting is not defined in the manual
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )
//  PORT_DIPSETTING(      0x0001, DEF_STR( 1C_3C ) )        // This setting is not defined in the manual
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Join In" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Japanese ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1") //$500000.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2") //$500002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM") //$500004.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)   // unused, test mode shows it
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM )    // Protection?
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                            Penguin Bros
***************************************************************************/

static INPUT_PORTS_START( penbros )
	PORT_START("DSW1") //$500300.w
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW1:3" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0020, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0080, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_2C ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2") //$500302.w
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x000c, "3" )
	PORT_DIPSETTING(      0x0004, "4" )
	PORT_DIPSETTING(      0x0008, "5" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0010, "150k and 500k" )
	PORT_DIPSETTING(      0x0030, "200k and 700k" )
	PORT_DIPSETTING(      0x0000, "Every 250k" )    // no extra life after the one at 1500k
	PORT_DIPSETTING(      0x0020, DEF_STR( None ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Winning Rounds (Player VS Player)" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x00c0, "2" )
	PORT_DIPSETTING(      0x0040, "3" )
	PORT_DIPSETTING(      0x0080, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1") //$600000.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Player 1 button 3 is unused
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2") //$600002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Player 2 button 3 is unused
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM") //$600004.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)   // unused, test mode shows it
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                              Reel'N Quake
***************************************************************************/

static INPUT_PORTS_START( reelquak )
	PORT_START("DSW1")  // $400300.w
	PORT_DIPNAME( 0x0001, 0x0001, "Game Style" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Standard ) )
	PORT_DIPSETTING(      0x0000, "Redemption" )
	PORT_DIPNAME( 0x000e, 0x000e, "Key-In Credits" ) PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x000c, "1 Turn / 2 Credits" )
	PORT_DIPSETTING(      0x000a, "1 Turn / 3 Credits" )
	PORT_DIPSETTING(      0x0008, "1 Turn / 5 Credits" )
	PORT_DIPSETTING(      0x000e, "1 Turn / 10 Credits" )
	PORT_DIPSETTING(      0x0006, "1 Turn / 20 Credits" )
	PORT_DIPSETTING(      0x0004, "1 Turn / 25 Credits" )
	PORT_DIPSETTING(      0x0002, "1 Turn / 50 Credits" )
	PORT_DIPSETTING(      0x0000, "1 Turn / 100 Credits" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:7,8")   // bit 7 tested according to game style
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, "1 Coin/10 Credits" )

	PORT_START("DSW2")  // $400302.w
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1")  // used
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P1")    // $400001.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_POKER_CANCEL  )                    // bet cancel
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE   )                    // collect
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP   )                    // double up
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH   ) PORT_NAME("Big")   // big
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_BET    )                    // bet
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_GAMBLE_LOW    ) PORT_NAME("Small") // small
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START1        )                    // start
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN       )

	PORT_START("TICKET")    // $400003.b
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM       ) PORT_READ_LINE_DEVICE_MEMBER("dispenser", ticket_dispenser_device, line_r)    // ticket sensor
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Knock Down")    // knock down
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE2      ) PORT_NAME("Ticket Clear")  // ticket clear
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE3      ) PORT_NAME("Ticket Resume") // ticket resume
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN  )                            // key in
	PORT_SERVICE_NO_TOGGLE(0x0080, IP_ACTIVE_LOW       )                            // test mode

	PORT_START("SYSTEM")    // $400005.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1         ) PORT_IMPULSE(5)    // coin a
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2         ) PORT_IMPULSE(5)    // coin b
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1      )                    // service
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK   )                    // diagnostic
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN       )
INPUT_PORTS_END


/***************************************************************************
                              Endless Riches
***************************************************************************/

static INPUT_PORTS_START( endrichs )
	PORT_INCLUDE(reelquak)

	PORT_MODIFY("DSW1")  // $400300.w
	PORT_DIPNAME( 0x0001, 0x0001, "Payout Style" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, "Normal Payout" )
	PORT_DIPSETTING(      0x0000, "Ticket Payout" ) // Ticket Printer?

	PORT_MODIFY("DSW2")  // $400302.w
	PORT_DIPUNUSED_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SW2:1" ) // DSW2 unpopulated
	PORT_DIPUNUSED_DIPLOC( 0x0002, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

/***************************************************************************
                              Star Audition
***************************************************************************/

// To activate ROM HACK items, use the debugger memory viewer:
// patch offsets 1E60,1E62,1E64 of the ':maincpu' region with 4E71

static INPUT_PORTS_START( staraudi )
	PORT_START("DSW1")  // $700300.w
	PORT_DIPUNKNOWN_DIPLOC(0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0002, 0x0002, "Monitor Sync (ROM HACK)" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC(0x0004, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC(0x0008, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPNAME( 0x0010, 0x0010, "Show Camera Variables" ) PORT_DIPLOCATION("SW1:5")   // camera test in service mode
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "Parallel/Serial" ) PORT_DIPLOCATION("SW1:6") // activates parallel / serial reading (ERROR if not active)
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC(0x0040, IP_ACTIVE_LOW, "SW1:7" )
	PORT_SERVICE_DIPLOC(   0x0080, IP_ACTIVE_LOW, "SW1:8" ) // service mode
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")  // $700302.w
	PORT_DIPUNKNOWN_DIPLOC(0x0001, IP_ACTIVE_LOW, "SW2:1" ) // ?
	PORT_DIPUNKNOWN_DIPLOC(0x0002, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC(0x0004, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC(0x0008, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC(0x0010, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC(0x0020, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPNAME( 0x0040, 0x0040, "Show Game Variables (ROM HACK)" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "? (ROM HACK)" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1") // $700000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Camera Variables? (Cheat)")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Flip Screen / Monitor Sync (Cheat)")   // keep pressed during boot / press together with up
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Reset Monitor Sync (Cheat)")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2") // $700002.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("Slow Motion (Cheat)")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("Pause (Cheat)")    // something in monitor sync menu too
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) // unused?
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM") // $700004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1    ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Degauss")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 ) // service coin
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Reset")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE4 ) // unused?
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2   ) // something (flash activity)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START3   ) // unused?
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM   ) PORT_VBLANK("screen")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_START4   ) // unused?
INPUT_PORTS_END


/***************************************************************************
                            Sammy Outdoor Shooting
***************************************************************************/

static INPUT_PORTS_START( deerhunt )
	PORT_START("DSW1") // $400000.w
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0005, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0028, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Discount To Continue" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT(     0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2") // fffd0a.w
	PORT_DIPNAME( 0x0001, 0x0001, "Vert. Flip Screen" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Horiz. Flip Screen" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Blood Color" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, "Red" )
	PORT_DIPSETTING(      0x0000, "Yellow" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0080, 0x0080, "Gun Type" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, "Pump Action" )
	PORT_DIPSETTING(      0x0000, "Hand Gun" )
	PORT_BIT(     0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("GUN1") // $500000
	PORT_BIT( 0x00ff, 0x0080, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1, 0, 0) PORT_MINMAX(0x0025,0x00c5) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_BIT( 0xff00, 0x8000, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1, 0, 0) PORT_MINMAX(0x0800,0xf800) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUN2")  // $580000.b
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )  // P2 gun, read but not used

	PORT_START("TRIGGER")   // $700000
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_CUSTOM )  // trigger
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0xff3f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PUMP")  // $700003.b
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_CUSTOM )  // pump
	PORT_BIT( 0xffbf, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN")  // $700005.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS")   // $400002
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 )  // trigger
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 )  // pump
	PORT_BIT( 0xfffc, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( turkhunt )
	PORT_INCLUDE(deerhunt)

	PORT_MODIFY("DSW2") // fffd0a.w
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
INPUT_PORTS_END


static INPUT_PORTS_START( wschamp )
	PORT_INCLUDE(deerhunt)

	PORT_MODIFY("DSW1") // $400000.w
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(      0x0009, "4 Coins Start, 4 Coins Continue" )
	PORT_DIPSETTING(      0x0008, "4 Coins Start, 3 Coins Continue" )
	PORT_DIPSETTING(      0x0007, "4 Coins Start, 2 Coins Continue" )
	PORT_DIPSETTING(      0x0006, "4 Coins Start, 1 Coin Continue" )
	PORT_DIPSETTING(      0x000c, "3 Coins Start, 3 Coins Continue" )
	PORT_DIPSETTING(      0x000b, "3 Coins Start, 2 Coins Continue" )
	PORT_DIPSETTING(      0x000a, "3 Coins Start, 1 Coin Continue" )
	PORT_DIPSETTING(      0x000e, "2 Coins Start, 2 Coins Continue" )
	PORT_DIPSETTING(      0x000d, "2 Coins Start, 1 Coin Continue" )
	PORT_DIPSETTING(      0x000f, "1 Coin Start, 1 Coin Continue" )
	PORT_DIPSETTING(      0x0005, "1 Coin 2 Credits, 1 Credit Start & Continue" )
	PORT_DIPSETTING(      0x0004, "1 Coin 3 Credits, 1 Credit Start & Continue" )
	PORT_DIPSETTING(      0x0003, "1 Coin 4 Credits, 1 Credit Start & Continue" )
	PORT_DIPSETTING(      0x0002, "1 Coin 5 Credits, 1 Credit Start & Continue" )
	PORT_DIPSETTING(      0x0001, "1 Coin 6 Credits, 1 Credit Start & Continue" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW1:7" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT(     0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW2") // fffd0a.w
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, "2" )
	PORT_DIPSETTING(      0x0000, "3" )

	PORT_MODIFY("GUN2") // $580000
	PORT_BIT( 0x00ff, 0x0080, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1, 0, 0) PORT_MINMAX(0x0025,0x00c5) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_BIT( 0xff00, 0x8000, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1, 0, 0) PORT_MINMAX(0x0800,0xf800) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_MODIFY("TRIGGER")  // $700000
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM )  // trigger P2
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_CUSTOM )  // trigger P1
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0xff1f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("PUMP") // $700003.b
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM )  // pump P2
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_CUSTOM )  // pump P1
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0xff1f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("COIN") // $700005.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("BUTTONS")  // $400002
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 )  // trigger P1
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 )  // pump P1
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )  PORT_PLAYER(2)  // trigger P2
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )  PORT_PLAYER(2)  // pump P2
	PORT_BIT( 0xffcc, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( trophyh )
	PORT_INCLUDE(wschamp)

	PORT_MODIFY("DSW2") // fffd0a.w
	PORT_DIPNAME( 0x0020, 0x0020, "Blood Color" ) PORT_DIPLOCATION("SW2:6") // WSChamp doesn't use Blood Color, so add it back in
	PORT_DIPSETTING(      0x0020, "Red" )
	PORT_DIPSETTING(      0x0000, "Yellow" )
INPUT_PORTS_END

static INPUT_PORTS_START( trophyht )
	PORT_INCLUDE(wschamp)

	PORT_MODIFY("DSW2") // fffd0a.w
	PORT_DIPNAME( 0x0020, 0x0020, "Blood Color" ) PORT_DIPLOCATION("SW2:6") // WSChamp doesn't use Blood Color, so add it back in
	PORT_DIPSETTING(      0x0020, "Red" )
	PORT_DIPSETTING(      0x0000, "Yellow" )
	PORT_DIPNAME( 0x0080, 0x0000, "Gun Type (Leave on Hand Gun)" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, "Pump Action" )
	PORT_DIPSETTING(      0x0000, "Hand Gun" )

	PORT_MODIFY("TRIGGER")  // $700000
	PORT_BIT( 0xff3f, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PCB only allows for two 4 pin gun connections - trigger p1

	PORT_MODIFY("PUMP") // $700003.b
	PORT_BIT( 0xff3f, IP_ACTIVE_LOW, IPT_UNKNOWN ) // PCB only allows for two 4 pin gun connections - trigger p2

	PORT_MODIFY("BUTTONS")  // $400002
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 )  // trigger P1
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 )  PORT_PLAYER(2)  // trigger P2
	PORT_BIT( 0xfffc, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                            TelePachi Fever Lion
***************************************************************************/

static INPUT_PORTS_START( telpacfl )
	PORT_START("DSW1")  // $600001.b ($200020.b)
	PORT_DIPNAME( 0x0001, 0x0001, "Clear NVRAM" )          PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Use Medal Sensor" )     PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )     PORT_DIPLOCATION("SW1:3") // used
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )     PORT_DIPLOCATION("SW1:5") // read but unsed?
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )     PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Force Hopper?" )        PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Freeze Screen" )        PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW2")  // $600003.b ($200021.b)
	PORT_DIPNAME( 0x000f, 0x000f, "Bonus Multiplier? (Low Hex Digit)" ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x000f, "0" )
	PORT_DIPSETTING(      0x000e, "1" )
	PORT_DIPSETTING(      0x000d, "2" )
	PORT_DIPSETTING(      0x000c, "3" )
	PORT_DIPSETTING(      0x000b, "4" )
	PORT_DIPSETTING(      0x000a, "5" )
	PORT_DIPSETTING(      0x0009, "6" )
	PORT_DIPSETTING(      0x0008, "7" )
	PORT_DIPSETTING(      0x0007, "8" )
	PORT_DIPSETTING(      0x0006, "9" )
	PORT_DIPSETTING(      0x0005, "A" )
	PORT_DIPSETTING(      0x0004, "B" )
	PORT_DIPSETTING(      0x0003, "C" )
	PORT_DIPSETTING(      0x0002, "D" )
	PORT_DIPSETTING(      0x0001, "E" )
	PORT_DIPSETTING(      0x0000, "F" )
	PORT_DIPNAME( 0x0070, 0x0070, "Bonus Multiplier? (High Hex Digit)" ) PORT_DIPLOCATION("SW2:5,6,7")
	PORT_DIPSETTING(      0x0070, "0" )
	PORT_DIPSETTING(      0x0060, "1" )
	PORT_DIPSETTING(      0x0050, "2" )
	PORT_DIPSETTING(      0x0040, "3" )
	PORT_DIPSETTING(      0x0030, "4" )
	PORT_DIPSETTING(      0x0020, "5" )
	PORT_DIPSETTING(      0x0010, "6" )
	PORT_DIPSETTING(      0x0000, "7" )
	PORT_DIPNAME( 0x0080, 0x0080, "Use Bonus Multiplier?" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )

	PORT_START("COIN")    // $700000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // -
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER         ) // coin1 connection
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE2      ) PORT_NAME("Reset") // reset switch (clear errors, play sound in sound test)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH,IPT_OTHER         ) // empty switch (out of medals error when low i.e. メダル切れ)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER         ) // coin2 connection
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER         ) // coin3 connection
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER         ) // coin4 connection
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) // pay out switch

	PORT_START("P1")    // $700002.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1       ) PORT_NAME("Bet") // bet switch (converts credits into balls)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // -
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR   ) // door switch
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM       ) PORT_READ_LINE_DEVICE_MEMBER("dispenser", ticket_dispenser_device, line_r) // coin out switch (medals jam error when stuck i.e. メダルづまり)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // -
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // -
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // -
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH,IPT_BUTTON2       ) PORT_NAME("Stop") // stop switch (active high)

	PORT_START("SERVICE")    // $700004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1         ) PORT_IMPULSE(5) // coin in switch
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2         ) PORT_IMPULSE(5) // 100yen in switch
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1      ) // service switch (next item in service mode)
	PORT_SERVICE_NO_TOGGLE(0x0008, IP_ACTIVE_LOW       ) // test switch
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // -
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // -
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // -
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER         ) // (freezes the game if high, eventually triggering the watchdog)

	PORT_START("UNKNOWN")    // $700006.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // -
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // -
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // -
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // -
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // -
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // -
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // -
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // -

	PORT_START("KNOB")    // $fffd0a (parallel port read)
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_MINMAX(0,0xff) PORT_SENSITIVITY(15) PORT_KEYDELTA(15) PORT_CENTERDELTA(0) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT)
INPUT_PORTS_END


/***************************************************************************
                               Funcube series
***************************************************************************/

static INPUT_PORTS_START( funcube )
	PORT_START("SWITCH")    // c00030.l
	PORT_BIT(     0x01, IP_ACTIVE_LOW,  IPT_COIN1    ) PORT_IMPULSE(1)  // coin solenoid 1
	PORT_BIT(     0x02, IP_ACTIVE_HIGH, IPT_CUSTOM  )                  // coin solenoid 2
	PORT_BIT(     0x04, IP_ACTIVE_HIGH, IPT_CUSTOM  )                  // hopper sensor
	PORT_BIT(     0x08, IP_ACTIVE_LOW,  IPT_BUTTON2  )                  // game select
	PORT_BIT(     0x10, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT )
	PORT_BIT(     0x20, IP_ACTIVE_LOW,  IPT_SERVICE1 ) PORT_NAME( "Reset Key" )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW   )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW   )

	PORT_START("BATTERY")
	PORT_DIPNAME( 0x10, 0x10, "Battery" )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x10, DEF_STR( On ) )

	PORT_START("DEBUG")
	// 500002.w
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(2)

	// 500000.w
	PORT_DIPNAME(    0x00010000, 0x00000000, "Debug 0" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00010000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00020000, 0x00000000, "Debug 1" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00020000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00040000, 0x00000000, "Debug 2" )    // Touch-Screen
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00040000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00080000, 0x00000000, "Debug 3" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00080000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00100000, 0x00000000, "Debug 4" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00100000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00200000, 0x00000000, "Debug 5" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00200000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00400000, 0x00000000, "Debug 6" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00400000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00800000, 0x00000000, "Debug 7" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00800000, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************


                            Graphics Layouts


***************************************************************************/

static const gfx_layout tile_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(7*8, -8) },
	{ STEP8(0, 1) },
	{ STEP8(0, 8*8) },
	8*8*8
};


/*  Tiles are 8bpp, but the hardware is additionally able to discard
    some bitplanes and use the low 4 bits only, or the high 4 bits only */
static GFXDECODE_START( gfx_seta2 )
	GFXDECODE_ENTRY( "sprites", 0, tile_layout, 0, 0x8000/16 )   // 8bpp, but 4bpp color granularity
GFXDECODE_END

/***************************************************************************

                                Machine Drivers

***************************************************************************/

void seta2_state::seta2(machine_config &config)
{
	TMP68301(config, m_maincpu, XTAL(50'000'000)/3);   // Verified on some PCBs

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(0x200, 0x100);
	m_screen->set_visarea(0x00, 0x180-1, 0x00, 0xf0-1);
	m_screen->set_screen_update(FUNC(seta2_state::screen_update));
	m_screen->screen_vblank().set(FUNC(seta2_state::screen_vblank));
	m_screen->screen_vblank().append_inputline(m_maincpu, 0);
	m_screen->set_palette(m_palette);
	//m_screen->set_video_attributes(VIDEO_UPDATE_SCANLINE);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_seta2);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x8000+0xf0);    // extra 0xf0 because we might draw 256-color object with 16-color granularity

	// sound hardware
	SPEAKER(config, "mono").front_center();

	x1_010_device &x1snd(X1_010(config, "x1snd", XTAL(50'000'000)/3));   // Verified on some PCBs
	x1snd.add_route(ALL_OUTPUTS, "mono", 1.0);
	x1snd.set_addrmap(0, &seta2_state::x1_map);
}


/*
    P0-113A PCB has different sound/cpu input clock (32.53047MHz / 2, common input clock is 50MHz / 3)
    and/or some PCB variant has uses this input clock?
    reference:
    https://youtu.be/6f-znVzcrmg, https://youtu.be/zJi_d463UQE (gundamex)
    https://youtu.be/Ung9XeLisV0 (grdiansa)
*/
void seta2_state::seta2_32m(machine_config &config)
{
	m_maincpu->set_clock(XTAL(32'530'470)/2);
	subdevice<x1_010_device>("x1snd")->set_clock(XTAL(32'530'470)/2);
}


void seta2_state::gundamex(machine_config &config)
{
	seta2(config);
	seta2_32m(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &seta2_state::gundamex_map);

	downcast<tmp68301_device &>(*m_maincpu).parallel_r_cb().set(FUNC(seta2_state::gundamex_eeprom_r));
	downcast<tmp68301_device &>(*m_maincpu).parallel_w_cb().set(FUNC(seta2_state::gundamex_eeprom_w));

	EEPROM_93C46_16BIT(config, "eeprom");

	// video hardware
	m_screen->set_visarea(0x00, 0x180-1, 0x000, 0x0e0-1);
}

// run in P-FG01-1 PCB, uses common input clock for sound/cpu - 32.53047MHz XTAL not populated
// reference: https://youtu.be/qj-TyKyAAVY
void seta2_state::grdians(machine_config &config)
{
	seta2(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &seta2_state::grdians_map);

	// video hardware
	m_screen->set_visarea(0x00, 0x130-1, 0x00, 0xe8 -1);
}

// run in P0-113A PCB, different sound/cpu input clock compared to P-FG01-1 PCB, same as gundamex?
void seta2_state::grdiansa(machine_config &config)
{
	grdians(config);
	seta2_32m(config);
}


void mj4simai_state::mj4simai(machine_config &config)
{
	seta2(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mj4simai_state::mj4simai_map);
}


void seta2_state::myangel(machine_config &config)
{
	seta2(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &seta2_state::myangel_map);

	// video hardware
	m_screen->set_visarea(0, 0x178-1, 0x00, 0xf0-1);
}


void seta2_state::myangel2(machine_config &config)
{
	seta2(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &seta2_state::myangel2_map);

	// video hardware
	m_screen->set_visarea(0, 0x178-1, 0x00, 0xf0-1);
}


void seta2_state::pzlbowl(machine_config &config)
{
	seta2(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &seta2_state::pzlbowl_map);

	// video hardware
	m_screen->set_visarea(0x00, 0x180-1, 0x00, 0xf0-1);
}


void seta2_state::penbros(machine_config &config)
{
	seta2(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &seta2_state::penbros_map);

	// video hardware
	m_screen->set_visarea(0, 0x140-1, 0x00, 0xe0-1);
}

void seta2_state::ablastb(machine_config &config)
{
	penbros(config);
	M68000(config.replace(), m_maincpu, XTAL(16'000'000)); // TMP68HC000P-16
	m_maincpu->set_addrmap(AS_PROGRAM, &seta2_state::ablastb_map);
	m_maincpu->set_vblank_int("screen", FUNC(seta2_state::irq2_line_hold));
}

void seta2_state::reelquak(machine_config &config)
{
	seta2(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &seta2_state::reelquak_map);

	downcast<tmp68301_device &>(*m_maincpu).parallel_w_cb().set(FUNC(seta2_state::reelquak_leds_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	TICKET_DISPENSER(config, m_dispenser, attotime::from_msec(200));

	m_screen->set_visarea(0x00, 0x140-1, 0x000, 0x0f0-1);
}


void seta2_state::samshoot(machine_config &config)
{
	seta2(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &seta2_state::samshoot_map);
	m_maincpu->set_periodic_int(FUNC(seta2_state::irq2_line_hold), attotime::from_hz(60));

	downcast<tmp68301_device &>(*m_maincpu).parallel_w_cb().set_ioport("DSW2");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	m_screen->set_visarea(0x00, 0x140-1, 0x000, 0x0f0-1);
}


void staraudi_state::staraudi(machine_config &config)
{
	seta2(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &staraudi_state::staraudi_map);

	SHARP_LH28F016S_16BIT(config, m_flash);
	UPD4992(config, m_rtc, 32'768);

	// video hardware
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));  // not accurate
	m_screen->set_visarea(0x00, 0x140-1, 0x000, 0x0f0-1);

	m_gfxdecode->set_info(gfx_seta2);
}


void seta2_state::telpacfl(machine_config &config)
{
	seta2(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &seta2_state::telpacfl_map);

	downcast<tmp68301_device &>(*m_maincpu).parallel_r_cb().set_ioport("KNOB");

	EEPROM_93C46_16BIT(config, "eeprom"); // not hooked up, seems unused

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	HOPPER(config, m_dispenser, attotime::from_msec(200));

	// video hardware
	m_screen->set_visarea(0x0, 0x180-1, 0x00, 0xf0-1); // still off by 1 because of different CRTC regs?
}


/***************************************************************************
                               Funcube series
***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(funcube_state::funcube_interrupt)
{
	int scanline = param;

	if (scanline == 368)
		m_maincpu->set_input_line(1, HOLD_LINE);

	if (scanline == 0)
		m_maincpu->set_input_line(2, HOLD_LINE);
}

void funcube_state::machine_start()
{
	seta2_state::machine_start();
	save_item(NAME(m_coin_start_cycles));
	save_item(NAME(m_hopper_motor));
	save_item(NAME(m_outputs));
	save_item(NAME(m_funcube_leds));

}

void funcube_state::machine_reset()
{
	m_coin_start_cycles = 0;
	m_hopper_motor = 0;
	m_outputs = 0;
	m_funcube_leds = 0;
}

void funcube_state::funcube(machine_config &config)
{
	MCF5206E(config, m_maincpu, XTAL(25'447'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &funcube_state::funcube_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(funcube_state::funcube_interrupt), "screen", 0, 1);

	H83007(config, m_sub, FUNCUBE_SUB_CPU_CLOCK);
	m_sub->set_addrmap(AS_PROGRAM, &funcube_state::funcube_sub_map);
	m_sub->read_port4().set(FUNC(funcube_state::battery_r));
	m_sub->read_port7().set(FUNC(funcube_state::coins_r));
	m_sub->read_porta().set(FUNC(funcube_state::outputs_r));
	m_sub->write_porta().set(FUNC(funcube_state::outputs_w));
	m_sub->write_portb().set(FUNC(funcube_state::leds_w));

	MCF5206E_PERIPHERAL(config, "maincpu_onboard", 0, m_maincpu);

	FUNCUBE_TOUCHSCREEN(config, "touchscreen", 200).tx_cb().set(m_sub, FUNC(h8_device::sci_rx_w<1>));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));  // not accurate
	m_screen->set_size(0x200, 0x200);
	m_screen->set_visarea(0x0+1, 0x140-1+1, 0x00, 0xf0-1);
	m_screen->set_screen_update(FUNC(funcube_state::screen_update));
	m_screen->screen_vblank().set(FUNC(funcube_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_seta2);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x8000+0xf0);    // extra 0xf0 because we might draw 256-color object with 16-color granularity

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	OKIM9810(config, m_oki, XTAL(4'096'000));
	m_oki->add_route(0, "lspeaker", 0.80);
	m_oki->add_route(1, "rspeaker", 0.80);
}


void funcube_state::funcube2(machine_config &config)
{
	funcube(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &funcube_state::funcube2_map);

	m_sub->read_port4().set([]() -> u8 { return 0; }); // unused

	// video hardware
	m_screen->set_visarea(0x0, 0x140-1, 0x00, 0xf0-1);
}


void seta2_state::namcostr(machine_config &config)
{
	TMP68301(config, m_maincpu, XTAL(50'000'000)/3);   // !! TMP68301 !!
	m_maincpu->set_addrmap(AS_PROGRAM, &seta2_state::namcostr_map);
	// does this have a ticket dispenser?

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(0x200, 0x200);
	m_screen->set_visarea(0x40, 0x1c0-1, 0x00, 0xf0-1);
	m_screen->set_screen_update(FUNC(seta2_state::screen_update));
	m_screen->screen_vblank().set(FUNC(seta2_state::screen_vblank));
	m_screen->screen_vblank().append_inputline(m_maincpu, 0);
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_seta2);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x8000+0xf0);    // extra 0xf0 because we might draw 256-color object with 16-color granularity

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	OKIM9810(config, m_oki, XTAL(4'096'000));
	m_oki->add_route(0, "lspeaker", 0.80);
	m_oki->add_route(1, "rspeaker", 0.80);
}


/***************************************************************************

                                ROMs Loading

***************************************************************************/

/***************************************************************************

FUNCUBE
EVA2E PCB

It's the same PCB as Namco Stars (P0-140B). It's a lot more complicated to dump
than the others because there are several surface mounted flash ROMs spread across
multiple daughterboards instead of simple socketed 32M DIP42 mask roms all on one PCB.

***************************************************************************/

ROM_START( funcube )
	ROM_REGION( 0x80000, "maincpu", 0 ) // XCF5206 Code
	ROM_LOAD( "fcu1_prg0-f.u08", 0x00000, 0x80000, CRC(57f4f340) SHA1(436fc66409b254aba68ae33fc994bc270ce803a6) )

	ROM_REGION( 0x20000, "sub", 0 )     // H8/3007 Code
	ROM_LOAD( "fcu_0_iopr-0b.1b", 0x00000, 0x20000, CRC(87e3690f) SHA1(1b9dc573de31543884678df2dba2d6a74d6a2496) )

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD32_BYTE( "fcu1_obj-0a.u12", 0x000000, 0x200000, CRC(908b6baf) SHA1(cb5aa8c9b16abb17d8cc16d0d3b2f690a48ee503) )
	ROM_LOAD32_BYTE( "fcu1_obj-0a.u13", 0x000001, 0x200000, CRC(8c31ca21) SHA1(e497ab1d7d30b41928a0c3db1ea7c3420376ad8c) )
	ROM_LOAD32_BYTE( "fcu1_obj-0a.u14", 0x000002, 0x200000, CRC(4298d599) SHA1(d245206bc78de5f17da85ae6063b662cf9cf67aa) )
	ROM_LOAD32_BYTE( "fcu1_obj-0a.u15", 0x000003, 0x200000, CRC(0669c78e) SHA1(0158fc4f90efa12d795b97873b8646c352864c69) )

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "fcu1_snd-0a.u40", 0x000000, 0x200000, CRC(448539bc) SHA1(9e53bd5e29d1a88bf634e58bfeebccd3a1c2d866) )
ROM_END

/***************************************************************************

              FUNCUBE (BET) Series PCB for chapters 2 through 5

PCB Number: B0-006B (also known as EVA3_A system and is non-JAMMA)
+------------------------------------------------+
|+--+ S +---+ +---+                CN5           |
||  | W |   | |   |                          CN6?|
||  | 4 | U | | U |                              |
||  |   | 4 | | 4 |     +---+                CN2?|
||  |   | 2 | | 3 |     |DX |                    |
||  |   |   | |   |     |102|                    |
||C |   |   | |   |     +---+                    |
||N |   +---+ +---+                              |
||4 |                                            |
||  |      +----------+   M1                     |
||  |  M3  |          |                        C |
||  |      |   NEC    |   M1                   N |
||  |  M3  |  DX-101  |                        3 |
||  |      |          |                          |
||  |      |          |   50MHz                  |
|+--+      +----------+                          |
| PIC  25.447MHz         +-----------+           |
|  CN7                   |    U47    |           |
|                        +-----------+           |
|          +-----------+  +---+ +---+       D    |
|          |     U3    |  |OKI| |DX |       S    |
|    M2    +-----------+  |   | |102|       W    |
|                         +---+ +---+       1    |
|                 ispLSI2032                     |
|    M1                      +---+               |
|          +----------+      |IDT|           +--+|
|          |          |  C   |   |           |  ||
| C        | ColdFire |  N   +---+           |  ||
| N  M2    | XCF5206E |  8                   |  ||
| 1        |          |        +---+         |C ||
|          |          |        |H8 |         |N ||
|    M1    +----------+        +---+      D  |9 ||
|                         14.7456MHz      S  |  ||
|                            +-----------+W  |  ||
|            SW1      BAT1   |    U49    |2  +--+|
|                            +-----------+       |
+------------------------------------------------+

   CPU: ColdFire XCF5206EFT54 (160 Pin PQFP)
        Hitachi H8/3007 (64130007F20) used for touch screen I/O
 Video: NEC DX-101 (240 Pin PQFP)
        NEC DX-102 (52 Pin PQFP x2)
 Sound: OKI MSM9810B 8-Channel Mixing ADPCM Type Voice Synthesis LSI
   OSC: 50MHz, 25.447MHz & 14.7456MHz
 Other: Lattice ispLSI2032 - stamped "EVA3A"
        BAT1 - CR2032 3Volt

ColdFire XCF5206EFT54:
  68K/ColdFire V2 core family
  8K internal SRAM
  54MHz (max) Bus Frequency
  32bit External Bus Width
  2 UART Serial Interfaces
  2 Timer Channels

PIC - PIC12C508 MCU used for security
       Labeled FC21A for Funcube 2
       Labeled FC41A for Funcube 4

Ram M1 are Toshiba TC55257DFL-70L
Ram M2 are NEC D43001GU-70L
Ram M3 are ISSI IS62C1024L-70Q
IDT - IDT 7130 64-pin TQFP High-speed 1K x 8 Dual-Port Static RAM

CN1 - Unused 64 pin double row connecter
CN2?  2x2 connecter
CN3 - Unused 50 pin double row connecter
CN4 - 96 pin triple row connecter
CN5 - 2x3 pin connecter
CN6?  3x3 connecter
CN7 - Unused 20 pin connecter
CN8 - 8 pin single row connecter
CN9 - 40 pin double row connecter

DSW1 - 8 position dipswitch
DSW2 - 2 position dipswitch
SW1  - Pushbutton
SW4  - Single position slider switch

U3  - Is a 27C4002 EPROM
U49 - Is a 27C1001 EPROM
U42, U43 & U47 are mask ROMs read as 27C322

The same H8/3007 code "FC21 IOPR-0" at U49 is used for FUNCUBE 2,3,4 & 5

***************************************************************************/

ROM_START( funcube2 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // XCF5206 Code
	ROM_LOAD( "fc21_prg-0b.u3", 0x00000, 0x80000, CRC(add1c8a6) SHA1(bf91518da659098a4bad4e756533525fcc910570) )

	ROM_REGION( 0x20000, "sub", 0 )     // H8/3007 Code
	ROM_LOAD( "fc21_iopr-0.u49", 0x00000, 0x20000, CRC(314555ef) SHA1(b17e3926c8ef7f599856c198c330d2051aae13ad) )

	ROM_REGION( 0x300, "pic", 0 )       // PIC12C508? Code
	ROM_LOAD( "fc21a.u57", 0x000, 0x300, NO_DUMP )

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD32_WORD( "fc21_obj-0.u43", 0x000000, 0x400000, CRC(08cfe6d9) SHA1(d10f362dcde01f7a9855d8f76af3084b5dd1573a) )
	ROM_LOAD32_WORD( "fc21_obj-1.u42", 0x000002, 0x400000, CRC(4c1fbc20) SHA1(ff83691c19ce3600b31c494eaec26d2ac79e0028) )

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "fc21_voi0.u47", 0x000000, 0x200000, CRC(4a49370a) SHA1(ac10e2c25626965b49475767ef5a0ec3ba9a2d01) )
ROM_END

ROM_START( funcube3 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // XCF5206 Code
	ROM_LOAD( "fc31_prg-0a.u4", 0x00000, 0x80000, CRC(ed7d70dd) SHA1(4ebfca9e60ab5e8de22821f0475abf515c83ce53) )

	ROM_REGION( 0x20000, "sub", 0 )     // H8/3007 Code
	ROM_LOAD( "fc21_iopr-0.u49", 0x00000, 0x20000, CRC(314555ef) SHA1(b17e3926c8ef7f599856c198c330d2051aae13ad) )

	ROM_REGION( 0x400, "pic", 0 )       // PIC12C508? Code
	ROM_LOAD( "fc31a.u57", 0x000, 0x400, NO_DUMP )

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD32_WORD( "fc31_obj-0.u43", 0x000000, 0x400000, CRC(08c5eb6f) SHA1(016d8f3067db487ccd47188142743897c9722b1f) )
	ROM_LOAD32_WORD( "fc31_obj-1.u42", 0x000002, 0x400000, CRC(4dadc76e) SHA1(cf82296b38dc22a618fd178816316af05f2459b3) )

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "fc31_snd-0.u47", 0x000000, 0x200000, CRC(36b03769) SHA1(20e583359421e0933c781a487fe5f7220052a6d4) )
ROM_END

ROM_START( funcube4 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // XCF5206 Code
	ROM_LOAD( "fc41_prg-0.u3", 0x00000, 0x80000, CRC(ef870874) SHA1(dcb8dc3f780ca135df55e4b4f3c95620597ad28f) )

	ROM_REGION( 0x20000, "sub", 0 )     // H8/3007 Code
	ROM_LOAD( "fc21_iopr-0.u49", 0x00000, 0x20000, CRC(314555ef) SHA1(b17e3926c8ef7f599856c198c330d2051aae13ad) )

	ROM_REGION( 0x300, "pic", 0 )       // PIC12C508? Code
	ROM_LOAD( "fc41a", 0x000, 0x300, NO_DUMP )

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD32_WORD( "fc41_obj-0.u43", 0x000000, 0x400000, CRC(9ff029d5) SHA1(e057f4929aa745ecaf9d4ff7e39974c82e440146) )
	ROM_LOAD32_WORD( "fc41_obj-1.u42", 0x000002, 0x400000, CRC(5ab7b087) SHA1(c600158b2358cdf947357170044dda2deacd4f37) )

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "fc41_snd0.u47", 0x000000, 0x200000, CRC(e6f7d2bc) SHA1(638c73d439eaaff8097cb0aa2684f9f7111bcade) )
ROM_END

ROM_START( funcube5 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // XCF5206 Code
	ROM_LOAD( "fc51_prg-0.u4", 0x00000, 0x80000, CRC(4e34c2d8) SHA1(1ace4f6edab291e69e5c36b15193fba62f4a6773) )

	ROM_REGION( 0x20000, "sub", 0 )     // H8/3007 Code
	ROM_LOAD( "fc21_iopr-0.u49", 0x00000, 0x20000, CRC(314555ef) SHA1(b17e3926c8ef7f599856c198c330d2051aae13ad) )

	ROM_REGION( 0x300, "pic", 0 )       // PIC12C508? Code
	ROM_LOAD( "fc51a.u57", 0x000, 0x300, NO_DUMP )

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD32_WORD( "fc51_obj-0.u43", 0x000000, 0x400000, CRC(116624b3) SHA1(c0b3dbe0ea4a0808222616c3ef77b2d1194a970a) )
	ROM_LOAD32_WORD( "fc51_obj-1.u42", 0x000002, 0x400000, CRC(35c6ec61) SHA1(424c9b66a2cdd5217d8a577d0179d1228112ee5b) )

	ROM_REGION( 0x1000000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "fc51_snd-0.u47", 0x000000, 0x200000, CRC(2a504fe1) SHA1(911ad650bf48aa78d9cb3c64284aa526ceb519ba) )
ROM_END

void funcube_state::init_funcube()
{
	uint32_t *main_cpu = (uint32_t *) memregion("maincpu")->base();

	main_cpu[0x064/4] = 0x0000042a; // PIC protection?
}

void funcube_state::init_funcube2()
{
	uint32_t *main_cpu = (uint32_t *) memregion("maincpu")->base();

	main_cpu[0xa5c/4] = 0x4e713e3c;       // PIC protection?
	main_cpu[0xa74/4] = 0x4e713e3c;
	main_cpu[0xa8c/4] = 0x4e7141f9;

}

void funcube_state::init_funcube3()
{
	uint32_t *main_cpu = (uint32_t *) memregion("maincpu")->base();

	main_cpu[0x008bc/4] = 0x4a804e71;
	main_cpu[0x19f0c/4] = 0x4e714e71;
	main_cpu[0x19fb8/4] = 0x4e714e71;

}

/***************************************************************************

Guardians
Banpresto, 1995

   CPU: Toshiba TMP68301AF-16 (100 Pin PQFP)
 Video: NEC DX-101 (240 Pin PQFP, @ U10)
        NEC DX-102 (52 Pin PQFP x2, @ U28 & U45)
 Sound: X1-010 (Mitsubishi M60016 Gate Array, 80 Pin PQFP @ U26)
   OSC: 50MHz
 Other: 8 Position Dipswitch x 2
        GAL 16V8 at U38

Memory:
M1 are HM628128LFP-10L at U42 & U43
M2 is  W2465K-70LL at U27
M3 are LH5168D-10L at U8 & U9
M4 are CXK58257AM-10L at U6, U7, U13 & U14

PCB Number: P-FG01-1
+-----------------------------------------------------------+
|             +------+      U  U             CN4*           |
| VOL         |Seta  |   M  5  5            +--------------+|
|             |X1-010|   2  8  7    +-+  M  |      U16     ||
|             +------+      *  *    | |  1  +--------------+|
+-+                                 |U|     +--------------+|
  |  +-++-++-++-+                   |3|     |      U20     ||
+-+  | || || || |      M            |2|  M  +--------------+|
|    |U||U||U||U| M M  4            | |  1  +--------------+|
|J   |3||5||2||4| 3 3               +-+     |      U15     ||
|A   | || || || |      M                    +--------------+|
|M   +-++-++-++-+      4                    +--------------+|
|M                                          |      U19     ||
|A                                          +--------------+|
|                                           +--------------+|
|C                                          |      U18     ||
|o                           +----------+   +--------------+|
|n          +-------+        |          |   +--------------+|
|n          |Toshiba|        |   NEC    |   |      U22     ||
|e          |  TMP  |        |  DX-101  |   +--------------+|
|c          | 68301 |        |          |   +--------------+|
|t        U +-------+        |          |   |      U17     ||
|e        5                  +----------+   +--------------+|
|r        6                                 +--------------+|
|         *                                 |      U21     ||
+-+            +---+       U                +--------------+|
  |            |DX |       3  50MHz 32MHz*                  |
  |            |102|       8                                |
+-+            +---+                    M  M   +---+        |
|       SW1*   D D                      4  4   |DX |        |
|              S S                             |102|        |
|              W W                             +---+        |
|              2 1                                          |
+-----------------------------------------------------------+

U56 is unpopulated 93C45 EEPROM
SW1 is unpopulated Reset push button
CN4 - 96 pin connector (3 rows by 32 pins)
* Denotes not populated.

Notes:
      HSync: 15.23kHz
      VSync: 58.5Hz

NOTE:  There is known to exist an undumped version of Guardians on the
       P-FG01-1 PCB half as many but larger ROMs.

  The following sockets have double size ROMS:
   U4 & U5 - program ROMs
   U15, U16, U17 & U18 - graphics ROMS
  The following sockets are unpopulated:
   U2 & U3 - program ROMs
   U19, U20, U21 & U22 - graphics ROMS
**********************************************************


 P0-113A PCB with P1-106-1 & P1-107-1 daughtercards

 program ROMs verified to match the P-FG01-1 set

   CPU: Toshiba TMP68301AF-16 (100 Pin PQFP)
 Video: Allumer X1-020 (208 Pin PQFP)
 Sound: X1-010 (Mitsubishi M60016 Gate Array, 80 Pin PQFP @ U26)
   OSC: 50MHz, 32.530MHz
 Other: 8 Position Dipswitch x 2
        Reset Push Button at SW1
        93C46 EEPROM

Memory:
M1 are TC551001BFL-70L at U56 & U57
M2 is  CY7C185-25PC at U27
M3 are N341256P-20

PCB Number: P0-113A  BP49KA
+---------------------------------------------------------------+
|             +------+            +---+                         |
| VOL         |Seta  |          M |   |        +---------------+|
|             |X1-010|          2 | U |     M  |KA2-001-014 U19||
|             +------+            | 2 |     1  +---------------+|
+-+                               | 8 |        +---------------+|
  |  +-++-+   +-++-+              |   |        |KA2-001-013 U17||
+-+  | || |   | || |              +---+     M  +---------------+|
|    |U||U| M |U||U| M                      1  +---------------+|
|J   |3||5| 3 |2||4| 3                         |      U15      ||
|A   | || |   | || |                           +---------------+|
|M   +-++-+   +-++-+                           +---------------+|
|M                                             |KA2-001-011 U20||
|A                              +----------+   +---------------+|
|                               |          |   +---------------+|
|C                              | ALLUMER  |   |KA2-001-010 U18||
|o     +---+                    | X1-020   |   +---------------+|
|n     |   |  +-------+         |          |   +---------------+|
|n     | U |  |Toshiba|         | 9426HK003|   |       U16*    ||
|e C   | 7 |  |  TMP  |         +----------+   +---------------+|
|c N   | 7 |  | 68301 |                        +---------------+|
|t 2   |   |  +-------+                        |KA2-001-008 U23||
|e     +---+                                   +---------------+|
|r          93C46                              +---------------+|
|                                              |KA2-001-007 U22||
+-+                           50MHz 32.530MHz  +---------------+|
  |                                            +---------------+|
  |                    P P                     |      U21*     ||
+-+  C                 A A       M M           +---------------+|
|    N    DSW1         L L       3 3                            |
|    1    DSW2 SW1     2 1                                      |
+---------------------------------------------------------------+

U2 is KA2 001 001 EPROM
U4 is KA2 001 002 EPROM
U5 is KA2 001 003 EPROM
U3 is KA2 001 004 EPROM
U28 is KA2-001-015 mask ROM (silkscreened SOUND ROM)
U15 is socketted to receive P1-106-1 daughtercard
U77 is socketted to receive P1-107-1 daughtercard
CN2 - 5 Pin header
CN1 - 10 Pin header
PAL1 at U51 is KA-201  GAL16V8B
PAL2 at U52 is KA-102  GAL16V8B
* Denotes not populated.

Notes:
      HSync: 15.19kHz
      VSync: 58.27Hz

The daughtercards below are NOT to scale with the above main board.

P1-107-1  (additional RAM)
+-------------------------------+
| 74LS32             JP5 JP6 JP7|
| CXK58257AM-10L CXK58257AM-10L |
|   +-----------------------+   |
|   |U7 42 pin header to U77|   |
|   +-----------------------+   |
+-------------------------------+

JP5 - JP7 single wire connections for power


P1-106-1
+-------------------------------+
|  HD74HC373P       HD74HC373P  |
|   +-----------------------+   |
|   |U3 42 pin header to U15|   |
|   +-----------------------+   |
|   +-----------------------+   |
|   |    KA2-001-017  U2    |   |
|   +-----------------------+   |
|   +-----------------------+   |
|   |    KA2-001-016  U1    |   |
|   +-----------------------+   |
|JP1 JP2 JP3 JP4        74LS00  |
+-------------------------------+

JP1 - JP4 single wire connections for power

***************************************************************************/

ROM_START( grdians ) // P-FG01-1 PCB
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "u2.bin", 0x000000, 0x080000, CRC(36adc6f2) SHA1(544e87f88179fe1342e7a06a8948ac1828e85108) )
	ROM_LOAD16_BYTE( "u3.bin", 0x000001, 0x080000, CRC(2704f416) SHA1(9081a12cbb9927d36e1c50b52aa2c6003810ee42) )
	ROM_LOAD16_BYTE( "u4.bin", 0x100000, 0x080000, CRC(bb52447b) SHA1(61433f683210ab2bc2cf1cc4b5b7a39cc5b6493d) )
	ROM_LOAD16_BYTE( "u5.bin", 0x100001, 0x080000, CRC(9c164a3b) SHA1(6d688c7af9e7e8e8d54b2e4dfbf41f59c79242eb) )

	ROM_REGION( 0x2000000, "sprites", ROMREGION_ERASE)  // Sprites
	ROM_LOAD64_WORD( "u16.bin",  0x0000000, 0x400000, CRC(6a65f265) SHA1(6cad11f718f8bbcff464d41eb4717460769237ed) )
	ROM_LOAD64_WORD( "u15.bin",  0x0000002, 0x400000, CRC(01672dcd) SHA1(f61f60e3343cc5b6ccee391ee529966a141566db) )
	ROM_LOAD64_WORD( "u18.bin",  0x0000004, 0x400000, CRC(967babf4) SHA1(42a6311576417c44aeaceb8ba6bb3cd7794e4882) )
	ROM_LOAD64_WORD( "u17.bin",  0x0000006, 0x400000, CRC(0fad0629) SHA1(1bdc8e7c5e39e83d327f14a672ec81b049112da6) )
	ROM_LOAD64_WORD( "u20.bin",  0x1800000, 0x200000, CRC(a7226ab7) SHA1(408580dd35c568ffef1ebbd87359e3ec1f867020) )
	ROM_CONTINUE(                0x1000000, 0x200000 )
	ROM_LOAD64_WORD( "u19.bin",  0x1800002, 0x200000, CRC(c0c998a0) SHA1(498fb1877527ed37412537f06a2c39ff0c60f146) )
	ROM_CONTINUE(                0x1000002, 0x200000 )
	ROM_LOAD64_WORD( "u22.bin",  0x1800004, 0x200000, CRC(6239997a) SHA1(87b6d6f30f152f625f82fd858c1290176c7e156e) )
	ROM_CONTINUE(                0x1000004, 0x200000 )
	ROM_LOAD64_WORD( "u21.bin",  0x1800006, 0x200000, CRC(6f95e466) SHA1(28482fad16a3ac9302f152d81552e6f84a44f3e4) )
	ROM_CONTINUE(                0x1000006, 0x200000 )

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "u32.bin", 0x000000, 0x100000, CRC(cf0f3017) SHA1(8376d3a674f71aec72f52c72758fbc53d9feb1a1) )
ROM_END

ROM_START( grdiansa ) // P0-113A PCB
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "ka2_001_001.u2", 0x000000, 0x080000, CRC(36adc6f2) SHA1(544e87f88179fe1342e7a06a8948ac1828e85108) ) // same program code as P-FG01-1 PCB above
	ROM_LOAD16_BYTE( "ka2_001_004.u3", 0x000001, 0x080000, CRC(2704f416) SHA1(9081a12cbb9927d36e1c50b52aa2c6003810ee42) )
	ROM_LOAD16_BYTE( "ka2_001_002.u4", 0x100000, 0x080000, CRC(bb52447b) SHA1(61433f683210ab2bc2cf1cc4b5b7a39cc5b6493d) )
	ROM_LOAD16_BYTE( "ka2_001_003.u5", 0x100001, 0x080000, CRC(9c164a3b) SHA1(6d688c7af9e7e8e8d54b2e4dfbf41f59c79242eb) )

	ROM_REGION( 0x2000000, "sprites", ROMREGION_ERASE)  // Sprites
	ROM_LOAD64_WORD( "ka2-001-010.u18",  0x0800000, 0x200000, CRC(b3e6e95f) SHA1(c61d3def136f4bb6c5857740b0fbea64a98dd1dc) )
	ROM_LOAD64_WORD( "ka2-001-013.u17",  0x0800002, 0x200000, CRC(9f7feb13) SHA1(0b3010faf87fb5bfe55101e5eabecec6107bf42f) )
	ROM_LOAD64_WORD( "ka2-001-007.u22",  0x0800004, 0x200000, CRC(d1035051) SHA1(0bc8871b91e777009002e340e1cef92487234271) )
	ROM_LOAD64_WORD( "ka2-001-016.u1",   0x0800006, 0x200000, CRC(99fc8efa) SHA1(eeaabb3b8d6c99a16c922a2b6ff0973935fd74bd) ) // located on P1-106-1 daughtercard
	ROM_LOAD64_WORD( "ka2-001-011.u20",  0x1000000, 0x200000, CRC(676edca6) SHA1(32bb507d000e19b004251d24c5fe61a09486cdd1) )
	ROM_LOAD64_WORD( "ka2-001-014.u19",  0x1000002, 0x200000, CRC(5465ef1b) SHA1(d1f0ff1950672444ece2fd86285a4051ea66f7bb) )
	ROM_LOAD64_WORD( "ka2-001-008.u23",  0x1000004, 0x200000, CRC(b2c94f31) SHA1(09891516806e2e79673b8b787d8e1caa51523a79) )
	ROM_LOAD64_WORD( "ka2-001-017.u2",   0x1000006, 0x200000, CRC(60ad7a2b) SHA1(a23c916959f3cfc8b1eead7a72c8312967b3acd7) ) // located on P1-106-1 daughtercard

	ROM_REGION( 0x200000, "x1snd", 0 )  // Samples
	ROM_LOAD( "ka2-001-015.u28", 0x000000, 0x200000, CRC(fa97cc54) SHA1(d9a869e9428e5f31aee917ea7733cca1247458f2) ) // Identical halves matching parent U32.BIN
ROM_END

ROM_START( grdiansbl ) // bootleg PCB based on the P-FG01-1 PCB, still has the X1-010, DX-101 and DX-102 customs. Pressing start in-game changes character.
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_WORD_SWAP( "p1.u4", 0x000000, 0x200000, CRC(4ba24d02) SHA1(97a7f36de772f005c8f377b1fb72fe4a57204158) ) // read as 27C160

	ROM_REGION( 0x2000000, "sprites", ROMREGION_ERASE)
	ROM_LOAD64_WORD( "u16.u16",  0x1000000, 0x200000, CRC(d24e007f) SHA1(fff8ca16f682a16094eb1e019f69025ce1992b44) )
	ROM_CONTINUE(                0x0800000, 0x200000 )
	ROM_LOAD64_WORD( "u15.u15",  0x1000002, 0x200000, CRC(2a92b8de) SHA1(ec723bf5c25cea57d146386d3d04a67bfd1e67d2) )
	ROM_CONTINUE(                0x0800002, 0x200000 )
	ROM_LOAD64_WORD( "u18.u18",  0x1000004, 0x200000, CRC(a3d0ba96) SHA1(164326662fe841039f3e6acebbe148cf3c048dd0) )
	ROM_CONTINUE(                0x0800004, 0x200000 )
	ROM_LOAD64_WORD( "u17.u17",  0x1000006, 0x200000, CRC(020ee44f) SHA1(dff65093b4789a28a391b0654ca46b6c328ed97b) )
	ROM_CONTINUE(                0x0800006, 0x200000 )

	ROM_REGION( 0x200000, "x1snd", 0 )  // Samples
	ROM_LOAD( "u32.u32", 0x000000, 0x200000, CRC(fa97cc54) SHA1(d9a869e9428e5f31aee917ea7733cca1247458f2) ) // 1ST AND 2ND HALF IDENTICAL, read as 27C160

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "ke-001.u38", 0x000, 0x117, NO_DUMP )
ROM_END

/***************************************************************************

MS Gundam Ex Revue
Banpresto, 1994

This game runs on Seta/Allumer hardware

   CPU: Toshiba TMP68301AF-16 (100 Pin PQFP)
 Video: Allumer X1-020 (208 Pin PQFP)
 Sound: X1-010 (Mitsubishi M60016 Gate Array, 80 Pin PQFP @ U26)
   OSC: 50MHz, 32.530MHz
 Other: 8 Position Dipswitch x 2
        Reset Push Button at SW1
        93C46 EEPROM

Memory:
M1 are TC551001BFL-70L at U56 & U57
M2 is  CY7C185-25PC at U27
M3 are N341256P-20

PCB Number: P0-113A  BP49KA
+--------------------------------------------------------------+
|             +------+            +---+                        |
| VOL         |Seta  |          M |   |        +--------------+|
|             |X1-010|          2 | U |     M  |KA-001-014 U19||
|             +------+            | 2 |     1  +--------------+|
+-+                               | 8 |        +--------------+|
  |  +-++-+   +-++-+              |   |        |KA-001-013 U17||
+-+  | || |   | || |              +---+     M  +--------------+|
|    |U||U| M |U||U| M                      1  +--------------+|
|J   |3||5| 3 |2||4| 3                         |KA-001-012 U15||
|A   | || |   | || |                           +--------------+|
|M   +-++-+   +-++-+                           +--------------+|
|M                                             |KA-001-011 U20||
|A                              +----------+   +--------------+|
|                               |          |   +--------------+|
|C                              | ALLUMER  |   |KA-001-010 U18||
|o     +---+                    | X1-020   |   +--------------+|
|n     |   |  +-------+         |          |   +--------------+|
|n     | U |  |Toshiba|         | 9426HK003|   |KA-001-009 U16||
|e C   | 7 |  |  TMP  |         +----------+   +--------------+|
|c N   | 7 |  | 68301 |                        +--------------+|
|t 2   |   |  +-------+                        |KA-001-008 U23||
|e     +---+                                   +--------------+|
|r          93C46                              +--------------+|
|                                              |KA-001-007 U22||
+-+                           50MHz 32.530MHz  +--------------+|
  |                                            +--------------+|
  |                    P P                     |KA-001-006 U21||
+-+  C                 A A       M M           +--------------+|
|    N    DSW1         L L       3 3                           |
|    1    DSW2 SW1                                             |
+--------------------------------------------------------------+

U28 is KA-001-015 mask ROM (silkscreened SOUND ROM)
U77 is KA-001-005 mask ROM
CN2 - 5 Pin header
CN1 - 10 Pin header

      VSync: 60Hz

***************************************************************************/

ROM_START( gundamex )
	ROM_REGION( 0x600000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE(      "ka_002_002.u2",  0x000000, 0x080000, CRC(e850f6d8) SHA1(026325e305676b1f8d3d9e7573920f8b70d7bccb) )
	ROM_LOAD16_BYTE(      "ka_002_004.u3",  0x000001, 0x080000, CRC(c0fb1208) SHA1(84b25e4c73cb8e023ee5dbf69f588be98700b43f) )
	ROM_LOAD16_BYTE(      "ka_002_001.u4",  0x100000, 0x080000, CRC(553ebe6b) SHA1(7fb8a159513d31a1d60520ff14e4c4d133fd3e19) )
	ROM_LOAD16_BYTE(      "ka_002_003.u5",  0x100001, 0x080000, CRC(946185aa) SHA1(524911c4c510d6c3e17a7ab42c7077c2fffbf06b) )
	ROM_LOAD16_WORD_SWAP( "ka-001-005.u77", 0x500000, 0x080000, CRC(f01d3d00) SHA1(ff12834e99a76261d619f10d186f4b329fb9cb7a) )

	ROM_REGION( 0x2000000, "sprites", ROMREGION_ERASE00)  // Sprites
	ROM_LOAD64_WORD( "ka-001-009.u16",  0x0000000, 0x200000, CRC(997d8d93) SHA1(4cb4cdb7e8208af4b14483610d9d6aa5e13acd89) )
	ROM_LOAD64_WORD( "ka-001-012.u15",  0x0000002, 0x200000, CRC(b789e4a8) SHA1(400b773f24d677a9d47466fdbbe68cb6efc1ad37) )
	ROM_LOAD64_WORD( "ka-001-006.u21",  0x0000004, 0x200000, CRC(6aac2f2f) SHA1(fac5478ca2941a93c57f670a058ff626e537bcde) )
	ROM_LOAD64_WORD( "ka-001-010.u18",  0x0800000, 0x200000, CRC(811b67ca) SHA1(c8cfae6f54c76d63bd625ff011c872ffb75fd2e2) )
	ROM_LOAD64_WORD( "ka-001-013.u17",  0x0800002, 0x200000, CRC(d8a0201f) SHA1(fe8a2407c872adde8aec8e9340b00be4f00a2872) )
	ROM_LOAD64_WORD( "ka-001-007.u22",  0x0800004, 0x200000, CRC(588f9d63) SHA1(ed5148d09d02e3bc12c50c39c5c86e6356b2dd7a) )
	ROM_LOAD64_WORD( "ka-001-011.u20",  0x1000000, 0x200000, CRC(08a72700) SHA1(fb8003aa02dd249c30a757cb43b516260b41c1bf) )
	ROM_LOAD64_WORD( "ka-001-014.u19",  0x1000002, 0x200000, CRC(7635e026) SHA1(116a3daab14a17faca85c4a956b356aaf0fc2276) )
	ROM_LOAD64_WORD( "ka-001-008.u23",  0x1000004, 0x200000, CRC(db55a60a) SHA1(03d118c7284ca86219891c473e2a89489710ea27) )

	ROM_REGION( 0x200000, "x1snd", 0 )  // Samples
	ROM_LOAD( "ka-001-015.u28", 0x000000, 0x200000, CRC(ada2843b) SHA1(09d06026031bc7558da511c3c0e29187ea0a0099) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom.bin", 0x0000, 0x0080, CRC(80f8e248) SHA1(1a9787811e56d95f7acbedfb00225b6e7df265eb) )
ROM_END

/***************************************************************************

Wakakusamonogatari Mahjong Yonshimai (JPN Ver.)
(c)1996 Maboroshi Ware

Board:  P0-123A

CPU:    TMP68301 (68000 core)
OSC:    50.0000MHz
        32.5304MHz

Sound:  X1-010

***************************************************************************/

ROM_START( mj4simai )
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "ll.u2",       0x000000, 0x080000, CRC(7be9c781) SHA1(d29e579706d98909933f6bed2ee292c88ed10d2c) )
	ROM_LOAD16_BYTE( "lh1.u3",      0x000001, 0x080000, CRC(82aa3f72) SHA1(a93d5dc7cdf12f852a692759d91f6f2951b6b5b5) )
	ROM_LOAD16_BYTE( "hl.u4",       0x100000, 0x080000, CRC(226063b7) SHA1(1737baffc16ff7261f887911187ece96925fa6ff) )
	ROM_LOAD16_BYTE( "hh.u5",       0x100001, 0x080000, CRC(23aaf8df) SHA1(b3d678afce4ddef32e48d690c6d07b723dd0c28f) )

	ROM_REGION( 0x2000000, "sprites", ROMREGION_ERASE00 )   // Sprites
	ROM_LOAD64_WORD( "cha-03.u16",  0x0000000, 0x400000, CRC(d367429a) SHA1(b32c215ef85c3d0a4c5550cef4f5c4c0e7030b7c) )
	ROM_LOAD64_WORD( "cha-05.u15",  0x0000002, 0x400000, CRC(e94ec40a) SHA1(2685dbc5680b5f76688c6b4fbe40ae682c525bfe) )
	ROM_LOAD64_WORD( "cha-01.u21",  0x0000004, 0x400000, CRC(35f47b37) SHA1(4a8eb088890272f2a069e2c3f00fadf6421f7b0e) )
	ROM_LOAD64_WORD( "cha-04.u18",  0x1000000, 0x400000, CRC(7f2008c3) SHA1(e45d863540eb2381f5d7660d64cdfef87c890768) )
	ROM_LOAD64_WORD( "cha-06.u17",  0x1000002, 0x400000, CRC(5cb0b3a9) SHA1(92fb82d45b4c46326d5796981f812e20a8ddb4f2) )
	ROM_LOAD64_WORD( "cha-02.u22",  0x1000004, 0x400000, CRC(f6346860) SHA1(4eebd3fa315b97964fa39b88224f9de7622ba881) )

	ROM_REGION( 0x400000, "x1snd", 0 )  // Samples
	ROM_LOAD( "cha-07.u32",  0x000000, 0x400000, CRC(817519ee) SHA1(ed09740cdbf61a328f7b50eb569cf498fb749416) )
ROM_END

/***************************************************************************

Kosodate Quiz My Angel (JPN Ver.)
(c)1996 Namco

   CPU: Toshiba TMP68301AF-16 (100 Pin PQFP)
 Video: NEC DX-101 (240 Pin PQFP, @ U10)
        NEC DX-102 (52 Pin PQFP x3, @ U28, U30 & U45)
 Sound: X1-010 (Mitsubishi M60016 Gate Array, 80 Pin PQFP @ U26)
   OSC: 50MHz
 Other: 8 Position Dipswitch x 2
        Reset Push Button at SW1
        GAL 16V8 at U38

Memory:
M1 are HM628128LFP-10L at U42 & U43
M2 is  W2465K-70LL at U27
M3 are LH5168D-10L at U8 & U9 (unpopulated)
M4 are CXK58257AM-10L at U6, U7, U13 & U14

PCB Number:  namco KE / P0-125A
+-----------------------------------------------------------+
|             +------+      U  U             CN4*           |
| VOL         |Seta  |   M  5  5            +--------------+|
|             |X1-010|   2  8  7    +-+  M  | KQ1 CG0  U16 ||
|             +------+      *  *    | |  1  +--------------+|
+-+                                 |U|     +--------------+|
  |  +-++-++-++-+            BT1*   |3|     | KQ1 CG2  U20 ||
+-+  | || || || |      M            |2|  M  +--------------+|
|    |U||U||U||U| M M  4            | |  1  +--------------+|
|J   |3||5||2||4| 3 3               +-+     | KQ1 CG1  U15 ||
|A   | || || || | * *  M                    +--------------+|
|M   +-++-++-++-+      4                    +--------------+|
|M  C                                       | KQ1 CG3  U19 ||
|A  N                                       +--------------+|
|   1                                       +--------------+|
|C  *                                       | KQ1 CG4  U18 ||
|o                           +----------+   +--------------+|
|n  C       +-------+        |          |   +--------------+|
|n  N       |Toshiba|        |   NEC    |   | KQ1 CG6  U22 ||
|e  2       |  TMP  |        |  DX-101  |   +--------------+|
|c  *       | 68301 |        |          |   +--------------+|
|t        U +-------+        |          |   | KQ1 CG5  U17 ||
|e  C     5                  +----------+   +--------------+|
|r  N     6                                 +--------------+|
|   3     *                                 | KQ1 CG7  U21 ||
+-+ * +---+    +---+       U                +--------------+|
  |   |DX |  S |DX |       3  50MHz 32MHz*                  |
  |   |102|  W |102|       8                                |
+-+   +---+  1 +---+                    M  M   +---+        |
|              D D                      4  4   |DX |        |
|              S S                             |102|        |
|              W W                             +---+        |
|              2 1                                          |
+-----------------------------------------------------------+

U2 is KQ1 PRG E EPROM
U3 is KQ1 PRG O EPROM
U4 is KQ1 TBL E EPROM
U5 is KQ1 TBL O EPROM
U32 is KG SND mask ROM (silkscreened SOUND ROM)

CN1 unpopulated 7 pin header
CN2 unpopulated 5 pin header
CN3 unpopulated 10 pin header
BT1 is unpopulated battery
U56 is unpopulated 93C45 EEPROM
CN4 - 96 pin connector (3 rows by 32 pins)
* Denotes not populated.

***************************************************************************/

ROM_START( myangel )
	ROM_REGION( 0x200000, "maincpu", 0 )        // TMP68301 Code
	ROM_LOAD16_BYTE( "kq1-prge.u2", 0x000000, 0x080000, CRC(6137d4c0) SHA1(762341e11b56e4a7787a0662833b702b78aee0a9) )
	ROM_LOAD16_BYTE( "kq1-prgo.u3", 0x000001, 0x080000, CRC(4aad10d8) SHA1(a08e1c4f57c64be829e0807ae2791da947fd60aa) )
	ROM_LOAD16_BYTE( "kq1-tble.u4", 0x100000, 0x080000, CRC(e332a514) SHA1(dfd255239c80c48c9865e70681b9ddd175b8bf55) )
	ROM_LOAD16_BYTE( "kq1-tblo.u5", 0x100001, 0x080000, CRC(760cab15) SHA1(fa7ea85ec2ebfaab3111b8631ea6ea3d794d449c) )

	ROM_REGION( 0x1000000, "sprites", 0 )   // Sprites
	ROM_LOAD64_WORD( "kq1-cg2.u20", 0x000000, 0x200000, CRC(80b4e8de) SHA1(c8685c4f4e3c0415ce0ec88e0288835e504cab00) )
	ROM_LOAD64_WORD( "kq1-cg3.u19", 0x000002, 0x200000, CRC(9bdc35c9) SHA1(fd0a1eb3dd10705bce5462263667353632558b58) )
	ROM_LOAD64_WORD( "kq1-cg6.u22", 0x000004, 0x200000, CRC(b25acf12) SHA1(5cca35921f3b376c3cc36f5b009eb845db2e1897) )
	ROM_LOAD64_WORD( "kq1-cg7.u21", 0x000006, 0x200000, CRC(9f48382c) SHA1(80dfc33a55123b5d3cdb3ed97b43a527f0254d61) )
	ROM_LOAD64_WORD( "kq1-cg0.u16", 0x800000, 0x200000, CRC(f8ae9a05) SHA1(4f3b41386a48a1608aa96b911e6b74ca775260fb) )
	ROM_LOAD64_WORD( "kq1-cg1.u15", 0x800002, 0x200000, CRC(23bd7ea4) SHA1(e925bbadc33fc2586bb18283cf989ab35f28c1e9) )
	ROM_LOAD64_WORD( "kq1-cg4.u18", 0x800004, 0x200000, CRC(dca7f8f2) SHA1(20595c7940a28d01bdc6610b67aaaeac61ba92e2) )
	ROM_LOAD64_WORD( "kq1-cg5.u17", 0x800006, 0x200000, CRC(a4bc4516) SHA1(0eb11fa54d16bba1b96f9dd943a68949a3bb9a2f) )

	ROM_REGION( 0x200000, "x1snd", 0 )  // Samples
	ROM_LOAD( "kq1-snd.u32", 0x000000, 0x200000, CRC(8ca1b449) SHA1(f54096fb5400843af4879135c96760485b6cb319) )
ROM_END

/***************************************************************************

Kosodate Quiz My Angel 2 (JPN Ver.)
(c)1997 Namco

Board:  KL (Namco) ; P0-136A (Seta)

CPU:    TMP68301 (68000 core)
OSC:    50.0000MHz
        32.5304MHz

Sound:  X1-010

***************************************************************************/

ROM_START( myangel2 )
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "kqs1ezpr.u2", 0x000000, 0x080000, CRC(2469aac2) SHA1(7dade2de31252e305d24c659c4801dd4687ad1f6) )
	ROM_LOAD16_BYTE( "kqs1ozpr.u3", 0x000001, 0x080000, CRC(6336375c) SHA1(72089f77e94832e74e0512944acadeccd0dec8b0) )
	ROM_LOAD16_BYTE( "kqs1e-tb.u4", 0x100000, 0x080000, CRC(e759b4cc) SHA1(4f806a144a47935b2710f8af800ec0d771f12a18) )
	ROM_LOAD16_BYTE( "kqs1o-tb.u5", 0x100001, 0x080000, CRC(b6168737) SHA1(4c3de877c0c1dca1c43ac737a0bf231335237d3a) )

	ROM_REGION( 0x1800000, "sprites", 0 )   // Sprites
	ROM_LOAD64_WORD( "kqs1-cg4.u20", 0x0000000, 0x200000, CRC(d1802241) SHA1(52c45a13d46f7ee8043e85b99d07b1765ca93dcc) )
	ROM_LOAD64_WORD( "kqs1-cg5.u19", 0x0000002, 0x200000, CRC(d86cf19c) SHA1(da5a5b576ce107433605b24d8b9dcd0abd46bcde) )
	ROM_LOAD64_WORD( "kqs1-cg6.u22", 0x0000004, 0x200000, CRC(3f08886b) SHA1(054546ae44ffa5d0973f4ead080fe720a340e144) )
	ROM_LOAD64_WORD( "kqs1-cg7.u21", 0x0000006, 0x200000, CRC(2c977904) SHA1(2589447f2471cdc414266b34aff552044c680d93) )
	ROM_LOAD64_WORD( "kqs1-cg0.u16", 0x0800000, 0x400000, CRC(c21a33a7) SHA1(bc6f479a8f4c716ba79a725f160ddeb95fdedbcb) )
	ROM_LOAD64_WORD( "kqs1-cg1.u15", 0x0800002, 0x400000, CRC(dca799ba) SHA1(8379b11472c27b1945fe7fc274c7fedf756accba) )
	ROM_LOAD64_WORD( "kqs1-cg2.u18", 0x0800004, 0x400000, CRC(f7f92c7e) SHA1(24a525a15fded0de6e382b346da6bd5e7b9eced5) )
	ROM_LOAD64_WORD( "kqs1-cg3.u17", 0x0800006, 0x400000, CRC(de3b2191) SHA1(d7d6ea07b665cfd834747d3c0776b968ce03bc6a) )

	ROM_REGION( 0x400000, "x1snd", 0 )  // Samples
	ROM_LOAD( "kqs1-snd.u32", 0x000000, 0x400000, CRC(792a6b49) SHA1(341b4e8f248b5032217733bada32e353c67e3888) )
ROM_END

/***************************************************************************

  Namco Stars

  EVA2B PCB (8829970101 P0-140B Serial Z033):

  TMP68301AF-16 CPU
  DX101
  DX102 x 2
  OKI M9810
  GAL 16V8d x 2
  MAX232
  DSW8
  Coin Battery
  Reset Button
  OSC: 25.447 MHz @ XM1, 50.000 MHz @ XM2, 32.53005 MHz @ X2
  Volume Trimmer

***************************************************************************/

ROM_START( namcostr )
	ROM_REGION( 0x80000, "maincpu", 0 ) // TMP68301 Code
	ROM_LOAD( "ns1mpr0.u08", 0x00000, 0x80000, BAD_DUMP CRC(008d23fe) SHA1(8c77a34dd0285c06809e99d20b9d8b31b81bfc68) )  // FIXED BITS (xxxxx1xxxxxxxxxx)

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD32_WORD( "ns1cha0.u39", 0x000000, 0x400000, BAD_DUMP CRC(372d1651) SHA1(355553992e5a474ae1e45bcdeb88804d5b75f802) ) // FIXED BITS (xxxxx1xxxxxxxxxx)
	ROM_LOAD32_WORD( "ns1cha1.u38", 0x000002, 0x400000, BAD_DUMP CRC(82e67809) SHA1(6b25726cd3683e1691e4d4e1628c13998f20933d) ) // FIXED BITS (xxxxx1xxxxxxxxxx)

	ROM_REGION( 0x1000000, "oki", 0 )
	ROM_LOAD( "ns1voi0.u40", 0x000000, 0x400000, BAD_DUMP CRC(fe5c2b16) SHA1(21e4423cc91e8833297d4588343237b8b3155196) )    // FIXED BITS (xxxxx1xxxxxxxxxx)
ROM_END

void seta2_state::init_namcostr()
{
	// attempt to patch a few of the stuck bits
	uint16_t *cpurom = &memregion("maincpu")->as_u16(0);
	for (offs_t addr = 0x00000; addr < 0x00100; addr += 2)
		cpurom[addr / 2] &= 0xfbff;
	for (offs_t addr = 0x00100; addr < 0x00180; addr += 2)
		if (!BIT(cpurom[addr / 2], 9))
			cpurom[addr / 2] &= 0xfbff;
	cpurom[0x00184 / 2] &= 0xfbff;
	for (offs_t addr = 0x00204; addr < 0x002ae; addr += 2)
		cpurom[addr / 2] &= 0xfbff;
	for (offs_t addr = 0x002b0; addr < 0x0032c; addr += 2)
		cpurom[addr / 2] &= 0xfbff;
	cpurom[0x00332 / 2] &= 0xfbff;
	for (offs_t addr = 0x00336; addr < 0x00344; addr += 2)
		cpurom[addr / 2] &= 0xfbff;
	for (offs_t addr = 0x00348; addr < 0x00364; addr += 2)
		cpurom[addr / 2] &= 0xfbff;
	for (offs_t addr = 0x00368; addr < 0x00370; addr += 2)
		cpurom[addr / 2] &= 0xfbff;
	for (offs_t addr = 0x00374; addr < 0x003ae; addr += 2)
		cpurom[addr / 2] &= 0xfbff;
	for (offs_t addr = 0x003b0; addr < 0x00400; addr += 2)
		cpurom[addr / 2] &= 0xfbff;
}

/***************************************************************************

                            Puzzle De Bowling (Japan)

(c)1999 MOSS / Nihon System

   CPU: Toshiba TMP68301AF-16 (100 Pin PQFP)
 Video: NEC DX-101 (240 Pin PQFP)
        NEC DX-102 (52 Pin PQFP x3)
 Sound: X1-010 (Mitsubishi M60016 Gate Array, 80 Pin PQFP)
   OSC: 50MHz & 32.53047MHz
 Other: 8 Position Dipswitch x 2
        Reset Push Button at SW1
        Lattice ispLSI2032 - stamped "KUDEC"

PCB Number: P0-142A
+-----------------------------------------------------------+
|                VOL                       +------+         |
|                                          |Seta  |     M1  |
|   +---+ +---+                            |X1-010|         |
|   |   | |   |  U4*  M                    |      |  +---+  |
+-+ | U | | U |       1                    +------+  | K |  |
  | | 0 | | 0 |                     U30*             | U |  |
+-+ | 7 | | 6 |  U5*  M                              | S |  |
|   |   | |   |       1                              |   |  |
|   +---+ +---+                                      | U |  |
|                                          Lattice   | 1 |  |
|J  D D  +---+                            ispLSI2032 | 8 |  |
|A  S S  |DX |                  +-------+            +---+  |
|M  W W  |102|                  |Toshiba|     CN2           |
|M  1 2  +---+      BAT1*       |  TMP  |                   |
|A                              | 68301 |  U50*             |
|                               +-------+                   |
|C                                                          |
|o                 50MHz        +----------+     XM2*       |
|n    +---+                     |          |                |
|n    |DX |     SW1             |   NEC    |    M   M       |
|e    |102|                     |  DX-101  |    2   2       |
|c    +---+         M  M        |          |                |
|t                  1  1        |          |                |
|e                              +----------+                |
|r                                                          |
|                             +---+      +---++---++---+    |
|                             | K |      | K || K || K |    |
|     +---+                   | U |      | U || U || U |    |
+-+   |DX |                   | C |      | C || C || C |    |
  |   |102|     32.53047MHz   |   |      |   ||   ||   |    |
+-+   +---+                   | U |      | U || U || U |    |
|                             | 4 |      | 4 || 3 || 3 |    |
|                             | 0 |      | 1 || 8 || 9 |    |
|                             +---+      +---++---++---+    |
+-----------------------------------------------------------+

* Unpopulated:
  U4 & U5 RAM HM62256 equivalent
  U50 93LC46BX EEPROM
  U30 74HC00
  BAT1 CR2032 3Volt battery
  XM2 OSC

Ram M1 are NEC D43001GU-70LL
Ram M2 are LGS GM76C8128ALLFW70

KUP U06 I03 U06 Program rom ST27C4001 (even)
KUP U07 I03 U07 Program rom ST27C4001 (odd)

KUS-U18-I00 U18 Mask rom (Samples 23C32000 32Mbit)

KUC-U38-I00 U38 Mask rom (Graphics 23C32000 32Mbit)
KUC-U39-I00 U39 Mask rom (Graphics 23C32000 32Mbit)
KUC-U40-I00 U40 Mask rom (Graphics 23C32000 32Mbit)
KUC-U41-I00 U41 Mask rom (Graphics 23C32000 32Mbit)

***************************************************************************/

ROM_START( pzlbowl )
	ROM_REGION( 0x100000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "kup_u06_i03.u6", 0x000000, 0x080000, CRC(314e03ac) SHA1(999398e55161dd75570d418f4c9899e3bf311cc8) )
	ROM_LOAD16_BYTE( "kup_u07_i03.u7", 0x000001, 0x080000, CRC(a0423a04) SHA1(9539023c5c2f2bf72ee3fb6105443ffd3d61e2f8) )

	ROM_REGION( 0x1000000, "sprites", 0 )   // Sprites
	ROM_LOAD64_WORD( "kuc-u38-i00.u38", 0x000000, 0x400000, CRC(3db24172) SHA1(89c39963e15c53b799994185d0c8b2e795478939) )
	ROM_LOAD64_WORD( "kuc-u39-i00.u39", 0x000002, 0x400000, CRC(9b26619b) SHA1(ea7a0bf46641d15353217b01e761d1a148bee4e7) )
	ROM_LOAD64_WORD( "kuc-u40-i00.u40", 0x000004, 0x400000, CRC(7e49a2cf) SHA1(d24683addbc54515c33fb620ac500e6702bd9e17) )
	ROM_LOAD64_WORD( "kuc-u41-i00.u41", 0x000006, 0x400000, CRC(2febf19b) SHA1(8081ac590c0463529777b5e4817305a1a6f6ea41) )

	ROM_REGION( 0x400000, "x1snd", 0 )  // Samples
	ROM_LOAD( "kus-u18-i00.u18", 0x000000, 0x400000, CRC(e2b1dfcf) SHA1(fb0b8be119531a1a27efa46ed7b86b05a37ed585) )
ROM_END

/***************************************************************************

Penguin Brothers / 轟天雷 (A-Blast)
(c)2000 Subsino

   CPU: Toshiba TMP68301AF-16 (100 Pin PQFP)
 Video: NEC DX-101 (240 Pin PQFP)
        NEC DX-102 (52 Pin PQFP x3)
 Sound: X1-010 (Mitsubishi M60016 Gate Array, 80 Pin PQFP)
   OSC: 50MHz, 32.53047MHz & 28MHz
 Other: 8 Position Dipswitch x 2
        Reset Push Button at SW1
        Lattice ispLSI2032

PCB Number: P0-142A
+-----------------------------------------------------------+
|                VOL                       +------+         |
|                                          |Seta  |     M1  |
|   +---+ +---+                            |X1-010|         |
|   |   | |   |   M   M                    |      |  +---+  |
+-+ | U | | U |   1   1                    +------+  |   |  |
  | | 0 | | 0 |                    74HC00            |   |  |
+-+ | 7 | | 6 |   M   M                              | U |  |
|   |   | |   |   1   1                              | 1 |  |
|   +---+ +---+                                      | 8 |  |
|                                          Lattice   |   |  |
|J  D D  +---+                            ispLSI2032 |   |  |
|A  S S  |DX |                  +-------+            +---+  |
|M  W W  |102|                  |Toshiba|     CN2           |
|M  1 2  +---+      BAT1*       |  TMP  |                   |
|A                              | 68301 |  U50*             |
|                               +-------+                   |
|C                                                          |
|o                 50MHz        +----------+     28MHz      |
|n    +---+                     |          |                |
|n    |DX |     SW1             |   NEC    |    M   M       |
|e    |102|                     |  DX-101  |    2   2       |
|c    +---+         M  M        |          |                |
|t                  1  1        |          |                |
|e                              +----------+                |
|r                                                          |
|                             +---+      +---++---++---+    |
|                             |   |      |   ||   ||   |    |
|     +---+                   |   |      |   ||   ||   |    |
+-+   |DX |                   | U |      | U || U || U |    |
  |   |102|     32.53047MHz   | 4 |      | 4 || 3 || 3 |    |
+-+   +---+                   | 0 |      | 1 || 8 || 9 |    |
|                             |   |      |   ||   ||   |    |
|                             |   |      |   ||   ||   |    |
|                             +---+      +---++---++---+    |
+-----------------------------------------------------------+

Notes:  pzlbowl PCB with these extra parts:
        28MHz OSC
        2x 62256 SRAM
        74HC00

U50  Unpopulated 93LC46BX EEPROM
BAT1 Unpopulated CR2032 3 Volt battery
* Denotes not populated.

Ram M1 are NEC D43001GU-70LL
Ram M2 are LGS GM76C8128ALLFW70

Notes about sets:
penbros: Original Japanese version with Japan region warning, title screen and all game text
         in Japanese. However the Subsino logo is the wrong color
 ablast: Title screen is in traditional Chinese. ROM labels imply Taiwan with "TWN" printed
         on them. The region warning states Japan only & all game text is in Japanese. Lastly
         the Subsino logo has correct color. The bootleg is a copy of A-Blast.
***************************************************************************/

ROM_START( penbros ) // Genuine P0-142A PCB & original ROM labels
	ROM_REGION( 0x100000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "a-blast_jpn_u06.u06", 0x000000, 0x080000, CRC(7bbdffac) SHA1(d5766cb171b8d2e4c04a6bae37181fa5ada9d797) )
	ROM_LOAD16_BYTE( "a-blast_jpn_u07.u07", 0x000001, 0x080000, CRC(d50cda5f) SHA1(fc66f55f2070b447c5db85c948ce40adc37512f7) )

	ROM_REGION( 0x1000000, "sprites", ROMREGION_ERASE00 )   // Sprites
	ROM_LOAD64_WORD( "a-blast_jpn_u38.u38", 0x000000, 0x400000, CRC(4247b39e) SHA1(f273931293beced312e02c870bf35e9cf0c91a8b) )
	ROM_LOAD64_WORD( "a-blast_jpn_u39.u39", 0x000002, 0x400000, CRC(f9f07faf) SHA1(66fc4a9ad422fb384d2c775e43619137226898fc) )
	ROM_LOAD64_WORD( "a-blast_jpn_u40.u40", 0x000004, 0x400000, CRC(dc9e0a96) SHA1(c2c8ccf9039ee0e179b08fdd2d37f29899349cda) )
	// 6bpp instead of 8bpp

	ROM_REGION( 0x200000, "x1snd", 0 )  // Samples
	ROM_LOAD( "a-blast_jpn_u18.u18", 0x000000, 0x200000, CRC(de4e65e2) SHA1(82d4e590c714b3e9bf0ffaf1500deb24fd315595) )
ROM_END

ROM_START( ablast ) // Genuine P0-142A PCB & original ROM labels
	ROM_REGION( 0x100000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "a-blast_twn_u06.u06", 0x000000, 0x080000, CRC(e62156d7) SHA1(509fd41a0109dc5c00d83250383d578fd75502f3) )
	ROM_LOAD16_BYTE( "a-blast_twn_u07.u07", 0x000001, 0x080000, CRC(d4ddc16b) SHA1(63312ce9ec6dffb47aa6aed505f077f20713e5ac) )

	ROM_REGION( 0x1000000, "sprites", 0 )   // Sprites
	ROM_LOAD64_WORD( "a-blast_twn_u38.u38", 0x000000, 0x400000, CRC(090923da) SHA1(c1eaa8847fe183819af040d97d0e6d1cd9928991) )
	ROM_LOAD64_WORD( "a-blast_twn_u39.u39", 0x000002, 0x400000, CRC(6bb17d83) SHA1(b53d8cfc3833df937b92993f9eca17c805c5f58d) )
	ROM_LOAD64_WORD( "a-blast_twn_u40.u40", 0x000004, 0x400000, CRC(db94847d) SHA1(fd2e29a45bb0acbd9e709256c7fc27bdd64a6634) )
	// 6bpp instead of 8bpp

	ROM_REGION( 0x200000, "x1snd", 0 )  // Samples
	ROM_LOAD( "a-blast_twn_u18.u18", 0x000000, 0x200000, CRC(de4e65e2) SHA1(82d4e590c714b3e9bf0ffaf1500deb24fd315595) )
ROM_END

ROM_START( ablastb ) // bootleg PCB with standard 68000 instead of TMP68301 and 4 FPGAs (3 A40MX04 and 1 A54SX16A)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "1.bin", 0x000000, 0x100000, CRC(4adbd826) SHA1(004e3d0d5cb44c00283bc02f6d727e023690226d) )

	ROM_REGION( 0x1000000, "sprites", 0 )   // Sprites
	ROM_LOAD64_WORD( "2.bin", 0x000000, 0x400000, CRC(090923da) SHA1(c1eaa8847fe183819af040d97d0e6d1cd9928991) )
	ROM_LOAD64_WORD( "3.bin", 0x000002, 0x400000, CRC(6bb17d83) SHA1(b53d8cfc3833df937b92993f9eca17c805c5f58d) )
	ROM_LOAD64_WORD( "4.bin", 0x000004, 0x400000, CRC(db94847d) SHA1(fd2e29a45bb0acbd9e709256c7fc27bdd64a6634) )
	// 6bpp instead of 8bpp

	ROM_REGION( 0x200000, "x1snd", 0 )  // Samples. ROM content matches the penbros' one, but there's no proper X1-010 on the PCB. Possibly one of the FPGAs acts as a substitute?
	ROM_LOAD( "29f1610.bin", 0x000000, 0x200000, CRC(de4e65e2) SHA1(82d4e590c714b3e9bf0ffaf1500deb24fd315595) )
ROM_END

/***************************************************************************

Reel'N Quake!

   CPU: Toshiba TMP68301AF-16 (100 Pin PQFP)
 Video: NEC DX-101 (240 Pin PQFP, @ U10)
        NEC DX-102 (52 Pin PQFP x3, @ U28, U30 & U45)
 Sound: X1-010 (Mitsubishi M60016 Gate Array, 80 Pin PQFP @ U26)
   OSC: 50MHz & 28MHz
 Other: 8 Position Dipswitch x 2
        Reset Push Button at SW1
        3.6V Battery at BT1
        GAL 16V8 - labeled "KF-001" at U38

Memory:
M1 are TC551001BFL-70L at U42 & U43
M2 is  W2465K-70LL at U27
M3 are LH5168D-10L at U8 & U9
M4 are UT62256SC-70L at U6, U7, U13 & U14

PCB Number: P-FG-02
+-----------------------------------------------------------+
|             +------+      U  U                            |
| VOL         |Seta  |   M  5  5            +--------------+|
|             |X1-010|   2  8  7    +-+  M  |KF-001-005 U16||
|             +------+      *  *    | |  1  +--------------+|
+-+                                 |U|                     |
  |  +-+    +-+           BT1       |3|            U20*     |
+-+  | |    | |         M           |2|  M                  |
|  C |U| U  |U| U  M M  4           | |  1  +--------------+|
|J N |3| 5  |2| 4  3 3              +-+     |KF-001-006 U15||
|A 1 | | *  | | *       M                   +--------------+|
|M   +-+    +-+         4                                   |
|M C                                               U19*     |
|A N                                                        |
|  2                                        +--------------+|
|C 1                                        |KF-001-007 U18||
|o                           +----------+   +--------------+|
|n C        +-------+        |          |                   |
|n N        |Toshiba|        |   NEC    |          U22*     |
|e 2        |  TMP  |        |  DX-101  |                   |
|c 2        | 68301 |        |          |   +--------------+|
|t        U +-------+        |          |   |KF-001-008 U17||
|e C      5                  +----------+   +--------------+|
|r N      6                                                 |
|  3      *                                        U21*     |
+-+   +---+    +---+       U  50MHz 32MHz*                  |
  |   |DX |  S |DX |       3                                |
  |   |102|  W |102|       8                   +---+   28MHz|
+-+   +---+  1 +---+                    M  M   |DX |        |
|              D D                      4  4   |102|        |
|              S S                             +---+        |
|              W W                                          |
|              2 1                                          |
+-----------------------------------------------------------+

CN1   - 7 Pin connector
CN2-1 - 3 Pin connector
CN2-2 - 3 Pin connector
CN3   - 10 Pin connector (used for extra buttons)

U56 is unpopulated 93C45 EEPROM
* Denotes not populated.

    U3-U5 silkscreened 27C4001
  U57-U58 silkscreened 23C8001E
  U15-U22 silkscreened 23C32000
      U32 silkscreened 23C32000

Note:
  The PCB is silkscreened with 23C32000 which would be equal to the 27C322.
  The graphics roms dumped that way have the first half as a bad mirror
  of the second half (even <- odd, odd <- FF). They seem OK dumped as 27C160.

Program ROMs:
  Labeled roms (undumped) have been seen as KF003002 & KF003004, which
  revision do have?

V1.05 program roms have been seen with labels dated 12/17/97

Reel'N Quake! is also known to be available on the P-FG-03 PCB which is
 essentially the same layout as the P-FG-02 but with standard 8-liner
 edge connectors. Program rom labels for this (undumped) set are:

   KFP       KFP
   U02  and  U03
   C00       C00

***************************************************************************/

ROM_START( reelquak )
	ROM_REGION( 0x100000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "rq_ver1.05.u2", 0x00000, 0x80000, CRC(7740d7a4) SHA1(21c28db5d4d7eea5a2506cb51b58533eba28c2cb) ) // Should be KF00x002, x = revision
	ROM_LOAD16_BYTE( "rq_ver1.05.u3", 0x00001, 0x80000, CRC(8c78889e) SHA1(584ba123e9caafdbddc96a4d9b2b6f6994fa84b0) ) // Should be KF00x004, x = revision

	ROM_REGION( 0x800000, "sprites", 0 )    // Sprites
	ROM_LOAD64_WORD( "kf-001-005_t42.u16", 0x000000, 0x200000, CRC(25e07d5c) SHA1(dd0818611f39be25dc6f0c737da4e79c6c0f9659) )
	ROM_LOAD64_WORD( "kf-001-006_t43.u15", 0x000002, 0x200000, CRC(67e2ecc4) SHA1(35cdaf7fcd29e0229da104baced41fa7620dba3d) )
	ROM_LOAD64_WORD( "kf-001-007_t44.u18", 0x000004, 0x200000, CRC(9daec83d) SHA1(07de144898deac5058d05466f29682d7840323b7) )
	ROM_LOAD64_WORD( "kf-001-008_t45.u17", 0x000006, 0x200000, CRC(f6ef6e41) SHA1(c3e838dd4dc340f44abdf45ec0b90de24f50dda9) )

	ROM_REGION( 0x200000, "x1snd", 0 )  // Samples
	ROM_LOAD( "kf-001-009_t46.u32", 0x000000, 0x200000, CRC(2a9641f9) SHA1(efb9df78f1877eddf29c4dae2461546adb9cea8f) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8_kf-001.u38", 0x000, 0x117, NO_DUMP )
ROM_END

/***************************************************************************

Endless Riches
(c) 199? E.N.Tiger

   CPU: Toshiba TMP68301AF-16 (100 Pin PQFP)
 Video: NEC DX-101 (240 Pin PQFP, @ U10)
        NEC DX-102 (52 Pin PQFP x3, @ U28, U30 & U45)
 Sound: X1-010 (Mitsubishi M60016 Gate Array, 80 Pin PQFP @ U26)
   OSC: 50MHz & 28MHz
 Other: 8 Position Dipswitch x 2
        Reset Push Button at SW1
        3.6V Battery at BT1
        GAL 16V8 - labeled "KF-001" at U38

Memory:
M1 are TC551001BFL-70L at U42 & U43
M2 is  W2465K-70LL at U27
M3 are LH5168D-10L at U8 & U9
M4 are UT62256SC-70L at U6, U7, U13 & U14

PCB Number: P-FG-03
+-----+_+----------------------------------------------------+
|             +------+      U  U                             |
| VOL         |Seta  |   M  5  5            +---------------+|
|             |X1-010|   2  8  7    +-+  M  |KFC-U16-C00 U16||
|             +------+      *  *    | |  1  +---------------+|
+-+                                 |U|                      |
  |  +-+    +-+           BT1       |3|            U20*      |
+-+  | |    | |         M           |2|  M                   |
|    |U| U  |U| U  M M  4           | |  1  +---------------+|
|    |3| 5  |2| 4  3 3              +-+     |KFC-U15-C00 U15||
|8   | | *  | | *       M                   +---------------+|
|    +-+    +-+         4                                    |
|L                                                 U19*      |
|I                                                           |
|N                                          +---------------+|
|E                                          |KFC-U18-C00 U18||
|R                           +----------+   +---------------+|
|           +-------+        |          |                    |
|C C        |Toshiba|        |   NEC    |          U22*      |
|o N        |  TMP  |        |  DX-101  |                    |
|n 1        | 68301 |        |          |   +---------------+|
|n        U +-------+        |          |   |KFC-U17-C00 U17||
|e C      5                  +----------+   +---------------+|
|c N      6                                                  |
|t 2      *                                        U21*      |
|e    +---+    +---+       U  50MHz 28MHz                    |
|r    |DX |  S |DX |       3                                 |
|     |102|  W |102|       8                   +---+    OSC2*|
|     +---+  1 +---+                    M  M   |DX |         |
+-+            D D                      4  4   |102|         |
  |            S S                             +---+         |
+-+            W W                                           |
|              2 1                                           |
+------------------------------------------------------------+

CN1 - 7 Pin connector
CN2 - 8 Pin connector

U56 is unpopulated 93C45 EEPROM
DSW2 is unpopulated
* Denotes not populated.

    U3-U5 silkscreened 27C4001
  U57-U58 silkscreened 23C8001E
  U15-U22 silkscreened 23C32000
      U32 silkscreened 23C32000

KFP is Program, KFC is Character Graphics and KFS is Sound

Note:
  8-Liner version of P-FG-02 (see Reel'N Quake! above)
  Hitting Service Mode "F2" will show Ver 1.7, but going through the diagnostic "0"
   Main Menu --> Test Mode --> Memory Test will show Version 1.20

***************************************************************************/

ROM_START( endrichs ) // Memory Test doesn't show version like the set below
	ROM_REGION( 0x100000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "endless_riches_u2_prg_even_v1.21_9-1-99.u2", 0x00000, 0x80000, CRC(bae6456c) SHA1(edbf4dc01095b9882243acf2bc8aecab8d9a1414) ) // handwritten label:  Endless Riches U2 PRG EVEN V1.21 9/1/99
	ROM_LOAD16_BYTE( "endless_riches_u3_prg_odd_v1.21_9-1-99.u3",  0x00001, 0x80000, CRC(2b0529d6) SHA1(b85fc5d598081bc96ecdecb5663de698c4b95e27) ) // handwritten label:  Endless Riches U3 PRG ODD V1.21 9/1/99

	ROM_REGION( 0x800000, "sprites", 0 )    // Sprites
	ROM_LOAD64_WORD( "kfc-u16-c00.u16", 0x000000, 0x200000, CRC(cbfe5e0f) SHA1(6c7c8088c43231997ac47ce05cf43c78c1fdad47) )
	ROM_LOAD64_WORD( "kfc-u15-c00.u15", 0x000002, 0x200000, CRC(98e4c36c) SHA1(651be122b78f225d38878ae90776f66989440590) )
	ROM_LOAD64_WORD( "kfc-u18-c00.u18", 0x000004, 0x200000, CRC(561ac136) SHA1(96da493157405a5d3d72b8cc3004abd3fa3eadfa) )
	ROM_LOAD64_WORD( "kfc-u17-c00.u17", 0x000006, 0x200000, CRC(34660029) SHA1(cf09b97422497d739f71e6ff8b9974fca0329928) )

	ROM_REGION( 0x200000, "x1snd", 0 )  // Samples
	ROM_LOAD( "kfs-u32-c00.u32", 0x000000, 0x200000, CRC(e9ffbecf) SHA1(3cc9ab3f4be1a305235603a68ca1e15797fb27cb) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8_kf-001.u38", 0x000, 0x117, NO_DUMP )
ROM_END

ROM_START( endrichsa )
	ROM_REGION( 0x100000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "kfp_u02_c12.u2", 0x00000, 0x80000, CRC(462341d2) SHA1(a88215d74469513f4239853f62d4dbbffe2aa83a) )
	ROM_LOAD16_BYTE( "kfp_u03_c12.u3", 0x00001, 0x80000, CRC(2baee8d1) SHA1(f86920382c54a259adb1dee253859561746d215a) )

	ROM_REGION( 0x800000, "sprites", 0 )    // Sprites
	ROM_LOAD64_WORD( "kfc-u16-c00.u16", 0x000000, 0x200000, CRC(cbfe5e0f) SHA1(6c7c8088c43231997ac47ce05cf43c78c1fdad47) )
	ROM_LOAD64_WORD( "kfc-u15-c00.u15", 0x000002, 0x200000, CRC(98e4c36c) SHA1(651be122b78f225d38878ae90776f66989440590) )
	ROM_LOAD64_WORD( "kfc-u18-c00.u18", 0x000004, 0x200000, CRC(561ac136) SHA1(96da493157405a5d3d72b8cc3004abd3fa3eadfa) )
	ROM_LOAD64_WORD( "kfc-u17-c00.u17", 0x000006, 0x200000, CRC(34660029) SHA1(cf09b97422497d739f71e6ff8b9974fca0329928) )

	ROM_REGION( 0x200000, "x1snd", 0 )  // Samples
	ROM_LOAD( "kfs-u32-c00.u32", 0x000000, 0x200000, CRC(e9ffbecf) SHA1(3cc9ab3f4be1a305235603a68ca1e15797fb27cb) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "gal16v8_kf-001.u38", 0x000, 0x117, NO_DUMP )
ROM_END

/***************************************************************************

Star Audition
(c)1997 Namco

The PCB has Namco number: M-133 MAIN PCB. On the back it has a Seta number: P0-130B.
There's a small plug-in sub board containing a Sony A1585Q chip. It's an RGB decoder.
There's no video outout on this PCB.

TMP68301-16
Only one OSC at 50MHz, so cpu clock is probably 50/4
1x NEC DX101
3x NEC DX102 graphics chips
Allumer X1-010 sound chip

RTC - NEC D4992 and 3.6v nicad barrel battery

Main RAM looks like 3x 128kx8 SRAMs
8kx8 SRAM near Allumer chip/SND ROM
4x 128kx8 near top of big NEC DX101 chip
2x 32kx8 SRAMs near bottom of big NEC DX101 chip
6x 32kx8 SRAMs near bottom right corner of PCB near CGx ROMs
Flash: Sharp LH28F016SAT-70 (TSOP56)

***************************************************************************/

ROM_START( staraudi )
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "su1_mpr2.u02", 0x000000, 0x80000, CRC(9e8d1943) SHA1(0a9cb7cb0e9dcd9db08f4bb7033986cb51f2cebf) )
	ROM_LOAD16_BYTE( "su1_mpr0.u03", 0x000001, 0x80000, CRC(0a93d1d1) SHA1(7624000afc08eb65cdfa38260ff5d39ac73bba16) )
	ROM_LOAD16_BYTE( "su1_mpr3.u04", 0x100000, 0x80000, CRC(74e07efd) SHA1(6400983c90a28c7d8e091557b0a4102b21035ac8) )
	ROM_LOAD16_BYTE( "su1_mpr1.u05", 0x100001, 0x80000, CRC(3feb93ec) SHA1(0900d9fb37c884c472b9858002713a2b8ba4e519) )

	ROM_REGION( 0x2000000, "sprites", ROMREGION_ERASE )   // Sprites
	ROM_LOAD64_WORD( "su1_cg0.u16", 0x000000, 0x200000, CRC(64281c22) SHA1(3e2b00bd623915a8be7e21812ff96280d071d08f) )
	ROM_LOAD64_WORD( "su1_cg1.u15", 0x000002, 0x200000, CRC(cd95be41) SHA1(c19c7e6212dab69b575c0e4ce1f7bc390abba67b) )
	ROM_LOAD64_WORD( "su1_cg2.u18", 0x000004, 0x200000, CRC(63eeee49) SHA1(14a6d358f8a0e4572065c715507d730cf2b77571) )
	ROM_LOAD64_WORD( "su1_cg3.u17", 0x000006, 0x200000, CRC(fefb2101) SHA1(0e9d63a779210b37565cd000b9d131e9c8f4e329) )
	// Additional tiles from RAM are decoded here (starting from tile code 7c000)

	ROM_REGION( 0x200000, "flash", ROMREGION_ERASE )
	ROM_LOAD( "lh28f016sat_flash.u08", 0x000000, 0x200000, CRC(002255bd) SHA1(5e94c29e9a785fe49229f57bc94234ac79dd2f3b) )

	ROM_REGION( 0x400000, "x1snd", 0 )  // Samples
	ROM_LOAD( "su1_snd.u32", 0x000000, 0x400000, BAD_DUMP CRC(d5376010) SHA1(89fab1fbb45c7cf8acb63c31ecafdeb3482c2fec) ) // BAD, inconsistent reads: FIXED BITS (xxxxxxxx00000000)
ROM_END

void staraudi_state::driver_start()
{
	seta2_state::driver_start();

	// bad sound rom: replace the missing (zero) sample with the previous one
	uint8_t *samples = memregion("x1snd")->base();
	for (int i = 0; i < 0x400000; i += 2)
		samples[i + 1] = samples[i];
}

/***************************************************************************

Sammy USA Outdoor Shooting Series PCB

PCB B0-003A (or B0-003B):
   Deer Hunting USA (c) 2000 Sammy USA
   Turkey Hunting USA (c) 2001 Sammy USA

PCB B0-010A:
   Wing Shooting Championship (c) 2001 Sammy USA
   Trophy Hunting - Bear & Moose (c) 2002 Sammy USA


PCB Number: B0-003A (or B0-003B)
+-----------------------------------------------------------+
|             VOL                          +------+         |
|                                          |X1-010|     M1  |
|   +---+ +---+                            |M60016|         |
|   |   | |   |  M    M                    |CALRUA|  +---+  |
+-+ | U | | U |  2    1                    +------+  |   |  |
  | | 0 | | 0 |                   D4992              |   |  |
+-+ | 7 | | 6 |  M    M         32.768kHz            | U |  |
|   |   | |   |  2    1                              | 1 |  |
|   +---+ +---+                                      | 8 |  |
|                                          Lattice   |   |  |
|J  D +---+  C                            ispLSI2032 |   |  |
|A  S |DX |  N   BAT1           +-------+            +---+  |
|M  W |102|  5                  |Toshiba|  D                |
|M  1 +---+                     |  TMP  |  S EEPROM       C |
|A           C                  | 68301 |  W              N |
|            N  Lattice         +-------+  2              2 |
|C           6  isp1016E                                    |
|o                              +----------+    50MHz       |
|n    +---+                     |          |              +-+
|n    |DX |  SW1                |   NEC    |    M   M     | |
|e    |102|                     |  DX-101  |    3   3     | |
|c    +---+         M  M        |          |              | |
|t                  1  1        |          |              | |
|e                              +----------+              | |
|r                                                        |C|
|                             +---+      +---++---++---+  |N|
|                  28MHz      |   |      |   ||   ||   |  |3|
|     +---+                   |   |      |   ||   ||   |  | |
+-+   |DX |                   | U |      | U || U || U |  | |
  |   |102|                   | 4 |      | 4 || 3 || 3 |  | |
+-+   +---+                   | 0 |      | 1 || 8 || 9 |  | |
| C                           |   |      |   ||   ||   |  | |
| N                           |   |      |   ||   ||   |  +-+
| 1                           +---+      +---++---++---+    |
+-----------------------------------------------------------+

PCB Number: B0-010A - This PCB is slightly revised for 2 player play
+-----------------------------------------------------------+
|             VOL                          +------+         |
|                                          |X1-010|     M1  |
|   +---+ +---+                            |M60016|         |
|   |   | |   |  M    M                    |CALRUA|  +---+  |
+-+ | U | | U |  2    1                    +------+  |   |  |
  | | 0 | | 0 |                   D4992              |   |  |
+-+ | 7 | | 6 |  M    M         32.768kHz            | U |  |
|   |   | |   |  2    1                              | 1 |  |
|   +---+ +---+                                      | 8 |  |
|                                          Lattice   |   |  |
|J  D +---+  C                            ispLSI2032 |   |  |
|A  S |DX |  N   BAT1           +-------+            +---+  |
|M  W |102|  5                  |Toshiba|  D                |
|M  1 +---+                     |  TMP  |  S EEPROM       C |
|A           C                  | 68301 |  W              N |
|            N  Lattice         +-------+  2              2 |
|C           6  isp1016E                                    |
|o                              +----------+    50MHz       |
|n    +---+                     |          |              +-+
|n    |DX |  SW1                |   NEC    |    M   M     | |
|e    |102|                     |  DX-101  |    3   3     | |
|c    +---+         M  M        |          |              | |
|t                  1  1        |          |              | |
|e                              +----------+              | |
|r                                                        |C|
|                             +---+      +---++---++---+  |N|
|                  28MHz      |   |      |   ||   ||   |  |3|
|     +---+              C    |   |      |   ||   ||   |  | |
+-+   |DX |              N    | U |      | U || U || U |  | |
  |   |102|              7    | 4 |      | 4 || 3 || 3 |  | |
+-+   +---+                   | 0 |      | 1 || 8 || 9 |  | |
| C           Lattice    C    |   |      |   ||   ||   |  | |
| N           isp1016E   N    |   |      |   ||   ||   |  +-+
| 1                      8    +---+      +---++---++---+    |
+-----------------------------------------------------------+


   CPU: Toshiba TMP68301AF-16 (100 Pin PQFP)
 Video: NEC DX-101 (240 Pin PQFP)
        NEC DX-102 (52 Pin PQFP x3)
 Sound: X1-010 (Mitsubishi M60016 Gate Array, 80 Pin PQFP)
EEPROM: 93LC46BX (1K Low-power 64 x 16-bit organization serial EEPROM)
   OSC: 50MHz, 28MHz & 32.768kHz
 Other: 8 Position Dipswitch x 2
        Lattice ispLSI2032 - stamped "KW001"
        Lattice isp1016E - stamped "GUN" (2 for PCB B0-010A, used for light gun input)
        NEC D4992 CMOS 8-Bit Parallel I/O Calendar Clock
        BAT1 - CR2032 3Volt

Ram M1 are Toshiba TC55257DFL-70L
Ram M2 are NEC D43001GU-70L
Ram M3 are ISSI IS62C1024L-70Q

U06 Program rom ST27C801 (even)
U07 Program rom ST27C801 (odd)

U18 Mask rom (Samples 23C32000 32Mbit (read as 27C322))

U38 - U40 Mask roms (Graphics 23c64020 64Mbit) - 23C64020 read as 27C322 with pin11 +5v & 27C322 with pin11 GND

Connectors:
  CN1 - Unpopulated 8 pin header
  CN2 - 8 Pin header - use unknown
  CN3 - Unpopulated 3 row, 96 pin header
  CN5 + CN6 labeled GUN:
   CN6-4 Pin (1-4)          CN5-4 Pin (7-10, standard HAPP light gun pinout)
    1 No Connection          7 Red    +5VDC
    2 Green  Pump Switch     8 White  Trigger Switch
    3 Brown  Pump Switch     9 Black  Ground
    4 No Connection         10 Blue   Optical
  CN7 + CN8 labeled GUN (on B0-010A only, same pinout as CN5 + CN6)

==========================================

Location Test version of Trophy Hunter:

On service menu is an additional option: "9. PLAY DATA ( for LOC TEST )"

  Under "7. OPTIONAL SETTING" is an added option:
        3. PLAY DATA CLEAR
        4. RETURN TO TEST MENU
    Release versions show selection 3. RETURN TO TEST MENU, with no fourth selection

Although in the I/O TEST screen seems to test for the shotgun PUMP, The PCB seems to be set up
 for two standard HAPP light guns.  It's unknown how, if at all, the PUMP buttons are mapped or
 hooked up through the PCB

PCB Number: P0-145-1
+-----------------------------------------------------------+
|             VOL                          +------+         |
|                                          |X1-010|     M1  |
|   +---+ +---+                            |M60016|         |
|   |   | |   |  M    M                    |CALRUA|  +---+  |
+-+ | U | | U |  2    1                    +------+  |   |  |
  | | 0 | | 0 |                   D4992              |   |  |
+-+ | 7 | | 6 |  M    M         32.768kHz            | U |  |
|   |   | |   |  2    1                              | 1 |  |
|   +---+ +---+                                      | 8 |  |
|                                          Lattice   |   |  |
|J  D +---+  C                            ispLSI2032 |   |  |
|A  S |DX |  N   BAT1           +-------+            +---+  |
|M  W |102|  5                  |Toshiba|  D                |
|M  1 +---+                     |  TMP  |  S EEPROM       C |
|A           C                  | 68301 |  W              N |
|            N  Lattice         +-------+  2              2 |
|C           6  isp1016E                                    |
|o                              +----------+    50MHz       |
|n    +---+                     |          |              +-+
|n    |DX |  SW1                |   NEC    |    M   M     | |
|e    |102|                     |  DX-101  |    3   3     | |
|c    +---+         M  M        |          |              | |
|t                  1  1        |          |              | |
|e                              +----------+              | |
|r                                                        |C|
|                             +---+      +---++---++---+  |N|
|                  28MHz      |   |      |   ||   ||   |  |3|
|     +---+                   |   |      |   ||   ||   |  | |
+-+   |DX |                   | U |      | U || U || U |  | |
  |   |102|                   | 4 |      | 4 || 3 || 3 |  | |
+-+   +---+                   | 0 |      | 1 || 8 || 9 |  | |
| C                           |   |      |   ||   ||   |  | |
| N                           |   |      |   ||   ||   |  +-+
| 1                           +---+      +---++---++---+    |
+-----------------------------------------------------------+

Differences from PCB B0-003A (or B0-003B):

CN1 is 8 pin header - unknown use
CN3 Female 3 row, 96 pin connection populated on the underside to connect to the P1-115A flash ROM PCB
CN5 is labeled pins 1-4 and silkscreened GUN1
CN6 is labeled pins 1-4 and silkscreened GUN2
Lattice isp1016E - labeled "2GUN"

U38 - U40 unpopulated (data comes from P0-145-1 PCB)

P1-115A
+-+--------------------+------------------------------------+
| |        CN3         |                   SW3              |
| +--------------------+                                    |
| |        CN4         |                           U  J  U  |
| +--------------------+                           5  P  6  |
|                                                  9  4  0  |
|                                                           |
|                                                           |
|    28F016.U23    28F016.U31    28F016.U27    28F016.U35   |
|                                                           |
|    28F016.U22    28F016.U30    28F016.U26    28F016.U34   |
|                                                           |
|    28F016.U21    28F016.U29    28F016.U25    28F016.U33   |
|                                                           |
|    28F016.U20    28F016.U28    28F016.U24    28F016.U32   |
|  +--------------------+                                   |
|  |        CN6         |                                   |
+--+--------------------+-----------------------------------+

Flash ROMs are SHARP LH28F016SAT-70

SW3 unpopulated switch
CN3 Unused 3 row, 96 pin connection (underside)
CN4 Male 3 row, 96 pin connection (underside, to main P0-145-1 PCB)
CN6 Unused 3 row, 96 pin connection (top side)
JP4 3 pin Jumper header - unknown use
U59 GAL labeled FLASHA02U59 3869
U60 GAL labeled FLASH3A 2A29

--------------------------------------------------------------------------

From the WSC upgrade instruction sheet:

 Wing Shooting Championship
      Game Echancement
          1/23/02

New Program chip Ver. 2.00 For Wing Shooting Championship
We are announcing NEW GAME FEATURES to enhance game play. Please refer below.

NEW FEATURES
------------

 * Easier play for the first 3 hunting spots in every state with the addition of more birds.
 * The "BEGINNER" weapon has been changed to the 5-shot PUMP SHOTGUN plus the "hit area"
    for each shot has been increased. Same as the 3-shot SEMI-AUTO SHOTGUN.
 * Player can now advance through all result screens faster by pulling gun trigger.
 * The Auto Select bird is now GOOSE (easiest target) if player fails to choose bird at start of game.

Commonly labeled as either:

Deer Hunting:
 Deer Hunting USA       AS0907
    U7 Ver 4.3      or  E05
    2000.11.1           U7
                        5D89

For version 3:
 Deer Hunting USA
    U7 Ver 3.0
    2000.5.31

Two PCBs (serial numbers WH 00111 & WH 00001) from Japan were labeled as:

  AS0      AS0
  909E01 & 908E01     <-- Higher ROM numbers and purports to be E01 but shows Ver .4.4.1
  U7 JDH   U6 JDH

Turkey Hunting:
    Turkey              ASX
  U7 Ver 1.00       or  907E01
     AB40               TH

Wing Shooting Championship:
     WSC                AS
  U7 Ver. 2.00      or  1007
     A48F               E03

Trophy Hunting - Bear & Moose:
    Trophy              AS
  U7 Ver 1.00       or  1107
    CEEF                E01
***************************************************************************/

ROM_START( deerhunt ) // Deer Hunting USA V4.3 (11/1/2000) - The "E05" breaks version label conventions but is correct & verified
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "as0906_e05_u6_694e.u06", 0x000000, 0x100000, CRC(20c81f17) SHA1(d41d93d6ee88738cec55f7bf3ce6be1dbec68e09) ) // checksum 694E printed on label
	ROM_LOAD16_BYTE( "as0907_e05_u7_5d89.u07", 0x000001, 0x100000, CRC(1731aa2a) SHA1(cffae7a99a7f960a62ef0c4454884df17a93c1a6) ) // checksum 5D89 printed on label

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD64_WORD( "as0901m01.u38", 0x0000000, 0x800000, CRC(1d6acf8f) SHA1(6f61fe21bebb7c87e8e6c3ef3ba73b8cf327dde9) )
	ROM_LOAD64_WORD( "as0902m01.u39", 0x0000002, 0x800000, CRC(c7ca2128) SHA1(86be3a3ec2f86f61acfa3d4d261faea3c27dc378) )
	ROM_LOAD64_WORD( "as0903m01.u40", 0x0000004, 0x800000, CRC(e8ef81b3) SHA1(97666942ca6cca5b8ea6451314a2aaabad9e06ba) )
	ROM_LOAD64_WORD( "as0904m01.u41", 0x0000006, 0x800000, CRC(d0f97fdc) SHA1(776c9d42d03a9f61155521212305e1ed696eaf47) )

	ROM_REGION( 0x400000, "x1snd", 0 )  // Samples
	ROM_LOAD( "as0905m01.u18", 0x000000, 0x400000, CRC(8d8165bb) SHA1(aca7051613d260734ee787b4c3db552c336bd600) )
ROM_END

ROM_START( deerhunta ) // Deer Hunting USA V4.2 (xx/x/2000)
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "as0906_e04_u6_6640.u06", 0x000000, 0x100000, CRC(bb3af36f) SHA1(f04071347e8ad361bf666fcb6c0136e522f19d47) ) // checksum 6640 printed on label
	ROM_LOAD16_BYTE( "as0907_e04_u7_595a.u07", 0x000001, 0x100000, CRC(83f02117) SHA1(70fc2291bc93af3902aae88688be6a8078f7a07e) ) // checksum 595A printed on label

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD64_WORD( "as0901m01.u38", 0x0000000, 0x800000, CRC(1d6acf8f) SHA1(6f61fe21bebb7c87e8e6c3ef3ba73b8cf327dde9) )
	ROM_LOAD64_WORD( "as0902m01.u39", 0x0000002, 0x800000, CRC(c7ca2128) SHA1(86be3a3ec2f86f61acfa3d4d261faea3c27dc378) )
	ROM_LOAD64_WORD( "as0903m01.u40", 0x0000004, 0x800000, CRC(e8ef81b3) SHA1(97666942ca6cca5b8ea6451314a2aaabad9e06ba) )
	ROM_LOAD64_WORD( "as0904m01.u41", 0x0000006, 0x800000, CRC(d0f97fdc) SHA1(776c9d42d03a9f61155521212305e1ed696eaf47) )

	ROM_REGION( 0x400000, "x1snd", 0 )  // Samples
	ROM_LOAD( "as0905m01.u18", 0x000000, 0x400000, CRC(8d8165bb) SHA1(aca7051613d260734ee787b4c3db552c336bd600) )
ROM_END

ROM_START( deerhuntb ) // Deer Hunting USA V4.0 (6/15/2000)
	ROM_REGION( 0x200000, "maincpu", 0 )        // TMP68301 Code
	ROM_LOAD16_BYTE( "as_0906_e04.u06", 0x000000, 0x100000, CRC(07d9b64a) SHA1(f9aac644aab920bbac84b14836ee589ccd51f6db) ) // also commonly labeled as: Deer Hunting USA U6 Ver 4.0 2000.6.15 - SUM16 = 7BBB
	ROM_LOAD16_BYTE( "as_0907_e04.u07", 0x000001, 0x100000, CRC(19973d08) SHA1(da1cc02ce480a62ccaf94d0af1246a340f054b43) ) // also commonly labeled as: Deer Hunting USA U7 Ver 4.0 2000.6.15 - SUM16 = 4C78

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD64_WORD( "as0901m01.u38", 0x0000000, 0x800000, CRC(1d6acf8f) SHA1(6f61fe21bebb7c87e8e6c3ef3ba73b8cf327dde9) )
	ROM_LOAD64_WORD( "as0902m01.u39", 0x0000002, 0x800000, CRC(c7ca2128) SHA1(86be3a3ec2f86f61acfa3d4d261faea3c27dc378) )
	ROM_LOAD64_WORD( "as0903m01.u40", 0x0000004, 0x800000, CRC(e8ef81b3) SHA1(97666942ca6cca5b8ea6451314a2aaabad9e06ba) )
	ROM_LOAD64_WORD( "as0904m01.u41", 0x0000006, 0x800000, CRC(d0f97fdc) SHA1(776c9d42d03a9f61155521212305e1ed696eaf47) )

	ROM_REGION( 0x400000, "x1snd", 0 )  // Samples
	ROM_LOAD( "as0905m01.u18", 0x000000, 0x400000, CRC(8d8165bb) SHA1(aca7051613d260734ee787b4c3db552c336bd600) )
ROM_END

	// Are there versions 3.x of Deer Hunting USA with labels "AS0906 E03 U06" & "AS0907 E03 U07" ??

ROM_START( deerhuntc ) // These rom labels break label conventions but is correct & verified. Version in program code is listed as 0.00
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "as_0937_e01.u06", 0x000000, 0x100000, CRC(8d74088e) SHA1(cb11ffaf4c0267cc8cbe01accc3daeed910a3af3) ) // SUM16 = C2CD - same as version dated 2000.5.31?
	ROM_LOAD16_BYTE( "as_0938_e01.u07", 0x000001, 0x100000, CRC(c7657889) SHA1(4cc707c8abbc0862457375a9a910d3c338859193) ) // SUM16 = 27D7 - same as version dated 2000.5.31?

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD64_WORD( "as0901m01.u38", 0x0000000, 0x800000, CRC(1d6acf8f) SHA1(6f61fe21bebb7c87e8e6c3ef3ba73b8cf327dde9) )
	ROM_LOAD64_WORD( "as0902m01.u39", 0x0000002, 0x800000, CRC(c7ca2128) SHA1(86be3a3ec2f86f61acfa3d4d261faea3c27dc378) )
	ROM_LOAD64_WORD( "as0903m01.u40", 0x0000004, 0x800000, CRC(e8ef81b3) SHA1(97666942ca6cca5b8ea6451314a2aaabad9e06ba) )
	ROM_LOAD64_WORD( "as0904m01.u41", 0x0000006, 0x800000, CRC(d0f97fdc) SHA1(776c9d42d03a9f61155521212305e1ed696eaf47) )

	ROM_REGION( 0x400000, "x1snd", 0 )  // Samples
	ROM_LOAD( "as0905m01.u18", 0x000000, 0x400000, CRC(8d8165bb) SHA1(aca7051613d260734ee787b4c3db552c336bd600) )
ROM_END

ROM_START( deerhuntd ) // Deer Hunting USA V2.x - No version number is printed to screen but "E02" in EPROM label signifies V2
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "as_0906_e02.u06", 0x000000, 0x100000, CRC(190cca42) SHA1(aef63f5e8c71ed0156b8b0104c5d23872c119167) ) // Version in program code is listed as 0.00
	ROM_LOAD16_BYTE( "as_0907_e02.u07", 0x000001, 0x100000, CRC(9de2b901) SHA1(d271bc54c41e30c0d9962eedd22f3ef2b7b8c9e5) ) // Verified with two different sets of chips

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD64_WORD( "as0901m01.u38", 0x0000000, 0x800000, CRC(1d6acf8f) SHA1(6f61fe21bebb7c87e8e6c3ef3ba73b8cf327dde9) )
	ROM_LOAD64_WORD( "as0902m01.u39", 0x0000002, 0x800000, CRC(c7ca2128) SHA1(86be3a3ec2f86f61acfa3d4d261faea3c27dc378) )
	ROM_LOAD64_WORD( "as0903m01.u40", 0x0000004, 0x800000, CRC(e8ef81b3) SHA1(97666942ca6cca5b8ea6451314a2aaabad9e06ba) )
	ROM_LOAD64_WORD( "as0904m01.u41", 0x0000006, 0x800000, CRC(d0f97fdc) SHA1(776c9d42d03a9f61155521212305e1ed696eaf47) )

	ROM_REGION( 0x400000, "x1snd", 0 )  // Samples
	ROM_LOAD( "as0905m01.u18", 0x000000, 0x400000, CRC(8d8165bb) SHA1(aca7051613d260734ee787b4c3db552c336bd600) )
ROM_END

ROM_START( deerhunte ) // Deer Hunting USA V1.x - No version number is printed to screen but "E01" in EPROM label signifies V1
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "as_0906_e01.u06", 0x000000, 0x100000, CRC(103e3ba3) SHA1(677d912ea9ed2ee1f26cdcac1687ce8ef416a96f) ) // Version in program code is listed as 0.00
	ROM_LOAD16_BYTE( "as_0907_e01.u07", 0x000001, 0x100000, CRC(ddeb0f97) SHA1(a2578071f3506d69057d2256685b969adc50d275) ) // Verified with two different sets of chips

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD64_WORD( "as0901m01.u38", 0x0000000, 0x800000, CRC(1d6acf8f) SHA1(6f61fe21bebb7c87e8e6c3ef3ba73b8cf327dde9) )
	ROM_LOAD64_WORD( "as0902m01.u39", 0x0000002, 0x800000, CRC(c7ca2128) SHA1(86be3a3ec2f86f61acfa3d4d261faea3c27dc378) )
	ROM_LOAD64_WORD( "as0903m01.u40", 0x0000004, 0x800000, CRC(e8ef81b3) SHA1(97666942ca6cca5b8ea6451314a2aaabad9e06ba) )
	ROM_LOAD64_WORD( "as0904m01.u41", 0x0000006, 0x800000, CRC(d0f97fdc) SHA1(776c9d42d03a9f61155521212305e1ed696eaf47) )

	ROM_REGION( 0x400000, "x1snd", 0 )  // Samples
	ROM_LOAD( "as0905m01.u18", 0x000000, 0x400000, CRC(8d8165bb) SHA1(aca7051613d260734ee787b4c3db552c336bd600) )
ROM_END

ROM_START( deerhuntj ) // Higher ROM labels indicate a specific version / region - No specific "For use in Japan" warning
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "as0_908e01_u6_jdh.u06", 0x000000, 0x100000, CRC(52f037da) SHA1(72afb4461be059655a2fe9b138e9feef19ecaa84) ) // Version shows as VER .4.4.1
	ROM_LOAD16_BYTE( "as0_909e01_u7_jdh.u07", 0x000001, 0x100000, CRC(b391bc87) SHA1(eb62e18b6ac9b0198911ec6684de73102c1d6df0) )

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD64_WORD( "as0901m01.u38", 0x0000000, 0x800000, CRC(1d6acf8f) SHA1(6f61fe21bebb7c87e8e6c3ef3ba73b8cf327dde9) )
	ROM_LOAD64_WORD( "as0902m01.u39", 0x0000002, 0x800000, CRC(c7ca2128) SHA1(86be3a3ec2f86f61acfa3d4d261faea3c27dc378) )
	ROM_LOAD64_WORD( "as0903m01.u40", 0x0000004, 0x800000, CRC(e8ef81b3) SHA1(97666942ca6cca5b8ea6451314a2aaabad9e06ba) )
	ROM_LOAD64_WORD( "as0904m01.u41", 0x0000006, 0x800000, CRC(d0f97fdc) SHA1(776c9d42d03a9f61155521212305e1ed696eaf47) )

	ROM_REGION( 0x400000, "x1snd", 0 )  // Samples
	ROM_LOAD( "as0905m01.u18", 0x000000, 0x400000, CRC(8d8165bb) SHA1(aca7051613d260734ee787b4c3db552c336bd600) )
ROM_END

ROM_START( turkhunt ) // V1.0 is currently the only known version
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "asx_906e01_th.u06", 0x000000, 0x100000, CRC(c96266e1) SHA1(0ca462b3b0f27198e36384eee6ea5c5d4e7e1293) ) // also commonly labeled as: Turkey U6 Ver 1.00 E510
	ROM_LOAD16_BYTE( "asx_907e01_th.u07", 0x000001, 0x100000, CRC(7c67b502) SHA1(6a0e8883a115dac4095d86897e7eca2a007a1c71) ) // also commonly labeled as: Turkey U7 Ver 1.00 AB40

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD64_WORD( "asx901m01.u38", 0x0000000, 0x800000, CRC(eabd3f44) SHA1(5a1ac986d11a8b019e18761cf4ea0a6f49fbdbfc) )
	ROM_LOAD64_WORD( "asx902m01.u39", 0x0000002, 0x800000, CRC(c32130c8) SHA1(70d56ebed1f51657aaee02f95ac51589733e6eb7) )
	ROM_LOAD64_WORD( "asx903m01.u40", 0x0000004, 0x800000, CRC(5f86c322) SHA1(5a72adb99eea176199f172384cb051e2b045ab94) )
	ROM_LOAD64_WORD( "asx904m01.u41", 0x0000006, 0x800000, CRC(c77e0b66) SHA1(0eba30e62e4bd38c198fa6cb69fb94d002ded77a) )

	ROM_REGION( 0x400000, "x1snd", 0 )  // Samples
	ROM_LOAD( "asx905m01.u18", 0x000000, 0x400000, CRC(8d9dd9a9) SHA1(1fc2f3688d2c24c720dca7357bca6bf5f4016c53) )
ROM_END

ROM_START( wschamp ) // Wing Shooting Championship V2.00 (01/23/2002) - The "E03" breaks version label conventions but is correct & verified
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "as_1006_e03.u06", 0x000000, 0x100000, CRC(0ad01677) SHA1(63e09b9f7cc8b781af1756f86caa0cc0962ae584) ) // also commonly labeled as: WSC U6 Ver 2.00 421E
	ROM_LOAD16_BYTE( "as_1007_e03.u07", 0x000001, 0x100000, CRC(572624f0) SHA1(0c2f67daa22f4edd66a2be990dc6cd999faff0fa) ) // also commonly labeled as: WSC U7 Ver 2.00 A48F

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD64_WORD( "as1001m01.u38", 0x0000000, 0x800000, CRC(92595579) SHA1(75a7131aedb18b7103677340c3cca7c91aaca2bf) )
	ROM_LOAD64_WORD( "as1002m01.u39", 0x0000002, 0x800000, CRC(16c2bb08) SHA1(63926464c8bd8db7d05905a953765e645942beb4) )
	ROM_LOAD64_WORD( "as1003m01.u40", 0x0000004, 0x800000, CRC(89618858) SHA1(a8bd07f233482e8f5a256af7ff9577648eb58ef4) )
	ROM_LOAD64_WORD( "as1004m01.u41", 0x0000006, 0x800000, CRC(500c0909) SHA1(73ff27d46b9285f34a50a81c21c54437f21e1939) )

	ROM_REGION( 0x400000, "x1snd", 0 )  // Samples
	ROM_LOAD( "as1005m01.u18", 0x000000, 0x400000, CRC(e4b137b8) SHA1(4d8d15073c51f7d383282cc5755ae5b2eab6226c) )
ROM_END

ROM_START( wschampa ) // Wing Shooting Championship V1.01
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "as_1006_e02.u06", 0x000000, 0x100000, CRC(d3d3b2b5) SHA1(2d036d795b40a4ed78bb9f7751f875cfc76276a9) ) // SUM16 = 31EF
	ROM_LOAD16_BYTE( "as_1007_e02.u07", 0x000001, 0x100000, CRC(78ede6d9) SHA1(e6d10f52cd4c6bf97288df44911f23bb64fc012c) ) // SUM16 = 615E

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD64_WORD( "as1001m01.u38", 0x0000000, 0x800000, CRC(92595579) SHA1(75a7131aedb18b7103677340c3cca7c91aaca2bf) )
	ROM_LOAD64_WORD( "as1002m01.u39", 0x0000002, 0x800000, CRC(16c2bb08) SHA1(63926464c8bd8db7d05905a953765e645942beb4) )
	ROM_LOAD64_WORD( "as1003m01.u40", 0x0000004, 0x800000, CRC(89618858) SHA1(a8bd07f233482e8f5a256af7ff9577648eb58ef4) )
	ROM_LOAD64_WORD( "as1004m01.u41", 0x0000006, 0x800000, CRC(500c0909) SHA1(73ff27d46b9285f34a50a81c21c54437f21e1939) )

	ROM_REGION( 0x400000, "x1snd", 0 )  // Samples
	ROM_LOAD( "as1005m01.u18", 0x000000, 0x400000, CRC(e4b137b8) SHA1(4d8d15073c51f7d383282cc5755ae5b2eab6226c) )
ROM_END

ROM_START( wschampb ) // Wing Shooting Championship V1.00, dumps match listed checksum but shows as "NG" on boot screen - need to verify correct at some point if possible
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "as10u6.u06", 0x000000, 0x100000, CRC(70a18bef) SHA1(3fb2e8a4db790dd732115d7d3d991b2d6c54feb9) ) // checksum 3F38 & 10/26 16:00 hand written on label
	ROM_LOAD16_BYTE( "as10u7.u07", 0x000001, 0x100000, CRC(cf23be7d) SHA1(b9130757466ff0d41d261b1c2435d36d2452df54) ) // checksum 1537 & 10/26 16:00 hand written on label

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD64_WORD( "as1001m01.u38", 0x0000000, 0x800000, CRC(92595579) SHA1(75a7131aedb18b7103677340c3cca7c91aaca2bf) )
	ROM_LOAD64_WORD( "as1002m01.u39", 0x0000002, 0x800000, CRC(16c2bb08) SHA1(63926464c8bd8db7d05905a953765e645942beb4) )
	ROM_LOAD64_WORD( "as1003m01.u40", 0x0000004, 0x800000, CRC(89618858) SHA1(a8bd07f233482e8f5a256af7ff9577648eb58ef4) )
	ROM_LOAD64_WORD( "as1004m01.u41", 0x0000006, 0x800000, CRC(500c0909) SHA1(73ff27d46b9285f34a50a81c21c54437f21e1939) )

	ROM_REGION( 0x400000, "x1snd", 0 )  // Samples
	ROM_LOAD( "as1005m01.u18", 0x000000, 0x400000, CRC(e4b137b8) SHA1(4d8d15073c51f7d383282cc5755ae5b2eab6226c) )
ROM_END

ROM_START( trophyh ) // Version 1.00 - v: Thu Mar 28 12:35:50 2002 JST-9 - on a B0-010A PCB with all mask ROMs
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "as_1106_e01.u06", 0x000000, 0x100000, CRC(b4950882) SHA1(2749f7ffc5b543c9f39815f0913a1d1e385b63f4) ) // also commonly labeled as: Trophy U6 Ver 1.00 D8DA
	ROM_LOAD16_BYTE( "as_1107_e01.u07", 0x000001, 0x100000, CRC(19ee67cb) SHA1(e75ce66d3ff5aad46ba997c09d6514260e617f55) ) // also commonly labeled as: Trophy U7 Ver 1.00 CEEF

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD64_WORD( "as1101m01.u38", 0x0000000, 0x800000, CRC(855ed675) SHA1(84ce229a9feb6331413253a5aed10b362e8102e5) )
	ROM_LOAD64_WORD( "as1102m01.u39", 0x0000002, 0x800000, CRC(d186d271) SHA1(3c54438b35adfab8be91df0a633270d6db49beef) )
	ROM_LOAD64_WORD( "as1103m01.u40", 0x0000004, 0x800000, CRC(adf8a54e) SHA1(bb28bf219d18082246f7964851a5c49b9c0ba7f5) )
	ROM_LOAD64_WORD( "as1104m01.u41", 0x0000006, 0x800000, CRC(387882e9) SHA1(0fdd0c77dabd1066c6f3bd64e357236a76f524ab) )

	ROM_REGION( 0x400000, "x1snd", 0 )  // Samples
	ROM_LOAD( "as1105m01.u18", 0x000000, 0x400000, CRC(633d0df8) SHA1(3401c424f5c207ef438a9269e0c0e7d482771fed) )
ROM_END

ROM_START( trophyht ) // V1.00 Location Test - v: Tue Feb 26 18:18:43 2002 JST-9 - on a P0-145-1 main PCB with a P1-115A flash ROM board
	ROM_REGION( 0x200000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "trophy_2-26_u6_2e9c.u06", 0x000000, 0x100000, CRC(74496d65) SHA1(8af7bce528557efe68e0ed8be8b60d0ba4409c35) ) // hand written label:  Trophy 2/26 U6  2E9C
	ROM_LOAD16_BYTE( "trophy_2-26_u6_de45.u07", 0x000001, 0x100000, CRC(9ae364f6) SHA1(9df8352345e59f1e0a5cb66a8b43d5ad7785ca29) ) // hand written label:  Trophy 2/26 U7  DE45

	ROM_REGION( 0x2000000, "sprites", 0 )   // Sprites
	ROM_LOAD( "lh28f016sat.u20", 0x0000000, 0x200000, NO_DUMP ) // None of the 28F016 flash ROMs are dumped
	ROM_LOAD( "lh28f016sat.u21", 0x0200000, 0x200000, NO_DUMP ) // The correct loading order is unknown
	ROM_LOAD( "lh28f016sat.u22", 0x0400000, 0x200000, NO_DUMP )
	ROM_LOAD( "lh28f016sat.u23", 0x0600000, 0x200000, NO_DUMP )
	ROM_LOAD( "lh28f016sat.u24", 0x0800000, 0x200000, NO_DUMP )
	ROM_LOAD( "lh28f016sat.u25", 0x0a00000, 0x200000, NO_DUMP )
	ROM_LOAD( "lh28f016sat.u26", 0x0c00000, 0x200000, NO_DUMP )
	ROM_LOAD( "lh28f016sat.u27", 0x0e00000, 0x200000, NO_DUMP )
	ROM_LOAD( "lh28f016sat.u28", 0x1000000, 0x200000, NO_DUMP )
	ROM_LOAD( "lh28f016sat.u29", 0x1200000, 0x200000, NO_DUMP )
	ROM_LOAD( "lh28f016sat.u30", 0x1400000, 0x200000, NO_DUMP )
	ROM_LOAD( "lh28f016sat.u31", 0x1600000, 0x200000, NO_DUMP )
	ROM_LOAD( "lh28f016sat.u32", 0x1800000, 0x200000, NO_DUMP )
	ROM_LOAD( "lh28f016sat.u33", 0x1a00000, 0x200000, NO_DUMP )
	ROM_LOAD( "lh28f016sat.u34", 0x1c00000, 0x200000, NO_DUMP )
	ROM_LOAD( "lh28f016sat.u35", 0x1e00000, 0x200000, NO_DUMP )
	ROM_LOAD64_WORD( "as1101m01.u38",   0x0000000, 0x800000, CRC(855ed675) SHA1(84ce229a9feb6331413253a5aed10b362e8102e5) ) // Load these in until the flash ROMs are dumped
	ROM_LOAD64_WORD( "as1102m01.u39",   0x0000002, 0x800000, CRC(d186d271) SHA1(3c54438b35adfab8be91df0a633270d6db49beef) ) // Load these in until the flash ROMs are dumped
	ROM_LOAD64_WORD( "as1103m01.u40",   0x0000004, 0x800000, CRC(adf8a54e) SHA1(bb28bf219d18082246f7964851a5c49b9c0ba7f5) ) // Load these in until the flash ROMs are dumped
	ROM_LOAD64_WORD( "as1104m01.u41",   0x0000006, 0x800000, CRC(387882e9) SHA1(0fdd0c77dabd1066c6f3bd64e357236a76f524ab) ) // Load these in until the flash ROMs are dumped

	ROM_REGION( 0x400000, "x1snd", 0 )  // Samples
	ROM_LOAD( "as1105m01.u18", 0x000000, 0x400000, CRC(633d0df8) SHA1(3401c424f5c207ef438a9269e0c0e7d482771fed) ) // unlabeled 27C322 with same data as AS1105M01 mask ROM
ROM_END

/***************************************************************************

 TelePachi Fever Lion
 (C) 1996 Sunsoft

   CPU: Toshiba TMP68301AF-16 (100 Pin PQFP)
 Video: Allumer X1-020 9426HK003 (@ U9)
        NEC DX-102               (52 Pin PQFP @ U8)
        Allumer X1-007 505100    (SDIP42 @ U110 - Feeds RGB DACs)
 Sound: X1-010 (Mitsubishi M60016 Gate Array, 80 Pin PQFP @ U26)
Inputs: Allumer X1-004 546100    (SDIP52)
   OSC: 50.0000MHz (@ X1), 32.5304MHz (@ X2) & 32.768kHz (@ X3)
 Other: 8 Position Dipswitch x 2
        Ricoh RP5C62 RTC (@ U128)
        3.6v Battery (@ BT1)
        93C46 EEPROM (@ U101)
        SW1 Push Button Reset

Memory:
M1 are TC551001BFL-70L at U56 & U57
M2 is  W2465K-70LL at U27
M3 are HM62256BLSP-7
M4 is LH5168D-80L

PCB Number: P0-121A / Sunsoft 2MP1-E00 (serial 0503)
+--------------------------------------------------------------+
|             +------+       +---++---+           CN3*         |
| VOL         |Seta  |   M   |   ||   |        +--------------+|
|             |X1-010|   2   | U || U |        |      U19*    ||
|             +------+       | 1 || 1 |        +--------------+|
+-+           U52  U51  BT1  | 1 || 1 |        +--------------+|
  |  +-++-+   +-++-+         | 2 || 1 |        |      U17*    ||
+-+  | || |   | || |         +---++---+   M M  +--------------+|
|    |U||U| M |U||U| M M  32.768kHz       1 1  +--------------+|
|J   |3||5| 3 |2||4| 3 4 RP5C62                | MP3 CG-1 U15 ||
|A   | ||*|   | ||*|                           +--------------+|
|M   +-++-+   +-++-+                           +--------------+|
|M                                             |      U20*    ||
|A                              +----------+   +--------------+|
|                               |          |   +--------------+|
|C                              | ALLUMER  |   |      U18*    ||
|o                              | X1-020   |   +--------------+|
|n          +-------+           |          |   +--------------+|
|n          |Toshiba|           | 9426HK003|   | MP3 CG-0 U16 ||
|e          |  TMP  |           +----------+   +--------------+|
|c          | 68301 |                          +--------------+|
|t          +-------+                          |      U23*    ||
|e         93C46                               +--------------+|
|r              D                              +--------------+|
|               S                              |      U22*    ||
+-+         X   W  +---+     50MHz 32.5304MHz  +--------------+|
  |         1   2  |DX |                       +--------------+|
  |  C      |   D  |102|                       | MP3 CG-2 U21 ||
+-+  N C    0   S  +---+         M M           +--------------+|
|    7 N    0   W        SW1     3 3                           |
|      6    4   1                                 X1-007 R G B |
+--------------------------------------------------------------+

U2 ST M27C4001 EPROM  MP3prgEVEN  U2 V1.0
U3 ST M27C4001 EPROM  MP3 prgODD  U3 V1.0
U4 unpopulated silkscreened 27C4001 TBL EVEN
U5 unpopulated silkscreened 27C4001 TBL ODD
U15 ST M27C160 EPROM  MP3 CG-1  U15 V1.0
U16 ST M27C160 EPROM  MP3 CG-0  U16 V1.0
U21 ST M27C160 EPROM  MP3 CG-2  U21 V1.0
U51 GAL KC-001C
U52 GAL KC-002C
U111 ST M27C4001 EPROM  MP3 SOUND0  U111 V1.0
U112 ST M27C4001 EPROM  MP3 SOUND1  U112 V1.0
U17, U18, U19, U20, U22 & U23 silkscreened 23C16000
* Denotes not populated.

R, G & B are resistor packs
CN3 - 96 pin connector (3 rows by 32 pins)
CN6 - Dual row 10 pin header
CN7 - 12 pin header

***************************************************************************/

ROM_START( telpacfl )
	ROM_REGION( 0x100000, "maincpu", 0 )    // TMP68301 Code
	ROM_LOAD16_BYTE( "mp3_prgeven__u2_v1.0.u2", 0x000000, 0x080000, CRC(9ab450c5) SHA1(57d9118df8a444e295cbda453a7c3238bd672ddd) )
	ROM_LOAD16_BYTE( "mp3_prgodd__u3_v1.0.u3",  0x000001, 0x080000, CRC(2a324139) SHA1(1812a7a8a2c4e222a1e5c7cb6d39cf7bf7f037db) )

	ROM_REGION( 0x800000, "sprites", ROMREGION_ERASE00 )    // Sprites
	ROM_LOAD64_WORD( "mp3_cg-0__u16_v1.0.u16", 0x000000, 0x200000, CRC(9d8453ba) SHA1(d97240ce68d6e64527930e919710764a7b669cdf) )
	ROM_LOAD64_WORD( "mp3_cg-1__u15_v1.0.u15", 0x000002, 0x200000, CRC(8ab83f38) SHA1(5ebc682b80d0d97025a97824a899946712e7acd4) )

	ROM_REGION( 0x800000, "unused", ROMREGION_ERASE00 )    // Sprites
	// not decoding the bad ROM is better than loading corrupt gfx data
	ROM_LOAD64_WORD( "mp3_cg-2__u21_v1.0.u21", 0x000004, 0x200000, BAD_DUMP CRC(54dc430b) SHA1(a2e55866249d01f6f2f2dd998421baf9fe0c6972) ) // physically damaged eprom

	ROM_REGION( 0x100000, "x1snd", 0 )  // Samples
	ROM_LOAD( "mp3_sound0__u111_v1.0.u111", 0x000000, 0x080000, CRC(711c915e) SHA1(d654a0c158cf54aab5faca913583c5620388aa46) )
	ROM_LOAD( "mp3_sound1__u112_v1.0.u112", 0x080000, 0x080000, CRC(27fd83cd) SHA1(d0261b2c5354ea17061e71bcea747d70efc18a49) )

	ROM_REGION( 0x117 * 2, "plds", 0 )
	ROM_LOAD( "kc-001c.u51", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "kc-002c.u52", 0x117, 0x117, NO_DUMP )
ROM_END

GAME( 1994, gundamex,  0,        gundamex, gundamex, seta2_state,    empty_init,    ROT0,   "Banpresto",             "Mobile Suit Gundam EX Revue",                         0 )

GAME( 1995, grdians,   0,        grdians,  grdians,  seta2_state,    empty_init,    ROT0,   "Winkysoft (Banpresto license)", "Guardians / Denjin Makai II (P-FG01-1 PCB)",  MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, grdiansa,  grdians,  grdiansa, grdians,  seta2_state,    empty_init,    ROT0,   "Winkysoft (Banpresto license)", "Guardians / Denjin Makai II (P0-113A PCB)",   MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, grdiansbl, grdians,  grdiansa, grdians,  seta2_state,    empty_init,    ROT0,   "bootleg (Intac Japan)",         "Guardians / Denjin Makai II (bootleg)",       MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )

GAME( 1996, mj4simai,  0,        mj4simai, mj4simai, mj4simai_state, empty_init,    ROT0,   "Maboroshi Ware",        "Wakakusamonogatari Mahjong Yonshimai (Japan)",        MACHINE_NO_COCKTAIL )

GAME( 1996, myangel,   0,        myangel,  myangel,  seta2_state,    empty_init,    ROT0,   "MOSS / Namco",          "Kosodate Quiz My Angel (Japan)",                      MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )

GAME( 1997, myangel2,  0,        myangel2, myangel2, seta2_state,    empty_init,    ROT0,   "MOSS / Namco",          "Kosodate Quiz My Angel 2 (Japan)",                    MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )

GAME( 1996, telpacfl,  0,        telpacfl, telpacfl, seta2_state,    empty_init,    ROT270, "Sunsoft",               "TelePachi Fever Lion (V1.0)",                         MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )

GAME( 1997, reelquak,  0,        reelquak, reelquak, seta2_state,    empty_init,    ROT0,   "<unknown>",             "Reel'N Quake! (Version 1.05)",                        MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )

GAME( 1999, endrichs,  0,        reelquak, endrichs, seta2_state,    empty_init,    ROT0,   "E.N.Tiger",             "Endless Riches (Ver 1.21)",                           MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, endrichsa, endrichs, reelquak, endrichs, seta2_state,    empty_init,    ROT0,   "E.N.Tiger",             "Endless Riches (Ver 1.20)",                           MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )

GAME( 1997, staraudi,  0,        staraudi, staraudi, staraudi_state, empty_init,    ROT0,   "Namco",                 "Star Audition",                                       MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // needs flipscreen hooking up properly with new code to function at all

GAME( 1999, pzlbowl,   0,        pzlbowl,  pzlbowl,  seta2_state,    empty_init,    ROT0,   "Nihon System / MOSS",   "Puzzle De Bowling (Japan)",                           MACHINE_NO_COCKTAIL )

GAME( 2000, penbros,   0,        penbros,  penbros,  seta2_state,    empty_init,    ROT0,   "Subsino",               "Penguin Brothers (Japan)",                            MACHINE_NO_COCKTAIL )
GAME( 2000, ablast,    penbros,  penbros,  penbros,  seta2_state,    empty_init,    ROT0,   "Subsino",               "Hong Tian Lei (A-Blast) (Japan)",                     MACHINE_NO_COCKTAIL ) // 轟天雷/Hōng tiān léi
GAME( 2000, ablastb,   penbros,  ablastb,  penbros,  seta2_state,    empty_init,    ROT0,   "bootleg",               "Hong Tian Lei (A-Blast) (bootleg)",                   MACHINE_NO_COCKTAIL | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND  ) // at least "tilemap sprite" scrolly flag differs, FPGA instead of x1-010

GAME( 2000, namcostr,  0,        namcostr, funcube,  seta2_state,    init_namcostr, ROT0,   "Namco",                 "Namco Stars",                                         MACHINE_NO_COCKTAIL | MACHINE_NOT_WORKING )

GAME( 2000, deerhunt,  0,        samshoot, deerhunt, seta2_state,    empty_init,    ROT0,   "Sammy USA Corporation", "Deer Hunting USA V4.3",                               MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2000, deerhunta, deerhunt, samshoot, deerhunt, seta2_state,    empty_init,    ROT0,   "Sammy USA Corporation", "Deer Hunting USA V4.2",                               MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2000, deerhuntb, deerhunt, samshoot, deerhunt, seta2_state,    empty_init,    ROT0,   "Sammy USA Corporation", "Deer Hunting USA V4.0",                               MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2000, deerhuntc, deerhunt, samshoot, deerhunt, seta2_state,    empty_init,    ROT0,   "Sammy USA Corporation", "Deer Hunting USA V3",                                 MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2000, deerhuntd, deerhunt, samshoot, deerhunt, seta2_state,    empty_init,    ROT0,   "Sammy USA Corporation", "Deer Hunting USA V2",                                 MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2000, deerhunte, deerhunt, samshoot, deerhunt, seta2_state,    empty_init,    ROT0,   "Sammy USA Corporation", "Deer Hunting USA V1",                                 MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2000, deerhuntj, deerhunt, samshoot, deerhunt, seta2_state,    empty_init,    ROT0,   "Sammy USA Corporation", "Deer Hunting USA V4.4.1 (Japan)",                     MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )

GAME( 2001, turkhunt,  0,        samshoot, turkhunt, seta2_state,    empty_init,    ROT0,   "Sammy USA Corporation", "Turkey Hunting USA V1.00",                            MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )

GAME( 2001, wschamp,   0,        samshoot, wschamp,  seta2_state,    empty_init,    ROT0,   "Sammy USA Corporation", "Wing Shooting Championship V2.00",                    MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2001, wschampa,  wschamp,  samshoot, wschamp,  seta2_state,    empty_init,    ROT0,   "Sammy USA Corporation", "Wing Shooting Championship V1.01",                    MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2001, wschampb,  wschamp,  samshoot, wschamp,  seta2_state,    empty_init,    ROT0,   "Sammy USA Corporation", "Wing Shooting Championship V1.00",                    MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )

GAME( 2002, trophyh,   0,        samshoot, trophyh,  seta2_state,    empty_init,    ROT0,   "Sammy USA Corporation", "Trophy Hunting - Bear & Moose V1.00",                 MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 2002, trophyht,  trophyh,  samshoot, trophyht, seta2_state,    empty_init,    ROT0,   "Sammy USA Corporation", "Trophy Hunting - Bear & Moose V1.00 (location test)", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )

GAME( 2000, funcube,   0,        funcube,  funcube,  funcube_state,  init_funcube,  ROT0,   "Namco",                 "Funcube (v1.5)",                                      MACHINE_NO_COCKTAIL )

GAME( 2001, funcube2,  0,        funcube2, funcube,  funcube_state,  init_funcube2, ROT0,   "Namco",                 "Funcube 2 (v1.1)",                                    MACHINE_NO_COCKTAIL )

GAME( 2001, funcube3,  0,        funcube2, funcube,  funcube_state,  init_funcube3, ROT0,   "Namco",                 "Funcube 3 (v1.1)",                                    MACHINE_NO_COCKTAIL )

GAME( 2001, funcube4,  0,        funcube2, funcube,  funcube_state,  init_funcube2, ROT0,   "Namco",                 "Funcube 4 (v1.0)",                                    MACHINE_NO_COCKTAIL )

GAME( 2002, funcube5,  0,        funcube2, funcube,  funcube_state,  init_funcube2, ROT0,   "Namco",                 "Funcube 5 (v1.0)",                                    MACHINE_NO_COCKTAIL )
