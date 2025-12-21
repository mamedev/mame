// license:BSD-3-Clause
// copyright-holders:Quench, Yochizo, David Haywood

#include "emu.h"

#include "raizing.h"

// TODO: this is the old toaplan2.cpp header text, it could do with splitting between the new drivers

/*****************************************************************************

        ToaPlan      game hardware from 1991 - 1994
        Raizing/8ing game hardware from 1993 onwards
        -------------------------------------------------
        Driver by: Quench and Yochizo
        Original othldrby.c by Nicola Salmoria

   Raizing games and Truxton 2 are heavily dependent on the Raine source -
   many thanks to Richard Bush and the Raine team. [Yochizo]


Supported games:

    Name        Board No      Maker         Game name
    ----------------------------------------------------------------------------
    sstriker    RA-MA7893-01  Raizing       Sorcer Striker
    sstrikera   RA-MA7893-01  Raizing       Sorcer Striker (Unite Trading license)
    mahoudai    RA-MA7893-01  Raizing       Mahou Daisakusen (Japan)
    kingdmgp    RA-MA9402-03  Raizing/8ing  Kingdom Grandprix
    shippumd    RA-MA9402-03  Raizing/8ing  Shippu Mahou Daisakusen (Japan)
    bgaregga    RA9503        Raizing/8ing  Battle Garegga (World - Sat Feb 3 1996)
    bgareggat   RA9503        Raizing/8ing  Battle Garegga (location test - Wed Jan 17 1996)
    bgareggahk  RA9503        Raizing/8ing  Battle Garegga (Hong Kong (and Austria?) - Sat Feb 3 1996)
    bgareggatw  RA9503        Raizing/8ing  Battle Garegga (Taiwan (and Germany?) - Thu Feb 1 1996)
    bgaregganv  RA9503        Raizing/8ing  Battle Garegga - New Version (Hong Kong (and Austria?) - Sat Mar 2 1996)
    bgareggat2  RA9503        Raizing/8ing  Battle Garegga - Type 2 (World - Sat Mar 2 1996)
    bgareggacn  RA9503        Raizing/8ing  Battle Garegga - Type 2 (China (and Denmark?) - Tue Apr 2 1996)

    SET NOTES:

    sstriker - The mahoudai set reads the region jumpers, but the lookup tables in ROM that map jumper
               settings to copyright text, coinage settings, etc., contain identical values for every
               jumper setting, effectively ignoring the jumpers and forcing the region to Japan.
               On the other hand, sstriker has its title screen and all its text in English even when
               the region is set to Japan. This seems odd but has been verified correct on two boards.
               The only difference between sstriker and sstrikera is the copyright text displayed when
               the region is set to Korea.

    kingdmgp - The kingdmgp and shippumd sets have identical program ROMs but a different graphics ROM
               for the text layer. Setting the region to Japan with the kingdmgp ROM, or to anything other
               than Japan with the shippumd ROM, results in a corrupt title screen and unreadable text.
               In kingdmgp some of the tiles needed for the credits screen in attract mode have been
               stripped out, resulting in boxes where letters should be. It doesn't seem very professional
               but appears to be a genuine release. A lot of boards being sold as 'Kingdom Grand Prix' are
               in fact conversions using Neill Corlett's hack.

    bgaregga - The later versions change the small bullet-shaped enemy bullets to bright yellow balls,
               eliminate the flying metal debris from explosions, and require additional joystick input
               to access the Extended, Harder, Special, and Stage Edit hidden features.
               In addition to these changes, the bgareggat2 set uses only two buttons. Instead of being
               able to change the formation of your options with the third button, each of the selectable
               ships has a different, fixed option formation. However, the third button can still be used
               to select an alternate ship color and to enter the secret character and Stage Edit codes.


 ****************************************************************************
 * Battle Garegga and Armed Police Batrider have secret characters          *
 * and game features.                                                       *
 * Try to input the following commands to use them.                         *
 * ======================================================================== *
 * Battle Garegga                                                           *
 *       The button you use to select your ship not only determines its     *
 *       color, but affects its characteristics.                            *
 *           A: Default characteristics.                                    *
 *           B: Slightly higher speed than A type.                          *
 *           C: Slightly smaller hitbox than A type.                        *
 *       A+B+C: Same speed as B type and same hitbox as C type.             *
 *                                                                          *
 *       After inserting a coin (pushing a credit button), input            *
 *       UP  UP  DOWN  DOWN  LEFT  RIGHT  LEFT  RIGHT  A  B  C  START       *
 *       then you can use Mahou Daisakusen characters.                      *
 *                                                                          *
 * Note: In versions of the game dated Mar 2 1996 or later, you must        *
 *       hold the joystick RIGHT in addition to the specified button(s)     *
 *       when entering any of the following commands. Even if Stage Edit    *
 *       is enabled via dipswitch, you need to hold RIGHT to use it.        *
 *                                                                          *
 * EXTENDED:   After inserting a coin, hold A and press START.              *
 *             You play through all stages twice before the game ends.      *
 * HARDER:     After inserting a coin, hold B and press START.              *
 *             Difficulty is increased.                                     *
 * SPECIAL:    After inserting a coin, hold A and B and press START.        *
 *             Combination of EXTENDED and HARDER modes.                    *
 * STAGE EDIT: After inserting a coin, hold C and press START.              *
 *             You can choose what order to play Stage 2, 3 and 4 in,       *
 *             or even skip them.                                           *
 *                                                                          *
 * EXTENDED, HARDER, and SPECIAL modes each have their own high score list. *
 * ------------------------------------------------------------------------ *
 ****************************************************************************

Game status:

To Do / Unknowns:
    - Find out how exactly how sound CPU communication really works in bgaregga/batrider/bbakraid
        current emulation seems to work (plays all sounds), but there are still some unknown reads/writes
    - Music timing is bit different on bbakraid.
        reference : https://www.youtube.com/watch?v=zjrWs0iHQ5A
    - kingdmgpbl moves around some registers to make up for missing VDP of the original. Implement this.

*****************************************************************************/

void raizing_base_state::reset_audiocpu(int state)
{
	if (m_audiocpu != nullptr)
		m_audiocpu->set_input_line(INPUT_LINE_RESET, state);
}


u32 raizing_base_state::screen_update_base(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_custom_priority_bitmap.fill(0, cliprect);
	m_vdp->render_vdp(bitmap, cliprect);

	return 0;
}

void raizing_base_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		m_vdp->screen_eof();
	}
}

u32 raizing_base_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update_base(screen, bitmap, cliprect);
	m_tx_tilemap->draw_tilemap(screen, bitmap, cliprect);
	return 0;
}



void raizing_base_state::bgaregga_common_video_start()
{
	m_screen->register_screen_bitmap(m_custom_priority_bitmap);
	m_vdp->custom_priority_bitmap = &m_custom_priority_bitmap;
}


void raizing_base_state::raizing_z80_bankswitch_w(u8 data)
{
	m_audiobank->set_entry(data & 0x0f);
}

