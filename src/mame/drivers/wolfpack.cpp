// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

    Atari Wolf Pack (prototype) driver

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "includes/wolfpack.h"


void wolfpack_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_PERIODIC:
		periodic_callback(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in wolfpack_state::device_timer");
	}
}


TIMER_CALLBACK_MEMBER(wolfpack_state::periodic_callback)
{
	int scanline = param;

	m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);

	scanline += 64;

	if (scanline >= 262)
		scanline = 0;

	timer_set(m_screen->time_until_pos(scanline), TIMER_PERIODIC, scanline);
}


void wolfpack_state::machine_reset()
{
	timer_set(m_screen->time_until_pos(0), TIMER_PERIODIC);
}


CUSTOM_INPUT_MEMBER(wolfpack_state::wolfpack_dial_r)
{
	int bit = (FPTR)param;
	return ((ioport("DIAL")->read() + bit) / 2) & 0x01;
}


READ8_MEMBER(wolfpack_state::wolfpack_misc_r)
{
	UINT8 val = 0;

	/* BIT0 => SPEECH BUSY */
	/* BIT1 => COMP SIREN  */
	/* BIT2 => SPARE       */
	/* BIT3 => SPARE       */
	/* BIT4 => COL DETECT  */
	/* BIT5 => UNUSED      */
	/* BIT6 => UNUSED      */
	/* BIT7 => VBLANK      */

	if (!m_s14001a->busy_r())
		val |= 0x01;

	if (!m_collision)
		val |= 0x10;

	if (m_screen->vpos() >= 240)
		val |= 0x80;

	return val;
}


WRITE8_MEMBER(wolfpack_state::wolfpack_high_explo_w){ }
WRITE8_MEMBER(wolfpack_state::wolfpack_sonar_ping_w){}
WRITE8_MEMBER(wolfpack_state::wolfpack_sirlat_w){}
WRITE8_MEMBER(wolfpack_state::wolfpack_pt_sound_w){}
WRITE8_MEMBER(wolfpack_state::wolfpack_launch_torpedo_w){}
WRITE8_MEMBER(wolfpack_state::wolfpack_low_explo_w){}
WRITE8_MEMBER(wolfpack_state::wolfpack_screw_cont_w){}
WRITE8_MEMBER(wolfpack_state::wolfpack_lamp_flash_w){}
WRITE8_MEMBER(wolfpack_state::wolfpack_warning_light_w){}
WRITE8_MEMBER(wolfpack_state::wolfpack_audamp_w){}

WRITE8_MEMBER(wolfpack_state::wolfpack_word_w)
{
	/* latch word from bus into temp register, and place on s14001a input bus */
	/* there is no real need for a temp register at all, since the bus 'register' acts as one */
	m_s14001a->data_w(space, 0, data & 0x1f); /* SA0 (IN5) is pulled low according to the schematic, so its 0x1f and not 0x3f as one would expect */
}

WRITE8_MEMBER(wolfpack_state::wolfpack_start_speech_w)
{
	m_s14001a->start_w(data&1);
}


WRITE8_MEMBER(wolfpack_state::wolfpack_attract_w)
{
	machine().bookkeeping().coin_lockout_global_w(!(data & 1));
}


WRITE8_MEMBER(wolfpack_state::wolfpack_credit_w)
{
	output().set_led_value(0, !(data & 1));
}


WRITE8_MEMBER(wolfpack_state::wolfpack_coldetres_w)
{
	m_collision = 0;
}


