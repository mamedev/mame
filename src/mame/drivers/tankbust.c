// license:???
// copyright-holders:Jarek Burczynski
/***************************************************************************

Tank Busters memory map

driver by Jarek Burczynski


Note:
    To enter the test mode:
    reset the game and keep start1 and start2 buttons pressed.

To do:
    - verify colors: prom to output mapping is unknown, resistor values are guess
    - remove the 'some_changing_input' hack (see below)
    - from time to time the game just hangs

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/tankbust.h"


void tankbust_state::machine_start()
{
	membank("bank1")->configure_entries(0, 2, memregion("maincpu")->base() + 0x10000, 0x4000);
	membank("bank2")->configure_entries(0, 2, memregion("maincpu")->base() + 0x18000, 0x2000);

	save_item(NAME(m_latch));
	save_item(NAME(m_timer1));
	save_item(NAME(m_e0xx_data));
	save_item(NAME(m_variable_data));
	save_item(NAME(m_irq_mask));
}

//port A of ay8910#0

TIMER_CALLBACK_MEMBER(tankbust_state::soundlatch_callback)
{
	m_latch = param;
}

WRITE8_MEMBER(tankbust_state::soundlatch_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(tankbust_state::soundlatch_callback),this), data);
}

READ8_MEMBER(tankbust_state::soundlatch_r)
{
	return m_latch;
}

//port B of ay8910#0
READ8_MEMBER(tankbust_state::soundtimer_r)
{
	int ret;

	m_timer1++;
	ret = m_timer1;
	return ret;
}

TIMER_CALLBACK_MEMBER(tankbust_state::soundirqline_callback)
{
//logerror("sound_irq_line write = %2x (after CPUs synced) \n",param);

		if ((param & 1) == 0)
			m_subcpu->set_input_line(0, HOLD_LINE);
}



WRITE8_MEMBER(tankbust_state::e0xx_w)
{
	m_e0xx_data[offset] = data;

#if 0
	popmessage("e0: %x %x (%x cnt) %x %x %x %x",
		m_e0xx_data[0], m_e0xx_data[1],
		m_e0xx_data[2], m_e0xx_data[3],
		m_e0xx_data[4], m_e0xx_data[5],
		m_e0xx_data[6] );
#endif

	switch (offset)
	{
	case 0: /* 0xe000 interrupt enable */
		m_irq_mask = data & 1;
		break;

	case 1: /* 0xe001 (value 0 then 1) written right after the soundlatch_byte_w */
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(tankbust_state::soundirqline_callback),this), data);
		break;

	case 2: /* 0xe002 coin counter */
		coin_counter_w(machine(), 0, data&1);
		break;

	case 6: /* 0xe006 screen disable ?? or disable screen update */
		/* program sets this to 0,
		   clears screen memory,
		   and sets this to 1 */

		/* ???? */
		break;

	case 7: /* 0xe007 bankswitch */
		/* bank 1 at 0x6000-9fff = from 0x10000 when bit0=0 else from 0x14000 */
		membank("bank1")->set_entry(data & 1);

		/* bank 2 at 0xa000-bfff = from 0x18000 when bit0=0 else from 0x1a000 */
		membank("bank2")->set_entry(data & 1); /* verified (the game will reset after the "game over" otherwise) */
		break;
	}
}

READ8_MEMBER(tankbust_state::debug_output_area_r)
{
	return m_e0xx_data[offset];
}




