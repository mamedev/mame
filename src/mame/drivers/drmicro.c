// license:BSD-3-Clause
// copyright-holders:Uki
/*****************************************************************************

Dr. Micro (c) 1983 Sanritsu

        driver by Uki

Quite similar to Appoooh

*****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/msm5205.h"
#include "sound/sn76496.h"
#include "includes/drmicro.h"

#define MCLK 18432000


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

INTERRUPT_GEN_MEMBER(drmicro_state::drmicro_interrupt)
{
	if (m_nmi_enable)
			device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_MEMBER(drmicro_state::nmi_enable_w)
{
	m_nmi_enable = data & 1;
	m_flipscreen = (data & 2) ? 1 : 0;
	flip_screen_set(data & 2);

	// bit2,3 unknown
}


WRITE_LINE_MEMBER(drmicro_state::pcm_w)
{
	UINT8 *PCM = memregion("adpcm")->base();

	int data = PCM[m_pcm_adr / 2];

	if (data != 0x70) // ??
	{
		if (~m_pcm_adr & 1)
			data >>= 4;

		m_msm->data_w(data & 0x0f);
		m_msm->reset_w(0);

		m_pcm_adr = (m_pcm_adr + 1) & 0x7fff;
	}
	else
		m_msm->reset_w(1);
}

WRITE8_MEMBER(drmicro_state::pcm_set_w)
{
	m_pcm_adr = ((data & 0x3f) << 9);
	pcm_w(1);
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( drmicro_map, AS_PROGRAM, 8, drmicro_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xefff) AM_RAM_WRITE(drmicro_videoram_w)
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, drmicro_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1") AM_DEVWRITE("sn1", sn76496_device, write)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("P2") AM_DEVWRITE("sn2", sn76496_device, write)
	AM_RANGE(0x02, 0x02) AM_DEVWRITE("sn3", sn76496_device, write)
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW1") AM_WRITE(pcm_set_w)
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSW2") AM_WRITE(nmi_enable_w)
	AM_RANGE(0x05, 0x05) AM_NOP // unused? / watchdog?
ADDRESS_MAP_END

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( drmicro )
	PORT_START("P1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_START("P2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!4,!5")
	PORT_DIPSETTING(    0x00, "30000 100000" )
	PORT_DIPSETTING(    0x08, "50000 150000" )
	PORT_DIPSETTING(    0x10, "70000 200000" )
	PORT_DIPSETTING(    0x18, "100000 300000" )
	PORT_SERVICE_DIPLOC(  0x20, IP_ACTIVE_HIGH, "SW1:!6" )  /* Service Mode shows as "X" */
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:!1,!2,!3")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_HIGH, "SW2:!4" ) /* Service Mode shows as "X" */
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_HIGH, "SW2:!5" ) /* Service Mode shows as "X" */
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_HIGH, "SW2:!6" ) /* Service Mode shows as "X" */
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_HIGH, "SW2:!7" ) /* Service Mode shows as "X" */
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_HIGH, "SW2:!8" ) /* Service Mode shows as "X" */
INPUT_PORTS_END

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout spritelayout4 =
{
	16,16,
	0x100,
	2,
	{0,0x2000*8},
	{STEP8(7,-1),STEP8(71,-1)},
	{STEP8(0,8),STEP8(128,8)},
	8*8*4
};

static const gfx_layout spritelayout8 =
{
	16,16,
	0x100,
	3,
	{0x2000*16,0x2000*8,0},
	{STEP8(7,-1),STEP8(71,-1)},
	{STEP8(0,8),STEP8(128,8)},
	8*8*4
};

static const gfx_layout charlayout4 =
{
	8,8,
	0x400,
	2,
	{0,0x2000*8},
	{STEP8(7,-1)},
	{STEP8(0,8)},
	8*8*1
};

static const gfx_layout charlayout8 =
{
	8,8,
	0x400,
	3,
	{0x2000*16,0x2000*8,0},
	{STEP8(7,-1)},
	{STEP8(0,8)},
	8*8*1
};

static GFXDECODE_START( drmicro )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout4,     0, 64 ) /* tiles */
	GFXDECODE_ENTRY( "gfx2", 0x0000, charlayout8,   256, 32 ) /* tiles */
	GFXDECODE_ENTRY( "gfx1", 0x0000, spritelayout4,   0, 64 ) /* sprites */
	GFXDECODE_ENTRY( "gfx2", 0x0000, spritelayout8, 256, 32 ) /* sprites */
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void drmicro_state::machine_start()
{
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_pcm_adr));
	save_item(NAME(m_flipscreen));
}