// bgaregga and batrider don't actually have a NMK112, but rather a GAL
// programmed to bankswitch the sound ROMs in a similar fashion.
// it may not be a coincidence that the composer and sound designer for
// these two games, Manabu "Santaruru" Namiki, came to Raizing from NMK...
void raizing_base_state::raizing_oki_bankswitch_w(offs_t offset, u8 data)
{
	m_raizing_okibank[(offset & 4) >> 2][offset & 3]->set_entry(data & 0xf);
	m_raizing_okibank[(offset & 4) >> 2][4 + (offset & 3)]->set_entry(data & 0xf);
	offset++;
	data >>= 4;
	m_raizing_okibank[(offset & 4) >> 2][offset & 3]->set_entry(data & 0xf);
	m_raizing_okibank[(offset & 4) >> 2][4 + (offset & 3)]->set_entry(data & 0xf);
}

void raizing_base_state::raizing_oki_reset()
{
	for (int chip = 0; chip < 2; chip++)
	{
		for (int i = 0; i < 8; i++)
		{
			if (m_raizing_okibank[chip][i] != nullptr)
				m_raizing_okibank[chip][i]->set_entry(0);
		}
	}
}

void raizing_base_state::common_mem(address_map &map, offs_t rom_limit)
{
	map(0x000000, rom_limit).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x218000, 0x21bfff).rw(FUNC(raizing_base_state::shared_ram_r), FUNC(raizing_base_state::shared_ram_w)).umask16(0x00ff);
	map(0x21c020, 0x21c021).portr("IN1");
	map(0x21c024, 0x21c025).portr("IN2");
	map(0x21c028, 0x21c029).portr("SYS");
	map(0x21c02c, 0x21c02d).portr("DSWA");
	map(0x21c030, 0x21c031).portr("DSWB");
	map(0x21c034, 0x21c035).portr("JMPR");
	map(0x21c03c, 0x21c03d).r(m_vdp, FUNC(gp9001vdp_device::vdpcount_r));
	map(0x300000, 0x30000d).rw(m_vdp, FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x501fff).rw(m_tx_tilemap, FUNC(toaplan_txtilemap_device::videoram_r), FUNC(toaplan_txtilemap_device::videoram_w));
	map(0x502000, 0x502fff).rw(m_tx_tilemap, FUNC(toaplan_txtilemap_device::lineselect_r), FUNC(toaplan_txtilemap_device::lineselect_w));
	map(0x503000, 0x5031ff).rw(m_tx_tilemap, FUNC(toaplan_txtilemap_device::linescroll_r), FUNC(toaplan_txtilemap_device::linescroll_w));
	map(0x503200, 0x503fff).ram();
}

void raizing_base_state::install_raizing_okibank(int chip)
{
	assert(m_oki_rom[chip] && m_raizing_okibank[chip][0]);

	for (int i = 0; i < 4; i++)
	{
		m_raizing_okibank[chip][i]->configure_entries(0, 16, &m_oki_rom[chip][(i * 0x100)], 0x10000);
	}
	m_raizing_okibank[chip][4]->configure_entries(0, 16, &m_oki_rom[chip][0x400], 0x10000);
	for (int i = 5; i < 8; i++)
	{
		m_raizing_okibank[chip][i]->configure_entries(0, 16, &m_oki_rom[chip][0], 0x10000);
	}
}


namespace {

class bgaregga_state : public raizing_base_state
{
public:
	bgaregga_state(const machine_config &mconfig, device_type type, const char *tag)
		: raizing_base_state(mconfig, type, tag)
	{ }

	void bgaregga(machine_config &config) ATTR_COLD;

	void init_bgaregga() ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void bgaregga_68k_mem(address_map &map) ATTR_COLD;
	void bgaregga_sound_z80_mem(address_map &map) ATTR_COLD;

	u8 bgaregga_E01D_r();
};

class bgaregga_bootleg_state : public bgaregga_state
{
public:
	bgaregga_bootleg_state(const machine_config &mconfig, device_type type, const char *tag)
		: bgaregga_state(mconfig, type, tag)
	{ }

	void bgareggabl(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	u32 screen_update_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


class sstriker_state : public raizing_base_state
{
public:
	sstriker_state(const machine_config &mconfig, device_type type, const char *tag)
		: raizing_base_state(mconfig, type, tag)
	{ }

	void mahoudai(machine_config &config) ATTR_COLD;
	void shippumd(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void mahoudai_68k_mem(address_map &map) ATTR_COLD;
	void shippumd_68k_mem(address_map &map) ATTR_COLD;
	void raizing_sound_z80_mem(address_map &map) ATTR_COLD;

	void shippumd_coin_w(u8 data);
};


void bgaregga_state::video_start()
{
	bgaregga_common_video_start();
}

void sstriker_state::video_start()
{
	bgaregga_common_video_start();
}

void bgaregga_bootleg_state::video_start()
{
	m_screen->register_screen_bitmap(m_custom_priority_bitmap);
	m_vdp->custom_priority_bitmap = &m_custom_priority_bitmap;
}




u32 bgaregga_bootleg_state::screen_update_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update_base(screen, bitmap, cliprect);
	m_tx_tilemap->draw_tilemap_bootleg(screen, bitmap, cliprect);
	return 0;
}


/*****************************************************************************
    Input Port definitions
    The following commands are available when the Invulnerability dipswitch
    is set (or, in some games, also when the JAMMA Test switch is pressed):

    P2 start                 : pause
    P1 start                 : resume
    Hold P1 start & P2 start : slow motion

    In bgaregga, batrider and bbakraid, the commands are different:

    Tap P1 start             : pause/resume
    Hold P1 start            : slow motion

    Additional per-game test features are as follows:

    truxton2 - While playing in invulnerable mode, press button 3 to suicide.

    fixeight - While playing in invulnerable mode, press button 3 to suicide
               (player 1 and player 2 only)

    batsugun - While playing in invulnerable mode, press the following buttons
               to stage skip:

               P2 button 3 & P2 button 1 : Skip to end of stage 1
               P2 button 3 & P2 button 2 : Skip to end of stage 2
               P2 button 3               : Skip to end of stage 3

   sstriker - While playing in invulnerable mode as player 2, press
   /kingdmgp  P2 button 3 to skip to the end of the current stage.

   bgaregga - Press and hold P1 button 1, P1 button 2 and P1 button 3 while
              powering on in service mode to enter the special service mode.
              "OPTIONS" and "PLAY DATAS" are added to the menu, and the
              dipswitch display will show the region jumpers (normally hidden).
              Choose "GAME MODE" from the special service mode to enter the
              special game mode. In the special game mode, you can use pause
              and slow motion even when not playing in invulnerable mode.

   batrider - While playing in invulnerable mode, press P1 Start and P2 Start
              to skip directly to the ending scene.

   batrider - Press and hold P1 button 1, P1 button 2 and P1 button 3 while
   /bbakraid  powering on in service mode to enter the special service mode.
              You can change the game's region by pressing left/right.
              Choose "GAME MODE" from the special service mode to enter the
              special game mode. In the special game mode, you can use pause
              and slow motion even when not playing in invulnerable mode.
              While the game is paused in special mode, press button 3 to
              display debugging information.

*****************************************************************************/

static INPUT_PORTS_START( 2b )
	PORT_START("IN1")
	TOAPLAN_JOY_UDLR_2_BUTTONS( 1 )

	PORT_START("IN2")
	TOAPLAN_JOY_UDLR_2_BUTTONS( 2 )

	PORT_START("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_TILT )
	TOAPLAN_TEST_SWITCH( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSWA")
	TOAPLAN_MACHINE_NO_COCKTAIL_LOC(SW1)
	// Coinage on bit mask 0x00f0
	PORT_BIT( 0x00f0, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Modified below

	PORT_START("DSWB")
	TOAPLAN_DIFFICULTY_LOC(SW2)
	// Per-game features on bit mask 0x00fc
	PORT_BIT( 0x00fc, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Modified below
INPUT_PORTS_END


static INPUT_PORTS_START( 3b )
	PORT_INCLUDE( 2b )

	PORT_MODIFY("IN1")
	TOAPLAN_JOY_UDLR_3_BUTTONS( 1 )

	PORT_MODIFY("IN2")
	TOAPLAN_JOY_UDLR_3_BUTTONS( 2 )
INPUT_PORTS_END


static INPUT_PORTS_START( sstriker )
	PORT_INCLUDE( 3b )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,   0x0000, DEF_STR( Free_Play ) )  PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0001, DEF_STR( On ) )
	// Various features on bit mask 0x000e - see above
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0x0e, 0x04, SW1 )

	PORT_MODIFY("DSWB")
	// Difficulty on bit mask 0x0003 - see above
	PORT_DIPNAME( 0x000c,   0x0000, DEF_STR( Bonus_Life ) )     PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(        0x0008, "200k only" )
	PORT_DIPSETTING(        0x0000, "Every 300k" )
	PORT_DIPSETTING(        0x0004, "200k and 500k" )
	PORT_DIPNAME( 0x0030,   0x0000, DEF_STR( Lives ) )          PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0020, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x0010, "5" )
	PORT_DIPNAME( 0x0040,   0x0000, "Invulnerability (Cheat)" ) PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Yes ) )

