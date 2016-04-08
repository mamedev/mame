// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Beast Busters           A9003   (c) 1989 SNK Corporation
    Mechanized Attack       A8002   (c) 1989 SNK Corporation

    Beast Busters is a large dedicated (non-jamma) triple machine gun game,
    the gun positions values are read in an interrupt routine that must be
    called for each position (X and Y for 3 guns, so at least 6 times a
    frame).  However I can't make it work reliably..  So for the moment
    I'm writing the gun positions directly to memory and bypassing
    the IRQ routine.

    Mechanized Attack (A8002) is an earlier design, it only has one sprite
    chip, no eeprom, and only 2 machine guns, but the tilemaps are twice
    the size.

    Emulation by Bryan McPhail, mish@tendril.co.uk


Stephh's notes (based on the games M68000 code and some tests) :

1) 'bbusters'

  - Country/version is stored at 0x003954.w and the possible values are
    0x0000, 0x0004 and 0x0008 (0x000c being the same as 0x0008), 0x0008
    being the value stored in ROM in the current set.
    I haven't noticed any major effect (copyright/logo, language, coinage),
    the only thing I found is that the text relative to number of coins
    needed is different (but it's a lie as coinage is NOT modified).
    So my guess is that if other versions exist, part of the code (or at
    least data in it) will have to be different.
    Anyway, here is my guess to determine the versions (by using some infos
    from 'mechatt' :

       Value   Country
      0x0000    Japan
      0x0004    US?
      0x0008    World?   (value stored in the current set)
      0x000c    World?   (same as 0x0008 - impossible choice ?)

  - Coin buttons act differently depending on the "Coin Slots" Dip Switch :

      * "Coin Slots" Dip Switch set to "Common" :

          . COIN1    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
          . COIN2    : adds coin(s)/credit(s) depending on "Coin B" Dip Switch
          . COIN3    : NO EFFECT !
          . COIN4    : NO EFFECT !
          . COIN5    : NO EFFECT !
          . COIN6    : NO EFFECT !
          . SERVICE1 : adds coin(s)/credit(s) depending on "Coin A" Dip Switch

      * "Coin Slots" Dip Switch set to "Individual" :

          . COIN1    : adds coin(s)/credit(s) for player 1 depending on "Coin A" Dip Switch
          . COIN2    : adds coin(s)/credit(s) for player 1 depending on "Coin B" Dip Switch
          . COIN3    : adds coin(s)/credit(s) for player 2 depending on "Coin A" Dip Switch
          . COIN4    : adds coin(s)/credit(s) for player 2 depending on "Coin B" Dip Switch
          . COIN5    : adds coin(s)/credit(s) for player 3 depending on "Coin A" Dip Switch
          . COIN6    : adds coin(s)/credit(s) for player 3 depending on "Coin B" Dip Switch
          . SERVICE1 : adds coin(s)/credit(s) for all players depending on "Coin A" Dip Switch

    Note that I had to map COIN5 and COIN6 to SERVICE2 and SERVICE3 to be
    able to use the default parametrable keys. Let me know if there is a
    another (better ?) way to do so.


2) 'mechatt'

  - Country/version is stored at 0x06a000.w and the possible values are :

       Value   Country
      0x0000    Japan
      0x1111    World    (value stored in the current set)
      0x2222    US
      0x3333    Asia?    (it looks like Japanese text but some "symbols" are missing)

2a) Japan version

  - All texts are in Japanese.
  - "(c) 1989 (Corp) S-N-K".
  - "Coin Slots" Dip Switch has no effect.
  - "Coin A" and "Coin B" Dip Switches are the same as in the World version.
  - Coin buttons effect :

      * "Coin Slots" are ALWAYS considered as "Common" :

          . COIN1    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
          . COIN2    : adds coin(s)/credit(s) depending on "Coin B" Dip Switch
          . COIN3    : NO EFFECT !
          . COIN4    : NO EFFECT !
          . SERVICE1 : adds coin(s)/credit(s) depending on "Coin A" Dip Switch

2b) World version

  - All texts are in English.
  - "(c) 1989 SNK Corporation".
  - Coin buttons effect :

      * "Coin Slots" Dip Switch set to "Common" :

          . COIN1    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
          . COIN2    : adds coin(s)/credit(s) depending on "Coin B" Dip Switch
          . COIN3    : NO EFFECT !
          . COIN4    : NO EFFECT !
          . SERVICE1 : adds coin(s)/credit(s) depending on "Coin A" Dip Switch

      * "Coin Slots" Dip Switch set to "Individual" :

          . COIN1    : adds coin(s)/credit(s) for player 1 depending on "Coin A" Dip Switch
          . COIN2    : adds coin(s)/credit(s) for player 1 depending on "Coin B" Dip Switch
          . COIN3    : adds coin(s)/credit(s) for player 2 depending on "Coin A" Dip Switch
          . COIN4    : adds coin(s)/credit(s) for player 2 depending on "Coin B" Dip Switch
          . SERVICE1 : adds coin(s)/credit(s) for all players depending on "Coin A" Dip Switch

2c) US version

  - All texts are in English.
  - "(c) 1989 SNK Corp. of America".
  - Additional FBI logo as first screen as well as small FBI notice at the bottom left
    of the screens until a coin is inserted.
  - "Coin Slots" Dip Switch has no effect.
  - "Coin A" Dip Switch is different from the World version :

      World      US
      4C_1C    "Free Play"
      3C_1C    special (see below)
      2C_1C    "2 Coins to Start, 1 to Continue"
      1C_1C    1C_1C

    It's a bit hard to explain the "special" coinage, so here are some infos :

      * when you insert a coin before starting a game, you are awarded 2 credits
        if credits == 0, else you are awarded 1 credit
      * when you insert a coin to continue, you are ALWAYS awarded 1 credit

  - "Coin B" Dip Switch has no effect.

  - Coin buttons effect :

      * "Coin Slots" are ALWAYS considered as "Individual" :

          . COIN1    : adds coin(s)/credit(s) for player 1 depending on "Coin A" Dip Switch
          . COIN2    : adds coin(s)/credit(s) for player 2 depending on "Coin A" Dip Switch
          . COIN3    : NO EFFECT !
          . COIN4    : NO EFFECT !
          . SERVICE1 : adds coin(s)/credit(s) for all players depending on "Coin A" Dip Switch

2d) Asia? version

  - All texts are in Japanese ? (to be confirmed)
  - "(c) 1989 SNK Corporation".
  - "Coin Slots" Dip Switch has no effect.
  - "Coin A" and "Coin B" Dip Switches are the same as in the World version.
  - Coin buttons effect :

      * "Coin Slots" are ALWAYS considered as "Common" :

          . COIN1    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
          . COIN2    : adds coin(s)/credit(s) depending on "Coin B" Dip Switch
          . COIN3    : NO EFFECT !
          . COIN4    : NO EFFECT !
          . SERVICE1 : adds coin(s)/credit(s) depending on "Coin A" Dip Switch



HIGHWAYMAN's notes:

after adding the mechanized attack u.s. roms I suspect that there is more than just a few bytes changed ;-)


RansAckeR's notes:

bbusters:

If you only calibrate the P1 gun or do not hit the correct spots for all guns
you will get either garbage or a black screen when rebooting.
According to the manual this happens when the eprom contains invalid gun aim data.

If you calibrate the guns correctly the game runs as expected:
1) Using P1 controls fire at the indicated spots.
2) Using P2 controls fire at the indicated spots.
3) Using P3 controls fire at the indicated spots.

---

Beast Busters notes (from Brian Hargrove)

