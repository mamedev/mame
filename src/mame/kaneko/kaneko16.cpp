// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/***************************************************************************

                            -= Kaneko 16 Bit Games =-

                    driver by   Luca Elia (l.elia@tin.it)


CPU    :  68000  +  MCU [Optional]

SOUND  :  OKI-M6295 x (1 | 2) + YM2149 x (0 | 2)
   OR  :  Z80  +  YM2151

OTHER  :  93C46 EEPROM [Optional]

CUSTOM :  VU-001 046A                  (48pin PQFP)
          VU-002 052 151021            (160pin PQFP)    <- Sprites
          VU-003 048 XJ009 (x3)        (40pin)          <- High Colour Background
          VIEW2-CHIP 23160-509 9047EAI (144pin PQFP)    <- Tilemaps
          MUX2-CHIP                    (64pin PQFP)
          HELP1-CHIP                   (64pin PQFP)
          IU-001 9045KP002             (44pin PQFP)
          I/O JAMMA MC-8282 047        (46pin)          <- Inputs


----------------------------------------------------------------------------------------
Year + Game                    PCB         Notes
----------------------------------------------------------------------------------------
91  The Berlin Wall            BW-002      3 x VU-003 (encrypted high colour background)
    Magical Crystals           Z00FC-02
92  Bakuretsu Breaker          ZOOFC-02
    Blaze On                   Z02AT-002   2 x VU-002 Sprites Chips (Atlus PCB ID: ATL-67140)
    Shogun Warriors            ZO1DK-002   CALC3 MCU protection (EEPROM handling, 68k code snippet, data - palettes, tilemaps, fighters)
    B.Rap Boys                 ZO1DK-002   CALC3 MCU protection (EEPROM handling, 68k code snippet, data - palettes, tilemaps, fighters)
93  Wing Force (prototype)     Z08AT-001   2 x VU-002 Sprites Chips, OKI sound
94  Great 1000 Miles Rally     Z09AF-005   TBSOP01 MCU protection (EEPROM handling etc.)
    Bonk's Adventure           Z09AF-003   TBSOP01 MCU protection (EEPROM handling, 68k code snippet, data)
    Blood Warrior              Z09AF-005   TBSOP01 MCU protection (EEPROM handling etc.)
    Pack'n Bang Bang           BW-002      (prototype)
95  Great 1000 Miles Rally 2   M201F00138  TBSOP02 MCU protection (EEPROM handling etc.)
----------------------------------------------------------------------------------------

Note: gtmr manual shows "Compatible with AX Kaneko System Board"
Note: Magic Crystals reports "TOYBOX SYSTEM Version 0.93B+"

Note: Decapping shows the CALC3 MCU to be a NEC uPD78322 series MCU with 16K internal ROM.  Both the
      TBSOP01 and TBSOP02 are thought to be the NEC uPD78324 series MCU with 32K internal ROM.

To Do:

[gtmr]
- Stage 4: The layers' scrolling is very jerky for a couple of seconds
  in the middle of this level (probably interrupt related)
- The layers' colours are not initialised when showing the self test
  screen and the very first screen (with the Kaneko logo in the middle).
  They're probably supposed to be disabled in those occasions, but the
  relevant registers aren't changed throughout the game (?)

[gtmr2]
- Finish the Inputs (different wheels and pedals);
- Implement LAN comms;
- Selecting Italian in the service mode settings causes no voice samples being played
  (with lots of OKI invalid samples in logerror), sound bank bug or btanb?

[brapboys / shogwarr]

- Verify collision protection on real hardware (appears to be a function of the system/memory controller, NOT the MCU)
  - currently B.Rap Boys has it's own special case, it should work with the same code as Blood Warriors / Shogun Warriors (and probably SuperNova)
- Figure out how MCU resets writeback address (currently hacked)
- Find relationship between Key tables and final datablock
- Clean up debug code file writes etc. (when above are done only!)
- Interrupt timing? (some sprites flicker) - some sprite flicker exists in youtube videos too, so probably not a bug
- Sprite buffering (1-2 frames?, or related to above) - some bg objects with sprite parts break up in shogun

Dip locations verified from manual for:

- berlwall
- mgcrystl
- blazeon
- bakubrkr
- bloodwar
- gtmr
- gtmr2
- shogwarr

[general]
- interrupt timing/behaviour

Non-Bugs (happen on real PCB)

[packbang]
 - Game crashes or gives you a corrupt stage if you attempt to continue on a bonus stage (due to buggy prototype code)
 - Background fading appears inverted, fading to a black screen during levels, this is correct
 - Some screens shown on the flyer (enemy select etc.) are never shown, the flyer probably shows a different version
   of the game.  The backgrounds used can be viewed in test mode.

***************************************************************************/

#include "emu.h"
#include "kaneko16.h"

#include "speaker.h"


/***************************************************************************


                            Machine Initialisation


***************************************************************************/



MACHINE_RESET_MEMBER(kaneko16_state,gtmr)
{
	m_VIEW2_2_pri = 1;
}

MACHINE_RESET_MEMBER(kaneko16_state,mgcrystl)
{
	m_VIEW2_2_pri = 0;
}


/***************************************************************************


                        Misc Machine Emulation Routines


***************************************************************************/


void kaneko16_state::coin_lockout_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT( data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT( data, 1));
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 2));
	machine().bookkeeping().coin_lockout_w(1, BIT(~data, 3));
}

void kaneko16_state::bloodwar_coin_lockout_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
	machine().bookkeeping().coin_lockout_w(0, BIT(data, 7));
	machine().bookkeeping().coin_lockout_w(1, BIT(data, 7));
}




/***************************************************************************


                                    Sound


***************************************************************************/

/* Two identically mapped YM2149 chips */
template<unsigned Chip>
u16 kaneko16_state::ym2149_r(offs_t offset)
{
	/* Each 2149 register is mapped to a different address */
	m_ym2149[Chip]->address_w(offset);
	return m_ym2149[Chip]->data_r();
}


template<unsigned Chip>
void kaneko16_state::ym2149_w(offs_t offset, u16 data, u16 mem_mask)
{
	/* Each 2149 register is mapped to a different address */
	m_ym2149[Chip]->address_w(offset);
	/* The registers are mapped to odd addresses, except one! */
	if (ACCESSING_BITS_0_7) m_ym2149[Chip]->data_w( data       & 0xff);
	else                m_ym2149[Chip]->data_w((data >> 8) & 0xff);
}

template<unsigned Mask>
void kaneko16_state::oki_bank0_w(u8 data)
{
	m_okibank[0]->set_entry(data & Mask);
//  logerror("%s:Selecting OKI1 bank %02X\n",machine().describe_context(),data&0xff);
}

template<unsigned Mask>
void kaneko16_state::oki_bank1_w(u8 data)
{
	m_okibank[1]->set_entry(data & Mask);
//  logerror("%s:Selecting OKI2 bank %02X\n",machine().describe_context(),data&0xff);
}

/***************************************************************************


                                    EEPROM


***************************************************************************/

void kaneko16_state::eeprom_w(u8 data)
{
	m_eepromout_port->write(data, 0xff);
}

u8 kaneko16_state::eeprom_r()
{
	return m_eeprom->do_read();
}

void kaneko16_state::eeprom_cs_w(u8 data)
{
	m_eeprom->cs_write(data);
}

/***************************************************************************


                            Memory Maps - Main CPU


***************************************************************************/

/***************************************************************************
                                The Berlin Wall
***************************************************************************/

void kaneko16_berlwall_state::berlwall_oki_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (mem_mask == 0xff00) // reads / writes to the upper byte only appear to act as a mirror to the lower byte, 16-bit reads/writes only access the lower byte.
	{
		data >>= 8;
		mem_mask >>= 8;
	}

	m_oki[0]->write(data);
}

u16 kaneko16_berlwall_state::berlwall_spriteram_r(offs_t offset)
{
	offset = bitswap<16>(offset, 15, 14, 13, 12, 2, 11, 10, 9, 8, 7, 6, 5, 4, 3, 1, 0);
	offset ^= 0x800;
	return m_spriteram->live()[offset];
}

void kaneko16_berlwall_state::berlwall_spriteram_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset = bitswap<16>(offset, 15, 14, 13, 12, 2, 11, 10, 9, 8, 7, 6, 5, 4, 3, 1, 0);
	offset ^= 0x800;
	COMBINE_DATA(&m_spriteram->live()[offset]);
}

u16 kaneko16_berlwall_state::berlwall_spriteregs_r(offs_t offset)
{
	if (offset & 0x4)
		return 0;
	offset = bitswap<8>(offset, 7, 6, 5, 2, 4, 3, 1, 0);
	return m_kaneko_spr->regs_r(offset);
}

void kaneko16_berlwall_state::berlwall_spriteregs_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (offset & 0x4)
		return;
	offset = bitswap<8>(offset, 7, 6, 5, 2, 4, 3, 1, 0);
	m_kaneko_spr->regs_w(offset, data, mem_mask);
}

void kaneko16_berlwall_state::berlwall_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();     // ROM
	map(0x200000, 0x20ffff).ram();     // Work RAM
	map(0x30e000, 0x30ffff).rw(FUNC(kaneko16_berlwall_state::berlwall_spriteram_r), FUNC(kaneko16_berlwall_state::berlwall_spriteram_w)).share("spriteram");       // Sprites (scrambled RAM)
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");                // Palette
	map(0x480000, 0x480001).ram().share("bg15_scroll");                                                       // High Color Background
	map(0x500001, 0x500001).rw(FUNC(kaneko16_berlwall_state::bg15_bright_r), FUNC(kaneko16_berlwall_state::bg15_bright_w)); // ""
	map(0x580001, 0x580001).rw(FUNC(kaneko16_berlwall_state::bg15_select_r), FUNC(kaneko16_berlwall_state::bg15_select_w)); // ""
	map(0x600000, 0x60003f).rw(FUNC(kaneko16_berlwall_state::berlwall_spriteregs_r), FUNC(kaneko16_berlwall_state::berlwall_spriteregs_w));                           // Sprite Regs (scrambled RAM)
	map(0x680000, 0x680001).portr("P1");
	map(0x680002, 0x680003).portr("P2");
	map(0x680004, 0x680005).portr("SYSTEM");
//  map(0x680006, 0x680007).portr("UNK");
	map(0x700000, 0x700000).w(FUNC(kaneko16_berlwall_state::coin_lockout_w));  // Coin Lockout
	map(0x780000, 0x780001).r(m_watchdog, FUNC(watchdog_timer_device::reset16_r));
	map(0x800000, 0x80001f).rw(FUNC(kaneko16_berlwall_state::ym2149_r<0>), FUNC(kaneko16_berlwall_state::ym2149_w<0>)); // Sound
	map(0x800200, 0x80021f).rw(FUNC(kaneko16_berlwall_state::ym2149_r<1>), FUNC(kaneko16_berlwall_state::ym2149_w<1>));
	map(0x8003fe, 0x8003ff).noprw(); // for OKI when accessed as .l
	map(0x800400, 0x800401).r(m_oki[0], FUNC(okim6295_device::read)).w(FUNC(kaneko16_berlwall_state::berlwall_oki_w));
	map(0xc00000, 0xc03fff).m(m_view2[0], FUNC(kaneko_view2_tilemap_device::vram_map));
	map(0xd00000, 0xd0001f).rw(m_view2[0], FUNC(kaneko_view2_tilemap_device::regs_r), FUNC(kaneko_view2_tilemap_device::regs_w));
}


/***************************************************************************
                            Bakuretsu Breaker
***************************************************************************/

/* Two un-assigned OKI samples get requested during game play. Leftover bug? */
/* The two YM2149 chips are only used when entering high score initials, and */
/* when the game is fully completed. Overkill??? */

void kaneko16_state::bakubrkr_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();     // ROM
	map(0x100000, 0x10ffff).ram();     // Work RAM
	map(0x400000, 0x40001f).rw(FUNC(kaneko16_state::ym2149_r<0>), FUNC(kaneko16_state::ym2149_w<0>)); // Sound
	map(0x400200, 0x40021f).rw(FUNC(kaneko16_state::ym2149_r<1>), FUNC(kaneko16_state::ym2149_w<1>));          // Sound
	map(0x400401, 0x400401).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));  //
	map(0x500000, 0x503fff).m(m_view2[0], FUNC(kaneko_view2_tilemap_device::vram_map));
	map(0x580000, 0x583fff).m(m_view2[1], FUNC(kaneko_view2_tilemap_device::vram_map));
	map(0x600000, 0x601fff).ram().share("spriteram");                   // Sprites
	map(0x700000, 0x700fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    // Palette
	map(0x800000, 0x80001f).rw(m_view2[0], FUNC(kaneko_view2_tilemap_device::regs_r), FUNC(kaneko_view2_tilemap_device::regs_w));
	map(0x900000, 0x90001f).rw(m_kaneko_spr, FUNC(kaneko16_sprite_device::regs_r), FUNC(kaneko16_sprite_device::regs_w));
	map(0xa80000, 0xa80001).r("watchdog", FUNC(watchdog_timer_device::reset16_r));
	map(0xb00000, 0xb0001f).rw(m_view2[1], FUNC(kaneko_view2_tilemap_device::regs_r), FUNC(kaneko_view2_tilemap_device::regs_w));
	map(0xd00000, 0xd00000).w(FUNC(kaneko16_state::bloodwar_coin_lockout_w));  // Coin Lockout
	map(0xd00001, 0xd00001).w(FUNC(kaneko16_state::eeprom_w));    // EEPROM
	map(0xe00000, 0xe00001).portr("P1");
	map(0xe00002, 0xe00003).portr("P2");
	map(0xe00004, 0xe00005).portr("SYSTEM");
	map(0xe00006, 0xe00007).portr("UNK");
}


/***************************************************************************
                                    Blaze On
***************************************************************************/

void kaneko16_state::blazeon_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();     // ROM
	map(0x300000, 0x30ffff).ram();     // Work RAM
	map(0x500000, 0x500fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    // Palette
	map(0x600000, 0x603fff).m(m_view2[0], FUNC(kaneko_view2_tilemap_device::vram_map));
	map(0x700000, 0x700fff).ram().share("spriteram");                   // Sprites
	map(0x800000, 0x80001f).rw(m_view2[0], FUNC(kaneko_view2_tilemap_device::regs_r), FUNC(kaneko_view2_tilemap_device::regs_w));
	map(0x900000, 0x90001f).rw(m_kaneko_spr, FUNC(kaneko16_sprite_device::regs_r), FUNC(kaneko16_sprite_device::regs_w));
	map(0x980000, 0x98001f).ram();                                                                             // Sprites Regs #2
	map(0xc00000, 0xc00001).portr("DSW2_P1");
	map(0xc00002, 0xc00003).portr("DSW1_P2");
	map(0xc00004, 0xc00005).portr("UNK");
	map(0xc00006, 0xc00007).portr("SYSTEM");
	map(0xd00000, 0xd00000).w(FUNC(kaneko16_state::coin_lockout_w));  // Coin Lockout
	map(0xe00000, 0xe00001).nopr(); // Read = IRQ Ack ?
	map(0xe00000, 0xe00000).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xe40000, 0xe40001).nopr(); // IRQ Ack ?
//  map(0xe80000, 0xe80001).nopr(); // IRQ Ack ?
	map(0xec0000, 0xec0001).nopr(); // Lev 4 IRQ Ack ?
}


/***************************************************************************
                                Blood Warrior
***************************************************************************/

void kaneko16_gtmr_state::bloodwar_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();     // ROM
	map(0x100000, 0x10ffff).ram();     // Work RAM
	map(0x200000, 0x20ffff).ram().share("mcuram");
	map(0x2a0000, 0x2a0001).w(m_toybox, FUNC(kaneko_toybox_device::mcu_com0_w));    // To MCU ?
	map(0x2b0000, 0x2b0001).w(m_toybox, FUNC(kaneko_toybox_device::mcu_com1_w));
	map(0x2c0000, 0x2c0001).w(m_toybox, FUNC(kaneko_toybox_device::mcu_com2_w));
	map(0x2d0000, 0x2d0001).w(m_toybox, FUNC(kaneko_toybox_device::mcu_com3_w));
	map(0x300000, 0x30ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    // Palette
	map(0x400000, 0x401fff).ram().share("spriteram");                   // Sprites
	map(0x500000, 0x503fff).m(m_view2[0], FUNC(kaneko_view2_tilemap_device::vram_map));
	map(0x580000, 0x583fff).m(m_view2[1], FUNC(kaneko_view2_tilemap_device::vram_map));
	map(0x600000, 0x60001f).rw(m_view2[0], FUNC(kaneko_view2_tilemap_device::regs_r), FUNC(kaneko_view2_tilemap_device::regs_w));
	map(0x680000, 0x68001f).rw(m_view2[1], FUNC(kaneko_view2_tilemap_device::regs_r), FUNC(kaneko_view2_tilemap_device::regs_w));
	map(0x700000, 0x70001f).rw(m_kaneko_spr, FUNC(kaneko16_sprite_device::regs_r), FUNC(kaneko16_sprite_device::regs_w));
	map(0x800001, 0x800001).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x880001, 0x880001).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x900000, 0x900039).rw(m_kaneko_hit, FUNC(kaneko_hit_device::kaneko_hit_r), FUNC(kaneko_hit_device::kaneko_hit_w));
	map(0xa00000, 0xa00001).rw(m_watchdog, FUNC(watchdog_timer_device::reset16_r), FUNC(watchdog_timer_device::reset16_w));
	map(0xb00000, 0xb00001).portr("P1");
	map(0xb00002, 0xb00003).portr("P2");
	map(0xb00004, 0xb00005).portr("SYSTEM");
	map(0xb00006, 0xb00007).portr("EXTRA");
	map(0xb80000, 0xb80000).w(FUNC(kaneko16_gtmr_state::bloodwar_coin_lockout_w));  // Coin Lockout
	map(0xc00000, 0xc00001).w(FUNC(kaneko16_gtmr_state::display_enable_w));
	map(0xd00000, 0xd00001).r(m_toybox, FUNC(kaneko_toybox_device::mcu_status_r));
	map(0xe00001, 0xe00001).w(FUNC(kaneko16_gtmr_state::oki_bank0_w<0xf>));
	map(0xe80001, 0xe80001).w(FUNC(kaneko16_gtmr_state::oki_bank1_w<0x3>));
}


/***************************************************************************
                                Bonk's Adventure
***************************************************************************/

void kaneko16_gtmr_state::bonkadv_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();     // ROM
	map(0x100000, 0x10ffff).ram();     // Work RAM
	map(0x200000, 0x20ffff).ram().share("mcuram");      // Shared With MCU
	map(0x2a0000, 0x2a0001).w(m_toybox, FUNC(kaneko_toybox_device::mcu_com0_w));    // To MCU ?
	map(0x2b0000, 0x2b0001).w(m_toybox, FUNC(kaneko_toybox_device::mcu_com1_w));
	map(0x2c0000, 0x2c0001).w(m_toybox, FUNC(kaneko_toybox_device::mcu_com2_w));
	map(0x2d0000, 0x2d0001).w(m_toybox, FUNC(kaneko_toybox_device::mcu_com3_w));
	map(0x300000, 0x30ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    // Palette
	map(0x400000, 0x401fff).ram().share("spriteram");                   // Sprites
	map(0x500000, 0x503fff).m(m_view2[0], FUNC(kaneko_view2_tilemap_device::vram_map));
	map(0x580000, 0x583fff).m(m_view2[1], FUNC(kaneko_view2_tilemap_device::vram_map));
	map(0x600000, 0x60001f).rw(m_view2[0], FUNC(kaneko_view2_tilemap_device::regs_r), FUNC(kaneko_view2_tilemap_device::regs_w));
	map(0x680000, 0x68001f).rw(m_view2[1], FUNC(kaneko_view2_tilemap_device::regs_r), FUNC(kaneko_view2_tilemap_device::regs_w));
	map(0x700000, 0x70001f).rw(m_kaneko_spr, FUNC(kaneko16_sprite_device::regs_r), FUNC(kaneko16_sprite_device::regs_w));
	map(0x800001, 0x800001).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x880001, 0x880001).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x900000, 0x900015).rw(m_kaneko_hit, FUNC(kaneko_hit_device::kaneko_hit_r), FUNC(kaneko_hit_device::kaneko_hit_w));
	map(0xa00000, 0xa00001).rw(m_watchdog, FUNC(watchdog_timer_device::reset16_r), FUNC(watchdog_timer_device::reset16_w));   // Watchdog
	map(0xb00000, 0xb00001).portr("P1");
	map(0xb00002, 0xb00003).portr("P2");
	map(0xb00004, 0xb00005).portr("SYSTEM");
	map(0xb00006, 0xb00007).portr("UNK");
	map(0xb80000, 0xb80000).w(FUNC(kaneko16_gtmr_state::bloodwar_coin_lockout_w));  // Coin Lockout
	map(0xc00000, 0xc00001).w(FUNC(kaneko16_gtmr_state::display_enable_w));
	map(0xd00000, 0xd00001).r(m_toybox, FUNC(kaneko_toybox_device::mcu_status_r));
	map(0xe00001, 0xe00001).w(FUNC(kaneko16_gtmr_state::oki_bank0_w<0xf>));
	map(0xe80001, 0xe80001).w(FUNC(kaneko16_gtmr_state::oki_bank1_w<0xf>));
}


/***************************************************************************
                            Great 1000 Miles Rally
***************************************************************************/


u16 kaneko16_gtmr_state::gtmr_wheel_r()
{
	// check 'Controls' dip switch
	switch (m_dsw_port->read() & 0x1000)
	{
		case 0x0000:    // 'Both Sides' = 270deg Wheel
			return  (m_wheel_port[0]->read());
		case 0x1000:    // '1P Side' = 360' Wheel
			return  (m_wheel_port[1]->read());
		default:
			return  (0);
	}
}

void kaneko16_gtmr_state::gtmr_map(address_map &map)
{
	map(0x000000, 0x0ffffd).rom();                                                                     // ROM
	map(0x0ffffe, 0x0fffff).r(FUNC(kaneko16_gtmr_state::gtmr_wheel_r));                                                      // Wheel Value

	map(0x100000, 0x10ffff).ram();                                                                     // Work RAM
	map(0x200000, 0x20ffff).ram().share(m_mcuram);                                          // Shared With MCU

	map(0x2a0000, 0x2a0001).w(m_toybox, FUNC(kaneko_toybox_device::mcu_com0_w));                                                // To MCU ?
	map(0x2b0000, 0x2b0001).w(m_toybox, FUNC(kaneko_toybox_device::mcu_com1_w));
	map(0x2c0000, 0x2c0001).w(m_toybox, FUNC(kaneko_toybox_device::mcu_com2_w));
	map(0x2d0000, 0x2d0001).w(m_toybox, FUNC(kaneko_toybox_device::mcu_com3_w));

	map(0x300000, 0x30ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    // Palette
	map(0x310000, 0x327fff).ram();                                                                     //
	map(0x400000, 0x401fff).ram().share("spriteram");                       // Sprites

	map(0x500000, 0x503fff).m(m_view2[0], FUNC(kaneko_view2_tilemap_device::vram_map));
	map(0x580000, 0x583fff).m(m_view2[1], FUNC(kaneko_view2_tilemap_device::vram_map));

	map(0x600000, 0x60000f).rw(m_view2[0], FUNC(kaneko_view2_tilemap_device::regs_r), FUNC(kaneko_view2_tilemap_device::regs_w));
	map(0x680000, 0x68001f).rw(m_view2[1], FUNC(kaneko_view2_tilemap_device::regs_r), FUNC(kaneko_view2_tilemap_device::regs_w));

	map(0x700000, 0x70001f).rw(m_kaneko_spr, FUNC(kaneko16_sprite_device::regs_r), FUNC(kaneko16_sprite_device::regs_w));

	map(0x800001, 0x800001).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));                 // Samples
	map(0x880001, 0x880001).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x900000, 0x900039).rw(m_kaneko_hit, FUNC(kaneko_hit_device::kaneko_hit_r), FUNC(kaneko_hit_device::kaneko_hit_w)); // only used for random number
	map(0xa00000, 0xa00001).rw(m_watchdog, FUNC(watchdog_timer_device::reset16_r), FUNC(watchdog_timer_device::reset16_w));

	map(0xb00000, 0xb00001).portr("P1");
	map(0xb00002, 0xb00003).portr("P2");
	map(0xb00004, 0xb00005).portr("SYSTEM");
	map(0xb00006, 0xb00007).portr("UNK");
	map(0xb80000, 0xb80000).w(FUNC(kaneko16_gtmr_state::coin_lockout_w));                                          // Coin Lockout
	map(0xc00000, 0xc00001).w(FUNC(kaneko16_gtmr_state::display_enable_w));                                          // might do more than that

	map(0xd00000, 0xd00001).r(m_toybox, FUNC(kaneko_toybox_device::mcu_status_r));

	map(0xe00001, 0xe00001).w(FUNC(kaneko16_gtmr_state::oki_bank0_w<0xf>));                                        // Samples Bankswitching
	map(0xe80001, 0xe80001).w(FUNC(kaneko16_gtmr_state::oki_bank1_w<0x1>));
}

/***************************************************************************
                            Great 1000 Miles Rally 2
***************************************************************************/


u16 kaneko16_gtmr_state::gtmr2_wheel_r()
{
	switch (m_dsw_port->read() & 0x1800)
	{
		case 0x0000:    // 270' A. Wheel
			return  (m_wheel_port[0]->read());
		case 0x1000:    // 270' D. Wheel
			return  (m_wheel_port[1]->read() << 8);
		case 0x0800:    // 360' Wheel
			return  (m_wheel_port[2]->read() << 8);
		default:
			logerror("gtmr2_wheel_r : read at %06x with joystick\n", m_maincpu->pc());
			return  (~0);
	}
}

u16 kaneko16_gtmr_state::gtmr2_IN1_r()
{
	return  (m_p2_port->read() & (m_fake_port->read() | ~0x7100));
}

void kaneko16_gtmr_state::gtmr2_map(address_map &map)
{
	map(0x000000, 0x0ffffd).rom(); // ROM
	map(0x0ffffe, 0x0fffff).r(FUNC(kaneko16_gtmr_state::gtmr2_wheel_r)); // Wheel Value

	map(0x100000, 0x10ffff).ram(); // Work RAM
	map(0x200000, 0x20ffff).ram().share(m_mcuram); // Shared With MCU

	map(0x2a0000, 0x2a0001).w(m_toybox, FUNC(kaneko_toybox_device::mcu_com0_w));    // To MCU ?
	map(0x2b0000, 0x2b0001).w(m_toybox, FUNC(kaneko_toybox_device::mcu_com1_w));
	map(0x2c0000, 0x2c0001).w(m_toybox, FUNC(kaneko_toybox_device::mcu_com2_w));
	map(0x2d0000, 0x2d0001).w(m_toybox, FUNC(kaneko_toybox_device::mcu_com3_w));

	map(0x300000, 0x30ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    // Palette
	map(0x310000, 0x327fff).ram(); //
	map(0x400000, 0x401fff).ram().share("spriteram"); // Sprites

	map(0x500000, 0x503fff).m(m_view2[0], FUNC(kaneko_view2_tilemap_device::vram_map));
	map(0x580000, 0x583fff).m(m_view2[1], FUNC(kaneko_view2_tilemap_device::vram_map));
	map(0x600000, 0x60000f).rw(m_view2[0], FUNC(kaneko_view2_tilemap_device::regs_r), FUNC(kaneko_view2_tilemap_device::regs_w));
	map(0x680000, 0x68001f).rw(m_view2[1], FUNC(kaneko_view2_tilemap_device::regs_r), FUNC(kaneko_view2_tilemap_device::regs_w));

	map(0x700000, 0x70001f).rw(m_kaneko_spr, FUNC(kaneko16_sprite_device::regs_r), FUNC(kaneko16_sprite_device::regs_w));
	map(0x800001, 0x800001).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // Samples
	map(0x880001, 0x880001).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x900000, 0x900039).rw(m_kaneko_hit, FUNC(kaneko_hit_device::kaneko_hit_r), FUNC(kaneko_hit_device::kaneko_hit_w)); // only used for random number
	map(0xa00000, 0xa00001).rw(m_watchdog, FUNC(watchdog_timer_device::reset16_r), FUNC(watchdog_timer_device::reset16_w));   // Watchdog

	map(0xb00000, 0xb00001).portr("P1");
//  map(0xb00002, 0xb00003).portr("P2");
	map(0xb00002, 0xb00003).r(FUNC(kaneko16_gtmr_state::gtmr2_IN1_r));
	map(0xb00004, 0xb00005).portr("SYSTEM");
	map(0xb00006, 0xb00007).portr("EXTRA");
	map(0xb80000, 0xb80000).w(FUNC(kaneko16_gtmr_state::coin_lockout_w));  // Coin Lockout
	map(0xc00000, 0xc00001).w(FUNC(kaneko16_gtmr_state::display_enable_w));  // might do more than that

	map(0xd00000, 0xd00001).r(m_toybox, FUNC(kaneko_toybox_device::mcu_status_r));

	map(0xe00001, 0xe00001).w(FUNC(kaneko16_gtmr_state::oki_bank0_w<0xf>));    // Samples Bankswitching
	map(0xe80001, 0xe80001).w(FUNC(kaneko16_gtmr_state::oki_bank1_w<0x1>));
}

/***************************************************************************
                                Magical Crystal
***************************************************************************/

void kaneko16_state::mgcrystl_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();     // ROM
	map(0x300000, 0x30ffff).ram();     // Work RAM
	map(0x400000, 0x40001f).rw(FUNC(kaneko16_state::ym2149_r<0>), FUNC(kaneko16_state::ym2149_w<0>)); // Sound
	map(0x400200, 0x40021f).rw(FUNC(kaneko16_state::ym2149_r<1>), FUNC(kaneko16_state::ym2149_w<1>));
	map(0x400401, 0x400401).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x500000, 0x500fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    // Palette
	map(0x600000, 0x603fff).m(m_view2[0], FUNC(kaneko_view2_tilemap_device::vram_map));
	map(0x680000, 0x683fff).m(m_view2[1], FUNC(kaneko_view2_tilemap_device::vram_map));
	map(0x700000, 0x701fff).ram().share("spriteram");                   // Sprites
	map(0x800000, 0x80001f).rw(m_view2[0], FUNC(kaneko_view2_tilemap_device::regs_r), FUNC(kaneko_view2_tilemap_device::regs_w));
	map(0x900000, 0x90001f).rw(m_kaneko_spr, FUNC(kaneko16_sprite_device::regs_r), FUNC(kaneko16_sprite_device::regs_w));
	map(0xb00000, 0xb0001f).rw(m_view2[1], FUNC(kaneko_view2_tilemap_device::regs_r), FUNC(kaneko_view2_tilemap_device::regs_w));
	map(0xa00000, 0xa00001).r(m_watchdog, FUNC(watchdog_timer_device::reset16_r));    // Watchdog
	map(0xc00000, 0xc00001).portr("DSW_P1");
	map(0xc00002, 0xc00003).portr("P2");
	map(0xc00004, 0xc00005).portr("SYSTEM");
	map(0xd00000, 0xd00000).w(FUNC(kaneko16_state::bloodwar_coin_lockout_w));  // Coin Lockout
	map(0xd00001, 0xd00001).w(FUNC(kaneko16_state::eeprom_w));    // EEPROM
}



