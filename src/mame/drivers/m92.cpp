// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*****************************************************************************

    Irem M92 system games:

    Gunforce (World)                M92-A   (c) 1991 Irem Corp
    Gunforce (USA)                  M92-A   (c) 1991 Irem America Corp
    Gunforce (Japan)                M92-A   (c) 1991 Irem Corp
    Blade Master (World)                    (c) 1991 Irem Corp
    Cross Blades! (Japan)                   (c) 1991 Irem Corp
    Lethal Thunder (World)                  (c) 1991 Irem Corp
    Thunder Blaster (Japan)                 (c) 1991 Irem Corp
    Undercover Cops (World)                 (c) 1992 Irem Corp
    Undercover Cops (Japan)                 (c) 1992 Irem Corp
    Mystic Riders (World)                   (c) 1992 Irem Corp
    Gun Hohki (Japan)                       (c) 1992 Irem Corp
    Major Title 2 (World)           M92-F   (c) 1992 Irem Corp
    Major Title 2 (Japan)           M92-F   (c) 1992 Irem Corp
    The Irem Skins Game (USA Set 1) M92-F   (c) 1992 Irem America Corp
    The Irem Skins Game (USA Set 2) M92-F   (c) 1992 Irem America Corp
    Hook (World)                            (c) 1992 Irem Corp
    Hook (Japan)                            (c) 1992 Irem Corp
    Hook (USA)                              (c) 1992 Irem America Corp
    R-Type Leo (World)                      (c) 1992 Irem Corp
    R-Type Leo (Japan)                      (c) 1992 Irem Corp
    In The Hunt (World)             M92-E   (c) 1993 Irem Corp
    In The Hunt (USA)               M92-E   (c) 1993 Irem Corp
    Kaitei Daisensou (Japan)        M92-E   (c) 1993 Irem Corp
    Ninja Baseball Batman (USA)             (c) 1993 Irem America Corp
    Yakyuu Kakutou League-Man (Japan)       (c) 1993 Irem Corp
    Perfect Soldiers (Japan)        M92-G   (c) 1993 Irem Corp
    Dream Soccer 94 (Japan)         M92-G   (c) 1994 Irem Corp
    Gunforce 2 (US)                 M92-G   (c) 1994 Irem Corp
    Geostorm (Japan)                M92-G   (c) 1994 Irem Corp

System notes:
    Each game has an encrypted sound cpu (see irem_cpu.c), the sound cpu and
    the sprite chip are on the game board rather than the main board and
    can differ between games.

    Irem Skins Game has an eeprom and ticket payout(?).
    R-Type Leo & Lethal Thunder have a memory card.

    Many games use raster IRQ's for special video effects, eg,
        * Scrolling water in Undercover Cops
        * Score display in R-Type Leo

Glitch list!

    All games:
        Flip screen/Cocktail Mode is unsupported (offsetted screens, and also Irem Skins Game
        hangs at title screen when flip is enabled), it's also unknown where exactly it's tied.

    Gunforce:
        Animated water sometimes doesn't appear on level 5 (but it
        always appears if you cheat and jump straight to the level).
        Almost certainly a core bug.

    Irem Skins:
        - EEPROM load/save not yet implemented - when done, MT2EEP should
          be removed from the ROM definition.

    LeagueMan:
        Raster effects don't work properly (not even cpu time per line?).

    (0.141 update: at least following two seems fixed from a lot of time ... -AS)
    Perfect Soldiers:
        Shortly into the fight, the sound CPU enters a tight loop, continuously
        writing to the status port and with interrupts disabled. I don't see how
        it is supposed to get out of that loop. Maybe it's not supposed to enter
        it at all?

    Dream Soccer 94:
        Slight priority problems when goal scoring animation is played

    Emulation by Bryan McPhail, mish@tendril.co.uk
    Thanks to Chris Hardy and Olli Bergmann too!


Sound programs:

Game                          Year  ID string
----------------------------  ----  ------------
Gunforce                      1991  -
Blade Master                  1991  -
Lethal Thunder                1991  -
Undercover Cops               1992  Rev 3.40 M92
Mystic Riders                 1992  Rev 3.44 M92
Major Title 2                 1992  Rev 3.44 M92
Hook                          1992  Rev 3.45 M92
R-Type Leo                    1992  Rev 3.45 M92
In The Hunt                   1993  Rev 3.45 M92
Ninja Baseball Batman         1993  Rev 3.50 M92
Perfect Soldiers              1993  Rev 3.50 M92
World PK Soccer               1995  Rev 3.51 M92
Fire Barrel                   1993  Rev 3.52 M92
Dream Soccer '94              1994  Rev 3.53 M92
Gunforce 2                    1994  Rev 3.53 M92

Gun Force
1991, Irem Corp.

PCB Layout
----------

Top board (Standard M92 Main Board)

M92-A-B   05C04170B1
|---------------------------|-----|--------------------|
|         MC3403  MC3403    |NANAO|                    |
|   064D                    |GA20 |         DSW3       |
|         MC3403  MC3403    |-----|                    |
|                             YM2151                   |
|   MC3403              YM3014                         |
|                                                      |
|J      CN6                           |-------|        |
|                      D71059C        |NEC    |        |
|A              6264                  |D71036L|        |
|                      62256   18MHz  |V33    |        |
|M                                    |-------|        |
|               6264   62256                           |
|M                           M92A-7J-.41(PAL)          |
|                                     M92A-9J-.51(PAL) |
|A                                                     |
|                               |-----|     |-----|    |
| CN4(4P)                       |NANAO|     |NANAO|    |
|                               |GA21 |     |GA22 |    |
|       M92A-3M-.11(PAL)        |-----|     |-----|    |
|                                                      |
|                       6264      6116                 |
| CN5(3P)                                              |
|       DSW2    DSW1    6264      6116      26.66666MHz|
|------------------------------------------------------|
Notes:
      V33 clock   : 9.000MHz
      GA20 clock  : 3.579545MHz (pin38)
      YM2151 clock: 3.579545MHz
      VSync       : 60Hz

      6116 : 2K x8 SRAM
      6264 : 8K x8 SRAM
      62256: 32K x8 SRAM

      CN4: Connector for 4th player controls
      CN5: Connector for 3rd player controls
      CN6: Connector for 2nd speaker (for stereo output)

      Custom chips:
                   NANAO GA20 (QFP80) - Sound chip
                   NANAO GA21 (QFP136)
                   NANAO GA22 (QFP160)



Bottom board (Game Board, differs per game)

M92-B-B   05C04171B1
|--------------------------------------------------------|
|              ROM_C0.9          ROM_001.29*  ROM_000.38 |
|                        |-----|                         |
|14.31818MHz   ROM_C1.10 |NANAO| ROM_011.30*  ROM_010.39 |
|                        |GA23 |                         |
| |----------| ROM_C2.11 |-----| ROM_021.31*  ROM_020.40 |
| |NANAO     |                                           |
| |08J27261A1| ROM_C3.12         ROM_031.32*  ROM_030.41 |
| |011       |                                           |
| |9108KK700 |                                           |
| |----------|                                           |
|                                                        |
|               GF_B-SH0-.14                             |
|                                                        |
|                                       M92_B-7H-.43(PAL)|
|                                                        |
|               6264                  GF_B-L0-C.25       |
|                                                        |
|                                     GF_B-L1-C.26       |
|               6264        62256                        |
|  M92B-2L-.7(PAL)                    GF_B-H1-C.27       |
|                           62256                        |
|  ROM_DA.8     GF_B-SL0.17           GF_B-H0-C.28       |
|                                                        |
|--------------------------------------------------------|
Notes:
      *: Unpopulated position (shown for reference for other M92 games)

      6264 : 8K x8 SRAM
      62256: 32K x8 SRAM

      Custom chips:
                   NANAO 08J27261A1 (PLCC84, encrypted V35 sound CPU, clocked at 14.31818MHz on pins 78 & 79)
                   NANAO GA23 (QFP180)


2008-08
Dip locations verified for:
    - dsoccr94j, gunforce, inthunt, majtitl2, uccops [manual]
    - bmaster, hook, lethalt, mysticri, rtypeleo [dip listing]
psoldier dip locations still need verification.

*****************************************************************************/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "cpu/nec/v25.h"
#include "includes/m92.h"
#include "includes/iremipt.h"
#include "machine/irem_cpu.h"
#include "sound/2151intf.h"
#include "sound/iremga20.h"
#include "sound/okim6295.h"


// I haven't managed to find a way to keep nbbatman happy when using the proper upd71059c device
// so we just use an ugly hack and get the vector base from it for now until this is properly resolved.
#define USE_HACKED_IRQS

#ifdef USE_HACKED_IRQS

#define M92_TRIGGER_IRQ0 m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_upd71059c->HACK_get_base_vector()+0 ); /* VBL interrupt */
#define M92_TRIGGER_IRQ1 m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_upd71059c->HACK_get_base_vector()+1 ); /* Sprite buffer complete interrupt */
#define M92_TRIGGER_IRQ2 m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_upd71059c->HACK_get_base_vector()+2 ); /* Raster interrupt */
#define M92_TRIGGER_IRQ3 m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_upd71059c->HACK_get_base_vector()+3 ); /* Sound cpu->Main cpu interrupt */
// not used due to HOLD LINE logic
#define M92_CLEAR_IRQ0 ;
#define M92_CLEAR_IRQ1 ;
#define M92_CLEAR_IRQ2 ;
#define M92_CLEAR_IRQ3 ;

#else

#define M92_TRIGGER_IRQ0 m_upd71059c->ir0_w(1);
#define M92_TRIGGER_IRQ1 m_upd71059c->ir1_w(1);
#define M92_TRIGGER_IRQ2 m_upd71059c->ir2_w(1);
#define M92_TRIGGER_IRQ3 m_upd71059c->ir3_w(1);
// not sure when these should happen, probably the source of our issues
#define M92_CLEAR_IRQ0 m_upd71059c->ir0_w(0);
#define M92_CLEAR_IRQ1 m_upd71059c->ir1_w(0);
#define M92_CLEAR_IRQ2 m_upd71059c->ir2_w(0);
#define M92_CLEAR_IRQ3 m_upd71059c->ir3_w(0);

#endif


/*****************************************************************************/

MACHINE_START_MEMBER(m92_state,m92)
{
	save_item(NAME(m_sound_status));
}

MACHINE_RESET_MEMBER(m92_state,m92)
{
	m_sprite_buffer_busy = 1;
}

/*****************************************************************************/


TIMER_DEVICE_CALLBACK_MEMBER(m92_state::m92_scanline_interrupt)
{
	int scanline = param;

	/* raster interrupt */
	if (scanline == m_raster_irq_position)
	{
		m_screen->update_partial(scanline);
		M92_TRIGGER_IRQ2
	}
	else
	{
		M92_CLEAR_IRQ2

		/* VBLANK interrupt */
		if (scanline == m_screen->visible_area().max_y + 1)
		{
			m_screen->update_partial(scanline);
			M92_TRIGGER_IRQ0
		}
		else
		{
			M92_CLEAR_IRQ0
		}

	}
}



/*****************************************************************************/

READ16_MEMBER(m92_state::m92_eeprom_r)
{
	UINT8 *RAM = memregion("eeprom")->base();
//  logerror("%05x: EEPROM RE %04x\n",space.device().safe_pc(),offset);
	return RAM[offset] | 0xff00;
}

WRITE16_MEMBER(m92_state::m92_eeprom_w)
{
	UINT8 *RAM = memregion("eeprom")->base();
//  logerror("%05x: EEPROM WR %04x\n",space.device().safe_pc(),offset);
	if (ACCESSING_BITS_0_7)
		RAM[offset] = data;
}

WRITE16_MEMBER(m92_state::m92_coincounter_w)
{
	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(machine(), 0, data & 0x01);
		coin_counter_w(machine(), 1, data & 0x02);
		/* Bit 0x8 is Motor(?!), used in Hook, In The Hunt, UCops */
		/* Bit 0x8 is Memcard related in RTypeLeo */
		/* Bit 0x40 set in Blade Master test mode input check */
	}
}

WRITE16_MEMBER(m92_state::m92_bankswitch_w)
{
	if (ACCESSING_BITS_0_7)
	{
		membank("bank1")->set_entry((data & 0x06) >> 1);
		if (data & 0xf9)
			logerror("%05x: bankswitch %04x\n", space.device().safe_pc(), data);
	}
}

CUSTOM_INPUT_MEMBER(m92_state::m92_sprite_busy_r)
{
	return m_sprite_buffer_busy;
}

/*****************************************************************************/

WRITE16_MEMBER(m92_state::m92_soundlatch_w)
{
	if (m_soundcpu)
		m_soundcpu->set_input_line(NEC_INPUT_LINE_INTP1, ASSERT_LINE);

	soundlatch_byte_w(space, 0, data & 0xff);
}

READ16_MEMBER(m92_state::m92_sound_status_r)
{
//logerror("%06x: read sound status\n",space.device().safe_pc());
	return m_sound_status;
}

READ16_MEMBER(m92_state::m92_soundlatch_r)
{
	if (m_soundcpu)
		m_soundcpu->set_input_line(NEC_INPUT_LINE_INTP1, CLEAR_LINE);

	return soundlatch_byte_r(space, offset) | 0xff00;
}

WRITE16_MEMBER(m92_state::m92_sound_irq_ack_w)
{
	if (m_soundcpu)
		m_soundcpu->set_input_line(NEC_INPUT_LINE_INTP1, CLEAR_LINE);
}

WRITE16_MEMBER(m92_state::m92_sound_status_w)
{
	COMBINE_DATA(&m_sound_status);
	M92_TRIGGER_IRQ3

}

WRITE16_MEMBER(m92_state::m92_sound_reset_w)
{
	if (m_soundcpu)
		m_soundcpu->set_input_line(INPUT_LINE_RESET, (data) ? CLEAR_LINE : ASSERT_LINE);
}

/*****************************************************************************/

/* appears to be an earlier board */
static ADDRESS_MAP_START( lethalth_map, AS_PROGRAM, 16, m92_state )
	AM_RANGE(0x00000, 0x7ffff) AM_ROM
	AM_RANGE(0x80000, 0x8ffff) AM_RAM_WRITE(m92_vram_w) AM_SHARE("vram_data")
	AM_RANGE(0xe0000, 0xeffff) AM_RAM /* System ram */
	AM_RANGE(0xf8000, 0xf87ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xf8800, 0xf8fff) AM_READWRITE(m92_paletteram_r, m92_paletteram_w)
	AM_RANGE(0xf9000, 0xf900f) AM_WRITE(m92_spritecontrol_w) AM_SHARE("spritecontrol")
	AM_RANGE(0xf9800, 0xf9801) AM_WRITE(m92_videocontrol_w)
	AM_RANGE(0xffff0, 0xfffff) AM_ROM AM_REGION("maincpu", 0x7fff0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( m92_map, AS_PROGRAM, 16, m92_state )
	AM_RANGE(0x00000, 0x9ffff) AM_ROM
	AM_RANGE(0xa0000, 0xbffff) AM_ROMBANK("bank1")
	AM_RANGE(0xc0000, 0xcffff) AM_ROM AM_REGION("maincpu", 0x00000) /* Mirror used by In The Hunt as protection */
	AM_RANGE(0xd0000, 0xdffff) AM_RAM_WRITE(m92_vram_w) AM_SHARE("vram_data")
	AM_RANGE(0xe0000, 0xeffff) AM_RAM /* System ram */
	AM_RANGE(0xf8000, 0xf87ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xf8800, 0xf8fff) AM_READWRITE(m92_paletteram_r, m92_paletteram_w)
	AM_RANGE(0xf9000, 0xf900f) AM_WRITE(m92_spritecontrol_w) AM_SHARE("spritecontrol")
	AM_RANGE(0xf9800, 0xf9801) AM_WRITE(m92_videocontrol_w)
	AM_RANGE(0xffff0, 0xfffff) AM_ROM AM_REGION("maincpu", 0x7fff0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( m92_portmap, AS_IO, 16, m92_state )
	AM_RANGE(0x00, 0x01) AM_READ_PORT("P1_P2")
	AM_RANGE(0x02, 0x03) AM_READ_PORT("COINS_DSW3")
	AM_RANGE(0x04, 0x05) AM_READ_PORT("DSW")
	AM_RANGE(0x06, 0x07) AM_READ_PORT("P3_P4")
	AM_RANGE(0x08, 0x09) AM_READ(m92_sound_status_r)    /* answer from sound CPU */
	AM_RANGE(0x00, 0x01) AM_WRITE(m92_soundlatch_w)
	AM_RANGE(0x02, 0x03) AM_WRITE(m92_coincounter_w)
	AM_RANGE(0x40, 0x43) AM_DEVREADWRITE8("upd71059c", pic8259_device, read, write, 0x00ff)
	AM_RANGE(0x80, 0x87) AM_WRITE(m92_pf1_control_w)
	AM_RANGE(0x88, 0x8f) AM_WRITE(m92_pf2_control_w)
	AM_RANGE(0x90, 0x97) AM_WRITE(m92_pf3_control_w)
	AM_RANGE(0x98, 0x9f) AM_WRITE(m92_master_control_w)
	AM_RANGE(0xc0, 0xc1) AM_WRITE(m92_sound_reset_w)
ADDRESS_MAP_END

WRITE16_MEMBER(m92_state::oki_bank_w)
{
	m_oki->set_bank_base(0x40000 * ((data+1) & 0x3)); // +1?
}

static ADDRESS_MAP_START( ppan_portmap, AS_IO, 16, m92_state )
	AM_RANGE(0x00, 0x01) AM_READ_PORT("P1_P2")
	AM_RANGE(0x02, 0x03) AM_READ_PORT("COINS_DSW3")
	AM_RANGE(0x04, 0x05) AM_READ_PORT("DSW")
	AM_RANGE(0x06, 0x07) AM_READ_PORT("P3_P4")
	AM_RANGE(0x08, 0x09) AM_READ(m92_sound_status_r)    /* answer from sound CPU */
	AM_RANGE(0x10, 0x11) AM_WRITE(oki_bank_w)
	AM_RANGE(0x18, 0x19) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x02, 0x03) AM_WRITE(m92_coincounter_w)
	AM_RANGE(0x40, 0x43) AM_DEVREADWRITE8("upd71059c", pic8259_device, read, write, 0x00ff)
	AM_RANGE(0x80, 0x87) AM_WRITE(m92_pf1_control_w)
	AM_RANGE(0x88, 0x8f) AM_WRITE(m92_pf2_control_w)
	AM_RANGE(0x90, 0x97) AM_WRITE(m92_pf3_control_w)
	AM_RANGE(0x98, 0x9f) AM_WRITE(m92_master_control_w)
ADDRESS_MAP_END


/******************************************************************************/

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 16, m92_state )
	AM_RANGE(0x00000, 0x1ffff) AM_ROM
	AM_RANGE(0xa0000, 0xa3fff) AM_RAM
	AM_RANGE(0xa8000, 0xa803f) AM_DEVREADWRITE8("irem", iremga20_device, irem_ga20_r, irem_ga20_w, 0x00ff)
	AM_RANGE(0xa8040, 0xa8043) AM_DEVREADWRITE8("ymsnd", ym2151_device, read, write, 0x00ff)
	AM_RANGE(0xa8044, 0xa8045) AM_READWRITE(m92_soundlatch_r, m92_sound_irq_ack_w)
	AM_RANGE(0xa8046, 0xa8047) AM_WRITE(m92_sound_status_w)
	AM_RANGE(0xffff0, 0xfffff) AM_ROM AM_REGION("soundcpu", 0x1fff0)
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( m92_2player )
	PORT_START("P1_P2")
	IREM_GENERIC_JOYSTICKS_2_BUTTONS(1, 2)

	PORT_START("COINS_DSW3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, m92_state,m92_sprite_busy_r, NULL)
	/* DIP switch bank 3 */
	PORT_DIPUNKNOWN_DIPLOC( 0x0100, 0x0100, "SW3:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0200, "SW3:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0400, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0800, "SW3:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "SW3:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW3:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "SW3:8" )

	PORT_START("DSW")
	/* Dip switch bank 1 */
	PORT_DIPUNKNOWN_DIPLOC( 0x0001, 0x0001, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0002, 0x0002, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0004, 0x0004, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0008, 0x0008, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, 0x0010, "SW1:5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" )
	/* Dip switch bank 2 */
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	/* According to the manuals, SW2:2,3 should control cabinet type and be NOT USED in m92_2player games */
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0200, "SW2:2" )
	PORT_DIPNAME( 0x0400, 0x0400, "Coin Slots" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Common" )
	PORT_DIPSETTING(      0x0000, "Separate" )
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	/* Coin Mode 1 */
	IREM_COIN_MODE_1_NEW_HIGH
	/* Coin mode 2 */
	IREM_COIN_MODE_2_HIGH

	PORT_START("P3_P4")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( m92_3player )
	PORT_INCLUDE(m92_2player)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0200, "2 Players" )
	PORT_DIPSETTING(      0x0000, "3 Players" )

	PORT_MODIFY("P3_P4")
	IREM_INPUT_PLAYER_3
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( m92_4player )
	PORT_INCLUDE(m92_3player)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0200, "2 Players" )
	PORT_DIPSETTING(      0x0000, "4 Players" )

	PORT_MODIFY("P3_P4")
	IREM_INPUT_PLAYER_4