void drmicro_state::machine_reset()
{
	m_nmi_enable = 0;
	m_pcm_adr = 0;
	m_flipscreen = 0;
}


static MACHINE_CONFIG_START( drmicro, drmicro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,MCLK/6) /* 3.072MHz? */
	MCFG_CPU_PROGRAM_MAP(drmicro_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", drmicro_state,  drmicro_interrupt)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(drmicro_state, screen_update_drmicro)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT270)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", drmicro)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(drmicro_state, drmicro)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn1", SN76496, MCLK/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("sn2", SN76496, MCLK/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("sn3", SN76496, MCLK/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("msm", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(drmicro_state, pcm_w))          /* IRQ handler */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S64_4B)  /* 6 KHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( drmicro )
	ROM_REGION( 0x10000, "maincpu", 0 ) // CPU
	ROM_LOAD( "dm-00.13b", 0x0000,  0x2000, CRC(270f2145) SHA1(1557428387e2c0f711c676a13a763c8d48aa497b) )
	ROM_LOAD( "dm-01.14b", 0x2000,  0x2000, CRC(bba30c80) SHA1(a084429fad58fa6348936084652235d5f55e3b89) )
	ROM_LOAD( "dm-02.15b", 0x4000,  0x2000, CRC(d9e4ca6b) SHA1(9fb6d1d6b45628891deae389cf1d142332b110ba) )
	ROM_LOAD( "dm-03.13d", 0x6000,  0x2000, CRC(b7bcb45b) SHA1(61035afc642bac2e1c56c36c188bed4e1949523f) )
	ROM_LOAD( "dm-04.14d", 0x8000,  0x2000, CRC(071db054) SHA1(75929b7692bebf2246fa84581b6d1eedb02c9aba) )
	ROM_LOAD( "dm-05.15d", 0xa000,  0x2000, CRC(f41b8d8a) SHA1(802830f3f0362ec3df257f31dc22390e8ae4207c) )

	ROM_REGION( 0x04000, "gfx1", 0 ) // gfx 1
	ROM_LOAD( "dm-23.5l",  0x0000,  0x2000, CRC(279a76b8) SHA1(635650621bdce5873bb5faf64f8352149314e784) )
	ROM_LOAD( "dm-24.5n",  0x2000,  0x2000, CRC(ee8ed1ec) SHA1(7afc05c73186af9fe3d3f3ce13412c8ee560b146) )

	ROM_REGION( 0x06000, "gfx2", 0 ) // gfx 2
	ROM_LOAD( "dm-20.4a",  0x0000,  0x2000, CRC(6f5dbf22) SHA1(41ef084336e2ebb1016b28505dcb43483e37a0de) )
	ROM_LOAD( "dm-21.4c",  0x2000,  0x2000, CRC(8b17ff47) SHA1(5bcc14489ea1d4f1fe8e51c24a72a8e787ab8159) )
	ROM_LOAD( "dm-22.4d",  0x4000,  0x2000, CRC(84daf771) SHA1(d187debcca59ceab6cd696be246370120ee575c6) )

	ROM_REGION( 0x04000, "adpcm", 0 ) // samples
	ROM_LOAD( "dm-40.12m",  0x0000,  0x2000, CRC(3d080af9) SHA1(f9527fae69fe3ca0762024ac4a44b1f02fbee66a) )
	ROM_LOAD( "dm-41.13m",  0x2000,  0x2000, CRC(ddd7bda2) SHA1(bbe9276cb47fa3e82081d592522640e04b4a9223) )

	ROM_REGION( 0x00220, "proms", 0 ) // PROMs
	ROM_LOAD( "dm-62.9h", 0x0000,  0x0020, CRC(e3e36eaf) SHA1(5954400190e587a20cad60f5829f4bddc85ea526) )
	ROM_LOAD( "dm-61.4m", 0x0020,  0x0100, CRC(0dd8e365) SHA1(cbd43a2d4af053860932af32ca5e13bef728e38a) )
	ROM_LOAD( "dm-60.6e", 0x0120,  0x0100, CRC(540a3953) SHA1(bc65388a1019dadf8c71705e234763f5c735e282) )
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1983, drmicro, 0, drmicro, drmicro, driver_device, 0, ROT270, "Sanritsu", "Dr. Micro", MACHINE_SUPPORTS_SAVE )