/***************************************************************************
                                Shogun Warriors
***************************************************************************/

void kaneko16_shogwarr_state::shogwarr_oki_bank_w(u8 data)
{
	m_okibank[0]->set_entry((data >> 4) & 0xf);
	m_okibank[1]->set_entry((data & 0xf));
}

void kaneko16_shogwarr_state::shogwarr_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();     // ROM
	map(0x100000, 0x10ffff).ram().share(m_mainram);     // Work RAM
	map(0x200000, 0x20ffff).ram().share(m_mcuram);
	map(0x280000, 0x280001).w(m_calc3_prot, FUNC(kaneko_calc3_device::mcu_com0_w));
	map(0x290000, 0x290001).w(m_calc3_prot, FUNC(kaneko_calc3_device::mcu_com1_w));
	map(0x2b0000, 0x2b0001).w(m_calc3_prot, FUNC(kaneko_calc3_device::mcu_com2_w));
	//map(0x2c0000, 0x2c0001) // run calc 3? or irq ack?
	map(0x2d0000, 0x2d0001).w(m_calc3_prot, FUNC(kaneko_calc3_device::mcu_com3_w));
	map(0x380000, 0x380fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    // Palette
	map(0x400001, 0x400001).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write)); // Samples
	map(0x480001, 0x480001).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x580000, 0x581fff).ram().share("spriteram");                   // Sprites
	map(0x600000, 0x603fff).m(m_view2[0], FUNC(kaneko_view2_tilemap_device::vram_map));
	map(0x800000, 0x80001f).rw(m_view2[0], FUNC(kaneko_view2_tilemap_device::regs_r), FUNC(kaneko_view2_tilemap_device::regs_w));
	map(0x900000, 0x90001f).rw(m_kaneko_spr, FUNC(kaneko16_sprite_device::regs_r), FUNC(kaneko16_sprite_device::regs_w));
	map(0xa00000, 0xa0007f).rw(m_kaneko_hit, FUNC(kaneko_hit_device::kaneko_hit_r), FUNC(kaneko_hit_device::kaneko_hit_w));
	map(0xa80000, 0xa80001).rw(m_watchdog, FUNC(watchdog_timer_device::reset16_r), FUNC(watchdog_timer_device::reset16_w));
	map(0xb80000, 0xb80001).portr("P1");
	map(0xb80002, 0xb80003).portr("P2");
	map(0xb80004, 0xb80005).portr("SYSTEM");
	map(0xb80006, 0xb80007).portr("UNK");
	map(0xd00000, 0xd00001).noprw();                         // ? (bit 0)
	map(0xe00001, 0xe00001).w(FUNC(kaneko16_shogwarr_state::shogwarr_oki_bank_w));  // Samples Bankswitching
}

/***************************************************************************


                            Memory Maps - Sound CPU


***************************************************************************/

/***************************************************************************
                                    Blaze On
***************************************************************************/

void kaneko16_state::blazeon_soundmem(address_map &map)
{
	map(0x0000, 0x7fff).rom();     // ROM
	map(0x8000, 0xbfff).rom();     // ROM (supposed to be banked?)
	map(0xc000, 0xdfff).ram();     // RAM
	//map(0xe000, 0xe001).noprw(); // Unknown read / writes in boot
}

void kaneko16_state::blazeon_soundport(address_map &map)
{
	map.global_mask(0xff);
	map(0x02, 0x03).rw(m_ymsnd, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x06, 0x06).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

/***************************************************************************
                                 Wing Force
***************************************************************************/

void kaneko16_state::wingforc_soundport(address_map &map)
{
	map.global_mask(0xff);
//  map(0x00, 0x00) // 02 written at boot
	map(0x02, 0x03).rw(m_ymsnd, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x06, 0x06).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x0a, 0x0a).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x0c, 0x0c).w(FUNC(kaneko16_state::oki_bank0_w<0x3>));
}


/***************************************************************************


                                Input Ports


***************************************************************************/

/***************************************************************************
                            Bakuretsu Breaker
***************************************************************************/

static INPUT_PORTS_START( bakubrkr )
	PORT_START("P1")    /* e00000.w */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0002, IP_ACTIVE_LOW, "SW1:2" )
	/* All other game settings done through the test mode */
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW1:3" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW1:4" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW1:5" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW1:6" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW1:7" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW1:8" ) /* Listed as "Unused" */

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")        /* e00002.b */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* e00004.b */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_SERVICE_NO_TOGGLE( 0x1000, IP_ACTIVE_LOW )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT ) // pause
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK")       /* Seems unused ! - e00006.b */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
INPUT_PORTS_END


/***************************************************************************
                            The Berlin Wall
***************************************************************************/

static INPUT_PORTS_START( berlwall )
	PORT_START("P1")        /* 680000.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")        /* 680002.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* 680000.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_SERVICE_NO_TOGGLE( 0x1000, IP_ACTIVE_LOW )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK")       /* ? - 680006.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")      /*$200018.b <- ! $80001d.b */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW1:2" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")      /* $200019.b <- $80001f.b */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")    // 1p lives at 202982.b
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x30, 0x30, "Country" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "England" )
	PORT_DIPSETTING(    0x20, "Italy" )
	PORT_DIPSETTING(    0x10, "Germany" )
	PORT_DIPSETTING(    0x00, "Freeze Screen" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END


/***************************************************************************
                            The Berlin Wall (bootleg ?)
***************************************************************************/

//  Same as berlwall, but for a different lives setting

static INPUT_PORTS_START( berlwallt )
	PORT_INCLUDE(berlwall)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "5" )
INPUT_PORTS_END


/***************************************************************************
                            The Berlin Wall (Korea)
***************************************************************************/

//  Same as berlwallt, but no country setting

static INPUT_PORTS_START( berlwallk )
	PORT_INCLUDE(berlwallt)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x30, 0x30, "Pause Mode" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "Off (1)" )
	PORT_DIPSETTING(    0x20, "Off (2)" )
	PORT_DIPSETTING(    0x10, "Off (3)" )
	PORT_DIPSETTING(    0x00, "Freeze Screen" )
INPUT_PORTS_END


/***************************************************************************
                                    Blaze On
***************************************************************************/

static INPUT_PORTS_START( blazeon )
	PORT_START("DSW2_P1")       /* c00000.w */
	PORT_DIPNAME( 0x0003,  0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(       0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(       0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(       0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(       0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c,  0x000c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(       0x0000, "2" )
	PORT_DIPSETTING(       0x000c, "3" )
	PORT_DIPSETTING(       0x0008, "4" )
	PORT_DIPSETTING(       0x0004, "5" )
	PORT_DIPNAME( 0x0010,  0x0010, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(       0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x0010, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" ) /* Listed as "Unused" */
	PORT_SERVICE_DIPLOC(   0x0080, IP_ACTIVE_LOW, "SW2:8" )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("DSW1_P2")       /* c00002.w */
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(      0x0007, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0005, "6 Coins/3 Credits" )
	PORT_DIPSETTING(      0x0009, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, "5 Coins/6 Credits" )
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_3C ) )
//  PORT_DIPSETTING(      0x0001, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )

	PORT_DIPNAME( 0x00f0, 0x00f0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(      0x0070, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0050, "6 Coins/3 Credits" )
	PORT_DIPSETTING(      0x0090, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x00f0, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, "5 Coins/6 Credits" )
	PORT_DIPSETTING(      0x0020, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_3C ) )
//  PORT_DIPSETTING(      0x0010, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00d0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x00b0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_6C ) )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)

	PORT_START("UNK")           /* ? - c00004.w */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused?

	PORT_START("SYSTEM")        /* c00006.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x2000, IP_ACTIVE_LOW )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE1 )
INPUT_PORTS_END

/***************************************************************************
                                 Wing Force
***************************************************************************/

static INPUT_PORTS_START( wingforc )
	PORT_START("DSW2_P1")       /* c00000.w */
	// The game reads and stores these the same as Blaze On.
	// However, none of them actually get used. Lives does work if you patch out the
	// code that actively zeroes out the value it reads from here.
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SW1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0002, "SW1:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW1:8" )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("DSW1_P2")       /* c00002.w */
	// In a similar story to DSW2, these are read and then forced to zero.
	// These are credit selections with the same values as Blaze On.
	// These work if you remove the code to force to zero, but they don't
	// give any feedback unless the coin increases the credit counter.
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0002, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW2:8" )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)

	PORT_START("UNK")           /* ? - c00004.w */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused?

	PORT_START("SYSTEM")        /* c00006.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x2000, IP_ACTIVE_LOW )     // unused
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_TILT )         // unused
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE1 )
INPUT_PORTS_END

/***************************************************************************
                                Blood Warrior
***************************************************************************/

static INPUT_PORTS_START( bloodwar )
	PORT_START("P1")        /* b0000.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START("P2")        /* b0002.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")        /* b0004.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_SERVICE_NO_TOGGLE( 0x1000, IP_ACTIVE_LOW )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE2 ) // tested

	PORT_START("EXTRA")         /* ? - b0006.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )  // tested
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)   // tested
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")          /* from the MCU - $10497e.b <- $208000.b */
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0200, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x3800, "1 Easy" )
	PORT_DIPSETTING(      0x3000, "2" )
	PORT_DIPSETTING(      0x2800, "3" )
	PORT_DIPSETTING(      0x2000, "4" )
	PORT_DIPSETTING(      0x1800, "5" )
	PORT_DIPSETTING(      0x1000, "6" )
	PORT_DIPSETTING(      0x0800, "7" )
	PORT_DIPSETTING(      0x0000, "8 Hard" )
	PORT_DIPNAME( 0x4000, 0x4000, "Can Join During Game" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Start 2 Credits/Continue 1 Credit" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
INPUT_PORTS_END

/***************************************************************************
                                Bonk's Adventure
***************************************************************************/

static INPUT_PORTS_START( bonkadv )
	PORT_START("P1")        /* b0000.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START("P2")        /* b0002.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")    /* b0004.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_SERVICE_NO_TOGGLE( 0x1000, IP_ACTIVE_LOW )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE2 )

	PORT_START("UNK")       /* ? - b0006.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // tested
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // tested
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")      /* from the MCU - $10019e.b <- $200200.b */
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Ticket dispenser" ) /* "Reserved" for Japan/Europe */
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, "Title Level Display" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Reserved" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Reserved" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )
INPUT_PORTS_END


/***************************************************************************
                            Great 1000 Miles Rally
***************************************************************************/

static INPUT_PORTS_START( gtmr )
	PORT_START("P1")        /* b0000.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("DSW1",0x0800,NOTEQUALS,0x00)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("DSW1",0x0800,NOTEQUALS,0x00)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("DSW1",0x0800,NOTEQUALS,0x00)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("DSW1",0x0800,NOTEQUALS,0x00)
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("DSW1",0x0800,EQUALS,0x00)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")        /* b0002.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("DSW1",0x0800,NOTEQUALS,0x00)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("DSW1",0x0800,NOTEQUALS,0x00)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("DSW1",0x0800,NOTEQUALS,0x00)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2) PORT_CONDITION("DSW1",0x0800,NOTEQUALS,0x00)
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("DSW1",0x0800,EQUALS,0x00)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* b0004.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_SERVICE_NO_TOGGLE( 0x1000, IP_ACTIVE_LOW )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK")       /* Seems unused ! - b0006.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")      /*  from the MCU - 101265.b <- 206000.b */
	PORT_SERVICE_DIPLOC(  0x0100, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Controller ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Joystick ) )
	PORT_DIPSETTING(      0x0000, "Wheel" )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Controls ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, "1P Side" )           PORT_CONDITION("DSW1",0x0800,NOTEQUALS,0x00)
	PORT_DIPSETTING(      0x0000, "Both Sides" )        PORT_CONDITION("DSW1",0x0800,NOTEQUALS,0x00)
	PORT_DIPSETTING(      0x1000, "360 degree wheel" )  PORT_CONDITION("DSW1",0x0800,EQUALS,0x00)
	PORT_DIPSETTING(      0x0000, "270 degree wheel" )  PORT_CONDITION("DSW1",0x0800,EQUALS,0x00)
	PORT_DIPNAME( 0x2000, 0x2000, "Use Brake" ) PORT_DIPLOCATION("SW1:6") /* Valid only when joystick is used */
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0xc000, 0xc000, "National Anthem & Flag" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0xc000, "Use Memory" )
	PORT_DIPSETTING(      0x8000, "Anthem Only" )
	PORT_DIPSETTING(      0x4000, "Flag Only" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )

	PORT_START("WHEEL0")    /* Wheel (270deg) - 100015.b <- ffffe.b */
	PORT_BIT ( 0x00ff, 0x0080, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(25) PORT_PLAYER(1) PORT_CONDITION("DSW1",0x1800,EQUALS,0x0000)
	PORT_BIT ( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("DSW1",0x1800,NOTEQUALS,0x0000)

	PORT_START("WHEEL1")    /* Wheel (360deg) */
	PORT_BIT ( 0x00ff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(70) PORT_KEYDELTA(25) PORT_PLAYER(1) PORT_CONDITION("DSW1",0x1800,EQUALS,0x1000)
	PORT_BIT ( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("DSW1",0x1800,NOTEQUALS,0x1000)
INPUT_PORTS_END


/***************************************************************************
                            Great 1000 Miles Rally 2
***************************************************************************/

static INPUT_PORTS_START( gtmr2 )
	PORT_START("P1")        /* 100004.w <- b00000.w (cpl) */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")        /* 10000c.w <- b00002.w (cpl) - for "test mode" only */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* 100014.w <- b00004.w (cpl) */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )   // only in "test mode"
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_SERVICE_NO_TOGGLE( 0x1000, IP_ACTIVE_LOW )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT   )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1   )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN    )

	PORT_START("EXTRA")     /* 100017.w <- b00006.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("IN 3-6")  // Code at 0x002236 - Centers 270D wheel ?
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")      /* from the MCU - 1016f7.b <- 206000.b */
	PORT_DIPNAME( 0x0700, 0x0700, "Linked Operation Board Number" ) PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(      0x0700, "No Communication" )
	PORT_DIPSETTING(      0x0600, "Board #1" )
	PORT_DIPSETTING(      0x0500, "Board #2" )
	PORT_DIPSETTING(      0x0400, "Board #3" )
	PORT_DIPSETTING(      0x0300, "Board #4" )
	/* 0x0000 to 0x0200 : "Machine 4"
	PORT_DIPSETTING(      0x0200, "Machine 4 (0x0200)" )
	PORT_DIPSETTING(      0x0100, "Machine 4 (0x0100)" )
	PORT_DIPSETTING(      0x0000, "Machine 4 (0x0000)" )
	*/
	PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Controls ) ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(      0x1800, DEF_STR( Joystick ) )
	PORT_DIPSETTING(      0x0800, "Wheel (360)" )           // Not working correctly in race }
	PORT_DIPSETTING(      0x1000, "Wheel (270D)" )          // Not working correctly !   } seems to work ok to me! (minwah)
	PORT_DIPSETTING(      0x0000, "Wheel (270A)" )          // Not working correctly in race }
	PORT_DIPNAME( 0x2000, 0x2000, "Optional Mode Of Pedal Function" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x2000, "Microswitch" )           // "This mode also corresponds to the two buttons used with joystick."
	PORT_DIPSETTING(      0x0000, "Potentiometer" )             // Not implemented yet
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW1:1" )

	PORT_START("WHEEL0")    /* Wheel (270A) - 100019.b <- fffff.b */
	PORT_BIT ( 0x00ff, 0x0080, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(1)

	PORT_START("WHEEL1")    /* Wheel (270D) - 100019.b <- ffffe.b */
	PORT_BIT ( 0x00ff, 0x0080, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(1)

	PORT_START("WHEEL2")    /* Wheel (360) - 100019.b <- ffffe.b */
	PORT_BIT( 0x00ff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT)

	PORT_START("FAKE")      /* Fake P2 - To be pressed during boot sequence - Code at 0x000c9e */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("IN 1-0") PORT_CODE(KEYCODE_H) // "sound test"
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("IN 1-4") PORT_CODE(KEYCODE_J) // "view tiles"
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("IN 1-5") PORT_CODE(KEYCODE_K) // "view memory"
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("IN 1-6") PORT_CODE(KEYCODE_L) // "view sprites ?"
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Magical Crystal
***************************************************************************/

static INPUT_PORTS_START( mgcrystl )
	PORT_START("DSW_P1")    /* c00000.w */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0002, IP_ACTIVE_LOW, "SW1:2" )
	/* All other game settings done through the test mode */
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW1:3" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW1:4" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW1:5" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW1:6" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW1:7" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW1:8" ) /* Listed as "Unused" */

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")        /* c00002.b */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* c00004.b */
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_SERVICE_NO_TOGGLE( 0x1000, IP_ACTIVE_LOW )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
INPUT_PORTS_END



/***************************************************************************
                                Pack'n Bang Bang
***************************************************************************/

static INPUT_PORTS_START( packbang )
	PORT_START("P1")        /* 680000.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")        /* 680002.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* 680000.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_SERVICE_NO_TOGGLE( 0x1000, IP_ACTIVE_LOW )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK")       /* ? - 680006.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW1:2" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME(       0x04, 0x04, "Timer Speed" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x04, "Standard" )
	PORT_DIPNAME(       0x18, 0x18, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x00, "Invalid" ) // Japanese text, Korean Kaneko logo 'Unusued' according to test mode
	PORT_DIPSETTING(    0x08, DEF_STR( Korea ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Japan ) )
	PORT_DIPSETTING(    0x18, DEF_STR( World ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

/***************************************************************************
                                Shogun Warriors
***************************************************************************/

static INPUT_PORTS_START( shogwarr )
	PORT_START("P1")        /* b80000.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P2")        /* b80002.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("SYSTEM")    /* b80004.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1   )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN    )

	PORT_START("UNK")       /* ? - b80006.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// dip defaults confirmed by manual ("Bolr (sic) face indicate standard setting")
	// including Demo Sounds
	PORT_START("DSW1")      /* from the MCU - 102e15.b <- 200059.b */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x38, 0x20, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x38, "1 Easy" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x28, "3" )
	PORT_DIPSETTING(    0x20, "4 Normal" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPSETTING(    0x00, "8 Hard" )
	PORT_DIPNAME( 0x40, 0x40, "Can Join During Game" ) PORT_DIPLOCATION("SW1:7")
	// 1p vs 2p is allowed only at title screen with a single credit by pressing 2p start. akin to an "event mode"
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )  //  "impossible", vs.play 1 credit, game over aftewards
	// "normal mode"
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )   //  "possible", vs.play 2 credits, winner plays against CPU afterwards
	PORT_DIPNAME( 0x80, 0x80, "Continue Coin" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
INPUT_PORTS_END


static INPUT_PORTS_START( brapboys )
	PORT_START("P1")        /* b80000.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P2")        /* b80002.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("SYSTEM")    /* b80004.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("UNK")       /* ? - b80006.w */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPNAME( 0x04, 0x04, "Switch Test" )       PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC(0x08, 0x08, "SW1:4" )
	PORT_DIPNAME( 0x10, 0x10, "Coin Slots" )        PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "Separate Coins" )
	PORT_DIPSETTING(    0x00, "Shared Coins" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Players ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard) )

/*****************************************************
Difficulty    Lives      Bonus Players    Play Level
  Easy          3      every 1000 / 2000   Level 2
  Normal        2      every 2000 / 3000   Level 3
  Hard          2      every 3000 / 4000   Level 4
  Very Hard     1       --- NONE ---       Level 6
******************************************************/

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
INPUT_PORTS_END


/***************************************************************************


                                Machine Drivers


***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(kaneko16_state::interrupt)
{
	int scanline = param;

	// main vblank interrupt
	if (scanline == 224)
	{
		// 2 frame delayed normaly; differs per PCB?
		m_kaneko_spr->render_sprites(m_screen->visible_area(), m_spriteram->buffer(), m_spriteram->bytes());
		m_spriteram->copy();
		m_maincpu->set_input_line(5, HOLD_LINE);
	}

	// each of these 2 int are responsible of translating a part of sprite buffer
	// from work ram to sprite ram. How these are scheduled is unknown.
	if (scanline == 64)
		m_maincpu->set_input_line(4, HOLD_LINE);

	if (scanline == 144)
		m_maincpu->set_input_line(3, HOLD_LINE);
}

/***************************************************************************
                                The Berlin Wall
***************************************************************************/

/*
    Berlwall interrupts:

    1-3]    e8c:
    4]      e54:
    5]      de4:
    6-7]    rte
*/

void kaneko16_berlwall_state::berlwall(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12000000); /* MC68000P12 */
	m_maincpu->set_addrmap(AS_PROGRAM, &kaneko16_berlwall_state::berlwall_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(kaneko16_berlwall_state::interrupt), "screen", 0, 1);

	WATCHDOG_TIMER(config, m_watchdog);

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 16, 240-1);
	m_screen->set_screen_update(FUNC(kaneko16_berlwall_state::screen_update));

	PALETTE(config, m_palette).set_format(palette_device::xGRB_555, 2048);

	PALETTE(config, m_bgpalette, palette_device::GRB_555); // 32768 static colors for the bg

	KANEKO_TMAP(config, m_view2[0]);
	m_view2[0]->set_colbase(0x40 * 16);
	m_view2[0]->set_offset(0x5b, -0x8, 256, 240+16);
	m_view2[0]->set_palette(m_palette);

	KANEKO_VU002_SPRITE(config, m_kaneko_spr);
	m_kaneko_spr->set_offsets(0, -1*64);
	m_kaneko_spr->set_palette(m_palette);
	m_kaneko_spr->set_color_base(0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2149(config, m_ym2149[0], 1000000);
	m_ym2149[0]->add_route(ALL_OUTPUTS, "mono", 0.5);
	m_ym2149[0]->port_a_read_callback().set_ioport("DSW1");
	m_ym2149[0]->port_b_read_callback().set_ioport("DSW2");

	YM2149(config, m_ym2149[1], 1000000);
	m_ym2149[1]->add_route(ALL_OUTPUTS, "mono", 0.5);

	OKIM6295(config, m_oki[0], 12000000/6, okim6295_device::PIN7_LOW);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************
                            Bakuretsu Breaker
***************************************************************************/

void kaneko16_state::bakubrkr_oki1_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr("okibank1");
}

void kaneko16_state::bakubrkr(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(12'000'000)); /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &kaneko16_state::bakubrkr_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(kaneko16_state::interrupt), "screen", 0, 1);

	MCFG_MACHINE_RESET_OVERRIDE(kaneko16_state,gtmr)
	EEPROM_93C46_16BIT(config, m_eeprom);

	WATCHDOG_TIMER(config, m_watchdog);

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(59);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 16, 240-1);
	m_screen->set_screen_update(FUNC(kaneko16_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xGRB_555, 2048);

	KANEKO_TMAP(config, m_view2[0]);
	m_view2[0]->set_colbase(0x40 * 16);
	m_view2[0]->set_offset(0x5b, -0x8, 256, 240);
	m_view2[0]->set_palette(m_palette);

	KANEKO_TMAP(config, m_view2[1]);
	m_view2[1]->set_colbase(0x40 * 16);
	m_view2[1]->set_offset(0x5b, -0x8, 256, 240);
	m_view2[1]->set_palette(m_palette);

	KANEKO_VU002_SPRITE(config, m_kaneko_spr);
	m_kaneko_spr->set_priorities(8,8,8,8); // above all
	m_kaneko_spr->set_palette(m_palette);
	m_kaneko_spr->set_color_base(0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2149(config, m_ym2149[0], XTAL(12'000'000)/6); /* verified on pcb */
	m_ym2149[0]->add_route(ALL_OUTPUTS, "mono", 0.5);
	m_ym2149[0]->port_b_write_callback().set(FUNC(kaneko16_state::oki_bank0_w<7>)); /* outputs B:  OKI bank Switch */

	YM2149(config, m_ym2149[1], XTAL(12'000'000)/6); /* verified on pcb */
	m_ym2149[1]->add_route(ALL_OUTPUTS, "mono", 0.5);
	m_ym2149[1]->port_a_read_callback().set(FUNC(kaneko16_state::eeprom_r));    /* inputs  A:  0,EEPROM bit read */
	m_ym2149[1]->port_b_write_callback().set(FUNC(kaneko16_state::eeprom_cs_w)); /* outputs B:  0,EEPROM reset */

	OKIM6295(config, m_oki[0], XTAL(12'000'000)/6, okim6295_device::PIN7_HIGH); /* verified on pcb */
	m_oki[0]->set_addrmap(0, &kaneko16_state::bakubrkr_oki1_map);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************
                                    Blaze On
***************************************************************************/

/*
    Blaze On:
        1]      busy loop
        2]      does nothing
        3]      rte
        4]      drives the game
        5]      == 2
        6-7]    busy loop
*/

void kaneko16_state::blazeon(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12000000);    /* TMP68HC000-12 */
	m_maincpu->set_addrmap(AS_PROGRAM, &kaneko16_state::blazeon_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(kaneko16_state::interrupt), "screen", 0, 1);

	Z80(config, m_audiocpu, 4000000);   /* D780C-2 (6 MHz) */
	m_audiocpu->set_addrmap(AS_PROGRAM, &kaneko16_state::blazeon_soundmem);
	m_audiocpu->set_addrmap(AS_IO, &kaneko16_state::blazeon_soundport);

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(320, 240);
	m_screen->set_visarea(0, 320-1, 0, 240-1 -8);
	m_screen->set_screen_update(FUNC(kaneko16_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xGRB_555, 2048);

	KANEKO_TMAP(config, m_view2[0], 0);
	m_view2[0]->set_colbase(0x40 * 16);
	m_view2[0]->set_offset(0x33, 0x8, 320, 240);
	m_view2[0]->set_palette(m_palette);

	KANEKO_VU002_SPRITE(config, m_kaneko_spr);
	m_kaneko_spr->set_priorities(1 /* "above tile[0], below the others" */ ,2 /* "above tile[0-1], below the others" */ ,8 /* above all */,8 /* above all */);
	m_kaneko_spr->set_offsets(0x10000 - 0x680, 0x000);
	m_kaneko_spr->set_palette(m_palette);
	m_kaneko_spr->set_color_base(0);

	// there is actually a 2nd sprite chip! looks like our device emulation handles both at once

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	YM2151(config, m_ymsnd, 4000000);
	m_ymsnd->add_route(0, "lspeaker", 1.0);
	m_ymsnd->add_route(1, "rspeaker", 1.0);
}


/***************************************************************************
                                 Wing Force
***************************************************************************/

void kaneko16_state::wingforc(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(16'000'000));    /* TMP68HC000N-16 */
	m_maincpu->set_addrmap(AS_PROGRAM, &kaneko16_state::blazeon_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(kaneko16_state::interrupt), "screen", 0, 1);

	Z80(config, m_audiocpu, XTAL(16'000'000)/4);   /* D780C-2 (6 MHz) */
	m_audiocpu->set_addrmap(AS_PROGRAM, &kaneko16_state::blazeon_soundmem);
	m_audiocpu->set_addrmap(AS_IO, &kaneko16_state::wingforc_soundport);

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(59.1854);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(320, 240);
	m_screen->set_visarea(0, 320-1, 0, 240-1 -16);
	m_screen->set_screen_update(FUNC(kaneko16_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xGRB_555, 2048);

	KANEKO_TMAP(config, m_view2[0]);
	m_view2[0]->set_colbase(0x40 * 16);
	m_view2[0]->set_offset(0x33, 0x8+1, 320, 240);
	m_view2[0]->set_palette(m_palette);

	KANEKO_VU002_SPRITE(config, m_kaneko_spr);
	m_kaneko_spr->set_priorities(1 /* "above tile[0], below the others" */ ,2 /* "above tile[0-1], below the others" */ ,8 /* above all */,8 /* above all */);
	m_kaneko_spr->set_offsets(0x10000 - 0x680, 0x000);
	m_kaneko_spr->set_palette(m_palette);
	m_kaneko_spr->set_color_base(0);

	// there is actually a 2nd sprite chip! looks like our device emulation handles both at once

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	YM2151(config, m_ymsnd, XTAL(16'000'000)/4);
	m_ymsnd->add_route(ALL_OUTPUTS, "mono", 0.2);

	OKIM6295(config, m_oki[0], XTAL(16'000'000)/16, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.5);
	m_oki[0]->set_addrmap(0, &kaneko16_state::bakubrkr_oki1_map);
}


/***************************************************************************
                            Great 1000 Miles Rally
***************************************************************************/

/*
    gtmr interrupts:

    3] 476:         time, input ports, scroll registers
    4] 466->258e:   set sprite ram
    5] 438:         set sprite colors

    VIDEO_UPDATE_AFTER_VBLANK fixes the mangled/wrong colored sprites
*/

void kaneko16_state::gtmr_oki1_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr("okibank1");
}

void kaneko16_state::gtmr_oki2_map(address_map &map)
{
	map(0x00000, 0x3ffff).bankr("okibank2");
}

void kaneko16_gtmr_state::gtmr(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(16'000'000)); /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &kaneko16_gtmr_state::gtmr_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(kaneko16_gtmr_state::interrupt), "screen", 0, 1);

	MCFG_MACHINE_RESET_OVERRIDE(kaneko16_gtmr_state,gtmr)

	EEPROM_93C46_16BIT(config, m_eeprom);

	WATCHDOG_TIMER(config, m_watchdog);

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(320, 240);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(kaneko16_gtmr_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xGRB_555, 32768);

	KANEKO_TMAP(config, m_view2[0]);
	m_view2[0]->set_colbase(0);
	m_view2[0]->set_offset(0x33, 0x00, 320, 240);
	m_view2[0]->set_palette(m_palette);

	KANEKO_TMAP(config, m_view2[1]);
	m_view2[1]->set_colbase(0);
	m_view2[1]->set_offset(0x33, 0x00, 320, 240);
	m_view2[1]->set_palette(m_palette);

	KANEKO_KC002_SPRITE(config, m_kaneko_spr);
	m_kaneko_spr->set_palette(m_palette);
	m_kaneko_spr->set_color_base(0x40 * 256);

	KANEKO_TOYBOX(config, m_toybox, m_eeprom, m_dsw_port, m_mcuram, "mcudata");
	/* part of the toybox? */
	KANEKO_HIT(config, m_kaneko_hit);
	m_kaneko_hit->set_type(1);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki[0], XTAL(16'000'000)/8, okim6295_device::PIN7_LOW);  /* verified on pcb */
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.5);
	m_oki[0]->set_addrmap(0, &kaneko16_gtmr_state::gtmr_oki1_map);

	OKIM6295(config, m_oki[1], XTAL(16'000'000)/8, okim6295_device::PIN7_LOW);  /* verified on pcb */
	m_oki[1]->add_route(ALL_OUTPUTS, "mono", 0.5);
	m_oki[1]->set_addrmap(0, &kaneko16_gtmr_state::gtmr_oki2_map);
}