INPUT_PORTS_END

/******************************************************************************/

static INPUT_PORTS_START( bmaster )
	PORT_INCLUDE(m92_2player)

	/* Game manual specificly mentions dip switch bank 3 is unused */

	PORT_MODIFY("DSW")
	/* Dip switch bank 1 */
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0003, "2" )
	PORT_DIPSETTING(      0x0002, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, "300k only" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )
INPUT_PORTS_END


static INPUT_PORTS_START( gunforce )
	PORT_INCLUDE(m92_2player)

	PORT_MODIFY("DSW")
	/* Dip switch bank 1 */
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0000, "15000 35000 75000 120000" )
	PORT_DIPSETTING(      0x0010, "20000 40000 90000 150000" )
INPUT_PORTS_END


static INPUT_PORTS_START( lethalth )
	PORT_INCLUDE(m92_2player)

	PORT_MODIFY("COINS_DSW3")
	/* Dip switch bank 3 */
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(      0x0200, "500K & 1M" )
	PORT_DIPSETTING(      0x0300, "700K & 1.5M" )
	PORT_DIPSETTING(      0x0000, "700K, 1.5M, 3M & 4.5M" )
	PORT_DIPSETTING(      0x0100, "1M & 2M" )

	PORT_MODIFY("DSW")
	/* Dip switch bank 1 */
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Continuous Play" ) PORT_DIPLOCATION("SW1:5") /* manual says Unused */
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( hook )
	PORT_INCLUDE(m92_4player)

	PORT_MODIFY("DSW")
	/* Dip switch bank 1 */
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0003, "2" )
	PORT_DIPSETTING(      0x0002, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, 0x0010, "SW1:5" )
	PORT_DIPNAME( 0x0020, 0x0020, "Any Button to Start" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
INPUT_PORTS_END


static INPUT_PORTS_START( majtitl2 )
	PORT_INCLUDE(m92_4player)

	PORT_MODIFY("COINS_DSW3")
	/* Dip switch bank 3 */
	PORT_DIPNAME( 0x0100, 0x0100, "Ticket Dispenser" ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )  /* "Ticket payout function is not working now" will be shown on screen */
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )   /* Stored data is shown on screen with the option to clear data */
	PORT_DIPNAME( 0x0600, 0x0600, "Points Per Ticket" ) PORT_DIPLOCATION("SW3:2,3") /* Conversion Rate for Ticket */
	PORT_DIPSETTING(      0x0600, "1 Point - 1 Ticket" )
	PORT_DIPSETTING(      0x0400, "2 Points - 1 Ticket" )
	PORT_DIPSETTING(      0x0200, "5 Points - 1 Ticket" )
	PORT_DIPSETTING(      0x0000, "10 Points - 1 Ticket" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW3:4" ) /* Game manual states dips 4, 5, 6 & 7 are "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "SW3:5" )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW3:6" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW3:7" )
	PORT_DIPNAME( 0x8000, 0x8000, "Deltronics Model" ) PORT_DIPLOCATION("SW3:8") /* Ticket Despenser Model Type */
	PORT_DIPSETTING(      0x8000, "DL 1275" )
	PORT_DIPSETTING(      0x0000, "DL 4SS" )

	PORT_MODIFY("DSW")
	/* Dip switch bank 2 */
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )

	/* Dip switch bank 1 */
	PORT_DIPNAME( 0x0001, 0x0001, "Given Holes/Stroke Play" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0001, "2" )
	PORT_DIPNAME( 0x0002, 0x0002, "Given Holes/Match or Skins" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Joystick Configuration" ) PORT_DIPLOCATION("SW1:4")    /* Listed as "Joysticks on" */
	PORT_DIPSETTING(      0x0008, DEF_STR( Upright ) )          /* "One Side" */
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )         /* "Both Sides" */
	PORT_DIPNAME( 0x0010, 0x0010, "Number of Joysticks" ) PORT_DIPLOCATION("SW1:5") /* 4 Way joysticks with 2 buttons each player */
	PORT_DIPSETTING(      0x0010, "2 Joysticks" )
	PORT_DIPSETTING(      0x0000, "4 Joysticks" )
	PORT_DIPNAME( 0x0020, 0x0000, "Any Button to Start" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
INPUT_PORTS_END


static INPUT_PORTS_START( mysticri )
	PORT_INCLUDE(m92_2player)

	PORT_MODIFY("DSW")
	/* Dip switch bank 1 */
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0000, "15000 35000 60000" )
	PORT_DIPSETTING(      0x0010, "20000 50000 90000" )
INPUT_PORTS_END


static INPUT_PORTS_START( uccops )
	PORT_INCLUDE(m92_3player)

	PORT_MODIFY("DSW")
	/* Dip switch bank 1 */
	PORT_DIPNAME( 0x0003, 0x0002, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0003, "2" )
	PORT_DIPSETTING(      0x0002, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	/* There is ALLWAYS a Bonus Life at 300K */
	/* It does not depends on the value of bit 0x0010 */
	PORT_DIPNAME( 0x0020, 0x0020, "Any Button to Start" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
INPUT_PORTS_END


static INPUT_PORTS_START( rtypeleo )
	PORT_INCLUDE(m92_2player)

	PORT_MODIFY("DSW")
	/* Dip switch bank 1 */
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
INPUT_PORTS_END


static INPUT_PORTS_START( inthunt )
	PORT_INCLUDE(m92_2player)

	PORT_MODIFY("DSW")
	/* Dip switch bank 1 */
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Any Button to Start" ) PORT_DIPLOCATION("SW1:6") /* not mentioned in the manual */
	PORT_DIPSETTING(      0x0020, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
INPUT_PORTS_END


static INPUT_PORTS_START( nbbatman )
	PORT_INCLUDE(m92_4player)

	PORT_MODIFY("DSW")
	/* Dip switch bank 1 */
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0003, "2" )
	PORT_DIPSETTING(      0x0002, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0020, 0x0000, "Any Button to Start" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
INPUT_PORTS_END


static INPUT_PORTS_START( psoldier )
	PORT_INCLUDE(m92_2player)

	PORT_MODIFY("P1_P2")
	IREM_GENERIC_JOYSTICKS_3_BUTTONS(1, 2)

	PORT_MODIFY("DSW")
	/* Dip switch bank 1 */
	PORT_DIPNAME( 0x0020, 0x0000, "Any Button to Start" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )

	PORT_MODIFY("P3_P4")    /* Extra connector for kick buttons */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( dsoccr94j )
	PORT_INCLUDE(m92_4player)
	/* Dip Switch 2, dip 2 is listed as "Don't Change" and is "OFF" */

	PORT_MODIFY("COINS_DSW3")
	/* Dip switch bank 3 */
	PORT_DIPNAME( 0x0300, 0x0300, "Player Power" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(      0x0000, "500" )
	PORT_DIPSETTING(      0x0300, "1000" )
	PORT_DIPSETTING(      0x0100, "1500" )
	PORT_DIPSETTING(      0x0200, "2000" )

	PORT_MODIFY("DSW")
	/* Dip switch bank 1 */
	PORT_DIPNAME( 0x0003, 0x0003, "Time" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, "1:30" )
	PORT_DIPSETTING(      0x0003, "2:00" )
	PORT_DIPSETTING(      0x0002, "2:30" )
	PORT_DIPSETTING(      0x0001, "3:00" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Game Mode" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, "Match Mode" )
	PORT_DIPSETTING(      0x0000, "Power Mode" )
/*
   Match Mode: Winner advances to the next game.  Game Over for the loser
   Power Mode: The Players can play the game until their respective powers run
               out, reguardless of whether they win or lose the game.
               Player 2 can join in any time during the game
               Player power (time) can be adjusted by dip switch #3
*/
	PORT_DIPNAME( 0x0020, 0x0020, "Starting Button" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, "Button 1" )
	PORT_DIPSETTING(      0x0020, "Start Button" )
INPUT_PORTS_END


static INPUT_PORTS_START( gunforc2 )
	PORT_INCLUDE(m92_2player)

	PORT_MODIFY("DSW")
	/* Dip switch bank 1 */
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "3" )
	PORT_DIPSETTING(      0x0003, "2" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
INPUT_PORTS_END


/***************************************************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,4),
	4,  /* 4 bits per pixel */
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ STEP8(0,1), STEP8(16*8,1) },
	{ STEP16(0,8) },
	32*8
};

static const gfx_layout spritelayout2 =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	32*8
};

static GFXDECODE_START( m92 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 128 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0, 128 )
GFXDECODE_END

static GFXDECODE_START( psoldier )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,    0, 128 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout2, 0, 128 )
GFXDECODE_END

static const gfx_layout bootleg_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 24,16,8,0 },
	{ 0,1,2,3,4,5,6,7 },
	{ STEP8(0,32) },
	8*32
};

static const gfx_layout bootleg_spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 48,32,16,0 },
	{ 8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7 },
	{ STEP16(0,64) },
	1024
};


static GFXDECODE_START( bootleg )
	GFXDECODE_ENTRY( "gfx1", 0, bootleg_charlayout,   0x400, 128 )
	GFXDECODE_ENTRY( "gfx2", 0, bootleg_spritelayout, 0x400, 128 )
GFXDECODE_END



/***************************************************************************/

void m92_state::m92_sprite_interrupt()
{
	M92_TRIGGER_IRQ1
}


static MACHINE_CONFIG_START( m92, m92_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",V33,XTAL_18MHz/2)
	MCFG_CPU_PROGRAM_MAP(m92_map)
	MCFG_CPU_IO_MAP(m92_portmap)
#ifndef USE_HACKED_IRQS
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("upd71059c", pic8259_device, inta_cb)
#endif

	MCFG_CPU_ADD("soundcpu" ,V35, XTAL_14_31818MHz)
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_PIC8259_ADD( "upd71059c", INPUTLINE("maincpu", 0), VCC, NULL)

	MCFG_MACHINE_START_OVERRIDE(m92_state,m92)
	MCFG_MACHINE_RESET_OVERRIDE(m92_state,m92)

	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", m92_state, m92_scanline_interrupt, "screen", 0, 1)

	/* video hardware */
	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(80, 511-112, 8, 247) /* 320 x 240 */
	MCFG_SCREEN_UPDATE_DRIVER(m92_state, screen_update_m92)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", m92)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(m92_state,m92)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", XTAL_14_31818MHz/4)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("soundcpu", NEC_INPUT_LINE_INTP0))
	MCFG_SOUND_ROUTE(0, "mono", 0.40)
	MCFG_SOUND_ROUTE(1, "mono", 0.40)

	MCFG_IREMGA20_ADD("irem", XTAL_14_31818MHz/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( gunforce, m92 )
	MCFG_CPU_MODIFY("soundcpu")
	MCFG_V25_CONFIG(gunforce_decryption_table)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bmaster, m92 )
	MCFG_CPU_MODIFY("soundcpu")
	MCFG_V25_CONFIG(bomberman_decryption_table)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( lethalth, m92 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(lethalth_map)
	MCFG_CPU_MODIFY("soundcpu")
	MCFG_V25_CONFIG(lethalth_decryption_table)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( uccops, m92 )
	MCFG_CPU_MODIFY("soundcpu")
	MCFG_V25_CONFIG(dynablaster_decryption_table)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mysticri, m92 )
	MCFG_CPU_MODIFY("soundcpu")
	MCFG_V25_CONFIG(mysticri_decryption_table)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( majtitl2, m92 )
	MCFG_CPU_MODIFY("soundcpu")
	MCFG_V25_CONFIG(majtitl2_decryption_table)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hook, m92 )
	MCFG_CPU_MODIFY("soundcpu")
	MCFG_V25_CONFIG(hook_decryption_table)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ppan, m92 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(ppan_portmap)

	MCFG_DEVICE_REMOVE("soundcpu")
	MCFG_DEVICE_REMOVE("ymsnd")
	MCFG_DEVICE_REMOVE("irem")

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(m92_state, screen_update_ppan)

	MCFG_VIDEO_START_OVERRIDE(m92_state,ppan)

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( rtypeleo, m92 )
	MCFG_CPU_MODIFY("soundcpu")
	MCFG_V25_CONFIG(rtypeleo_decryption_table)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( inthunt, m92 )
	MCFG_CPU_MODIFY("soundcpu")
	MCFG_V25_CONFIG(inthunt_decryption_table)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( nbbatman, m92 )
	MCFG_CPU_MODIFY("soundcpu")
	MCFG_V25_CONFIG(leagueman_decryption_table)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( nbbatman2bl, m92 )
	MCFG_DEVICE_REMOVE("soundcpu")
	MCFG_DEVICE_REMOVE("ymsnd")
	MCFG_DEVICE_REMOVE("irem")

	MCFG_GFXDECODE_MODIFY("gfxdecode", bootleg)

	/* 8951 MCU as sound CPU */
	/* OKI6295 (AD-65) as sound */

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( psoldier, m92 )
	MCFG_CPU_MODIFY("soundcpu")
	MCFG_V25_CONFIG(psoldier_decryption_table)
	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", psoldier)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dsoccr94j, m92 )
	MCFG_CPU_MODIFY("soundcpu")
	MCFG_V25_CONFIG(dsoccr94_decryption_table)
	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", psoldier)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gunforc2, m92 )
	MCFG_CPU_MODIFY("soundcpu")
	MCFG_V25_CONFIG(lethalth_decryption_table)
MACHINE_CONFIG_END

/***************************************************************************/

ROM_START( bmaster ) /* M92-B-B PCB used for program rom locations */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bm_d-h0-b.5m", 0x000001, 0x40000, CRC(49b257c7) SHA1(cb4917ef6c5f959094f95b8535ea12e6b9b0bcc2) )
	ROM_LOAD16_BYTE( "bm_d-l0-b.5f", 0x000000, 0x40000, CRC(a873523e) SHA1(9aee134c299e12064842e16db296f4259eccdf5b) )
	ROM_LOAD16_BYTE( "bm_d-h1-b.5l", 0x080001, 0x10000, CRC(082b7158) SHA1(ca2cfcb3ecd1f130d3fb893f08d53521e7d443d4) )
	ROM_LOAD16_BYTE( "bm_d-l1-b.5j", 0x080000, 0x10000, CRC(6ff0c04e) SHA1(7293a50445053101d22bc596d13e1a7ed67a65c6) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "bm_d-sh0.rom", 0x00001, 0x10000, CRC(9f7c075b) SHA1(1dd3fb4dc41d3adea9ca8d1b4363dadebea49bda) )
	ROM_LOAD16_BYTE( "bm_d-sl0.rom", 0x00000, 0x10000, CRC(1fa87c89) SHA1(971eae7dd2591191ed7a948a444387896735e149) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "bm_c0.rom", 0x000000, 0x40000, CRC(2cc966b8) SHA1(4d55954813efe975b7e644448effc61b22896e0b) )
	ROM_LOAD( "bm_c1.rom", 0x040000, 0x40000, CRC(46df773e) SHA1(6f075492c06768f7d2315906ec1349fe09def22f) )
	ROM_LOAD( "bm_c2.rom", 0x080000, 0x40000, CRC(05b867bd) SHA1(d44667f3f4908bacb6e10becc431b0f213c20407) )
	ROM_LOAD( "bm_c3.rom", 0x0c0000, 0x40000, CRC(0a2227a4) SHA1(30499e99f3731993607e04c77637f6bbe641c05c) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "bm_000.rom", 0x000000, 0x80000, CRC(339fc9f3) SHA1(36be0f3b5add2ecf3f602933f5456091daaeb1f6) )
	ROM_LOAD( "bm_010.rom", 0x080000, 0x80000, CRC(6a14377d) SHA1(699e5b1984810ee9e504f9ddaec604671c0cb0b7) )
	ROM_LOAD( "bm_020.rom", 0x100000, 0x80000, CRC(31532198) SHA1(7a285e003a7c359f5b1afe4da3b44069f716f7b5) )
	ROM_LOAD( "bm_030.rom", 0x180000, 0x80000, CRC(d1a041d3) SHA1(84a8cf5911426ed785cb678395f52da0a9199546) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "bm_da.rom", 0x000000, 0x80000, CRC(62ce5798) SHA1(f7bf7706f71ce36d85c99e531d4789c4d7a095a0) )
