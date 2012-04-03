/******************************************************************************************

Sengoku Mahjong (c) 1991 Sigma

driver by Angelo Salese & Pierpaolo Prazzoli

Uses the same Seibu custom chips of the D-Con HW.

TODO:
- Find what the remaining video C.R.T. registers does;
- Fix sprites bugs at a start of a play;
- Check NVRAM boudaries;
- How the "SW Service Mode" (press F2 during gameplay) really works (inputs etc)? Nothing mapped works with it...

Notes:
- Some strings written in the sound rom:
  "SENGOKU-MAHJONG Z80 PROGRAM ROM VERSION 1.00 WRITTEN BY K.SAEKI" at location 0x00c0-0x00ff.
  "Copyright 1990/1991 Sigma" at location 0x770-0x789.
- To bypass the startup message, toggle "Reset" dip-switch or reset with F3.
- If the Work RAM is not hooked-up (areas $67xx),a sound sample is played.I can't understand what it says though,
  appears to japanese words for "RAM failed".
- Playing with the debugger I've found this -> http://img444.imageshack.us/img444/2980/0000ti3.png (notice the "credit" at
  the bottom). Maybe a non-BET version exists? Or there's a jumper setting?

CPU:    uPD70116C-8 (V30)
Sound:  Z80-A
        YM3812
        M6295
OSC:    14.31818MHz
        16.000MHz
Chips:  SEI0100
        SEI0160
        SEI0200
        SEI0210
        SEI0220


MAH1-1-1.915  samples

MAH1-2-1.013  sound prg. (ic1013:27c512)

MM01-1-1.21   main prg.
MM01-2-1.24

RS006.89      video timing?

RSSENGO0.64   chr.
RSSENGO1.68

RSSENGO2.72   chr.

*******************************************************************************************/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "audio/seibu.h"
#include "sound/3812intf.h"
#include "includes/sei_crtc.h"
#include "machine/nvram.h"


class sengokmj_state : public driver_device
{
public:
	sengokmj_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 m_sengokumj_mux_data;
	UINT8 m_hopper_io;
	DECLARE_READ16_MEMBER(mahjong_panel_r);
	DECLARE_WRITE16_MEMBER(mahjong_panel_w);
	DECLARE_WRITE16_MEMBER(sengokmj_out_w);
	DECLARE_READ16_MEMBER(sengokmj_system_r);
};



/* Multiplexer device for the mahjong panel */
READ16_MEMBER(sengokmj_state::mahjong_panel_r)
{
	switch(m_sengokumj_mux_data)
	{
		case 0x0100: return input_port_read(machine(), "KEY0");
		case 0x0200: return input_port_read(machine(), "KEY1");
		case 0x0400: return input_port_read(machine(), "KEY2");
		case 0x0800: return input_port_read(machine(), "KEY3");
		case 0x1000: return input_port_read(machine(), "KEY4");
		case 0x2000: return input_port_read(machine(), "UNUSED");
	}

	return 0xffff;
}

WRITE16_MEMBER(sengokmj_state::mahjong_panel_w)
{
	m_sengokumj_mux_data = data;
}

WRITE16_MEMBER(sengokmj_state::sengokmj_out_w)
{
	/* ---- ---- ---x ---- J.P. Signal (?)*/
	/* ---- ---- ---- -x-- Coin counter (done AFTER that you press start)*/
	/* ---- ---- ---- --x- Cash enable (lockout)*/
	/* ---- ---- ---- ---x Hopper 10 */
	coin_lockout_w(machine(), 0,~data & 2);
	coin_lockout_w(machine(), 1,~data & 2);
	coin_counter_w(machine(), 0,data & 4);
	m_hopper_io = ((data & 1)<<6);
//  popmessage("%02x",m_hopper_io);
}

READ16_MEMBER(sengokmj_state::sengokmj_system_r)
{
	return (input_port_read(machine(), "SYSTEM") & 0xffbf) | m_hopper_io;
}

