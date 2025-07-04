// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                      -= Billiard Academy Real Break =-

                    driver by   Luca Elia (l.elia@tin.it)


CPU    :    TMP68301

Sound  :    YMZ280b + YM2413

Chips  :    4L10F2467 (QFP160)
            4L10F2468 (QFP160)
            ACTEL A1010A
            PST532 (system reset and battery backup switching)


---------------------------------------------------------------------------
Year + Game                                 Board
---------------------------------------------------------------------------
98  Pachinko Gindama Shoubu (Japan)         NM5050905-1
98  Pachinko Gindama Shoubu DX (Japan)      NM522-1-9803
98  Billiard Academy Real Break (Japan)     NM523-1-9805
98  Billiard Academy Real Break (Europe)    MM600-1-9807
98  Billiard Academy Real Break (Korea)     MM600-1-9807
04  Dai-Dai-Kakumei (Japan)                 NM522-1-9803
---------------------------------------------------------------------------

Notes:

- To reach the (hidden?) test modes put a breakpoint at 9f80, go in service
  mode and select "results" from the menu - this hits the breakpoint - then
  set ($3,a2) to non zero.

To Do:

- Priorities (e.g during the intro, there are two black bands in the background
  that should obscure sprites).
- Sometimes sprites are shrunk to end up overlapping the background image
  in the tilemaps, but they are a few pixels off

***************************************************************************/

#include "emu.h"
#include "realbrk.h"

#include "sound/ymopl.h"
#include "sound/ymz280b.h"
#include "speaker.h"


/* Read 4 ten bit dip switches */
u16 realbrk_state::realbrk_dsw_r()
{
	const u16 sel = ~m_dsw_select[0];
	if (sel & 0x01) return  (m_dsw_io[0]->read() & 0x00ff) << 8;      // DSW1 low bits
	if (sel & 0x02) return  (m_dsw_io[1]->read() & 0x00ff) << 8;      // DSW2 low bits
	if (sel & 0x04) return  (m_dsw_io[2]->read() & 0x00ff) << 8;      // DSW3 low bits
	if (sel & 0x08) return  (m_dsw_io[3]->read() & 0x00ff) << 8;      // DSW4 low bits

	if (sel & 0x10) return  ((m_dsw_io[0]->read() & 0x0300) << 0) |   // DSWs high 2 bits
							((m_dsw_io[1]->read() & 0x0300) << 2) |
							((m_dsw_io[2]->read() & 0x0300) << 4) |
							((m_dsw_io[3]->read() & 0x0300) << 6) ;

	logerror("CPU #0 PC %06X: read with unknown dsw_select = %02x\n",m_maincpu->pc(),m_dsw_select[0]);
	return 0xffff;
}

u16 realbrk_state::pkgnsh_input_r(offs_t offset)
{
	switch(offset)
	{
		case 0x00/2: return 0xffff;
		case 0x02/2: return 0xffff;
		case 0x04/2: return m_in_io[0]->read();      /*Service buttons*/
		case 0x06/2: return m_dsw_io[0]->read();      /*DIP 2*/
		case 0x08/2: return m_dsw_io[1]->read();      /*DIP 1*/
		case 0x0a/2: return m_dsw_io[2]->read();      /*DIP 1+2 Hi-Bits*/
		case 0x0c/2: return m_paddle_io[0]->read();  /*Handle 1p*/
		case 0x0e/2: return m_player_io[0]->read();           /*Buttons 1p*/
		case 0x10/2: return m_paddle_io[1]->read();  /*Handle 2p*/
		case 0x12/2: return m_player_io[1]->read();           /*Buttons 2p*/
	}
	return 0xffff;
}

u16 realbrk_state::pkgnshdx_input_r(offs_t offset)
{
	const u16 sel = ~m_dsw_select[0];

	switch(offset)
	{
		case 0x00/2: return 0xffff;
		case 0x02/2: return m_in_io[0]->read();  /*Service buttons*/
		/*DSW,same handling as realbrk*/
		case 0x04/2:
			if (sel & 0x01) return  (m_dsw_io[0]->read() & 0x00ff) << 8;      // DSW1 low bits
			if (sel & 0x02) return  (m_dsw_io[1]->read() & 0x00ff) << 8;      // DSW2 low bits
			if (sel & 0x04) return  (m_dsw_io[2]->read() & 0x00ff) << 8;      // DSW3 low bits
			if (sel & 0x08) return  (m_dsw_io[3]->read() & 0x00ff) << 8;      // DSW4 low bits

			if (sel & 0x10) return  ((m_dsw_io[0]->read() & 0x0300) << 0) |   // DSWs high 2 bits
									((m_dsw_io[1]->read() & 0x0300) << 2) |
									((m_dsw_io[2]->read() & 0x0300) << 4) |
									((m_dsw_io[3]->read() & 0x0300) << 6) ;

			return 0xffff;
		case 0x06/2: return m_player_io[1]->read();/*Buttons+Handle 2p*/
		case 0x08/2: return m_player_io[0]->read();/*Buttons+Handle 1p*/
		case 0x0a/2: return 0xffff;
		case 0x0c/2: return 0xffff;
		case 0x0e/2: return 0xffff;
		case 0x10/2: return 0xffff;
		case 0x12/2: return 0xffff;
	}

	return 0xffff;
}


u16 realbrk_state::backup_ram_r(offs_t offset)
{
	/*TODO: understand the format & cmds of the backup-ram,maybe it's an
	        unemulated tmp68301 feature?*/
	if(m_maincpu->pcbase() == 0x02c08e)
		return 0xffff;
	else
		return m_backup_ram[offset];
}


u16 realbrk_state::backup_ram_dx_r(offs_t offset)
{
	/*TODO: understand the format & cmds of the backup-ram,maybe it's an
	        unemulated tmp68301 feature?*/
	if(m_maincpu->pcbase() == 0x02f046)
		return 0xffff;
	else
		return m_backup_ram[offset];
}

void realbrk_state::backup_ram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_backup_ram[offset]);
}

template<int Layer>
void realbrk_state::vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vram[Layer][offset]);
	m_tilemap[Layer]->mark_tile_dirty(offset/2);
}

/***************************************************************************

                                Memory Maps

***************************************************************************/