ROM_END

ROM_START( crossbld ) /* M92-D-A PCB used for program rom locations */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bm_d-h0.3h", 0x000001, 0x40000, CRC(a28a5821) SHA1(2e79ec82dd79697f4a6b4082d49400d39cc3bad9) )
	ROM_LOAD16_BYTE( "bm_d-l0.5h", 0x000000, 0x40000, CRC(a504f1a0) SHA1(33ccc944b08b89e6a975a164c72b36aa79b99392) )
	ROM_LOAD16_BYTE( "bm_d-h1.3e", 0x080001, 0x10000, CRC(18da6c47) SHA1(7b8cf82cf0c94d1ec64e77e15b877b5ffd307bc3) )
	ROM_LOAD16_BYTE( "bm_d-l1.5e", 0x080000, 0x10000, CRC(a65c1b42) SHA1(beb4131d045158231ba999b72f21c97c014672d0) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "bm_d-sh0.rom", 0x00001, 0x10000, CRC(9f7c075b) SHA1(1dd3fb4dc41d3adea9ca8d1b4363dadebea49bda) )
	ROM_LOAD16_BYTE( "bm_d-sl0.rom", 0x00000, 0x10000, CRC(1fa87c89) SHA1(971eae7dd2591191ed7a948a444387896735e149) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "bm_c0.rom", 0x000000, 0x40000, CRC(2cc966b8) SHA1(4d55954813efe975b7e644448effc61b22896e0b) )
	ROM_LOAD( "bm_c1.rom", 0x040000, 0x40000, CRC(46df773e) SHA1(6f075492c06768f7d2315906ec1349fe09def22f) )
	ROM_LOAD( "bm_c2.rom", 0x080000, 0x40000, CRC(05b867bd) SHA1(d44667f3f4908bacb6e10becc431b0f213c20407) )
	ROM_LOAD( "bm_c3.rom", 0x0c0000, 0x40000, CRC(0a2227a4) SHA1(30499e99f3731993607e04c77637f6bbe641c05c) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "bm_000.rom", 0x000000, 0x80000, CRC(339fc9f3) SHA1(36be0f3b5add2ecf3f602933f5456091daaeb1f6) )
	ROM_LOAD( "bm_010.rom", 0x080000, 0x80000, CRC(6a14377d) SHA1(699e5b1984810ee9e504f9ddaec604671c0cb0b7) )
	ROM_LOAD( "bm_020.rom", 0x100000, 0x80000, CRC(31532198) SHA1(7a285e003a7c359f5b1afe4da3b44069f716f7b5) )
	ROM_LOAD( "bm_030.rom", 0x180000, 0x80000, CRC(d1a041d3) SHA1(84a8cf5911426ed785cb678395f52da0a9199546) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "bm_da.rom", 0x000000, 0x80000, CRC(62ce5798) SHA1(f7bf7706f71ce36d85c99e531d4789c4d7a095a0) )
ROM_END