static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, wolfpack_state )
	AM_RANGE(0x0000, 0x00ff) AM_RAM AM_MIRROR(0x100)
	AM_RANGE(0x1000, 0x1000) AM_READ_PORT("INPUTS")
	AM_RANGE(0x1000, 0x10ff) AM_WRITEONLY AM_SHARE("alpha_num_ram")
	AM_RANGE(0x2000, 0x2000) AM_READ(wolfpack_misc_r)
	AM_RANGE(0x2000, 0x2000) AM_WRITE(wolfpack_high_explo_w)
	AM_RANGE(0x2001, 0x2001) AM_WRITE(wolfpack_sonar_ping_w)
	AM_RANGE(0x2002, 0x2002) AM_WRITE(wolfpack_sirlat_w)
	AM_RANGE(0x2003, 0x2003) AM_WRITE(wolfpack_pt_sound_w)
	AM_RANGE(0x2004, 0x2004) AM_WRITE(wolfpack_start_speech_w)
	AM_RANGE(0x2005, 0x2005) AM_WRITE(wolfpack_launch_torpedo_w)
	AM_RANGE(0x2006, 0x2006) AM_WRITE(wolfpack_low_explo_w)
	AM_RANGE(0x2007, 0x2007) AM_WRITE(wolfpack_screw_cont_w)
	AM_RANGE(0x2008, 0x2008) AM_WRITE(wolfpack_video_invert_w)
	AM_RANGE(0x2009, 0x2009) AM_WRITE(wolfpack_ship_reflect_w)
	AM_RANGE(0x200a, 0x200a) AM_WRITE(wolfpack_lamp_flash_w)
	AM_RANGE(0x200c, 0x200c) AM_WRITE(wolfpack_credit_w)
	AM_RANGE(0x200d, 0x200d) AM_WRITE(wolfpack_attract_w)
	AM_RANGE(0x200e, 0x200e) AM_WRITE(wolfpack_pt_pos_select_w)
	AM_RANGE(0x200f, 0x200f) AM_WRITE(wolfpack_warning_light_w)
	AM_RANGE(0x3000, 0x3000) AM_READ_PORT("DSW")
	AM_RANGE(0x3000, 0x3000) AM_WRITE(wolfpack_audamp_w)
	AM_RANGE(0x3001, 0x3001) AM_WRITE(wolfpack_pt_horz_w)
	AM_RANGE(0x3003, 0x3003) AM_WRITE(wolfpack_pt_pic_w)
	AM_RANGE(0x3004, 0x3004) AM_WRITE(wolfpack_word_w)
	AM_RANGE(0x3007, 0x3007) AM_WRITE(wolfpack_coldetres_w)
	AM_RANGE(0x4000, 0x4000) AM_WRITE(wolfpack_ship_h_w)
	AM_RANGE(0x4001, 0x4001) AM_WRITE(wolfpack_torpedo_pic_w)
	AM_RANGE(0x4002, 0x4002) AM_WRITE(wolfpack_ship_size_w)
	AM_RANGE(0x4003, 0x4003) AM_WRITE(wolfpack_ship_h_precess_w)
	AM_RANGE(0x4004, 0x4004) AM_WRITE(wolfpack_ship_pic_w)
	AM_RANGE(0x4005, 0x4005) AM_WRITE(wolfpack_torpedo_h_w)
	AM_RANGE(0x4006, 0x4006) AM_WRITE(wolfpack_torpedo_v_w)
	AM_RANGE(0x5000, 0x5fff) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x7000, 0x7fff) AM_ROM
	AM_RANGE(0x9000, 0x9000) AM_READNOP /* debugger ROM location? */
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( wolfpack )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, wolfpack_state,wolfpack_dial_r, (void *)nullptr)    /* dial connects here */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, wolfpack_state,wolfpack_dial_r, (void *)1)    /* dial connects here */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_SERVICE( 0x10, IP_ACTIVE_HIGH )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x04, 0x00, "Extended Play" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x18, 0x08, "Play Time" )
	PORT_DIPSETTING(    0x00, "65 seconds" )
	PORT_DIPSETTING(    0x08, "97 seconds" )
	PORT_DIPSETTING(    0x10, "130 seconds" )
	PORT_DIPSETTING(    0x18, "160 seconds" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) ) /* demo sound? */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x80, "Score for Extended Play" )
	PORT_DIPSETTING(    0x00, "8000" )
	PORT_DIPSETTING(    0x40, "12000" )
	PORT_DIPSETTING(    0x80, "16000" )
	PORT_DIPSETTING(    0xc0, "20000" )

	PORT_START("DIAL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(5)
INPUT_PORTS_END


static const gfx_layout tile_layout =
{
	16, 8,
	64,
	1,
	{ 0 },
	{
		0x7, 0x7, 0x6, 0x6, 0x5, 0x5, 0x4, 0x4,
		0xf, 0xf, 0xe, 0xe, 0xd, 0xd, 0xc, 0xc
	},
	{
		0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10, 0x00
	},
	0x80
};

static const UINT32 ship_layout_xoffset[64] =
{
		0x04, 0x05, 0x06, 0x07, 0x0c, 0x0d, 0x0e, 0x0f,
		0x14, 0x15, 0x16, 0x17, 0x1c, 0x1d, 0x1e, 0x1f,
		0x24, 0x25, 0x26, 0x27, 0x2c, 0x2d, 0x2e, 0x2f,
		0x34, 0x35, 0x36, 0x37, 0x3c, 0x3d, 0x3e, 0x3f,
		0x44, 0x45, 0x46, 0x47, 0x4c, 0x4d, 0x4e, 0x4f,
		0x54, 0x55, 0x56, 0x57, 0x5c, 0x5d, 0x5e, 0x5f,
		0x64, 0x65, 0x66, 0x67, 0x6c, 0x6d, 0x6e, 0x6f,
		0x74, 0x75, 0x76, 0x77, 0x7c, 0x7d, 0x7e, 0x7f
};

static const gfx_layout ship_layout =
{
	64, 16,
	16,
	1,
	{ 0 },
	EXTENDED_XOFFS,
	{
			0x780, 0x700, 0x680, 0x600, 0x580, 0x500, 0x480, 0x400,
			0x380, 0x300, 0x280, 0x200, 0x180, 0x100, 0x080, 0x000
	},
	0x800,
	ship_layout_xoffset,
	nullptr
};

static const UINT32 pt_layout_xoffset[64] =
	{
		0x3f, 0x3f, 0x3e, 0x3e, 0x3d, 0x3d, 0x3c, 0x3c,
		0x37, 0x37, 0x36, 0x36, 0x35, 0x35, 0x34, 0x34,
		0x2f, 0x2f, 0x2e, 0x2e, 0x2d, 0x2d, 0x2c, 0x2c,
		0x27, 0x27, 0x26, 0x26, 0x25, 0x25, 0x24, 0x24,
		0x1f, 0x1f, 0x1e, 0x1e, 0x1d, 0x1d, 0x1c, 0x1c,
		0x17, 0x17, 0x16, 0x16, 0x15, 0x15, 0x14, 0x14,
		0x0f, 0x0f, 0x0e, 0x0e, 0x0d, 0x0d, 0x0c, 0x0c,
		0x07, 0x07, 0x06, 0x06, 0x05, 0x05, 0x04, 0x04
	};

static const gfx_layout pt_layout =
{
	64, 8,
	16,
	1,
	{ 0 },
	EXTENDED_XOFFS,
	{ 0x000, 0x040, 0x080, 0x0c0, 0x100, 0x140, 0x180, 0x1c0 },
	0x200,
	pt_layout_xoffset,
	nullptr
};


static const gfx_layout torpedo_layout =
{
	16, 32,
	16,
	1,
	{ 0 },
	{
		0x4, 0x4, 0x5, 0x5, 0x6, 0x6, 0x7, 0x7,
		0xc, 0xc, 0xd, 0xd, 0xe, 0xe, 0xf, 0xf
	},
	{
		0x000, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070,
		0x080, 0x090, 0x0a0, 0x0b0, 0x0c0, 0x0d0, 0x0e0, 0x0f0,
		0x100, 0x110, 0x120, 0x130, 0x140, 0x150, 0x160, 0x170,
		0x180, 0x190, 0x1a0, 0x1b0, 0x1c0, 0x1d0, 0x1e0, 0x1f0
	},
	0x0200
};


static GFXDECODE_START( wolfpack )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout, 0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, ship_layout, 6, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, pt_layout, 0, 1 )
	GFXDECODE_ENTRY( "gfx4", 0, torpedo_layout, 4, 1 )