1. Stage 2 for example, has background sprites and enemies that float on the
foreground and not behind the moving elevator layer.


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/2608intf.h"
#include "sound/2610intf.h"
#include "includes/bbusters.h"
#include "machine/nvram.h"


/******************************************************************************/


/* Beast Busters Region code works as follows

ROM[0x003954/2] = data * 4;

Country/Version :

 - 0x0000 : Japan?
 - 0x0004 : US?
 - 0x0008 : World?    (default)
 - 0x000c : World?    (same as 0x0008)

*/

/* Mech Attack Region code works as follows

ROM[0x06a000/2] = (data << 12) | (data << 8) | (data << 4) | (data << 0);

Country :
- 0x0000 : Japan
- 0x1111 : World (default)
- 0x2222 : US
- 0x3333 : Asia?

*/


/******************************************************************************/

void bbusters_state::machine_start()
{
	save_item(NAME(m_sound_status));
	save_item(NAME(m_gun_select));
}

READ16_MEMBER(bbusters_state::sound_status_r)
{
	return m_sound_status;
}

WRITE8_MEMBER(bbusters_state::sound_status_w)
{
	m_sound_status = data;
}

WRITE16_MEMBER(bbusters_state::sound_cpu_w)
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_byte_w(space, 0, data&0xff);
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
}

/* Eprom is byte wide, top half of word _must_ be 0xff */
READ16_MEMBER(bbusters_state::eprom_r)
{
	return (m_eprom_data[offset]&0xff) | 0xff00;
}

READ16_MEMBER(bbusters_state::control_3_r)
{
	static const char *const port[] = { "GUNX1", "GUNY1", "GUNX2", "GUNY2", "GUNX3", "GUNY3" };

	UINT16 retdata = ioport(port[m_gun_select])->read();

	retdata >>=1; // by lowering the precision of the gun reading hardware the game seems to work better

	return retdata;
}

WRITE16_MEMBER(bbusters_state::gun_select_w)
{
	logerror("%08x: gun r\n",space.device().safe_pc());

	space.device().execute().set_input_line(2, HOLD_LINE);

	m_gun_select = data & 0xff;
}

WRITE16_MEMBER(bbusters_state::two_gun_output_w)
{
	output().set_value("Player1_Gun_Recoil",(data & 0x01));
	output().set_value("Player2_Gun_Recoil",(data & 0x02)>>1);
}

WRITE16_MEMBER(bbusters_state::three_gun_output_w)
{
	output().set_value("Player1_Gun_Recoil",(data & 0x01));
	output().set_value("Player2_Gun_Recoil",(data & 0x02)>>1);
	output().set_value("Player3_Gun_Recoil",(data & 0x04)>>2);
}

READ16_MEMBER(bbusters_state::kludge_r)
{
	// might latch the gun value?
	return 0x0000;
}

READ16_MEMBER(bbusters_state::mechatt_gun_r)
{
	int x, y;

	x = ioport(offset ? "GUNX2" : "GUNX1")->read();
	y = ioport(offset ? "GUNY2" : "GUNY1")->read();

	/* Todo - does the hardware really clamp like this? */
	x += 0x18;
	if (x > 0xff) x = 0xff;
	if (y > 0xef) y = 0xef;

	return x | (y<<8);
}

/*******************************************************************************/

static ADDRESS_MAP_START( bbusters_map, AS_PROGRAM, 16, bbusters_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x08ffff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x090000, 0x090fff) AM_RAM_WRITE(video_w) AM_SHARE("videoram")
	AM_RANGE(0x0a0000, 0x0a0fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x0a1000, 0x0a7fff) AM_RAM     /* service mode */
	AM_RANGE(0x0a8000, 0x0a8fff) AM_RAM AM_SHARE("spriteram2")
	AM_RANGE(0x0a9000, 0x0affff) AM_RAM     /* service mode */
	AM_RANGE(0x0b0000, 0x0b1fff) AM_RAM_WRITE(pf1_w) AM_SHARE("pf1_data")
	AM_RANGE(0x0b2000, 0x0b3fff) AM_RAM_WRITE(pf2_w) AM_SHARE("pf2_data")
	AM_RANGE(0x0b4000, 0x0b5fff) AM_RAM     /* service mode */
	AM_RANGE(0x0b8000, 0x0b8003) AM_WRITEONLY AM_SHARE("pf1_scroll_data")
	AM_RANGE(0x0b8008, 0x0b800b) AM_WRITEONLY AM_SHARE("pf2_scroll_data")
	AM_RANGE(0x0d0000, 0x0d0fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x0e0000, 0x0e0001) AM_READ_PORT("COINS")  /* Coins */
	AM_RANGE(0x0e0002, 0x0e0003) AM_READ_PORT("IN0")    /* Player 1 & 2 */
	AM_RANGE(0x0e0004, 0x0e0005) AM_READ_PORT("IN1")    /* Player 3 */
	AM_RANGE(0x0e0008, 0x0e0009) AM_READ_PORT("DSW1")   /* Dip 1 */
	AM_RANGE(0x0e000a, 0x0e000b) AM_READ_PORT("DSW2")   /* Dip 2 */
	AM_RANGE(0x0e0018, 0x0e0019) AM_READ(sound_status_r)
	AM_RANGE(0x0e8000, 0x0e8001) AM_READWRITE(kludge_r, gun_select_w)
	AM_RANGE(0x0e8002, 0x0e8003) AM_READ(control_3_r)
	/* AM_RANGE(0x0f0008, 0x0f0009) AM_WRITENOP */
	AM_RANGE(0x0f0008, 0x0f0009) AM_WRITE(three_gun_output_w)
	AM_RANGE(0x0f0018, 0x0f0019) AM_WRITE(sound_cpu_w)
	AM_RANGE(0x0f8000, 0x0f80ff) AM_READ(eprom_r) AM_WRITEONLY AM_SHARE("eeprom") /* Eeprom */
ADDRESS_MAP_END

/*******************************************************************************/

static ADDRESS_MAP_START( mechatt_map, AS_PROGRAM, 16, bbusters_state )
	AM_RANGE(0x000000, 0x06ffff) AM_ROM
	AM_RANGE(0x070000, 0x07ffff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x090000, 0x090fff) AM_RAM_WRITE(video_w) AM_SHARE("videoram")
	AM_RANGE(0x0a0000, 0x0a0fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x0a1000, 0x0a7fff) AM_WRITENOP
	AM_RANGE(0x0b0000, 0x0b3fff) AM_RAM_WRITE(pf1_w) AM_SHARE("pf1_data")
	AM_RANGE(0x0b8000, 0x0b8003) AM_WRITEONLY AM_SHARE("pf1_scroll_data")
	AM_RANGE(0x0c0000, 0x0c3fff) AM_RAM_WRITE(pf2_w) AM_SHARE("pf2_data")
	AM_RANGE(0x0c8000, 0x0c8003) AM_WRITEONLY AM_SHARE("pf2_scroll_data")
	AM_RANGE(0x0d0000, 0x0d07ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x0e0000, 0x0e0001) AM_READ_PORT("IN0")
	AM_RANGE(0x0e0002, 0x0e0003) AM_READ_PORT("DSW1")
	AM_RANGE(0x0e0004, 0x0e0007) AM_READ(mechatt_gun_r)
	/* AM_RANGE(0x0e4002, 0x0e4003) AM_WRITENOP  Gun force feedback? */
	AM_RANGE(0x0e4002, 0x0e4003) AM_WRITE(two_gun_output_w)
	AM_RANGE(0x0e8000, 0x0e8001) AM_READWRITE(sound_status_r, sound_cpu_w)
ADDRESS_MAP_END