static ADDRESS_MAP_START( sengokmj_map, AS_PROGRAM, 16, sengokmj_state )
	AM_RANGE(0x00000, 0x07fff) AM_RAM
	AM_RANGE(0x08000, 0x09fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x0c000, 0x0c7ff) AM_RAM_WRITE_LEGACY(seibucrtc_sc0vram_w) AM_BASE_LEGACY(&seibucrtc_sc0vram)
	AM_RANGE(0x0c800, 0x0cfff) AM_RAM_WRITE_LEGACY(seibucrtc_sc1vram_w) AM_BASE_LEGACY(&seibucrtc_sc1vram)
	AM_RANGE(0x0d000, 0x0d7ff) AM_RAM_WRITE_LEGACY(seibucrtc_sc2vram_w) AM_BASE_LEGACY(&seibucrtc_sc2vram)
	AM_RANGE(0x0d800, 0x0e7ff) AM_RAM_WRITE_LEGACY(seibucrtc_sc3vram_w) AM_BASE_LEGACY(&seibucrtc_sc3vram)
	AM_RANGE(0x0e800, 0x0f7ff) AM_RAM_WRITE_LEGACY(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x0f800, 0x0ffff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xc0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sengokmj_io_map, AS_IO, 16, sengokmj_state )
	AM_RANGE(0x4000, 0x400f) AM_READWRITE_LEGACY(seibu_main_word_r, seibu_main_word_w)
	/*Areas from 8000-804f are for the custom Seibu CRTC.*/
	AM_RANGE(0x8000, 0x804f) AM_RAM_WRITE_LEGACY(seibucrtc_vregs_w) AM_BASE_LEGACY(&seibucrtc_vregs)

//  AM_RANGE(0x8080, 0x8081) CRTC extra register?
//  AM_RANGE(0x80c0, 0x80c1) CRTC extra register?
//  AM_RANGE(0x8100, 0x8101) AM_WRITENOP // always 0
	AM_RANGE(0x8180, 0x8181) AM_WRITE(sengokmj_out_w)
	AM_RANGE(0x8140, 0x8141) AM_WRITE(mahjong_panel_w)
	AM_RANGE(0xc000, 0xc001) AM_READ_PORT("DSW1")
	AM_RANGE(0xc002, 0xc003) AM_READ(mahjong_panel_r)
	AM_RANGE(0xc004, 0xc005) AM_READ(sengokmj_system_r) //switches
ADDRESS_MAP_END