GFXDECODE_END


static MACHINE_CONFIG_START( wolfpack, wolfpack_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 12096000 / 16)
	MCFG_CPU_PROGRAM_MAP(main_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(512, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 16, 239)
	MCFG_SCREEN_UPDATE_DRIVER(wolfpack_state, screen_update_wolfpack)
	MCFG_SCREEN_VBLANK_DRIVER(wolfpack_state, screen_eof_wolfpack)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", wolfpack)
	MCFG_PALETTE_ADD("palette", 12)
	MCFG_PALETTE_INDIRECT_ENTRIES(8)
	MCFG_PALETTE_INIT_OWNER(wolfpack_state, wolfpack)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speech", S14001A_NEW, 20000) /* RC Clock (C=100pf, R=470K-670K ohms, adjustable) ranging from 14925.37313hz to 21276.59574hz, likely factory set to 20000hz since anything below 19500 is too slow */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


ROM_START( wolfpack )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_LOW ( "30285.e3", 0x7000, 0x0800, CRC(b4d30b33) SHA1(46645c227828632b57244bdccad455e1831b5273) )
	ROM_RELOAD       (             0xF000, 0x0800 )
	ROM_LOAD_NIB_HIGH( "30287.g3", 0x7000, 0x0800, CRC(c6300dc9) SHA1(6a0ec0bfa6ad4c870aa6f21bfde094da6975b58b) )
	ROM_RELOAD       (             0xF000, 0x0800 )
	ROM_LOAD_NIB_LOW ( "30286.f3", 0x7800, 0x0800, CRC(17dce9e8) SHA1(9c7bac1aa676548dc7908f1518efd58c72645ab7) )
	ROM_RELOAD       (             0xF800, 0x0800 )
	ROM_LOAD_NIB_HIGH( "30288.h3", 0x7800, 0x0800, CRC(b80ab7b6) SHA1(f2ede98ac5337064499ae2262a8a81f83505bd66) )
	ROM_RELOAD       (             0xF800, 0x0800 )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "30291.c1", 0x0000, 0x0400, CRC(7e3d22cf) SHA1(92e6bbe049dc8fcd674f2ff96cde3786f714508d) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "30289.j6", 0x0000, 0x0800, CRC(f63e5629) SHA1(d64f19fc62060d395df5bb8663a7694a23b0aa2e) )
	ROM_LOAD( "30290.k6", 0x0800, 0x0800, CRC(70d5430e) SHA1(d512fc3bb0cf0816a1c987f7188c4b331303347f) )

	ROM_REGION( 0x0400, "gfx3", 0 )
	ROM_LOAD( "30294.p4", 0x0000, 0x0400, CRC(ea93f4b9) SHA1(48b4e0136f5349eb53fea7127a969d87457d70f9) )

	ROM_REGION( 0x0400, "gfx4", 0 )
	ROM_LOAD( "30293.m6", 0x0000, 0x0400, CRC(11900d47) SHA1(2dcb3c3488a5e9ed7f1751649f8dc25696f0f57a) )

	ROM_REGION( 0x0800, "speech", 0 ) /* voice data */
	ROM_LOAD_NIB_LOW ( "30863.r1", 0x0000, 0x0800, CRC(3f779f13) SHA1(8ed8a1bf680e8277066416f467388e3875e8cbbd) )
	ROM_LOAD_NIB_HIGH( "30864.r3", 0x0000, 0x0800, CRC(c4a58d1d) SHA1(a2ba9354b99c739bbfa94458d671c109be163ca0) )
ROM_END


GAME( 1978, wolfpack, 0, wolfpack, wolfpack, driver_device, 0, ORIENTATION_FLIP_Y, "Atari", "Wolf Pack (prototype)", MACHINE_IMPERFECT_SOUND )