ROM_START( skingame )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "is-h0-d.5m", 0x00001, 0x40000, CRC(80940abb) SHA1(7af5b667383f40987cc8190d81937410ea2c0301) )
	ROM_LOAD16_BYTE( "is-l0-d.5f", 0x00000, 0x40000, CRC(b84beed6) SHA1(b026a68623d7d96545a4b01770fc6cdd2a0ed0f4) )
	ROM_LOAD16_BYTE( "is-h1.5l",   0x80001, 0x40000, CRC(9ba8e1f2) SHA1(ae86697a97223d236e2e6dd33ddb8105b9f926cb) )
	ROM_LOAD16_BYTE( "is-l1.5j",   0x80000, 0x40000, CRC(e4e00626) SHA1(e8c6c7ad6a367da4036915a155c8695ad90ae47b) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "mt2sh0",  0x00001, 0x10000, CRC(1ecbea43) SHA1(8d66ef419f75569f2c83a89c3985742b8a47914f) )
	ROM_LOAD16_BYTE( "mt2sl0",  0x00000, 0x10000, CRC(8fd5b531) SHA1(92cae3f6dac7f89b559063de3be2f38587536b65) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "c0", 0x000000, 0x40000, CRC(7e61e4b5) SHA1(d0164862937bd506e701777c51dea1ddb3e2eda4) )
	ROM_LOAD( "c1", 0x040000, 0x40000, CRC(0a667564) SHA1(d122e0619ae5cc0202f30270933784c954eb1e5d) )
	ROM_LOAD( "c2", 0x080000, 0x40000, CRC(5eb44312) SHA1(75b584b63d4f4f2236a679235461f11004aa317f) )
	ROM_LOAD( "c3", 0x0c0000, 0x40000, CRC(f2866294) SHA1(75e0071bf6282c93034dc7e73466af0f51046d01) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "k30", 0x000000, 0x100000, CRC(8c9a2678) SHA1(e8ed119c16ddd59af9e83d243e7be25974f7cbf8) )
	ROM_LOAD( "k31", 0x100000, 0x100000, CRC(5455df78) SHA1(9e49bde1d5a310ff611932c3429601fbddf3a7b1) )
	ROM_LOAD( "k32", 0x200000, 0x100000, CRC(3a258c41) SHA1(1d93fcd01728929848b782870f80a8cd0af44796) )
	ROM_LOAD( "k33", 0x300000, 0x100000, CRC(c1e91a14) SHA1(1f0dbd99d8c5067dc3f8795fc3f1bd4466f64156) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "da", 0x000000, 0x80000, CRC(713b9e9f) SHA1(91384d67d4ba9c7d926fbecb077293c661b8ec83) )

	ROM_REGION( 0x4000, "eeprom", 0 )   /* D28C64C-20 EEPROM */
	ROM_LOAD( "mt2eep",  0x000000, 0x800, CRC(208af971) SHA1(69384cac24b7af35a031f9b60e035131a8b10cb2) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "pal16l8-m92-a-3m.ic11", 0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8-m92-a-7j.ic41", 0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8-m92-a-9j.ic51", 0x0400, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8-m92-b-2l.ic7",  0x0600, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8-m92-b-7h.ic47", 0x0800, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( majtitl2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mt2-h0-b.5m", 0x00001, 0x40000, CRC(b163b12e) SHA1(cdb01a5266bd11f4cff1cb5c05cf24de13a527b2) )
	ROM_LOAD16_BYTE( "mt2-l0-b.5f", 0x00000, 0x40000, CRC(6f3b5d9d) SHA1(a39f25f29195023fb507dc9ffbfcbd57a4e6b30a) )
	ROM_LOAD16_BYTE( "is-h1.5l",    0x80001, 0x40000, CRC(9ba8e1f2) SHA1(ae86697a97223d236e2e6dd33ddb8105b9f926cb) )
	ROM_LOAD16_BYTE( "is-l1.5j",    0x80000, 0x40000, CRC(e4e00626) SHA1(e8c6c7ad6a367da4036915a155c8695ad90ae47b) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "mt2sh0",  0x00001, 0x10000, CRC(1ecbea43) SHA1(8d66ef419f75569f2c83a89c3985742b8a47914f) )
	ROM_LOAD16_BYTE( "mt2sl0",  0x00000, 0x10000, CRC(8fd5b531) SHA1(92cae3f6dac7f89b559063de3be2f38587536b65) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "c0", 0x000000, 0x40000, CRC(7e61e4b5) SHA1(d0164862937bd506e701777c51dea1ddb3e2eda4) )
	ROM_LOAD( "c1", 0x040000, 0x40000, CRC(0a667564) SHA1(d122e0619ae5cc0202f30270933784c954eb1e5d) )
	ROM_LOAD( "c2", 0x080000, 0x40000, CRC(5eb44312) SHA1(75b584b63d4f4f2236a679235461f11004aa317f) )
	ROM_LOAD( "c3", 0x0c0000, 0x40000, CRC(f2866294) SHA1(75e0071bf6282c93034dc7e73466af0f51046d01) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "k30", 0x000000, 0x100000, CRC(8c9a2678) SHA1(e8ed119c16ddd59af9e83d243e7be25974f7cbf8) )
	ROM_LOAD( "k31", 0x100000, 0x100000, CRC(5455df78) SHA1(9e49bde1d5a310ff611932c3429601fbddf3a7b1) )
	ROM_LOAD( "k32", 0x200000, 0x100000, CRC(3a258c41) SHA1(1d93fcd01728929848b782870f80a8cd0af44796) )
	ROM_LOAD( "k33", 0x300000, 0x100000, CRC(c1e91a14) SHA1(1f0dbd99d8c5067dc3f8795fc3f1bd4466f64156) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "da", 0x000000, 0x80000, CRC(713b9e9f) SHA1(91384d67d4ba9c7d926fbecb077293c661b8ec83) )

	ROM_REGION( 0x4000, "eeprom", 0 )   /* D28C64C-20 EEPROM */
	ROM_LOAD( "mt2eep",  0x000000, 0x800, CRC(208af971) SHA1(69384cac24b7af35a031f9b60e035131a8b10cb2) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "pal16l8-m92-a-3m.ic11", 0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8-m92-a-7j.ic41", 0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8-m92-a-9j.ic51", 0x0400, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8-m92-b-2l.ic7",  0x0600, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8-m92-b-7h.ic47", 0x0800, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( majtitl2j )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mt2-h0",      0x00001, 0x40000, CRC(8a8d71ad) SHA1(8c45d947d15eb3a2c2584c9e1cd0e42988955905) )
	ROM_LOAD16_BYTE( "mt2-l1",      0x00000, 0x40000, CRC(dd4fff51) SHA1(9281bac10fdbfa9eede9d069b70eb38d9ae612ce) )
	ROM_LOAD16_BYTE( "is-h1.5l",    0x80001, 0x40000, CRC(9ba8e1f2) SHA1(ae86697a97223d236e2e6dd33ddb8105b9f926cb) )
	ROM_LOAD16_BYTE( "is-l1.5j",    0x80000, 0x40000, CRC(e4e00626) SHA1(e8c6c7ad6a367da4036915a155c8695ad90ae47b) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "mt2sh0",  0x00001, 0x10000, CRC(1ecbea43) SHA1(8d66ef419f75569f2c83a89c3985742b8a47914f) )
	ROM_LOAD16_BYTE( "mt2sl0",  0x00000, 0x10000, CRC(8fd5b531) SHA1(92cae3f6dac7f89b559063de3be2f38587536b65) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "c0", 0x000000, 0x40000, CRC(7e61e4b5) SHA1(d0164862937bd506e701777c51dea1ddb3e2eda4) )
	ROM_LOAD( "c1", 0x040000, 0x40000, CRC(0a667564) SHA1(d122e0619ae5cc0202f30270933784c954eb1e5d) )
	ROM_LOAD( "c2", 0x080000, 0x40000, CRC(5eb44312) SHA1(75b584b63d4f4f2236a679235461f11004aa317f) )
	ROM_LOAD( "c3", 0x0c0000, 0x40000, CRC(f2866294) SHA1(75e0071bf6282c93034dc7e73466af0f51046d01) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "k30", 0x000000, 0x100000, CRC(8c9a2678) SHA1(e8ed119c16ddd59af9e83d243e7be25974f7cbf8) )
	ROM_LOAD( "k31", 0x100000, 0x100000, CRC(5455df78) SHA1(9e49bde1d5a310ff611932c3429601fbddf3a7b1) )
	ROM_LOAD( "k32", 0x200000, 0x100000, CRC(3a258c41) SHA1(1d93fcd01728929848b782870f80a8cd0af44796) )
	ROM_LOAD( "k33", 0x300000, 0x100000, CRC(c1e91a14) SHA1(1f0dbd99d8c5067dc3f8795fc3f1bd4466f64156) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "da", 0x000000, 0x80000, CRC(713b9e9f) SHA1(91384d67d4ba9c7d926fbecb077293c661b8ec83) )

	ROM_REGION( 0x4000, "eeprom", 0 )   /* D28C64C-20 EEPROM */
	ROM_LOAD( "mt2eep",  0x000000, 0x800, CRC(208af971) SHA1(69384cac24b7af35a031f9b60e035131a8b10cb2) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "pal16l8-m92-a-3m.ic11", 0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8-m92-a-7j.ic41", 0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8-m92-a-9j.ic51", 0x0400, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8-m92-b-2l.ic7",  0x0600, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8-m92-b-7h.ic47", 0x0800, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( skingame2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mt2-h0-a.5m", 0x00001, 0x40000, CRC(7c6dbbc7) SHA1(6ac2df542cbcac782d733aaa0f2e4ded702ec24f) )
	ROM_LOAD16_BYTE( "mt2-l0-a.5f", 0x00000, 0x40000, CRC(9de5f689) SHA1(ea5057cab0a2f5c4586337fc5a17f1a728450cbf) )
	ROM_LOAD16_BYTE( "is-h1.5l",    0x80001, 0x40000, CRC(9ba8e1f2) SHA1(ae86697a97223d236e2e6dd33ddb8105b9f926cb) )
	ROM_LOAD16_BYTE( "is-l1.5j",    0x80000, 0x40000, CRC(e4e00626) SHA1(e8c6c7ad6a367da4036915a155c8695ad90ae47b) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "mt2sh0",  0x00001, 0x10000, CRC(1ecbea43) SHA1(8d66ef419f75569f2c83a89c3985742b8a47914f) )
	ROM_LOAD16_BYTE( "mt2sl0",  0x00000, 0x10000, CRC(8fd5b531) SHA1(92cae3f6dac7f89b559063de3be2f38587536b65) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "c0", 0x000000, 0x40000, CRC(7e61e4b5) SHA1(d0164862937bd506e701777c51dea1ddb3e2eda4) )
	ROM_LOAD( "c1", 0x040000, 0x40000, CRC(0a667564) SHA1(d122e0619ae5cc0202f30270933784c954eb1e5d) )
	ROM_LOAD( "c2", 0x080000, 0x40000, CRC(5eb44312) SHA1(75b584b63d4f4f2236a679235461f11004aa317f) )
	ROM_LOAD( "c3", 0x0c0000, 0x40000, CRC(f2866294) SHA1(75e0071bf6282c93034dc7e73466af0f51046d01) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "k30", 0x000000, 0x100000, CRC(8c9a2678) SHA1(e8ed119c16ddd59af9e83d243e7be25974f7cbf8) )
	ROM_LOAD( "k31", 0x100000, 0x100000, CRC(5455df78) SHA1(9e49bde1d5a310ff611932c3429601fbddf3a7b1) )
	ROM_LOAD( "k32", 0x200000, 0x100000, CRC(3a258c41) SHA1(1d93fcd01728929848b782870f80a8cd0af44796) )
	ROM_LOAD( "k33", 0x300000, 0x100000, CRC(c1e91a14) SHA1(1f0dbd99d8c5067dc3f8795fc3f1bd4466f64156) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "da", 0x000000, 0x80000, CRC(713b9e9f) SHA1(91384d67d4ba9c7d926fbecb077293c661b8ec83) )

	ROM_REGION( 0x4000, "eeprom", 0 )   /* D28C64C-20 EEPROM */
	ROM_LOAD( "mt2eep",  0x000000, 0x800, CRC(208af971) SHA1(69384cac24b7af35a031f9b60e035131a8b10cb2) )

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "pal16l8-m92-a-3m.ic11", 0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8-m92-a-7j.ic41", 0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8-m92-a-9j.ic51", 0x0400, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8-m92-b-2l.ic7",  0x0600, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8-m92-b-7h.ic47", 0x0800, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( gunforce )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gf_h0-c.5m", 0x000001, 0x20000, CRC(c09bb634) SHA1(9b0e3174beeef173f5ef70f86f5db42bb01d9989) )
	ROM_LOAD16_BYTE( "gf_l0-c.5f", 0x000000, 0x20000, CRC(1bef6f7d) SHA1(ff4d674fc5f97f5b298b4b5dc73fb8a6772b5f09) )
	ROM_LOAD16_BYTE( "gf_h1-c.5l", 0x040001, 0x20000, CRC(c84188b7) SHA1(ff710be742f610d90538db296acdd435260bef12) )
	ROM_LOAD16_BYTE( "gf_l1-c.5j", 0x040000, 0x20000, CRC(b189f72a) SHA1(f17d87349a57e1a4b20c4947e41edd7c39eaca13) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "gf_sh0.rom", 0x00001, 0x10000, CRC(3f8f16e0) SHA1(a9f568c1b585c2cf13b21716954dac0a89936fc6) )
	ROM_LOAD16_BYTE( "gf_sl0.rom", 0x00000, 0x10000, CRC(db0b13a3) SHA1(6723026010610b706725a5284a7b8d70fe479dae) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "gf_c0.rom", 0x000000, 0x40000, CRC(b3b74979) SHA1(b5b4a4775e0b28c3f37273f93f69886c911af4aa) )
	ROM_LOAD( "gf_c1.rom", 0x040000, 0x40000, CRC(f5c8590a) SHA1(a7f90f23051f8ab2b2d925e950a5ef3c260170ca) )
	ROM_LOAD( "gf_c2.rom", 0x080000, 0x40000, CRC(30f9fb64) SHA1(f86e01b0d74a1f6c19d97d6d0e0f624f050dad10) )
	ROM_LOAD( "gf_c3.rom", 0x0c0000, 0x40000, CRC(87b3e621) SHA1(8e2655c6e83d00c38210fdced25003793bd93d9f) )

	ROM_REGION( 0x100000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "gf_000.rom", 0x000000, 0x40000, CRC(209e8e8d) SHA1(9720be888905be709733c53da207c3406d73aeb1) )
	ROM_LOAD( "gf_010.rom", 0x040000, 0x40000, CRC(6e6e7808) SHA1(92c30eecf8f3669581720be6e49db87fbfac7d88) )
	ROM_LOAD( "gf_020.rom", 0x080000, 0x40000, CRC(6f5c3cb0) SHA1(e41572c267489e2078f8d5605c97abe2034a091a) )
	ROM_LOAD( "gf_030.rom", 0x0c0000, 0x40000, CRC(18978a9f) SHA1(aa484710a7c3561a9922f119a064f9205475ae64) )

	ROM_REGION( 0x20000, "irem", 0 )
	ROM_LOAD( "gf-da.rom", 0x000000, 0x020000, CRC(933ba935) SHA1(482811e01239feecf10e232566a7809d0d4f11b8) )
ROM_END

ROM_START( gunforcej )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gfb_h0-e.5m", 0x000001, 0x20000, CRC(43c36e0f) SHA1(08c278861568f0a2fb2699b89a4170f6843bbcb7) )
	ROM_LOAD16_BYTE( "gfb_l0-e.5f", 0x000000, 0x20000, CRC(24a558d8) SHA1(89a9fb737d51798bdd5c08f448d2d8b3e161396a) )
	ROM_LOAD16_BYTE( "gfb_h1-e.5l", 0x040001, 0x20000, CRC(d9744f5d) SHA1(056d6e6e9874c33dcebe2e0ec946117d5eaa5d76) )
	ROM_LOAD16_BYTE( "gfb_l1-e.5j", 0x040000, 0x20000, CRC(a0f7b61b) SHA1(5fc7fc3f57e82a9ae4e1f3c3e8e3e3b0bd3ff8f5) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "gf_sh0.rom", 0x00001, 0x10000, CRC(3f8f16e0) SHA1(a9f568c1b585c2cf13b21716954dac0a89936fc6) )
	ROM_LOAD16_BYTE( "gf_sl0.rom", 0x00000, 0x10000, CRC(db0b13a3) SHA1(6723026010610b706725a5284a7b8d70fe479dae) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "gf_c0.rom", 0x000000, 0x40000, CRC(b3b74979) SHA1(b5b4a4775e0b28c3f37273f93f69886c911af4aa) )
	ROM_LOAD( "gf_c1.rom", 0x040000, 0x40000, CRC(f5c8590a) SHA1(a7f90f23051f8ab2b2d925e950a5ef3c260170ca) )
	ROM_LOAD( "gf_c2.rom", 0x080000, 0x40000, CRC(30f9fb64) SHA1(f86e01b0d74a1f6c19d97d6d0e0f624f050dad10) )
	ROM_LOAD( "gf_c3.rom", 0x0c0000, 0x40000, CRC(87b3e621) SHA1(8e2655c6e83d00c38210fdced25003793bd93d9f) )

	ROM_REGION( 0x100000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "gf_000.rom", 0x000000, 0x40000, CRC(209e8e8d) SHA1(9720be888905be709733c53da207c3406d73aeb1) )
	ROM_LOAD( "gf_010.rom", 0x040000, 0x40000, CRC(6e6e7808) SHA1(92c30eecf8f3669581720be6e49db87fbfac7d88) )
	ROM_LOAD( "gf_020.rom", 0x080000, 0x40000, CRC(6f5c3cb0) SHA1(e41572c267489e2078f8d5605c97abe2034a091a) )
	ROM_LOAD( "gf_030.rom", 0x0c0000, 0x40000, CRC(18978a9f) SHA1(aa484710a7c3561a9922f119a064f9205475ae64) )

	ROM_REGION( 0x20000, "irem", 0 )
	ROM_LOAD( "gf-da.rom", 0x000000, 0x020000, CRC(933ba935) SHA1(482811e01239feecf10e232566a7809d0d4f11b8) )
ROM_END

ROM_START( gunforceu )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gf_h0-d.5m", 0x000001, 0x20000, CRC(a6db7b5c) SHA1(5656473599e924ab799ea3c6f39d8ce34b08cb29) )
	ROM_LOAD16_BYTE( "gf_l0-d.5f", 0x000000, 0x20000, CRC(82cf55f6) SHA1(42a2de61f2c5294c81fb135ea2472cc78637c66c) )
	ROM_LOAD16_BYTE( "gf_h1-d.5l", 0x040001, 0x20000, CRC(08a3736c) SHA1(0ae904cf486a371f8b635c1f9dc5201e38a73f5a) )
	ROM_LOAD16_BYTE( "gf_l1-d.5j", 0x040000, 0x20000, CRC(435f524f) SHA1(65c282ec50123747880850bc32c7ace0471ed9f2) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "gf_sh0.rom", 0x00001, 0x10000, CRC(3f8f16e0) SHA1(a9f568c1b585c2cf13b21716954dac0a89936fc6) )
	ROM_LOAD16_BYTE( "gf_sl0.rom", 0x00000, 0x10000, CRC(db0b13a3) SHA1(6723026010610b706725a5284a7b8d70fe479dae) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "gf_c0.rom", 0x000000, 0x40000, CRC(b3b74979) SHA1(b5b4a4775e0b28c3f37273f93f69886c911af4aa) )
	ROM_LOAD( "gf_c1.rom", 0x040000, 0x40000, CRC(f5c8590a) SHA1(a7f90f23051f8ab2b2d925e950a5ef3c260170ca) )
	ROM_LOAD( "gf_c2.rom", 0x080000, 0x40000, CRC(30f9fb64) SHA1(f86e01b0d74a1f6c19d97d6d0e0f624f050dad10) )
	ROM_LOAD( "gf_c3.rom", 0x0c0000, 0x40000, CRC(87b3e621) SHA1(8e2655c6e83d00c38210fdced25003793bd93d9f) )

	ROM_REGION( 0x100000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "gf_000.rom", 0x000000, 0x40000, CRC(209e8e8d) SHA1(9720be888905be709733c53da207c3406d73aeb1) )
	ROM_LOAD( "gf_010.rom", 0x040000, 0x40000, CRC(6e6e7808) SHA1(92c30eecf8f3669581720be6e49db87fbfac7d88) )
	ROM_LOAD( "gf_020.rom", 0x080000, 0x40000, CRC(6f5c3cb0) SHA1(e41572c267489e2078f8d5605c97abe2034a091a) )
	ROM_LOAD( "gf_030.rom", 0x0c0000, 0x40000, CRC(18978a9f) SHA1(aa484710a7c3561a9922f119a064f9205475ae64) )

	ROM_REGION( 0x20000, "irem", 0 )
	ROM_LOAD( "gf-da.rom", 0x000000, 0x020000, CRC(933ba935) SHA1(482811e01239feecf10e232566a7809d0d4f11b8) )
ROM_END

ROM_START( inthunt )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ith-h0-d.bin", 0x000001, 0x040000, CRC(52f8e7a6) SHA1(26d9e272b01e7b82019812059dcc9fbb043c6129) )
	ROM_LOAD16_BYTE( "ith-l0-d.bin", 0x000000, 0x040000, CRC(5db79eb7) SHA1(ffd4228d7b88a44a82e639a5583753da183fcb23) )
	ROM_LOAD16_BYTE( "ith-h1-b.bin", 0x080001, 0x020000, CRC(fc2899df) SHA1(f811ff5fd55655afdb25950d317db85c8091b6d6) )
	ROM_LOAD16_BYTE( "ith-l1-b.bin", 0x080000, 0x020000, CRC(955a605a) SHA1(2515accc2f4a06b07418e45eb62e746d09c81720) )

	ROM_REGION( 0x20000, "soundcpu", 0 )    /* Irem D8000011A1 */
	ROM_LOAD16_BYTE( "ith-sh0.rom", 0x00001, 0x10000, CRC(209c8b7f) SHA1(eaf4a6d9222fe181df65cea1f13c3f2ebff2ec5b) )
	ROM_LOAD16_BYTE( "ith-sl0.rom", 0x00000, 0x10000, CRC(18472d65) SHA1(2705e94ee350ffda272c50ea3bf605826aa19978) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "ith_ic26.rom", 0x000000, 0x080000, CRC(4c1818cf) SHA1(fc8c2ae640bc3504a52736be46febb92c998fd7d) )
	ROM_LOAD( "ith_ic25.rom", 0x080000, 0x080000, CRC(91145bae) SHA1(71b2695575f189a2fc72635831ba408f824d4928) )
	ROM_LOAD( "ith_ic24.rom", 0x100000, 0x080000, CRC(fc03fe3b) SHA1(7e34220b9b21b82e012dcbf3052cccb118e3c382) )
	ROM_LOAD( "ith_ic23.rom", 0x180000, 0x080000, CRC(ee156a0a) SHA1(4a303ed292ce79e3f990139c35b921213eb2711d) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "ith_ic34.rom", 0x000000, 0x100000, CRC(a019766e) SHA1(59012a41d152a471a95f1f86b6b1e0f9dd3f9711) )
	ROM_LOAD( "ith_ic35.rom", 0x100000, 0x100000, CRC(3fca3073) SHA1(bdae171cb7705647f28354ca83ecdea3a15f6e22) )
	ROM_LOAD( "ith_ic36.rom", 0x200000, 0x100000, CRC(20d1b28b) SHA1(290947d77242e837444766ff5d420bc9b53b5b01) )
	ROM_LOAD( "ith_ic37.rom", 0x300000, 0x100000, CRC(90b6fd4b) SHA1(99237ebab7cf4689e06965bd546cd80a825ab024) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "ith_ic9.rom", 0x000000, 0x080000, CRC(318ee71a) SHA1(e6f49a7adf7155ba40c4f33a8fdc9553c00f5e3d) )
ROM_END

ROM_START( inthuntu )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ith-h0-c.bin", 0x000001, 0x040000, CRC(563dcec0) SHA1(0c7588ba603926fb0b490f2ba324ff73362a54d5) )
	ROM_LOAD16_BYTE( "ith-l0-c.bin", 0x000000, 0x040000, CRC(1638c705) SHA1(8ca7a12c2f75172d4c2c808ea666b2f2e969398c) )
	ROM_LOAD16_BYTE( "ith-h1-a.bin", 0x080001, 0x020000, CRC(0253065f) SHA1(a11e6bf014c19b2e317b75f01a7f0d7a9a85c7d3) )
	ROM_LOAD16_BYTE( "ith-l1-a.bin", 0x080000, 0x020000, CRC(a57d688d) SHA1(aa049de5c41097b6f1da31e9bf3bac132f67aa6c) )

	ROM_REGION( 0x20000, "soundcpu", 0 )    /* Irem D8000011A1 */
	ROM_LOAD16_BYTE( "ith-sh0.rom", 0x00001, 0x10000, CRC(209c8b7f) SHA1(eaf4a6d9222fe181df65cea1f13c3f2ebff2ec5b) )
	ROM_LOAD16_BYTE( "ith-sl0.rom", 0x00000, 0x10000, CRC(18472d65) SHA1(2705e94ee350ffda272c50ea3bf605826aa19978) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "ith_ic26.rom", 0x000000, 0x080000, CRC(4c1818cf) SHA1(fc8c2ae640bc3504a52736be46febb92c998fd7d) )
	ROM_LOAD( "ith_ic25.rom", 0x080000, 0x080000, CRC(91145bae) SHA1(71b2695575f189a2fc72635831ba408f824d4928) )
	ROM_LOAD( "ith_ic24.rom", 0x100000, 0x080000, CRC(fc03fe3b) SHA1(7e34220b9b21b82e012dcbf3052cccb118e3c382) )
	ROM_LOAD( "ith_ic23.rom", 0x180000, 0x080000, CRC(ee156a0a) SHA1(4a303ed292ce79e3f990139c35b921213eb2711d) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "ith_ic34.rom", 0x000000, 0x100000, CRC(a019766e) SHA1(59012a41d152a471a95f1f86b6b1e0f9dd3f9711) )
	ROM_LOAD( "ith_ic35.rom", 0x100000, 0x100000, CRC(3fca3073) SHA1(bdae171cb7705647f28354ca83ecdea3a15f6e22) )
	ROM_LOAD( "ith_ic36.rom", 0x200000, 0x100000, CRC(20d1b28b) SHA1(290947d77242e837444766ff5d420bc9b53b5b01) )
	ROM_LOAD( "ith_ic37.rom", 0x300000, 0x100000, CRC(90b6fd4b) SHA1(99237ebab7cf4689e06965bd546cd80a825ab024) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "ith_ic9.rom", 0x000000, 0x080000, CRC(318ee71a) SHA1(e6f49a7adf7155ba40c4f33a8fdc9553c00f5e3d) )
ROM_END

ROM_START( kaiteids )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ith-h0j.bin", 0x000001, 0x040000, CRC(dc1dec36) SHA1(f0a6e3be19752bffd9fd5f435405c8f591eab258) )
	ROM_LOAD16_BYTE( "ith-l0j.bin", 0x000000, 0x040000, CRC(8835d704) SHA1(42be25ccdc31824797a17e6f76bd06edfe853833) )
	ROM_LOAD16_BYTE( "ith-h1j.bin", 0x080001, 0x020000, CRC(5a7b212d) SHA1(50562d804a43aed7c34c19c8345782ac2f85caa7) )
	ROM_LOAD16_BYTE( "ith-l1j.bin", 0x080000, 0x020000, CRC(4c084494) SHA1(4f32003db32f13e19dd07c66996b4328ac2a671e) )

	ROM_REGION( 0x20000, "soundcpu", 0 )    /* Irem D8000011A1 */
	ROM_LOAD16_BYTE( "ith-sh0.rom", 0x00001, 0x10000, CRC(209c8b7f) SHA1(eaf4a6d9222fe181df65cea1f13c3f2ebff2ec5b) )
	ROM_LOAD16_BYTE( "ith-sl0.rom", 0x00000, 0x10000, CRC(18472d65) SHA1(2705e94ee350ffda272c50ea3bf605826aa19978) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "ith_ic26.rom", 0x000000, 0x080000, CRC(4c1818cf) SHA1(fc8c2ae640bc3504a52736be46febb92c998fd7d) )
	ROM_LOAD( "ith_ic25.rom", 0x080000, 0x080000, CRC(91145bae) SHA1(71b2695575f189a2fc72635831ba408f824d4928) )
	ROM_LOAD( "ith_ic24.rom", 0x100000, 0x080000, CRC(fc03fe3b) SHA1(7e34220b9b21b82e012dcbf3052cccb118e3c382) )
	ROM_LOAD( "ith_ic23.rom", 0x180000, 0x080000, CRC(ee156a0a) SHA1(4a303ed292ce79e3f990139c35b921213eb2711d) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "ith_ic34.rom", 0x000000, 0x100000, CRC(a019766e) SHA1(59012a41d152a471a95f1f86b6b1e0f9dd3f9711) )
	ROM_LOAD( "ith_ic35.rom", 0x100000, 0x100000, CRC(3fca3073) SHA1(bdae171cb7705647f28354ca83ecdea3a15f6e22) )
	ROM_LOAD( "ith_ic36.rom", 0x200000, 0x100000, CRC(20d1b28b) SHA1(290947d77242e837444766ff5d420bc9b53b5b01) )
	ROM_LOAD( "ith_ic37.rom", 0x300000, 0x100000, CRC(90b6fd4b) SHA1(99237ebab7cf4689e06965bd546cd80a825ab024) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "ith_ic9.rom", 0x000000, 0x080000, CRC(318ee71a) SHA1(e6f49a7adf7155ba40c4f33a8fdc9553c00f5e3d) )
ROM_END

ROM_START( hook )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "h-h0-d.3h", 0x000001, 0x040000, CRC(40189ff6) SHA1(ed86a566f0f47c03dd0628cda8b31a167788116c) )
	ROM_LOAD16_BYTE( "h-l0-d.3h", 0x000000, 0x040000, CRC(14567690) SHA1(74ddc300e81b006fdc57a4a86f5f178a30732dd8) )
	ROM_LOAD16_BYTE( "h-h1.rom",  0x080001, 0x020000, CRC(264ba1f0) SHA1(49ecf9b3e5375629607fb747abe264406065580b) )
	ROM_LOAD16_BYTE( "h-l1.rom",  0x080000, 0x020000, CRC(f9913731) SHA1(be7871d6843e76f66fae6b501c5ee83ccc366463) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "h-sh0.rom", 0x00001, 0x10000, CRC(86a4e56e) SHA1(61163010e713be64368a4126f17d33cbdcf0c5ed) )
	ROM_LOAD16_BYTE( "h-sl0.rom", 0x00000, 0x10000, CRC(10fd9676) SHA1(1b51181a8f0711997e107e9a8b8f44341d08ea81) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "hook-c0.rom", 0x000000, 0x040000, CRC(dec63dcf) SHA1(e9869110f832d782c460b123928b042c65fdf8bd) )
	ROM_LOAD( "hook-c1.rom", 0x040000, 0x040000, CRC(e4eb0b92) SHA1(159da3ec973490a153c69c96c1373cf4e0290736) )
	ROM_LOAD( "hook-c2.rom", 0x080000, 0x040000, CRC(a52b320b) SHA1(1522562239bb3b93ef552c47445daa4ee021495c) )
	ROM_LOAD( "hook-c3.rom", 0x0c0000, 0x040000, CRC(7ef67731) SHA1(af0b0ee6e1c06af04c609af7e077d4a7d76d8817) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "hook-000.rom", 0x000000, 0x100000, CRC(ccceac30) SHA1(16e2b4393840344debe869034135feead7450184) )
	ROM_LOAD( "hook-010.rom", 0x100000, 0x100000, CRC(8ac8da67) SHA1(a9b962cb0bc0d8bc3bda8a0ed1ce06641d666b41) )
	ROM_LOAD( "hook-020.rom", 0x200000, 0x100000, CRC(8847af9a) SHA1(f82cdbd640fac373136219422172ca9fbf5d1830) )
	ROM_LOAD( "hook-030.rom", 0x300000, 0x100000, CRC(239e877e) SHA1(445e1096619c4e3a2d5b50a645fd45bd7c501590) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "hook-da.rom", 0x000000, 0x080000, CRC(88cd0212) SHA1(789532f5544b5d024d8af60eb8a5c133ae0d19d4) )
ROM_END

ROM_START( hooku )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "h-h0-c.3h", 0x000001, 0x040000, CRC(84cc239e) SHA1(0a3011cd64cd27336b967b1b2446c8916b8be8e7) )
	ROM_LOAD16_BYTE( "h-l0-c.5h", 0x000000, 0x040000, CRC(45e194fe) SHA1(2049f242ea3058d42004bafb8b208759020be5bc) )
	ROM_LOAD16_BYTE( "h-h1.rom",  0x080001, 0x020000, CRC(264ba1f0) SHA1(49ecf9b3e5375629607fb747abe264406065580b) )
	ROM_LOAD16_BYTE( "h-l1.rom",  0x080000, 0x020000, CRC(f9913731) SHA1(be7871d6843e76f66fae6b501c5ee83ccc366463) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "h-sh0.rom", 0x00001, 0x10000, CRC(86a4e56e) SHA1(61163010e713be64368a4126f17d33cbdcf0c5ed) )
	ROM_LOAD16_BYTE( "h-sl0.rom", 0x00000, 0x10000, CRC(10fd9676) SHA1(1b51181a8f0711997e107e9a8b8f44341d08ea81) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "hook-c0.rom", 0x000000, 0x040000, CRC(dec63dcf) SHA1(e9869110f832d782c460b123928b042c65fdf8bd) )
	ROM_LOAD( "hook-c1.rom", 0x040000, 0x040000, CRC(e4eb0b92) SHA1(159da3ec973490a153c69c96c1373cf4e0290736) )
	ROM_LOAD( "hook-c2.rom", 0x080000, 0x040000, CRC(a52b320b) SHA1(1522562239bb3b93ef552c47445daa4ee021495c) )
	ROM_LOAD( "hook-c3.rom", 0x0c0000, 0x040000, CRC(7ef67731) SHA1(af0b0ee6e1c06af04c609af7e077d4a7d76d8817) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "hook-000.rom", 0x000000, 0x100000, CRC(ccceac30) SHA1(16e2b4393840344debe869034135feead7450184) )
	ROM_LOAD( "hook-010.rom", 0x100000, 0x100000, CRC(8ac8da67) SHA1(a9b962cb0bc0d8bc3bda8a0ed1ce06641d666b41) )
	ROM_LOAD( "hook-020.rom", 0x200000, 0x100000, CRC(8847af9a) SHA1(f82cdbd640fac373136219422172ca9fbf5d1830) )
	ROM_LOAD( "hook-030.rom", 0x300000, 0x100000, CRC(239e877e) SHA1(445e1096619c4e3a2d5b50a645fd45bd7c501590) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "hook-da.rom", 0x000000, 0x080000, CRC(88cd0212) SHA1(789532f5544b5d024d8af60eb8a5c133ae0d19d4) )
ROM_END

ROM_START( hookj )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "h-h0-g.3h", 0x000001, 0x040000, CRC(5964c886) SHA1(fe15f328d0e62b6be09c8ae9892f5b669585fcdb) )
	ROM_LOAD16_BYTE( "h-l0-g.5h", 0x000000, 0x040000, CRC(7f7433f2) SHA1(e85c170332ed7195e713fd5a2a20c97d56a7297b) )
	ROM_LOAD16_BYTE( "h-h1.rom",  0x080001, 0x020000, CRC(264ba1f0) SHA1(49ecf9b3e5375629607fb747abe264406065580b) )
	ROM_LOAD16_BYTE( "h-l1.rom",  0x080000, 0x020000, CRC(f9913731) SHA1(be7871d6843e76f66fae6b501c5ee83ccc366463) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "h-sh0-a.3l", 0x00001, 0x10000, CRC(bd3d1f61) SHA1(0c884a0b5519f9c0823128872baf7b0c4078e5c4) )
	ROM_LOAD16_BYTE( "h-sl0-a.3n", 0x00000, 0x10000, CRC(76371def) SHA1(b7a86fd4eecdd8a538c32e08cd920c27bd50924b) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "hook-c0.rom", 0x000000, 0x040000, CRC(dec63dcf) SHA1(e9869110f832d782c460b123928b042c65fdf8bd) )
	ROM_LOAD( "hook-c1.rom", 0x040000, 0x040000, CRC(e4eb0b92) SHA1(159da3ec973490a153c69c96c1373cf4e0290736) )
	ROM_LOAD( "hook-c2.rom", 0x080000, 0x040000, CRC(a52b320b) SHA1(1522562239bb3b93ef552c47445daa4ee021495c) )
	ROM_LOAD( "hook-c3.rom", 0x0c0000, 0x040000, CRC(7ef67731) SHA1(af0b0ee6e1c06af04c609af7e077d4a7d76d8817) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "hook-000.rom", 0x000000, 0x100000, CRC(ccceac30) SHA1(16e2b4393840344debe869034135feead7450184) )
	ROM_LOAD( "hook-010.rom", 0x100000, 0x100000, CRC(8ac8da67) SHA1(a9b962cb0bc0d8bc3bda8a0ed1ce06641d666b41) )
	ROM_LOAD( "hook-020.rom", 0x200000, 0x100000, CRC(8847af9a) SHA1(f82cdbd640fac373136219422172ca9fbf5d1830) )
	ROM_LOAD( "hook-030.rom", 0x300000, 0x100000, CRC(239e877e) SHA1(445e1096619c4e3a2d5b50a645fd45bd7c501590) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "hook-da.rom", 0x000000, 0x080000, CRC(88cd0212) SHA1(789532f5544b5d024d8af60eb8a5c133ae0d19d4) )
ROM_END

ROM_START( ppan )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1.u6", 0x000001, 0x080000, CRC(b135dd6e) SHA1(3e7ac75db53804c605fb628546f5a506ba7f7a5f) )
	ROM_LOAD16_BYTE( "2.u5", 0x000000, 0x080000, CRC(7785289c) SHA1(8125c4ae8e99b6eed5216c1d956426bf2034ada0) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "7.u114", 0x000000, 0x040000, CRC(dec63dcf) SHA1(e9869110f832d782c460b123928b042c65fdf8bd) )
	ROM_LOAD( "6.u115", 0x040000, 0x040000, CRC(e4eb0b92) SHA1(159da3ec973490a153c69c96c1373cf4e0290736) )
	ROM_LOAD( "5.u116", 0x080000, 0x040000, CRC(a52b320b) SHA1(1522562239bb3b93ef552c47445daa4ee021495c) )
	ROM_LOAD( "4.u117", 0x0c0000, 0x040000, CRC(7ef67731) SHA1(af0b0ee6e1c06af04c609af7e077d4a7d76d8817) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "15.u106", 0x000000, 0x080000, CRC(cdfc2f78) SHA1(02981c5b48afe532a74c9aa72ebdaaaca7a091e5) )
	ROM_LOAD( "14.u110", 0x080000, 0x080000, CRC(87e767f0) SHA1(ddf3c5a04c8fc1551bddb7e7753972a80442b88b) )
	ROM_LOAD( "13.u107", 0x100000, 0x080000, CRC(e07f2abe) SHA1(1b404fcf6bcc1a25e510c95a9eb83df0c780934a) )
	ROM_LOAD( "12.u111", 0x180000, 0x080000, CRC(f446150e) SHA1(1bb964c9060906d0d9f2cdb465b20e04827e9b86) )
	ROM_LOAD( "11.u108", 0x200000, 0x080000, CRC(5c114daa) SHA1(6dd28b3e9f82aa9370986e137453ef8d4c641483) )
	ROM_LOAD( "10.u112", 0x280000, 0x080000, CRC(fa11fa40) SHA1(63092a0df1f8e52c3caad196aada57fb8f3c3629) )
	ROM_LOAD( "9.u109",  0x300000, 0x080000, CRC(9d466b1a) SHA1(c65b7afcfbd6bfec1b495a5dbce806ff34a7cbc1) )
	ROM_LOAD( "8.u113",  0x380000, 0x080000, CRC(d08a5f6b) SHA1(ab762be9e5fadac2dc3149bfa69b8cbdbac3218b) )

	ROM_REGION( 0x080000, "okidata", 0 ) /* OKI M6295 samples */
	ROM_LOAD( "3.u122",  0x000000, 0x080000, CRC(d0d37028) SHA1(0f58d220a1972bafa1299a19e704b7735886c8b6) )

	ROM_REGION( 0x100000, "oki", 0) /* OKI Samples copied here */
	ROM_COPY( "okidata",  0x000000, 0x000000, 0x20000 )
	ROM_COPY( "okidata",  0x000000, 0x020000, 0x20000 )
	ROM_COPY( "okidata",  0x000000, 0x040000, 0x20000 )
	ROM_COPY( "okidata",  0x020000, 0x060000, 0x20000 )
	ROM_COPY( "okidata",  0x000000, 0x080000, 0x20000 )
	ROM_COPY( "okidata",  0x040000, 0x0a0000, 0x20000 )
	ROM_COPY( "okidata",  0x000000, 0x0c0000, 0x20000 )
	ROM_COPY( "okidata",  0x060000, 0x0e0000, 0x20000 )
ROM_END

ROM_START( rtypeleo )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rtl-h0-c.bin", 0x000001, 0x040000, CRC(5fef7fa1) SHA1(7d18d4ea979d887d6da42c79734b8c695f3df02b) )
	ROM_LOAD16_BYTE( "rtl-l0-c.bin", 0x000000, 0x040000, CRC(8156456b) SHA1(9755ab80feb92e3e3a36154d57ee2b53185b6816) )
	ROM_LOAD16_BYTE( "rtl-h1-d.bin", 0x080001, 0x020000, CRC(352ff444) SHA1(e302bc8dbf80abe5c1aaf02e92473fc72a796e72) )
	ROM_LOAD16_BYTE( "rtl-l1-d.bin", 0x080000, 0x020000, CRC(fd34ea46) SHA1(aca12d46ebff94505d03884e45805e84bbece6a7) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "rtl-sh0a.bin", 0x00001, 0x10000, CRC(e518b4e3) SHA1(44ec1d6b27bc3e49ad967f43960398ba1a19c5e3) )
	ROM_LOAD16_BYTE( "rtl-sl0a.bin", 0x00000, 0x10000, CRC(896f0d36) SHA1(9246b1a5a8717dd823340d4cb79012a3df6fa4b7) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "rtl-c0.bin", 0x000000, 0x080000, CRC(fb588d7c) SHA1(78e96db9912b22f8eff03d57e470b1ef946f7351) )
	ROM_LOAD( "rtl-c1.bin", 0x080000, 0x080000, CRC(e5541bff) SHA1(cd8293603298b7ead79a16697845603223bb6a45) )
	ROM_LOAD( "rtl-c2.bin", 0x100000, 0x080000, CRC(faa9ae27) SHA1(de6c7f1843adcaa9fce0d0d9407999babbf52e27) )
	ROM_LOAD( "rtl-c3.bin", 0x180000, 0x080000, CRC(3a2343f6) SHA1(dea1af889d6a422af3f49abf2cee91aec4d0cac3) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "rtl-000.bin", 0x000000, 0x100000, CRC(82a06870) SHA1(c7233019c4dcfcab55b665a7b0973e74cca879cd) )
	ROM_LOAD( "rtl-010.bin", 0x100000, 0x100000, CRC(417e7a56) SHA1(d33a40eb7ec0afde0a59799a428aadee12dd5c63) )
	ROM_LOAD( "rtl-020.bin", 0x200000, 0x100000, CRC(f9a3f3a1) SHA1(b4eb9326ff992e62b70925277fbbd3ea2eabf359) )
	ROM_LOAD( "rtl-030.bin", 0x300000, 0x100000, CRC(03528d95) SHA1(f2705646ee8d9e7b7f70cfd2c31b6e32798f459d) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "rtl-da.bin", 0x000000, 0x080000, CRC(dbebd1ff) SHA1(b369d6e944331e6773608ff24f04b8f16267b8da) )
ROM_END

ROM_START( rtypeleoj )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rtl-h0-d.bin", 0x000001, 0x040000, CRC(3dbac89f) SHA1(bfb4d1ab480b7828f6b7374df6d30d766f327b95) )
	ROM_LOAD16_BYTE( "rtl-l0-d.bin", 0x000000, 0x040000, CRC(f85a2537) SHA1(50eeca8de0c7fd28375d082a05f18473d0b15ed4) )
	ROM_LOAD16_BYTE( "rtl-h1-d.bin", 0x080001, 0x020000, CRC(352ff444) SHA1(e302bc8dbf80abe5c1aaf02e92473fc72a796e72) )
	ROM_LOAD16_BYTE( "rtl-l1-d.bin", 0x080000, 0x020000, CRC(fd34ea46) SHA1(aca12d46ebff94505d03884e45805e84bbece6a7) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "rtl-sh0a.bin", 0x00001, 0x10000, CRC(e518b4e3) SHA1(44ec1d6b27bc3e49ad967f43960398ba1a19c5e3) )
	ROM_LOAD16_BYTE( "rtl-sl0a.bin", 0x00000, 0x10000, CRC(896f0d36) SHA1(9246b1a5a8717dd823340d4cb79012a3df6fa4b7) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "rtl-c0.bin", 0x000000, 0x080000, CRC(fb588d7c) SHA1(78e96db9912b22f8eff03d57e470b1ef946f7351) )
	ROM_LOAD( "rtl-c1.bin", 0x080000, 0x080000, CRC(e5541bff) SHA1(cd8293603298b7ead79a16697845603223bb6a45) )
	ROM_LOAD( "rtl-c2.bin", 0x100000, 0x080000, CRC(faa9ae27) SHA1(de6c7f1843adcaa9fce0d0d9407999babbf52e27) )
	ROM_LOAD( "rtl-c3.bin", 0x180000, 0x080000, CRC(3a2343f6) SHA1(dea1af889d6a422af3f49abf2cee91aec4d0cac3) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "rtl-000.bin", 0x000000, 0x100000, CRC(82a06870) SHA1(c7233019c4dcfcab55b665a7b0973e74cca879cd) )
	ROM_LOAD( "rtl-010.bin", 0x100000, 0x100000, CRC(417e7a56) SHA1(d33a40eb7ec0afde0a59799a428aadee12dd5c63) )
	ROM_LOAD( "rtl-020.bin", 0x200000, 0x100000, CRC(f9a3f3a1) SHA1(b4eb9326ff992e62b70925277fbbd3ea2eabf359) )
	ROM_LOAD( "rtl-030.bin", 0x300000, 0x100000, CRC(03528d95) SHA1(f2705646ee8d9e7b7f70cfd2c31b6e32798f459d) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "rtl-da.bin", 0x000000, 0x080000, CRC(dbebd1ff) SHA1(b369d6e944331e6773608ff24f04b8f16267b8da) )
ROM_END

ROM_START( mysticri )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mr-h0-b.bin", 0x000001, 0x040000, CRC(d529f887) SHA1(fedfedd23fdbb8c4a19970dc4e2c8c0f96915982) )
	ROM_LOAD16_BYTE( "mr-l0-b.bin", 0x000000, 0x040000, CRC(a457ab44) SHA1(6f85428061cf384c6d645ff0aacd850730a86987) )
	ROM_LOAD16_BYTE( "mr-h1-b.bin", 0x080001, 0x010000, CRC(e17649b9) SHA1(fb09a0ccd22475d81ba667c88d1b5eb7cc64728f) )
	ROM_LOAD16_BYTE( "mr-l1-b.bin", 0x080000, 0x010000, CRC(a87c62b4) SHA1(d3cae0f420faeb4556767b6ad817fc39d31b7273) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "mr-sh0.bin", 0x00001, 0x10000, CRC(50d335e4) SHA1(a1a92e95fbd6b99d904a82cea4a1ff6fd2ac8dde) )
	ROM_LOAD16_BYTE( "mr-sl0.bin", 0x00000, 0x10000, CRC(0fa32721) SHA1(1561ddd2597592060b8a78f1dff6cbb25fb7cd2e) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "mr-c0.bin", 0x000000, 0x040000, CRC(872a8fad) SHA1(236406e5959c81a1cffe96fef02d637c2150ce1e) )
	ROM_LOAD( "mr-c1.bin", 0x040000, 0x040000, CRC(d2ffb27a) SHA1(fedfb430ce8a8953b2f78970d0b0dc5571de333c) )
	ROM_LOAD( "mr-c2.bin", 0x080000, 0x040000, CRC(62bff287) SHA1(cb7b73c4a26737f1a1f9cc9423ae51c284368b1b) )
	ROM_LOAD( "mr-c3.bin", 0x0c0000, 0x040000, CRC(d0da62ab) SHA1(96c7c8e1d8dafb797731652fa91d3048aa157185) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "mr-o00.bin", 0x000000, 0x080000, CRC(a0f9ce16) SHA1(ae423313d189ebddc6d5d0785ac484e0cdf79112) )
	ROM_LOAD( "mr-o10.bin", 0x100000, 0x080000, CRC(4e70a9e9) SHA1(8f6b043b03420a590a1081c99311723169126332) )
	ROM_LOAD( "mr-o20.bin", 0x200000, 0x080000, CRC(b9c468fc) SHA1(dc42a5b80cad5373fce03cc416b9d742fcbec6e9) )
	ROM_LOAD( "mr-o30.bin", 0x300000, 0x080000, CRC(cc32433a) SHA1(a1a1ab09c4bd6c9ae85529c1aa5427ad3126b914) )

	ROM_REGION( 0x40000, "irem", 0 )
	ROM_LOAD( "mr-da.bin", 0x000000, 0x040000, CRC(1a11fc59) SHA1(6d1f4ca688bf015ecbbe369fbc0eb5e2bcaefcfc) )
ROM_END

ROM_START( gunhohki )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mr-h0.bin", 0x000001, 0x040000, CRC(83352270) SHA1(25393ac0ec0f91c2890bbfc8c1b12e0f6bccb2ab) )
	ROM_LOAD16_BYTE( "mr-l0.bin", 0x000000, 0x040000, CRC(9db308ae) SHA1(eadec2e07a602d104a38bf9e159865405ab11581) )
	ROM_LOAD16_BYTE( "mr-h1.bin", 0x080001, 0x010000, CRC(c9532b60) SHA1(b83322ba7bb3eea4c64dd65b3c0a5cade61841d8) )
	ROM_LOAD16_BYTE( "mr-l1.bin", 0x080000, 0x010000, CRC(6349b520) SHA1(406620d9c63ce3d6801105c8122e1d0bbe6152ad) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "mr-sh0.bin", 0x00001, 0x10000, CRC(50d335e4) SHA1(a1a92e95fbd6b99d904a82cea4a1ff6fd2ac8dde) )
	ROM_LOAD16_BYTE( "mr-sl0.bin", 0x00000, 0x10000, CRC(0fa32721) SHA1(1561ddd2597592060b8a78f1dff6cbb25fb7cd2e) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "mr-c0.bin", 0x000000, 0x040000, CRC(872a8fad) SHA1(236406e5959c81a1cffe96fef02d637c2150ce1e) )
	ROM_LOAD( "mr-c1.bin", 0x040000, 0x040000, CRC(d2ffb27a) SHA1(fedfb430ce8a8953b2f78970d0b0dc5571de333c) )
	ROM_LOAD( "mr-c2.bin", 0x080000, 0x040000, CRC(62bff287) SHA1(cb7b73c4a26737f1a1f9cc9423ae51c284368b1b) )
	ROM_LOAD( "mr-c3.bin", 0x0c0000, 0x040000, CRC(d0da62ab) SHA1(96c7c8e1d8dafb797731652fa91d3048aa157185) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "mr-o00.bin", 0x000000, 0x080000, CRC(a0f9ce16) SHA1(ae423313d189ebddc6d5d0785ac484e0cdf79112) )
	ROM_LOAD( "mr-o10.bin", 0x100000, 0x080000, CRC(4e70a9e9) SHA1(8f6b043b03420a590a1081c99311723169126332) )
	ROM_LOAD( "mr-o20.bin", 0x200000, 0x080000, CRC(b9c468fc) SHA1(dc42a5b80cad5373fce03cc416b9d742fcbec6e9) )
	ROM_LOAD( "mr-o30.bin", 0x300000, 0x080000, CRC(cc32433a) SHA1(a1a1ab09c4bd6c9ae85529c1aa5427ad3126b914) )

	ROM_REGION( 0x40000, "irem", 0 )
	ROM_LOAD( "mr-da.bin", 0x000000, 0x040000, CRC(1a11fc59) SHA1(6d1f4ca688bf015ecbbe369fbc0eb5e2bcaefcfc) )
ROM_END

ROM_START( mysticrib )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "h0", 0x000001, 0x040000, CRC(e38c1f56) SHA1(491f370c66c36ab56a4bee3f335fe7357ff5668b) )
	ROM_LOAD16_BYTE( "l0", 0x000000, 0x040000, CRC(77846e48) SHA1(c715136c4ed8dda24ec1ed634e6308d23c92ec05) )
	ROM_LOAD16_BYTE( "h1", 0x080001, 0x010000, CRC(4dcb085b) SHA1(7c053f5ef2978e574d3d2d9f5c12035473d13c3b) )
	ROM_LOAD16_BYTE( "l1", 0x080000, 0x010000, CRC(88df4f70) SHA1(f55769a107fe3f5446d8268f66e895b02727c61e) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	// older revision code? rev 3.31, doesn't work?
	ROM_LOAD16_BYTE( "sh0", 0x00001, 0x10000, CRC(fc7221ee) SHA1(4e714f31ce0d1bb2f6c649a26af748f96912848e) )
	ROM_LOAD16_BYTE( "sl0", 0x00000, 0x10000, CRC(65c809e6) SHA1(45a860b250219a15aa8a2177251f4d3f2e559b9e) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "mr-c0.bin", 0x000000, 0x040000, CRC(872a8fad) SHA1(236406e5959c81a1cffe96fef02d637c2150ce1e) )
	ROM_LOAD( "mr-c1.bin", 0x040000, 0x040000, CRC(d2ffb27a) SHA1(fedfb430ce8a8953b2f78970d0b0dc5571de333c) )
	ROM_LOAD( "mr-c2.bin", 0x080000, 0x040000, CRC(62bff287) SHA1(cb7b73c4a26737f1a1f9cc9423ae51c284368b1b) )
	ROM_LOAD( "mr-c3.bin", 0x0c0000, 0x040000, CRC(d0da62ab) SHA1(96c7c8e1d8dafb797731652fa91d3048aa157185) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "mr-o00.bin", 0x000000, 0x080000, CRC(a0f9ce16) SHA1(ae423313d189ebddc6d5d0785ac484e0cdf79112) )
	ROM_LOAD( "mr-o10.bin", 0x100000, 0x080000, CRC(4e70a9e9) SHA1(8f6b043b03420a590a1081c99311723169126332) )
	ROM_LOAD( "mr-o20.bin", 0x200000, 0x080000, CRC(b9c468fc) SHA1(dc42a5b80cad5373fce03cc416b9d742fcbec6e9) )
	ROM_LOAD( "mr-o30.bin", 0x300000, 0x080000, CRC(cc32433a) SHA1(a1a1ab09c4bd6c9ae85529c1aa5427ad3126b914) )

	ROM_REGION( 0x40000, "irem", 0 )
	ROM_LOAD( "mr-da.bin", 0x000000, 0x040000, CRC(1a11fc59) SHA1(6d1f4ca688bf015ecbbe369fbc0eb5e2bcaefcfc) )
ROM_END


ROM_START( uccops )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "uc_h0.rom", 0x000001, 0x040000, CRC(240aa5f7) SHA1(8d864bb1377e9f6d266631ed365c5809b9da33f8) )
	ROM_LOAD16_BYTE( "uc_l0.rom", 0x000000, 0x040000, CRC(df9a4826) SHA1(298033d97b9587e3548cb3bffa16b7ba9a6ff20d) )
	ROM_LOAD16_BYTE( "uc_h1.rom", 0x080001, 0x020000, CRC(8d29bcd6) SHA1(470b77d1b8f88824bac294bd12a205a23dad2287) )
	ROM_LOAD16_BYTE( "uc_l1.rom", 0x080000, 0x020000, CRC(a8a402d8) SHA1(0b40fb69f0a3e24e6b60117d2d2fd4cc170bc621) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "uc_sh0.rom", 0x00001, 0x10000, CRC(df90b198) SHA1(6b334457f06f6b9cfb355ba3d399bebb37b5733e) )
	ROM_LOAD16_BYTE( "uc_sl0.rom", 0x00000, 0x10000, CRC(96c11aac) SHA1(16c47b4f97f0532fff30bb163f26d8cf6b923a2e) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "uc_w38m.rom", 0x000000, 0x080000, CRC(130a40e5) SHA1(f70bad2fe126bb0e451a3fa6100a610928e9a502) )
	ROM_LOAD( "uc_w39m.rom", 0x080000, 0x080000, CRC(e42ca144) SHA1(ea83b1027d403e874fda6e68097814f8b9ce25d6) )
	ROM_LOAD( "uc_w40m.rom", 0x100000, 0x080000, CRC(c2961648) SHA1(b5d28638e72ab50d598e284f31bf389956ae12c6) )
	ROM_LOAD( "uc_w41m.rom", 0x180000, 0x080000, CRC(f5334b80) SHA1(6fa70ceba4f67fb0562be7b24b28bda0ffc13ef5) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "uc_k16m.rom", 0x000000, 0x100000, CRC(4a225f09) SHA1(f4d27813241fd9b020e4df8b03c852c8ecb92586) )
	ROM_LOAD( "uc_k17m.rom", 0x100000, 0x100000, CRC(e4ed9a54) SHA1(55befbd2e156c765c5e79a3176cf4336d2111293) )
	ROM_LOAD( "uc_k18m.rom", 0x200000, 0x100000, CRC(a626eb12) SHA1(826c4796c2e63f777490b43f84ffa37a6b749ca2) )
	ROM_LOAD( "uc_k19m.rom", 0x300000, 0x100000, CRC(5df46549) SHA1(87b0b799b50bf2b6ee916d9f8dfc1ee7666ce800) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "uc_w42.rom", 0x000000, 0x080000, CRC(d17d3fd6) SHA1(b02da0d01c41c7bf50cd35d6c75bacc3e3e0b85a) )
ROM_END

ROM_START( uccopsu )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "uc_h0.rom",   0x000001, 0x040000, CRC(240aa5f7) SHA1(8d864bb1377e9f6d266631ed365c5809b9da33f8) )
	ROM_LOAD16_BYTE( "uc_l0.rom",   0x000000, 0x040000, CRC(df9a4826) SHA1(298033d97b9587e3548cb3bffa16b7ba9a6ff20d) )
	ROM_LOAD16_BYTE( "uc_h1-g.rom", 0x080001, 0x020000, CRC(6b8ca2de) SHA1(1096b93bbaa4c97d4900e8c083cde99195cad5ba) )
	ROM_LOAD16_BYTE( "uc_l1-g.rom", 0x080000, 0x020000, CRC(2bdec7dd) SHA1(58817099e74fd5bc299b7bc14d83ee75ed200b53) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "uc_sh0.rom", 0x00001, 0x10000, CRC(df90b198) SHA1(6b334457f06f6b9cfb355ba3d399bebb37b5733e) )
	ROM_LOAD16_BYTE( "uc_sl0.rom", 0x00000, 0x10000, CRC(96c11aac) SHA1(16c47b4f97f0532fff30bb163f26d8cf6b923a2e) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "uc_w38m.rom", 0x000000, 0x080000, CRC(130a40e5) SHA1(f70bad2fe126bb0e451a3fa6100a610928e9a502) )
	ROM_LOAD( "uc_w39m.rom", 0x080000, 0x080000, CRC(e42ca144) SHA1(ea83b1027d403e874fda6e68097814f8b9ce25d6) )
	ROM_LOAD( "uc_w40m.rom", 0x100000, 0x080000, CRC(c2961648) SHA1(b5d28638e72ab50d598e284f31bf389956ae12c6) )
	ROM_LOAD( "uc_w41m.rom", 0x180000, 0x080000, CRC(f5334b80) SHA1(6fa70ceba4f67fb0562be7b24b28bda0ffc13ef5) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "uc_k16m.rom", 0x000000, 0x100000, CRC(4a225f09) SHA1(f4d27813241fd9b020e4df8b03c852c8ecb92586) )
	ROM_LOAD( "uc_k17m.rom", 0x100000, 0x100000, CRC(e4ed9a54) SHA1(55befbd2e156c765c5e79a3176cf4336d2111293) )
	ROM_LOAD( "uc_k18m.rom", 0x200000, 0x100000, CRC(a626eb12) SHA1(826c4796c2e63f777490b43f84ffa37a6b749ca2) )
	ROM_LOAD( "uc_k19m.rom", 0x300000, 0x100000, CRC(5df46549) SHA1(87b0b799b50bf2b6ee916d9f8dfc1ee7666ce800) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "uc_w42.rom", 0x000000, 0x080000, CRC(d17d3fd6) SHA1(b02da0d01c41c7bf50cd35d6c75bacc3e3e0b85a) )