	PORT_START("JMPR")
	PORT_CONFNAME( 0x0001,  0x0001, "FBI Logo" )        //PORT_CONFLOCATION("JP:!4")
	PORT_CONFSETTING(       0x0001, DEF_STR( Off ) )
	PORT_CONFSETTING(       0x0000, DEF_STR( On ) )
	PORT_CONFNAME( 0x000e,  0x0004, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!3,!2,!1")
	PORT_CONFSETTING(       0x0004, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0002, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0006, DEF_STR( Southeast_Asia ) )
	PORT_CONFSETTING(       0x0008, DEF_STR( China ) )
	PORT_CONFSETTING(       0x000a, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x000c, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x000e, DEF_STR( Taiwan ) )
INPUT_PORTS_END


static INPUT_PORTS_START( sstrikerk ) // Although the region jumper is functional, it's a Korean board / version
	PORT_INCLUDE( sstriker )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x000e,  0x000a, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!3,!2,!1")
	PORT_CONFSETTING(       0x0004, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0002, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0006, DEF_STR( Southeast_Asia ) )
	PORT_CONFSETTING(       0x0008, DEF_STR( China ) )
	PORT_CONFSETTING(       0x000a, "Korea (Unite Trading)" )
	PORT_CONFSETTING(       0x000c, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x000e, DEF_STR( Taiwan ) )
INPUT_PORTS_END


static INPUT_PORTS_START( mahoudai )
	PORT_INCLUDE( sstriker )

	PORT_MODIFY("JMPR")
	// Effectively unused by this set - see notes
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( kingdmgp )
	PORT_INCLUDE( sstriker )

	// The code and lookup tables pertaining to the jumpers are almost identical to sstriker.
	// However, this set apparently lacks (reachable) code to display the FBI logo,
	// even though the logo itself is present in the gfx ROMs.
	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x0001,  0x0000, DEF_STR( Unused ) ) //PORT_CONFLOCATION("JP:!4")
	PORT_CONFSETTING(       0x0000, DEF_STR( Off ) )
	PORT_CONFSETTING(       0x0001, DEF_STR( On ) )
	PORT_CONFNAME( 0x000e,  0x0004, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!3,!2,!1")
	PORT_CONFSETTING(       0x0004, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0002, DEF_STR( USA ) )
//  PORT_CONFSETTING(        0x0000, DEF_STR( Japan ) )  // Corrupt title screen and text - use shippumd
	PORT_CONFSETTING(       0x0006, DEF_STR( Southeast_Asia ) )
	PORT_CONFSETTING(       0x0008, DEF_STR( China ) )
	PORT_CONFSETTING(       0x000a, "Korea (Unite Trading license)" )
	PORT_CONFSETTING(       0x000c, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x000e, DEF_STR( Taiwan ) )
INPUT_PORTS_END


static INPUT_PORTS_START( shippumd )
	PORT_INCLUDE( sstriker )

	PORT_MODIFY("JMPR")
	// Title screen and text are corrupt for anything but Japan
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( bgaregga )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Unknown/Unused

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Unknown/Unused

	PORT_START("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	TOAPLAN_TEST_SWITCH( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Unknown/Unused

	PORT_START("DSWA")
	PORT_SERVICE_DIPLOC(0x0001, IP_ACTIVE_HIGH, "SW1:!1")
	PORT_DIPNAME( 0x0002,   0x0000, "Credits to Start" )    PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(        0x0000, "1" )
	PORT_DIPSETTING(        0x0002, "2" )
	PORT_DIPNAME( 0x001c,   0x0000, DEF_STR( Coin_A ) )     PORT_DIPLOCATION("SW1:!3,!4,!5")
	PORT_DIPSETTING(        0x0018, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(        0x0014, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(        0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(        0x0004, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(        0x0008, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(        0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(        0x001c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x00e0,   0x0000, DEF_STR( Coin_B ) )     PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)   PORT_DIPLOCATION("SW1:!6,!7,!8")
	PORT_DIPSETTING(        0x00c0, DEF_STR( 4C_1C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x00a0, DEF_STR( 3C_1C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0080, DEF_STR( 2C_1C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0000, DEF_STR( 1C_1C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
//  PORT_DIPSETTING(        0x00e0, DEF_STR( 1C_1C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0020, DEF_STR( 1C_2C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0040, DEF_STR( 1C_3C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0060, DEF_STR( 1C_4C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
	// When Coin_A is set to Free_Play, Coin_A becomes Coin_A and Coin_B, and the following dips occur
	PORT_DIPNAME( 0x0020,   0x0000, "Joystick Mode" )       PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)  PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(        0x0000, "90 degrees ACW" )      PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)
	PORT_DIPSETTING(        0x0020, DEF_STR( Normal ) )     PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)
	PORT_DIPNAME( 0x0040,   0x0000, "Effect" )              PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)  PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )        PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )         PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)
	PORT_DIPNAME( 0x0080,   0x0000, "Music" )               PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)  PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )        PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)
	PORT_DIPSETTING(        0x0080, DEF_STR( On ) )         PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)

	PORT_START("DSWB")
	PORT_DIPNAME( 0x0003,   0x0000, DEF_STR( Difficulty ) )     PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(        0x0001, DEF_STR( Easy ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(        0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(        0x0003, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0004,   0x0000, DEF_STR( Flip_Screen ) )    PORT_DIPLOCATION("SW2:!3")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008,   0x0000, DEF_STR( Demo_Sounds ) )    PORT_DIPLOCATION("SW2:!4")
	PORT_DIPSETTING(        0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0070,   0x0000, DEF_STR( Lives ) )          PORT_DIPLOCATION("SW2:!5,!6,!7")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0020, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x0010, "4" )
	PORT_DIPSETTING(        0x0040, "5" )
	PORT_DIPSETTING(        0x0050, "6" )
	PORT_DIPSETTING(        0x0060, DEF_STR( Infinite ) )
	PORT_DIPSETTING(        0x0070, "Invulnerability (Cheat)" )
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0000, DEF_STR( None ) )       PORT_CONDITION("JMPR",0x0003,NOTEQUALS,0x0000)  // Non-Japan
	PORT_DIPSETTING(        0x0080, "Every 2000k" )         PORT_CONDITION("JMPR",0x0003,NOTEQUALS,0x0000)  // Non-Japan
	PORT_DIPSETTING(        0x0080, "1000k and 2000k" )     PORT_CONDITION("JMPR",0x0003,EQUALS,0x0000) // Japan
	PORT_DIPSETTING(        0x0000, "Every 1000k" )         PORT_CONDITION("JMPR",0x0003,EQUALS,0x0000) // Japan

	PORT_START("JMPR") // DSW3 and jumper
	PORT_DIPNAME( 0x0008,  0x0000, "Stage Edit" ) PORT_DIPLOCATION("SW3:!1")
	PORT_DIPSETTING(       0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004,  0x0000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW3:!2")
	PORT_DIPSETTING(       0x0004, DEF_STR( No ) )
	PORT_DIPSETTING(       0x0000, DEF_STR( Yes ) )
	PORT_CONFNAME( 0x0003,  0x0001, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!2,!1")
	PORT_CONFSETTING(       0x0001, "Europe (Tuning)" )
	PORT_CONFSETTING(       0x0002, "USA (Fabtek)" )
	PORT_CONFSETTING(       0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0003, DEF_STR( Asia ) )
INPUT_PORTS_END


static INPUT_PORTS_START( bgareggahk )
	PORT_INCLUDE( bgaregga )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x0003,  0x0003, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!2,!1")
	PORT_CONFSETTING(       0x0001, "Austria (Tuning)" )
	// These two settings end up reporting ROM-0 as BAD
//  PORT_CONFSETTING(        0x0002, "USA (Fabtek)" )
//  PORT_CONFSETTING(        0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0003, "Hong Kong (Metrotainment)" )
INPUT_PORTS_END


static INPUT_PORTS_START( bgareggatw )
	PORT_INCLUDE( bgaregga )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x0003,  0x0003, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!2,!1")
	PORT_CONFSETTING(       0x0001, "Germany (Tuning)" )
	// These two settings end up reporting ROM-0 as BAD
//  PORT_CONFSETTING(        0x0002, "USA (Fabtek)" )
//  PORT_CONFSETTING(        0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0003, "Taiwan (Liang Hwa)" )
INPUT_PORTS_END


static INPUT_PORTS_START( bgareggak )
	PORT_INCLUDE( bgaregga )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x0003,  0x0003, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!2,!1")
	PORT_CONFSETTING(       0x0001, "Greece" )
	PORT_CONFSETTING(       0x0003, "Korea" )
INPUT_PORTS_END


static INPUT_PORTS_START( bgareggacn )
	PORT_INCLUDE( bgaregga )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x0003,  0x0003, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!2,!1")
	PORT_CONFSETTING(       0x0001, "Denmark (Tuning)" )
	// These two settings end up reporting ROM-0 as BAD
//  PORT_CONFSETTING(        0x0002, "USA (Fabtek)" )
//  PORT_CONFSETTING(        0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0003, DEF_STR( China ) )
INPUT_PORTS_END


static INPUT_PORTS_START( bgareggabl )
	PORT_INCLUDE( bgaregga )

	PORT_MODIFY("DSWB") // Always uses bonus life settings from Japan region
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0080, "1000k and 2000k" )
	PORT_DIPSETTING(        0x0000, "Every 1000k" )

	PORT_MODIFY("JMPR") // Region is hard-coded
	PORT_BIT( 0x0003, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END



u8 bgaregga_state::bgaregga_E01D_r()
{
	// the Z80 reads this address during its IRQ routine,
	// and reads the soundlatch only if the lowest bit is clear.
	return m_soundlatch[0]->pending_r() ? 0 : 1;
}

void sstriker_state::shippumd_coin_w(u8 data)
{
	m_coincounter->coin_w(data & ~0x10);
	m_oki[0]->set_rom_bank(BIT(data, 4));
}

void bgaregga_state::machine_reset()
{
	raizing_base_state::machine_reset();

	raizing_oki_reset();
}

void sstriker_state::mahoudai_68k_mem(address_map &map)
{
	common_mem(map, 0x07ffff);

	map(0x21c01d, 0x21c01d).w(m_coincounter, FUNC(toaplan_coincounter_device::coin_w));
	map(0x401000, 0x4017ff).ram();                         // Unused palette RAM
}

void sstriker_state::shippumd_68k_mem(address_map &map)
{
	common_mem(map, 0x0fffff);

//  map(0x21c008, 0x21c009).nopw();                    // ???
	map(0x21c01d, 0x21c01d).w(FUNC(sstriker_state::shippumd_coin_w)); // Coin count/lock + oki bankswitch
	map(0x401000, 0x4017ff).ram();                         // Unused palette RAM
}

void bgaregga_state::bgaregga_68k_mem(address_map &map)
{
	common_mem(map, 0x0fffff);

	map(0x21c01d, 0x21c01d).w(m_coincounter, FUNC(toaplan_coincounter_device::coin_w));
	map(0x600001, 0x600001).w(m_soundlatch[0], FUNC(generic_latch_8_device::write));
}

void sstriker_state::raizing_sound_z80_mem(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xdfff).ram().share(m_shared_ram);
	map(0xe000, 0xe001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xe004, 0xe004).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xe00e, 0xe00e).w(m_coincounter, FUNC(toaplan_coincounter_device::coin_w));
}

void bgaregga_state::bgaregga_sound_z80_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_audiobank);
	map(0xc000, 0xdfff).ram().share(m_shared_ram);
	map(0xe000, 0xe001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xe004, 0xe004).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xe006, 0xe008).w(FUNC(bgaregga_state::raizing_oki_bankswitch_w));
	map(0xe00a, 0xe00a).w(FUNC(bgaregga_state::raizing_z80_bankswitch_w));
	map(0xe00c, 0xe00c).w(m_soundlatch[0], FUNC(generic_latch_8_device::acknowledge_w));
	map(0xe01c, 0xe01c).r(m_soundlatch[0], FUNC(generic_latch_8_device::read));
	map(0xe01d, 0xe01d).r(FUNC(bgaregga_state::bgaregga_E01D_r));
}