/******************************************************************************/

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, bbusters_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xf800) AM_READ(soundlatch_byte_r) AM_WRITE(sound_status_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap, AS_IO, 8, bbusters_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ymsnd", ym2610_device, read, write)
	AM_RANGE(0xc0, 0xc1) AM_WRITENOP /* -> Main CPU */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sounda_portmap, AS_IO, 8, bbusters_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ymsnd", ym2608_device, read, write)
	AM_RANGE(0xc0, 0xc1) AM_WRITENOP /* -> Main CPU */
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( bbusters )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Fire")    // "Fire"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Grenade") // "Grenade"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Fire")    // "Fire"
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Grenade") // "Grenade"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("P3 Fire")    // "Fire"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_NAME("P3 Grenade") // "Grenade"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN6 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )       // See notes
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x06, 0x06, "Magazine / Grenade" )        PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x04, "5 / 2" )
	PORT_DIPSETTING(    0x06, "7 / 3" )
	PORT_DIPSETTING(    0x02, "9 / 4" )
	PORT_DIPSETTING(    0x00, "12 / 5" )
	/* Manual (from a different revision/region?) says:
	                    SW1:4   SW1:5   SW1:6
	1C_1C 1 To continue OFF     OFF     OFF
	2C_1C 1 To continue ON      OFF     OFF
	1C_2C 1 To continue OFF     ON      OFF
	2C_1C 2 To continue ON      ON      OFF
	3C_1C 1 To continue OFF     OFF     ON
	3C_1C 2 To continue ON      OFF     ON
	4C_3C 1 To continue OFF     ON      ON
	Free Play Mode      OFF     OFF     OFF

	SW1:7 Unused
	SW1:8 Blood color: ON=green OFF=red */
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" )                PORT_DIPLOCATION("SW1:8") // See notes
	PORT_DIPSETTING(    0x80, "Common" )
	PORT_DIPSETTING(    0x00, "Individual" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Game Mode" )                 PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x0c, "Demo Sounds On" )
	PORT_DIPSETTING(    0x04, "Infinite Energy (Cheat)")
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )            /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )            /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )            /* Listed as "Unused" */
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("GUNX1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("GUNY1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUNX2")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_START("GUNY2")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("GUNX3")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(3)
	PORT_START("GUNY3")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(3)
INPUT_PORTS_END

static INPUT_PORTS_START( mechatt )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )     // See notes
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Fire")  // "Fire"
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Grenade")   // "Grenade"
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Fire")  // "Fire"
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Grenade")   // "Grenade"
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, "Coin Slots" )                PORT_DIPLOCATION("SW1:1") // Listed as "Unused" (manual from different revision/region?), See notes
	PORT_DIPSETTING(      0x0001, "Common" )
	PORT_DIPSETTING(      0x0000, "Individual" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x000c, 0x000c, "Magazine / Grenade" )        PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0008, "5 / 2" )
	PORT_DIPSETTING(      0x000c, "6 / 3" )
	PORT_DIPSETTING(      0x0004, "7 / 4" )
	PORT_DIPSETTING(      0x0000, "8 / 5" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:5,6") // See notes
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:7,8") // Listed as "Unused" (manual from different revision/region?), See notes
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Game Mode" )                 PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, "Demo Sounds Off" )
	PORT_DIPSETTING(      0x0c00, "Demo Sounds On" )
	PORT_DIPSETTING(      0x0400, "Infinite Energy (Cheat)")
	PORT_DIPSETTING(      0x0000, "Freeze" )
	PORT_DIPUNUSED_DIPLOC(0x1000, 0x1000, "SW2:5" )         /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC(0x2000, 0x2000, "SW2:6" )         /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC(0x4000, 0x4000, "SW2:7" )         /* Listed as "Unused" */
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("GUNX1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("GUNY1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUNX2")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_START("GUNY2")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( mechattj )
	PORT_INCLUDE( mechatt )

	PORT_MODIFY("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SW1:1" )
INPUT_PORTS_END

static INPUT_PORTS_START( mechattu )
	PORT_INCLUDE( mechatt )

	PORT_MODIFY("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SW1:1" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0010, "1 Coin/2 Credits first, then 1 Coin/1 Credit" )
	PORT_DIPSETTING(      0x0020, "2 Coins/1 Credit first, then 1 Coin/1 Credit" )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPUNUSED_DIPLOC( 0x00c0, 0x00c0, "SW1:7,8" )
INPUT_PORTS_END


/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0,4,8,12,16,20,24,28 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{
		0, 1, 2, 3,
		16,17,18,19,

		0+32*8, 1+32*8, 2+32*8, 3+32*8,
		16+32*8,17+32*8,18+32*8,19+32*8,
	},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	128*8
};

static const gfx_layout tilelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28,
		0+64*8, 4+64*8, 8+64*8, 12+64*8,
		16+64*8,20+64*8,24+64*8,28+64*8,
	},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	128*8
};

static GFXDECODE_START( bbusters )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 512, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, tilelayout,   768, 16 )
	GFXDECODE_ENTRY( "gfx5", 0, tilelayout,  1024+256, 16 )
GFXDECODE_END

static GFXDECODE_START( mechatt )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 512, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, tilelayout,   512, 16 )
	GFXDECODE_ENTRY( "gfx5", 0, tilelayout,   768, 16 )
GFXDECODE_END


/******************************************************************************/

void bbusters_state::screen_eof_bbuster(screen_device &screen, bool state)
{
	m_spriteram->vblank_copy_rising(screen, state);
	m_spriteram2->vblank_copy_rising(screen, state);
}