ROM_END


/*
Undercover Cops Alpha Renewal Version
Irem, 1992

An alt. version, runs on standard
M92 main board:  M92-A-B 05C04170B1

ROM board:  M92-E-B 05C04238B1
Chips used are...
Nanao 08J27504A1
Nanao 08J27291A5  @ 14.31818MHz
*/

ROM_START( uccopsar ) /* Alpha Renewal Version */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "uc_h0_a.ic28", 0x000001, 0x040000, CRC(9e17cada) SHA1(086bb9c1ab851cab3734c2f9188d8ff3c5f98913) )
	ROM_LOAD16_BYTE( "uc_l0_a.ic39", 0x000000, 0x040000, CRC(4a4e3208) SHA1(d61c74d46584e2c15e70f7a17b598e51981da9e8) )
	ROM_LOAD16_BYTE( "uc_h1.ic27",   0x080001, 0x020000, CRC(79d79742) SHA1(f9c03c4d42b5b3d0f0185462868b04f1bb679f90) )
	ROM_LOAD16_BYTE( "uc_l1.ic38",   0x080000, 0x020000, CRC(37211581) SHA1(b8fdff96b2c7d5cf2975dcf81c00581ccb595c15) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "uc_sh0.ic30", 0x00001, 0x10000, CRC(f0ca1b03) SHA1(07154a2c747091f8be23587c109d91ed1672da6e) )
	ROM_LOAD16_BYTE( "uc_sl0.ic31", 0x00000, 0x10000, CRC(d1661723) SHA1(bdc00196aa2074e7b21e5949f73e9f2b93d76fd9) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "uc_c0.ic26", 0x000000, 0x080000, CRC(6a419a36) SHA1(1907d15fcc4a8bf875d19768667ee4de4702cc2a) )
	ROM_LOAD( "uc_c1.ic25", 0x080000, 0x080000, CRC(d703ecc7) SHA1(9716a8fde668e63cf3060450eb32ea43edf143d8) )
	ROM_LOAD( "uc_c2.ic24", 0x100000, 0x080000, CRC(96397ac6) SHA1(6dfe507bd9f41b5d46d85ef5f46a368745593b52) )
	ROM_LOAD( "uc_c3.ic23", 0x180000, 0x080000, CRC(5d07d10d) SHA1(ee1a928b37043c476346f189f75d2bfcc44bffe6) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "uc_030.ic37", 0x000000, 0x100000, CRC(97f7775e) SHA1(5cd147fd940b1ab6eba8e6c6f803bdcc5da5a563) )
	ROM_LOAD( "uc_020.ic36", 0x100000, 0x100000, CRC(5e0b1d65) SHA1(9e45753d10b2d7b580cd11cef74181209a424189) )
	ROM_LOAD( "uc_010.ic35", 0x200000, 0x100000, CRC(bdc224b3) SHA1(09477ec39890d954fac6ff653b9f46c9adea56b6) )
	ROM_LOAD( "uc_000.ic34", 0x300000, 0x100000, CRC(7526daec) SHA1(79431d711deb6ed09dc52be753b7b0f2c5588dc3) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "uc_da.bin", 0x000000, 0x080000, CRC(0b2855e9) SHA1(70f9decd78eab679a2ccad69e01cb303b61e0d38) )