static GFXDECODE_START( gfx_textrom )
	GFXDECODE_ENTRY( "text", 0, gfx_8x8x4_packed_msb, 64*16, 64 )
GFXDECODE_END

void sstriker_state::mahoudai(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32_MHz_XTAL/2);   // 16MHz, 32MHz Oscillator
	m_maincpu->set_addrmap(AS_PROGRAM, &sstriker_state::mahoudai_68k_mem);
	m_maincpu->reset_cb().set(FUNC(sstriker_state::reset_audiocpu));

	Z80(config, m_audiocpu, 32_MHz_XTAL/8);     // 4MHz, 32MHz Oscillator
	m_audiocpu->set_addrmap(AS_PROGRAM, &sstriker_state::raizing_sound_z80_mem);

	TOAPLAN_COINCOUNTER(config, m_coincounter, 0);

	config.set_maximum_quantum(attotime::from_hz(600));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(sstriker_state::screen_update));
	m_screen->screen_vblank().set(FUNC(sstriker_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp, 27_MHz_XTAL);
	m_vdp->set_palette(m_palette);
	m_vdp->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	TOAPLAN_TXTILEMAP(config, m_tx_tilemap, 27_MHz_XTAL, m_palette, gfx_textrom);
	m_tx_tilemap->set_offset(0x1d4, 0, 0x16b, 0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.35);

	OKIM6295(config, m_oki[0], 32_MHz_XTAL/32, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void sstriker_state::shippumd(machine_config &config)
{
	mahoudai(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &sstriker_state::shippumd_68k_mem);
}

void bgaregga_state::bgaregga(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32_MHz_XTAL/2);   // 16MHz, 32MHz Oscillator
	m_maincpu->set_addrmap(AS_PROGRAM, &bgaregga_state::bgaregga_68k_mem);
	m_maincpu->reset_cb().set(FUNC(bgaregga_state::reset_audiocpu));

	Z80(config, m_audiocpu, 32_MHz_XTAL/8);     // 4MHz, 32MHz Oscillator
	m_audiocpu->set_addrmap(AS_PROGRAM, &bgaregga_state::bgaregga_sound_z80_mem);

	TOAPLAN_COINCOUNTER(config, m_coincounter, 0);

	config.set_maximum_quantum(attotime::from_hz(6000));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(bgaregga_state::screen_update));
	m_screen->screen_vblank().set(FUNC(bgaregga_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp, 27_MHz_XTAL);
	m_vdp->set_palette(m_palette);
	m_vdp->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	TOAPLAN_TXTILEMAP(config, m_tx_tilemap, 27_MHz_XTAL, m_palette, gfx_textrom);
	m_tx_tilemap->set_offset(0x1d4, 0, 0x16b, 0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch[0]);
	m_soundlatch[0]->data_pending_callback().set_inputline(m_audiocpu, 0);
	m_soundlatch[0]->set_separate_acknowledge(true);

	YM2151(config, "ymsnd", 32_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.3);

	OKIM6295(config, m_oki[0], 32_MHz_XTAL/16, okim6295_device::PIN7_HIGH);
	m_oki[0]->set_addrmap(0, &bgaregga_state::raizing_oki<0>);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.6);
}