void kaneko16_gtmr_state::gtmre(machine_config &config)
{
	gtmr(config);
	m_toybox->set_table(kaneko_toybox_device::TABLE_ALT);
}

/***************************************************************************
                            Great 1000 Miles Rally 2
***************************************************************************/

void kaneko16_gtmr_state::gtmr2(machine_config &config)
{
	gtmre(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &kaneko16_gtmr_state::gtmr2_map);
}

/***************************************************************************
                                Blood Warrior
***************************************************************************/

void kaneko16_gtmr_state::bloodwar(machine_config &config)
{
	gtmr(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &kaneko16_gtmr_state::bloodwar_map);
	MCFG_MACHINE_RESET_OVERRIDE(kaneko16_gtmr_state, gtmr)

	m_kaneko_spr->set_priorities(2 /* never used? */ ,3 /* character selection / vs. portraits */ ,5 /* winning portrait*/ ,7 /* ? */);
}



/***************************************************************************
                            Bonk's Adventure
***************************************************************************/

void kaneko16_gtmr_state::bonkadv(machine_config &config)
{
	gtmr(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &kaneko16_gtmr_state::bonkadv_map);
	MCFG_MACHINE_RESET_OVERRIDE(kaneko16_gtmr_state, gtmr)

	m_kaneko_spr->set_priorities(2 /* never used? */ ,3 /* volcano lava on level 2 */ ,5 /* in-game player */ ,7 /* demostration text */);

	m_toybox->set_game_type(kaneko_toybox_device::GAME_BONK);
	m_kaneko_hit->set_type(0);
}

/***************************************************************************
                                Magical Crystal
***************************************************************************/

void kaneko16_state::mgcrystl(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(12'000'000)); /* verified on pcb, TMP68HC000N-12 @U31 and X2 is 12MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &kaneko16_state::mgcrystl_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(kaneko16_state::interrupt), "screen", 0, 1);

	MCFG_MACHINE_RESET_OVERRIDE(kaneko16_state,mgcrystl)

	EEPROM_93C46_16BIT(config, m_eeprom);

	WATCHDOG_TIMER(config, m_watchdog);

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0+16, 256-16-1);
	m_screen->set_screen_update(FUNC(kaneko16_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xGRB_555, 2048);

	KANEKO_TMAP(config, m_view2[0]);
	m_view2[0]->set_colbase(0x40 * 16);
	m_view2[0]->set_offset(0x5b, -0x8, 256, 240);
	m_view2[0]->set_palette(m_palette);

	KANEKO_TMAP(config, m_view2[1]);
	m_view2[1]->set_colbase(0x40 * 16);
	m_view2[1]->set_offset(0x5b, -0x8, 256, 240);
	m_view2[1]->set_palette(m_palette);

	KANEKO_VU002_SPRITE(config, m_kaneko_spr);
	m_kaneko_spr->set_priorities(2 /* below all */ ,3 /* above tile[0], below the other */ ,5 /* above all */ ,7 /* above all */);
	m_kaneko_spr->set_palette(m_palette);
	m_kaneko_spr->set_color_base(0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2149(config, m_ym2149[0], XTAL(12'000'000)/6); /* verified on pcb */
	m_ym2149[0]->add_route(ALL_OUTPUTS, "mono", 0.5);

	YM2149(config, m_ym2149[1], XTAL(12'000'000)/6); /* verified on pcb */
	m_ym2149[1]->add_route(ALL_OUTPUTS, "mono", 0.5);
	m_ym2149[1]->port_a_read_callback().set(FUNC(kaneko16_state::eeprom_r));    /* inputs  A:  0,EEPROM bit read */
	m_ym2149[1]->port_b_write_callback().set(FUNC(kaneko16_state::eeprom_cs_w)); /* outputs B:  0,EEPROM reset */

	OKIM6295(config, m_oki[0], XTAL(12'000'000)/6, okim6295_device::PIN7_HIGH); /* verified on pcb */
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.5);
}



/***************************************************************************
                                Shogun Warriors
***************************************************************************/

/*
    shogwarr interrupts:

    2] 100: rte
    3] 102:
    4] 136:
        movem.l D0-D7/A0-A6, -(A7)
        movea.l $207808.l, A0   ; from mcu?
        jmp     ($4,A0)

    other: busy loop
*/

TIMER_DEVICE_CALLBACK_MEMBER(kaneko16_shogwarr_state::shogwarr_interrupt)
{
	int scanline = param;

	if (scanline == 224)
	{
		m_kaneko_spr->render_sprites(m_screen->visible_area(), m_spriteram->buffer(), m_spriteram->bytes());
		m_spriteram->copy();
		m_maincpu->set_input_line(4, HOLD_LINE);
	}

	if (scanline == 64)
		m_maincpu->set_input_line(3, HOLD_LINE);

	if (scanline == 144)
		m_maincpu->set_input_line(2, HOLD_LINE);
}

/*
static const u16 shogwarr_default_eeprom[64] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x4B41, 0x4E45, 0x4B4F, 0x2F41, 0x544F, 0x5020, 0x3139, 0x3932,
    0x4655, 0x4A49, 0xFFFF, 0x4D41, 0x2042, 0x5553, 0x5445, 0x5220,
    0x2053, 0x484F, 0xFFFF, 0x4E20, 0x5741, 0x5252, 0x494F, 0x5253,
    0xFFFF, 0x7079, 0x7269, 0xFFFF, 0x7420, 0x4B41, 0x4E45, 0x4B4F,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0010, 0x0000, 0x0000, 0xFFFF
};
*/
// the above eeprom looks corrupt, some of the text is wrong, the game never writes this text tho.. maybe it should be as below
// leaving both here incase they relate to which tables get 'locked out' by the MCU somehow
static const u16 shogwarr_default_eeprom[64] = {
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x4B41, 0x4E45, 0x4B4F, 0x2F41, 0x544F, 0x5020, 0x3139, 0x3932,
	0x4655, 0x4A49, 0x5941, 0x4D41, 0x2042, 0x5553, 0x5445, 0x5220,
	0x2053, 0x484F, 0x4755, 0x4E20, 0x5741, 0x5252, 0x494F, 0x5253,
	0x636F, 0x7079, 0x7269, 0x6768, 0x7420, 0x4B41, 0x4E45, 0x4B4F,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0010, 0x0000, 0x0000, 0xFFFF
};

void kaneko16_shogwarr_state::shogwarr(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &kaneko16_shogwarr_state::shogwarr_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(kaneko16_shogwarr_state::shogwarr_interrupt), "screen", 0, 1);

	MCFG_MACHINE_RESET_OVERRIDE(kaneko16_shogwarr_state,mgcrystl)

	EEPROM_93C46_16BIT(config, m_eeprom).default_data(shogwarr_default_eeprom, 128);

	WATCHDOG_TIMER(config, m_watchdog);

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(59.1854);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(320, 240);
	m_screen->set_visarea(40, 296-1, 16, 240-1);
	m_screen->set_screen_update(FUNC(kaneko16_shogwarr_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xGRB_555, 2048);

	KANEKO_TMAP(config, m_view2[0]);
	m_view2[0]->set_colbase(0x40 * 16);
	m_view2[0]->set_offset(0x33, -0x8, 320, 240);
	m_view2[0]->set_palette(m_palette);

	KANEKO_VU002_SPRITE(config, m_kaneko_spr);
	m_kaneko_spr->set_priorities(1 /* below all */ ,3 /* above tile[0], below the others */ ,5 /* above all */ ,7 /* above all */);
	m_kaneko_spr->set_offsets(0xa00, -0x40);
	m_kaneko_spr->set_fliptype(1);
	m_kaneko_spr->set_palette(m_palette);
	m_kaneko_spr->set_color_base(0);

	KANEKO_HIT(config, m_kaneko_hit);
	m_kaneko_hit->set_type(1);

	KANEKO_CALC3(config, m_calc3_prot, m_maincpu, m_eeprom, "calc3_rom");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki[0], XTAL(16'000'000)/8, okim6295_device::PIN7_LOW);
	m_oki[0]->set_addrmap(0, &kaneko16_shogwarr_state::gtmr_oki1_map);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.5);

	OKIM6295(config, m_oki[1], XTAL(16'000'000)/8, okim6295_device::PIN7_LOW);
	m_oki[1]->set_addrmap(0, &kaneko16_shogwarr_state::gtmr_oki2_map);
	m_oki[1]->add_route(ALL_OUTPUTS, "mono", 0.5);
}


static const u16 brapboys_default_eeprom[64] = {
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0005, 0x0006, 0x2030, 0x0003, 0x6818, 0x0101, 0x0101,
	0x0101, 0x0001, 0x0004, 0x0008, 0x4B41, 0x4E45, 0x4B4F, 0x2020,
	0x4265, 0x2052, 0x6170, 0x2042, 0x6F79, 0x7300, 0x3030, 0x302E,
	0x3038, 0x10FF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0035, 0xFFFF, 0xFFFF, 0xFFFF
};

// sample banking is different on brap boys for the music, why? GALs / PALs ?
void kaneko16_shogwarr_state::brapboys_oki2_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr("okibank2");
}

void kaneko16_shogwarr_state::brapboys(machine_config &config)
{
	shogwarr(config);
	m_oki[1]->set_addrmap(0, &kaneko16_shogwarr_state::brapboys_oki2_map);
	m_kaneko_hit->set_type(2);
	m_eeprom->default_data(brapboys_default_eeprom, 128);
}

/***************************************************************************


                                ROMs Loading


***************************************************************************/

void kaneko16_state::common_oki_bank_install(int bankno, size_t fixedsize, size_t bankedsize)
{
	assert((m_okibank[bankno].found()) && (m_okiregion[bankno].found()));
	u8 *sample  = m_okiregion[bankno]->base();
	size_t length = m_okiregion[bankno]->bytes();

	if (fixedsize > 0)
	{
		int max_bank = (length - fixedsize) / bankedsize;
		m_okibank[bankno]->configure_entries(0, max_bank, &sample[fixedsize], bankedsize);
		int i = max_bank;
		while (i < length / bankedsize)
		{
			m_okibank[bankno]->configure_entry(i++, &sample[length - bankedsize]);
		}
	}
	else
	{
		m_okibank[bankno]->configure_entries(0, length / bankedsize, &sample[0], bankedsize);
	}
}

void kaneko16_gtmr_state::common_oki_bank_install(int bankno, size_t fixedsize, size_t bankedsize)
{
	assert((m_okibank[bankno].found()) && (m_okiregion[bankno].found()));
	u8 *sample  = m_okiregion[bankno]->base();
	size_t length = m_okiregion[bankno]->bytes();

	/* bank 0 maps to itself, so we just leave it alone */
	if (fixedsize > 0)
	{
		int bank_offs = fixedsize / bankedsize;
		int i = 0;
		while (i < bank_offs)
		{
			m_okibank[bankno]->configure_entry(i++, &sample[fixedsize]);
		}
		m_okibank[bankno]->configure_entries(bank_offs, (length - fixedsize) / bankedsize, &sample[fixedsize], bankedsize);
	}
	else
	{
		m_okibank[bankno]->configure_entries(0, length / bankedsize, &sample[0], bankedsize);
	}
}

void kaneko16_state::init_bakubrkr()
{
	common_oki_bank_install(0, 0x20000, 0x20000);
}

/*

berlwallt original bug ?

info from SebV:

After about level 5 or so, if you insert a coin when the continue screen
pops up, an error message "Copy Board" pops up.
----------------------------------------
Happened to me when player dies, at level 4.
The message is not written in ROM, its sprite ram address is (always?)
$30fd40
Routine $337d2 writes it (and routine $5c8c erases it)
The 'COPY BOARD!' message in stored in ROM directly as sprite number
($1cfa8)

$20288a : seems to contain the level number (initialized to 2 (?) when a
game is started, and is incremented by 1 once a level is finished)

01CF3E: move.b  $20288a.l, D0
01CF44: cmpi.b  #$d, D0
01CF48: bcs     1cf76                   ; branch not taken -=> 'COPY BOARD!'
01CF4A: movem.l D0/A0-A2, -(A7)
01CF4E: movea.l A0, A1
01CF50: lea     ($4c,PC), A0; ($1cf9e)
01CF54: nop
01CF56: lea     ($a,A0), A0             ; A0 = $1cfa8 = 'COPY BOARD!'
01CF5A: lea     $30e064.l, A1
01CF60: lea     (-$64,A1), A1
01CF64: lea     ($1d40,A1), A1
01CF68: move.b  #$80, D1
01CF6C: jsr     $33776.l                ; display routine
01CF72: movem.l (A7)+, D0/A0-A2
01CF76:

berlwall  levels: 1-1,2,3(anim),...
berlwallt levels: 1-1(anim)2-1/2/3/4/5(anim)3-1/2/3/4/5(anim)4-1(*)

note: berlwall may be genuine while berlwallt may be bootleg! because
stage 1-1 of berlwallt is stage 1-3 of berlwall, and berlwall has
explanation ingame.
(TAFA flyers exist for both berlwall and berlwallt player graphics)
--------------------------------------------------------------------------------
*/
// TODO: this is certainly a protection issue, the level number variable can go up to 0x2f, which is way bigger than 0xd.
// Sprite DMA decryption? @see berlwall_spriteram_r
void kaneko16_berlwall_state::patch_protection(u32 bra_offset,u16 bra_value,u16 checksum)
{
	m_mainregion[bra_offset/2] = bra_value;
	m_mainregion[0x3fffe/2] = checksum;
}

void kaneko16_berlwall_state::init_berlwall_common()
{
}

void kaneko16_berlwall_state::init_berlwall()
{
	init_berlwall_common();
	patch_protection(0x1a3ea,0x602c,0xc40d);
}

void kaneko16_berlwall_state::init_berlwallt()
{
	init_berlwall_common();
	patch_protection(0x1cf48,0x602c,0xaed4);
}

void kaneko16_berlwall_state::init_berlwallk()
{
	init_berlwall_common();
	patch_protection(0x1ceb0,0x602c,0x8364);
}

void kaneko16_gtmr_state::init_gtmr()
{
	common_oki_bank_install(0, 0x30000, 0x10000);
	common_oki_bank_install(1, 0x00000, 0x40000);
}




/***************************************************************************

                                Bakuretsu Breaker

    USES TOSHIBA 68000 CPU W/TWO YM2149 & OKI M6295 FOR SOUND

    LOCATION    TYPE
    ------------------
    U38         27C040
    U37         "
    U36         27C020
    U19         "
    U18         "

Bakuretsu Breaker / Explosive Breaker
Kaneko, 1992

PCB Layout
----------

ZOOFC-02
|------------------------------------------|
| PAL    TS020.U33                PAL      |
| 6264                            6264     |
| 6264    VIEW2-CHIP              6264     |
| 4464 4464           VIEW2-CHIP  TS010.U4 |
| 4464 4464                                |
| 4464 4464                       699206P  |
|                         5116    (QFP44)  |
|                                          |
|         TS002J.U36 MUX2-CHIP    699205P  |
|VU-002   TS001J.U37      5116    (QFP44) J|
|(QFP160) TS000J.U38                      A|
|                   62256 TS100J.U18      M|
|        6116 6116  62256 TS101J.U19      M|
|        PAL                     PAL      A|
|16MHz   PAL  PAL  TMP68HC000N-12          |
|12MHz        PAL                IU-001    |
|   VU-001                      (QFP44)    |
|   (QFP48)  YM2149      PAL               |
|93C46       YM2149  TS030.U5  M6295       |
|    DSW1(4)                               |
|------------------------------------------|

Notes:
       68000 clock: 12.000MHz
      YM2149 clock: 2.000MHz
       M6295 clock: 2.000MHz, sample rate = clock /132
             VSync: 59Hz
             HSync: 15.68kHz


***************************************************************************/

ROM_START( explbrkr ) /* MASTER UP= 92/10/08 20:10:16 */
	ROM_REGION( 0x080000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "ts100e.u18", 0x000000, 0x040000, CRC(cc84a985) SHA1(1732a607cc1f894dd45cfc915dfe0407335f0073) )
	ROM_LOAD16_BYTE( "ts101e.u19", 0x000001, 0x040000, CRC(88f4afb7) SHA1(08b8efd6bd935bc1b8cf9753d58b38ccf9a70b4d) )

	/* these actually match the other set but have different names on the board..*/
	ROM_REGION( 0x240000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "ts001e.u37",  0x000000, 0x080000, CRC(70b66e7e) SHA1(307ba27b623f67ee4b4023179870c270bac8ea22) )
	ROM_RELOAD(              0x100000, 0x080000  )
	ROM_LOAD( "ts000e.u38",  0x080000, 0x080000, CRC(a7a94143) SHA1(d811a7597402c161850ddf98cdb00661ea506c7d) )
	ROM_RELOAD(              0x180000, 0x080000  )
	ROM_LOAD( "ts002e.u36",  0x200000, 0x040000, CRC(611271e6) SHA1(811c21822b074fbb4bb809fed29d48bbd51d57a0) )

	ROM_REGION( 0x100000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "ts010.u4",  0x000000, 0x100000, CRC(df935324) SHA1(73b7aff8800a4e88a47ad426190b73dabdfbf142) )

	ROM_REGION( 0x100000, "view2_1", 0 )   /* Tiles */
	ROM_LOAD( "ts020.u33",  0x000000, 0x100000, CRC(eb58c35d) SHA1(762c5219de6f729a0fc1df90fce09cdf711c2a1e) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x100000, "oki1", 0 )  /* OKI Sample ROM */
	ROM_LOAD( "ts030.u5",    0x000000, 0x100000, CRC(1d68e9d1) SHA1(aaa64a8e8d7cd7f91d2be346fafb9d1f29b40eda) )
ROM_END

ROM_START( explbrkrk ) /* MASTER UP= 92/10/08 21:35:56 - No "For use in Korea.." but has the KA+<cat image> logo which was only used in Korea; the Kaneko logo is scratched out on all custom ICs */
	ROM_REGION( 0x080000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "u18", 0x000000, 0x040000, CRC(08267644) SHA1(8a5c638b414d2b1a8bb68e0d8ba6747253734e82) ) /* TMS 27C020 with non descript label "MF" */
	ROM_LOAD16_BYTE( "u19", 0x000001, 0x040000, CRC(f0a243b1) SHA1(3448c85c9388b590ea28b9adcaf81b68c14c678c) ) /* TMS 27C020 with non descript label "MF" */

	/* these actually match the other set but have different names on the board..*/
	ROM_REGION( 0x240000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "ts001e.u37",  0x000000, 0x080000, CRC(70b66e7e) SHA1(307ba27b623f67ee4b4023179870c270bac8ea22) )
	ROM_RELOAD(              0x100000, 0x080000  )
	ROM_LOAD( "ts000e.u38",  0x080000, 0x080000, CRC(a7a94143) SHA1(d811a7597402c161850ddf98cdb00661ea506c7d) )
	ROM_RELOAD(              0x180000, 0x080000  )
	ROM_LOAD( "ts002e.u36",  0x200000, 0x040000, CRC(611271e6) SHA1(811c21822b074fbb4bb809fed29d48bbd51d57a0) )

	ROM_REGION( 0x100000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "ts010.u4",  0x000000, 0x100000, CRC(df935324) SHA1(73b7aff8800a4e88a47ad426190b73dabdfbf142) )

	ROM_REGION( 0x100000, "view2_1", 0 )   /* Tiles */
	ROM_LOAD( "ts020.u33",  0x000000, 0x100000, CRC(eb58c35d) SHA1(762c5219de6f729a0fc1df90fce09cdf711c2a1e) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x100000, "oki1", 0 )  /* OKI Sample ROM */
	ROM_LOAD( "ts030.u5",    0x000000, 0x100000, CRC(1d68e9d1) SHA1(aaa64a8e8d7cd7f91d2be346fafb9d1f29b40eda) )
ROM_END

ROM_START( bakubrkr ) /* MASTER UP= 92/10/08 20:56:03 */
	ROM_REGION( 0x080000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "ts100j.u18", 0x000000, 0x040000, CRC(8cc0a4fd) SHA1(e7e18b5ea236522a79ba9db8f573ac8f7ade504b) )
	ROM_LOAD16_BYTE( "ts101j.u19", 0x000001, 0x040000, CRC(aea92195) SHA1(e89f964e7e936fd7774f21956eb4ff5c9104837b) )

	ROM_REGION( 0x240000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "ts001j.u37",  0x000000, 0x080000, CRC(70b66e7e) SHA1(307ba27b623f67ee4b4023179870c270bac8ea22) )
	ROM_RELOAD(              0x100000, 0x080000  )
	ROM_LOAD( "ts000j.u38",  0x080000, 0x080000, CRC(a7a94143) SHA1(d811a7597402c161850ddf98cdb00661ea506c7d) )
	ROM_RELOAD(              0x180000, 0x080000  )
	ROM_LOAD( "ts002j.u36",  0x200000, 0x040000, CRC(611271e6) SHA1(811c21822b074fbb4bb809fed29d48bbd51d57a0) )

	ROM_REGION( 0x100000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "ts010.u4",  0x000000, 0x100000, CRC(df935324) SHA1(73b7aff8800a4e88a47ad426190b73dabdfbf142) )

	ROM_REGION( 0x100000, "view2_1", 0 )   /* Tiles */
	ROM_LOAD( "ts020.u33",  0x000000, 0x100000, CRC(eb58c35d) SHA1(762c5219de6f729a0fc1df90fce09cdf711c2a1e) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x100000, "oki1", 0 )  /* OKI Sample ROM */
	ROM_LOAD( "ts030.u5",    0x000000, 0x100000, CRC(1d68e9d1) SHA1(aaa64a8e8d7cd7f91d2be346fafb9d1f29b40eda) )
ROM_END


/***************************************************************************

                                The Berlin Wall

The Berlin Wall, Kaneko 1991, BW-002
(berlwallt set)

----

BW-004 BW-008                    VU-003
BW-005 BW-009                    VU-003
BW-006 BW-00A                    VU-003
BW-007 BW-00B                          6116-90
                                       6116-90
BW-003                           52256  52256
                                 BW101A BW100A
5864
5864                   MUX2      68000
            VIEW2
BW300
BW-002
BW-001                      42101
                            42101
41464 41464      VU-002
41464 41464                      YM2149  IU-004
41464 41464                      YM2149
                           SWB             BW-000  6295
                           SWA


GAL16V8A-25LP : BW-U47 & BW-U48 (backgrounds encryption), BW-U54

***************************************************************************/

ROM_START( berlwall )
	ROM_REGION( 0x040000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "bw100e_u23-01.u23", 0x000000, 0x020000, CRC(76b526ce) SHA1(95ba7cccbe88fd695c28b6a7c25a1afd130c1aa6) )
	ROM_LOAD16_BYTE( "bw101e_u39-01.u39", 0x000001, 0x020000, CRC(78fa7ef2) SHA1(8392de6e307dcd2bf5bcbeb37d578d33246acfcf) )

	ROM_REGION( 0x100000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "bw001.u84",  0x000000, 0x080000, CRC(bc927260) SHA1(44273a8b6a041504d54da4a7897adf23e3e9db10) )
	ROM_LOAD( "bw002.u83",  0x080000, 0x080000, CRC(223f5465) SHA1(6ed077514ab4370a215a4a60c3aecc8b72ed1c97) )

	ROM_REGION( 0x080000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "bw003.u77",  0x000000, 0x080000, CRC(fbb4b72d) SHA1(07a0590f18b3bba1843ef6a89a5c214e8e605cc3) )

	ROM_REGION( 0x400000, "gfx3", 0 )   /* High Color Background */
	ROM_LOAD16_BYTE( "bw004.u73",  0x000000, 0x080000, CRC(5300c34d) SHA1(ccb12ea05f89ef68bcfe003faced2ffea24c4bf0) )
	ROM_LOAD16_BYTE( "bw008.u65",  0x000001, 0x080000, CRC(9aaf2f2f) SHA1(1352856159e19f07e8e30f9c44b21347103ce024) ) // FIXED BITS (xxxxxxx0)
	ROM_LOAD16_BYTE( "bw005.u74",  0x100000, 0x080000, CRC(16db6d43) SHA1(0158d0278d085487400ad4384b8cc9618503319e) )
	ROM_LOAD16_BYTE( "bw009.u66",  0x100001, 0x080000, CRC(1151a0b0) SHA1(584a0da7eb7f06450f95e76faa20d19f053cb74c) ) // FIXED BITS (xxxxxxx0)
	ROM_LOAD16_BYTE( "bw006.u75",  0x200000, 0x080000, CRC(73a35d1f) SHA1(af919cf858c5923aea45e0d8d91493e6284cb99e) )
	ROM_LOAD16_BYTE( "bw00a.u67",  0x200001, 0x080000, CRC(f447dfc2) SHA1(1254eafea92e8e416deedf21cb01990ffc4f896c) ) // FIXED BITS (xxxxxxx0)
	ROM_LOAD16_BYTE( "bw007.u76",  0x300000, 0x080000, CRC(97f85c87) SHA1(865e076e098c49c96639f62be793f2de24b4926b) )
	ROM_LOAD16_BYTE( "bw00b.u68",  0x300001, 0x080000, CRC(b0a48225) SHA1(de256bb6e2a824114274bff0c6c1234934c31c49) ) // FIXED BITS (xxxxxxx0)

	ROM_REGION( 0x040000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "bw000.u46",  0x000000, 0x040000, CRC(d8fe869d) SHA1(75e9044c4164ca6db9519fcff8eca6c8a2d8d5d1) )

	ROM_REGION( 0x600, "plds", 0 )     /* 3 x GAL16V8A-25LP */
	ROM_LOAD( "bw_u47.u47", 0x000, 0x0117, NO_DUMP)
	ROM_LOAD( "bw_u48.u48", 0x200, 0x0117, NO_DUMP)
	ROM_LOAD( "bw_u54.u54", 0x400, 0x0117, NO_DUMP)
ROM_END

ROM_START( berlwallt )
	ROM_REGION( 0x040000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "bw100a.u23", 0x000000, 0x020000, CRC(e6bcb4eb) SHA1(220b8fddc79230b4f6a8cf33e1035355c485e8d1) )
	ROM_LOAD16_BYTE( "bw101a.u39", 0x000001, 0x020000, CRC(38056fb2) SHA1(48338b9a5ebea872286541a3c45016673c4af76b) )

	ROM_REGION( 0x120000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "bw001.u84",  0x000000, 0x080000, CRC(bc927260) SHA1(44273a8b6a041504d54da4a7897adf23e3e9db10) )
	ROM_LOAD( "bw002.u83",  0x080000, 0x080000, CRC(223f5465) SHA1(6ed077514ab4370a215a4a60c3aecc8b72ed1c97) )
	ROM_LOAD( "bw300.u82",  0x100000, 0x020000, CRC(b258737a) SHA1(b5c8fe44a8dcfc19bccba896bdb73030c5843544) ) // Masked players, Japanese text

	ROM_REGION( 0x080000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "bw003.u77",  0x000000, 0x080000, CRC(fbb4b72d) SHA1(07a0590f18b3bba1843ef6a89a5c214e8e605cc3) )

	ROM_REGION( 0x400000, "gfx3", 0 )   /* High Color Background */
	ROM_LOAD16_BYTE( "bw004.u73",  0x000000, 0x080000, CRC(5300c34d) SHA1(ccb12ea05f89ef68bcfe003faced2ffea24c4bf0) )
	ROM_LOAD16_BYTE( "bw008.u65",  0x000001, 0x080000, CRC(9aaf2f2f) SHA1(1352856159e19f07e8e30f9c44b21347103ce024) ) // FIXED BITS (xxxxxxx0)
	ROM_LOAD16_BYTE( "bw005.u74",  0x100000, 0x080000, CRC(16db6d43) SHA1(0158d0278d085487400ad4384b8cc9618503319e) )
	ROM_LOAD16_BYTE( "bw009.u66",  0x100001, 0x080000, CRC(1151a0b0) SHA1(584a0da7eb7f06450f95e76faa20d19f053cb74c) ) // FIXED BITS (xxxxxxx0)
	ROM_LOAD16_BYTE( "bw006.u75",  0x200000, 0x080000, CRC(73a35d1f) SHA1(af919cf858c5923aea45e0d8d91493e6284cb99e) )
	ROM_LOAD16_BYTE( "bw00a.u67",  0x200001, 0x080000, CRC(f447dfc2) SHA1(1254eafea92e8e416deedf21cb01990ffc4f896c) ) // FIXED BITS (xxxxxxx0)
	ROM_LOAD16_BYTE( "bw007.u76",  0x300000, 0x080000, CRC(97f85c87) SHA1(865e076e098c49c96639f62be793f2de24b4926b) )
	ROM_LOAD16_BYTE( "bw00b.u68",  0x300001, 0x080000, CRC(b0a48225) SHA1(de256bb6e2a824114274bff0c6c1234934c31c49) ) // FIXED BITS (xxxxxxx0)

	ROM_REGION( 0x040000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "bw000.u46",  0x000000, 0x040000, CRC(d8fe869d) SHA1(75e9044c4164ca6db9519fcff8eca6c8a2d8d5d1) )

	ROM_REGION( 0x600, "plds", 0 )     /* 3 x GAL16V8A-25LP */
	ROM_LOAD( "bw_u47.u47", 0x000, 0x0117, NO_DUMP)
	ROM_LOAD( "bw_u48.u48", 0x200, 0x0117, NO_DUMP)
	ROM_LOAD( "bw_u54.u54", 0x400, 0x0117, NO_DUMP)
ROM_END

/* This set definitely comes from an original board but suffers the same 'COPY BOARD' issue in MAME, will be checked on the PCB shortly */