ROM_END

ROM_START( uccopsj )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "uc_h0_a.ic28", 0x000001, 0x040000, CRC(9e17cada) SHA1(086bb9c1ab851cab3734c2f9188d8ff3c5f98913) )
	ROM_LOAD16_BYTE( "uc_l0_a.ic39", 0x000000, 0x040000, CRC(4a4e3208) SHA1(d61c74d46584e2c15e70f7a17b598e51981da9e8) )
	ROM_LOAD16_BYTE( "uc_h1_a.ic27", 0x080001, 0x020000, CRC(83f78dea) SHA1(6d197c3ea76beac31c3ea6e54a3ffea9d6c0c653) )
	ROM_LOAD16_BYTE( "uc_l1_a.ic38", 0x080000, 0x020000, CRC(19628280) SHA1(e6c06cb7c37e46a7db3b4f318e836aa5a2390eda) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "uc_sh0.ic30", 0x00001, 0x10000, CRC(f0ca1b03) SHA1(07154a2c747091f8be23587c109d91ed1672da6e) )
	ROM_LOAD16_BYTE( "uc_sl0.ic31", 0x00000, 0x10000, CRC(d1661723) SHA1(bdc00196aa2074e7b21e5949f73e9f2b93d76fd9) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "uc_c0.ic26", 0x000000, 0x080000, CRC(6a419a36) SHA1(1907d15fcc4a8bf875d19768667ee4de4702cc2a) )
	ROM_LOAD( "uc_c1.ic25", 0x080000, 0x080000, CRC(d703ecc7) SHA1(9716a8fde668e63cf3060450eb32ea43edf143d8) )
	ROM_LOAD( "uc_c2.ic24", 0x100000, 0x080000, CRC(96397ac6) SHA1(6dfe507bd9f41b5d46d85ef5f46a368745593b52) )
	ROM_LOAD( "uc_c3.ic23", 0x180000, 0x080000, CRC(5d07d10d) SHA1(ee1a928b37043c476346f189f75d2bfcc44bffe6) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "uc_030.ic37", 0x000000, 0x100000, CRC(97f7775e) SHA1(5cd147fd940b1ab6eba8e6c6f803bdcc5da5a563) )
	ROM_LOAD( "uc_020.ic36", 0x100000, 0x100000, CRC(5e0b1d65) SHA1(9e45753d10b2d7b580cd11cef74181209a424189) )
	ROM_LOAD( "uc_010.ic35", 0x200000, 0x100000, CRC(bdc224b3) SHA1(09477ec39890d954fac6ff653b9f46c9adea56b6) )
	ROM_LOAD( "uc_000.ic34", 0x300000, 0x100000, CRC(7526daec) SHA1(79431d711deb6ed09dc52be753b7b0f2c5588dc3) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "uc_da.bin", 0x000000, 0x080000, CRC(0b2855e9) SHA1(70f9decd78eab679a2ccad69e01cb303b61e0d38) )
ROM_END

ROM_START( lethalth )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lt_d-h0.rom", 0x000001, 0x020000, CRC(20c68935) SHA1(edbb5322082bde7136ab015931fdcd18e5c293a8) )
	ROM_LOAD16_BYTE( "lt_d-l0.rom", 0x000000, 0x020000, CRC(e1432fb3) SHA1(4b6c22d740cd598d0e34e257910fe7c3d4f3fd32) )
	ROM_LOAD16_BYTE( "lt_d-h1.rom", 0x040001, 0x020000, CRC(d7dd3d48) SHA1(b848feee55159e334f711e4f661d415ffc1e3513) )
	ROM_LOAD16_BYTE( "lt_d-l1.rom", 0x040000, 0x020000, CRC(b94b3bd8) SHA1(7b89d9177d8b357b09317606cb2070c14c3449a5) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "lt_d-sh0.rom", 0x00001, 0x10000, CRC(af5b224f) SHA1(a07f2c6ca0e65af016d74b90342cfaab7535324e) )
	ROM_LOAD16_BYTE( "lt_d-sl0.rom", 0x00000, 0x10000, CRC(cb3faac3) SHA1(e1ee32fac7ee9e97fbf68904572e90aa9d0c9460) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "lt_7a.rom", 0x000000, 0x040000, CRC(ada0fd50) SHA1(7eeb33360cacddf8887f3acce65350af0251936d) )
	ROM_LOAD( "lt_7b.rom", 0x040000, 0x040000, CRC(d2596883) SHA1(5a9f7384c63304c3c1e27375419d59a2b476f46a) )
	ROM_LOAD( "lt_7d.rom", 0x080000, 0x040000, CRC(2de637ef) SHA1(bd1be59d4fe9bf365454c1d471effd88aa942df6) )
	ROM_LOAD( "lt_7h.rom", 0x0c0000, 0x040000, CRC(9f6585cd) SHA1(5d59addc65c3ce20e7ea090a178fe9e17fba525b) )

	ROM_REGION( 0x100000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "lt_7j.rom", 0x000000, 0x040000, CRC(baf8863e) SHA1(fd5937cd70fcffd861a207dc4769d34459dd28d3) )
	ROM_LOAD( "lt_7l.rom", 0x040000, 0x040000, CRC(40fd50af) SHA1(d9bf1e339671fd167fad237bb5bc6d1b183686f5) )
	ROM_LOAD( "lt_7s.rom", 0x080000, 0x040000, CRC(c8e970df) SHA1(7771c17b36dcc9f01ac9e033f3f86e571c5ebbd3) )
	ROM_LOAD( "lt_7y.rom", 0x0c0000, 0x040000, CRC(f5436708) SHA1(e8cb278f4d310eeeb67e01534d17562c7fce62f0) )

	ROM_REGION( 0x40000, "irem", 0 )
	ROM_LOAD( "lt_8a.rom", 0x000000, 0x040000, CRC(357762a2) SHA1(d13b2a0f5d48c0171bcef708589cad194a7ea1ed) )