/*Basic memory map for this HW*/
void realbrk_state::base_mem(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                         // ROM
	map(0x200000, 0x203fff).ram().share("spriteram"); // Sprites
	map(0x400000, 0x40ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");   // Palette
	map(0x600000, 0x601fff).ram().w(FUNC(realbrk_state::vram_w<0>)).share("vram_0");  // Background   (0)
	map(0x602000, 0x603fff).ram().w(FUNC(realbrk_state::vram_w<1>)).share("vram_1");  // Background   (1)
	map(0x604000, 0x604fff).ram().w(FUNC(realbrk_state::vram_2_w)).share("vram_2");  // Text         (2)
	map(0x605000, 0x605fff).ram();                                         //
	map(0x606000, 0x60600f).ram().w(FUNC(realbrk_state::vregs_w)).share("vregs");    // Scroll + Video Regs
	map(0x606010, 0x61ffff).ram();                                         //
	map(0x800000, 0x800003).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask16(0xff00);   // YMZ280
	map(0xfe0000, 0xfeffff).ram();                                         // RAM
}

/*realbrk specific memory map*/
void realbrk_state::realbrk_mem(address_map &map)
{
	base_mem(map);
	map(0x800008, 0x80000b).w("ymsnd", FUNC(ym2413_device::write)).umask16(0x00ff); //
	map(0xc00000, 0xc00001).portr("IN0");                            // P1 & P2 (Inputs)
	map(0xc00002, 0xc00003).portr("IN1");                            // Coins
	map(0xc00004, 0xc00005).ram().r(FUNC(realbrk_state::realbrk_dsw_r)).share("dsw_select");  // DSW select
	map(0xff0000, 0xfffbff).ram();                                         // RAM
}

/*pkgnsh specific memory map*/
void realbrk_state::pkgnsh_mem(address_map &map)
{
	base_mem(map);
	map(0x800008, 0x80000b).w("ymsnd", FUNC(ym2413_device::write)).umask16(0xff00);   // YM2413
	map(0xc00000, 0xc00013).r(FUNC(realbrk_state::pkgnsh_input_r));   // P1 & P2 (Inputs)
	map(0xff0000, 0xfffbff).rw(FUNC(realbrk_state::backup_ram_r), FUNC(realbrk_state::backup_ram_w)).share("backup_ram"); // RAM
}

/*pkgnshdx specific memory map*/
void realbrk_state::pkgnshdx_mem(address_map &map)
{
	base_mem(map);
	map(0x800008, 0x80000b).w("ymsnd", FUNC(ym2413_device::write)).umask16(0x00ff); //
	map(0xc00000, 0xc00013).r(FUNC(realbrk_state::pkgnshdx_input_r));   // P1 & P2 (Inputs)
	map(0xc00004, 0xc00005).writeonly().share("dsw_select"); // DSW select
	map(0xff0000, 0xfffbff).rw(FUNC(realbrk_state::backup_ram_dx_r), FUNC(realbrk_state::backup_ram_w)).share("backup_ram");  // RAM
}

/*dai2kaku specific memory map*/
void realbrk_state::dai2kaku_mem(address_map &map)
{
	base_mem(map);
	map(0x605000, 0x6053ff).ram().share("vram_0ras");   // rasterinfo   (0)
	map(0x605400, 0x6057ff).ram().share("vram_1ras");   // rasterinfo   (1)
	map(0x800008, 0x80000b).w("ymsnd", FUNC(ym2413_device::write)).umask16(0x00ff); //
	map(0xc00000, 0xc00001).portr("IN0");                            // P1 & P2 (Inputs)
	map(0xc00002, 0xc00003).portr("IN1");                            // Coins
	map(0xc00004, 0xc00005).ram().r(FUNC(realbrk_state::realbrk_dsw_r)).share("dsw_select");  // DSW select
	map(0xff0000, 0xfffbff).ram();                                         // RAM
}

/***************************************************************************

                                Input Ports

***************************************************************************/

/***************************************************************************
                        Billiard Academy Real Break
***************************************************************************/

static INPUT_PORTS_START( realbrk )
	PORT_START("IN0")   // IN0 - $c00000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )   // BUTTON3 in test mode
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )   // BUTTON3 in test mode
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")   // IN1 - $c00002.w
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_SERVICE ) PORT_NAME(DEF_STR( Test ))
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // the vblank routine wants these 2 bits high
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SW1")   // IN2 - $c00004.w (DSW1)
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:9")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0200, IP_ACTIVE_LOW, "SW1:10" )

	PORT_START("SW2")   // IN3 - $c00004.w (DSW2)
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPSETTING(      0x0001, "5" )
	PORT_DIPSETTING(      0x0003, "6" )
	PORT_DIPSETTING(      0x0002, "7" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0004, DEF_STR( Harder )  )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0030, 0x0030, "Time" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0000, "110" )
	PORT_DIPSETTING(      0x0030, "120" )
	PORT_DIPSETTING(      0x0020, "130" )
	PORT_DIPSETTING(      0x0010, "150" )
	PORT_DIPNAME( 0x0040, 0x0040, "Showtime With Easy Mode" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Coins for Versus" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0080, "2" )
	PORT_DIPUNUSED_DIPLOC( 0x0100, 0x0100, "SW2:9" ) /* Manual doesn't even show switches 9 & 10 */
	PORT_DIPUNUSED_DIPLOC( 0x0200, 0x0200, "SW2:10" )

	PORT_START("SW3")   // IN4 - $c00004.w (DSW3) - Unused
	PORT_BIT( 0xffff, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("SW4")   // IN5 - $c00004.w (DSW4) - Unused
	PORT_BIT( 0xffff, IP_ACTIVE_LOW,  IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( pkgnsh )
	PORT_START("IN0")   // IN0 - $c00002.w
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_SERVICE ) PORT_NAME(DEF_STR( Test ))
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // the vblank routine wants these 2 bits high
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SW1")
	PORT_BIT( 0x003f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_UNUSED )/*pkgnsh wants these two bits to be 0*/
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SW2")   // IN3 - $c00004.w (DSW2)
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0xf000, 0xf000, "Balls Per Credit" )
	PORT_DIPSETTING(      0x7000, "5 Balls" )
	PORT_DIPSETTING(      0x6000, "6 Balls" )
	PORT_DIPSETTING(      0x5000, "10 Balls" )
	PORT_DIPSETTING(      0x4000, "15 Balls" )
	PORT_DIPSETTING(      0x3000, "20 Balls" )
	PORT_DIPSETTING(      0x0000, "30 Balls" )
//  PORT_DIPSETTING(      0x1000, "30 Balls" ) /* Duplicate Setting for 30 balls */
//  PORT_DIPSETTING(      0x2000, "30 Balls" ) /* Duplicate Setting for 30 balls */
	PORT_DIPSETTING(      0xf000, "40 Balls" )
	PORT_DIPSETTING(      0xe000, "50 Balls" )
	PORT_DIPSETTING(      0xd000, "60 Balls" )
	PORT_DIPSETTING(      0xc000, "80 Balls" )
	PORT_DIPSETTING(      0x8000, "100 Balls" )
//  PORT_DIPSETTING(      0x9000, "100 Balls" ) /* Duplicate Setting for 100 balls */
//  PORT_DIPSETTING(      0xa000, "100 Balls" ) /* Duplicate Setting for 100 balls */
//  PORT_DIPSETTING(      0xb000, "100 Balls" ) /* Duplicate Setting for 100 balls */

	PORT_START("SW3")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PADDLE1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, 0x0000, IPT_PADDLE ) PORT_PLAYER(1) PORT_MINMAX(0,0x6400) PORT_SENSITIVITY(15) PORT_KEYDELTA(15) PORT_CENTERDELTA(0) //PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M)

	PORT_START("P1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) //F/F
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) //Stop
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PADDLE2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, 0x0000, IPT_PADDLE ) PORT_PLAYER(2) PORT_MINMAX(0,0x6400) PORT_SENSITIVITY(15) PORT_KEYDELTA(15) PORT_CENTERDELTA(0) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M)

	PORT_START("P2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )/*There's the Pay Out SW here IIRC*/
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) //F/F
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) //Stop
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( pkgnshdx )
	PORT_START("IN0")   // IN0 - $c00002.w
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_SERVICE ) PORT_NAME(DEF_STR( Test ))
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // the vblank routine wants these 2 bits high
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SW1") // IN1
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x00f0, 0x00f0, "Balls Per Credit" )
	PORT_DIPSETTING(      0x0070, "5 Balls" )
	PORT_DIPSETTING(      0x0060, "6 Balls" )
	PORT_DIPSETTING(      0x0050, "10 Balls" )
	PORT_DIPSETTING(      0x0040, "15 Balls" )
	PORT_DIPSETTING(      0x0030, "20 Balls" )
	PORT_DIPSETTING(      0x0000, "30 Balls" )