ROM_START( berlwallk )
	ROM_REGION( 0x040000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "bw100k.u23", 0x000000, 0x020000, CRC(0ce1d336) SHA1(9dbff4a72f03dd506726c3b305fd0a32e7da4ee1) )
	ROM_LOAD16_BYTE( "bw101k.u39", 0x000001, 0x020000, CRC(3355be65) SHA1(bc3506236ee2f37b3cba7c4d8fe1b4dad61b06f1) )

	ROM_REGION( 0x120000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "bw001.u84",  0x000000, 0x080000, CRC(bc927260) SHA1(44273a8b6a041504d54da4a7897adf23e3e9db10) )
	ROM_LOAD( "bw002.u83",  0x080000, 0x080000, CRC(223f5465) SHA1(6ed077514ab4370a215a4a60c3aecc8b72ed1c97) )
	ROM_LOAD( "bw300k.u82", 0x100000, 0x020000, CRC(b8de79d7) SHA1(c9a78aa213105f3657349995aca2866bc6d80093) )

	ROM_REGION( 0x080000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "bw003.u77",  0x000000, 0x080000, CRC(fbb4b72d) SHA1(07a0590f18b3bba1843ef6a89a5c214e8e605cc3) )

	ROM_REGION( 0x400000, "gfx3", 0 )   /* High Color Background */
	ROM_LOAD16_BYTE( "bw004.u73",  0x000000, 0x080000, CRC(5300c34d) SHA1(ccb12ea05f89ef68bcfe003faced2ffea24c4bf0) )
	ROM_LOAD16_BYTE( "bw008.u65",  0x000001, 0x080000, CRC(9aaf2f2f) SHA1(1352856159e19f07e8e30f9c44b21347103ce024) ) // FIXED BITS (xxxxxxx0)
	ROM_LOAD16_BYTE( "bw005.u74",  0x100000, 0x080000, CRC(16db6d43) SHA1(0158d0278d085487400ad4384b8cc9618503319e) )
	ROM_LOAD16_BYTE( "bw009.u66",  0x100001, 0x080000, CRC(1151a0b0) SHA1(584a0da7eb7f06450f95e76faa20d19f053cb74c) ) // FIXED BITS (xxxxxxx0)
	ROM_LOAD16_BYTE( "bw006.u75",  0x200000, 0x080000, CRC(73a35d1f) SHA1(af919cf858c5923aea45e0d8d91493e6284cb99e) )
	ROM_LOAD16_BYTE( "bw00a.u67",  0x200001, 0x080000, CRC(f447dfc2) SHA1(1254eafea92e8e416deedf21cb01990ffc4f896c) ) // FIXED BITS (xxxxxxx0)
	ROM_LOAD16_BYTE( "bw007.u76",  0x300000, 0x080000, CRC(97f85c87) SHA1(865e076e098c49c96639f62be793f2de24b4926b) )
	ROM_LOAD16_BYTE( "bw00b.u68",  0x300001, 0x080000, CRC(b0a48225) SHA1(de256bb6e2a824114274bff0c6c1234934c31c49) ) // FIXED BITS (xxxxxxx0)

	ROM_REGION( 0x040000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "bw000k.u46",  0x000000, 0x040000, CRC(52e81a50) SHA1(0adf6b42dee244dba2a4fb237155b04699b0254f) )

	ROM_REGION( 0x600, "plds", 0 )     /* 3 x GAL16V8A-25LP */
	ROM_LOAD( "bw_u47.u47", 0x000, 0x0117, NO_DUMP)
	ROM_LOAD( "bw_u48.u48", 0x200, 0x0117, NO_DUMP)
	ROM_LOAD( "bw_u54.u54", 0x400, 0x0117, NO_DUMP)
ROM_END


/***************************************************************************

                                Pack'n Bang Bang

Pack'n Bang Bang, Kaneko 1994, BW-002 (same PCB as Berlin Wall)

For the final/retail version, the observed differences include:
 The use of standard production mask ROMs instead of EPROMs
 Slightly longer more polished DEMO routines
 These tests have been removed from the test menu:
   -4- OBJ CHARA. TEST
   -5- BG 32000 TEST
   -6- BG CHARA. TEST
   -7- SPR ANIME TEST

***************************************************************************/

ROM_START( packbang ) /* Final/retail version */
	ROM_REGION( 0x040000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "bbp0x3_u23.u23", 0x000000, 0x020000, CRC(8f879c9d) SHA1(07c793f486f2c00624ee5b4d982de42358854ac9) ) /* labeled BBP0X3/U23 */
	ROM_LOAD16_BYTE( "bbp1x3_u39.u39", 0x000001, 0x020000, CRC(3a90ad84) SHA1(b25d2de8d6ee15822a5f4ca445956cfce3d30cc4) ) /* labeled BBP1X3/U39 */

	ROM_REGION( 0x120000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "bb-u84-007__w22.u84", 0x000000, 0x080000, CRC(97837aaa) SHA1(303780621afea01f9e4d1386229c7421307562ec) ) /* mask ROM */
	ROM_LOAD( "bbs0x1_u83.u83",      0x080000, 0x040000, CRC(3d95b1e5) SHA1(3bc54b4af4a059feb88779d786798dd06386cc48) ) /* labeled BBS0X1/U83 */

	ROM_REGION( 0x080000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "bbb0x1_u77.u77",  0x000000, 0x080000, CRC(b2ffd081) SHA1(e4b8b60ed0c5f2e0709477cc840864e1c0a351ea) ) // labeled BBB0X1/U77 - 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x400000, "gfx3", 0 )   /* High Color Background */
	ROM_LOAD16_BYTE( "bb-u73-004__w19.u73",  0x000000, 0x080000, CRC(896d88cb) SHA1(7546e64149d8d8e3425d9112a7a63b2d2e59b8bb) ) // These are all mask ROMs
	ROM_LOAD16_BYTE( "bb-u65-001__w16.u65",  0x000001, 0x080000, CRC(fe17c5b5) SHA1(daea65bd87d2137526250d521f36f122f733fd9d) ) // FIXED BITS (xxxxxxx0)
	ROM_LOAD16_BYTE( "bb-u74-005__w20.u74",  0x100000, 0x080000, CRC(b01e77b9) SHA1(73f3adaf6468f4e9c54bff63268af1765cfc5f67) )
	ROM_LOAD16_BYTE( "bb-u66-002__w17.u66",  0x100001, 0x080000, CRC(caec5098) SHA1(9966cd643abe498f84a9e01bc32003f4654584de) ) // FIXED BITS (xxxxxxx0)
	ROM_LOAD16_BYTE( "bb-u75-006__w21.u75",  0x200000, 0x080000, CRC(5cb4669f) SHA1(ab061f5b34435dca46f710ea8118c919a3a9f87c) )
	ROM_LOAD16_BYTE( "bb-u67-003__w18.u67",  0x200001, 0x080000, CRC(ce5c9417) SHA1(30aca496d1f4218b44a32b3630e58889f0c54564) ) // FIXED BITS (xxxxxxx0)

	ROM_REGION( 0x040000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "bw_u46.u46",  0x000000, 0x040000, CRC(d8fe869d) SHA1(75e9044c4164ca6db9519fcff8eca6c8a2d8d5d1) ) /* labeled BW    /U46 */
ROM_END

ROM_START( packbangp ) /* prototype version */
	ROM_REGION( 0x040000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "bbp0x3_527b.u23", 0x000000, 0x020000, CRC(105e978a) SHA1(d2aa72a25b70726ebe4b16bfe16da149bb37cd85) ) /* hand written checksum on label - 527B */
	ROM_LOAD16_BYTE( "bbp1x3_c5c8.u39", 0x000001, 0x020000, CRC(465d36f5) SHA1(d3bc9e5d444e086652d2bc562d9adfb8a1fd0d2d) ) /* hand written checksum on label - C5C8 */

	ROM_REGION( 0x120000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "bb.u84",                       0x000000, 0x080000, CRC(97837aaa) SHA1(303780621afea01f9e4d1386229c7421307562ec) )
	ROM_LOAD( "pb_spr_ext_9_20_ver_ba63.u83", 0x080000, 0x040000, CRC(666a1217) SHA1(0d7b08d63b229d70b7e9e77a36516a695533c4cb) ) /* hand written label plus checksum BA63 */

	ROM_REGION( 0x080000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "bbb0x1.u77",  0x000000, 0x080000, CRC(b2ffd081) SHA1(e4b8b60ed0c5f2e0709477cc840864e1c0a351ea) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x400000, "gfx3", 0 )   /* High Color Background */
	ROM_LOAD16_BYTE( "bb.u73",  0x000000, 0x080000, CRC(896d88cb) SHA1(7546e64149d8d8e3425d9112a7a63b2d2e59b8bb) )
	ROM_LOAD16_BYTE( "bb.u65",  0x000001, 0x080000, CRC(fe17c5b5) SHA1(daea65bd87d2137526250d521f36f122f733fd9d) ) // FIXED BITS (xxxxxxx0)
	ROM_LOAD16_BYTE( "bb.u74",  0x100000, 0x080000, CRC(b01e77b9) SHA1(73f3adaf6468f4e9c54bff63268af1765cfc5f67) )
	ROM_LOAD16_BYTE( "bb.u66",  0x100001, 0x080000, CRC(caec5098) SHA1(9966cd643abe498f84a9e01bc32003f4654584de) ) // FIXED BITS (xxxxxxx0)
	ROM_LOAD16_BYTE( "bb.u75",  0x200000, 0x080000, CRC(5cb4669f) SHA1(ab061f5b34435dca46f710ea8118c919a3a9f87c) )
	ROM_LOAD16_BYTE( "bb.u67",  0x200001, 0x080000, CRC(ce5c9417) SHA1(30aca496d1f4218b44a32b3630e58889f0c54564) ) // FIXED BITS (xxxxxxx0)

	ROM_REGION( 0x040000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "bw000.u46",  0x000000, 0x040000, CRC(d8fe869d) SHA1(75e9044c4164ca6db9519fcff8eca6c8a2d8d5d1) )
ROM_END


/***************************************************************************

                            Blaze On (Japan version)

CPU:          TMP68HC000-12/D780C-2(Z80)
SOUND:        YM2151
OSC:          13.3330/16.000MHz
CUSTOM:       KANEKO VU-002 x2
              KANEKO 23160-509 VIEW2-CHIP
              KANEKO MUX2-CHIP
              KANEKO HELP1-CHIP

---------------------------------------------------
 filemanes          devices       kind
---------------------------------------------------
 BZ_PRG1.U80        27C020        68000 main prg.
 BZ_PRG2.U81        27C020        68000 main prg.
 3.U45              27C010        Z80 sound prg.
 BZ_BG.U2           57C8200       BG CHR
 BZ_SP1.U20         27C8001       OBJ
 BZ_SP2.U21         27C8001       OBJ
 BZ_SP1.U68         27C8001       OBJ (same as BZ_SP1.U20 for 2nd sprite chip)
 BZ_SP2.U86         27C8001       OBJ (same as BZ_SP2.U21 for 2nd sprite chip)

***************************************************************************/

ROM_START( blazeon )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )            /* 68000 Code */
	ROM_LOAD16_BYTE( "bz-prg1.u80", 0x000000, 0x040000, CRC(3d79aa70) SHA1(419088bd4330c674ec395612e0ef4296638080be) )
	ROM_LOAD16_BYTE( "bz-prg2.u81", 0x000001, 0x040000, CRC(a16d3b1e) SHA1(45b205cad4401255725c649353ca86d4204efcfd) )

	ROM_REGION( 0x020000, "audiocpu", 0 )           /* Z80 Code */
	ROM_LOAD( "3.u45", 0x000000, 0x020000, CRC(52fe4c94) SHA1(896230e4627503292575bbd84edc3cf9cb18b27e) )   // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "bz_sp1.u20", 0x000000, 0x100000, CRC(0d5809a1) SHA1(e72669f95b050d1967d10a865bab8f3634c9daad) )
	ROM_LOAD( "bz_sp1.u68", 0x000000, 0x100000, CRC(0d5809a1) SHA1(e72669f95b050d1967d10a865bab8f3634c9daad) )
	ROM_LOAD( "bz_sp2.u21", 0x100000, 0x100000, CRC(56ead2bd) SHA1(463723f3c533603ce3a95310e9ce12b4e582b52d) )
	ROM_LOAD( "bz_sp2.u86", 0x100000, 0x100000, CRC(56ead2bd) SHA1(463723f3c533603ce3a95310e9ce12b4e582b52d) )

	ROM_REGION( 0x100000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "bz_bg.u2", 0x000000, 0x100000, CRC(fc67f19f) SHA1(f5d9e037a736b0932efbfb48587de08bec93df5d) )
ROM_END

ROM_START( blazeonj )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )            /* 68000 Code */
	ROM_LOAD16_BYTE( "bz_prg1.u80", 0x000000, 0x040000, CRC(8409e31d) SHA1(a9dfc299f4b457df190314401aef309adfaf9bae) )
	ROM_LOAD16_BYTE( "bz_prg2.u81", 0x000001, 0x040000, CRC(b8a0a08b) SHA1(5f275b98d3e49a834850b45179d26e8c2f9fd604) )

	ROM_REGION( 0x020000, "audiocpu", 0 )           /* Z80 Code */
	ROM_LOAD( "3.u45", 0x000000, 0x020000, CRC(52fe4c94) SHA1(896230e4627503292575bbd84edc3cf9cb18b27e) )   // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "bz_sp1.u20", 0x000000, 0x100000, CRC(0d5809a1) SHA1(e72669f95b050d1967d10a865bab8f3634c9daad) )
	ROM_LOAD( "bz_sp1.u68", 0x000000, 0x100000, CRC(0d5809a1) SHA1(e72669f95b050d1967d10a865bab8f3634c9daad) )
	ROM_LOAD( "bz_sp2.u21", 0x100000, 0x100000, CRC(56ead2bd) SHA1(463723f3c533603ce3a95310e9ce12b4e582b52d) )
	ROM_LOAD( "bz_sp2.u86", 0x100000, 0x100000, CRC(56ead2bd) SHA1(463723f3c533603ce3a95310e9ce12b4e582b52d) )

	ROM_REGION( 0x100000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "bz_bg.u2", 0x000000, 0x100000, CRC(fc67f19f) SHA1(f5d9e037a736b0932efbfb48587de08bec93df5d) )
ROM_END

/***************************************************************************

                       Wing Force (Japan, prototype)

Proto board without the later Z09AF-00x's Toybox protection MCU.
The missing chip "KD" at U97 is a 74 pqfp just like the Toybox and connected to U92
where the MCU code would be as well as connected to the EEPROM.
Half way between the Blaze On board and the later Toybox protected Z09AF boards.

Z08AT-001 PCB:

    Toshiba TMP68HC000N-16

    2 x VU-002
    VIEW2-CHIP
    MUX2-CHIP
    HELP1-CHIP

    YM2151
    Y3014B

    ATMEL AT93C46 @ U82 (EEPROM, looks like all its pins go to the unpopulated KD MCU)
    DSW8

    XTALs: 16.0000 MHz, 13.3330 MHz

ATLAS-SUB PCB:

    NEC D780C-2 @ U1
    OKI M6295 @ U4
    TL082CP @ U7 (Wide Bandwidth Dual JFET Input Operational AMP)

ROMS:

    Name            Size    Location    Device  Note

    E_2.24.U80      524288  U80         27C040  32 pin riser
    O_2.24.U81      524288  U81         27C040  32 pin riser
    SP0M.U1         524288  U1          27C040  32 pin riser
    SP1M.U1         524288  U1          27C040  32 pin riser
    BG0AM.U2        524288  U2          27C040  42 pin riser
    BG0BM.U2        524288  U2          27C040  42 pin riser
    BG1AM.U3        524288  U3          27C040  42 pin riser
    BG1BM.U3        524288  U3          27C040  42 pin riser
    SP2M.U20        524288  U20         27C040  32 pin riser
    SP3M.U20        524288  U20         27C040  32 pin riser
    SP2M.U68        524288  U68         27C040  32 pin riser
    SP3M.U68        524288  U68         27C040  32 pin riser
    SP0M.U69        524288  U69         27C040  32 pin riser
    SP1M.U69        524288  U69         27C040  32 pin riser
    S-DRV_2.22.U45          U45          27512  under ATLAS SUB
    PCM.U5          524288  U5          27C040  ATLAS SUB
    EEPROM.U7               U7           TL082  ATLAS SUB, not dumped

Video from the PCB:

    http://tmblr.co/ZgJvzv1-0SVG1

***************************************************************************/