ROM_END

ROM_START( thndblst )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lt_d-h0j.rom", 0x000001, 0x020000, CRC(dc218a18) SHA1(f092245fd56ca75096c77ce6abf848454b905cfc) )
	ROM_LOAD16_BYTE( "lt_d-l0j.rom", 0x000000, 0x020000, CRC(ae9a3f81) SHA1(c323073e2c245b3c52f93e07b98b0c2d4f4e97b1) )
	ROM_LOAD16_BYTE( "lt_d-h1.rom",  0x040001, 0x020000, CRC(d7dd3d48) SHA1(b848feee55159e334f711e4f661d415ffc1e3513) )
	ROM_LOAD16_BYTE( "lt_d-l1.rom",  0x040000, 0x020000, CRC(b94b3bd8) SHA1(7b89d9177d8b357b09317606cb2070c14c3449a5) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "lt_d-sh0.rom", 0x00001, 0x10000, CRC(af5b224f) SHA1(a07f2c6ca0e65af016d74b90342cfaab7535324e) )
	ROM_LOAD16_BYTE( "lt_d-sl0.rom", 0x00000, 0x10000, CRC(cb3faac3) SHA1(e1ee32fac7ee9e97fbf68904572e90aa9d0c9460) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "lt_7a.rom", 0x000000, 0x040000, CRC(ada0fd50) SHA1(7eeb33360cacddf8887f3acce65350af0251936d) )
	ROM_LOAD( "lt_7b.rom", 0x040000, 0x040000, CRC(d2596883) SHA1(5a9f7384c63304c3c1e27375419d59a2b476f46a) )
	ROM_LOAD( "lt_7d.rom", 0x080000, 0x040000, CRC(2de637ef) SHA1(bd1be59d4fe9bf365454c1d471effd88aa942df6) )
	ROM_LOAD( "lt_7h.rom", 0x0c0000, 0x040000, CRC(9f6585cd) SHA1(5d59addc65c3ce20e7ea090a178fe9e17fba525b) )

	ROM_REGION( 0x100000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "lt_7j.rom", 0x000000, 0x040000, CRC(baf8863e) SHA1(fd5937cd70fcffd861a207dc4769d34459dd28d3) )
	ROM_LOAD( "lt_7l.rom", 0x040000, 0x040000, CRC(40fd50af) SHA1(d9bf1e339671fd167fad237bb5bc6d1b183686f5) )
	ROM_LOAD( "lt_7s.rom", 0x080000, 0x040000, CRC(c8e970df) SHA1(7771c17b36dcc9f01ac9e033f3f86e571c5ebbd3) )
	ROM_LOAD( "lt_7y.rom", 0x0c0000, 0x040000, CRC(f5436708) SHA1(e8cb278f4d310eeeb67e01534d17562c7fce62f0) )

	ROM_REGION( 0x40000, "irem", 0 )
	ROM_LOAD( "lt_8a.rom", 0x000000, 0x040000, CRC(357762a2) SHA1(d13b2a0f5d48c0171bcef708589cad194a7ea1ed) )
ROM_END

ROM_START( nbbatman )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "6_h0.34",   0x000001, 0x040000, CRC(5c4a1e3f) SHA1(583f83ae789040b28b62af67218abfdc6ea74d25) )
	ROM_LOAD16_BYTE( "3_l0.31",   0x000000, 0x040000, CRC(3d6d70ae) SHA1(6585d0e2367d9400d83be483d849d2390cae7d39) )
	ROM_LOAD16_BYTE( "a1-h1-.33", 0x080001, 0x040000, CRC(3ce2aab5) SHA1(b39f17853bcab7ab290fdfaf9f3d8e8c2d91072a) )
	ROM_LOAD16_BYTE( "a1-l1-.32", 0x080000, 0x040000, CRC(116d9bcc) SHA1(c2faf8d1c6b51ac1483757777fd55961b74501fb) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "a1-sh0-.14", 0x00001, 0x10000, CRC(b7fae3e6) SHA1(ce41380d6c0f29f2facf9bf23dd4403648cd9eb4) )
	ROM_LOAD16_BYTE( "a1-sl0-.17", 0x00000, 0x10000, CRC(b26d54fc) SHA1(136e1a83da08a0dc9046faf71f3f58d8d3095fde) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "lh534k0c.9",  0x000000, 0x080000, CRC(314a0c6d) SHA1(a918ae638f10b18165f7d34ab7db54fbf258df01) )
	ROM_LOAD( "lh534k0e.10", 0x080000, 0x080000, CRC(dc31675b) SHA1(81b0a6b35285e855c778c7f32f31115f1edce099) )
	ROM_LOAD( "lh534k0f.11", 0x100000, 0x080000, CRC(e15d8bfb) SHA1(74ea6f9748ed52e579cb08445282c871b3fd0f3a) )
	ROM_LOAD( "lh534k0g.12", 0x180000, 0x080000, CRC(888d71a3) SHA1(d1609e326fda5ac579ddf1ad5dc77443ec2a180f) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "lh538393.42", 0x000000, 0x100000, CRC(26cdd224) SHA1(ab2a3dd8eafec78866a0d45c1f051209025bdc77) )
	ROM_LOAD( "lh538394.43", 0x100000, 0x100000, CRC(4bbe94fa) SHA1(7c13b22e056dc1cf497ea0b3e9766579c33d4370) )
	ROM_LOAD( "lh538395.44", 0x200000, 0x100000, CRC(2a533b5e) SHA1(ceb9750b674adfa5fa0f88e46bce7b2b58440873) )
	ROM_LOAD( "lh538396.45", 0x300000, 0x100000, CRC(863a66fa) SHA1(0edc4734daee8fc1738df4f4f17bcd817f0ade0a) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "lh534k0k.8", 0x000000, 0x080000, CRC(735e6380) SHA1(bf019815e579ef2393c00869f101a01f746e04d6) )
ROM_END

ROM_START( nbbatmanu )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "a1-h0-a.34", 0x000001, 0x040000, CRC(24a9b794) SHA1(a4867a89ea2749d60e6d1225bd84a488403b8cf3) )
	ROM_LOAD16_BYTE( "a1-l0-a.31", 0x000000, 0x040000, CRC(846d7716) SHA1(28434fd74b168ef73d00779b3e5d8b36b1f3ef80) )
	ROM_LOAD16_BYTE( "a1-h1-.33",  0x080001, 0x040000, CRC(3ce2aab5) SHA1(b39f17853bcab7ab290fdfaf9f3d8e8c2d91072a) )
	ROM_LOAD16_BYTE( "a1-l1-.32",  0x080000, 0x040000, CRC(116d9bcc) SHA1(c2faf8d1c6b51ac1483757777fd55961b74501fb) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "a1-sh0-.14", 0x00001, 0x10000, CRC(b7fae3e6) SHA1(ce41380d6c0f29f2facf9bf23dd4403648cd9eb4) )
	ROM_LOAD16_BYTE( "a1-sl0-.17", 0x00000, 0x10000, CRC(b26d54fc) SHA1(136e1a83da08a0dc9046faf71f3f58d8d3095fde) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "lh534k0c.9",  0x000000, 0x080000, CRC(314a0c6d) SHA1(a918ae638f10b18165f7d34ab7db54fbf258df01) )
	ROM_LOAD( "lh534k0e.10", 0x080000, 0x080000, CRC(dc31675b) SHA1(81b0a6b35285e855c778c7f32f31115f1edce099) )
	ROM_LOAD( "lh534k0f.11", 0x100000, 0x080000, CRC(e15d8bfb) SHA1(74ea6f9748ed52e579cb08445282c871b3fd0f3a) )
	ROM_LOAD( "lh534k0g.12", 0x180000, 0x080000, CRC(888d71a3) SHA1(d1609e326fda5ac579ddf1ad5dc77443ec2a180f) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "lh538393.42", 0x000000, 0x100000, CRC(26cdd224) SHA1(ab2a3dd8eafec78866a0d45c1f051209025bdc77) )
	ROM_LOAD( "lh538394.43", 0x100000, 0x100000, CRC(4bbe94fa) SHA1(7c13b22e056dc1cf497ea0b3e9766579c33d4370) )
	ROM_LOAD( "lh538395.44", 0x200000, 0x100000, CRC(2a533b5e) SHA1(ceb9750b674adfa5fa0f88e46bce7b2b58440873) )
	ROM_LOAD( "lh538396.45", 0x300000, 0x100000, CRC(863a66fa) SHA1(0edc4734daee8fc1738df4f4f17bcd817f0ade0a) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "lh534k0k.8", 0x000000, 0x080000, CRC(735e6380) SHA1(bf019815e579ef2393c00869f101a01f746e04d6) )
ROM_END

ROM_START( leaguemn )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lma1-h0.34", 0x000001, 0x040000, CRC(47c54204) SHA1(59de4e9a75c88dba71aa1949e7ac2c4b9e98f413) )
	ROM_LOAD16_BYTE( "lma1-l0.31", 0x000000, 0x040000, CRC(1d062c82) SHA1(8d5969dc0264a05334196132bc2b5a3a59fb9e3a) )
	ROM_LOAD16_BYTE( "a1-h1-.33",  0x080001, 0x040000, CRC(3ce2aab5) SHA1(b39f17853bcab7ab290fdfaf9f3d8e8c2d91072a) )
	ROM_LOAD16_BYTE( "a1-l1-.32",  0x080000, 0x040000, CRC(116d9bcc) SHA1(c2faf8d1c6b51ac1483757777fd55961b74501fb) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "a1-sh0-.14", 0x00001, 0x10000, CRC(b7fae3e6) SHA1(ce41380d6c0f29f2facf9bf23dd4403648cd9eb4) )
	ROM_LOAD16_BYTE( "a1-sl0-.17", 0x00000, 0x10000, CRC(b26d54fc) SHA1(136e1a83da08a0dc9046faf71f3f58d8d3095fde) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "lh534k0c.9",  0x000000, 0x080000, CRC(314a0c6d) SHA1(a918ae638f10b18165f7d34ab7db54fbf258df01) )
	ROM_LOAD( "lh534k0e.10", 0x080000, 0x080000, CRC(dc31675b) SHA1(81b0a6b35285e855c778c7f32f31115f1edce099) )
	ROM_LOAD( "lh534k0f.11", 0x100000, 0x080000, CRC(e15d8bfb) SHA1(74ea6f9748ed52e579cb08445282c871b3fd0f3a) )
	ROM_LOAD( "lh534k0g.12", 0x180000, 0x080000, CRC(888d71a3) SHA1(d1609e326fda5ac579ddf1ad5dc77443ec2a180f) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "lh538393.42", 0x000000, 0x100000, CRC(26cdd224) SHA1(ab2a3dd8eafec78866a0d45c1f051209025bdc77) )
	ROM_LOAD( "lh538394.43", 0x100000, 0x100000, CRC(4bbe94fa) SHA1(7c13b22e056dc1cf497ea0b3e9766579c33d4370) )
	ROM_LOAD( "lh538395.44", 0x200000, 0x100000, CRC(2a533b5e) SHA1(ceb9750b674adfa5fa0f88e46bce7b2b58440873) )
	ROM_LOAD( "lh538396.45", 0x300000, 0x100000, CRC(863a66fa) SHA1(0edc4734daee8fc1738df4f4f17bcd817f0ade0a) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "lh534k0k.8", 0x000000, 0x080000, CRC(735e6380) SHA1(bf019815e579ef2393c00869f101a01f746e04d6) )
ROM_END

ROM_START( nbbatman2bl )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "1.sys_rom",  0x000000, 0x100000, CRC(88526580) SHA1(175ec535d1d4641b3057afd1979b18148060f397) )

	ROM_REGION( 0x20000, "mcu", 0 )
	ROM_LOAD( "at89c4051-24pc.mcu", 0x00000, 0x04000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "4.bin",  0x000000, 0x200000, CRC(17148932) SHA1(f15777b842691dcabc6336a3c33ab6b61c83ae8b) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "2.bin", 0x000000, 0x400000, CRC(bae2eb19) SHA1(609c805b23ceaf4cb02f4ad6192ab4dd50b89711) )

	ROM_REGION( 0x200000, "irem", 0 )
	ROM_LOAD( "3.sou_rom", 0x000000, 0x100000, CRC(776ed65d) SHA1(0e3321c024a62fc48aa5541215af8af14c95ccc6) )
ROM_END


ROM_START( ssoldier )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "f3-h0-h.bin", 0x000001, 0x040000, CRC(b63fb9da) SHA1(429beb7ebc98815809fdd0ff69fcb4a14e1d8a14) )
	ROM_LOAD16_BYTE( "f3-l0-h.bin", 0x000000, 0x040000, CRC(419361a2) SHA1(42284a7afedefdb58a9b505e87effeee8bb5a9d8) )
	ROM_LOAD16_BYTE( "f3-h1-a.bin", 0x080001, 0x020000, CRC(e3d9f619) SHA1(7f450413d1fae7250d2fcbe0ff4ee13d52fa15e8) )
	ROM_LOAD16_BYTE( "f3-l1-a.bin", 0x080000, 0x020000, CRC(8cb5c396) SHA1(af130632b4ffb846cf355064391130d8c7ba73ad) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "f3_sh0.sh0", 0x00001, 0x10000, CRC(90b55e5e) SHA1(cf77ccb68a10a29289bc42db348f480e21c3a558) )
	ROM_LOAD16_BYTE( "f3_sl0.sl0", 0x00000, 0x10000, CRC(77c16d57) SHA1(68c7f026b718b700f1f9162f53cdc859b65944b9) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "f3_w50.c0", 0x000000, 0x040000, CRC(47e788ee) SHA1(79a6624c9a36f380057c4fbda511128d62f9161e) )
	ROM_LOAD( "f3_w51.c1", 0x080000, 0x040000, CRC(8e535e3f) SHA1(a51a5a660d13e95da559e7c1eaf23479eddd196f) )
	ROM_LOAD( "f3_w52.c2", 0x100000, 0x040000, CRC(a6eb2e56) SHA1(db45fd5ffefbe407247069c611a1d40849770297) )
	ROM_LOAD( "f3_w53.c3", 0x180000, 0x040000, CRC(2f992807) SHA1(bc0fe02b7ad31cb06ab0bf3f91de4ca5130893f1) )

	ROM_REGION( 0x800000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "f3_w37.000", 0x000001, 0x100000, CRC(fd4cda03) SHA1(34bfabb5a0fdc96507d3c3c028a0b087c406a0d1) )
	ROM_LOAD16_BYTE( "f3_w38.001", 0x000000, 0x100000, CRC(755bab10) SHA1(8d3a584f5e34da24a162c1812ec5a3fea49778d7) )
	ROM_LOAD16_BYTE( "f3_w39.010", 0x200001, 0x100000, CRC(b21ced92) SHA1(0af44bddaef77f9427f7073dfc96e8a59d7a9ba5) )
	ROM_LOAD16_BYTE( "f3_w40.011", 0x200000, 0x100000, CRC(2e906889) SHA1(2aee05ce8f0074302090f1b1c58054c4a861ae68) )
	ROM_LOAD16_BYTE( "f3_w41.020", 0x400001, 0x100000, CRC(02455d10) SHA1(4f83d8349d39b220a2150a52d0202c7f8d2b588f) )
	ROM_LOAD16_BYTE( "f3_w42.021", 0x400000, 0x100000, CRC(124589b9) SHA1(dc8f95a0ff205fd24136738941a8931c16c380a4) )
	ROM_LOAD16_BYTE( "f3_w43.030", 0x600001, 0x100000, CRC(dae7327a) SHA1(3c742b57f30df3ee8d5f5b36dc890af1ec396df5) )
	ROM_LOAD16_BYTE( "f3_w44.031", 0x600000, 0x100000, CRC(d0fc84ac) SHA1(19154f81c4182be1fe835b5647fa30360c3507aa) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "f3_w95.da", 0x000000, 0x080000, CRC(f7ca432b) SHA1(274458b68f906e6043bc36110a4903280647ac2d) )
ROM_END

ROM_START( psoldier )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "f3_h0-d.bin", 0x000001, 0x040000, CRC(38f131fd) SHA1(0e513a5edfd8ab14440c360000a40b9d750cb54a) )
	ROM_LOAD16_BYTE( "f3_l0-d.bin", 0x000000, 0x040000, CRC(1662969c) SHA1(8de1683076d7128ec16f1d053afb5e236add73e6) )
	ROM_LOAD16_BYTE( "f3_h1-.bin",  0x080001, 0x040000, CRC(c8d1947c) SHA1(832a448f117224941799aeece2ec0b25065be3e2) )
	ROM_LOAD16_BYTE( "f3_l1-.bin",  0x080000, 0x040000, CRC(7b9492fc) SHA1(335166d096dec3773ec69b05dad6763505818dd6) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "f3_sh0.sh0", 0x00001, 0x10000, CRC(90b55e5e) SHA1(cf77ccb68a10a29289bc42db348f480e21c3a558) )
	ROM_LOAD16_BYTE( "f3_sl0.sl0", 0x00000, 0x10000, CRC(77c16d57) SHA1(68c7f026b718b700f1f9162f53cdc859b65944b9) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "f3_w50.c0", 0x000000, 0x040000, CRC(47e788ee) SHA1(79a6624c9a36f380057c4fbda511128d62f9161e) )
	ROM_LOAD( "f3_w51.c1", 0x080000, 0x040000, CRC(8e535e3f) SHA1(a51a5a660d13e95da559e7c1eaf23479eddd196f) )
	ROM_LOAD( "f3_w52.c2", 0x100000, 0x040000, CRC(a6eb2e56) SHA1(db45fd5ffefbe407247069c611a1d40849770297) )
	ROM_LOAD( "f3_w53.c3", 0x180000, 0x040000, CRC(2f992807) SHA1(bc0fe02b7ad31cb06ab0bf3f91de4ca5130893f1) )

	ROM_REGION( 0x800000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "f3_w37.000", 0x000001, 0x100000, CRC(fd4cda03) SHA1(34bfabb5a0fdc96507d3c3c028a0b087c406a0d1) )
	ROM_LOAD16_BYTE( "f3_w38.001", 0x000000, 0x100000, CRC(755bab10) SHA1(8d3a584f5e34da24a162c1812ec5a3fea49778d7) )
	ROM_LOAD16_BYTE( "f3_w39.010", 0x200001, 0x100000, CRC(b21ced92) SHA1(0af44bddaef77f9427f7073dfc96e8a59d7a9ba5) )
	ROM_LOAD16_BYTE( "f3_w40.011", 0x200000, 0x100000, CRC(2e906889) SHA1(2aee05ce8f0074302090f1b1c58054c4a861ae68) )
	ROM_LOAD16_BYTE( "f3_w41.020", 0x400001, 0x100000, CRC(02455d10) SHA1(4f83d8349d39b220a2150a52d0202c7f8d2b588f) )
	ROM_LOAD16_BYTE( "f3_w42.021", 0x400000, 0x100000, CRC(124589b9) SHA1(dc8f95a0ff205fd24136738941a8931c16c380a4) )
	ROM_LOAD16_BYTE( "f3_w43.030", 0x600001, 0x100000, CRC(dae7327a) SHA1(3c742b57f30df3ee8d5f5b36dc890af1ec396df5) )
	ROM_LOAD16_BYTE( "f3_w44.031", 0x600000, 0x100000, CRC(d0fc84ac) SHA1(19154f81c4182be1fe835b5647fa30360c3507aa) )

	ROM_REGION( 0x80000, "irem", 0 )
	ROM_LOAD( "f3_w95.da", 0x000000, 0x080000, CRC(f7ca432b) SHA1(274458b68f906e6043bc36110a4903280647ac2d) )