//  PORT_DIPSETTING(      0x0010, "30 Balls" ) /* Duplicate Setting for 30 balls */
//  PORT_DIPSETTING(      0x0020, "30 Balls" ) /* Duplicate Setting for 30 balls */
	PORT_DIPSETTING(      0x00f0, "40 Balls" )
	PORT_DIPSETTING(      0x00e0, "50 Balls" )
	PORT_DIPSETTING(      0x00d0, "60 Balls" )
	PORT_DIPSETTING(      0x00c0, "80 Balls" )
	PORT_DIPSETTING(      0x0080, "100 Balls" )
//  PORT_DIPSETTING(      0x0090, "100 Balls" ) /* Duplicate Setting for 100 balls */
//  PORT_DIPSETTING(      0x00a0, "100 Balls" ) /* Duplicate Setting for 100 balls */
//  PORT_DIPSETTING(      0x00b0, "100 Balls" ) /* Duplicate Setting for 100 balls */
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SW2") //IN2
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SW3") //IN3
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SW4") //IN4
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P1") //IN5
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x0002, 0x0002, "Pay-Out SW" )//Not a real DIP-Switch
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED ) //F/F
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) //Stop
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0040, 0x0040, "Analize SW" )//Not a real DIP-Switch
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, 0x0000, IPT_PADDLE ) PORT_PLAYER(1) PORT_MINMAX(0,0x6400) PORT_SENSITIVITY(15) PORT_KEYDELTA(15) PORT_CENTERDELTA(0) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M)

	PORT_START("P2") //IN6
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED ) //Again Pay-Out SW in test mode,but it doesn't work,maybe it is for Player-2?
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED ) //F/F
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) //Stop
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0040, 0x0040, "Mem Clear SW" )//Not a real DIP-Switch
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, 0x0000, IPT_PADDLE ) PORT_PLAYER(2) PORT_MINMAX(0,0x6400) PORT_SENSITIVITY(15) PORT_KEYDELTA(15) PORT_CENTERDELTA(0) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M)
INPUT_PORTS_END