ROM_START( wingforc )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "e_2.24.u80", 0x000000, 0x80000, CRC(837e0726) SHA1(349013edc5ccdfd05ae022563e6a831ce98e4a1a) )
	ROM_LOAD16_BYTE( "o_2.24.u81", 0x000001, 0x80000, CRC(b6983437) SHA1(0a124e64ad37f9381f8a10a7b462a29563c2ccd9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )           /* Z80 Code */
	ROM_LOAD( "s-drv_2.22.u45", 0x00000, 0x10000, CRC(ccdc2758) SHA1(5c0448a70306bd7574f35056ad45ffcbd4a866a8) )

	ROM_REGION( 0x200000, "kan_spr", 0 )   /* Sprites */
	// two sprite chips, roms are doubled
	ROM_LOAD( "sp0m.u69", 0x000000, 0x80000, CRC(8be26a05) SHA1(5b54cd74235c0e32a234ddbe9cf26817700451f1) )
	ROM_LOAD( "sp0m.u1",  0x000000, 0x80000, CRC(8be26a05) SHA1(5b54cd74235c0e32a234ddbe9cf26817700451f1) )

	ROM_LOAD( "sp1m.u1",  0x080000, 0x80000, CRC(ad8c5b68) SHA1(438b58df301a06266284aad63e19d607d7e9f726) )
	ROM_LOAD( "sp1m.u69", 0x080000, 0x80000, CRC(ad8c5b68) SHA1(438b58df301a06266284aad63e19d607d7e9f726) )

	ROM_LOAD( "sp2m.u20", 0x100000, 0x80000, CRC(b5994bda) SHA1(66bd9664e31ac0c831a1c538d895386cabb03ac8) )
	ROM_LOAD( "sp2m.u68", 0x100000, 0x80000, CRC(b5994bda) SHA1(66bd9664e31ac0c831a1c538d895386cabb03ac8) )

	ROM_LOAD( "sp3m.u20", 0x180000, 0x80000, CRC(889ddf72) SHA1(1eaeb4580133d38185ff52fbdc445744c207a202) )
	ROM_LOAD( "sp3m.u68", 0x180000, 0x80000, CRC(889ddf72) SHA1(1eaeb4580133d38185ff52fbdc445744c207a202) )

	ROM_REGION( 0x200000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD16_BYTE( "bg0am.u2", 0x000000, 0x80000, CRC(f4276860) SHA1(8c0848d43ec07f88734993a996a62919979c75ea) )
	ROM_LOAD16_BYTE( "bg0bm.u2", 0x000001, 0x80000, CRC(9df92283) SHA1(53bcac1d63b7bb84b664507906ee768a83be28c9) )
	ROM_LOAD16_BYTE( "bg1am.u3", 0x100000, 0x80000, CRC(a44fdebb) SHA1(676ade63d22818c7a7adf39d42aad41fa93319d2) )
	ROM_LOAD16_BYTE( "bg1bm.u3", 0x100001, 0x80000, CRC(a9b9fc5d) SHA1(33db691007a8cf25aea6b87a0f009c50df2676f2) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x80000, "oki1", 0 )  /* OKI Sample ROM */
	ROM_LOAD( "pcm.u5", 0x00000, 0x80000, CRC(233569fd) SHA1(eb835008bcb961528c0ef4ca72e44ee08c517b81) )
ROM_END


/***************************************************************************

                                Blood Warrior

Kaneko 1994

TOP BOARD: Z09AF-005

CPU - Toshiba TMP68HC000N - 16
SOUND - OKI M6295  x2
QUARTZ OSCILLATORS AT 27.0000MHz, 16.0000MHz, 20.0000MHz and 33.3330MHz
RAM - LC3664 x6, 424260 x2, LH52B256D x6, D42101C x2


PCU11.u11 - 18CV8PC    \
PCU15.u15 - 18CV8PC     | NEAR OKI
Z091P016.u16 - 18CV8PC /
Z091P013.u13 - 18CV8PC \
PCU17.u17 - 18CV8PC     |
PCU14.u14 - 18CV8PC     | NEAR 68000
PCU94.u94 - 18CV8PC     |
PCU92.u92 - 18CV8PC     |
Z092P093.u93 - 18CV8PC /
ALL ABOVE NOT DUMPED

Custom Chips

231609-509 View2-Chip x2
KC-002 L0002 023 9321EK702
9343T - 44 pin PQFP (NEAR JAMMMA CONNECTOR)

BOTTOM BOARD: Z09AF-ROM4

TBS0P01 452 9339PK001 - 74 pin PQFP (NEC uPD78324 series MCU)

ofs1g006.u6 - GAL16V8B
ofs1g007.u7 - GAL16V8B
ofs1p059.u59 - 18CV8PC
ofs1p511.u511 - 18CV8PC
ALL ABOVE NOT DUMPED

ROMS

9346.u126 - 93C46
ofdox3.U124 - 27C010
ofp1f3.U513 - 27C4000
ofpof3.U514 - 27C4000
of101f0223.U101 - 27C800
of20902011.u17 - 27C080
of210000213.u19 - 27C080
of2110215.u21 - 27C080
of21200217.u23 - 27C080
of21300219.u25 - 27C080
of21400221.u27 - 27C080
of1000222.u99 - 27C800
of0010226.u55 - 27C800
of3000225.u51 - 27C800
of2080209.u28 - 27V160
of2060207.u14 - 27V160
of2070208.u15 - 27V160
of2050206.u13 - 27V160
of2040205.u12 - 27V160
of2030204.u11 - 27V160
of2020203.u10 - 27V160
of2010202.u9 - 27V160
of2000201.u8 - 27V160
of209e0210.u16 - 27C080
of210e0212.u18 - 27C080
of211e0214.u20 - 27C080
of212e0216.u22 - 27C080
of213e0218.u24 - 27C080
of214e0220.u26 - 27C080

***************************************************************************/

ROM_START( bloodwar )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "ofp0f3.514", 0x000000, 0x080000, CRC(0c93da15) SHA1(65b6b1b4acfc32c551ae4fbe6a13f7f2b8554dbf) )
	ROM_LOAD16_BYTE( "ofp1f3.513", 0x000001, 0x080000, CRC(894ecbe5) SHA1(bf403d19e6315266114ac742a08cac903e7b54b5) )

	ROM_REGION( 0x020000, "mcudata", 0 )            /* MCU Code */
	ROM_LOAD16_WORD_SWAP( "ofd0x3.124",  0x000000, 0x020000, CRC(399f2005) SHA1(ff0370724770c35963953fd9596d9f808ba87d8f) )

	ROM_REGION( 0x1e00000, "kan_spr", 0 )  /* Sprites */
	ROM_LOAD       ( "of-200-0201.8",   0x0000000, 0x200000, CRC(bba63025) SHA1(daec5285469ee953f6f838fe3cb3903524e9ac39) )
	ROM_LOAD       ( "of-201-0202.9",   0x0200000, 0x200000, CRC(4ffd9ddc) SHA1(62bc8c0ed2efab407fc2956c514c3e732bcc47ee) )
	ROM_LOAD       ( "of-202-0203.10",  0x0400000, 0x200000, CRC(fbcc5363) SHA1(9eff48c29d5c887d39e4db442c6ee51ec879521e) )
	ROM_LOAD       ( "of-203-0204.11",  0x0600000, 0x200000, CRC(8e818ce9) SHA1(bc37d35247edfc563400cd67459d455b1fea6eab) )
	ROM_LOAD       ( "of-204-0205.12",  0x0800000, 0x200000, CRC(70c4a76b) SHA1(01b17bda156f2e6f480bdc976927c8bba47c1186) )
	ROM_LOAD       ( "of-205-0206.13",  0x0a00000, 0x200000, CRC(80c667bb) SHA1(7edf33c713c8448ff73fa84b9f7684dd4d46eed1) )
	ROM_LOAD       ( "of-206-0207.14",  0x0c00000, 0x200000, CRC(c2028c97) SHA1(ac3b73ff34f84015432ceb22cf9c57ab0ff07a70) )
	ROM_LOAD       ( "of-207-0208.15",  0x0e00000, 0x200000, CRC(b1f30c61) SHA1(2ae010c10b7a2ae09df904f7ea81425e80389622) )
	ROM_LOAD       ( "of-208-0209.28",  0x1000000, 0x200000, CRC(a8f29545) SHA1(5d018147aa71207f679909343104deaa0f08fd9d) )
	ROM_LOAD16_BYTE( "of-209e-0210.16", 0x1200000, 0x100000, CRC(93018468) SHA1(d156f408a78fbd736048ce33e44c0b1e10403b0e) )
	ROM_LOAD16_BYTE( "of-2090-2011.17", 0x1200001, 0x100000, CRC(3fb226a1) SHA1(efba54f82fb9914559faad5fba92aa108ec039d5) )
	ROM_LOAD16_BYTE( "of-210e-0212.18", 0x1400000, 0x100000, CRC(80f3fa1b) SHA1(ca0e84cb47228ef5ac3b94238a33c3ebc3c2f528) )
	ROM_LOAD16_BYTE( "of-2100-0213.19", 0x1400001, 0x100000, CRC(8ca3a3d6) SHA1(b6f3876f987ce6828bfa26ca492ff6ca2d282d80) )
	ROM_LOAD16_BYTE( "of-211e-0214.20", 0x1600000, 0x100000, CRC(8d3d96f7) SHA1(0a7c459f02938f86d53979498647c73837eb9e51) )
	ROM_LOAD16_BYTE( "of-2110-0215.21", 0x1600001, 0x100000, CRC(78268230) SHA1(63fbf88551f6fde833222ae6a4382891e1bf5b39) )
	ROM_LOAD16_BYTE( "of-212e-0216.22", 0x1800000, 0x100000, CRC(5a013d99) SHA1(c4af944c8f0b33a93b4bb083e32e2901c5607a39) )
	ROM_LOAD16_BYTE( "of-2120-0217.23", 0x1800001, 0x100000, CRC(84ed25bd) SHA1(ddff811d326586eb7353230e74db37867af075eb) )
	ROM_LOAD16_BYTE( "of-213e-0218.24", 0x1a00000, 0x100000, CRC(861bc5b1) SHA1(a85936781a56b5406bee2e4f36fadcb5b9f43b05) )
	ROM_LOAD16_BYTE( "of-2130-0219.25", 0x1a00001, 0x100000, CRC(a79b8119) SHA1(62e3fb28fd3a538a8191a51242dbed7e88c62a54) )
	ROM_LOAD16_BYTE( "of-214e-0220.26", 0x1c00000, 0x100000, CRC(43c622de) SHA1(73efe57233f056127e2d34590c624f39d1c0ab79) )
	ROM_LOAD16_BYTE( "of-2140-0221.27", 0x1c00001, 0x100000, CRC(d10bf03c) SHA1(a81d6b7df7382fc8d50614c1332611e0c202b805) )

	ROM_REGION( 0x100000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "of-300-0225.51",    0x000000, 0x100000, CRC(fbc3c08a) SHA1(0ba52b381e7a10fb1513244b394438b440950af3) )

	ROM_REGION( 0x100000, "view2_1", 0 )   /* Tiles */
	ROM_LOAD( "of-301-0226.55",    0x000000, 0x100000, CRC(fcf215de) SHA1(83015f10e62b917efd6e3edfbd45fb8f9b35db2b) )

	ROM_REGION( 0x100000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "of-101-f-0223.101", 0x000000, 0x100000, CRC(295f3c93) SHA1(558698f1d04b23dd2a73e2eae5ecce598defb228) ) /* 0224 ? */

	ROM_REGION( 0x100000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "of-100-0222.99",    0x000000, 0x100000, CRC(42b12269) SHA1(f9d9c42057e176710f09e8db0bfcbf603c15ca11) )
ROM_END

ROM_START( oedfight ) // Bloodshed version
	ROM_REGION( 0x100000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "ofp0j3.514", 0x000000, 0x080000, CRC(0c93da15) SHA1(65b6b1b4acfc32c551ae4fbe6a13f7f2b8554dbf) )
	ROM_LOAD16_BYTE( "ofp1j3.513", 0x000001, 0x080000, CRC(cc59de49) SHA1(48ff4ed40ad22768054a59bdf5ce0e00891d8f0e) )

	ROM_REGION( 0x020000, "mcudata", 0 )            /* MCU Code */
	ROM_LOAD16_WORD_SWAP( "ofd0x3.124",  0x000000, 0x020000, CRC(399f2005) SHA1(ff0370724770c35963953fd9596d9f808ba87d8f) )

	ROM_REGION( 0x1e00000, "kan_spr", 0 )  /* Sprites */
	ROM_LOAD       ( "of-200-0201.8",   0x0000000, 0x200000, CRC(bba63025) SHA1(daec5285469ee953f6f838fe3cb3903524e9ac39) )
	ROM_LOAD       ( "of-201-0202.9",   0x0200000, 0x200000, CRC(4ffd9ddc) SHA1(62bc8c0ed2efab407fc2956c514c3e732bcc47ee) )
	ROM_LOAD       ( "of-202-0203.10",  0x0400000, 0x200000, CRC(fbcc5363) SHA1(9eff48c29d5c887d39e4db442c6ee51ec879521e) )
	ROM_LOAD       ( "of-203-0204.11",  0x0600000, 0x200000, CRC(8e818ce9) SHA1(bc37d35247edfc563400cd67459d455b1fea6eab) )
	ROM_LOAD       ( "of-204-0205.12",  0x0800000, 0x200000, CRC(70c4a76b) SHA1(01b17bda156f2e6f480bdc976927c8bba47c1186) )
	ROM_LOAD       ( "of-205-0206.13",  0x0a00000, 0x200000, CRC(80c667bb) SHA1(7edf33c713c8448ff73fa84b9f7684dd4d46eed1) )
	ROM_LOAD       ( "of-206-0207.14",  0x0c00000, 0x200000, CRC(c2028c97) SHA1(ac3b73ff34f84015432ceb22cf9c57ab0ff07a70) )
	ROM_LOAD       ( "of-207-0208.15",  0x0e00000, 0x200000, CRC(b1f30c61) SHA1(2ae010c10b7a2ae09df904f7ea81425e80389622) )
	ROM_LOAD       ( "of-208-0209.28",  0x1000000, 0x200000, CRC(a8f29545) SHA1(5d018147aa71207f679909343104deaa0f08fd9d) )
	ROM_LOAD16_BYTE( "of-209e-0210.16", 0x1200000, 0x100000, CRC(93018468) SHA1(d156f408a78fbd736048ce33e44c0b1e10403b0e) )
	ROM_LOAD16_BYTE( "of-2090-2011.17", 0x1200001, 0x100000, CRC(3fb226a1) SHA1(efba54f82fb9914559faad5fba92aa108ec039d5) )
	ROM_LOAD16_BYTE( "of-210e-0212.18", 0x1400000, 0x100000, CRC(80f3fa1b) SHA1(ca0e84cb47228ef5ac3b94238a33c3ebc3c2f528) )
	ROM_LOAD16_BYTE( "of-2100-0213.19", 0x1400001, 0x100000, CRC(8ca3a3d6) SHA1(b6f3876f987ce6828bfa26ca492ff6ca2d282d80) )
	ROM_LOAD16_BYTE( "of-211e-0214.20", 0x1600000, 0x100000, CRC(8d3d96f7) SHA1(0a7c459f02938f86d53979498647c73837eb9e51) )
	ROM_LOAD16_BYTE( "of-2110-0215.21", 0x1600001, 0x100000, CRC(78268230) SHA1(63fbf88551f6fde833222ae6a4382891e1bf5b39) )
	ROM_LOAD16_BYTE( "of-212e-0216.22", 0x1800000, 0x100000, CRC(5a013d99) SHA1(c4af944c8f0b33a93b4bb083e32e2901c5607a39) )
	ROM_LOAD16_BYTE( "of-2120-0217.23", 0x1800001, 0x100000, CRC(84ed25bd) SHA1(ddff811d326586eb7353230e74db37867af075eb) )
	ROM_LOAD16_BYTE( "of-213e-0218.24", 0x1a00000, 0x100000, CRC(861bc5b1) SHA1(a85936781a56b5406bee2e4f36fadcb5b9f43b05) )
	ROM_LOAD16_BYTE( "of-2130-0219.25", 0x1a00001, 0x100000, CRC(a79b8119) SHA1(62e3fb28fd3a538a8191a51242dbed7e88c62a54) )
	ROM_LOAD16_BYTE( "of-214e-0220.26", 0x1c00000, 0x100000, CRC(43c622de) SHA1(73efe57233f056127e2d34590c624f39d1c0ab79) )
	ROM_LOAD16_BYTE( "of-2140-0221.27", 0x1c00001, 0x100000, CRC(d10bf03c) SHA1(a81d6b7df7382fc8d50614c1332611e0c202b805) )

	ROM_REGION( 0x100000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "of-300-0225.51",    0x000000, 0x100000, CRC(fbc3c08a) SHA1(0ba52b381e7a10fb1513244b394438b440950af3) )

	ROM_REGION( 0x100000, "view2_1", 0 )   /* Tiles */
	ROM_LOAD( "of-301-0226.55",    0x000000, 0x100000, CRC(fcf215de) SHA1(83015f10e62b917efd6e3edfbd45fb8f9b35db2b) )

	ROM_REGION( 0x100000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "of-101-j-0224.101", 0x000000, 0x100000, CRC(83a1f826) SHA1(3b5f576735d5954770cd572b03e71bf121ae88d2) )

	ROM_REGION( 0x100000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "of-100-0222.99",    0x000000, 0x100000, CRC(42b12269) SHA1(f9d9c42057e176710f09e8db0bfcbf603c15ca11) )
ROM_END

ROM_START( oedfighta ) // Original Bloodless version
	ROM_REGION( 0x100000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "ofp0j3_u514.u514", 0x000000, 0x080000, CRC(b5146417) SHA1(24d51f342c05e4f36aff8a47e0a7f8fdbb1f04d7) ) // labeled OFP0J3/U514
	ROM_LOAD16_BYTE( "ofp1j3_u513.u513", 0x000001, 0x080000, CRC(afe42a8a) SHA1(817241463feb9f5b94526e5ea83ba8e5f1edf8ef) ) // labeled OFP1J3/U513

	ROM_REGION( 0x020000, "mcudata", 0 )            /* MCU Code */
	ROM_LOAD16_WORD_SWAP( "ofd0x3.124",  0x000000, 0x020000, CRC(399f2005) SHA1(ff0370724770c35963953fd9596d9f808ba87d8f) )

	ROM_REGION( 0x1e00000, "kan_spr", 0 )  /* Sprites */
	ROM_LOAD       ( "of-200-0201.8",   0x0000000, 0x200000, CRC(bba63025) SHA1(daec5285469ee953f6f838fe3cb3903524e9ac39) )
	ROM_LOAD       ( "of-201-0202.9",   0x0200000, 0x200000, CRC(4ffd9ddc) SHA1(62bc8c0ed2efab407fc2956c514c3e732bcc47ee) )
	ROM_LOAD       ( "of-202-0203.10",  0x0400000, 0x200000, CRC(fbcc5363) SHA1(9eff48c29d5c887d39e4db442c6ee51ec879521e) )
	ROM_LOAD       ( "of-203-0204.11",  0x0600000, 0x200000, CRC(8e818ce9) SHA1(bc37d35247edfc563400cd67459d455b1fea6eab) )
	ROM_LOAD       ( "of-204-0205.12",  0x0800000, 0x200000, CRC(70c4a76b) SHA1(01b17bda156f2e6f480bdc976927c8bba47c1186) )
	ROM_LOAD       ( "of-205-0206.13",  0x0a00000, 0x200000, CRC(80c667bb) SHA1(7edf33c713c8448ff73fa84b9f7684dd4d46eed1) )
	ROM_LOAD       ( "of-206-0207.14",  0x0c00000, 0x200000, CRC(c2028c97) SHA1(ac3b73ff34f84015432ceb22cf9c57ab0ff07a70) )
	ROM_LOAD       ( "of-207-0208.15",  0x0e00000, 0x200000, CRC(b1f30c61) SHA1(2ae010c10b7a2ae09df904f7ea81425e80389622) )
	ROM_LOAD       ( "of-208-0209.28",  0x1000000, 0x200000, CRC(a8f29545) SHA1(5d018147aa71207f679909343104deaa0f08fd9d) )
	ROM_LOAD16_BYTE( "of-209e-0210.16", 0x1200000, 0x100000, CRC(93018468) SHA1(d156f408a78fbd736048ce33e44c0b1e10403b0e) )
	ROM_LOAD16_BYTE( "of-2090-2011.17", 0x1200001, 0x100000, CRC(3fb226a1) SHA1(efba54f82fb9914559faad5fba92aa108ec039d5) )
	ROM_LOAD16_BYTE( "of-210e-0212.18", 0x1400000, 0x100000, CRC(80f3fa1b) SHA1(ca0e84cb47228ef5ac3b94238a33c3ebc3c2f528) )
	ROM_LOAD16_BYTE( "of-2100-0213.19", 0x1400001, 0x100000, CRC(8ca3a3d6) SHA1(b6f3876f987ce6828bfa26ca492ff6ca2d282d80) )
	ROM_LOAD16_BYTE( "of-211e-0214.20", 0x1600000, 0x100000, CRC(8d3d96f7) SHA1(0a7c459f02938f86d53979498647c73837eb9e51) )
	ROM_LOAD16_BYTE( "of-2110-0215.21", 0x1600001, 0x100000, CRC(78268230) SHA1(63fbf88551f6fde833222ae6a4382891e1bf5b39) )
	ROM_LOAD16_BYTE( "of-212e-0216.22", 0x1800000, 0x100000, CRC(5a013d99) SHA1(c4af944c8f0b33a93b4bb083e32e2901c5607a39) )
	ROM_LOAD16_BYTE( "of-2120-0217.23", 0x1800001, 0x100000, CRC(84ed25bd) SHA1(ddff811d326586eb7353230e74db37867af075eb) )
	ROM_LOAD16_BYTE( "of-213e-0218.24", 0x1a00000, 0x100000, CRC(861bc5b1) SHA1(a85936781a56b5406bee2e4f36fadcb5b9f43b05) )
	ROM_LOAD16_BYTE( "of-2130-0219.25", 0x1a00001, 0x100000, CRC(a79b8119) SHA1(62e3fb28fd3a538a8191a51242dbed7e88c62a54) )
	ROM_LOAD16_BYTE( "of-214e-0220.26", 0x1c00000, 0x100000, CRC(43c622de) SHA1(73efe57233f056127e2d34590c624f39d1c0ab79) )
	ROM_LOAD16_BYTE( "of-2140-0221.27", 0x1c00001, 0x100000, CRC(d10bf03c) SHA1(a81d6b7df7382fc8d50614c1332611e0c202b805) )

	ROM_REGION( 0x100000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "of-300-0225.51",    0x000000, 0x100000, CRC(fbc3c08a) SHA1(0ba52b381e7a10fb1513244b394438b440950af3) )

	ROM_REGION( 0x100000, "view2_1", 0 )   /* Tiles */
	ROM_LOAD( "of-301-0226.55",    0x000000, 0x100000, CRC(fcf215de) SHA1(83015f10e62b917efd6e3edfbd45fb8f9b35db2b) )

	ROM_REGION( 0x100000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "of-101-j-0224.101", 0x000000, 0x100000, CRC(83a1f826) SHA1(3b5f576735d5954770cd572b03e71bf121ae88d2) )

	ROM_REGION( 0x100000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "of-100-0222.99",    0x000000, 0x100000, CRC(42b12269) SHA1(f9d9c42057e176710f09e8db0bfcbf603c15ca11) )
ROM_END


/***************************************************************************

                            Great 1000 Miles Rally

Kaneko 1994

TOP BOARD: Z09AF-005

CPU - Toshiba TMP68HC000N - 16
SOUND - OKI M6295  x2
QUARTZ OSCILLATORS AT 27.0000MHz, 16.0000MHz, 20.0000MHz and 33.3330MHz
RAM - LC3664 x6, 424260 x2, LH52B256D x6, D42101C x2

Custom Chips

231609-509 View2-Chip x2
KC-002 L0002 023 9321EK702
9343T - 44 pin PQFP (NEAR JAMMMA CONNECTOR)

BOTTOM BOARD: Z09AF-ROM5

TBS0P01 452 9339PK001 - 74 pin PQFP (NEC uPD78324 series MCU)

GMMU2+1 512K * 2    68k
GMMU23  1M      OKI6295: 00000-2ffff + chunks of 0x10000 with headers
GMMU24  1M      OKI6295: chunks of 0x40000 with headers - FIRST AND SECOND HALF IDENTICAL

GMMU27  2M      sprites
GMMU28  2M      sprites
GMMU29  2M      sprites
GMMU30  512k    sprites

GMMU64  1M      sprites - FIRST AND SECOND HALF IDENTICAL
GMMU65  1M      sprites - FIRST AND SECOND HALF IDENTICAL

GMMU52  2M      tiles


---------------------------------------------------------------------------
                                Game code
---------------------------------------------------------------------------

100000.b    <- (!b00000.b) & 7f [1p]
    01.b    previous value of the above
    02.b    bits gone high

100008.b    <- (!b00002.b) & 7f [2p]

100010.b    <- !b00004.b [coins]
    11.b    previous value of the above
    12.b    bits gone high

100013.b    <- b00006.b (both never accessed again?)

100015.b    <- wheel value

600000.w    <- 100a20.w + 100a30.w      600002.w    <- 100a22.w + 100a32.w
600004.w    <- 100a24.w + 100a34.w      600006.w    <- 100a26.w + 100a36.w

680000.w    <- 100a28.w + 100a38.w      680002.w    <- 100a2a.w + 100a3a.w
680004.w    <- 100a2c.w + 100a3c.w      680006.w    <- 100a2e.w + 100a3e.w

101265.b    <- DSW (from 206000)
101266      <- Settings from NVRAM (0x80 bytes from 208000)

1034f8.b    credits
103502.b    coins x ..
103503.b    .. credits

1035ec.l    *** Time (BCD: seconds * 10000) ***
103e64.w    *** Speed << 4 ***

10421a.b    bank for the oki mapped at 800000
104216.b    last value of the above

10421c.b    bank for the oki mapped at 880000
104218.b    last value of the above

ROUTINES:

dd6 print string: a2->scr ; a1->string ; d1.l = xpos.w<<6|ypos.w<<6

Trap #2 = 43a0 ; d0.w = routine index ; (where not specified: 43c0):
1:  43C4    2:  43F8    3:  448E    4:  44EE
5:  44D2    6:  4508    7:  453A    10: 0AF6
18: 4580    19: 4604
20> 2128    writes 700000-70001f
21: 21F6
24> 2346    clears 400000-401407 (641*8 = $281*8)
30> 282A    writes 600008/9/b/e-f, 680008/9/b/e-f
31: 295A
32> 2B36    100a30-f <- 100a10-f
34> 2B4C    clears 500000-503fff, 580000-583fff
35> 2B9E    d1.w = selects between: 500000;501000;580000;581000.
            Fill 0x1000 bytes from there with d2.l

70: 2BCE>   11d8a
71: 2BD6
74: 2BDE    90: 3D44
91> 3D4C    wait for bit 0 of d00000 to be 0
92> 3D5C    200010.w<-D1    200012.w<-D2    200014.w<-D3
f1: 10F6

***************************************************************************/

/*  This version displays:

    tb05mm-eu "1000 miglia"
    master up= 94/07/18 15:12:35            */

ROM_START( gtmr )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "u2.bin", 0x000000, 0x080000, CRC(031799f7) SHA1(a59a9635002d139247828e3b74f6cf2fbdd5e569) )
	ROM_LOAD16_BYTE( "u1.bin", 0x000001, 0x080000, CRC(6238790a) SHA1(a137fd581138804534f3193068f117611a982004) )

	ROM_REGION( 0x020000, "mcudata", 0 )            /* MCU Code */
	ROM_LOAD16_WORD_SWAP( "mmd0x2.u124.bin",  0x000000, 0x020000, CRC(3d7cb329) SHA1(053106acde642a414fde0b01105fe6762b6a10f6) ) // from gtmra

	ROM_REGION( 0x840000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "mm-200-402-s0.bin",  0x000000, 0x200000, CRC(c0ab3efc) SHA1(e6cd15480977b036234d91e6f3a6e21b7f0a3c3e) )
	ROM_LOAD( "mm-201-403-s1.bin",  0x200000, 0x200000, CRC(cf6b23dc) SHA1(ccfd0b17507e091e55c169361cd6a6b19641b717) )
	ROM_LOAD( "mm-202-404-s2.bin",  0x400000, 0x200000, CRC(8f27f5d3) SHA1(219a86446ce2556682009d8aff837480f040a01e) )
	ROM_LOAD( "mm-203-405-s3.bin",  0x600000, 0x080000, CRC(e9747c8c) SHA1(2507102ec34755c6f110eadb3444e6d3a3474051) )
	ROM_LOAD16_BYTE( "mms1x2.u30.bin",  0x800001, 0x020000, CRC(b42b426f) SHA1(6aee5759b5f0786c5ee074d9df3d2716919ea621) )
	ROM_LOAD16_BYTE( "mms0x2.u29.bin",  0x800000, 0x020000, CRC(bd22b7d2) SHA1(ef82d00d72439590c71aed33ecfabc6ee71a6ff9) )

	ROM_REGION( 0x200000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "mm-300-406-a0.bin",  0x000000, 0x200000, CRC(b15f6b7f) SHA1(5e84919d788add53fc87f4d85f437df413b1dbc5) )

	ROM_REGION( 0x200000, "view2_1", 0 )   /* Tiles */
	ROM_COPY("view2_0",0x000000,0,0x200000) // it isn't on the board twice.

	ROM_REGION( 0x100000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "mm-100-401-e0.bin",  0x000000, 0x100000, CRC(b9cbfbee) SHA1(051d48a68477ef9c29bd5cc0bb7955d513a0ab94) )  // 16 x $10000

	ROM_REGION( 0x100000, "oki2", ROMREGION_ERASE00 )   /* Samples */
	/* Not present on this board */
ROM_END

ROM_START( gtmra )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "mmp0x2.u514.bin", 0x000000, 0x080000, CRC(ba4a77c8) SHA1(efb6ae0e7aa71ab0c5f486f799bf31edcec24e2b) )
	ROM_LOAD16_BYTE( "mmp1x2.u513.bin", 0x000001, 0x080000, CRC(a2b9034e) SHA1(466bcb1bf7124eb15d23b25c4e1307b9706474ec) )

	ROM_REGION( 0x020000, "mcudata", 0 )            /* MCU Code */
	ROM_LOAD16_WORD_SWAP( "mmd0x2.u124.bin",  0x000000, 0x020000, CRC(3d7cb329) SHA1(053106acde642a414fde0b01105fe6762b6a10f6) )

	ROM_REGION( 0x840000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "mm-200-402-s0.bin",  0x000000, 0x200000, CRC(c0ab3efc) SHA1(e6cd15480977b036234d91e6f3a6e21b7f0a3c3e) )
	ROM_LOAD( "mm-201-403-s1.bin",  0x200000, 0x200000, CRC(cf6b23dc) SHA1(ccfd0b17507e091e55c169361cd6a6b19641b717) )
	ROM_LOAD( "mm-202-404-s2.bin",  0x400000, 0x200000, CRC(8f27f5d3) SHA1(219a86446ce2556682009d8aff837480f040a01e) )
	ROM_LOAD( "mm-203-405-s3.bin",  0x600000, 0x080000, CRC(e9747c8c) SHA1(2507102ec34755c6f110eadb3444e6d3a3474051) )
	ROM_LOAD16_BYTE( "mms1x2.u30.bin",  0x800001, 0x020000, CRC(b42b426f) SHA1(6aee5759b5f0786c5ee074d9df3d2716919ea621) )
	ROM_LOAD16_BYTE( "mms0x2.u29.bin",  0x800000, 0x020000, CRC(bd22b7d2) SHA1(ef82d00d72439590c71aed33ecfabc6ee71a6ff9) )

	ROM_REGION( 0x200000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "mm-300-406-a0.bin",  0x000000, 0x200000, CRC(b15f6b7f) SHA1(5e84919d788add53fc87f4d85f437df413b1dbc5) )

	ROM_REGION( 0x200000, "view2_1", 0 )   /* Tiles */
	ROM_COPY("view2_0",0x000000,0,0x200000) // it isn't on the board twice.

	ROM_REGION( 0x100000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "mm-100-401-e0.bin",  0x000000, 0x100000, CRC(b9cbfbee) SHA1(051d48a68477ef9c29bd5cc0bb7955d513a0ab94) )  // 16 x $10000

	ROM_REGION( 0x100000, "oki2", ROMREGION_ERASE00 )   /* Samples */
	/* Not present on this board */
ROM_END

ROM_START( gtmrb )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "mmp0x1.u514", 0x000000, 0x080000, CRC(6c163f12) SHA1(7f33d1475dcb754c83f68b5fb686fb236ba81256) )
	ROM_LOAD16_BYTE( "mmp1x1.u513", 0x000001, 0x080000, CRC(424dc7e1) SHA1(a9cb8d1fd0549c8c77462552c649c180c30eef89) )

	ROM_REGION( 0x020000, "mcudata", 0 )            /* MCU Code */
	ROM_LOAD16_WORD_SWAP( "mmd0x1.u124",  0x000000, 0x020000, CRC(3d7cb329) SHA1(053106acde642a414fde0b01105fe6762b6a10f6) ) // == mmd0x2

	ROM_REGION( 0x840000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "mm-200-402-s0.bin",  0x000000, 0x200000, CRC(c0ab3efc) SHA1(e6cd15480977b036234d91e6f3a6e21b7f0a3c3e) )
	ROM_LOAD( "mm-201-403-s1.bin",  0x200000, 0x200000, CRC(cf6b23dc) SHA1(ccfd0b17507e091e55c169361cd6a6b19641b717) )
	ROM_LOAD( "mm-202-404-s2.bin",  0x400000, 0x200000, CRC(8f27f5d3) SHA1(219a86446ce2556682009d8aff837480f040a01e) )
	ROM_LOAD( "mm-203-405-s3.bin",  0x600000, 0x080000, CRC(e9747c8c) SHA1(2507102ec34755c6f110eadb3444e6d3a3474051) )
	ROM_LOAD16_BYTE( "mms1x1.u30",  0x800001, 0x020000, CRC(9463825c) SHA1(696bbfc816b564b3cff1487e1b848d375951f923) )
	ROM_LOAD16_BYTE( "mms0x1.u29",  0x800000, 0x020000, CRC(bd22b7d2) SHA1(ef82d00d72439590c71aed33ecfabc6ee71a6ff9) ) // == mms0x2

	ROM_REGION( 0x200000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "mm-300-406-a0.bin",  0x000000, 0x200000, CRC(b15f6b7f) SHA1(5e84919d788add53fc87f4d85f437df413b1dbc5) )

	ROM_REGION( 0x200000, "view2_1", 0 )   /* Tiles */
	ROM_COPY("view2_0",0x000000,0,0x200000) // it isn't on the board twice.

	ROM_REGION( 0x100000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "mm-100-401-e0.bin",  0x000000, 0x100000, CRC(b9cbfbee) SHA1(051d48a68477ef9c29bd5cc0bb7955d513a0ab94) )  // 16 x $10000

	ROM_REGION( 0x100000, "oki2", ROMREGION_ERASE00 )   /* Samples */
	/* Not present on this board */
ROM_END


ROM_START( gtmro )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "u514.bin", 0x000000, 0x080000, CRC(2e857685) SHA1(43b6d88df51a3b4fb0cb910f63a5ec26b06e216a) )
	ROM_LOAD16_BYTE( "u513.bin", 0x000001, 0x080000, CRC(d5003870) SHA1(6a4353fa8f94a119c23f232861d790f50be26ea8) )

	ROM_REGION( 0x020000, "mcudata", 0 )            /* MCU Code */
	ROM_LOAD16_WORD_SWAP( "mmd0x0.u124",  0x000000, 0x020000, CRC(e1f6159e) SHA1(e4af85036756482d6fa27494842699e2647809c7) )

	ROM_REGION( 0x800000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD16_BYTE( "mm200-e.bin",  0x000000, 0x100000, CRC(eb104408) SHA1(a7805597161dc5acd2804d607dd0acac0c40111d) )
	ROM_LOAD16_BYTE( "mm200-o.bin",  0x000001, 0x100000, CRC(b6d04e7c) SHA1(1fa9d6b967724ed0c9e6ae3eda7089a081120d54) )
	ROM_LOAD16_BYTE( "mm201-e.bin",  0x200000, 0x100000, CRC(b8c64e14) SHA1(8e2b19f0ba715dfdf0d423a41989e715145adbeb) )
	ROM_LOAD16_BYTE( "mm201-o.bin",  0x200001, 0x100000, CRC(3ecd6c0a) SHA1(cb48564e2bd3014eeaad9cfa589bdef3f828c282) )
	ROM_LOAD16_BYTE( "mm202-e.bin",  0x400000, 0x100000, CRC(f0fd5688) SHA1(a3f5edfef253c81b27434519b0b9527f6c9a6e82) )
	ROM_LOAD16_BYTE( "mm202-o.bin",  0x400001, 0x100000, CRC(e0fe1b2b) SHA1(e66bd09eed6dfea524d8610a3c7e1792a1ff6286) )
	ROM_LOAD16_BYTE( "mm203-e.bin",  0x600000, 0x100000, CRC(b9001f28) SHA1(b112c17b960a535a543565ca2e22734c7c510f18) )
	ROM_LOAD16_BYTE( "mm203-o.bin",  0x600001, 0x100000, CRC(2ed6227d) SHA1(d9abbb739ef15437194c90cd01d5d82dbd4b7859) )

	ROM_REGION( 0x200000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD16_BYTE( "mm300-e.u53",  0x000000, 0x100000, CRC(f9ee708d) SHA1(4c11a9574ea815a87d7e4af04db4368b14bf7530) )
	ROM_LOAD16_BYTE( "mm300-o.u54",  0x000001, 0x100000, CRC(76299353) SHA1(01997905ba019d770ac1998633f4ebf6f91a3945) )

	ROM_REGION( 0x200000, "view2_1", 0 )   /* Tiles */
	ROM_COPY("view2_0",0x000000,0,0x200000) // it isn't on the board twice.

	ROM_REGION( 0x100000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "mm-100-401-e0.bin",  0x000000, 0x100000, CRC(b9cbfbee) SHA1(051d48a68477ef9c29bd5cc0bb7955d513a0ab94) )  // 16 x $10000

	ROM_REGION( 0x100000, "oki2", ROMREGION_ERASE00 )   /* Samples */
	/* Not present on this board */
ROM_END


/* The evolution and USA versions seem to be more like GTMR 1.5, they have some fairly significant changes */

/*  This version displays:

    tb05mm-eu "1000 miglia"
    master up= 94/09/06 14:49:19            */

ROM_START( gtmre )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "gmmu2.bin", 0x000000, 0x080000, CRC(36dc4aa9) SHA1(0aea4dc169d7aad2ea957a1de698d1fa12c71556) )
	ROM_LOAD16_BYTE( "gmmu1.bin", 0x000001, 0x080000, CRC(8653c144) SHA1(a253a01327a9443337a55a13c063ea5096444c4c) )

	ROM_REGION( 0x020000, "mcudata", 0 )            /* MCU Code */
	//ROM_LOAD16_WORD_SWAP( "mcu_code.u12",  0x000000, 0x020000, NO_DUMP )
	// this rom has the right version string, so is probably correct
	ROM_LOAD16_WORD_SWAP( "gtmrusa.u12",  0x000000, 0x020000, CRC(2e1a06ff) SHA1(475a7555653eefac84307492a385895b839cab0d) )

	ROM_REGION( 0x800000, "kan_spr", 0 )   /* Sprites */
	/* fill the 0x700000-7fffff range first, with the second of the identical halves */
	ROM_LOAD16_BYTE( "gmmu64.bin",  0x600000, 0x100000, CRC(57d77b33) SHA1(f7ae28ae889be4442b7b236705943eaad1f0c84e) )  // HALVES IDENTICAL
	ROM_LOAD16_BYTE( "gmmu65.bin",  0x600001, 0x100000, CRC(05b8bdca) SHA1(44471d66787d5b48ae8b13676f42f27af44e5c6a) )  // HALVES IDENTICAL
	ROM_LOAD( "gmmu27.bin",  0x000000, 0x200000, CRC(c0ab3efc) SHA1(e6cd15480977b036234d91e6f3a6e21b7f0a3c3e) )
	ROM_LOAD( "gmmu28.bin",  0x200000, 0x200000, CRC(cf6b23dc) SHA1(ccfd0b17507e091e55c169361cd6a6b19641b717) )
	ROM_LOAD( "gmmu29.bin",  0x400000, 0x200000, CRC(8f27f5d3) SHA1(219a86446ce2556682009d8aff837480f040a01e) )
	ROM_LOAD( "gmmu30.bin",  0x600000, 0x080000, CRC(e9747c8c) SHA1(2507102ec34755c6f110eadb3444e6d3a3474051) )
	/* codes 6800-6fff are explicitly skipped */

	ROM_REGION( 0x200000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "gmmu52.bin",  0x000000, 0x200000, CRC(b15f6b7f) SHA1(5e84919d788add53fc87f4d85f437df413b1dbc5) )

	ROM_REGION( 0x200000, "view2_1", 0 )   /* Tiles */
	ROM_COPY("view2_0",0x000000,0,0x200000) // it isn't on the board twice.

	ROM_REGION( 0x100000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "gmmu23.bin",  0x000000, 0x100000, CRC(b9cbfbee) SHA1(051d48a68477ef9c29bd5cc0bb7955d513a0ab94) ) // 16 x $10000

	ROM_REGION( 0x100000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "gmmu24.bin",  0x000000, 0x100000, CRC(380cdc7c) SHA1(ba7f51201b0f2bf15e66557e45bb2af5cf797779) ) //  2 x $40000 - HALVES IDENTICAL
ROM_END


/*  This version displays:

    tb05mm-eu "1000 miglia"
    master up= 94/09/06 20:30:39            */

ROM_START( gtmrusa )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "mmp0x3.u2", 0x000000, 0x080000, CRC(5be615c4) SHA1(c14d11a5bf6e025a65b932039165302ff407c4e1) )
	ROM_LOAD16_BYTE( "mmp1x3.u1", 0x000001, 0x080000, CRC(ae853e4e) SHA1(31eaa73b0c5ddab1292f521ceec43b202653efe9) )

	ROM_REGION( 0x020000, "mcudata", 0 )            /* MCU Code? */
	ROM_LOAD16_WORD_SWAP( "mmd0x3.u12",  0x000000, 0x020000, CRC(2e1a06ff) SHA1(475a7555653eefac84307492a385895b839cab0d) )

	ROM_REGION( 0x800000, "kan_spr", 0 )   /* Sprites */
	/* fill the 0x700000-7fffff range first, with the second of the identical halves */
	ROM_LOAD16_BYTE( "mm-204-564.bin",  0x600000, 0x100000, CRC(57d77b33) SHA1(f7ae28ae889be4442b7b236705943eaad1f0c84e) )  // HALVES IDENTICAL
	ROM_LOAD16_BYTE( "mm-204-406-565.bin",  0x600001, 0x100000, CRC(05b8bdca) SHA1(44471d66787d5b48ae8b13676f42f27af44e5c6a) )  // HALVES IDENTICAL
	ROM_LOAD( "mm-200-402-s0.bin",  0x000000, 0x200000, CRC(c0ab3efc) SHA1(e6cd15480977b036234d91e6f3a6e21b7f0a3c3e) )
	ROM_LOAD( "mm-201-403-s1.bin",  0x200000, 0x200000, CRC(cf6b23dc) SHA1(ccfd0b17507e091e55c169361cd6a6b19641b717) )
	ROM_LOAD( "mm-202-404-s2.bin",  0x400000, 0x200000, CRC(8f27f5d3) SHA1(219a86446ce2556682009d8aff837480f040a01e) )
	ROM_LOAD( "mm-203-405-s3.bin",  0x600000, 0x080000, CRC(e9747c8c) SHA1(2507102ec34755c6f110eadb3444e6d3a3474051) )
	/* codes 6800-6fff are explicitly skipped */

	ROM_REGION( 0x200000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "mm-300-406-a0.bin",  0x000000, 0x200000, CRC(b15f6b7f) SHA1(5e84919d788add53fc87f4d85f437df413b1dbc5) )

	ROM_REGION( 0x200000, "view2_1", 0 )   /* Tiles */
	ROM_COPY("view2_0",0x000000,0,0x200000) // it isn't on the board twice.

	ROM_REGION( 0x100000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "mm-100-401-a0.bin",  0x000000, 0x100000, CRC(b9cbfbee) SHA1(051d48a68477ef9c29bd5cc0bb7955d513a0ab94) ) // 16 x $10000

	ROM_REGION( 0x100000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "mm-101-402-e24.bin",  0x000000, 0x100000, CRC(380cdc7c) SHA1(ba7f51201b0f2bf15e66557e45bb2af5cf797779) ) //  2 x $40000 - HALVES IDENTICAL