void bgaregga_bootleg_state::bgareggabl(machine_config &config)
{
	bgaregga(config);

	m_screen->set_screen_update(FUNC(bgaregga_bootleg_state::screen_update_bootleg));
	m_tx_tilemap->set_offset(4, 0, 4, 0);
}


void bgaregga_state::init_bgaregga()
{
	u8 *Z80 = memregion("audiocpu")->base();

	m_audiobank->configure_entries(0, 8, Z80, 0x4000); // Test mode only, Mirror of First 128KB Areas?
	m_audiobank->configure_entries(8, 8, Z80, 0x4000);
	install_raizing_okibank(0);
}



/* -------------------------- Raizing games ------------------------- */


/*
For the two sets of Sorcer Striker (World) the only differences
are 2 bytes plus a corrected checksum for each set:

File Offset     sstriker   sstrikera
  0x160            17         0B   <-- Rom checksum value
  0x161            79         6D   <-- Rom checksum value

  0x92C            18         0C   <-- Index of copyright strings to display for Korea
  0x92D            18         0C   <-- Index of copyright strings to display for Korea

0C points to the strings "LICENSED TO UNITE TRADING" / "FOR KOREA".
18 points to a pair of empty strings.

Printed labels for the eproms look like:

RA-MA-01
   01
RAIZING

Both English and Japanese sets use the same labels and numbers for the roms
even if the roms contain different code / data.
*/

ROM_START( sstriker )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "ra-ma_01_01.u65", 0x000000, 0x080000, CRC(708fd51d) SHA1(167186d4cf13af37ec0fa6a59c738c54dbbf3c7c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "ra-ma-01_02.u66", 0x00000, 0x10000, CRC(eabfa46d) SHA1(402c99ebf88f9025f74f0a28ced22b7882a65eb3) )

	ROM_REGION( 0x200000, "gp9001", 0 )
	ROM_LOAD( "ra-ma01-rom2.u2",  0x000000, 0x100000, CRC(54e2bd95) SHA1(341359dd46152615675bb90e8a184216c8feebff) )
	ROM_LOAD( "ra-ma01-rom3.u1",  0x100000, 0x100000, CRC(21cd378f) SHA1(e1695bccec949d18b1c03e9c42dca384554b0d7c) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "ra-ma-01_05.u81",  0x000000, 0x008000, CRC(88b58841) SHA1(1d16b538c11a291bd1f46a510bfbd6259b45a0b5) )

	ROM_REGION( 0x40000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "ra-ma01-rom1.u57", 0x00000, 0x40000, CRC(6edb2ab8) SHA1(e3032e8eda2686f30df4b7a088c5a4d4d45782ed) )
ROM_END


ROM_START( sstrikerk )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "ra-ma-01_01.u65", 0x000000, 0x080000, CRC(92259f84) SHA1(127e62e407d95efd360bfe2cac9577f326abf6ef) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "ra-ma-01_02.u66", 0x00000, 0x10000, CRC(eabfa46d) SHA1(402c99ebf88f9025f74f0a28ced22b7882a65eb3) )

	ROM_REGION( 0x200000, "gp9001", 0 )
	ROM_LOAD( "ra-ma01-rom2.u2",  0x000000, 0x100000, CRC(54e2bd95) SHA1(341359dd46152615675bb90e8a184216c8feebff) )
	ROM_LOAD( "ra-ma01-rom3.u1",  0x100000, 0x100000, CRC(21cd378f) SHA1(e1695bccec949d18b1c03e9c42dca384554b0d7c) )
	// also seen with 4 smaller ROMs instead of 2
	// 01.bin                  ra-ma01-rom2.u2 [even]     IDENTICAL
	// 02.bin                  ra-ma01-rom2.u2 [odd]      IDENTICAL
	// 03.bin                  ra-ma01-rom3.u1 [even]     IDENTICAL
	// 04.bin                  ra-ma01-rom3.u1 [odd]      IDENTICAL

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "ra-ma-01_05.u81",  0x000000, 0x008000, CRC(88b58841) SHA1(1d16b538c11a291bd1f46a510bfbd6259b45a0b5) )

	ROM_REGION( 0x40000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "ra-ma01-rom1.u57", 0x00000, 0x40000, CRC(6edb2ab8) SHA1(e3032e8eda2686f30df4b7a088c5a4d4d45782ed) )