static INPUT_PORTS_START( dai2kaku )
	PORT_START("IN0")   // IN0 - $c00000.w
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")   // IN1 - $c00002.w
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_SERVICE ) PORT_NAME(DEF_STR( Test ))
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // the vblank routine wants these 2 bits high
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SW1")   // IN2 - $c00004.w (DSW1)
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(      0x0009, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_1C ) )
		PORT_DIPNAME( 0x00f0, 0x00f0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(      0x0090, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x00b0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x00d0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(      0x00f0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 4C_1C ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:9")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0200, "SW1:10" )

	PORT_START("SW2")   // IN3 - $c00004.w (DSW2)
	PORT_DIPUNKNOWN_DIPLOC( 0x0001, 0x0001, "SW2:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0002, 0x0002, "SW2:2" )
	PORT_DIPNAME( 0x000c, 0x000c, "Starting Points" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0008, "350" )
	PORT_DIPSETTING(      0x0004, "650" )
	PORT_DIPSETTING(      0x000c, "1000" )
	PORT_DIPSETTING(      0x0000, "1500" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, 0x0010, "SW2:5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0040, 0x0040, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0080, 0x0080, "SW2:8" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0100, 0x0100, "SW2:9" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0200, "SW2:10" )

	PORT_START("SW3")   // IN4 - $c00004.w (DSW3) - Unused
	PORT_BIT( 0xffff, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("SW4")   // IN5 - $c00004.w (DSW4) - Unused
	PORT_BIT( 0xffff, IP_ACTIVE_LOW,  IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************

                            Graphics Layouts

***************************************************************************/

static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{   STEP4(0,1)      },
	{   STEP4(3*4,-4),STEP4(7*4,-4)     },
	{   STEP8(0,8*4)    },
	8*8*4
};

static const gfx_layout layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{   STEP8(0,1)      },
	{   STEP16(0,8)     },
	{   STEP16(0,16*8)  },
	16*16*8
};

static GFXDECODE_START( gfx_realbrk )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x8,         0, 0x80     )   // [0] Backgrounds
	GFXDECODE_ENTRY( "gfx2", 0, layout_8x8x4,           0, 0x800    )   // [1] Text
	GFXDECODE_ENTRY( "gfx3", 0, layout_16x16x8,         0, 0x80     )   // [2] Sprites (256 colors)
	GFXDECODE_ENTRY( "gfx4", 0, gfx_16x16x4_packed_lsb, 0, 0x800    )   // [3] Sprites (16 colors)
GFXDECODE_END

static GFXDECODE_START( gfx_dai2kaku )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x8,     0, 0x80     )   // [0] Backgrounds
	GFXDECODE_ENTRY( "gfx2", 0, layout_8x8x4,       0, 0x800    )   // [1] Text
	GFXDECODE_ENTRY( "gfx3", 0, layout_16x16x8,     0, 0x80     )   // [2] Sprites (256 colors)
GFXDECODE_END


/***************************************************************************

                                Machine Drivers

***************************************************************************/

/***************************************************************************
                        Billiard Academy Real Break
***************************************************************************/

void realbrk_state::realbrk(machine_config &config)
{
	/* basic machine hardware */
	TMP68301(config, m_maincpu, XTAL(32'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &realbrk_state::realbrk_mem);
	m_maincpu->parallel_w_cb().set(FUNC(realbrk_state::realbrk_flipscreen_w));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(0x140, 0xe0);
	m_screen->set_visarea(0, 0x140-1, 0, 0xe0-1);
	m_screen->set_screen_update(FUNC(realbrk_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, 1);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_realbrk);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x8000);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	ymz280b_device &ymz(YMZ280B(config, "ymz", XTAL(33'868'800) / 2));
	ymz.add_route(0, "speaker", 0.50, 0);
	ymz.add_route(1, "speaker", 0.50, 1);

	ym2413_device &ymsnd(YM2413(config, "ymsnd", XTAL(3'579'545)));
	ymsnd.add_route(ALL_OUTPUTS, "speaker", 0.25, 0);
	ymsnd.add_route(ALL_OUTPUTS, "speaker", 0.25, 1);
}

void realbrk_state::pkgnsh(machine_config &config)
{
	realbrk(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &realbrk_state::pkgnsh_mem);
	m_maincpu->parallel_w_cb().set_nop();
}

void realbrk_state::pkgnshdx(machine_config &config)
{
	pkgnsh(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &realbrk_state::pkgnshdx_mem);
}

void realbrk_state::dai2kaku(machine_config &config)
{
	realbrk(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &realbrk_state::dai2kaku_mem);
	m_maincpu->parallel_w_cb().set(FUNC(realbrk_state::dai2kaku_flipscreen_w));

	m_gfxdecode->set_info(gfx_dai2kaku);
	m_screen->set_screen_update(FUNC(realbrk_state::screen_update_dai2kaku));
}


/***************************************************************************

                                ROMs Loading

***************************************************************************/

/***************************************************************************

Pachinko Gindama Shoubu
Nakanihon Inc. / Dynax, 1998

PCB Layout
----------

NM5050905-1
|----------------------------------------------------------------------|
| TA8201 75074 YM2413          |-----|     |------------------------|  |
|   PST532A  3V_BATT 32.768kHz |68301|  CN2|------------------------|  |
|        75074  M6242B         |     |     TC55257   YMZ280B           |
|  YAC513      3.579545MHz     |-----|     TC55257             TC55257 |
|                              |--------|                              |
|                              |ACTEL   |                      TC55257 |
|                              |A1010B  |                              |
|                              |(PLCC68)|                      TC55257 |
|                              |--------|                              |
|J         TC55257                                             TC55257 |
|A                                                                     |
|M         TC55257                                                     |
|M                                  M548262                            |
|A                                  M548262         32.000MHz          |
|                                               TC55257                |
|                                  |---------|             |---------| |
|    DSW1(10)                      |NIKANIHON|  TC55257    |NIKANIHON| |
|                                  |4L10F2468|             |4L10F2467| |
|    DSW2(10)     TC55257          |         |             |         | |
|                                  |(QFP160) |             |(QFP160) | |
|                 TC55257          |---------|             |---------| |
|   CN6                                    |------------------------|  |
|   CN7              33.8688MHz         CN3|------------------------|  |
|----------------------------------------------------------------------|
Notes:
              *: unpopulated 32 pin sockets
         PST532: IC for system reset and battery backup switching
        DIPSW's: Each have 10 switches.
        CN2/CN3: ROM daughterboard connectors
        CN6/CN7: Connectors for player 3 & 4 controls
        TC55257: 32K x8 SRAM
        M548262: 256K x8 fastpage VRAM

  Vertical Sync: 60Hz
    Horiz. Sync: 15.81kHz
    68301 clock: 16.000MHz (32 / 2)
  YMZ280B clock: 16.9344MHz (33.8688 / 2)
   YM2413 clock: 3.579545MHz
OKI M6242 clock: 32.768kHz


ROM Daughterboard
-----------------

NS5050905-2
|----------------------------------------|
|            |------------------------|  |
|         CN1|------------------------|  |
|                                        |
|  50505.1K     50507.1F    50510.1C     |
|     50506.1H      50508.1E    50511.1B |
|                                        |
|              PAL   50509.2D    50512.2A|
|                                        |
|              PAL                       |
|                                        |
|                                        |
|50503.3L          *    *    *    *      |
|      50504.3K                          |
|                                        |
|                                        |
|                                        |
|                                        |
|50501.4L          *    *    *    *      |
|      50502.4K                          |
|                                        |
|                                        |
|            |------------------------|  |
|         CN2|------------------------|  |
|----------------------------------------|
Notes:
      *: Unpopulated 32pin sockets

***************************************************************************/

ROM_START( pkgnsh )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* TMP68301 Code */
	ROM_LOAD16_BYTE( "50506.1h", 0x000001, 0x080000, CRC(06949a7d) SHA1(1276c28bc5cebeae749e0cded2da631353efbbb4) )
	ROM_LOAD16_BYTE( "50505.1k", 0x000000, 0x080000, CRC(26df869f) SHA1(d716e561441da6ae8ca61e17335aab44770157a6) )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* Backgrounds */
	ROM_LOAD32_WORD( "50512.2a", 0x0000002, 0x200000, CRC(5adae7bb) SHA1(de7cf952155459f7aab1448620bf26a925ca0572) )
	ROM_LOAD32_WORD( "50509.2d", 0x0000000, 0x200000, CRC(ad937ab5) SHA1(ebe02c203358787c6b406fe3cbd3eca3b245456e) )

	ROM_REGION( 0x40000, "gfx2", 0 )    /* Text Layer */
	ROM_LOAD16_BYTE( "50510.1c", 0x000001, 0x020000, CRC(e26f1ad6) SHA1(5713af5fb02885657889cb2df7f58a39987ace6a) )
	ROM_LOAD16_BYTE( "50511.1b", 0x000000, 0x020000, CRC(3da9af01) SHA1(21313fd5c8cf7ccb72c85422dbddfceedab2542f) )

	ROM_REGION( 0xc00000, "gfx3", 0 )   /* Sprites (256 colors) */
	ROM_LOAD32_WORD( "50502.4k", 0x0000000, 0x400000, CRC(f7c04779) SHA1(fcbc2d166d405d0fe2a4ca67950fe6ec060b9fc1) ) // same as 52206.9f on dx
	ROM_LOAD32_WORD( "50504.3k", 0x0000002, 0x400000, CRC(8e872be5) SHA1(0568a70ca640624f665b8b92ca5e9239b13ed116) ) // same as 52208.9d on dx
	ROM_LOAD32_WORD( "50501.4l", 0x0800000, 0x200000, CRC(ca31e1ad) SHA1(7508547de5617f6091fc46f6eb1b45673419c483) )
	ROM_LOAD32_WORD( "50503.3l", 0x0800002, 0x200000, CRC(80b5e8d0) SHA1(27359affaa84c7cb4dfc019bbfeae0f384602faa) )

	ROM_REGION( 0x200000, "gfx4", ROMREGION_ERASEFF )   /* Sprites (16 colors) Not Used */

	ROM_REGION( 0x100000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "50508.1e", 0x000000, 0x080000, CRC(95a1473a) SHA1(d382a9a603711747c2fe5bd5721de5af369ccc42) )
	ROM_LOAD( "50507.1f", 0x080000, 0x080000, CRC(34a003a1) SHA1(f3fa4de1f75e8fa18a8431a8c2ce495aa47989b9) )
ROM_END

/***************************************************************************

Pachinko Gindama Shoubu DX
Nakanihon Inc. / Dynax, 1998

PCB Layout
----------

NM522-1-9803
|----------------------------------------------------------------------|
||-----|              |--------|                                       |
||68301| 52202B       |ACTEL   | M548262   YMZ280B          52204      |
||     | 52201B       |TPC1010 | M548262   32.768kHz 52203  52205      |
||-----|              |(PLCC68)|           M6242B   3V_BATT            |
|                     |--------||---------|  PST532A      |---------|  |
|            TC55257 33.8688MHz |NIKANIHON|               |NIKANIHON|  |
|            TC55257            |4L10F2468|    32.000MHz  |4L10F2467|  |
|2            TC55257           |         |               |         |  |
|8            TC55257           |(QFP160) |               |(QFP160) |  |
|W                              |---------| PAL           |---------|  |
|A                           LC3664     52206     52208     52210      |
|Y      VOL1                 LC3664          52207     52209     52211 |
|       VOL2                     LC3664                                |
| TA7252      3404               LC3664                                |
|     3404                                                             |
|  YAC516  YM2413        TC55257                                       |
|            3.579545MHz TC55257                                       |
| DSW2(10) DSW1(10)                                                    |
| DSW3(10) DSW4(10)             *    *    *    *    *    *    *    *   |
|                        TC55257                                       |
| CN3                    TC55257                                       |
| CN4                                                                  |
|----------------------------------------------------------------------|

Notes:
              *: unpopulated 32 pin sockets
         PST532: IC for system reset and battery backup switching
        DIPSW's: Each have 10 switches.
      VOL1/VOL2: Separate volume levels for sound and voice
        CN3/CN4: Connectors for player 3 & 4 controls
        TC55257: 32K x8 SRAM
         LC3664: 8K x8 SRAM
        M548262: 256K x8 fastpage VRAM
          28WAY: Edge connector is JAMMA backwards (GND is pin 28,27; +5V is pins 26,25 etc)
                 and with RBG locations moved (i.e. not standard JAMMA)

  Vertical Sync: 60Hz
    Horiz. Sync: 15.81kHz
    68301 clock: 16.000MHz (32 / 2)
  YMZ280B clock: 16.9344MHz (33.8688 / 2)
   YM2413 clock: 3.579545MHz
OKI M6242 clock: 32.768kHz

***************************************************************************/

ROM_START( pkgnshdx )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* TMP68301 Code */
	ROM_LOAD16_BYTE( "52202b.1r", 0x000000, 0x080000, CRC(3c1a10de) SHA1(44a13adec64645aa01e216dfd527b59e7298c732) )
	ROM_LOAD16_BYTE( "52201b.2r", 0x000001, 0x080000, CRC(d63797ce) SHA1(d1b0b57b5426135e36772be296e94e04822e54ac) )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* Backgrounds */
	ROM_LOAD32_WORD( "52210.9b", 0x0000000, 0x200000, CRC(6865b76a) SHA1(26215ec38b1fef279b3c3c1453116a0afe938b6b) )
	ROM_LOAD32_WORD( "52211.9a", 0x0000002, 0x200000, CRC(8e227328) SHA1(200f9e4419dac62b191e5e8c6c32b777a9c08e5e) )

	ROM_REGION( 0x40000, "gfx2", 0 )    /* Text Layer */
	ROM_LOAD16_BYTE( "52205.1a", 0x000000, 0x020000, CRC(4b7d16c0) SHA1(5f6410121ec13bea2869d61db169dbe2536453ea) )
	ROM_LOAD16_BYTE( "52204.1b", 0x000001, 0x020000, CRC(47a39496) SHA1(3ac9499b70c63185fb65378c18d4ff30ba1d2f2b) )

	ROM_REGION( 0xc00000, "gfx3", 0 )   /* Sprites (256 colors) */
	ROM_LOAD32_WORD( "52206.9f", 0x0000000, 0x400000, CRC(f7c04779) SHA1(fcbc2d166d405d0fe2a4ca67950fe6ec060b9fc1) )
	ROM_LOAD32_WORD( "52208.9d", 0x0000002, 0x400000, CRC(8e872be5) SHA1(0568a70ca640624f665b8b92ca5e9239b13ed116) )
	ROM_LOAD32_WORD( "52207.9e", 0x0800000, 0x200000, CRC(ae7a983f) SHA1(ba8ff28068e21dd24ea2e523a5b4023e86ea26cb) )
	ROM_LOAD32_WORD( "52209.9c", 0x0800002, 0x200000, CRC(83ac2ea9) SHA1(aa1c45b7a404eed51e950bea3edcd34814f09213) )

	ROM_REGION( 0x200000, "gfx4", ROMREGION_ERASEFF )   /* Sprites (16 colors) Not Used */

	ROM_REGION( 0x100000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "52203.2e", 0x000000, 0x100000, CRC(342a193d) SHA1(1e75ec7ac48dcc8396a0fa6db14f2661c28f671c) )
ROM_END

/***************************************************************************

                        Billiard Academy Real Break

Dynax, 1998

PCB Layout
----------


NM523-1-9805
------------------------------------------------------------------------
|                                                                      |
|  68301-16    52302    ACTEL    M548262   YMZ280B     *    52304      |
|              52301    A1010A   M548262             52303  52305      |
|                             33.8688MHz                               |
|                      TC55257               PST532A   %               |
|                      TC55257   4L10F2468                4L10F2467    |
|                      TC55257   (QFP160)      32.000MHz  (QFP160)     |
| J                    TC55257                                         |
| A                                     PAL                            |
| M                                                                    |
| M                              LH5160 52306     52308     52310      |
| A     VOL1                     LH5160      52307     52309     52311 |
|       VOL2                     LH5160                                |
|                                LH5160                                |
|                                                                      |
|          YM2413                TC55257                               |
|                    3.579MHz    TC55257  *  52312  *    *    *    *   |
|                                                                      |
|   YAC516                                                             |
|                             TC55257  TC55257                         |
|   DSW2(10) DSW1(10)         TC55257  TC55257                         |
|                                                                      |
------------------------------------------------------------------------

Notes:
            *: unpopulated 42 pin sockets.
            %: unpopulated position for coin battery.
       PST532: IC for system reset and battery backup switching
      DIPSW's: Each have 10 switches.
    VOL1/VOL2: Separate volume levels for sound and voice
Vertical Sync: 60Hz
  Horiz. Sync: 15.81kHz

***************************************************************************/

ROM_START( realbrk )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* TMP68301 Code */
	ROM_LOAD16_BYTE( "600k02b.1r", 0x000000, 0x080000, CRC(6954ff7f) SHA1(dc17be7dadb2d6acff039d4d6484ee71070e466d) )
	ROM_LOAD16_BYTE( "600k01b.2r", 0x000001, 0x080000, CRC(6eb865bf) SHA1(07bcdbec8fd8d280b1cdb4b5545607d3a87e2395) )

	ROM_REGION( 0x800000, "gfx1", 0 )   /* Backgrounds */
	ROM_LOAD32_WORD( "52310.9b", 0x0000000, 0x400000, CRC(07dfd9f5) SHA1(8722a98adc33f56df1e3b194ce923bc987e15cbe) )
	ROM_LOAD32_WORD( "52311.9a", 0x0000002, 0x400000, CRC(136a93a4) SHA1(b4bd46ba6c2b367aaf362f67d8be4757f1160864) )

	ROM_REGION( 0x40000, "gfx2", 0 )    /* Text Layer */
	ROM_LOAD16_BYTE( "52305.1a", 0x000000, 0x020000, CRC(56546fb4) SHA1(5e4dc1665ca96bf24b89d92c24f5ff8420cb465e) ) // 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "52304.1b", 0x000001, 0x020000, CRC(b22b0aac) SHA1(8c62e19071a4031d0dcad621cce0ba550702659b) ) // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0xc00000, "gfx3", 0 )   /* Sprites (256 colors) */
	ROM_LOAD32_WORD( "52306.9f",   0x0000000, 0x400000, CRC(5ff0f666) SHA1(e3f1d9dc84fbef73af37cefd90bdf87a35f59e0e) )
	ROM_LOAD32_WORD( "52308.9d",   0x0000002, 0x400000, CRC(20817051) SHA1(4c9a443b5d6353ce67d5b1fe716f5ac20d194ef0) )
	ROM_LOAD32_WORD( "mm60007.9e", 0x0800000, 0x200000, CRC(a1d40934) SHA1(59b85435b13c6617e79b8d995506e585b6c8bedd) )
	ROM_LOAD32_WORD( "mm60009.9c", 0x0800002, 0x200000, CRC(58c03a6c) SHA1(ec7ae49bba6ffdba0f79f1e41e14945f6c3acb1d) )

	ROM_REGION( 0x200000, "gfx4", 0 )   /* Sprites (16 colors) */
	ROM_LOAD( "mm60012.14f", 0x000000, 0x200000, CRC(2b5ba1ec) SHA1(d548ef8c96b7b868c866dedb314f56583726564d) )

	ROM_REGION( 0x400000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "mm60003.2e", 0x000000, 0x400000, CRC(39512459) SHA1(b5859a7d8f2f87d923e7f86f095cbffd31f9cbfa) )
ROM_END

ROM_START( realbrko )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* TMP68301 Code */
	ROM_LOAD16_BYTE( "52302.1r", 0x000000, 0x080000, CRC(76de5e26) SHA1(d05b48f024053198fb63b1c4e5454ecb8fe302a4) ) // sldh w/realbrkj
	ROM_LOAD16_BYTE( "52301.2r", 0x000001, 0x080000, CRC(29e979df) SHA1(61b30b2f8f16bb92a3ec1cd7abd157930f1a3f29) ) // sldh w/realbrkj

	// note, the numbering on all the roms is shifted by 1 due to the sample data being split across 2 roms
	//  this is how the board is labeled, it is not a mistake.
	ROM_REGION( 0x800000, "gfx1", 0 )   /* Backgrounds */
	ROM_LOAD32_WORD( "52311.9b", 0x0000000, 0x400000, CRC(07dfd9f5) SHA1(8722a98adc33f56df1e3b194ce923bc987e15cbe) )
	ROM_LOAD32_WORD( "52312.9a", 0x0000002, 0x400000, CRC(136a93a4) SHA1(b4bd46ba6c2b367aaf362f67d8be4757f1160864) )

	ROM_REGION( 0x40000, "gfx2", 0 )    /* Text Layer */
	ROM_LOAD16_BYTE( "52306.1a", 0x000000, 0x020000, CRC(56546fb4) SHA1(5e4dc1665ca96bf24b89d92c24f5ff8420cb465e) ) // 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "52305.1b", 0x000001, 0x020000, CRC(b22b0aac) SHA1(8c62e19071a4031d0dcad621cce0ba550702659b) ) // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0xc00000, "gfx3", 0 )   /* Sprites (256 colors) */
	ROM_LOAD32_WORD( "52307.9f",   0x0000000, 0x400000, CRC(5ff0f666) SHA1(e3f1d9dc84fbef73af37cefd90bdf87a35f59e0e) )
	ROM_LOAD32_WORD( "52309.9d",   0x0000002, 0x400000, CRC(20817051) SHA1(4c9a443b5d6353ce67d5b1fe716f5ac20d194ef0) )
	ROM_LOAD32_WORD( "52308.9e",   0x0800000, 0x200000, CRC(a1d40934) SHA1(59b85435b13c6617e79b8d995506e585b6c8bedd) )
	ROM_LOAD32_WORD( "52310.9c",   0x0800002, 0x200000, CRC(58c03a6c) SHA1(ec7ae49bba6ffdba0f79f1e41e14945f6c3acb1d) )

	ROM_REGION( 0x200000, "gfx4", 0 )   /* Sprites (16 colors) */
	ROM_LOAD( "52313.14f", 0x000000, 0x200000, CRC(2b5ba1ec) SHA1(d548ef8c96b7b868c866dedb314f56583726564d) )

	ROM_REGION( 0x400000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "52303.2e", 0x000000, 0x200000, CRC(8a8a7d42) SHA1(7108203cf5a6a1603bfcbc5bde40e71ac960fffc) ) // sldh w/realbrkj
	ROM_LOAD( "52304.1e", 0x200000, 0x200000, CRC(c8c5ef57) SHA1(4f363b36191f9c647fa88f07286bf9d667005553) )
ROM_END


ROM_START( realbrkj )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* TMP68301 Code */
	ROM_LOAD16_BYTE( "52302.1r", 0x000000, 0x080000, CRC(ab0379b0) SHA1(67af6670f2b37a7d4d6e03508f291f8ffe64d4cb) ) // sldh w/realbrko
	ROM_LOAD16_BYTE( "52301.2r", 0x000001, 0x080000, CRC(9cc1596e) SHA1(a598f18eaac1ed6943069e9500b07b77e263f0d0) ) // sldh w/realbrko

	ROM_REGION( 0x800000, "gfx1", 0 )   /* Backgrounds */
	ROM_LOAD32_WORD( "52310.9b", 0x0000000, 0x400000, CRC(07dfd9f5) SHA1(8722a98adc33f56df1e3b194ce923bc987e15cbe) )
	ROM_LOAD32_WORD( "52311.9a", 0x0000002, 0x400000, CRC(136a93a4) SHA1(b4bd46ba6c2b367aaf362f67d8be4757f1160864) )

	ROM_REGION( 0x40000, "gfx2", 0 )    /* Text Layer */
	ROM_LOAD16_BYTE( "52305.1a", 0x000000, 0x020000, CRC(56546fb4) SHA1(5e4dc1665ca96bf24b89d92c24f5ff8420cb465e) ) // 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "52304.1b", 0x000001, 0x020000, CRC(b22b0aac) SHA1(8c62e19071a4031d0dcad621cce0ba550702659b) ) // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0xc00000, "gfx3", 0 )   /* Sprites (256 colors) */
	ROM_LOAD32_WORD( "52306.9f", 0x0000000, 0x400000, CRC(5ff0f666) SHA1(e3f1d9dc84fbef73af37cefd90bdf87a35f59e0e) )
	ROM_LOAD32_WORD( "52308.9d", 0x0000002, 0x400000, CRC(20817051) SHA1(4c9a443b5d6353ce67d5b1fe716f5ac20d194ef0) )
	ROM_LOAD32_WORD( "52307.9e", 0x0800000, 0x200000, CRC(01555191) SHA1(7751e2e16345acc638d4dff997a5b52e9171fced) )
	ROM_LOAD32_WORD( "52309.9c", 0x0800002, 0x200000, CRC(ef4f4bd9) SHA1(3233f501002a2622ddda581167ae24b1a13ea79e) )

	ROM_REGION( 0x200000, "gfx4", 0 )   /* Sprites (16 colors) */
	ROM_LOAD( "52312.14f", 0x000000, 0x200000, CRC(2203d7c5) SHA1(0403f02b8f2bfc6cf98ff598eb9c2e3facc7ac4c) )

	ROM_REGION( 0x400000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "52303.2e", 0x000000, 0x400000, CRC(d3005b1e) SHA1(3afd10cdbc3aa7605083a9fcf3c4b8276937c2c4) ) // sldh w/realbrko
ROM_END

ROM_START( realbrkk )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* TMP68301 Code */
	ROM_LOAD16_BYTE( "600k_02b", 0x000000, 0x080000, CRC(fdca08b1) SHA1(69b35c85b1842d0a8c98fc519b46c72954322ceb) )
	ROM_LOAD16_BYTE( "600k_01b", 0x000001, 0x080000, CRC(b6fe8998) SHA1(86f7d6067e007de50a02478a0e583ab64408bc4f) )

	ROM_REGION( 0x800000, "gfx1", 0 )   /* Backgrounds */
	ROM_LOAD32_WORD( "52310.9b", 0x0000000, 0x400000, CRC(07dfd9f5) SHA1(8722a98adc33f56df1e3b194ce923bc987e15cbe) )
	ROM_LOAD32_WORD( "52311.9a", 0x0000002, 0x400000, CRC(136a93a4) SHA1(b4bd46ba6c2b367aaf362f67d8be4757f1160864) )

	ROM_REGION( 0x40000, "gfx2", 0 )    /* Text Layer */
	ROM_LOAD16_BYTE( "600k_05", 0x000000, 0x020000, CRC(4de1d95e) SHA1(093d6d229b0e43e35f84a8d1bd707ccd1452fa91) )  // 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "600k_04", 0x000001, 0x020000, CRC(70f2cf3d) SHA1(214550b1a838243fadf5c6b8ba6cbecef2031985) )  // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0xc00000, "gfx3", 0 )   /* Sprites (256 colors) */
	ROM_LOAD32_WORD( "52306.9f",   0x0000000, 0x400000, CRC(5ff0f666) SHA1(e3f1d9dc84fbef73af37cefd90bdf87a35f59e0e) )
	ROM_LOAD32_WORD( "52308.9d",   0x0000002, 0x400000, CRC(20817051) SHA1(4c9a443b5d6353ce67d5b1fe716f5ac20d194ef0) )
	ROM_LOAD32_WORD( "mm60007.9e", 0x0800000, 0x200000, CRC(a1d40934) SHA1(59b85435b13c6617e79b8d995506e585b6c8bedd) )
	ROM_LOAD32_WORD( "mm60009.9c", 0x0800002, 0x200000, CRC(58c03a6c) SHA1(ec7ae49bba6ffdba0f79f1e41e14945f6c3acb1d) )

	ROM_REGION( 0x200000, "gfx4", 0 )   /* Sprites (16 colors) */
	ROM_LOAD( "mm60012.14f", 0x000000, 0x200000, CRC(2b5ba1ec) SHA1(d548ef8c96b7b868c866dedb314f56583726564d) )

	ROM_REGION( 0x400000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "mm60003.2e", 0x000000, 0x400000, CRC(39512459) SHA1(b5859a7d8f2f87d923e7f86f095cbffd31f9cbfa) )
ROM_END

// Team Japump board:

ROM_START( dai2kaku )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* TMP68301 Code */
	ROM_LOAD16_BYTE( "52201b.2r", 0x000000, 0x080000, CRC(5672cbe6) SHA1(4379edd0725e1b8cd5b3f9201e484487eccd1714) )
	ROM_LOAD16_BYTE( "52202b.1r", 0x000001, 0x080000, CRC(e45d6368) SHA1(5fb39b7c2e0fd474e7c366279f616b9244e6cf2e) )

	ROM_REGION( 0x800000, "gfx1", 0 )   /* Backgrounds */
	ROM_LOAD32_WORD( "52210.9b", 0x0000000, 0x400000, CRC(29f0cd88) SHA1(e8eab4f3e4cb12663874d4f4a2fefc77d15fa078) )
	ROM_LOAD32_WORD( "52211.9a", 0x0000002, 0x400000, CRC(304f896d) SHA1(fe46e0a9c497f1a9587933929520e1b7a9321c01) )

	ROM_REGION( 0x40000, "gfx2", 0 )    /* Text Layer */
	ROM_LOAD16_BYTE( "52205.1a", 0x000000, 0x020000, CRC(4b7d16c0) SHA1(5f6410121ec13bea2869d61db169dbe2536453ea) )
	ROM_LOAD16_BYTE( "52204.1b", 0x000001, 0x020000, CRC(47a39496) SHA1(3ac9499b70c63185fb65378c18d4ff30ba1d2f2b) )

	ROM_REGION( 0x800000, "gfx3", 0 )   /* Sprites (256 colors) */
	ROM_LOAD32_WORD( "52206.9f", 0x0000000, 0x400000, CRC(a8811f46) SHA1(f26fd3b567cd2974970a9e4495a16d8a3406c5d1) )
	ROM_LOAD32_WORD( "52208.9d", 0x0000002, 0x400000, CRC(00c39300) SHA1(2dd2420700d9a6ec2ade595bccd25725bf60762b) )

	ROM_REGION( 0x100000, "ymz", 0 )    /* Samples */
	ROM_LOAD( "52203.2e", 0x000000, 0x100000, CRC(a612ba97) SHA1(b7bb903f8e00ce5febf3b68a3d892da9162b45be) )
ROM_END

// Guru board:

/***************************************************************************

Dai Dai Kakumei
SystemBit Corp. 2004

Same PCB as Pachinko Gindama Shoubu DX (Japan)

PCB Layout
----------

NM522-1-9803
|-------------------------------------------------------|
|  TMP68301   1R    TPC1010  M548262     2E    MN52204  |
|             2R             M548262           MN52205  |
|                 TC55257         32.768kHz             |
|                 TC55257  33.8688MHz   PST532          |
|M                             M6242 BATTERY   4L10F2467|
|A     VR1                          PAL                 |
|H     VR2                4L10F2468     32MHz           |
|J  TA7252                                              |
|O        JRC3404                                       |
|N  YAC516  JRC3404                                     |
|G   YM2413                LC3664                       |
|                          LC3664    9F    9D    9B 9A  |
|DSW2 DSW1   3.579545MHz           LC3664               |
|DSW3 DSW4             TC55257     LC3664               |
|                      TC55257                          |
|CN3                   TC55257                          |
|CN4                   TC55257                          |
|-------------------------------------------------------|
Notes:
      TMP68301 - clock 16.000MHz
      YM2413   - clock 3.579545MHz
      YMZ280B  - clock 16.9344MHz [33.8688/2]
      VSync    - 59.8986Hz
      HSync    - 15.332kHz
      TPC1010  - Actel CPLD
      TC55257  - 256kx8 SRAM
      LC3664   - 64kx8 SRAM
      M548262  - 256k x8 fastpage VRAM
      PST532   - IC for system reset and battery backup switching
      DIPSW's  - Each have 10 switches
      VOL1/VOL2- Separate volume levels for sound and voice
      CN3/CN4  - Connectors for player 3 & 4 controls

27C160 EPROMs. They don't have numbers like the other dump.
There is no 52203 number on the ROMs (or any other),
just 9A 9B 9D 9F 2E 2R 1R.
The only ones with numbers are the EPROMs at 1A and 1B.

***************************************************************************/
/*
This dump matches the already supported set, except for rom sizes:

    1r                      52201B.2R               IDENTICAL
    2r                      52202B.1R               IDENTICAL
    nm52204.1b              52204.1B                IDENTICAL
    nm52205.1a              52205.1A                IDENTICAL
    2e           [1/2]      52203.2E                IDENTICAL
    9a                      52211.9A     [1/2]      IDENTICAL
    9b                      52210.9B     [1/2]      IDENTICAL
    9d                      52208.9D     [1/2]      IDENTICAL
    9f                      52206.9F     [1/2]      IDENTICAL
*/
#if 0
ROM_START( dai2kaku_alt_rom_size )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1r", 0x000000, 0x080000, CRC(5672cbe6) SHA1(4379edd0725e1b8cd5b3f9201e484487eccd1714) )
	ROM_LOAD16_BYTE( "2r", 0x000001, 0x080000, CRC(e45d6368) SHA1(5fb39b7c2e0fd474e7c366279f616b9244e6cf2e) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD32_WORD( "9b", 0x000000, 0x200000, CRC(eab21697) SHA1(28540e04c0d4040328f3d3373e623138a8730c07) )
	ROM_LOAD32_WORD( "9a", 0x000002, 0x200000, CRC(1b12dd4e) SHA1(8be13b72b2427eb66f1c4f736aa8bde0a8e1bc9b) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "nm52204.1b", 0x000000, 0x020000, CRC(47a39496) SHA1(3ac9499b70c63185fb65378c18d4ff30ba1d2f2b) )
	ROM_LOAD16_BYTE( "nm52205.1a", 0x000001, 0x020000, CRC(4b7d16c0) SHA1(5f6410121ec13bea2869d61db169dbe2536453ea) )

	ROM_REGION( 0x400000, "gfx3", 0 )
	ROM_LOAD32_WORD( "9f", 0x000000, 0x200000, CRC(63a1a280) SHA1(a6a4b7ebe2b8d57f5c1fce43c9b28c5dc2d6057a) )
	ROM_LOAD32_WORD( "9d", 0x000002, 0x200000, CRC(1743a929) SHA1(4756e9c567c406f635c6380e263f38a6a2a82038) )

	ROM_REGION( 0x200000, "ymz", 0 )
	ROM_LOAD( "2e", 0x000000, 0x200000, CRC(833856b5) SHA1(10663abd2a051f95c07b301573124ae53f902044) )
ROM_END
#endif

GAME( 1998, pkgnsh,   0,       pkgnsh,   pkgnsh,   realbrk_state, empty_init, ROT0, "Nakanihon / Dynax", "Pachinko Gindama Shoubu (Japan)",      MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 1998, pkgnshdx, 0,       pkgnshdx, pkgnshdx, realbrk_state, empty_init, ROT0, "Nakanihon / Dynax", "Pachinko Gindama Shoubu DX (Japan)",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 1998, realbrk,  0,       realbrk,  realbrk,  realbrk_state, empty_init, ROT0, "Nakanihon",         "Billiard Academy Real Break (Europe)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1998, realbrko, realbrk, realbrk,  realbrk,  realbrk_state, empty_init, ROT0, "Nakanihon",         "Billiard Academy Real Break (Europe, older)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1998, realbrkj, realbrk, realbrk,  realbrk,  realbrk_state, empty_init, ROT0, "Nakanihon",         "Billiard Academy Real Break (Japan)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1998, realbrkk, realbrk, realbrk,  realbrk,  realbrk_state, empty_init, ROT0, "Nakanihon",         "Billiard Academy Real Break (Korea)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 2004, dai2kaku, 0,       dai2kaku, dai2kaku, realbrk_state, empty_init, ROT0, "SystemBit",         "Dai-Dai-Kakumei (Japan)",              MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