ROM_END


/***************************************************************************

Great 1000 Miles Rally 2 USA
Kaneko, 1995

PCB Layouts
-----------

KANEKO AX-SYSTEM BOARD
M201F00584 KANEKO (sticker)
|------------------------------------------------------|
|    PX4460  CNC1                                      |
|LA4461     M6295            6264  6264                |
|    LPF6K  M6295     PAL1           |----------|      |
|                     PAL2           |KANEKO    |      |
|                     PAL3           |VIEW2-CHIP|   CN3|
|                                    |(QFP144)  |      |
|           CNC2                     |          |      |
|                                    |----------|      |
|J                                   6264              |
|A                                   6264              |
|M               62256                                 |
|M               62256               |----------|      |
|A                                   |KANEKO    |      |
|                                    |VIEW2-CHIP|      |
|   MC1091                           |(QFP144)  |   CN2|
|                                    |          |      |
|       CNA                          |----------|      |
|                               6264                   |
|    |----|                     6264 |----------|      |
|    | *1 |      62256               |KANEKO    |      |
|    |----|      62256               |KC-002    |      |
|             |---------------|      |(QFP208)  |      |
|             |     68000     |      |          |      |
|             |               |      |----------|      |
|      PAL4   |---------------|33.3333MHz       424260 |
|      PAL5              PAL6  20MHz            424260 |
|                        PAL7  PAL8                    |
|62256                PAL9                             |
|62256                PAL10                            |
|                     PAL11                            |
|                     27MHz  16MHz                     |
|            CNB                            CN1        |
|------------------------------------------------------|
Notes:
      68000 - Toshiba TMP68HC000N-16 CPU running at 16.000MHz (SDIP64)
      M6295 - Oki M6295 ADPCM Sample Player IC, clock 2.000MHz (both), sample rate = 2000000/165 (QFP44)
      LA4461- Sanyo LA4461 12W Power Amplifier (SIP10, same as LA4460 but with reverse pinout)
      LPF6K - KANEKO Custom Ceramic AD/DA Module (SIP7)
      PX4460- KANEKO Custom Ceramic AD/DA Module (SIP5)
      MC1091- KANEKO Custom Ceramic Input Module (ZIP46)
      6264  - Sony CXK5864CM-10LL 8k x8 SRAM (SOP28)
      62256 - Sony CXK58257ASP-70L 32k x8 SRAM (DIP28)
      424260- NEC 424260-80 256k x16 DRAM (SOJ40)
      PAL1  - AMI 18CV8PC-25 stamped 'AX0P048' (DIP20)
      PAL2  - AMI 18CV8PC-25 stamped 'AX0P049' (DIP20)
      PAL3  - AMI 18CV8PC-25 stamped 'AX0P050' (DIP20)
      PAL4  - AMI 18CV8PC-25 stamped 'AX0P021' (DIP20)
      PAL5  - AMI 18CV8PC-25 stamped 'AX0P022' (DIP20)
      PAL6  - AMI 18CV8PC-25 stamped 'AX0P070' (DIP20)
      PAL7  - AMI 18CV8PC-25 stamped 'AX0P071' (DIP20)
      PAL8  - AMI 18CV8PC-25 stamped 'AX0P089' (DIP20)
      PAL9  - AMI 18CV8PC-25 stamped 'AX0P062' (DIP20)
      PAL10 - AMI 18CV8PC-25 stamped 'AX0P063' (DIP20)
      PAL11 - AMI 18CV8PC-25 stamped 'AX0P064' (DIP20)
      CNC1/CNC2 \
      CNA/CNB   | Multi-pin connectors joining top board to bottom board
      CN1/2/3   /
      Custom IC's -
                   KANEKO VIEW2-CHIP (x2, QFP144)
                   KANEKO KC-002 (QFP208)
                   *1 - KANEKO JAPAN 9448 TA (QFP44)


KANEKO AX-SYSTEM BOARD ROM-08
AX09S00584 KANEKO (sticker)
|------------------------------------------------------|
| 93C46  SW1      CNB                        CN1       |
|     |-------| EPROM.U31   SP_ROM0.U49                |
|     |KANEKO |                                        |
|CN4  |TBS0P02| EPROM.U32   SP_ROM1.U50    PAL8        |
|     |(QFP74)|                                        |
|     |-------| EPROM.U33   SP_ROM2.U51    SP-ROM4.U85 |
|                                                      |
|       PAL1                SP_ROM3.U52    SP-ROM5.U86 |
|       PAL2                                           |
|PAL3                                      SP-ROM6.U87 |
|PAL4                                                  |
|                                          SP-ROM7.U88 |
|EPROM.U5                                              |
|                         6116                         |
|EPROM.U6                                  BG0_ROM0.U89|
|         CNA             6116                         |
|EPROM.U7                                  BG0_ROM1.U90|
|                                                      |
|EPROM.U8                                  BG1_ROM0.U93|
|                                                      |
|      PAL5                                BG1_ROM1.U94|
|      PAL6               6116             PAL9        |
|UPC339C                                            CN2|
|UPC339C                  6116             BG2_ROM0.U91|
|                                                      |
|CNN3            CNC2                      BG2_ROM1.U92|
|                                                      |
|CNN2            S2_ROM0.U47  S2_ROM1.U65              |
|                                          BG2_ROM0.U96|
|CNN1            S1_ROM0.U48  S1_ROM1.U66           CN3|
|   NE555                                  BG2_ROM1.U97|
|AD7820          CNC1         PAL7                     |
|------------------------------------------------------|
Notes:
      AD7820  - Analog Devices AD7820 Linear Compatible CMOS High Speed 8-Bit ADC with Track/Hold Function (DIP20)
      93C46   - Atmel 93C46 Serial EEPROM (DIP8)
      6116    - 2k x8 SRAM (DIP24)
      NE555   - Texas Instruments NE555 General Purpose Single Bipolar Timer (DIP8)
      UPC339C - NEC uPC339C Low Power Quad Comparator (DIP14)
      PAL1    - Lattice GAL16V8B stamped 'COMUX2'   (DIP20)
      PAL2    - Lattice GAL16V8B stamped 'COMUX3'   (DIP20)
      PAL3    - AMI 18CV8PC-25 stamped 'COMUX1'     (DIP20)
      PAL4    - AMI 18CV8PC-25 stamped 'MMS4P004'   (DIP20)
      PAL5    - Lattice GAL16V8B stamped 'COMUX5'   (DIP20)
      PAL6    - Lattice GAL16V8B stamped 'COMUX4'   (DIP20)
      PAL7    - AMI 18CV8PC-25 stamped 'MMS4P067'   (DIP20)
      PAL8    - Lattice GAL16V8B stamped 'MMS4G084' (DIP20)
      PAL9    - Lattice GAL16V8B stamped 'MMS4G095' (DIP20)
      CNN1/2/3- Connectors for analog controls
      CN4     - 16 Pin Flat Cable Connector
      CNC1/CNC2 \
      CNA/CNB   | Multi-pin connectors joining top board to bottom board
      CN1/2/3   /
      Custom IC - KANEKO TBS0P02 Custom Microcontroller (QFP74)

      ROMs - (Note: Not all ROM locations are populated on the above layout)

            Filename       Device Type           Use
            -----------------------------------------------------
            M2P1A1.U7      27C040 (4M)           \  68000 Program
            M2P0A1.U8      27C040 (4M)           /

            M2D0X0.U31     27C010 (1M)              MCU Program

            M2S0A1.U32     27C040 (4M)           \
            M2S1A1.U33     27C040 (4M)           |
            M2-200-00.U49  32M MaskROM (42-Pin)  |  Sprites
            M2-201-00.U50  16M MaskROM (42-Pin)  |
            M2-202-00.U51  16M MaskROM (42-Pin)  /

            M2-300-00.U89  16M MaskROM (42-Pin)  \
            M2-301-00.U90  16M MaskROM (42-Pin)  |  Tiles
            M2B0X0.U93     27C010 (1M)           |
            M2B1X0.U94     27C010 (1M)           /

            M2-100-00.U48  8M mask ROM (32-Pin)  \
            M2W1A1.U47     27C040 (4M)           /  Oki Samples

***************************************************************************/

ROM_START( gtmr2 )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "m2p0x1a.u8",  0x000000, 0x080000, CRC(c29039fb) SHA1(a16e8863608353c2931e9d45359fbcec8f11ef9d) )
	ROM_LOAD16_BYTE( "m2p1x1a.u7",  0x000001, 0x080000, CRC(8ef392c4) SHA1(06bd720d931911e32264183dd215ab70ad6d2961) )

	ROM_REGION( 0x020000, "mcudata", 0 )            /* MCU Code? */
	ROM_LOAD16_WORD_SWAP( "m2d0x0.u31",        0x000000, 0x020000, CRC(2e1a06ff) SHA1(475a7555653eefac84307492a385895b839cab0d) )

	ROM_REGION( 0x800000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "m2-200-0.u49",       0x000000, 0x400000, CRC(93aafc53) SHA1(1d28b6e3bd61ce9c938fc5303aeabcdefa549852) )
	ROM_LOAD( "m2-201-0.u50",       0x400000, 0x200000, CRC(39b60a83) SHA1(aa7b37c7c92bbcf685f4fec84cc6d8a77d26433c) )
	ROM_LOAD( "m2-202-0.u51",       0x600000, 0x200000, CRC(fd06b339) SHA1(5de0af7d23147f6eb403700eabd66794198f3641) )
	ROM_LOAD16_BYTE( "m2s0x1a.u32", 0x700000, 0x080000, CRC(a485eec6) SHA1(f8aff62daed95a63544106472a9ef34902feaaa2) )
	ROM_LOAD16_BYTE( "m2s1x1a.u33", 0x700001, 0x080000, CRC(c5b71bb2) SHA1(874e2a2e19cd8f916afa6fcf54169a8db035fe64) )

	ROM_REGION( 0x440000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "m2-300-0.u89",      0x000000, 0x200000, CRC(4dc42fbb) SHA1(f14c287bc60f561eb9a57db4e3390aae9a81c392) )
	ROM_LOAD( "m2-301-0.u90",      0x200000, 0x200000, CRC(f4e894f2) SHA1(1f983a1d93845fe298afba60d4dacdd1a10cab7f) )
	ROM_LOAD16_BYTE( "m2b0x0.u93", 0x400000, 0x020000, CRC(e023d51b) SHA1(3c9f591f3ca2ee8e1100b83ae8eb593e11e6eac7) )
	ROM_LOAD16_BYTE( "m2b1x0.u94", 0x400001, 0x020000, CRC(03c48bdb) SHA1(f5ba45d026530d46f760cf06d02a1ffcca89aa3c) )

	ROM_REGION( 0x440000, "view2_1", 0 )   /* Tiles */
	ROM_COPY("view2_0",0x000000,0,0x440000) // it isn't on the board twice.

	ROM_REGION( 0x100000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "m2-100-0.u48",      0x000000, 0x100000, CRC(5250fa45) SHA1(b1ad4660906997faea0aa89866de01a0e9f2b61d) )

	ROM_REGION( 0x080000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "m2w1x0.u47",        0x040000, 0x040000, CRC(1b0513c5) SHA1(8c9ddef19297e1b39d900297005203b7ff28667e) )
ROM_END

ROM_START( gtmr2a )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "m2p0x1.u8",  0x000000, 0x080000, CRC(525f6618) SHA1(da8008cc7768b4e8c0091aa3ea21752d0ca33691) )
	ROM_LOAD16_BYTE( "m2p1x1.u7",  0x000001, 0x080000, CRC(914683e5) SHA1(dbb2140f7de86073647abc6e73ba739ea201dd30) )

	ROM_REGION( 0x020000, "mcudata", 0 )            /* MCU Code? */
	ROM_LOAD16_WORD_SWAP( "m2d0x0.u31",        0x000000, 0x020000, CRC(2e1a06ff) SHA1(475a7555653eefac84307492a385895b839cab0d) )

	ROM_REGION( 0x800000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "m2-200-0.u49",      0x000000, 0x400000, CRC(93aafc53) SHA1(1d28b6e3bd61ce9c938fc5303aeabcdefa549852) )
	ROM_LOAD( "m2-201-0.u50",      0x400000, 0x200000, CRC(39b60a83) SHA1(aa7b37c7c92bbcf685f4fec84cc6d8a77d26433c) )
	ROM_LOAD( "m2-202-0.u51",      0x600000, 0x200000, CRC(fd06b339) SHA1(5de0af7d23147f6eb403700eabd66794198f3641) )
	ROM_LOAD16_BYTE( "m2s0x1.u32", 0x700000, 0x080000, CRC(4069d6c7) SHA1(2ed1cbb7ebde8347e0359cd56ee3a0d4d42d551f) )
	ROM_LOAD16_BYTE( "m2s1x1.u33", 0x700001, 0x080000, CRC(c53fe269) SHA1(e6c485bbaea4b67f074b89e047f686f107805713) )

	ROM_REGION( 0x440000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "m2-300-0.u89",      0x000000, 0x200000, CRC(4dc42fbb) SHA1(f14c287bc60f561eb9a57db4e3390aae9a81c392) )
	ROM_LOAD( "m2-301-0.u90",      0x200000, 0x200000, CRC(f4e894f2) SHA1(1f983a1d93845fe298afba60d4dacdd1a10cab7f) )
	ROM_LOAD16_BYTE( "m2b0x0.u93", 0x400000, 0x020000, CRC(e023d51b) SHA1(3c9f591f3ca2ee8e1100b83ae8eb593e11e6eac7) )
	ROM_LOAD16_BYTE( "m2b1x0.u94", 0x400001, 0x020000, CRC(03c48bdb) SHA1(f5ba45d026530d46f760cf06d02a1ffcca89aa3c) )

	ROM_REGION( 0x440000, "view2_1", 0 )   /* Tiles */
	ROM_COPY("view2_0",0x000000,0,0x440000) // it isn't on the board twice.

	ROM_REGION( 0x100000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "m2-100-0.u48",      0x000000, 0x100000, CRC(5250fa45) SHA1(b1ad4660906997faea0aa89866de01a0e9f2b61d) )

	ROM_REGION( 0x080000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "m2w1x0.u47",        0x040000, 0x040000, CRC(1b0513c5) SHA1(8c9ddef19297e1b39d900297005203b7ff28667e) )
ROM_END

ROM_START( gtmr2u )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "m2p0a2.u8",  0x000000, 0x080000, CRC(1a008ea4) SHA1(970ae4787117a388223e19d72eb920ea2264ce7f) )
	ROM_LOAD16_BYTE( "m2p1a2.u7",  0x000001, 0x080000, CRC(bbf8b43e) SHA1(03a01153e46d0003efa798b211e786ca15321202) )

	ROM_REGION( 0x020000, "mcudata", 0 )            /* MCU Code? */
	ROM_LOAD16_WORD_SWAP( "m2d0x0.u31",        0x000000, 0x020000, CRC(2e1a06ff) SHA1(475a7555653eefac84307492a385895b839cab0d) )

	ROM_REGION( 0x800000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "m2-200-0.u49",      0x000000, 0x400000, CRC(93aafc53) SHA1(1d28b6e3bd61ce9c938fc5303aeabcdefa549852) )
	ROM_LOAD( "m2-201-0.u50",      0x400000, 0x200000, CRC(39b60a83) SHA1(aa7b37c7c92bbcf685f4fec84cc6d8a77d26433c) )
	ROM_LOAD( "m2-202-0.u51",      0x600000, 0x200000, CRC(fd06b339) SHA1(5de0af7d23147f6eb403700eabd66794198f3641) )
	ROM_LOAD16_BYTE( "m2s0a1.u32", 0x700000, 0x080000, CRC(98977171) SHA1(5b69462e07778b5bd1f5119cae6b63ede38cd642) )
	ROM_LOAD16_BYTE( "m2s1a1.u33", 0x700001, 0x080000, CRC(c69a732e) SHA1(810b333f442c0714f4cb8b4a73136d0b44443277) )

	ROM_REGION( 0x440000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "m2-300-0.u89",      0x000000, 0x200000, CRC(4dc42fbb) SHA1(f14c287bc60f561eb9a57db4e3390aae9a81c392) )
	ROM_LOAD( "m2-301-0.u90",      0x200000, 0x200000, CRC(f4e894f2) SHA1(1f983a1d93845fe298afba60d4dacdd1a10cab7f) )
	ROM_LOAD16_BYTE( "m2b0x0.u93", 0x400000, 0x020000, CRC(e023d51b) SHA1(3c9f591f3ca2ee8e1100b83ae8eb593e11e6eac7) )
	ROM_LOAD16_BYTE( "m2b1x0.u94", 0x400001, 0x020000, CRC(03c48bdb) SHA1(f5ba45d026530d46f760cf06d02a1ffcca89aa3c) )

	ROM_REGION( 0x440000, "view2_1", 0 )   /* Tiles */
	ROM_COPY("view2_0",0x000000,0,0x440000) // it isn't on the board twice.

	ROM_REGION( 0x100000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "m2-100-0.u48",      0x000000, 0x100000, CRC(5250fa45) SHA1(b1ad4660906997faea0aa89866de01a0e9f2b61d) )

	ROM_REGION( 0x080000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "m2w1a1.u47",        0x000000, 0x080000, CRC(15f25342) SHA1(9947e66575738700345c12c104701b812c62ce03) )
ROM_END

ROM_START( gtmr2ua )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "m2p0a1.u8",  0x000000, 0x080000, CRC(813e1d5e) SHA1(602df02933dc7b77be311113af1d1edad2751cc9) )
	ROM_LOAD16_BYTE( "m2p1a1.u7",  0x000001, 0x080000, CRC(bee63666) SHA1(07585a63f901f50f2a2314eb4dc4307e7028ded7) )

	ROM_REGION( 0x020000, "mcudata", 0 )            /* MCU Code? */
	ROM_LOAD16_WORD_SWAP( "m2d0x0.u31",        0x000000, 0x020000, CRC(2e1a06ff) SHA1(475a7555653eefac84307492a385895b839cab0d) )

	ROM_REGION( 0x800000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "m2-200-0.u49",      0x000000, 0x400000, CRC(93aafc53) SHA1(1d28b6e3bd61ce9c938fc5303aeabcdefa549852) )
	ROM_LOAD( "m2-201-0.u50",      0x400000, 0x200000, CRC(39b60a83) SHA1(aa7b37c7c92bbcf685f4fec84cc6d8a77d26433c) )
	ROM_LOAD( "m2-202-0.u51",      0x600000, 0x200000, CRC(fd06b339) SHA1(5de0af7d23147f6eb403700eabd66794198f3641) )
	ROM_LOAD16_BYTE( "m2s0a1.u32", 0x700000, 0x080000, CRC(98977171) SHA1(5b69462e07778b5bd1f5119cae6b63ede38cd642) )
	ROM_LOAD16_BYTE( "m2s1a1.u33", 0x700001, 0x080000, CRC(c69a732e) SHA1(810b333f442c0714f4cb8b4a73136d0b44443277) )

	ROM_REGION( 0x440000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "m2-300-0.u89",      0x000000, 0x200000, CRC(4dc42fbb) SHA1(f14c287bc60f561eb9a57db4e3390aae9a81c392) )
	ROM_LOAD( "m2-301-0.u90",      0x200000, 0x200000, CRC(f4e894f2) SHA1(1f983a1d93845fe298afba60d4dacdd1a10cab7f) )
	ROM_LOAD16_BYTE( "m2b0x0.u93", 0x400000, 0x020000, CRC(e023d51b) SHA1(3c9f591f3ca2ee8e1100b83ae8eb593e11e6eac7) )
	ROM_LOAD16_BYTE( "m2b1x0.u94", 0x400001, 0x020000, CRC(03c48bdb) SHA1(f5ba45d026530d46f760cf06d02a1ffcca89aa3c) )

	ROM_REGION( 0x440000, "view2_1", 0 )   /* Tiles */
	ROM_COPY("view2_0",0x000000,0,0x440000) // it isn't on the board twice.

	ROM_REGION( 0x100000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "m2-100-0.u48",      0x000000, 0x100000, CRC(5250fa45) SHA1(b1ad4660906997faea0aa89866de01a0e9f2b61d) )

	ROM_REGION( 0x080000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "m2w1a1.u47",        0x000000, 0x080000, CRC(15f25342) SHA1(9947e66575738700345c12c104701b812c62ce03) )
ROM_END

/***************************************************************************

                                Magical Crystals

(c)1991 Kaneko/Atlus
Z00FC-02

CPU  : TMP68HC000N-12
Sound: YM2149Fx2 M6295
OSC  : 16.0000MHz(X1) 12.0000MHz(X2)

ROMs:
mc100j.u18 - Main programs
mc101j.u19 /

mc000.u38  - Graphics (32pin mask)
mc001.u37  | (32pin mask)
mc002j.u36 / (27c010)

mc010.u04 - Graphics (42pin mask)

mc020.u33 - Graphics (42pin mask)

mc030.u32 - Samples (32pin mask)

PALs (18CV8PC):
u08, u20, u41, u42, u50, u51, u54

Custom chips:
KANEKO VU-001 046A (u53, 48pin PQFP)
KANEKO VU-002 052 151021 (u60, 160pin PQFP)
KANEKO 23160-509 9047EAI VIEW2-CHIP (u24 & u34, 144pin PQFP)
KANEKO MUX2-CHIP (u28, 64pin PQFP)
KANEKO IU-001 9045KP002 (u22, 44pin PQFP)
KANEKO I/O JAMMA MC-8282 047 (u5, 46pin)
699206p (u09, 44pin PQFP)
699205p (u10, 44pin PQFP)

Other:
93C46 EEPROM

DIP settings:
1: Flip screen
2: Test mode
3: Unused
4: Unused

Yes, one program rom actually is a 27C010 and the other one is a 27C020

***************************************************************************/

ROM_START( mgcrystl ) /* Master Up: 92/01/10 14:21:30 */
	ROM_REGION( 0x040000*2, "maincpu", ROMREGION_ERASE )            /* 68000 Code */
	ROM_LOAD16_BYTE( "mc100e02.u18", 0x000000, 0x020000, CRC(246a1335) SHA1(8333945a92e08a7bff425d2d6602557386016dc5) ) /* Labeled as MC100E/U18-02 */
	ROM_LOAD16_BYTE( "mc101e02.u19", 0x000001, 0x040000, CRC(708ea1dc) SHA1(ae6eca6620729bc1e815f1bfbd8fe130f0ba943c) ) /* Labeled as MC101E/U19-02 */

	ROM_REGION( 0x280000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "mc000.u38",    0x000000, 0x100000, CRC(28acf6f4) SHA1(6647ad90ea580b65ed28772f9d65352b06833d0c) )
	ROM_LOAD( "mc001.u37",    0x100000, 0x080000, CRC(005bc43d) SHA1(6f6cd99e8e60562fa86581008455a6d9d646fa95) )
	ROM_RELOAD(               0x180000, 0x080000             )
	ROM_LOAD( "mc002e02.u36", 0x200000, 0x020000, CRC(27ac1056) SHA1(34b07c1a0d403ca45c9849d3d8d311012f787df6) ) /* Labeled as MC002E/U36-02 */
	ROM_RELOAD(               0x220000, 0x020000             )
	ROM_RELOAD(               0x240000, 0x020000             )
	ROM_RELOAD(               0x260000, 0x020000             )

	ROM_REGION( 0x100000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "mc010.u04",  0x000000, 0x100000, CRC(85072772) SHA1(25e903cc2c893d61db791d1fe60a1205a4395667) )

	ROM_REGION( 0x100000, "view2_1", 0 )   /* Tiles */
	ROM_LOAD( "mc020.u34",  0x000000, 0x100000, CRC(1ea92ff1) SHA1(66ec53e664b2a5a751a280a538aaeceafc187ceb) )

	ROM_REGION( 0x040000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "mc030.u32",  0x000000, 0x040000, CRC(c165962e) SHA1(f7e130db387ae9dcb7223f7ad6e51270d3033bc9) )
ROM_END

ROM_START( mgcrystlo ) /* Master Up: 91/12/10 01:56:06 */
	ROM_REGION( 0x040000*2, "maincpu", ROMREGION_ERASE )            /* 68000 Code */
	ROM_LOAD16_BYTE( "mc100h00.u18", 0x000000, 0x020000, CRC(c7456ba7) SHA1(96c25c3432069373fa86d7af3e093e02e39aea34) ) /* Labeled as MC100H/U18-00 */
	ROM_LOAD16_BYTE( "mc101h00.u19", 0x000001, 0x040000, CRC(ea8f9300) SHA1(0cd0d448805aa45986b63befca00b08fe066dbb2) ) /* Labeled as MC101H/U19-00 */

	ROM_REGION( 0x280000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "mc000.u38",    0x000000, 0x100000, CRC(28acf6f4) SHA1(6647ad90ea580b65ed28772f9d65352b06833d0c) )
	ROM_LOAD( "mc001.u37",    0x100000, 0x080000, CRC(005bc43d) SHA1(6f6cd99e8e60562fa86581008455a6d9d646fa95) )
	ROM_RELOAD(               0x180000, 0x080000             )
	ROM_LOAD( "mc002h00.u36", 0x200000, 0x020000, CRC(22729037) SHA1(de4e1bdab57aa617411b6327f3db4856970e8953) ) /* Labeled as MC002H/U36-00 */
	ROM_RELOAD(               0x220000, 0x020000             )
	ROM_RELOAD(               0x240000, 0x020000             )
	ROM_RELOAD(               0x260000, 0x020000             )

	ROM_REGION( 0x100000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "mc010.u04",  0x000000, 0x100000, CRC(85072772) SHA1(25e903cc2c893d61db791d1fe60a1205a4395667) )

	ROM_REGION( 0x100000, "view2_1", 0 )   /* Tiles */
	ROM_LOAD( "mc020.u34",  0x000000, 0x100000, CRC(1ea92ff1) SHA1(66ec53e664b2a5a751a280a538aaeceafc187ceb) )

	ROM_REGION( 0x040000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "mc030.u32",  0x000000, 0x040000, CRC(c165962e) SHA1(f7e130db387ae9dcb7223f7ad6e51270d3033bc9) )
ROM_END

ROM_START( mgcrystlj ) /* Master Up: 92/01/13 14:44:20 */
	ROM_REGION( 0x040000*2, "maincpu", ROMREGION_ERASE )            /* 68000 Code */
	ROM_LOAD16_BYTE( "kaneko__mc100-u18j-00.u18", 0x000000, 0x020000, CRC(afe5882d) SHA1(176e6e12e3df63c08d7aff781f5e5a9bd83ec293) ) /* Labeled as MC100J/U18-00 */
	ROM_LOAD16_BYTE( "kaneko__mc101-u19j-00.u19", 0x000001, 0x040000, CRC(60da5492) SHA1(82b90a617d355825624ce9fb30bddf4714bd0d18) ) /* Labeled as MC101J/U19-00 */

	ROM_REGION( 0x280000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "kaneko__mc-000_0001.u38",  0x000000, 0x100000, CRC(28acf6f4) SHA1(6647ad90ea580b65ed28772f9d65352b06833d0c) ) // Mask rom
	ROM_LOAD( "kaneko__mc-001_0002_r44.u37",  0x100000, 0x080000, CRC(005bc43d) SHA1(6f6cd99e8e60562fa86581008455a6d9d646fa95) ) // Mask rom
	ROM_RELOAD(             0x180000, 0x080000             )
	ROM_LOAD( "kaneko__mc002j-u36-02.u36", 0x200000, 0x020000, CRC(27ac1056) SHA1(34b07c1a0d403ca45c9849d3d8d311012f787df6) ) /* Labeled as MC002J/U36-02, but same as MC002E/U36-02 */
	ROM_RELOAD(             0x220000, 0x020000             )
	ROM_RELOAD(             0x240000, 0x020000             )
	ROM_RELOAD(             0x260000, 0x020000             )

	ROM_REGION( 0x100000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "kaneko__mc-010_0003.u04",  0x000000, 0x100000, CRC(85072772) SHA1(25e903cc2c893d61db791d1fe60a1205a4395667) ) // Mask rom

	ROM_REGION( 0x100000, "view2_1", 0 )   /* Tiles */
	ROM_LOAD( "kaneko__mc-020_0004.u34",  0x000000, 0x100000, CRC(1ea92ff1) SHA1(66ec53e664b2a5a751a280a538aaeceafc187ceb) ) // Mask rom

	ROM_REGION( 0x040000, "oki1", 0 )    /* Samples */
	ROM_LOAD( "kaneko__mc-030_0005_t99.u32",  0x000000, 0x040000, CRC(c165962e) SHA1(f7e130db387ae9dcb7223f7ad6e51270d3033bc9) )
ROM_END



/***************************************************************************

Shogun Warriors (Europe Rev.xx)(Kaneko 1992)

Dumped from an original Kaneko PCB. Board No. Z01DK-002 / Serial FB92E01342. + Top board "Z05DP FOR Z01DK"

Two Program roms are equal to the Japanese version. See Below:

fb000e.u43  = fb001e.u43  Shogun Warriors               <Note: Different label compared to Mame.
fb001e.u101 = fb000e.u42  Shogun Warriors               <Note: Different label/position compared to Mame.
fb030e.u61  = fb030j.u61  Fujiyama Buster(Japan)        <Note: Same as the Japanese version in Mame.
fb031e.u62 NO MATCH
fb040e.u33  = fb040j.u33  Fujiyama Buster (Japan)       <Note: Same as the Japanese version in Mame.


The rom positions are equal to the Japanese version in Mame (Same Hardware revision).
The "e" suffix on the roms of "Shogun Warriors" that is previously
in Mame indicates that it is an European Version too, so this must be
an earlier or later European revision.

Some hardware(custom chip) differences compared with the info in the Mame source:

Fujiyama Buster(Japan)                      Shogun Warriors (Europe) (My Dump)

KANEKO JAPAN 9152EV 175101 (160 Pin PQFP) = KANEKO JAPAN 9202EV 175071 (160 Pin PQFP)
KANEKO JAPAN 9203 T (44 PIN PQFP)         = KANEKO JAPAN 9204 T (44 PIN PQFP)

***************************************************************************/