static INPUT_PORTS_START( sengokmj )
	SEIBU_COIN_INPUTS	/* coin inputs read through sound cpu */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(	  0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Re-start" )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(	  0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Double G" )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(	  0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Double L" )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(	  0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Kamon" )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(	  0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW1:6" )
	PORT_DIPNAME( 0x0040, 0x0040, "Out Sw" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(	  0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "Hopper" ) PORT_DIPLOCATION("SW1:8") //game gives hopper error with this off.
	PORT_DIPSETTING(	  0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNUSED")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_DIPNAME( 0x0001, 0x0001, "Door" )
	PORT_DIPSETTING(	  0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0002, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0004, 0x0004, "Opt. 1st" )
	PORT_DIPSETTING(	  0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Reset" )
	PORT_DIPSETTING(	  0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Cash" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
//  0x40 Hopper
	PORT_DIPNAME( 0x0080, 0x0080, "Meter" )
	PORT_DIPSETTING(	  0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static const gfx_layout tilelayout =
{
	16,16,	/* 16*16 sprites  */
	RGN_FRAC(1,1),
	4,	/* 4 bits per pixel */
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 16+3, 16+2, 16+1, 16+0,
             3+32*16, 2+32*16, 1+32*16, 0+32*16, 16+3+32*16, 16+2+32*16, 16+1+32*16, 16+0+32*16 },
	{ 0*16, 2*16, 4*16, 6*16, 8*16, 10*16, 12*16, 14*16,
			16*16, 18*16, 20*16, 22*16, 24*16, 26*16, 28*16, 30*16 },
	128*8	/* every sprite takes 128 consecutive bytes */
};

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,	/* 4 bits per pixel */
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 16+3, 16+2, 16+1, 16+0 },
	{ 0*16, 2*16, 4*16, 6*16, 8*16, 10*16, 12*16, 14*16 },
	128*8	/* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( sengokmj )
	GFXDECODE_ENTRY( "spr_gfx",0, tilelayout, 0x000, 0x40 ) /* Sprites */
	GFXDECODE_ENTRY( "bg_gfx", 0, tilelayout, 0x400, 0x10 ) /* Tiles */
	GFXDECODE_ENTRY( "md_gfx", 0, tilelayout, 0x500, 0x10 ) /* Tiles */
	GFXDECODE_ENTRY( "fg_gfx", 0, tilelayout, 0x600, 0x10 ) /* Tiles */
	GFXDECODE_ENTRY( "tx_gfx", 0, charlayout, 0x700, 0x10 ) /* Text */
GFXDECODE_END

static INTERRUPT_GEN( sengokmj_interrupt )
{
	device_set_input_line_and_vector(device,0,HOLD_LINE,0xc8/4);
}

static MACHINE_CONFIG_START( sengokmj, sengokmj_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30, 16000000/2) /* V30-8 */
	MCFG_CPU_PROGRAM_MAP(sengokmj_map)
	MCFG_CPU_IO_MAP(sengokmj_io_map)
	MCFG_CPU_VBLANK_INT("screen", sengokmj_interrupt)

	SEIBU_SOUND_SYSTEM_CPU(14318180/4)

	MCFG_MACHINE_RESET(seibu_sound)
	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 16, 256-1) //TODO: dynamic resolution
	MCFG_SCREEN_UPDATE_STATIC(seibu_crtc)

	MCFG_GFXDECODE(sengokmj)
	MCFG_PALETTE_LENGTH(0x800)

	MCFG_VIDEO_START(seibu_crtc)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM3812_INTERFACE(14318180/4,1320000)
MACHINE_CONFIG_END


ROM_START( sengokmj )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* V30 code */
	ROM_LOAD16_BYTE( "mm01-1-1.21",  0xc0000, 0x20000, CRC(74076b46) SHA1(64b0ed5a8c32e21157ae12fe40519e4c605b329c) )
	ROM_LOAD16_BYTE( "mm01-2-1.24",  0xc0001, 0x20000, CRC(f1a7c131) SHA1(d0fbbdedbff8f05da0e0296baa41369bc41a67e4) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "mah1-2-1.013", 0x000000, 0x08000, CRC(6a4f31b8) SHA1(5e1d7ed299c1fd65c7a43faa02831220f4251733) )
	ROM_CONTINUE(             0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0,     0x018000, 0x08000 )

	ROM_REGION( 0x100000, "spr_gfx", 0 ) /*Sprites gfx rom*/
	ROM_LOAD( "rssengo2.72", 0x00000, 0x100000, CRC(fb215ff8) SHA1(f98c0a53ad9b97d209dd1f85c994fc17ec585bd7) )

	ROM_REGION( 0x200000, "gfx_tiles", 0 ) /*Tiles data,to be reloaded*/
	ROM_LOAD( "rssengo0.64", 0x000000, 0x100000, CRC(36924b71) SHA1(814b2c69ab9876ccc57774e5718c05059ea23150) )
	ROM_LOAD( "rssengo1.68", 0x100000, 0x100000, CRC(1bbd00e5) SHA1(86391323b8e0d3b7e09a5914d87fb2adc48e5af4) )

	ROM_REGION( 0x080000, "bg_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x000000, 0x00000, 0x080000)

	ROM_REGION( 0x080000, "md_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x080000, 0x00000, 0x080000)

	ROM_REGION( 0x080000, "fg_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x100000, 0x00000, 0x080000)

	ROM_REGION( 0x080000, "tx_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x180000, 0x00000, 0x080000)

	ROM_REGION( 0x40000, "oki", 0 )	 /* ADPCM samples */
	ROM_LOAD( "mah1-1-1.915", 0x00000, 0x20000, CRC(d4612e95) SHA1(937c5dbd25c89d4f4178b0bed510307020c5f40e) )

	ROM_REGION( 0x200, "user1", 0 ) /* not used */
	ROM_LOAD( "rs006.89", 0x000, 0x200, CRC(96f7646e) SHA1(400a831b83d6ac4d2a46ef95b97b1ee237099e44) ) /* Priority */
ROM_END

GAME( 1991, sengokmj, 0, sengokmj, sengokmj, 0, ROT0, "Sigma", "Sengoku Mahjong [BET] (Japan)", GAME_IMPERFECT_GRAPHICS )
/*Non-Bet Version?*/