PALETTE_INIT_MEMBER(tankbust_state, tankbust)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < 128; i++)
	{
		int bit0,bit1,bit2,r,g,b;

//7 6   5 4 3   2 1 0
//bb    r r r   g g g - bad (for sure - no green for tank)
//bb    g g g   r r r - bad (for sure - no yellow, no red)
//gg    r r r   b b b - bad
//gg    b b b   r r r - bad
//rr    b b b   g g g - bad

//rr    g g g   b b b - very close (green,yellow,red present)

//rr    r g g   g b b - bad
//rr    r g g   b b b - bad
//rr    g g g   b b r - bad

//rr    g g b   b x x - bad (x: unused)
//rr    g g x   x b b - bad but still close
//rr    g g r   g b b - bad but still close
//rr    g g g   r b b - bad but still close


		/* blue component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* red component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		r = 0x55 * bit0 + 0xaa * bit1;

		palette.set_pen_color(i,rgb_t(r,g,b));
	}
}

#if 0
READ8_MEMBER(tankbust_state::read_from_unmapped_memory)
{
	return 0xff;
}
#endif

READ8_MEMBER(tankbust_state::some_changing_input)
{
	m_variable_data += 8;
	return m_variable_data;
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, tankbust_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x9fff) AM_ROMBANK("bank1")
	AM_RANGE(0xa000, 0xbfff) AM_ROMBANK("bank2")
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(background_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(background_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(txtram_w) AM_SHARE("txtram")
	AM_RANGE(0xd800, 0xd8ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xe000, 0xe007) AM_READWRITE(debug_output_area_r, e0xx_w)
	AM_RANGE(0xe800, 0xe800) AM_READ_PORT("INPUTS") AM_WRITE(yscroll_w)
	AM_RANGE(0xe801, 0xe801) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xe802, 0xe802) AM_READ_PORT("DSW")
	AM_RANGE(0xe801, 0xe802) AM_WRITE(xscroll_w)
	AM_RANGE(0xe803, 0xe803) AM_READWRITE(some_changing_input, soundlatch_w)   /*unknown. Game expects this to change so this is not player input */
	AM_RANGE(0xe804, 0xe804) AM_WRITENOP    /* watchdog ? ; written in long-lasting loops */
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	//AM_RANGE(0xf800, 0xffff) AM_READ(read_from_unmapped_memory)   /* a bug in game code ? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( port_map_cpu2, AS_IO, 8, tankbust_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_DEVWRITE("ay2", ay8910_device, data_w)
	AM_RANGE(0x30, 0x30) AM_DEVREADWRITE("ay2", ay8910_device, data_r, address_w)
	AM_RANGE(0x40, 0x40) AM_DEVWRITE("ay1", ay8910_device, data_w)
	AM_RANGE(0xc0, 0xc0) AM_DEVREADWRITE("ay1", ay8910_device, data_r, address_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( map_cpu2, AS_PROGRAM, 8, tankbust_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_WRITENOP    /* garbage, written in initialization loop */
	//0x4000 and 0x4040-0x4045 seem to be used (referenced in the code)
	AM_RANGE(0x4000, 0x7fff) AM_WRITENOP    /* garbage, written in initialization loop */
	AM_RANGE(0x8000, 0x87ff) AM_READONLY
	AM_RANGE(0x8000, 0x87ff) AM_WRITEONLY
ADDRESS_MAP_END


static INPUT_PORTS_START( tankbust )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x08, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, DEF_STR( French ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x10, "No Bonus" )
	PORT_DIPSETTING(    0x00, "60000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x20, "1C/1C 1C/2C 1C/6C 1C/14C" )
	PORT_DIPSETTING(    0x00, "2C/1C 1C/1C 1C/3C 1C/7C" )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "4" )
INPUT_PORTS_END

static const gfx_layout spritelayout =
{
	32,32,  /* 32*32 pixels */
	64,     /* 64 sprites */
	4,      /* 4 bits per pixel */
	{ 0, 8192*8*1, 8192*8*2, 8192*8*3 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7,
		32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
		40*8+0, 40*8+1, 40*8+2, 40*8+3, 40*8+4, 40*8+5, 40*8+6, 40*8+7 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8,
		23*8, 22*8, 21*8, 20*8, 19*8, 18*8, 17*8, 16*8,
		71*8, 70*8, 69*8, 68*8, 67*8, 66*8, 65*8, 64*8,
		87*8, 86*8, 85*8, 84*8, 83*8, 82*8, 81*8, 80*8 },
	128*8   /* every sprite takes 128 consecutive bytes */
};

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 pixels */
	2048,   /* 2048 characters */
	3,      /* 3 bits per pixel */
	{ 0, 16384*8*1, 16384*8*2 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8     /* every char takes 8 consecutive bytes */
};

static const gfx_layout charlayout2 =
{
	8,8,    /* 8*8 pixels */
	256,    /* 256 characters */
	1,      /* 1 bit per pixel - the data repeats 4 times within one ROM */
	{ 0 }, /* , 2048*8*1, 2048*8*2, 2048*8*3 },*/
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( tankbust )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,   0x00, 2 )   /* sprites 32x32  (2 * 16 colors) */
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,     0x20, 8 )   /* bg tilemap characters */
	GFXDECODE_ENTRY( "gfx3", 0, charlayout2,        0x60, 16  ) /* txt tilemap characters*/
GFXDECODE_END

void tankbust_state::machine_reset()
{
	m_variable_data = 0x11;
}

INTERRUPT_GEN_MEMBER(tankbust_state::vblank_irq)
{
	if(m_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}

static MACHINE_CONFIG_START( tankbust, tankbust_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_14_31818MHz/2)    /* Verified on PCB */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", tankbust_state,  vblank_irq)

	MCFG_CPU_ADD("sub", Z80, XTAL_14_31818MHz/4)        /* Verified on PCB */
//  MCFG_CPU_ADD("sub", Z80, XTAL_14_31818MHz/3)        /* Accurate to audio recording, but apparently incorrect clock */
	MCFG_CPU_PROGRAM_MAP(map_cpu2)
	MCFG_CPU_IO_MAP(port_map_cpu2)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE   ( 64*8, 32*8 )
	MCFG_SCREEN_VISIBLE_AREA  ( 16*8, 56*8-1, 1*8, 31*8-1 )
//  MCFG_SCREEN_VISIBLE_AREA  (  0*8, 64*8-1, 1*8, 31*8-1 )
	MCFG_SCREEN_UPDATE_DRIVER(tankbust_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", tankbust )

	MCFG_PALETTE_ADD( "palette", 128 )
	MCFG_PALETTE_INIT_OWNER(tankbust_state, tankbust)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_14_31818MHz/16)  /* Verified on PCB */
	MCFG_AY8910_PORT_A_READ_CB(READ8(tankbust_state, soundlatch_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(tankbust_state, soundtimer_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_14_31818MHz/16)  /* Verified on PCB */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tankbust )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "a-s4-6.bin",     0x00000, 0x4000, CRC(8ebe7317) SHA1(bc45d530ad6335312c9c3efdcedf7acd2cdeeb55) )
	ROM_LOAD( "a-s7-9.bin",     0x04000, 0x2000, CRC(047aee33) SHA1(62ee776c403b228e065baa9218f32597951ca935) )

	ROM_LOAD( "a-s5_7.bin",     0x12000, 0x2000, CRC(dd4800ca) SHA1(73a6caa029c27fb45217f9372d9541c6fe206f08) ) /* banked at 0x6000-0x9fff */
	ROM_CONTINUE(                   0x10000, 0x2000)

	ROM_LOAD( "a-s6-8.bin",     0x16000, 0x2000, CRC(f8801238) SHA1(fd3abe18542660a8c31dc316012a99d48c9bb5aa) ) /* banked at 0x6000-0x9fff */
	ROM_CONTINUE(                   0x14000, 0x2000)

//  ROM_LOAD( "a-s5_7.bin",     0x10000, 0x4000, CRC(dd4800ca) SHA1(73a6caa029c27fb45217f9372d9541c6fe206f08) ) /* banked at 0x6000-0x9fff */
//  ROM_LOAD( "a-s6-8.bin",     0x14000, 0x4000, CRC(f8801238) SHA1(fd3abe18542660a8c31dc316012a99d48c9bb5aa) ) /* banked at 0x6000-0x9fff */

	ROM_LOAD( "a-s8-10.bin",    0x18000, 0x4000, CRC(9e826faa) SHA1(6a252428c69133d3e9d7a9938140d5ae37fb0c7d) ) /* banked at 0xa000-0xbfff */

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "a-b3-1.bin",     0x0000, 0x2000, CRC(b0f56102) SHA1(4f427c3bd6131b7cba42a0e24a69bd1b6a1b0a3c) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "a-d5-2.bin",     0x0000, 0x2000, CRC(0bbf3fdb) SHA1(035c2db6eca701be690042e006c0d07c90d752f1) )  /* sprites 32x32 */
	ROM_LOAD( "a-d6-3.bin",     0x2000, 0x2000, CRC(4398dc21) SHA1(3b23433d0c9daa554ad6615af2fdec715e4e3794) )
	ROM_LOAD( "a-d7-4.bin",     0x4000, 0x2000, CRC(aca197fc) SHA1(03ecd94b84a31389539074079ed7f2a500e588ab) )
	ROM_LOAD( "a-d8-5.bin",     0x6000, 0x2000, CRC(1e6edc17) SHA1(4dbc91938c999348bcbd5f960fc3bb49f3174059) )

	ROM_REGION( 0xc000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "b-m4-11.bin",    0x0000, 0x4000, CRC(eb88ee1f) SHA1(60ec2d77186c196a27278b0639cbfa838986e2e2) )  /* background tilemap characters 8x8 */
	ROM_LOAD( "b-m5-12.bin",    0x4000, 0x4000, CRC(4c65f399) SHA1(72db15884f346c001d3b86cb33e3f6d339eedb56) )
	ROM_LOAD( "b-m6-13.bin",    0x8000, 0x4000, CRC(a5baa413) SHA1(dc772042706c3a92594ee8422aafed77375c0632) )

	ROM_REGION( 0x2000, "gfx3", 0 )
	ROM_LOAD( "b-r3-14.bin",    0x0000, 0x2000, CRC(4310a815) SHA1(bf58a7a8d3f82fcaa0c46d9ebb13cac1231b80ad) )  /* text tilemap characters 8x8 */

	ROM_REGION( 0x0080, "proms", 0 )
	ROM_LOAD( "tb-prom.1s8",    0x0000, 0x0020, CRC(dfaa086c) SHA1(f534aedddd18addd0833a3a28a4297689c4a46ac) ) //sprites
	ROM_LOAD( "tb-prom.2r8",    0x0020, 0x0020, CRC(ec50d674) SHA1(64c8961eca33b23e14b7383eb7e64fcac8772ee7) ) //background
	ROM_LOAD( "tb-prom.3p8",    0x0040, 0x0020, CRC(3e70eafd) SHA1(b200350a3f6c166228706734419dd3ef1207eeef) ) //background palette 2 ??
	ROM_LOAD( "tb-prom.4k8",    0x0060, 0x0020, CRC(624f40d2) SHA1(8421f1d774afc72e0817d41edae74a2837021a5f) ) //text
ROM_END


GAME( 1985, tankbust,    0,       tankbust, tankbust, driver_device,  0, ROT90, "Valadon Automation", "Tank Busters", MACHINE_SUPPORTS_SAVE )