ROM_START( shogwarr )
	ROM_REGION( 0x040000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "fb030e.u61", 0x000000, 0x020000, CRC(32ce7909) SHA1(02d87342706ac9547eb611bd542f8498ba41e34a) )
	ROM_LOAD16_BYTE( "fb031e.u62", 0x000001, 0x020000, CRC(228aeaf5) SHA1(5e080d7975bc5dcf6fccfbc286eafe939496d9bf) )

	ROM_REGION( 0x020000, "calc3_rom", 0 )/* MCU Data */
	ROM_LOAD( "fb040e.u33",  0x000000, 0x020000, CRC(299d0746) SHA1(67fe3a47ab01fa02ce2bb5836c2041986c19d875) )

	ROM_REGION( 0x1000000, "kan_spr", ROMREGION_ERASEFF )  /* Sprites */
	ROM_LOAD( "fb-020a.u1", 0x000000, 0x100000, CRC(87e55c6d) SHA1(87886c045d7c30b8dee3c8fb0bf8f2cdbc5fd7fb) )
	ROM_LOAD( "fb020b.u2",  0x100000, 0x100000, CRC(276b9d7b) SHA1(7a154f65b4737f2b6ac8effa3352711079f571dc) )
	ROM_LOAD( "fb021a.u3",  0x200000, 0x100000, CRC(7da15d37) SHA1(345cf2242e8210a697294a45197f2b3b974de885) )
	ROM_LOAD( "fb021b.u4",  0x300000, 0x100000, CRC(6a512d7b) SHA1(7fc3002d23262a9a590a283ea9e111e38d889ef2) )
	ROM_LOAD( "fb-22a.u5",  0x400000, 0x100000, CRC(9039e5d3) SHA1(222452cd7947f7c99c68e495835cca62e0449b5c) )
	ROM_LOAD( "fb-22b.u6",  0x500000, 0x100000, CRC(96ac9e54) SHA1(2b066375963dc57fe2ce89d65f6c0a9d183a838d) )
	ROM_LOAD( "fb023.u7",   0x600000, 0x100000, CRC(132794bd) SHA1(bcc73c3183c59a4b66f79d04774773b8a9239501) )

	ROM_REGION( 0x400000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "fb010.u65",  0x000000, 0x100000, CRC(296ffd92) SHA1(183a28e4594c428deb4726ed22d5166592b94b60) )  // 42 pin mask rom
	ROM_LOAD( "fb011.u66",  0x100000, 0x080000, CRC(500a0367) SHA1(6dc5190f81b21f59ee56a3b2332c8d86d6599782) )  // 40 pin mask rom (verified correct)

	ROM_REGION( 0x100000, "oki1", 0 )
	ROM_LOAD( "fb001e.u43",  0x000000, 0x080000, CRC(f524aaa1) SHA1(006a886f9df2e57c51b61c6cea70a6574fc20304) )
	ROM_LOAD( "fb000e.u42",  0x080000, 0x080000, CRC(969f1465) SHA1(4f56d1ad341b08f4db41b7ab2498740612ff7c3d) )

	ROM_REGION( 0x200000, "oki2", 0 )
	ROM_LOAD( "fb-002.u45",   0x000000, 0x100000, CRC(010acc17) SHA1(2dc0897c7778eacf6bce12ff0adbadb307ea6c17) )
	ROM_LOAD( "fb-003.u44",   0x100000, 0x100000, CRC(0aea4ac5) SHA1(8f3b30e505b0ba51c140a0a2c071680d4fa05db9) )
ROM_END


ROM_START( shogwarrk )
	ROM_REGION( 0x040000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "fb-030a.u61", 0x000000, 0x020000, CRC(32ce7909) SHA1(02d87342706ac9547eb611bd542f8498ba41e34a) )
	ROM_LOAD16_BYTE( "fb-031a.u62", 0x000001, 0x020000, CRC(bbffe957) SHA1(d916fde185985f6de216b8fb68a851fc75e3d04b) )

	ROM_REGION( 0x020000, "calc3_rom", 0 )/* MCU Data */
	ROM_LOAD( "fb-040a.u33",  0x000000, 0x020000, CRC(299d0746) SHA1(67fe3a47ab01fa02ce2bb5836c2041986c19d875) )

	ROM_REGION( 0x1000000, "kan_spr", ROMREGION_ERASEFF )  /* Sprites */
	ROM_LOAD( "fb-020a.u1", 0x000000, 0x100000, CRC(87e55c6d) SHA1(87886c045d7c30b8dee3c8fb0bf8f2cdbc5fd7fb) )
	ROM_LOAD( "fb020b.u2",  0x100000, 0x100000, CRC(276b9d7b) SHA1(7a154f65b4737f2b6ac8effa3352711079f571dc) )
	ROM_LOAD( "fb021a.u3",  0x200000, 0x100000, CRC(7da15d37) SHA1(345cf2242e8210a697294a45197f2b3b974de885) )
	ROM_LOAD( "fb021b.u4",  0x300000, 0x100000, CRC(6a512d7b) SHA1(7fc3002d23262a9a590a283ea9e111e38d889ef2) )
	ROM_LOAD( "fb-22a.u5",  0x400000, 0x100000, CRC(9039e5d3) SHA1(222452cd7947f7c99c68e495835cca62e0449b5c) )
	ROM_LOAD( "fb-22b.u6",  0x500000, 0x100000, CRC(96ac9e54) SHA1(2b066375963dc57fe2ce89d65f6c0a9d183a838d) )
	ROM_LOAD( "fb023.u7",   0x600000, 0x100000, CRC(132794bd) SHA1(bcc73c3183c59a4b66f79d04774773b8a9239501) )
	ROM_LOAD( "fb-024.u8",  0x700000, 0x080000, CRC(4bf3282d) SHA1(6058e46f20fe4faf32cddb6f2bb0fbf70f256a54) )  // extra censored gfx

	ROM_REGION( 0x400000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "fb010.u65",  0x000000, 0x100000, CRC(296ffd92) SHA1(183a28e4594c428deb4726ed22d5166592b94b60) )  // 42 pin mask rom
	ROM_LOAD( "fb011.u66",  0x100000, 0x080000, CRC(500a0367) SHA1(6dc5190f81b21f59ee56a3b2332c8d86d6599782) )  // 40 pin mask rom (verified correct)

	ROM_REGION( 0x100000, "oki1", 0 )
	ROM_LOAD( "fb-000.u43",  0x000000, 0x080000, CRC(f524aaa1) SHA1(006a886f9df2e57c51b61c6cea70a6574fc20304) )
	ROM_LOAD( "fb-003.u101",  0x080000, 0x080000, CRC(969f1465) SHA1(4f56d1ad341b08f4db41b7ab2498740612ff7c3d) )

	ROM_REGION( 0x200000, "oki2", 0 )
	ROM_LOAD( "fb-002.u45",   0x000000, 0x100000, CRC(010acc17) SHA1(2dc0897c7778eacf6bce12ff0adbadb307ea6c17) )
	ROM_LOAD( "fb-003.u44",   0x100000, 0x100000, CRC(0aea4ac5) SHA1(8f3b30e505b0ba51c140a0a2c071680d4fa05db9) )
ROM_END

/***************************************************************************

                                Shogun Warriors

Shogun Warriors, Kaneko 1992

   fb010.u65           fb040.u33
   fb011.u66
   rb012.u67
   rb013.u68

                         fb001.u43
     68000-12            fb000.u42  m6295
    51257     fb030.u61  fb002.u44  m6295
    51257     fb031.u62  fb003.u45


                fb021a.u3
                fb021b.u4
                fb022a.u5
   fb023.u7     fb022b.u6
   fb020a.u1    fb020b.u2



---------------------------------------------------------------------------
                                Game code
---------------------------------------------------------------------------

102e04-7    <- !b80004-7
102e18.w    -> $800000
102e1c.w    -> $800002 , $800006
102e1a.w    -> $800004
102e20.w    -> $800008

ROUTINES:

6622    print ($600000)

***************************************************************************/


// the 'green garbage' on the VS logo shown in the video ( http://www.youtube.com/watch?v=lz4gY9d7uxw ) doesn't happen on the real PCB,
// it appears to be an encoding artifact on the videos uploaded by this poster

ROM_START( shogwarru )
	ROM_REGION( 0x040000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "fb030a.u61", 0x000000, 0x020000, CRC(a04106c6) SHA1(95ab084f2e709be7cec2964cb09bcf5a8d3aacdf) )
	ROM_LOAD16_BYTE( "fb031a.u62", 0x000001, 0x020000, CRC(d1def5e2) SHA1(f442de4433547e52b483549aca5786e4597a7122) )

	ROM_REGION( 0x020000, "calc3_rom", 0 )/* MCU Data */
	ROM_LOAD( "fb040a.u33",  0x000000, 0x020000, CRC(4b62c4d9) SHA1(35c943dde70438a411714070e42a84366db5ef83) )

	ROM_REGION( 0x1000000, "kan_spr", ROMREGION_ERASEFF )  /* Sprites */
	ROM_LOAD( "fb-020a.u1", 0x000000, 0x100000, CRC(87e55c6d) SHA1(87886c045d7c30b8dee3c8fb0bf8f2cdbc5fd7fb) )
	ROM_LOAD( "fb020b.u2",  0x100000, 0x100000, CRC(276b9d7b) SHA1(7a154f65b4737f2b6ac8effa3352711079f571dc) )
	ROM_LOAD( "fb021a.u3",  0x200000, 0x100000, CRC(7da15d37) SHA1(345cf2242e8210a697294a45197f2b3b974de885) )
	ROM_LOAD( "fb021b.u4",  0x300000, 0x100000, CRC(6a512d7b) SHA1(7fc3002d23262a9a590a283ea9e111e38d889ef2) )
	ROM_LOAD( "fb-22a.u5",  0x400000, 0x100000, CRC(9039e5d3) SHA1(222452cd7947f7c99c68e495835cca62e0449b5c) )
	ROM_LOAD( "fb-22b.u6",  0x500000, 0x100000, CRC(96ac9e54) SHA1(2b066375963dc57fe2ce89d65f6c0a9d183a838d) )
	ROM_LOAD( "fb023.u7",   0x600000, 0x100000, CRC(132794bd) SHA1(bcc73c3183c59a4b66f79d04774773b8a9239501) )

	ROM_REGION( 0x400000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "fb010.u65",  0x000000, 0x100000, CRC(296ffd92) SHA1(183a28e4594c428deb4726ed22d5166592b94b60) )  // 42 pin mask rom
	ROM_LOAD( "fb011.u66",  0x100000, 0x080000, CRC(500a0367) SHA1(6dc5190f81b21f59ee56a3b2332c8d86d6599782) )  // 40 pin mask rom (verified correct)

	ROM_REGION( 0x100000, "oki1", 0 )
	ROM_LOAD( "fb001e.u43",  0x000000, 0x080000, CRC(f524aaa1) SHA1(006a886f9df2e57c51b61c6cea70a6574fc20304) )
	ROM_LOAD( "fb000e.u42",  0x080000, 0x080000, CRC(969f1465) SHA1(4f56d1ad341b08f4db41b7ab2498740612ff7c3d) )

	ROM_REGION( 0x200000, "oki2", 0 )
	ROM_LOAD( "fb-002.u45",   0x000000, 0x100000, CRC(010acc17) SHA1(2dc0897c7778eacf6bce12ff0adbadb307ea6c17) )
	ROM_LOAD( "fb-003.u44",   0x100000, 0x100000, CRC(0aea4ac5) SHA1(8f3b30e505b0ba51c140a0a2c071680d4fa05db9) )
ROM_END

/***************************************************************************

                                Fujiyama Buster

Japan (c) 1992 Kaneko
This is the Japanese version of Shogun Warriors

Main PCB No: ZO1DK-002 (Same as B.Rap Boys)
 ROM PCB No: ZO5DP
        CPU: TMP68HC000N-12 (Toshiba)
        SND: OKI M6295 x 2
        OSC: 16.000MHz, 12.000MHz
        DIP: 1 x 8 POSITION

OTHER:
93C46 (8 Pin DIP, EEPROM, Linked to FB-040.U33 & CALC3 Chip)
KANEKO JAPAN 9152EV 175101 (160 Pin PQFP)
KANEKO VIEW2-CHIP (144 Pin PQFP)
KANEKO MUX2-CHIP (64 Pin PQFP)
KANEKO CALC3 508 (74 Pin PQFP, NEC uPD78322 MCU, Linked to FB-040.U33)
KANEKO JAPAN 9203 T (44 PIN PQFP)

Differences from Shogun Warriors:

File Name    CRC32       Labelled As   ROM Type
===============================================
fb030j.u61   0x32ce7909  FB030J/U61-00   27C010 | 68000 CPU Code
fb031j.u62   0x000c8c08  FB031J/U62-00   27C010 /

fb040j.u33   0x299d0746  FB040J/U33-00   27C010 - MCU Code? 68000 Code snipets??

fb000j.u43   0xa7522555  FB000J/U43-00   27C040 | Japanese Sound Samples
fb001j_u.101 0x07d4e8e2  FB001J/U101-0   27C040 /

NOTE: U67 & U68 are empty on this Original board.

***************************************************************************/

ROM_START( fjbuster )   // Fujiyama Buster - Japan version of Shogun Warriors
	ROM_REGION( 0x040000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "fb030j.u61", 0x000000, 0x020000, CRC(32ce7909) SHA1(02d87342706ac9547eb611bd542f8498ba41e34a) )
	ROM_LOAD16_BYTE( "fb031j.u62", 0x000001, 0x020000, CRC(000c8c08) SHA1(439daac1541c34557b5a4308ed69dfebb93abe13) )

	ROM_REGION( 0x020000, "calc3_rom", 0 )/* MCU Data */
	ROM_LOAD( "fb040j.u33",  0x000000, 0x020000, CRC(299d0746) SHA1(67fe3a47ab01fa02ce2bb5836c2041986c19d875) )

	ROM_REGION( 0x1000000, "kan_spr", ROMREGION_ERASEFF )  /* Sprites */
	ROM_LOAD( "fb-020a.u1", 0x000000, 0x100000, CRC(87e55c6d) SHA1(87886c045d7c30b8dee3c8fb0bf8f2cdbc5fd7fb) )
	ROM_LOAD( "fb020b.u2",  0x100000, 0x100000, CRC(276b9d7b) SHA1(7a154f65b4737f2b6ac8effa3352711079f571dc) )
	ROM_LOAD( "fb021a.u3",  0x200000, 0x100000, CRC(7da15d37) SHA1(345cf2242e8210a697294a45197f2b3b974de885) )
	ROM_LOAD( "fb021b.u4",  0x300000, 0x100000, CRC(6a512d7b) SHA1(7fc3002d23262a9a590a283ea9e111e38d889ef2) )
	ROM_LOAD( "fb-22a.u5",  0x400000, 0x100000, CRC(9039e5d3) SHA1(222452cd7947f7c99c68e495835cca62e0449b5c) )
	ROM_LOAD( "fb-22b.u6",  0x500000, 0x100000, CRC(96ac9e54) SHA1(2b066375963dc57fe2ce89d65f6c0a9d183a838d) )
	ROM_LOAD( "fb023.u7",   0x600000, 0x100000, CRC(132794bd) SHA1(bcc73c3183c59a4b66f79d04774773b8a9239501) )

	ROM_REGION( 0x400000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "fb010.u65",  0x000000, 0x100000, CRC(296ffd92) SHA1(183a28e4594c428deb4726ed22d5166592b94b60) )  // 42 pin mask rom
	ROM_LOAD( "fb011.u66",  0x100000, 0x080000, CRC(500a0367) SHA1(6dc5190f81b21f59ee56a3b2332c8d86d6599782) )  // 40 pin mask rom (verified correct)

	ROM_REGION( 0x100000, "oki1", 0 )
	ROM_LOAD( "fb000j.u43",    0x000000, 0x080000, CRC(a7522555) SHA1(ea88d90dda20bc309f98a1924c41551e7708e6af) )
	ROM_LOAD( "fb001j_u.101",  0x080000, 0x080000, CRC(07d4e8e2) SHA1(0de911f452ddeb54b0b435b9c1cf5d5881175d44) )

	ROM_REGION( 0x200000, "oki2", 0 )
	ROM_LOAD( "fb-002.u45",   0x000000, 0x100000, CRC(010acc17) SHA1(2dc0897c7778eacf6bce12ff0adbadb307ea6c17) )
	ROM_LOAD( "fb-003.u44",   0x100000, 0x100000, CRC(0aea4ac5) SHA1(8f3b30e505b0ba51c140a0a2c071680d4fa05db9) )
ROM_END

/***************************************************************************

                                B.Rap Boys

B.Rap Boys
Kaneko, 1992

Game is a beat-em up, where a bunch of rapping boys (The B.Rap Boys) beats
up anyone and anything that gets in their way, smashing shop windows
and other property with their fists, chairs, wooden bats and whatever else
they can lay their hands on!


Main PCB No: ZO1DK-002
ROM PCB No:  ZO1DK-EXROM
CPU: TMP68HC000N-12 @ 12.000MHz
SND: OKI M6295 x 2 @ 2.000MHz [16/8]. Pin 7 is low.
OSC: 16.000MHz, 12.000MHz
DIP: 1 x 8 POSITION
SW1 - PCB location for 2 position DIP but location is unpopulated.
SW2:

                            1   2   3   4   5   6   7   8
SCREEN FLIP     NORMAL      OFF
                FLIP        ON
MODE            NORMAL          OFF
                TEST            ON
SWITCH TEST [1] NO                  OFF
                YES                 ON
POSITION #4     NOT USED                OFF
COIN TYPE       LOCAL COIN                  OFF
                COMMON COIN                 ON
GAME TYPE       3 PLAYERS                       OFF
                2 PLAYERS                       ON
DIFFICULTY [2]  EASY                                ON  OFF
                NORMAL                              OFF OFF
                HARD                                OFF ON
                VERY HARD                           ON  ON

[1] This additional test becomes available in test mode when this DIP is ON.
[2] Additional settings available in test mode via another on-screen menu.
Some text is written in Japanese. See scan in archive for details.

Control is via 8 Way Joystick and 2 buttons

There are two extra pin connectors near the JAMMA connector.
Pinouts are....

(A)
10 3P START SW
 9 3P COIN SW
 8 3P BUTTON 2
 7 3P BUTTON 1
 6 3P UP
 5 3P DOWN
 4 3P LEFT
 3 3P RIGHT
 2 GND
 1 GND

(B)
6 COIN COUNTER 3
5 COIN LOCKOUT 3
4 TOTAL COIN COUNTER
3 NC
2 NC
1 NC

RAM:
M5M4464 x 6, M51257AL x 2, KM6264BLS x 2, D42101C x 2, LH5116D x 2, CAT71C256 x 2

OTHER:
93C46 (8 PIN DIP, EEPROM, LINKED TO RB-006.U33)
KANEKO JAPAN 9152EV 175101 (160 PIN PQFP)
KANEKO VIEW2-CHIP (144 PIN PQFP)
KANEKO MUX2-CHIP (64 PIN PQFP)
KANEKO CALC3 508 (74 PIN PQFP, NEC uPD78322 MCU, LINKED TO RB-006.U33)
KANEKO JAPAN 9204 T (44 PIN PQFP)
PALs (x 11, read protected, not dumped)

VSync is 59.1854Hz
HSync is 15.625kHz

ROMs:
RB-004.U61  27C010    \     Main program
RB-005.U62  27C010    /

RB-000.U43  2M mask   \
RB-001.U44  4M mask    |    Located near main program and OKI M6295 chips
RB-002.U45  4M mask    |    Possibly sound related / OKI Samples etc..
RB-003.101  4M mask   /

RB-006.U33  27C010      MCU program? (Linked to CALC3 508)

RB-010.U65  8M mask   \
RB-011.U66  8M mask    |    GFX
RB-012.U67  8M mask    |
RB-013.U68  8M mask   /

RB-021.U76  4M mask   \
RB-022.U77  4M mask    |
RB-023.U78  4M mask    |    GFX (located under a plug-in ROM PCB)
RB-024.U79  4M mask   /

RB-020.U2   4M mask   \
RB-025.U4   27C040     |    GFX (located on a plug-in ROM PCB)
RB-026.U5   27C040    /


-----

Game can be ROM Swapped onto a Shogun Warriors board and works

***************************************************************************/


ROM_START( brapboys ) /* World 'normal version', no rom sub board, serial RB92E000?x; suffix codes -00 and -01 */
	ROM_REGION( 0x040000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "rb-030.01.u61", 0x000000, 0x020000, CRC(ccbe9a53) SHA1(b96baf0ecbf6550bfaf8e512d9275c53a3928bee) ) /* eprom labeled RB-030/U61-01 (green label) */
	ROM_LOAD16_BYTE( "rb-031.01.u62", 0x000001, 0x020000, CRC(c72b8dda) SHA1(450e1fb8acb140fa0ab23630daad82924f7ce72b) ) /* eprom labeled RB-031/U62-01 (green label) */

	ROM_REGION( 0x020000, "calc3_rom", 0 )/* MCU Data */
	ROM_LOAD( "rb-040.00.u33",  0x000000, 0x020000, CRC(757c6e19) SHA1(0f1c37b1b1eb6b230c593e4648c4302f413a61f5) ) /* eprom labeled RB-040/U33-00 (green label) */

	ROM_REGION( 0x800000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "rb-020.u100", 0x000000, 0x100000, CRC(ce220d38) SHA1(b88d7c89a3e1a826bf19a1fa692ec77c944596d9) )
	ROM_LOAD( "rb-021.u76",  0x100000, 0x100000, CRC(74001407) SHA1(90002056ceb4e0401246950b8c3f996af0a2463c) )
	ROM_LOAD( "rb-022.u77",  0x200000, 0x100000, CRC(cb3f42dc) SHA1(5415f15621924dd263b8fe7daaf3dc25d470b814) )
	ROM_LOAD( "rb-023.u78",  0x300000, 0x100000, CRC(0e6530c5) SHA1(72bff46f0672927e540f4f3546ae533dd0a231e0) )
	ROM_LOAD( "rb-024.u79",  0x400000, 0x080000, CRC(65fa6447) SHA1(551e540d7bf412753b4a7098e25e6f9d8774bcf4) ) // correct, both halves identical when dumped as larger
	ROM_RELOAD( 0x480000,  0x080000 )
	ROM_LOAD( "rb-025.01.u80",  0x500000, 0x040000, CRC(36cd6b90) SHA1(45c50f2652726ded67c9c24185a71a6367e09270) ) // eprom labeled RB-025/U80-01 (green label), contains title logo for this version

	ROM_REGION( 0x400000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "rb-010.u65",  0x000000, 0x100000, CRC(ffd73f87) SHA1(1a661f71976be61c22d9b962850e738ba17f1d45) )
	ROM_LOAD( "rb-011.u66",  0x100000, 0x100000, CRC(d9325f78) SHA1(346832608664aa8f3ac9260a549903386b4125a8) )
	ROM_LOAD( "rb-012.u67",  0x200000, 0x100000, CRC(bfdbe0d1) SHA1(3abc5398ee8ee1871b4d081f9b748539d69bcdba) )
	ROM_LOAD( "rb-013.u68",  0x300000, 0x100000, CRC(28c37fe8) SHA1(e10dd1a810983077328b44e6e33ce2e899c506d2) )

	ROM_REGION( 0x100000, "oki1", 0 )
	ROM_LOAD( "rb-000.u43",  0x000000, 0x080000, CRC(58ad1a62) SHA1(1d2643b5f6eac22682972a88d284e00de3e3b223) )
	ROM_LOAD( "rb-003.00.u101", 0x080000, 0x080000, CRC(2cac25d7) SHA1(0412c317bf650a93051b9304d23035efde0c026a) ) /* eprom labeled RB-003/U43-00 (green label) however actual IC location is u101 */

	ROM_REGION( 0x200000, "oki2", 0 )
	ROM_LOAD( "rb-001.u44",  0x000000, 0x100000, CRC(7cf774b3) SHA1(3fb0a5096ce9480f97e311439042eb8cbc26efb4) )
	ROM_LOAD( "rb-002.u45",  0x100000, 0x100000, CRC(e4b30444) SHA1(be6756dce3721226e0b7f5d4d168008c31aeea8e) )
ROM_END

ROM_START( brapboysp ) /* World 'special version' with EXROM sub board; serial RB92E00554; suffix codes -A0 and -01 */
	ROM_REGION( 0x040000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "rb-030.a0.u61", 0x000000, 0x020000, CRC(fed2ecea) SHA1(0a7a017ffd39bd2440d45337b0adbec0f49a7529) ) /* eprom labeled RB-030/U61-A0 (black label) */
	ROM_LOAD16_BYTE( "rb-031.a0.u62", 0x000001, 0x020000, CRC(f64b9763) SHA1(69d37863aa9d263380aca6100a9166baf7300082) ) /* eprom labeled RB-031/U62-A0 (black label) */

	ROM_REGION( 0x020000, "calc3_rom", 0 )/* MCU Data */
	ROM_LOAD( "rb-040.a0.u33",  0x000000, 0x020000, CRC(7b856d27) SHA1(c9b3cb858302ed33c332a25f08573611d1468e94) ) /* eprom labeled RB-040/U33-A0 (black label) */

	ROM_REGION( 0x1000000, "kan_spr", 0 )  /* Sprites */
	ROM_LOAD( "rb-020.u100", 0x000000, 0x100000, CRC(ce220d38) SHA1(b88d7c89a3e1a826bf19a1fa692ec77c944596d9) ) // really at location next to capacitor C2 on Z01DK-EXROM daughterboard
	ROM_LOAD( "rb-021.u76",  0x100000, 0x100000, CRC(74001407) SHA1(90002056ceb4e0401246950b8c3f996af0a2463c) )
	ROM_LOAD( "rb-022.u77",  0x200000, 0x100000, CRC(cb3f42dc) SHA1(5415f15621924dd263b8fe7daaf3dc25d470b814) )
	ROM_LOAD( "rb-023.u78",  0x300000, 0x100000, CRC(0e6530c5) SHA1(72bff46f0672927e540f4f3546ae533dd0a231e0) )
	ROM_LOAD( "rb-024.u79",  0x400000, 0x080000, CRC(65fa6447) SHA1(551e540d7bf412753b4a7098e25e6f9d8774bcf4) ) // correct, both halves identical when dumped as larger
	ROM_RELOAD( 0x480000,  0x080000 )
	ROM_LOAD( "rb-025.a0.u80a",   0x500000, 0x080000, CRC(aa795ba5) SHA1(c5256dcceded2e76f548b60c18e51d0dd0209d81) ) // eprom labeled RB-025/U80aA0 (blue label), really at location next to capacitor C4 on Z01DK-EXROM daughterboard
	ROM_LOAD( "rb-026.10.u80b",   0x580000, 0x080000, CRC(bb7604d4) SHA1(57d51ce4ea2000f9a50bae326cfcb66ec494249f) ) // eprom labeled RB-026/U80b10 (red label), really at location next to capacitor C5 on Z01DK-EXROM daughterboard

	ROM_REGION( 0x400000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "rb-010.u65",  0x000000, 0x100000, CRC(ffd73f87) SHA1(1a661f71976be61c22d9b962850e738ba17f1d45) )
	ROM_LOAD( "rb-011.u66",  0x100000, 0x100000, CRC(d9325f78) SHA1(346832608664aa8f3ac9260a549903386b4125a8) )
	ROM_LOAD( "rb-012.u67",  0x200000, 0x100000, CRC(bfdbe0d1) SHA1(3abc5398ee8ee1871b4d081f9b748539d69bcdba) )
	ROM_LOAD( "rb-013.u68",  0x300000, 0x100000, CRC(28c37fe8) SHA1(e10dd1a810983077328b44e6e33ce2e899c506d2) )

	ROM_REGION( 0x100000, "oki1", 0 )
	ROM_LOAD( "rb-000.u43",  0x000000, 0x080000, CRC(58ad1a62) SHA1(1d2643b5f6eac22682972a88d284e00de3e3b223) )
	ROM_LOAD( "rb-003.00.u101", 0x080000, 0x080000, CRC(2cac25d7) SHA1(0412c317bf650a93051b9304d23035efde0c026a) ) /* eprom labeled RB-003/U43-00 however actual IC location is u101 */

	ROM_REGION( 0x200000, "oki2", 0 )
	ROM_LOAD( "rb-001.u44",  0x000000, 0x100000, CRC(7cf774b3) SHA1(3fb0a5096ce9480f97e311439042eb8cbc26efb4) )
	ROM_LOAD( "rb-002.u45",  0x100000, 0x100000, CRC(e4b30444) SHA1(be6756dce3721226e0b7f5d4d168008c31aeea8e) )
ROM_END