ROM_END

ROM_START( dsoccr94j )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("a3_-h0-e.bin", 0x000001, 0x040000, CRC(8de1dbcd) SHA1(3726c7f8bc1e61a488ab7ef0b79a7a45054235c2) )
	ROM_LOAD16_BYTE("a3_-l0-e.bin", 0x000000, 0x040000, CRC(d3df8bfd) SHA1(b98064579491aef8eb8ccb94195412e79674a0c1) )
	ROM_LOAD16_BYTE("ds_h1-c.rom",  0x080001, 0x040000, CRC(6109041b) SHA1(063898a88f8a6a9f1510aa55e53a39f037b02903) )
	ROM_LOAD16_BYTE("ds_l1-c.rom",  0x080000, 0x040000, CRC(97a01f6b) SHA1(e188e28f880f5f3f4d7b49eca639d643989b1468) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE("ds_sh0.rom", 0x00001, 0x10000, CRC(23fe6ffc) SHA1(896377961cafc19e44d9d889f9fbfdbaedd556da) )
	ROM_LOAD16_BYTE("ds_sl0.rom", 0x00000, 0x10000, CRC(768132e5) SHA1(1bb64516eb58d3b246f08e1c07f091e78085689f) )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* chars */
	ROM_LOAD("c0.bin", 0x000000, 0x100000, CRC(83ea8a47) SHA1(b29c8cc50da85c8168dda92446dfa12582580f96) )
	ROM_LOAD("c1.bin", 0x100000, 0x100000, CRC(64063e6d) SHA1(80b66e08292a3682f80d5670c5fe9f0fcc92062e) )
	ROM_LOAD("c2.bin", 0x200000, 0x100000, CRC(cc1f621a) SHA1(a0bdfe582206d49ca01bedc2b6973ebe5248efe4) )
	ROM_LOAD("c3.bin", 0x300000, 0x100000, CRC(515829e1) SHA1(2b5a5151eeb56cd3da30c8cb6415605cbe1d82e9) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites */
	ROM_LOAD16_BYTE("a3-o00-w.bin", 0x000001, 0x80000, CRC(b094e5ad) SHA1(9acceb24a72eeb3c6e629c08d4cc9ef2a171da32) )
	ROM_LOAD16_BYTE("a3-o01-w.bin", 0x000000, 0x80000, CRC(91f34018) SHA1(4982b914ecce0358d63800caf7e249e1723bf7cf) )
	ROM_LOAD16_BYTE("a3-o10-w.bin", 0x100001, 0x80000, CRC(edddeef4) SHA1(73a90c20c99209206370e8bff35199c3a6b9dc3d) )
	ROM_LOAD16_BYTE("a3-o11-w.bin", 0x100000, 0x80000, CRC(274a9526) SHA1(2844079b2ec33ff2ccf6f73586ff426bdab9cf83) )
	ROM_LOAD16_BYTE("a3-o20-w.bin", 0x200001, 0x80000, CRC(32064393) SHA1(bacd4902079557141133920c44b16b52242692e7) )
	ROM_LOAD16_BYTE("a3-o21-w.bin", 0x200000, 0x80000, CRC(57bae3d9) SHA1(11face0e157ed42d7836bb8d60a4b75740de52d5) )
	ROM_LOAD16_BYTE("a3-o30-w.bin", 0x300001, 0x80000, CRC(be838e2f) SHA1(fc8a42b9183dfc60c317cbcef1da6798fed125ef) )
	ROM_LOAD16_BYTE("a3-o31-w.bin", 0x300000, 0x80000, CRC(bf899f0d) SHA1(f781454df089743186816ce98d863c94f7b208bd) )

	ROM_REGION( 0x100000, "irem", 0 )
	ROM_LOAD("ds_da0.rom",  0x000000, 0x100000, CRC(67fc52fd) SHA1(5771e948115af8fe4a6d3f448c03a2a9b42b6f20) )
ROM_END

ROM_START( gunforc2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("a2-h0-a.6h", 0x000001, 0x040000, CRC(49965e22) SHA1(077283c66a4cc2c47221c5f3267f440223615a15) )
	ROM_LOAD16_BYTE("a2-l0-a.8h", 0x000000, 0x040000, CRC(8c88b278) SHA1(0fd8e663619dcd8c81b3baa290bb0e72c185273a) )
	ROM_LOAD16_BYTE("a2-h1-a.6f", 0x080001, 0x040000, CRC(34280b88) SHA1(3fd3cdf8acfa845abacb0708fb48741ee44dbf13) )
	ROM_LOAD16_BYTE("a2-l1-a.8f", 0x080000, 0x040000, CRC(c8c13f51) SHA1(fde3fd983ebb920f79e6898aa0576da9dd9f0c15) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE("a2_sh0.3l",  0x00001, 0x10000, CRC(2e2d103d) SHA1(6b663948f69218308d9ecdb677557b2db1dfbf5a) )
	ROM_LOAD16_BYTE("a2_sl0.5l",  0x00000, 0x10000, CRC(2287e0b3) SHA1(755dab510915161428ed57ab18410c393e138e65) )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* chars */
	ROM_LOAD("a2_c0.1a",   0x000000, 0x080000, CRC(68b8f574) SHA1(fb935947cdde43e84453f82caeea141a4ae7226d) )
	ROM_LOAD("a2_c1.1b",   0x080000, 0x080000, CRC(0b9efe67) SHA1(1df4108d30d2538f6407e328513517cd3412321f) )
	ROM_LOAD("a2_c2.3a",   0x100000, 0x080000, CRC(7a9e9978) SHA1(241dc310e75960e306701a2e86e30d9c1a60ebff) )
	ROM_LOAD("a2_c3.3b",   0x180000, 0x080000, CRC(1395ee6d) SHA1(e9befc966e6ee046eaca185a9969976304a119d8) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "a2_000.8a", 0x000000, 0x100000, CRC(38e03147) SHA1(cc5bacad9592aa5e91632b139955e1c704a67a33) )
	ROM_LOAD( "a2_010.8b", 0x100000, 0x100000, CRC(1d5b05f8) SHA1(884f134ed51b432965a4e5e79915ba9c0ab562c6) )
	ROM_LOAD( "a2_020.8c", 0x200000, 0x100000, CRC(f2f461cc) SHA1(04e91efc749d022c8012caac493767ec1f6a992d) )
	ROM_LOAD( "a2_030.8d", 0x300000, 0x100000, CRC(97609d9d) SHA1(71ddff85a8ddeac69863bbf6c493c5c3973fd175) )

	ROM_REGION( 0x100000, "irem", 0 )
	ROM_LOAD("a2_da.1l",  0x000000, 0x100000, CRC(3c8cdb6a) SHA1(d1f4186e8ddf99698443f8ee1c60a6e6bc367b09) )
ROM_END

ROM_START( geostorm )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("geo-h0.bin", 0x000001, 0x040000, CRC(9be58d09) SHA1(ab98b91abc8129c342c59674eab9683cccc6ca35) )
	ROM_LOAD16_BYTE("geo-l0.bin", 0x000000, 0x040000, CRC(59abb75d) SHA1(52b48685470ffa3f36a8259bf333448bf40caea9) )
	ROM_LOAD16_BYTE("a2-h1-a.6f", 0x080001, 0x040000, CRC(34280b88) SHA1(3fd3cdf8acfa845abacb0708fb48741ee44dbf13) )
	ROM_LOAD16_BYTE("a2-l1-a.8f", 0x080000, 0x040000, CRC(c8c13f51) SHA1(fde3fd983ebb920f79e6898aa0576da9dd9f0c15) )

	ROM_REGION( 0x20000, "soundcpu", 0 )
	ROM_LOAD16_BYTE("a2_sh0.3l",  0x00001, 0x10000, CRC(2e2d103d) SHA1(6b663948f69218308d9ecdb677557b2db1dfbf5a) )
	ROM_LOAD16_BYTE("a2_sl0.5l",  0x00000, 0x10000, CRC(2287e0b3) SHA1(755dab510915161428ed57ab18410c393e138e65) )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* chars */
	ROM_LOAD("a2_c0.1a",   0x000000, 0x080000, CRC(68b8f574) SHA1(fb935947cdde43e84453f82caeea141a4ae7226d) )
	ROM_LOAD("a2_c1.1b",   0x080000, 0x080000, CRC(0b9efe67) SHA1(1df4108d30d2538f6407e328513517cd3412321f) )
	ROM_LOAD("a2_c2.3a",   0x100000, 0x080000, CRC(7a9e9978) SHA1(241dc310e75960e306701a2e86e30d9c1a60ebff) )
	ROM_LOAD("a2_c3.3b",   0x180000, 0x080000, CRC(1395ee6d) SHA1(e9befc966e6ee046eaca185a9969976304a119d8) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD( "a2_000.8a", 0x000000, 0x100000, CRC(38e03147) SHA1(cc5bacad9592aa5e91632b139955e1c704a67a33) )
	ROM_LOAD( "a2_010.8b", 0x100000, 0x100000, CRC(1d5b05f8) SHA1(884f134ed51b432965a4e5e79915ba9c0ab562c6) )
	ROM_LOAD( "a2_020.8c", 0x200000, 0x100000, CRC(f2f461cc) SHA1(04e91efc749d022c8012caac493767ec1f6a992d) )
	ROM_LOAD( "a2_030.8d", 0x300000, 0x100000, CRC(97609d9d) SHA1(71ddff85a8ddeac69863bbf6c493c5c3973fd175) )

	ROM_REGION( 0x100000, "irem", 0 )
	ROM_LOAD("a2_da.1l",  0x000000, 0x100000, CRC(3c8cdb6a) SHA1(d1f4186e8ddf99698443f8ee1c60a6e6bc367b09) )
ROM_END

/***************************************************************************/

DRIVER_INIT_MEMBER(m92_state,m92)
{
	UINT8 *ROM = memregion("maincpu")->base();

	membank("bank1")->set_base(&ROM[0xa0000]);

	m_game_kludge = 0;
}

/* different address map (no bank1) */
DRIVER_INIT_MEMBER(m92_state,lethalth)
{
	m_game_kludge = 0;
}

/* has bankswitching */
DRIVER_INIT_MEMBER(m92_state,m92_bank)
{
	UINT8 *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 4, &ROM[0x80000], 0x20000);
	m_maincpu->space(AS_IO).install_write_handler(0x20, 0x21, write16_delegate(FUNC(m92_state::m92_bankswitch_w),this));

	m_game_kludge = 0;
}

/* has bankswitching, has eeprom, needs sprite kludge */
DRIVER_INIT_MEMBER(m92_state,majtitl2)
{
	UINT8 *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 4, &ROM[0x80000], 0x20000);
	m_maincpu->space(AS_IO).install_write_handler(0x20, 0x21, write16_delegate(FUNC(m92_state::m92_bankswitch_w),this));

	/* This game has an eeprom on the game board */
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xf0000, 0xf3fff, read16_delegate(FUNC(m92_state::m92_eeprom_r),this), write16_delegate(FUNC(m92_state::m92_eeprom_w),this));

	m_game_kludge = 2;
}

/* TODO: figure out actual address map and other differences from real Irem h/w */
DRIVER_INIT_MEMBER(m92_state,ppan)
{
	UINT8 *ROM = memregion("maincpu")->base();
	membank("bank1")->set_base(&ROM[0xa0000]);

	m_game_kludge = 0;
}

/***************************************************************************/

GAME( 1991, gunforce, 0,        gunforce,      gunforce, m92_state, m92,      ROT0,   "Irem",         "Gunforce - Battle Fire Engulfed Terror Island (World)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1991, gunforcej,gunforce, gunforce,      gunforce, m92_state, m92,      ROT0,   "Irem",         "Gunforce - Battle Fire Engulfed Terror Island (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1991, gunforceu,gunforce, gunforce,      gunforce, m92_state, m92,      ROT0,   "Irem America", "Gunforce - Battle Fire Engulfed Terror Island (US)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )

GAME( 1991, bmaster,  0,        bmaster,       bmaster, m92_state,  m92,      ROT0,   "Irem",         "Blade Master (World)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1991, crossbld, bmaster,  bmaster,       bmaster, m92_state,  m92,      ROT0,   "Irem",         "Cross Blades! (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)

GAME( 1991, lethalth, 0,        lethalth,      lethalth, m92_state, lethalth, ROT270, "Irem",         "Lethal Thunder (World)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1991, thndblst, lethalth, lethalth,      lethalth, m92_state, lethalth, ROT270, "Irem",         "Thunder Blaster (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )

GAME( 1992, uccops,   0,        uccops,        uccops, m92_state,   m92,      ROT0,   "Irem",         "Undercover Cops (World)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1992, uccopsu,  uccops,   uccops,        uccops, m92_state,   m92,      ROT0,   "Irem",         "Undercover Cops (US)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1992, uccopsar, uccops,   uccops,        uccops, m92_state,   m92,      ROT0,   "Irem",         "Undercover Cops - Alpha Renewal Version", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1992, uccopsj,  uccops,   uccops,        uccops, m92_state,   m92,      ROT0,   "Irem",         "Undercover Cops (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )

GAME( 1992, mysticri, 0,        mysticri,      mysticri, m92_state, m92,      ROT0,   "Irem",         "Mystic Riders (World)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME( 1992, gunhohki, mysticri, mysticri,      mysticri, m92_state, m92,      ROT0,   "Irem",         "Mahou Keibitai Gun Hohki (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
// cheaply produced Korean board, has original chips, but lacks any proper labels
// main code is also significantly different to the supported original set, so it might just be a legitimate early revision on a cheap board
GAME( 1992, mysticrib,mysticri, mysticri,      mysticri, m92_state, m92,      ROT0,   "Irem",         "Mystic Riders (bootleg?)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )

GAME( 1992, majtitl2, 0,        majtitl2,      majtitl2, m92_state, majtitl2, ROT0,   "Irem",         "Major Title 2 (World)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME( 1992, majtitl2j,majtitl2, majtitl2,      majtitl2, m92_state, majtitl2, ROT0,   "Irem",         "Major Title 2 (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)

GAME( 1992, skingame, majtitl2, majtitl2,      majtitl2, m92_state, majtitl2, ROT0,   "Irem America", "The Irem Skins Game (US set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1992, skingame2,majtitl2, majtitl2,      majtitl2, m92_state, majtitl2, ROT0,   "Irem America", "The Irem Skins Game (US set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )

GAME( 1992, hook,     0,        hook,          hook, m92_state,     m92,      ROT0,   "Irem",         "Hook (World)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME( 1992, hooku,    hook,     hook,          hook, m92_state,     m92,      ROT0,   "Irem America", "Hook (US)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME( 1992, hookj,    hook,     hook,          hook, m92_state,     m92,      ROT0,   "Irem",         "Hook (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME( 1992, ppan,     hook,     ppan,          hook, m92_state,     ppan,     ROT0,   "bootleg",      "Peter Pan (bootleg of Hook)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL) // PCB marked 'Peter Pan', no title screen, made in Italy?

GAME( 1992, rtypeleo, 0,        rtypeleo,      rtypeleo, m92_state, m92,     ROT0,   "Irem",         "R-Type Leo (World)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME( 1992, rtypeleoj,rtypeleo, rtypeleo,      rtypeleo, m92_state, m92,     ROT0,   "Irem",         "R-Type Leo (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)

GAME( 1993, inthunt,  0,        inthunt,       inthunt, m92_state,  m92,      ROT0,   "Irem",         "In The Hunt (World)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME( 1993, inthuntu, inthunt,  inthunt,       inthunt, m92_state,  m92,      ROT0,   "Irem America", "In The Hunt (US)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
GAME( 1993, kaiteids, inthunt,  inthunt,       inthunt, m92_state,  m92,      ROT0,   "Irem",         "Kaitei Daisensou (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )

GAME( 1993, nbbatman, 0,        nbbatman,      nbbatman, m92_state, m92_bank, ROT0,   "Irem",         "Ninja Baseball Bat Man (World)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL )
GAME( 1993, nbbatmanu,nbbatman, nbbatman,      nbbatman, m92_state, m92_bank, ROT0,   "Irem America", "Ninja Baseball Bat Man (US)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL)
GAME( 1993, leaguemn, nbbatman, nbbatman,      nbbatman, m92_state, m92_bank, ROT0,   "Irem",         "Yakyuu Kakutou League-Man (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL )
GAME( 1993, nbbatman2bl,nbbatman,nbbatman2bl,  nbbatman, m92_state, m92_bank, ROT0,   "bootleg",      "Ninja Baseball Bat Man II (bootleg)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_NO_COCKTAIL ) // different sprite system, MCU as soundcpu, OKI samples for music/sound

GAME( 1993, ssoldier, 0,        psoldier,      psoldier, m92_state, m92,      ROT0,   "Irem America", "Superior Soldiers (US)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1993, psoldier, ssoldier, psoldier,      psoldier, m92_state, m92,      ROT0,   "Irem",         "Perfect Soldiers (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )

GAME( 1994, dsoccr94j,dsoccr94, dsoccr94j,     dsoccr94j, m92_state,m92_bank, ROT0,   "Irem",         "Dream Soccer '94 (Japan, M92 hardware)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )

GAME( 1994, gunforc2, 0,        gunforc2,      gunforc2, m92_state, m92_bank, ROT0,   "Irem",         "Gun Force II (US)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1994, geostorm, gunforc2, gunforc2,      gunforc2, m92_state, m92_bank, ROT0,   "Irem",         "Geo Storm (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL)