ROM_END


ROM_START( mahoudai )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "ra_ma_01_01.u65", 0x000000, 0x080000, CRC(970ccc5c) SHA1(c87cab83bde0284e631f02e50068407fee81d941) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "ra-ma-01_02.u66", 0x00000, 0x10000, CRC(eabfa46d) SHA1(402c99ebf88f9025f74f0a28ced22b7882a65eb3) )

	ROM_REGION( 0x200000, "gp9001", 0 )
	ROM_LOAD( "ra-ma01-rom2.u2",  0x000000, 0x100000, CRC(54e2bd95) SHA1(341359dd46152615675bb90e8a184216c8feebff) )
	ROM_LOAD( "ra-ma01-rom3.u1",  0x100000, 0x100000, CRC(21cd378f) SHA1(e1695bccec949d18b1c03e9c42dca384554b0d7c) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "ra_ma_01_05.u81",  0x000000, 0x008000, CRC(c00d1e80) SHA1(53e64c4c0c6309130b37597d13b44a9e95b717d8) )

	ROM_REGION( 0x40000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "ra-ma01-rom1.u57", 0x00000, 0x40000, CRC(6edb2ab8) SHA1(e3032e8eda2686f30df4b7a088c5a4d4d45782ed) )
ROM_END


ROM_START( kingdmgp )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "ma02rom1.bin", 0x000000, 0x080000, CRC(a678b149) SHA1(8c1a631e023dbba0a3fa6cd1b7d10dec1663213a) )
	ROM_LOAD16_BYTE( "ma02rom0.bin", 0x000001, 0x080000, CRC(f226a212) SHA1(526acf3d05fdc88054a772fbea3de2af532bf3d2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "ma02rom2.bin", 0x00000, 0x10000, CRC(dde8a57e) SHA1(f522a3f17e229c71512464349760a9e27778bf6a) )

	ROM_REGION( 0x400000, "gp9001", 0 )
	ROM_LOAD( "ma02rom3.bin",  0x000000, 0x200000, CRC(0e797142) SHA1(a480ccd151e49b886d3175a6deff56e1f2c26c3e) )
	ROM_LOAD( "ma02rom4.bin",  0x200000, 0x200000, CRC(72a6fa53) SHA1(ce92e65205b84361cfb90305a61e9541b5c4dc2f) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "ma02rom5.eng",  0x000000, 0x008000, CRC(8c28460b) SHA1(0aed170762f6044896a7e608df60bbd37c583a71) )

	ROM_REGION( 0x80000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "ma02rom6.bin", 0x00000, 0x80000, CRC(199e7cae) SHA1(0f5e13cc8ec42c80bb4bbff90aba29cdb15213d4) )
ROM_END


ROM_START( shippumd )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "ma02rom1.bin", 0x000000, 0x080000, CRC(a678b149) SHA1(8c1a631e023dbba0a3fa6cd1b7d10dec1663213a) )
	ROM_LOAD16_BYTE( "ma02rom0.bin", 0x000001, 0x080000, CRC(f226a212) SHA1(526acf3d05fdc88054a772fbea3de2af532bf3d2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "ma02rom2.bin", 0x00000, 0x10000, CRC(dde8a57e) SHA1(f522a3f17e229c71512464349760a9e27778bf6a) )

	ROM_REGION( 0x400000, "gp9001", 0 )
	ROM_LOAD( "ma02rom3.bin",  0x000000, 0x200000, CRC(0e797142) SHA1(a480ccd151e49b886d3175a6deff56e1f2c26c3e) )
	ROM_LOAD( "ma02rom4.bin",  0x200000, 0x200000, CRC(72a6fa53) SHA1(ce92e65205b84361cfb90305a61e9541b5c4dc2f) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "ma02rom5.bin",  0x000000, 0x008000, CRC(116ae559) SHA1(4cc2d2a23cc0aefd457111b7990e47184e79204c) )

	ROM_REGION( 0x80000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "ma02rom6.bin", 0x00000, 0x80000, CRC(199e7cae) SHA1(0f5e13cc8ec42c80bb4bbff90aba29cdb15213d4) )
ROM_END


ROM_START( kingdmgpbl )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68K code, very minor differences from the original
	ROM_LOAD16_BYTE( "d-26.bin", 0x00000, 0x80000, CRC(5838f306) SHA1(3773974aaba45a18ae34df0056275a358cf3e2f6) )
	ROM_LOAD16_BYTE( "d-25.bin", 0x00001, 0x80000, CRC(fb1c415c) SHA1(5872c466843c2cccad03935e90df04b107a9d454) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80 code, same as the original
	ROM_LOAD( "d-23.bin", 0x00000, 0x10000, CRC(dde8a57e) SHA1(f522a3f17e229c71512464349760a9e27778bf6a) )

	ROM_REGION( 0x400000, "gp9001", 0 ) // same as the original, just arranged differently
	ROM_LOAD16_BYTE( "d-27.bin", 0x000000, 0x80000, CRC(a5afe775) SHA1(c3d4dec2b24d003ee11c4c03bcc8305ac708fd27) )
	ROM_LOAD16_BYTE( "d-29.bin", 0x000001, 0x80000, CRC(cf2331fa) SHA1(cc7fb817c1cf425d7d3b04ca562b234a30a2dec7) )
	ROM_LOAD16_BYTE( "d-28.bin", 0x100000, 0x80000, CRC(a79e9c3e) SHA1(6c49ccc77ef220ef0b339654712e505c307def5e) )
	ROM_LOAD16_BYTE( "d-30.bin", 0x100001, 0x80000, CRC(0f917278) SHA1(f1ba8ddd19ca3c6a364ebd1652b5195d6426e3e2) )
	ROM_LOAD16_BYTE( "d-31.bin", 0x200000, 0x80000, CRC(da6099d8) SHA1(76ad53eec73a65ed17d4db0cd4417acdc97555d4) ) // sticker was actually d-21, but probably due to availability
	ROM_LOAD16_BYTE( "d-33.bin", 0x200001, 0x80000, CRC(f9127208) SHA1(61fc9b68082c5d8c8982687f454ed6744431f382) ) // sticker was actually d-23, but probably due to availability
	ROM_LOAD16_BYTE( "d-32.bin", 0x300000, 0x80000, CRC(82d54b26) SHA1(1f7517182b336bd4d635dcbf39b7c0eba40dd18b) ) // sticker was actually d-22, but probably due to availability
	ROM_LOAD16_BYTE( "d-34.bin", 0x300001, 0x80000, CRC(8bb802d6) SHA1(786220b14e4b20ac5d482ef4363df97ac1cb5f17) ) // sticker was actually d-24, but probably due to availability

	ROM_REGION( 0x8000, "text", 0 ) // same as the original
	ROM_LOAD( "d-21.bin", 0x0000, 0x8000, CRC(8c28460b) SHA1(0aed170762f6044896a7e608df60bbd37c583a71) )

	ROM_REGION( 0x80000, "oki1", 0 ) // same as the original
	ROM_LOAD( "d-22.bin", 0x00000, 0x80000, CRC(199e7cae) SHA1(0f5e13cc8ec42c80bb4bbff90aba29cdb15213d4) )

	ROM_REGION( 0x8000, "unknown", 0 )
	ROM_LOAD( "d-24.bin", 0x0000, 0x8000, CRC(456dd16e) SHA1(84779ee64d3ea33ba1ba4dee39b504a81c6811a1) ) // BADADDR         ---xxxxxxxxxxxx

	ROM_REGION( 0x1000, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8.1",      0x000, 0x117, CRC(c1d254eb) SHA1(65ebc26f70db1bb14d1bf6a1563073d2981e5c4b) )
	ROM_LOAD( "gal16v8.6",      0x200, 0x117, CRC(359337d4) SHA1(f3f950ce6eae36126a719bcb6f837659b85bc36a) )
	ROM_LOAD( "palce20v10h.8",  0x400, 0x2dd, CRC(2f5b291f) SHA1(684f5ace6d43806a890e11a1946b36e0d6a3f4f1) )
	ROM_LOAD( "palce20v10h.10", 0x700, 0x2dd, CRC(6e2a2bb3) SHA1(a716a21f18bb2344e967dd612244da609dfc67f4) )
	ROM_LOAD( "gal16v8.13",     0xa00, 0x117, CRC(3f4a2f5e) SHA1(2fc4f7d6443f5f86366deda3524daa9ce015c0ce) )
	ROM_LOAD( "gal16v8.14",     0xc00, 0x117, CRC(c3bbc41b) SHA1(19372e19af15c8729a7872e3eda4b7ebcc5a2b96) )
	ROM_LOAD( "gal16v8.nn",     0xe00, 0x117, CRC(2d9efaeb) SHA1(37cb914ffda7613976c44ecfcd6d49e79feb2e9c) )
ROM_END


ROM_START( bgareggat )
	/* Dumped from a location test board, with some minor changes compared to the final.
	* All ROMs are socketed
	* All PAL/GALs are socketed
	* PLDs at U33, U125 are PALCE16V8H/4 instead of GAL16V8B as usual (no functional impact)
	* JP4 features four DIP switches, instead of two DIPs + two jumpers as in the final.

	The date codes are written referencing Heisei year 8 (1996).

	The program ROMs feature hand-written labels formatted like this:
	BATTLE GAREGGA
	     PRG 0
	    8.1.17.
	*/
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "battlegaregga-prg0-8-1-17.bin", 0x000000, 0x080000, CRC(c032176f) SHA1(799ba0424489361dd2f814afaf841326bc23300c) )
	ROM_LOAD16_BYTE( "battlegaregga-prg1-8-1-17.bin", 0x000001, 0x080000, CRC(3822f375) SHA1(a5a84cf48c86d8ac97f401280667658d7f451896) )

	/* Hand-written label that reads
	BATTLE GAREGGA
	      SND
	8.1.18. ロケVer.
	*/
	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "battlegaregga-snd-8-1-18-loke-ver.bin", 0x00000, 0x20000, CRC(f5ea56f7) SHA1(9db04069b378dbad6626fd29d3762e3361b9aa0d) )

	/* Stored on NEC ES23C16000W Mask ROMs with no Raizing/8ing custom markings.*/
	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	/* Hand-written label that reads
	BATTLE GAREGGA
	     TEXT
	8.1.17. 8AA6

	8AA6 is the checksum of the text ROM, which matches release.
	*/
	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	/* Stored on an NEC ES23C8001EJ Mask ROM with no Raizing/8ing custom markings.*/
	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END

ROM_START( bgaregga )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "prg0.bin", 0x000000, 0x080000, CRC(f80c2fc2) SHA1(a9aac5c7f5439b6fe8d1b3db1fb02a27cc28fdf6) )
	ROM_LOAD16_BYTE( "prg1.bin", 0x000001, 0x080000, CRC(2ccfdd1e) SHA1(7a9f11f851854f3f8389b9c3c0906ebb8dc28712) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


ROM_START( bgareggahk )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "prg_0.rom", 0x000000, 0x080000, CRC(26e0019e) SHA1(5197001f5d59246b137e19ed1952a8207b25d4c0) )
	ROM_LOAD16_BYTE( "prg_1.rom", 0x000001, 0x080000, CRC(2ccfdd1e) SHA1(7a9f11f851854f3f8389b9c3c0906ebb8dc28712) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


ROM_START( bgareggatw )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "garegga_prg0.u123", 0x000000, 0x080000, CRC(235b7405) SHA1(a2434801df4231a6b48f6c63f47c202d25a89e79) )
	ROM_LOAD16_BYTE( "garegga_prg1.u65",  0x000001, 0x080000, CRC(c29ccf6a) SHA1(38806e0b4ff852f4bfefd80c56ca23f71623e275) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


// only the program ROMs' dumps were provided for this set.
// According to the dumper: 'In the Korea Region Setting, DIP SWITCH's 'STAGE EDIT' does not work and
// the C button (formation change) function in the in-game is also deleted.'
ROM_START( bgareggak )
	ROM_REGION( 0x100000, "maincpu", 0 )            // Main 68K code
	ROM_LOAD16_BYTE( "prg0.bin", 0x000000, 0x080000, CRC(40a108a7) SHA1(cc3227dc87ffefb961dbcdff146e787dbfbdfc2c) )
	ROM_LOAD16_BYTE( "prg1.bin", 0x000001, 0x080000, CRC(45a6e48a) SHA1(f4d4158b8556b4261291ba9905b9731623b47e54) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            // Sound Z80 code + bank
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, BAD_DUMP CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, BAD_DUMP CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, BAD_DUMP CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, BAD_DUMP CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, BAD_DUMP CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, BAD_DUMP CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x100000, "oki1", 0 )        // ADPCM Samples
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, BAD_DUMP CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


ROM_START( bgaregganv )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "prg_0.bin", 0x000000, 0x080000, CRC(951ecc07) SHA1(a82e4b59e4a974566e59f3ab2fbae1aec7d88a2b) )
	ROM_LOAD16_BYTE( "prg_1.bin", 0x000001, 0x080000, CRC(729a60c6) SHA1(cb6f5d138bb82c32910f42d8ee16fa573a23cef3) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


ROM_START( bgareggat2 )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "prg0", 0x000000, 0x080000, CRC(84094099) SHA1(49fc68a8bcdae4477e20eade9dd569de88b0b798) )
	ROM_LOAD16_BYTE( "prg1", 0x000001, 0x080000, CRC(46f92fe4) SHA1(62a02cc1dbdc3ac362339aebb62368eb89b06bad) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


ROM_START( bgareggacn )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "u123", 0x000000, 0x080000, CRC(88a4e66a) SHA1(ca97e564eed0c5e028b937312e55da56400d5c8c) )
	ROM_LOAD16_BYTE( "u65",  0x000001, 0x080000, CRC(5dea32a3) SHA1(59df6689e3eb5ea9e49a758604d21a64c65ca14d) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END

// 1945二代 (1945 Èr Dài)
ROM_START( bgareggabl )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "xt-8m.bin", 0x000000, 0x100000, CRC(4a6657cb) SHA1(1838956e7597eaa78ea5ee58d0ccc79cbbd7ded5) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "6@-322",  0x000000, 0x400000, CRC(37fe48ed) SHA1(ded5d13c33b4582310cdfb3dd52c052f741c00c5) ) /* == rom4.bin+rom3.bin */
	ROM_LOAD( "5@-322",  0x400000, 0x400000, CRC(5a06c031) SHA1(ee241ff90117cec1f33ab163601a9d5c75609739) ) /* == rom2.bin+rom1.bin */

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "1@-256", 0x00000, 0x08000, CRC(760dcd14) SHA1(e151e5d7ca5557277f306b9484ec021f4edf1e07) )

	ROM_REGION( 0x010000, "user1", 0 ) // not graphics
	ROM_LOAD( "2@-256", 0x00000, 0x08000, CRC(456dd16e) SHA1(84779ee64d3ea33ba1ba4dee39b504a81c6811a1) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END

// 雷神传 (Léishén Chuán)
ROM_START( bgareggabla )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "27c8100.mon-sys", 0x000000, 0x100000, CRC(d334e5aa) SHA1(41607b5630d7b92a96607ea95c5b55ad43745857) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.bin", 0x00000, 0x08000, CRC(00d100bd) SHA1(fb6028e3519d6588a966d1b16d47453db2e51fd7))

	ROM_REGION( 0x010000, "user1", 0 ) // not graphics
	ROM_LOAD( "base.bin", 0x00000, 0x08000, CRC(456dd16e) SHA1(84779ee64d3ea33ba1ba4dee39b504a81c6811a1) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END

ROM_START( bgareggablj ) // fixed on Japanese region
	ROM_REGION( 0x100000, "maincpu", 0 )            // Main 68K code
	ROM_LOAD16_WORD_SWAP( "sys.bin", 0x000000, 0x100000, CRC(b2a1225f) SHA1(dda00aa5e7ce3f39bb0eebbcd9500ef22a5d74d3) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_IGNORE(                                0x100000 )

	ROM_REGION( 0x20000, "audiocpu", 0 )            // Sound Z80 code + bank
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "322_2.bin",  0x000000, 0x400000, CRC(37fe48ed) SHA1(ded5d13c33b4582310cdfb3dd52c052f741c00c5) ) // == rom4.bin+rom3.bin
	ROM_LOAD( "322_1.bin",  0x400000, 0x400000, CRC(5a06c031) SHA1(ee241ff90117cec1f33ab163601a9d5c75609739) ) // == rom2.bin+rom1.bin

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.bin", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x010000, "user1", 0 ) // not graphics
	ROM_LOAD( "base.bin", 0x00000, 0x08000, CRC(456dd16e) SHA1(84779ee64d3ea33ba1ba4dee39b504a81c6811a1) )

	ROM_REGION( 0x100000, "oki1", 0 )        // ADPCM Samples
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END

} // anonymous namespace


GAME( 1993, sstriker,    0,        mahoudai,   sstriker,   sstriker_state, empty_init,      ROT270, "Raizing",                         "Sorcer Striker",           MACHINE_SUPPORTS_SAVE ) // verified on two different PCBs
GAME( 1993, sstrikerk,   sstriker, mahoudai,   sstrikerk,  sstriker_state, empty_init,      ROT270, "Raizing (Unite Trading license)", "Sorcer Striker (Korea)",   MACHINE_SUPPORTS_SAVE ) // Although the region jumper is functional, it's a Korean board / version
GAME( 1993, mahoudai,    sstriker, mahoudai,   mahoudai,   sstriker_state, empty_init,      ROT270, "Raizing (Able license)",          "Mahou Daisakusen (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1994, kingdmgp,    0,        shippumd,   kingdmgp,   sstriker_state, empty_init,      ROT270, "Raizing / Eighting", "Kingdom Grandprix",               MACHINE_SUPPORTS_SAVE ) // from Korean board, missing letters on credits screen but this is correct
GAME( 1994, shippumd,    kingdmgp, shippumd,   shippumd,   sstriker_state, empty_init,      ROT270, "Raizing / Eighting", "Shippu Mahou Daisakusen (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, kingdmgpbl,  kingdmgp, shippumd,   kingdmgp,   sstriker_state, empty_init,      ROT270, "bootleg", "Kingdom Grandprix (bootleg)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

GAME( 1996, bgaregga,    0,        bgaregga,   bgaregga,   bgaregga_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga (Europe / USA / Japan / Asia) (Sat Feb 3 1996)",            MACHINE_SUPPORTS_SAVE )
GAME( 1996, bgareggat,   bgaregga, bgaregga,   bgaregga,   bgaregga_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga (location test) (Wed Jan 17 1996)",                         MACHINE_SUPPORTS_SAVE )
GAME( 1996, bgareggahk,  bgaregga, bgaregga,   bgareggahk, bgaregga_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga (Austria / Hong Kong) (Sat Feb 3 1996)",                    MACHINE_SUPPORTS_SAVE )
GAME( 1996, bgareggatw,  bgaregga, bgaregga,   bgareggatw, bgaregga_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga (Taiwan / Germany) (Thu Feb 1 1996)",                       MACHINE_SUPPORTS_SAVE )
GAME( 1996, bgareggak,   bgaregga, bgaregga,   bgareggak,  bgaregga_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga (Korea / Greece) (Wed Feb 7 1996)",                         MACHINE_SUPPORTS_SAVE )
GAME( 1996, bgaregganv,  bgaregga, bgaregga,   bgareggahk, bgaregga_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga - New Version (Austria / Hong Kong) (Sat Mar 2 1996)",      MACHINE_SUPPORTS_SAVE ) // displays New Version only when set to HK
GAME( 1996, bgareggat2,  bgaregga, bgaregga,   bgaregga,   bgaregga_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga - Type 2 (Europe / USA / Japan / Asia) (Sat Mar 2 1996)",   MACHINE_SUPPORTS_SAVE ) // displays Type 2 only when set to Europe
GAME( 1996, bgareggacn,  bgaregga, bgaregga,   bgareggacn, bgaregga_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga - Type 2 (Denmark / China) (Tue Apr 2 1996)",               MACHINE_SUPPORTS_SAVE ) // displays Type 2 only when set to Denmark
GAME( 1998, bgareggabl,  bgaregga, bgareggabl, bgareggabl, bgaregga_bootleg_state, init_bgaregga,   ROT270, "bootleg (Melody)",   "1945 Er Dai / 1945 Part-2 (Chinese hack of Battle Garegga)",               MACHINE_SUPPORTS_SAVE ) // based on Thu Feb 1 1996 set, Region hardcoded to China
GAME( 1997, bgareggabla, bgaregga, bgareggabl, bgareggabl, bgaregga_bootleg_state, init_bgaregga,   ROT270, "bootleg (Melody)",   "Leishen Chuan / Thunder Deity Biography (Chinese hack of Battle Garegga)", MACHINE_SUPPORTS_SAVE ) // based on Thu Feb 1 1996 set, Region hardcoded to Asia
GAME( 1996, bgareggablj, bgaregga, bgareggabl, bgareggabl, bgaregga_bootleg_state, init_bgaregga,   ROT270, "bootleg",            "Battle Garegga (Japan, bootleg) (Sat Feb 3 1996)",                         MACHINE_SUPPORTS_SAVE )