ROM_START( brapboysp2 ) // World (?) 'special version' with EXROM sub board; suffix code N */
	ROM_REGION( 0x040000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "rb-30n.u61", 0x000000, 0x020000, CRC(632269b0) SHA1(4dfe63b02cb95022b8c439d106b8b28d47b0e1da) ) // EPROM labeled RB-30N/U61 (white label)
	ROM_LOAD16_BYTE( "rb-31n.u62", 0x000001, 0x020000, CRC(99623650) SHA1(498d58716298ce6e1bb8146a75089301f46079b1) ) // EPROM labeled RB-31N/U62 (white label)

	ROM_REGION( 0x020000, "calc3_rom", 0 )
	ROM_LOAD( "rb-40n.u33",  0x000000, 0x020000, CRC(313b7a6d) SHA1(59e1ac88c70498a8731dc3b2b01515d0ba1ac65b) ) // EPROM labeled RB-40N/U33 (white label), 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x1000000, "kan_spr", 0 )
	ROM_LOAD( "rb-020.c2",   0x000000, 0x100000, CRC(ce220d38) SHA1(b88d7c89a3e1a826bf19a1fa692ec77c944596d9) ) // rb-020 0013 mask ROM; really at location next to capacitor C2 on Z01DK-EXROM-K daughterboard
	ROM_LOAD( "rb-021.u76",  0x100000, 0x100000, CRC(74001407) SHA1(90002056ceb4e0401246950b8c3f996af0a2463c) ) // rb-021 0014 mask ROM
	ROM_LOAD( "rb-022.u77",  0x200000, 0x100000, CRC(cb3f42dc) SHA1(5415f15621924dd263b8fe7daaf3dc25d470b814) ) // rb-022 0015 mask ROM
	ROM_LOAD( "rb-023.u78",  0x300000, 0x100000, CRC(0e6530c5) SHA1(72bff46f0672927e540f4f3546ae533dd0a231e0) ) // rb-023 0016 mask ROM
	ROM_LOAD( "rb-024.u79",  0x400000, 0x080000, CRC(65fa6447) SHA1(551e540d7bf412753b4a7098e25e6f9d8774bcf4) ) // rb-023 0017 w29 mask ROM, both halves identical when dumped as larger
	ROM_RELOAD( 0x480000,  0x080000 )
	ROM_LOAD( "rb-25n.c3",   0x500000, 0x040000, CRC(065d9bb5) SHA1(1f5b2f3cc95a4257eeeb9e41f9cc1c68f8118989) ) // EPROM labeled RB-25N (white label), really at location next to capacitor C3 on Z01DK-EXROM-K daughterboard
	ROM_LOAD( "rb-26n.c4",   0x540000, 0x040000, CRC(8fac668b) SHA1(0f98f8b0ab366b30b7fb613c910c16ff32d214b7) ) // EPROM labeled RB-26N (white label), really at location next to capacitor C4 on Z01DK-EXROM-K daughterboard
	ROM_LOAD( "rb-27n.c5",   0x580000, 0x040000, CRC(fb9ed35f) SHA1(7c1cd1f25febfeaeac4f51ff32b2d35b535d2539) ) // EPROM labeled RB-27N (white label), really at location next to capacitor C5 on Z01DK-EXROM-K daughterboard
	ROM_LOAD( "rb-28n.c7",   0x5c0000, 0x040000, CRC(13acdcab) SHA1(d2e3e9cfd6b6a61ecb5dde85fd16c13877bd6eb1) ) // EPROM labeled RB-28N (white label), really at location next to capacitor C7 on Z01DK-EXROM-K daughterboard

	ROM_REGION( 0x400000, "view2_0", 0 )
	ROM_LOAD( "rb-010.u65",  0x000000, 0x100000, CRC(ffd73f87) SHA1(1a661f71976be61c22d9b962850e738ba17f1d45) ) // rb-010 0009 w17 mask ROM
	ROM_LOAD( "rb-011.u66",  0x100000, 0x100000, CRC(d9325f78) SHA1(346832608664aa8f3ac9260a549903386b4125a8) ) // rb-011 0010 w18 mask ROM
	ROM_LOAD( "rb-012.u67",  0x200000, 0x100000, CRC(bfdbe0d1) SHA1(3abc5398ee8ee1871b4d081f9b748539d69bcdba) ) // rb-012 0011 w21 mask ROM
	ROM_LOAD( "rb-013.u68",  0x300000, 0x100000, CRC(28c37fe8) SHA1(e10dd1a810983077328b44e6e33ce2e899c506d2) ) // rb-013 0012 w22 mask ROM

	ROM_REGION( 0x100000, "oki1", 0 )
	ROM_LOAD( "rb-000.u43",  0x000000, 0x080000, CRC(58ad1a62) SHA1(1d2643b5f6eac22682972a88d284e00de3e3b223) ) // rb-000 0006 w28 mask ROM
	ROM_LOAD( "rb-003.u101", 0x080000, 0x080000, CRC(2cac25d7) SHA1(0412c317bf650a93051b9304d23035efde0c026a) ) // rb-003 0018 mask ROM

	ROM_REGION( 0x200000, "oki2", 0 )
	ROM_LOAD( "rb-001.u44",  0x000000, 0x100000, CRC(7cf774b3) SHA1(3fb0a5096ce9480f97e311439042eb8cbc26efb4) ) // rb-001 0007 mask ROM
	ROM_LOAD( "rb-002.u45",  0x100000, 0x100000, CRC(e4b30444) SHA1(be6756dce3721226e0b7f5d4d168008c31aeea8e) ) // rb-002 0008 w11 mask ROM
ROM_END

// TODO: the eproms for brapboysj are missing the region-specific numeric suffix;
// it should be something like -00 or -01 or -10 or -11 or etc
ROM_START( brapboyspj ) /* Japanese 'special version' with EXROM sub board; serial unknown; suffix codes unknown */
	ROM_REGION( 0x040000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "rb-004.u61", 0x000000, 0x020000, CRC(5432442c) SHA1(f0f7328ece96ef25e6d4fd1958d734f64a9ef371) ) // fill in suffix!
	ROM_LOAD16_BYTE( "rb-005.u62", 0x000001, 0x020000, CRC(118b3cfb) SHA1(1690ecf5c629879bd97131ff77029e152919e45d) ) // fill in suffix!

	ROM_REGION( 0x020000, "calc3_rom", 0 )/* MCU Data */
	ROM_LOAD( "rb-006.u33",  0x000000, 0x020000, CRC(f1d76b20) SHA1(c571b5f28e529589ee2d7697ef5d4b60ccb66e7a) ) // fill in suffix!

	ROM_REGION( 0x1000000, "kan_spr", 0 )  /* Sprites */
	ROM_LOAD( "rb-020.u100", 0x000000, 0x100000, CRC(ce220d38) SHA1(b88d7c89a3e1a826bf19a1fa692ec77c944596d9) ) // really at location next to capacitor C2 on Z01DK-EXROM daughterboard
	ROM_LOAD( "rb-021.u76",  0x100000, 0x100000, CRC(74001407) SHA1(90002056ceb4e0401246950b8c3f996af0a2463c) )
	ROM_LOAD( "rb-022.u77",  0x200000, 0x100000, CRC(cb3f42dc) SHA1(5415f15621924dd263b8fe7daaf3dc25d470b814) )
	ROM_LOAD( "rb-023.u78",  0x300000, 0x100000, CRC(0e6530c5) SHA1(72bff46f0672927e540f4f3546ae533dd0a231e0) )
	ROM_LOAD( "rb-024.u79",  0x400000, 0x080000, CRC(65fa6447) SHA1(551e540d7bf412753b4a7098e25e6f9d8774bcf4) ) // correct, both halves identical when dumped as larger
	ROM_RELOAD( 0x480000,  0x080000 )
	ROM_LOAD( "rb-025.a0.u80a",   0x500000, 0x080000, CRC(aa795ba5) SHA1(c5256dcceded2e76f548b60c18e51d0dd0209d81) ) // eprom labeled RB-025/U80aA0 (blue label), really at location next to capacitor C4 on Z01DK-EXROM daughterboard
	ROM_LOAD( "rb-026.10.u80b",   0x580000, 0x080000, CRC(bb7604d4) SHA1(57d51ce4ea2000f9a50bae326cfcb66ec494249f) ) // eprom labeled RB-026/U80b10 (red label), really at location next to capacitor C5 on Z01DK-EXROM daughterboard

	ROM_REGION( 0x400000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "rb-010.u65",  0x000000, 0x100000, CRC(ffd73f87) SHA1(1a661f71976be61c22d9b962850e738ba17f1d45) )
	ROM_LOAD( "rb-011.u66",  0x100000, 0x100000, CRC(d9325f78) SHA1(346832608664aa8f3ac9260a549903386b4125a8) )
	ROM_LOAD( "rb-012.u67",  0x200000, 0x100000, CRC(bfdbe0d1) SHA1(3abc5398ee8ee1871b4d081f9b748539d69bcdba) )
	ROM_LOAD( "rb-013.u68",  0x300000, 0x100000, CRC(28c37fe8) SHA1(e10dd1a810983077328b44e6e33ce2e899c506d2) )

	ROM_REGION( 0x100000, "oki1", 0 )
	ROM_LOAD( "rb-000.u43",  0x000000, 0x080000, CRC(58ad1a62) SHA1(1d2643b5f6eac22682972a88d284e00de3e3b223) )
	ROM_LOAD( "rb-003.00.u101", 0x080000, 0x080000, CRC(2cac25d7) SHA1(0412c317bf650a93051b9304d23035efde0c026a) ) /* eprom labeled RB-003/U43-00 however actual IC location is u101 */

	ROM_REGION( 0x200000, "oki2", 0 )
	ROM_LOAD( "rb-001.u44",  0x000000, 0x100000, CRC(7cf774b3) SHA1(3fb0a5096ce9480f97e311439042eb8cbc26efb4) )
	ROM_LOAD( "rb-002.u45",  0x100000, 0x100000, CRC(e4b30444) SHA1(be6756dce3721226e0b7f5d4d168008c31aeea8e) )
ROM_END

ROM_START( brapboyspu ) /* US 'special version' with EXROM sub board; Serial RB92A0008x/9x; suffix code -10 */
	ROM_REGION( 0x040000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "rb-030.10.u61", 0x000000, 0x020000, CRC(527eb92a) SHA1(64727675e58a4a71bea1d88d7f76f60929196505) ) /* eprom labeled RB-030/U61-10 (red label) */
	ROM_LOAD16_BYTE( "rb-031.10.u62", 0x000001, 0x020000, CRC(d5962bdd) SHA1(9badab4cc2a9064bd29c582d82ec0b003b9fb091) ) /* eprom labeled RB-031/U62-10 (red label) */

	ROM_REGION( 0x020000, "calc3_rom", 0 )/* MCU Data */
	ROM_LOAD( "rb-040.10.u33",  0x000000, 0x020000, CRC(0c90d758) SHA1(9b1a9856ab00f80f15bffc01276f636f92f0bd12) ) /* eprom labeled RB-040/U33-10 (red label)*/

	ROM_REGION( 0x1000000, "kan_spr", 0 )  /* Sprites */
	ROM_LOAD( "rb-020.u100", 0x000000, 0x100000, CRC(ce220d38) SHA1(b88d7c89a3e1a826bf19a1fa692ec77c944596d9) ) // rb-020 0013 mask rom; really at location next to capacitor C2 on Z01DK-EXROM daughterboard
	ROM_LOAD( "rb-021.u76",  0x100000, 0x100000, CRC(74001407) SHA1(90002056ceb4e0401246950b8c3f996af0a2463c) ) // rb-021 0014 mask rom
	ROM_LOAD( "rb-022.u77",  0x200000, 0x100000, CRC(cb3f42dc) SHA1(5415f15621924dd263b8fe7daaf3dc25d470b814) ) // rb-022 0015 mask rom
	ROM_LOAD( "rb-023.u78",  0x300000, 0x100000, CRC(0e6530c5) SHA1(72bff46f0672927e540f4f3546ae533dd0a231e0) ) // rb-023 0016 mask rom
	ROM_LOAD( "rb-024.u79",  0x400000, 0x080000, CRC(65fa6447) SHA1(551e540d7bf412753b4a7098e25e6f9d8774bcf4) ) // rb-023 0017 w29 mask rom, both halves identical when dumped as larger
	ROM_RELOAD( 0x480000,  0x080000 )
	ROM_LOAD( "rb-025.10.u80a",   0x500000, 0x080000, CRC(140fe400) SHA1(a764767aacec2f895f93256ab82125962c272951) ) // eprom labeled RB-025/U80a10 (red label), really at location next to capacitor C4 on Z01DK-EXROM daughterboard
	ROM_LOAD( "rb-026.10.u80b",   0x580000, 0x080000, CRC(bb7604d4) SHA1(57d51ce4ea2000f9a50bae326cfcb66ec494249f) ) // eprom labeled RB-026/U80b10 (red label), really at location next to capacitor C5 on Z01DK-EXROM daughterboard

	ROM_REGION( 0x400000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "rb-010.u65",  0x000000, 0x100000, CRC(ffd73f87) SHA1(1a661f71976be61c22d9b962850e738ba17f1d45) ) // rb-010 0009 w17 mask rom
	ROM_LOAD( "rb-011.u66",  0x100000, 0x100000, CRC(d9325f78) SHA1(346832608664aa8f3ac9260a549903386b4125a8) ) // rb-011 0010 w18 mask rom
	ROM_LOAD( "rb-012.u67",  0x200000, 0x100000, CRC(bfdbe0d1) SHA1(3abc5398ee8ee1871b4d081f9b748539d69bcdba) ) // rb-012 0011 w21 mask rom
	ROM_LOAD( "rb-013.u68",  0x300000, 0x100000, CRC(28c37fe8) SHA1(e10dd1a810983077328b44e6e33ce2e899c506d2) ) // rb-013 0012 w22 mask rom

	ROM_REGION( 0x100000, "oki1", 0 )
	ROM_LOAD( "rb-000.u43",  0x000000, 0x080000, CRC(58ad1a62) SHA1(1d2643b5f6eac22682972a88d284e00de3e3b223) ) // rb-000 0006 w28 mask rom
	ROM_LOAD( "rb-003.00.u101", 0x080000, 0x080000, CRC(2cac25d7) SHA1(0412c317bf650a93051b9304d23035efde0c026a) ) /* eprom labeled RB-003/U43-00 (green label) however actual IC location is u101 */

	ROM_REGION( 0x200000, "oki2", 0 )
	ROM_LOAD( "rb-001.u44",  0x000000, 0x100000, CRC(7cf774b3) SHA1(3fb0a5096ce9480f97e311439042eb8cbc26efb4) ) // rb-001 0007 mask rom
	ROM_LOAD( "rb-002.u45",  0x100000, 0x100000, CRC(e4b30444) SHA1(be6756dce3721226e0b7f5d4d168008c31aeea8e) ) // rb-002 0008 w11 mask rom
ROM_END

/**********************************************************************

                            Bonk's Adventure

Bonk's Adventure
Kaneko, 1994

PCB Layout
----------

Z09AF-003
|--------------------------------------------------------|
| LA4460  PC604109.101     PAL   3664 3664  PC500105.55  |
|  M6295   PC603108.102    PAL                           |
|  M6295   PC602107.100  PAL      VIEW2-CHIP             |
|          PC601106.99   PAL                             |
|                                           PC400104.51  |
|                                 3664  3664             |
|              62256                                     |
|J  KANEKO     62256              VIEW2-CHIP             |
|A  JAPAN                                                |
|M  9203T                                     424260     |
|M                                                       |
|A   62256    62256   6116                               |
|    PRG.7    PRG.8   6116      KANEKO    424260         |
|                    3364       KC-002    PAL            |
| 62256    68000     3364                                |
| 62256                                                  |
|          PAL      PAL  20MHz                           |
|          PAL      PAL  16MHz   PC600106.42             |
|                   PAL          PC700107.43             |
|        MCU.124                 PC200102.40             |
| DSW(8) KANEKO  93C46           PC100101.37 PC300103.38 |
|        TBSOP01 27MHz 33.3333MHz                        |
|--------------------------------------------------------|

Notes:
      68000 clock: 16.000MHz
      M6295 clock: 2.000MHz, Sample Rate: /165 (both)
      VSync: 60Hz
      HSync: 15.625kHz

      PC100-PC500: 16M MASK
      PC601-PC604: 8M MASK
      PC600-PC700: 27C4001
            PRG's: 27C4001
            MCU  : 27C010

**********************************************************************/

ROM_START( bonkadv )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "prg.8",        0x000000, 0x080000, CRC(af2e60f8) SHA1(406f79e155d1244b84f8c89c25b37188e1b4f4a6) )
	ROM_LOAD16_BYTE( "prg.7",        0x000001, 0x080000, CRC(a1cc6a78) SHA1(a9cea21a6a0dfd3b0952664681c057190aa27f8c) )

	ROM_REGION( 0x020000, "mcudata", 0 )            /* MCU Code */
	ROM_LOAD16_WORD_SWAP( "mcu.124",             0x000000, 0x020000, CRC(9d4e2724) SHA1(9dd43703265e39f876877020a0ac3875de6faa8d) )

	ROM_REGION( 0x500000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "pc100101.37",         0x000000, 0x200000, CRC(c96e7c10) SHA1(607cc7745abc3ff820047e8a00060ece61646623) )
	ROM_LOAD( "pc200102.40",         0x200000, 0x100000, CRC(c2b7a26a) SHA1(1c8783442e0ccf30c5640866c5493f1dc1dd48f8) )
	ROM_LOAD( "pc300103.38",         0x300000, 0x100000, CRC(51ee162c) SHA1(b33afc7d1e9f55f191e08472e8c51ca931b0389d) )
	ROM_LOAD16_BYTE( "pc600106.42",  0x400000, 0x080000, CRC(25877026) SHA1(96814d97e9f9284f98c35edfe5e76677ac50dd97) )
	ROM_LOAD16_BYTE( "pc700107.43",  0x400001, 0x080000, CRC(bfe21c44) SHA1(9900a6fe4182b720a90d64d368bd0fd08bf936a8) )

	ROM_REGION( 0x200000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "pc400104.51",         0x000000, 0x100000, CRC(3b176f84) SHA1(0ad6fd5f03d275165490881173bafcb0a94762eb) )

	ROM_REGION( 0x200000, "view2_1", 0 )   /* Tiles */
	ROM_LOAD( "pc500105.55",         0x000000, 0x100000, CRC(bebb3edc) SHA1(e0fed4307316deaeb811ec29f5022adeaf577a95) )

	ROM_REGION( 0x100000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "pc604109.101",        0x000000, 0x100000, CRC(76025530) SHA1(e0c8192d783057798eea084aa3e87938f6e01cb7) )

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "pc601106.99",         0x000000, 0x100000, CRC(a893651c) SHA1(d221ce89f19a76be497724f6c16fab82c8a52661) )
	ROM_LOAD( "pc602107.100",        0x100000, 0x100000, CRC(0fbb23aa) SHA1(69b620375c65246317d7105fbc414f3c36e02b2c) )
	ROM_LOAD( "pc603108.102",        0x200000, 0x100000, CRC(58458985) SHA1(9a846d604ba901eb2a59d2b6cd9c42e3b43adb6a) )
ROM_END


// Z09AF-003 PCB. No language select (but it has strings in Italian and German?). Also has a much longer list of level titles in ROM (the extra ones don't seem used, though).
// Handwritten sticker (in Japanese) says: 'Normal board version' 'Sensor sound: English'
ROM_START( bonkadva )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* 68000 Code */
	ROM_LOAD16_BYTE( "1-13.8",        0x000000, 0x080000, CRC(9a02e4ca) SHA1(5e8cbcedd11e8b3bb6c17da8f33951a08977ba1f) ) // handwritten: PC原人 AT 1/13
	ROM_LOAD16_BYTE( "1-13.7",        0x000001, 0x080000, CRC(6b0afe3d) SHA1(3cb1063c8c2ac93202ed9141caae810a8f6d9907) ) // handwritten: PC原人 AT 1/13

	ROM_REGION( 0x020000, "mcudata", 0 )            /* MCU Code */
	ROM_LOAD16_WORD_SWAP( "9200.124",             0x000000, 0x020000, CRC(9d4e2724) SHA1(9dd43703265e39f876877020a0ac3875de6faa8d) )

	ROM_REGION( 0x500000, "kan_spr", 0 )   /* Sprites */
	ROM_LOAD( "pc100101.37",         0x000000, 0x200000, CRC(c96e7c10) SHA1(607cc7745abc3ff820047e8a00060ece61646623) )
	ROM_LOAD( "pc200102.40",         0x200000, 0x100000, CRC(c2b7a26a) SHA1(1c8783442e0ccf30c5640866c5493f1dc1dd48f8) )
	ROM_LOAD( "pc300103.38",         0x300000, 0x100000, CRC(51ee162c) SHA1(b33afc7d1e9f55f191e08472e8c51ca931b0389d) )
	ROM_LOAD16_BYTE( "pc600106.42",  0x400000, 0x080000, CRC(25877026) SHA1(96814d97e9f9284f98c35edfe5e76677ac50dd97) )
	ROM_LOAD16_BYTE( "pc700107.43",  0x400001, 0x080000, CRC(bfe21c44) SHA1(9900a6fe4182b720a90d64d368bd0fd08bf936a8) )

	ROM_REGION( 0x200000, "view2_0", 0 )   /* Tiles */
	ROM_LOAD( "pc400104.51",         0x000000, 0x100000, CRC(3b176f84) SHA1(0ad6fd5f03d275165490881173bafcb0a94762eb) )

	ROM_REGION( 0x200000, "view2_1", 0 )   /* Tiles */
	ROM_LOAD( "pc500105.55",         0x000000, 0x100000, CRC(bebb3edc) SHA1(e0fed4307316deaeb811ec29f5022adeaf577a95) )

	ROM_REGION( 0x100000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "pc604109.101",        0x000000, 0x100000, CRC(76025530) SHA1(e0c8192d783057798eea084aa3e87938f6e01cb7) )

	ROM_REGION( 0x380000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "pc601106.99",         0x000000, 0x100000, CRC(a893651c) SHA1(d221ce89f19a76be497724f6c16fab82c8a52661) )
	ROM_LOAD( "pc602107.100",        0x100000, 0x100000, CRC(0fbb23aa) SHA1(69b620375c65246317d7105fbc414f3c36e02b2c) )
	ROM_LOAD( "pc603108.102",        0x200000, 0x100000, CRC(58458985) SHA1(9a846d604ba901eb2a59d2b6cd9c42e3b43adb6a) )
	ROM_LOAD( "ekt.103",             0x300000, 0x080000, CRC(d0d4014c) SHA1(2928739ce00c766bfd80a8cc279d3c1f86653890) ) // EEPROM instead of MASK ROM. TODO: verify if ROM loading is correct
ROM_END


void kaneko16_shogwarr_state::init_shogwarr()
{
	// default sample banks
	common_oki_bank_install(0, 0x30000, 0x10000);
	common_oki_bank_install(1, 0x00000, 0x40000);
}


void kaneko16_shogwarr_state::init_brapboys()
{
	// default sample banks
	common_oki_bank_install(0, 0x30000, 0x10000);
	common_oki_bank_install(1, 0x20000, 0x20000);
}


/***************************************************************************


                                Game drivers


***************************************************************************/

GAME( 1991, berlwall,   0,        berlwall, berlwall,  kaneko16_berlwall_state, init_berlwall,  ROT0,  "Kaneko", "The Berlin Wall", MACHINE_SUPPORTS_SAVE )
GAME( 1991, berlwallt,  berlwall, berlwall, berlwallt, kaneko16_berlwall_state, init_berlwallt, ROT0,  "Kaneko", "The Berlin Wall (bootleg?)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, berlwallk,  berlwall, berlwall, berlwallk, kaneko16_berlwall_state, init_berlwallk, ROT0,  "Kaneko (Inter license)", "The Berlin Wall (Korea)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, packbang,   0,        berlwall, packbang,  kaneko16_berlwall_state, init_berlwall_common, ROT90, "Kaneko", "Pack'n Bang Bang", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) // priorities between stages?
GAME( 1994, packbangp,  packbang, berlwall, packbang,  kaneko16_berlwall_state, init_berlwall_common, ROT90, "Kaneko", "Pack'n Bang Bang (prototype)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) // priorities between stages?

GAME( 1991, mgcrystl,   0,        mgcrystl, mgcrystl,  kaneko16_state,          empty_init,    ROT0,  "Kaneko", "Magical Crystals (World, 92/01/10)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, mgcrystlo,  mgcrystl, mgcrystl, mgcrystl,  kaneko16_state,          empty_init,    ROT0,  "Kaneko", "Magical Crystals (World, 91/12/10)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, mgcrystlj,  mgcrystl, mgcrystl, mgcrystl,  kaneko16_state,          empty_init,    ROT0,  "Kaneko (Atlus license)", "Magical Crystals (Japan, 92/01/13)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, blazeon,    0,        blazeon,  blazeon,   kaneko16_state,          empty_init,    ROT0,  "A.I (Atlus license)",  "Blaze On (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, blazeonj,   blazeon,  blazeon,  blazeon,   kaneko16_state,          empty_init,    ROT0,  "A.I (Atlus license)",  "Blaze On (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, explbrkr,   0,        bakubrkr, bakubrkr,  kaneko16_state,          init_bakubrkr, ROT90, "Kaneko", "Explosive Breaker (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, explbrkrk,  explbrkr, bakubrkr, bakubrkr,  kaneko16_state,          init_bakubrkr, ROT90, "Kaneko", "Explosive Breaker (Korea)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, bakubrkr,   explbrkr, bakubrkr, bakubrkr,  kaneko16_state,          init_bakubrkr, ROT90, "Kaneko", "Bakuretsu Breaker (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, wingforc,   0,        wingforc, wingforc,  kaneko16_state,          init_bakubrkr, ROT270,"A.I (Atlus license)",  "Wing Force (Japan, prototype)", MACHINE_SUPPORTS_SAVE )

GAME( 1994, bonkadv,    0,        bonkadv,  bonkadv,   kaneko16_gtmr_state,     init_gtmr,     ROT0,  "Kaneko", "B.C. Kid / Bonk's Adventure / Kyukyoku!! PC Genjin", MACHINE_SUPPORTS_SAVE )
GAME( 1994, bonkadva,   bonkadv,  bonkadv,  bonkadv,   kaneko16_gtmr_state,     init_gtmr,     ROT0,  "Kaneko", "Bonk's Adventure (prototype)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // shows an upside down KO! sprite, probably cause of check failing. Playable apart from that
GAME( 1994, bloodwar,   0,        bloodwar, bloodwar,  kaneko16_gtmr_state,     init_gtmr,     ROT0,  "Kaneko", "Blood Warrior", MACHINE_SUPPORTS_SAVE )
GAME( 1994, oedfight,   bloodwar, bloodwar, bloodwar,  kaneko16_gtmr_state,     init_gtmr,     ROT0,  "Kaneko", "Oedo Fight (Japan, Bloodshed version)", MACHINE_SUPPORTS_SAVE ) // shows blood effects like Blood Warrior version
GAME( 1994, oedfighta,  bloodwar, bloodwar, bloodwar,  kaneko16_gtmr_state,     init_gtmr,     ROT0,  "Kaneko", "Oedo Fight (Japan, Bloodless version)", MACHINE_SUPPORTS_SAVE ) // shows no blood effects
GAME( 1994, gtmr,       0,        gtmr,     gtmr,      kaneko16_gtmr_state,     init_gtmr,     ROT0,  "Kaneko", "1000 Miglia: Great 1000 Miles Rally (Taiwan 94/07/18)", MACHINE_SUPPORTS_SAVE ) // this set shows 'PCB by Jinwei Co Ltd. ROC', bootleg?
GAME( 1994, gtmra,      gtmr,     gtmr,     gtmr,      kaneko16_gtmr_state,     init_gtmr,     ROT0,  "Kaneko", "1000 Miglia: Great 1000 Miles Rally (94/06/13)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, gtmrb,      gtmr,     gtmr,     gtmr,      kaneko16_gtmr_state,     init_gtmr,     ROT0,  "Kaneko", "1000 Miglia: Great 1000 Miles Rally (94/05/26)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, gtmro,      gtmr,     gtmr,     gtmr,      kaneko16_gtmr_state,     init_gtmr,     ROT0,  "Kaneko", "1000 Miglia: Great 1000 Miles Rally (94/05/10)", MACHINE_SUPPORTS_SAVE ) // possible prototype
GAME( 1994, gtmre,      gtmr,     gtmre,    gtmr,      kaneko16_gtmr_state,     init_gtmr,     ROT0,  "Kaneko", "Great 1000 Miles Rally: Evolution Model!!! (94/09/06)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, gtmrusa,    gtmr,     gtmre,    gtmr,      kaneko16_gtmr_state,     init_gtmr,     ROT0,  "Kaneko", "Great 1000 Miles Rally: U.S.A Version! (94/09/06)", MACHINE_SUPPORTS_SAVE ) // U.S.A version seems part of the title, rather than region
GAME( 1995, gtmr2,      0,        gtmr2,    gtmr2,     kaneko16_gtmr_state,     init_gtmr,     ROT0,  "Kaneko", "Mille Miglia 2: Great 1000 Miles Rally (95/05/24)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, gtmr2a,     gtmr2,    gtmr2,    gtmr2,     kaneko16_gtmr_state,     init_gtmr,     ROT0,  "Kaneko", "Mille Miglia 2: Great 1000 Miles Rally (95/04/04)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, gtmr2u,     gtmr2,    gtmr2,    gtmr2,     kaneko16_gtmr_state,     init_gtmr,     ROT0,  "Kaneko", "Great 1000 Miles Rally 2 USA (95/07/11)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, gtmr2ua,    gtmr2,    gtmr2,    gtmr2,     kaneko16_gtmr_state,     init_gtmr,     ROT0,  "Kaneko", "Great 1000 Miles Rally 2 USA (95/05/18)", MACHINE_SUPPORTS_SAVE )

GAME( 1992, brapboys,   0,        brapboys, brapboys,  kaneko16_shogwarr_state, init_brapboys, ROT0,  "Kaneko", "B.Rap Boys (World)",                MACHINE_SUPPORTS_SAVE ) // KANEKO Be Rap Boys.xxx.06 string at 0x270
GAME( 1992, brapboysp,  brapboys, brapboys, brapboys,  kaneko16_shogwarr_state, init_brapboys, ROT0,  "Kaneko", "B.Rap Boys Special (World, newer)", MACHINE_SUPPORTS_SAVE ) // KANEKO Be Rap Boys.000.08 string at 0x270
GAME( 1992, brapboysp2, brapboys, brapboys, brapboys,  kaneko16_shogwarr_state, init_brapboys, ROT0,  "Kaneko", "B.Rap Boys Special (World, older)", MACHINE_SUPPORTS_SAVE ) // KANEKO Be Rap Boys.00x.07 string at 0x270
GAME( 1992, brapboyspj, brapboys, brapboys, brapboys,  kaneko16_shogwarr_state, init_brapboys, ROT0,  "Kaneko", "B.Rap Boys Special (Japan)",        MACHINE_SUPPORTS_SAVE ) // KANEKO Be Rap Boys.000.08 string at 0x270
GAME( 1992, brapboyspu, brapboys, brapboys, brapboys,  kaneko16_shogwarr_state, init_brapboys, ROT0,  "Kaneko", "B.Rap Boys Special (US)",           MACHINE_SUPPORTS_SAVE ) // KANEKO Be Rap Boys.000.08 string at 0x270
// these 3 are all the same code revision (differ by region byte + extra gfx roms/sample roms)
GAME( 1992, shogwarr,   0,        shogwarr, shogwarr,  kaneko16_shogwarr_state, init_shogwarr, ROT0,  "Kaneko", "Shogun Warriors (World)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1992, shogwarrk,  shogwarr, shogwarr, shogwarr,  kaneko16_shogwarr_state, init_shogwarr, ROT0,  "Kaneko", "Shogun Warriors (Korea?)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // censored Japanese flags etc.
GAME( 1992, fjbuster,   shogwarr, shogwarr, shogwarr,  kaneko16_shogwarr_state, init_shogwarr, ROT0,  "Kaneko", "Fujiyama Buster (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
// different revision
GAME( 1992, shogwarru,  shogwarr, shogwarr, shogwarr,  kaneko16_shogwarr_state, init_shogwarr, ROT0,  "Kaneko", "Shogun Warriors (US)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