static MACHINE_CONFIG_START( bbusters, bbusters_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(bbusters_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", bbusters_state,  irq6_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80,4000000) /* Accurate */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_portmap)

	MCFG_NVRAM_ADD_0FILL("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(bbusters_state, screen_update_bbuster)
	MCFG_SCREEN_VBLANK_DRIVER(bbusters_state, screen_eof_bbuster)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", bbusters)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBxxxx)

	MCFG_VIDEO_START_OVERRIDE(bbusters_state,bbuster)

	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")
	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram2")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2610, 8000000)
	MCFG_YM2610_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "lspeaker",  1.0)
	MCFG_SOUND_ROUTE(0, "rspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "lspeaker",  1.0)
	MCFG_SOUND_ROUTE(2, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( mechatt, bbusters_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(mechatt_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", bbusters_state,  irq4_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80,4000000) /* Accurate */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sounda_portmap)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(bbusters_state, screen_update_mechatt)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mechatt)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBxxxx)

	MCFG_VIDEO_START_OVERRIDE(bbusters_state,mechatt)

	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2608, 8000000)
	MCFG_YM2608_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "lspeaker",  0.50)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "lspeaker",  1.0)
	MCFG_SOUND_ROUTE(2, "rspeaker", 1.0)
MACHINE_CONFIG_END

/******************************************************************************/

ROM_START( bbusters )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bb-3.k10",   0x000000, 0x20000, CRC(04da1820) SHA1(0b6e06adf9c181d7aef28f781efbdd2c225fe81e) )
	ROM_LOAD16_BYTE( "bb-5.k12",   0x000001, 0x20000, CRC(777e0611) SHA1(b7ac0c6ea3738d560a5be75aed286821de918808) )
	ROM_LOAD16_BYTE( "bb-2.k8",    0x040000, 0x20000, CRC(20141805) SHA1(0958579681bda81bcf48d020a14bc147c1e575f1) )
	ROM_LOAD16_BYTE( "bb-4.k11",   0x040001, 0x20000, CRC(d482e0e9) SHA1(e56ca92965e8954b613ba4b0e3975e3a12840c30) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bb-1.e6",     0x000000, 0x10000, CRC(4360f2ee) SHA1(4c6b212f59389bdf4388893d2030493b110ac087) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "bb-10.l9",    0x000000, 0x20000, CRC(490c0d9b) SHA1(567c25a6d96407259c64061d674305e4117d9fa4) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "bb-f11.m16",  0x000000, 0x80000, CRC(39fdf9c0) SHA1(80392947e3a1831c3ee80139f6f3bdc3bafa4f0d) )
	ROM_LOAD( "bb-f12.m13",  0x080000, 0x80000, CRC(69ee046b) SHA1(5c0435f1ce76b584fa8d154d7617d73c7ab5f62f) )
	ROM_LOAD( "bb-f13.m12",  0x100000, 0x80000, CRC(f5ef840e) SHA1(dd0f630c52076e0d330f47931e68a3ae9a401078) )
	ROM_LOAD( "bb-f14.m11",  0x180000, 0x80000, CRC(1a7df3bb) SHA1(1f27a528e6f89fe56a7342c4f1ff733da0a09327) )

	ROM_REGION( 0x200000, "gfx3", 0 )
	ROM_LOAD( "bb-f21.l10",  0x000000, 0x80000, CRC(530f595b) SHA1(820898693b878c4423de9c244f943d39ea69515e) )
	ROM_LOAD( "bb-f22.l12",  0x080000, 0x80000, CRC(889c562e) SHA1(d19172d6515ab9793c98de75d6e41687e61a408d) )
	ROM_LOAD( "bb-f23.l13",  0x100000, 0x80000, CRC(c89fe0da) SHA1(92be860a7191e7473c42aa2da981eda873219d3d) )
	ROM_LOAD( "bb-f24.l15",  0x180000, 0x80000, CRC(e0d81359) SHA1(2213c17651b6c023a456447f352b0739439f913a) )

	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "bb-back1.m4", 0x000000, 0x80000, CRC(b5445313) SHA1(3c99b557b2af30ff0fbc8a7dc6c40448c4f327db) )

	ROM_REGION( 0x80000, "gfx5", 0 )
	ROM_LOAD( "bb-back2.m6", 0x000000, 0x80000, CRC(8be996f6) SHA1(1e2c56f4c24793f806d7b366b92edc03145ae94c) )

	ROM_REGION( 0x10000, "user1", 0 ) /* Zoom table */
	/* same rom exists in 4 different locations on the board */
	ROM_LOAD( "bb-6.e7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-7.h7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-8.a14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-9.c14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )

	ROM_REGION( 0x80000, "ymsnd", 0 )
	ROM_LOAD( "bb-pcma.l5",  0x000000, 0x80000, CRC(44cd5bfe) SHA1(26a612191a0aa614c090203485aba17c99c763ee) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )
	ROM_LOAD( "bb-pcmb.l3",  0x000000, 0x80000, CRC(c8d5dd53) SHA1(0f7e94532cc14852ca12c1b792e5479667af899e) )

	ROM_REGION( 0x100, "eeprom", 0 )
	ROM_LOAD( "bbusters-eeprom.bin", 0x00, 0x100, CRC(a52ebd66) SHA1(de04db6f1510700c61bf152799452a80220ae87c) )
ROM_END

ROM_START( bbustersu )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bb-ver3-u3.k10", 0x000000, 0x20000, CRC(c80ec3bc) SHA1(81cccc920c6dc58ccd20fb38bfede717f534986f) )
	ROM_LOAD16_BYTE( "bb-ver3-u5.k12", 0x000001, 0x20000, CRC(5ded86d1) SHA1(de2ce91b85a1d74e60a7093211c1a7d3c27c1d72) )
	ROM_LOAD16_BYTE( "bb-2.k8",        0x040000, 0x20000, CRC(20141805) SHA1(0958579681bda81bcf48d020a14bc147c1e575f1) )
	ROM_LOAD16_BYTE( "bb-4.k11",       0x040001, 0x20000, CRC(d482e0e9) SHA1(e56ca92965e8954b613ba4b0e3975e3a12840c30) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bb-1.e6",     0x000000, 0x10000, CRC(4360f2ee) SHA1(4c6b212f59389bdf4388893d2030493b110ac087) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "bb-10.l9",    0x000000, 0x20000, CRC(490c0d9b) SHA1(567c25a6d96407259c64061d674305e4117d9fa4) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "bb-f11.m16",  0x000000, 0x80000, CRC(39fdf9c0) SHA1(80392947e3a1831c3ee80139f6f3bdc3bafa4f0d) )
	ROM_LOAD( "bb-f12.m13",  0x080000, 0x80000, CRC(69ee046b) SHA1(5c0435f1ce76b584fa8d154d7617d73c7ab5f62f) )
	ROM_LOAD( "bb-f13.m12",  0x100000, 0x80000, CRC(f5ef840e) SHA1(dd0f630c52076e0d330f47931e68a3ae9a401078) )
	ROM_LOAD( "bb-f14.m11",  0x180000, 0x80000, CRC(1a7df3bb) SHA1(1f27a528e6f89fe56a7342c4f1ff733da0a09327) )

	ROM_REGION( 0x200000, "gfx3", 0 )
	ROM_LOAD( "bb-f21.l10",  0x000000, 0x80000, CRC(530f595b) SHA1(820898693b878c4423de9c244f943d39ea69515e) )
	ROM_LOAD( "bb-f22.l12",  0x080000, 0x80000, CRC(889c562e) SHA1(d19172d6515ab9793c98de75d6e41687e61a408d) )
	ROM_LOAD( "bb-f23.l13",  0x100000, 0x80000, CRC(c89fe0da) SHA1(92be860a7191e7473c42aa2da981eda873219d3d) )
	ROM_LOAD( "bb-f24.l15",  0x180000, 0x80000, CRC(e0d81359) SHA1(2213c17651b6c023a456447f352b0739439f913a) )

	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "bb-back1.m4", 0x000000, 0x80000, CRC(b5445313) SHA1(3c99b557b2af30ff0fbc8a7dc6c40448c4f327db) )

	ROM_REGION( 0x80000, "gfx5", 0 )
	ROM_LOAD( "bb-back2.m6", 0x000000, 0x80000, CRC(8be996f6) SHA1(1e2c56f4c24793f806d7b366b92edc03145ae94c) )

	ROM_REGION( 0x10000, "user1", 0 ) /* Zoom table */
	/* same rom exists in 4 different locations on the board */
	ROM_LOAD( "bb-6.e7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-7.h7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-8.a14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-9.c14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )

	ROM_REGION( 0x80000, "ymsnd", 0 )
	ROM_LOAD( "bb-pcma.l5",  0x000000, 0x80000, CRC(44cd5bfe) SHA1(26a612191a0aa614c090203485aba17c99c763ee) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )
	ROM_LOAD( "bb-pcma.l5",  0x000000, 0x80000, CRC(44cd5bfe) SHA1(26a612191a0aa614c090203485aba17c99c763ee) )

	ROM_REGION( 0x100, "eeprom", 0 )
	ROM_LOAD( "bbusters-eeprom.bin", 0x00, 0x100, CRC(a52ebd66) SHA1(de04db6f1510700c61bf152799452a80220ae87c) )
ROM_END


ROM_START( bbustersj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bb3_ver2_j2.k10",   0x000000, 0x20000, CRC(605eb62f) SHA1(b13afd561731ad9115c5b997b8a7a79a57557612) )
	ROM_LOAD16_BYTE( "bb5_ver2_j2.k12",   0x000001, 0x20000, CRC(9deea26f) SHA1(c5436db0c55da9b0c5e0e053f59a1e17ee4690a6) )
	ROM_LOAD16_BYTE( "bb-2.k8",    0x040000, 0x20000, CRC(20141805) SHA1(0958579681bda81bcf48d020a14bc147c1e575f1) )
	ROM_LOAD16_BYTE( "bb-4.k11",   0x040001, 0x20000, CRC(d482e0e9) SHA1(e56ca92965e8954b613ba4b0e3975e3a12840c30) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bb-1.e6",     0x000000, 0x10000, CRC(4360f2ee) SHA1(4c6b212f59389bdf4388893d2030493b110ac087) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "bb-10.l9",    0x000000, 0x20000, CRC(490c0d9b) SHA1(567c25a6d96407259c64061d674305e4117d9fa4) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "bb-f11.m16",  0x000000, 0x80000, CRC(39fdf9c0) SHA1(80392947e3a1831c3ee80139f6f3bdc3bafa4f0d) )
	ROM_LOAD( "bb-f12.m13",  0x080000, 0x80000, CRC(69ee046b) SHA1(5c0435f1ce76b584fa8d154d7617d73c7ab5f62f) )
	ROM_LOAD( "bb-f13.m12",  0x100000, 0x80000, CRC(f5ef840e) SHA1(dd0f630c52076e0d330f47931e68a3ae9a401078) )
	ROM_LOAD( "bb-f14.m11",  0x180000, 0x80000, CRC(1a7df3bb) SHA1(1f27a528e6f89fe56a7342c4f1ff733da0a09327) )

	ROM_REGION( 0x200000, "gfx3", 0 )
	ROM_LOAD( "bb-f21.l10",  0x000000, 0x80000, CRC(530f595b) SHA1(820898693b878c4423de9c244f943d39ea69515e) )
	ROM_LOAD( "bb-f22.l12",  0x080000, 0x80000, CRC(889c562e) SHA1(d19172d6515ab9793c98de75d6e41687e61a408d) )
	ROM_LOAD( "bb-f23.l13",  0x100000, 0x80000, CRC(c89fe0da) SHA1(92be860a7191e7473c42aa2da981eda873219d3d) )
	ROM_LOAD( "bb-f24.l15",  0x180000, 0x80000, CRC(e0d81359) SHA1(2213c17651b6c023a456447f352b0739439f913a) )

	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "bb-back1.m4", 0x000000, 0x80000, CRC(b5445313) SHA1(3c99b557b2af30ff0fbc8a7dc6c40448c4f327db) )

	ROM_REGION( 0x80000, "gfx5", 0 )
	ROM_LOAD( "bb-back2.m6", 0x000000, 0x80000, CRC(8be996f6) SHA1(1e2c56f4c24793f806d7b366b92edc03145ae94c) )

	ROM_REGION( 0x10000, "user1", 0 ) /* Zoom table */
	/* same rom exists in 4 different locations on the board */
	ROM_LOAD( "bb-6.e7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-7.h7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-8.a14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-9.c14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )

	ROM_REGION( 0x80000, "ymsnd", 0 )
	ROM_LOAD( "bb-pcma.l5",  0x000000, 0x80000, CRC(44cd5bfe) SHA1(26a612191a0aa614c090203485aba17c99c763ee) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )
	ROM_LOAD( "bb-pcmb.l3",  0x000000, 0x80000, CRC(c8d5dd53) SHA1(0f7e94532cc14852ca12c1b792e5479667af899e) )

	ROM_REGION( 0x100, "eeprom", 0 )
	ROM_LOAD( "bbusters-eeprom.bin", 0x00, 0x100, CRC(a52ebd66) SHA1(de04db6f1510700c61bf152799452a80220ae87c) )
ROM_END

ROM_START( bbustersua )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bb-ver2-u3.k10", 0x000000, 0x20000, CRC(6930088b) SHA1(265f0b584d81b6fdcda5c3a2e0bd15d56443bb35) )
	ROM_LOAD16_BYTE( "bb-ver2-u5.k12", 0x000001, 0x20000, CRC(cfdb2c6c) SHA1(54a837dc84b74d12e931f607f3dc9ee06a7e4d31) )
	ROM_LOAD16_BYTE( "bb-2.k8",        0x040000, 0x20000, CRC(20141805) SHA1(0958579681bda81bcf48d020a14bc147c1e575f1) )
	ROM_LOAD16_BYTE( "bb-4.k11",       0x040001, 0x20000, CRC(d482e0e9) SHA1(e56ca92965e8954b613ba4b0e3975e3a12840c30) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bb-1.e6",     0x000000, 0x10000, CRC(4360f2ee) SHA1(4c6b212f59389bdf4388893d2030493b110ac087) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "bb-10.l9",    0x000000, 0x20000, CRC(490c0d9b) SHA1(567c25a6d96407259c64061d674305e4117d9fa4) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "bb-f11.m16",  0x000000, 0x80000, CRC(39fdf9c0) SHA1(80392947e3a1831c3ee80139f6f3bdc3bafa4f0d) )
	ROM_LOAD( "bb-f12.m13",  0x080000, 0x80000, CRC(69ee046b) SHA1(5c0435f1ce76b584fa8d154d7617d73c7ab5f62f) )
	ROM_LOAD( "bb-f13.m12",  0x100000, 0x80000, CRC(f5ef840e) SHA1(dd0f630c52076e0d330f47931e68a3ae9a401078) )
	ROM_LOAD( "bb-f14.m11",  0x180000, 0x80000, CRC(1a7df3bb) SHA1(1f27a528e6f89fe56a7342c4f1ff733da0a09327) )

	ROM_REGION( 0x200000, "gfx3", 0 )
	ROM_LOAD( "bb-f21.l10",  0x000000, 0x80000, CRC(530f595b) SHA1(820898693b878c4423de9c244f943d39ea69515e) )
	ROM_LOAD( "bb-f22.l12",  0x080000, 0x80000, CRC(889c562e) SHA1(d19172d6515ab9793c98de75d6e41687e61a408d) )
	ROM_LOAD( "bb-f23.l13",  0x100000, 0x80000, CRC(c89fe0da) SHA1(92be860a7191e7473c42aa2da981eda873219d3d) )
	ROM_LOAD( "bb-f24.l15",  0x180000, 0x80000, CRC(e0d81359) SHA1(2213c17651b6c023a456447f352b0739439f913a) )

	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "bb-back1.m4", 0x000000, 0x80000, CRC(b5445313) SHA1(3c99b557b2af30ff0fbc8a7dc6c40448c4f327db) )

	ROM_REGION( 0x80000, "gfx5", 0 )
	ROM_LOAD( "bb-back2.m6", 0x000000, 0x80000, CRC(8be996f6) SHA1(1e2c56f4c24793f806d7b366b92edc03145ae94c) )

	ROM_REGION( 0x10000, "user1", 0 ) /* Zoom table */
	/* same rom exists in 4 different locations on the board */
	ROM_LOAD( "bb-6.e7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-7.h7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-8.a14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-9.c14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )

	ROM_REGION( 0x80000, "ymsnd", 0 )
	ROM_LOAD( "bb-pcma.l5",  0x000000, 0x80000, CRC(44cd5bfe) SHA1(26a612191a0aa614c090203485aba17c99c763ee) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )
	ROM_LOAD( "bb-pcma.l5",  0x000000, 0x80000, CRC(44cd5bfe) SHA1(26a612191a0aa614c090203485aba17c99c763ee) )

	ROM_REGION( 0x100, "eeprom", 0 )
	ROM_LOAD( "bbusters-eeprom.bin", 0x00, 0x100, CRC(a52ebd66) SHA1(de04db6f1510700c61bf152799452a80220ae87c) )
ROM_END


ROM_START( mechatt )
	ROM_REGION( 0x80000, "maincpu", 0 ) // Located on the A8002-1 main board
	ROM_LOAD16_BYTE( "ma5-e.n12", 0x000000, 0x20000, CRC(9bbb852a) SHA1(34b696bf79cf53cac1c384a3143c0f3f243a71f3) )
	ROM_LOAD16_BYTE( "ma4.l12",   0x000001, 0x20000, CRC(0d414918) SHA1(0d51b893d37ba124b983beebb691e65bdc52d300) )
	ROM_LOAD16_BYTE( "ma7.n13",   0x040000, 0x20000, CRC(61d85e1b) SHA1(46234d48ac21c481a5e70c6a654a341ebdd4cd3a) )
	ROM_LOAD16_BYTE( "ma6-f.l13", 0x040001, 0x20000, CRC(4055fe8d) SHA1(b4d8bd5f73805ce1c332eff657dddbb88ff45b37) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Located on the A8002-1 main board
	ROM_LOAD( "ma_3.e13", 0x000000, 0x10000, CRC(c06cc8e1) SHA1(65f5f1901120d633f7c3ba07432a188fd7fd7272) )

	ROM_REGION( 0x020000, "gfx1", 0 ) // Located on the A8002-2 board
	ROM_LOAD( "ma_1.l2", 0x000000, 0x10000, CRC(24766917) SHA1(9082a8ae849605ce65b5a0493ae69cfe282f7e7b) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // Located on the A8002-2 board
	ROM_LOAD( "mao89p13.bin",  0x000000, 0x80000, CRC(8bcb16cf) SHA1(409ee1944188d9ce39adce29b1df029b560dd5b0) )
	ROM_LOAD( "ma189p15.bin",  0x080000, 0x80000, CRC(b84d9658) SHA1(448adecb0067d8f5b219ec2f94a8dec84187a554) )
	ROM_LOAD( "ma289p17.bin",  0x100000, 0x80000, CRC(6cbe08ac) SHA1(8f81f6e92b84ab6867452011d52f3e7689c62a1a) )
	ROM_LOAD( "ma389m15.bin",  0x180000, 0x80000, CRC(34d4585e) SHA1(38d9fd5d775e4b3c8b8b487a6ba9b8bdcb3274b0) )

	ROM_REGION( 0x10000, "gfx3", ROMREGION_ERASEFF )
	/* Unused */

	ROM_REGION( 0x80000, "gfx4", 0 ) // Located on the A8002-2 board
	ROM_LOAD( "mab189a2.bin",  0x000000, 0x80000, CRC(e1c8b4d0) SHA1(2f8a1839cca892f8380c7cffe7a12e615d38fd55) )

	ROM_REGION( 0x80000, "gfx5", 0 ) // Located on the A8002-2 board
	ROM_LOAD( "mab289c2.bin",  0x000000, 0x80000, CRC(14f97ceb) SHA1(a22033532ea616dc3a3db8b66ad6ccc6172ed7cc) )

	ROM_REGION( 0x20000, "ymsnd", 0 ) // Located on the A8002-1 main board
	ROM_LOAD( "ma_2.d10", 0x000000, 0x20000, CRC(ea4cc30d) SHA1(d8f089fc0ce76309411706a8110ad907f93dc97e) )

	ROM_REGION( 0x20000, "user1", 0 ) // Zoom table - Located on the A8002-2 board
	ROM_LOAD( "ma_8.f10", 0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "ma_9.f12", 0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) ) // identical to ma_8.f10
ROM_END

ROM_START( mechattj ) // Uses EPROMs on official SNK A8002-5 & A8002-6 sub boards instead of MaskROMs
	ROM_REGION( 0x80000, "maincpu", 0 ) // Located on the A8002-1 main board
	ROM_LOAD16_BYTE( "ma5j.n12", 0x000000, 0x20000, CRC(e6bb5952) SHA1(3b01eccc20d99fd33ff8e303afa902abb66e1036) )
	ROM_LOAD16_BYTE( "ma4j.l12", 0x000001, 0x20000, CRC(c78baa62) SHA1(c3554698fbc94e3625269c5cb1fc664364f3fb3f) )
	ROM_LOAD16_BYTE( "ma7j.n13", 0x040000, 0x20000, CRC(12a68fc2) SHA1(c935788723d8ea3bfe99244b8c7b2aff85579912) )
	ROM_LOAD16_BYTE( "ma6j.l13", 0x040001, 0x20000, CRC(332b2f54) SHA1(c768f5437a20ea406523d3de9e1ea807b39e1622) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Located on the A8002-1 main board
	ROM_LOAD( "ma_3.e13", 0x000000, 0x10000, CRC(c06cc8e1) SHA1(65f5f1901120d633f7c3ba07432a188fd7fd7272) )

	ROM_REGION( 0x020000, "gfx1", 0 ) // Located on the A8002-2 board
	ROM_LOAD( "ma_1.l2", 0x000000, 0x10000, CRC(24766917) SHA1(9082a8ae849605ce65b5a0493ae69cfe282f7e7b) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // Located on the A8002-6 sub board
	ROM_LOAD16_BYTE( "s_9.a1",  0x000000, 0x20000, CRC(6e8e194c) SHA1(02bbd573a322a3f7f8e92ccceebffdd598b5489e) ) // these 4 == mao89p13.bin
	ROM_LOAD16_BYTE( "s_1.b1",  0x000001, 0x20000, CRC(fd9161ed) SHA1(b3e2434dd9cb1cafe1022774b863b5f1a008a9d2) )
	ROM_LOAD16_BYTE( "s_10.a2", 0x040000, 0x20000, CRC(fad6a1ab) SHA1(5347b4493c8004dc8cedc0b37aba494f203142b8) )
	ROM_LOAD16_BYTE( "s_2.b2",  0x040001, 0x20000, CRC(549056f0) SHA1(f515aa98ab25f3735dbfdefcb8d55ba3b2075b70) )
	ROM_LOAD16_BYTE( "s_11.a3", 0x080000, 0x20000, CRC(3887a382) SHA1(b40861fc1414b2fa299772e76a78cb8dc00b71b7) ) // these 4 == ma189p15.bin
	ROM_LOAD16_BYTE( "s_3.b3",  0x080001, 0x20000, CRC(cb99f565) SHA1(9ed1b21f4a33b9a614bca38610378857560cdaba) )
	ROM_LOAD16_BYTE( "s_12.a4", 0x0c0000, 0x20000, CRC(63417b49) SHA1(786249fa7e8770de5b5882debdc2913d58e9170e) )
	ROM_LOAD16_BYTE( "s_4.b4",  0x0c0001, 0x20000, CRC(d739d48a) SHA1(04d2ecea72b6e651b815865946c9c9cfae4e5c4d) )
	ROM_LOAD16_BYTE( "s_13.a5", 0x100000, 0x20000, CRC(eccd47b6) SHA1(6b9c63fee97a7568114f227a89a1effd6b04806a) ) // these 4 == ma289p17.bin
	ROM_LOAD16_BYTE( "s_5.b5",  0x100001, 0x20000, CRC(e15244da) SHA1(ebf3072565c53d0098d373b5093ba6918c4eddae) )
	ROM_LOAD16_BYTE( "s_14.a6", 0x140000, 0x20000, CRC(bbbf0461) SHA1(c5299ab1d45f685a5d160492247cf1303ef6937a) )
	ROM_LOAD16_BYTE( "s_6.b6",  0x140001, 0x20000, CRC(4ee89f75) SHA1(bda0e9095da2d424faac341fd934000a621796eb) )
	ROM_LOAD16_BYTE( "s_15.a7", 0x180000, 0x20000, CRC(cde29bad) SHA1(24c1b43c6d717eaaf7c01ec7de89837947334224) ) // these 4 == ma389m15.bin
	ROM_LOAD16_BYTE( "s_7.b7",  0x180001, 0x20000, CRC(065ed221) SHA1(c03ca5b4d1198939a57b5fccf6a79d70afe1faaf) )
	ROM_LOAD16_BYTE( "s_16.a8", 0x1c0000, 0x20000, CRC(70f28040) SHA1(91012728953563fcc576725337e6ba7e1b49d1ba) )
	ROM_LOAD16_BYTE( "s_8.b8",  0x1c0001, 0x20000, CRC(a6f8574f) SHA1(87c041669b2eaec495ae10a6f45b6668accb92bf) )

	ROM_REGION( 0x10000, "gfx3", ROMREGION_ERASEFF )
	/* Unused */

	ROM_REGION( 0x80000, "gfx4", 0 ) // these 4 == mab189a2.bin - Located on the A8002-5 sub board
	ROM_LOAD( "s_21.b3", 0x000000, 0x20000, CRC(701a0072) SHA1(b03b6fa18e0cfcd5c7c541025fa2d3632d2f8387) )
	ROM_LOAD( "s_22.b4", 0x020000, 0x20000, CRC(34e6225c) SHA1(f6335084f4f4c7a4b6528e6ad74962b88f81e3bc) )
	ROM_LOAD( "s_23.b5", 0x040000, 0x20000, CRC(9a7399d3) SHA1(04e0327b0da75f621b51e1831cbdc4537082e32b) )
	ROM_LOAD( "s_24.b6", 0x060000, 0x20000, CRC(f097459d) SHA1(466364677f048519eb2894ddecf76f5c52f6afe9) )

	ROM_REGION( 0x80000, "gfx5", 0 ) // these 4 == mab289c2.bin - Located on the A8002-5 sub board
	ROM_LOAD( "s_17.a3", 0x000000, 0x20000, CRC(cc47c4a3) SHA1(140f53b671b4eaed6fcc516c4018f07a6d7c2290) )
	ROM_LOAD( "s_18.a4", 0x020000, 0x20000, CRC(a04377e8) SHA1(841c6c3073b137f6a5c875db32039186c014f785) )
	ROM_LOAD( "s_19.a5", 0x040000, 0x20000, CRC(b07f5289) SHA1(8817bd225edf9b0fa439b220617f925365e39253) )
	ROM_LOAD( "s_20.a6", 0x060000, 0x20000, CRC(a9bb4fa9) SHA1(ccede784671a864667b92a8101d686c26c78d76f) )

	ROM_REGION( 0x20000, "ymsnd", 0 ) // Located on the A8002-1 main board
	ROM_LOAD( "ma_2.d10", 0x000000, 0x20000, CRC(ea4cc30d) SHA1(d8f089fc0ce76309411706a8110ad907f93dc97e) )

	ROM_REGION( 0x20000, "user1", 0 ) // Zoom table - Located on the A8002-2 board
	ROM_LOAD( "ma_8.f10", 0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "ma_9.f12", 0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) ) // identical to ma_8.f10
ROM_END

ROM_START( mechattu )
	ROM_REGION( 0x80000, "maincpu", 0 ) // Located on the A8002-1 main board
	ROM_LOAD16_BYTE( "ma5u.n12", 0x000000, 0x20000, CRC(485ea606) SHA1(0c499f08d7c6d861ba7c50a8f577823613a7923c) )
	ROM_LOAD16_BYTE( "ma4u.l12", 0x000001, 0x20000, CRC(09fa31ec) SHA1(008abb2e09f83614c277471e534f20cba3e354d7) )
	ROM_LOAD16_BYTE( "ma7u.n13", 0x040000, 0x20000, CRC(f45b2c70) SHA1(65523d202d378bab890f1f7bffdde152dd246d4a) )
	ROM_LOAD16_BYTE( "ma6u.l13", 0x040001, 0x20000, CRC(d5d68ce6) SHA1(16057d882781015f6d1c7bb659e0812a8459c3f0) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Located on the A8002-1 main board
	ROM_LOAD( "ma_3.e13", 0x000000, 0x10000, CRC(c06cc8e1) SHA1(65f5f1901120d633f7c3ba07432a188fd7fd7272) )

	ROM_REGION( 0x020000, "gfx1", 0 ) // Located on the A8002-2 board
	ROM_LOAD( "ma_1.l2", 0x000000, 0x10000, CRC(24766917) SHA1(9082a8ae849605ce65b5a0493ae69cfe282f7e7b) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // Located on the A8002-2 board
	ROM_LOAD( "mao89p13.bin",  0x000000, 0x80000, CRC(8bcb16cf) SHA1(409ee1944188d9ce39adce29b1df029b560dd5b0) )
	ROM_LOAD( "ma189p15.bin",  0x080000, 0x80000, CRC(b84d9658) SHA1(448adecb0067d8f5b219ec2f94a8dec84187a554) )
	ROM_LOAD( "ma289p17.bin",  0x100000, 0x80000, CRC(6cbe08ac) SHA1(8f81f6e92b84ab6867452011d52f3e7689c62a1a) )
	ROM_LOAD( "ma389m15.bin",  0x180000, 0x80000, CRC(34d4585e) SHA1(38d9fd5d775e4b3c8b8b487a6ba9b8bdcb3274b0) )

	ROM_REGION( 0x10000, "gfx3", ROMREGION_ERASEFF )
	/* Unused */

	ROM_REGION( 0x80000, "gfx4", 0 ) // Located on the A8002-2 board
	ROM_LOAD( "mab189a2.bin",  0x000000, 0x80000, CRC(e1c8b4d0) SHA1(2f8a1839cca892f8380c7cffe7a12e615d38fd55) )

	ROM_REGION( 0x80000, "gfx5", 0 ) // Located on the A8002-2 board
	ROM_LOAD( "mab289c2.bin",  0x000000, 0x80000, CRC(14f97ceb) SHA1(a22033532ea616dc3a3db8b66ad6ccc6172ed7cc) )

	ROM_REGION( 0x20000, "ymsnd", 0 ) // Located on the A8002-1 main board
	ROM_LOAD( "ma_2.d10", 0x000000, 0x20000, CRC(ea4cc30d) SHA1(d8f089fc0ce76309411706a8110ad907f93dc97e) )

	ROM_REGION( 0x20000, "user1", 0 ) // Zoom table - Located on the A8002-2 board
	ROM_LOAD( "ma_8.f10", 0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "ma_9.f12", 0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) ) // identical to ma_8.f10
ROM_END

/* does Ver1 on the roms mean it's a revision, the first version, or used because it's the single player version? */
ROM_START( mechattu1 ) // Uses EPROMs on official SNK A8002-5 & A8002-6 sub boards instead of MaskROMs
	ROM_REGION( 0x80000, "maincpu", 0 ) // Located on the A8002-1 main board
	ROM_LOAD16_BYTE( "ma_ver1_5u.n12", 0x000000, 0x20000, CRC(dcd2e971) SHA1(e292b251c429b6990e97233e86360e5d43f573f2) )
	ROM_LOAD16_BYTE( "ma_ver1_4u.l12", 0x000001, 0x20000, CRC(69c8a85b) SHA1(07c6d395772a5e096e3ac42c5248eadccc146ad1) )
	ROM_LOAD16_BYTE( "ma7u.n13",       0x040000, 0x20000, CRC(f45b2c70) SHA1(65523d202d378bab890f1f7bffdde152dd246d4a) )
	ROM_LOAD16_BYTE( "ma6u.l13",       0x040001, 0x20000, CRC(d5d68ce6) SHA1(16057d882781015f6d1c7bb659e0812a8459c3f0) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Located on the A8002-1 main board
	ROM_LOAD( "ma_3.e13", 0x000000, 0x10000, CRC(c06cc8e1) SHA1(65f5f1901120d633f7c3ba07432a188fd7fd7272) )

	ROM_REGION( 0x020000, "gfx1", 0 ) // Located on the A8002-2 board
	ROM_LOAD( "ma_1.l2", 0x000000, 0x10000, CRC(24766917) SHA1(9082a8ae849605ce65b5a0493ae69cfe282f7e7b) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // Located on the A8002-6 sub board
	ROM_LOAD16_BYTE( "s_9.a1",  0x000000, 0x20000, CRC(6e8e194c) SHA1(02bbd573a322a3f7f8e92ccceebffdd598b5489e) ) // these 4 == mao89p13.bin
	ROM_LOAD16_BYTE( "s_1.b1",  0x000001, 0x20000, CRC(fd9161ed) SHA1(b3e2434dd9cb1cafe1022774b863b5f1a008a9d2) )
	ROM_LOAD16_BYTE( "s_10.a2", 0x040000, 0x20000, CRC(fad6a1ab) SHA1(5347b4493c8004dc8cedc0b37aba494f203142b8) )
	ROM_LOAD16_BYTE( "s_2.b2",  0x040001, 0x20000, CRC(549056f0) SHA1(f515aa98ab25f3735dbfdefcb8d55ba3b2075b70) )
	ROM_LOAD16_BYTE( "s_11.a3", 0x080000, 0x20000, CRC(3887a382) SHA1(b40861fc1414b2fa299772e76a78cb8dc00b71b7) ) // these 4 == ma189p15.bin
	ROM_LOAD16_BYTE( "s_3.b3",  0x080001, 0x20000, CRC(cb99f565) SHA1(9ed1b21f4a33b9a614bca38610378857560cdaba) )
	ROM_LOAD16_BYTE( "s_12.a4", 0x0c0000, 0x20000, CRC(63417b49) SHA1(786249fa7e8770de5b5882debdc2913d58e9170e) )
	ROM_LOAD16_BYTE( "s_4.b4",  0x0c0001, 0x20000, CRC(d739d48a) SHA1(04d2ecea72b6e651b815865946c9c9cfae4e5c4d) )
	ROM_LOAD16_BYTE( "s_13.a5", 0x100000, 0x20000, CRC(eccd47b6) SHA1(6b9c63fee97a7568114f227a89a1effd6b04806a) ) // these 4 == ma289p17.bin
	ROM_LOAD16_BYTE( "s_5.b5",  0x100001, 0x20000, CRC(e15244da) SHA1(ebf3072565c53d0098d373b5093ba6918c4eddae) )
	ROM_LOAD16_BYTE( "s_14.a6", 0x140000, 0x20000, CRC(bbbf0461) SHA1(c5299ab1d45f685a5d160492247cf1303ef6937a) )
	ROM_LOAD16_BYTE( "s_6.b6",  0x140001, 0x20000, CRC(4ee89f75) SHA1(bda0e9095da2d424faac341fd934000a621796eb) )
	ROM_LOAD16_BYTE( "s_15.a7", 0x180000, 0x20000, CRC(cde29bad) SHA1(24c1b43c6d717eaaf7c01ec7de89837947334224) ) // these 4 == ma389m15.bin
	ROM_LOAD16_BYTE( "s_7.b7",  0x180001, 0x20000, CRC(065ed221) SHA1(c03ca5b4d1198939a57b5fccf6a79d70afe1faaf) )
	ROM_LOAD16_BYTE( "s_16.a8", 0x1c0000, 0x20000, CRC(70f28040) SHA1(91012728953563fcc576725337e6ba7e1b49d1ba) )
	ROM_LOAD16_BYTE( "s_8.b8",  0x1c0001, 0x20000, CRC(a6f8574f) SHA1(87c041669b2eaec495ae10a6f45b6668accb92bf) )

	ROM_REGION( 0x10000, "gfx3", ROMREGION_ERASEFF )
	/* Unused */

	ROM_REGION( 0x80000, "gfx4", 0 ) // these 4 == mab189a2.bin - Located on the A8002-5 sub board
	ROM_LOAD( "s_21.b3", 0x000000, 0x20000, CRC(701a0072) SHA1(b03b6fa18e0cfcd5c7c541025fa2d3632d2f8387) )
	ROM_LOAD( "s_22.b4", 0x020000, 0x20000, CRC(34e6225c) SHA1(f6335084f4f4c7a4b6528e6ad74962b88f81e3bc) )
	ROM_LOAD( "s_23.b5", 0x040000, 0x20000, CRC(9a7399d3) SHA1(04e0327b0da75f621b51e1831cbdc4537082e32b) )
	ROM_LOAD( "s_24.b6", 0x060000, 0x20000, CRC(f097459d) SHA1(466364677f048519eb2894ddecf76f5c52f6afe9) )

	ROM_REGION( 0x80000, "gfx5", 0 ) // these 4 == mab289c2.bin - Located on the A8002-5 sub board
	ROM_LOAD( "s_17.a3", 0x000000, 0x20000, CRC(cc47c4a3) SHA1(140f53b671b4eaed6fcc516c4018f07a6d7c2290) )
	ROM_LOAD( "s_18.a4", 0x020000, 0x20000, CRC(a04377e8) SHA1(841c6c3073b137f6a5c875db32039186c014f785) )
	ROM_LOAD( "s_19.a5", 0x040000, 0x20000, CRC(b07f5289) SHA1(8817bd225edf9b0fa439b220617f925365e39253) )
	ROM_LOAD( "s_20.a6", 0x060000, 0x20000, CRC(a9bb4fa9) SHA1(ccede784671a864667b92a8101d686c26c78d76f) )

	ROM_REGION( 0x20000, "ymsnd", 0 ) // Located on the A8002-1 main board
	ROM_LOAD( "ma_2.d10", 0x000000, 0x20000, CRC(ea4cc30d) SHA1(d8f089fc0ce76309411706a8110ad907f93dc97e) )

	ROM_REGION( 0x20000, "user1", 0 ) // Zoom table - Located on the A8002-2 board
	ROM_LOAD( "ma_8.f10", 0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "ma_9.f12", 0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) ) // identical to ma_8.f10
ROM_END


/******************************************************************************/

// as soon as you calibrate the guns in test mode the game refuses to boot
GAME( 1989, bbusters,   0,        bbusters, bbusters, driver_device, 0, ROT0,  "SNK", "Beast Busters (World)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1989, bbustersu,  bbusters, bbusters, bbusters, driver_device, 0, ROT0,  "SNK", "Beast Busters (US, Version 3)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1989, bbustersua, bbusters, bbusters, bbusters, driver_device, 0, ROT0,  "SNK", "Beast Busters (US, Version 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1989, bbustersj,  bbusters, bbusters, bbusters, driver_device, 0, ROT0,  "SNK", "Beast Busters (Japan, Version 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1989, mechatt,    0,        mechatt,  mechatt,  driver_device, 0, ROT0,  "SNK", "Mechanized Attack (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, mechattj,   mechatt,  mechatt,  mechattj, driver_device, 0, ROT0,  "SNK", "Mechanized Attack (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, mechattu,   mechatt,  mechatt,  mechattu, driver_device, 0, ROT0,  "SNK", "Mechanized Attack (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, mechattu1,  mechatt,  mechatt,  mechattu, driver_device, 0, ROT0,  "SNK", "Mechanized Attack (US, Version 1, Single Player)", MACHINE_SUPPORTS_SAVE )
