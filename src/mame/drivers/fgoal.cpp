// license:???
// copyright-holders:Stefan Jokisch
/***************************************************************************

Taito Field Goal driver

    set #1 / orig Taito PCB / sticker "AFN00004" / Field Goal
    set #2 / orig Taito PCB / sticker "MFN00001" / Field Goal

Differences between these sets include

    - ball speed
    - paddle color and position
    - scoring of bonus points
    - when bonus score reaches 1000...
        set #1: paddle gets sticky
        set #2: paddle reflects the ball vertically upward

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "includes/fgoal.h"


int fgoal_state::intensity(int bits)
{
	int v = 0;

	/* contrary to the schems pull-up resistors are 270 and not 390 */

	if (1)
	{
		v += 0x2e; /* 100 + 270 */
	}
	if (bits & 1)
	{
		v += 0x27; /* 100 + 330 */
	}
	if (bits & 2)
	{
		v += 0xaa; /* 100 */
	}

	return v;
}


PALETTE_INIT_MEMBER(fgoal_state, fgoal)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* for B/W screens PCB can be jumpered to use lower half of PROM */

	for (i = 0; i < 128; i++)
	{
		UINT8 color = color_prom[0x80 | i] & 63;
		palette.set_pen_color(i, intensity(color >> 4), intensity(color >> 2), intensity(color >> 0));
	}

	for (i = 0; i < 8; i++)
	{
		palette.set_pen_color(128 + 0*8 + i, rgb_t(0x2e,0x80,0x2e));
		palette.set_pen_color(128 + 1*8 + i, rgb_t(0x2e,0x2e,0x2e));
	}

	/* ball is a fixed color */
	palette.set_pen_color(128 + 16, intensity(0x38 >> 4), intensity(0x38 >> 2), intensity(0x38 >> 0));
}


void fgoal_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_INTERRUPT:
		interrupt_callback(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in fgoal_state::device_timer");
	}
}


TIMER_CALLBACK_MEMBER(fgoal_state::interrupt_callback)
{
	int scanline;
	int coin = (ioport("IN1")->read() & 2);

	m_maincpu->set_input_line(0, ASSERT_LINE);

	if (!coin && m_prev_coin)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

	m_prev_coin = coin;

	scanline = m_screen->vpos() + 128;

	if (scanline > 256)
		scanline = 0;

	m_interrupt_timer->adjust(m_screen->time_until_pos(scanline));
}


unsigned fgoal_state::video_ram_address( )
{
	return 0x4000 | (m_row << 5) | (m_col >> 3);
}


READ8_MEMBER(fgoal_state::analog_r)
{
	return ioport(m_player ? "PADDLE1" : "PADDLE0")->read(); /* PCB can be jumpered to use a single dial */
}


CUSTOM_INPUT_MEMBER(fgoal_state::_80_r)
{
	UINT8 ret = (m_screen->vpos() & 0x80) ? 1 : 0;

	return ret;
}

READ8_MEMBER(fgoal_state::nmi_reset_r)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	return 0;
}


READ8_MEMBER(fgoal_state::irq_reset_r)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);

	return 0;
}


READ8_MEMBER(fgoal_state::row_r)
{
	return m_row;
}


WRITE8_MEMBER(fgoal_state::row_w)
{
	m_row = data;
	m_mb14241->shift_data_w(space, 0, 0);
}

WRITE8_MEMBER(fgoal_state::col_w)
{
	m_col = data;
	m_mb14241->shift_count_w(space, 0, data);
}

READ8_MEMBER(fgoal_state::address_hi_r)
{
	return video_ram_address() >> 8;
}

READ8_MEMBER(fgoal_state::address_lo_r)
{
	return video_ram_address() & 0xff;
}

READ8_MEMBER(fgoal_state::shifter_r)
{
	UINT8 v = m_mb14241->shift_result_r(space, 0);

	return BITSWAP8(v, 7, 6, 5, 4, 3, 2, 1, 0);
}

READ8_MEMBER(fgoal_state::shifter_reverse_r)
{
	UINT8 v = m_mb14241->shift_result_r(space, 0);

	return BITSWAP8(v, 0, 1, 2, 3, 4, 5, 6, 7);
}


WRITE8_MEMBER(fgoal_state::sound1_w)
{
	/* BIT0 => SX2 */
	/* BIT1 => SX1 */
	/* BIT2 => SX1 */
	/* BIT3 => SX1 */
	/* BIT4 => SX1 */
	/* BIT5 => SX1 */
	/* BIT6 => SX1 */
	/* BIT7 => SX1 */
}


WRITE8_MEMBER(fgoal_state::sound2_w)
{
	/* BIT0 => CX0 */
	/* BIT1 => SX6 */
	/* BIT2 => N/C */
	/* BIT3 => SX5 */
	/* BIT4 => SX4 */
	/* BIT5 => SX3 */
	m_player = data & 1;
}


static ADDRESS_MAP_START( cpu_map, AS_PROGRAM, 8, fgoal_state )

	AM_RANGE(0x0000, 0x00ef) AM_RAM

	AM_RANGE(0x00f0, 0x00f0) AM_READ(row_r)
	AM_RANGE(0x00f1, 0x00f1) AM_READ(analog_r)
	AM_RANGE(0x00f2, 0x00f2) AM_READ_PORT("IN0")
	AM_RANGE(0x00f3, 0x00f3) AM_READ_PORT("IN1")
	AM_RANGE(0x00f4, 0x00f4) AM_READ(address_hi_r)
	AM_RANGE(0x00f5, 0x00f5) AM_READ(address_lo_r)
	AM_RANGE(0x00f6, 0x00f6) AM_READ(shifter_r)
	AM_RANGE(0x00f7, 0x00f7) AM_READ(shifter_reverse_r)
	AM_RANGE(0x00f8, 0x00fb) AM_READ(nmi_reset_r)
	AM_RANGE(0x00fc, 0x00ff) AM_READ(irq_reset_r)

	AM_RANGE(0x00f0, 0x00f0) AM_WRITE(row_w)
	AM_RANGE(0x00f1, 0x00f1) AM_WRITE(col_w)
	AM_RANGE(0x00f2, 0x00f2) AM_WRITE(row_w)
	AM_RANGE(0x00f3, 0x00f3) AM_WRITE(col_w)
	AM_RANGE(0x00f4, 0x00f7) AM_DEVWRITE("mb14241", mb14241_device, shift_data_w)
	AM_RANGE(0x00f8, 0x00fb) AM_WRITE(sound1_w)
	AM_RANGE(0x00fc, 0x00ff) AM_WRITE(sound2_w)

	AM_RANGE(0x0100, 0x03ff) AM_RAM
	AM_RANGE(0x4000, 0x7fff) AM_RAM AM_SHARE("video_ram")

	AM_RANGE(0x8000, 0x8000) AM_WRITE(ypos_w)
	AM_RANGE(0x8001, 0x8001) AM_WRITE(xpos_w)
	AM_RANGE(0x8002, 0x8002) AM_WRITE(color_w)

	AM_RANGE(0xa000, 0xbfff) AM_ROM
	AM_RANGE(0xd000, 0xffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( fgoal )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT )
	PORT_DIPNAME( 0x40, 0x40, "Display Coinage Settings" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Lives ))
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x18, 0x18, "Options" ) /* bit #4 comes from a jumper */
	PORT_DIPSETTING(    0x00, "Clear All Helmets" )
	PORT_DIPSETTING(    0x08, "No Extra Ball" )
	PORT_DIPSETTING(    0x10, "No Extra Credit" )
	PORT_DIPSETTING(    0x18, "Default" )
	PORT_DIPNAME( 0x07, 0x05, "Initial Extra Credit Score" )
	PORT_DIPSETTING(    0x00, "9000" )
	PORT_DIPSETTING(    0x01, "17000" )
	PORT_DIPSETTING(    0x02, "28000" )
	PORT_DIPSETTING(    0x03, "39000" )
	PORT_DIPSETTING(    0x04, "50000" )
	PORT_DIPSETTING(    0x05, "65000" )
	PORT_DIPSETTING(    0x06, "79000" )
	PORT_DIPSETTING(    0x07, "93000" )
	/* extra credit score changes depending on player's performance */

	PORT_START("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, fgoal_state, _80_r, NULL) /* 128V */
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ))
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ))
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ))
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Language ))
	PORT_DIPSETTING(    0x00, DEF_STR( Japanese ))
	PORT_DIPSETTING(    0x10, DEF_STR( English ))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))

	/* game freezes when analog controls read $00 or $ff */
	PORT_START("PADDLE0")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(1, 254) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("PADDLE1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(1, 254) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(2)
INPUT_PORTS_END

static const UINT32 gfxlayout_xoffset[64] =
{
	0x000, 0x008, 0x010, 0x018, 0x020, 0x028, 0x030, 0x038,
	0x040, 0x048, 0x050, 0x058, 0x060, 0x068, 0x070, 0x078,
	0x080, 0x088, 0x090, 0x098, 0x0a0, 0x0a8, 0x0b0, 0x0b8,
	0x0c0, 0x0c8, 0x0d0, 0x0d8, 0x0e0, 0x0e8, 0x0f0, 0x0f8,
	0x100, 0x108, 0x110, 0x118, 0x120, 0x128, 0x130, 0x138,
	0x140, 0x148, 0x150, 0x158, 0x160, 0x168, 0x170, 0x178,
	0x180, 0x188, 0x190, 0x198, 0x1a0, 0x1a8, 0x1b0, 0x1b8,
	0x1c0, 0x1c8, 0x1d0, 0x1d8, 0x1e0, 0x1e8, 0x1f0, 0x1f8
};

static const UINT32 gfxlayout_yoffset[64] =
{
	0x0000, 0x0200, 0x0400, 0x0600, 0x0800, 0x0a00, 0x0c00, 0x0e00,
	0x1000, 0x1200, 0x1400, 0x1600, 0x1800, 0x1a00, 0x1c00, 0x1e00,
	0x2000, 0x2200, 0x2400, 0x2600, 0x2800, 0x2a00, 0x2c00, 0x2e00,
	0x3000, 0x3200, 0x3400, 0x3600, 0x3800, 0x3a00, 0x3c00, 0x3e00,
	0x4000, 0x4200, 0x4400, 0x4600, 0x4800, 0x4a00, 0x4c00, 0x4e00,
	0x5000, 0x5200, 0x5400, 0x5600, 0x5800, 0x5a00, 0x5c00, 0x5e00,
	0x6000, 0x6200, 0x6400, 0x6600, 0x6800, 0x6a00, 0x6c00, 0x6e00,
	0x7000, 0x7200, 0x7400, 0x7600, 0x7800, 0x7a00, 0x7c00, 0x7e00
};

static const gfx_layout gfxlayout =
{
	64, 64,
	1,
	4,
	{ 4, 5, 6, 7 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	0,
	gfxlayout_xoffset,
	gfxlayout_yoffset
};


static GFXDECODE_START( fgoal )
	GFXDECODE_ENTRY( "gfx1", 0, gfxlayout, 0x00, 8 ) /* foreground */
	GFXDECODE_ENTRY( "gfx1", 0, gfxlayout, 0x80, 1 ) /* background */
GFXDECODE_END



void fgoal_state::machine_start()
{
	m_interrupt_timer = timer_alloc(TIMER_INTERRUPT);

	save_item(NAME(m_xpos));
	save_item(NAME(m_ypos));
	save_item(NAME(m_current_color));
	save_item(NAME(m_player));
	save_item(NAME(m_row));
	save_item(NAME(m_col));
	save_item(NAME(m_prev_coin));
}

void fgoal_state::machine_reset()
{
	m_interrupt_timer->adjust(m_screen->time_until_pos(0));

	m_xpos = 0;
	m_ypos = 0;
	m_current_color = 0;
	m_player = 0;
	m_row = 0;
	m_col = 0;
	m_prev_coin = 0;
}

static MACHINE_CONFIG_START( fgoal, fgoal_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 10065000 / 10) /* ? */
	MCFG_CPU_PROGRAM_MAP(cpu_map)


	/* add shifter */
	MCFG_MB14241_ADD("mb14241")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(256, 263)
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 16, 255)
	MCFG_SCREEN_UPDATE_DRIVER(fgoal_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", fgoal)
	MCFG_PALETTE_ADD("palette", 128 + 16 + 1)
	MCFG_PALETTE_INIT_OWNER(fgoal_state, fgoal)

	/* sound hardware */
MACHINE_CONFIG_END


ROM_START( fgoal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tf04.m28", 0xa000, 0x0800, CRC(45fd7b03) SHA1(adc75a7fff6402c5c668ac28aec5d7c31c67c948) )
	ROM_RELOAD(           0xe000, 0x0800 )
	ROM_LOAD( "tf03.m31", 0xa800, 0x0800, CRC(01891c32) SHA1(013480dc970da83bda969506b2bd8865753a78ad) )
	ROM_RELOAD(           0xe800, 0x0800 )
	ROM_LOAD( "tf02.m38", 0xb000, 0x0800, CRC(c297d509) SHA1(a180e5203008db6b358dceee7349682ae3675c20) )
	ROM_RELOAD(           0xf000, 0x0800 )
	ROM_LOAD( "tf01.m46", 0xb800, 0x0800, CRC(1b0bfa5c) SHA1(768e14f08063cc022d7e18a9cb2197d64a9e1b8d) )
	ROM_RELOAD(           0xf800, 0x0800 )

	ROM_REGION( 0x1000, "gfx1", 0 ) /* overlay proms */
	ROM_LOAD( "tf05.m11", 0x0000, 0x0400, CRC(925b78ab) SHA1(97d6e572658715dc4f6c37b98ba5352643fc8e27) )
	ROM_LOAD( "tf06.m4",  0x0400, 0x0400, CRC(3d2f007b) SHA1(7f4b6f3f08be8c886af3e2ccd3c0d93ae54d4649) )
	ROM_LOAD( "tf07.m12", 0x0800, 0x0400, CRC(0b1d01c4) SHA1(8680602fecd412e5136e1107618a2e0a59b37d08) )
	ROM_LOAD( "tf08.m5",  0x0c00, 0x0400, CRC(5cbc7dfd) SHA1(1a054dc72d25615ea6f903f6da8108033514fd1f) )

	ROM_REGION( 0x0100, "proms", ROMREGION_INVERT )
	ROM_LOAD_NIB_LOW ( "tf09.m13", 0x0000, 0x0100, CRC(b0fc4b80) SHA1(c6029f6d912275aa65302ca97281e10ccbf63159) )
	ROM_LOAD_NIB_HIGH( "tf10.m6",  0x0000, 0x0100, CRC(7b30b15d) SHA1(e9826a107b209e18d891ead341eda3d4523ce195) )
ROM_END


ROM_START( fgoala )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mf04.m28", 0xa000, 0x0800, CRC(acba21bc) SHA1(4a82e88555491883628a07f905d130380d5274f1) )
	ROM_RELOAD(           0xe000, 0x0800 )
	ROM_LOAD( "mf03.m31", 0xa800, 0x0800, CRC(4ce7462d) SHA1(ff02b4a831967c4e75e1d42e0679224b107d61bd) )
	ROM_RELOAD(           0xe800, 0x0800 )
	ROM_LOAD( "mf02.m38", 0xb000, 0x0800, CRC(5cd889b9) SHA1(7c8d810fed6d5e57c9b6a00e699f5b1d1253e84e) )
	ROM_RELOAD(           0xf000, 0x0800 )
	ROM_LOAD( "mf01.m46", 0xb800, 0x0800, CRC(9b9f5faa) SHA1(f944fff2c07e70f86fdd28fa5c9dc6c75ea2028b) )
	ROM_RELOAD(           0xf800, 0x0800 )
	ROM_LOAD( "mf05.m22", 0xd800, 0x0800, CRC(58082b8b) SHA1(72cd4153f7939cd33fc69ba82b44391fc19ae152) )

	ROM_REGION( 0x1000, "gfx1", 0 ) /* overlay proms */
	ROM_LOAD( "tf05.m11", 0x0000, 0x0400, CRC(925b78ab) SHA1(97d6e572658715dc4f6c37b98ba5352643fc8e27) )
	ROM_LOAD( "tf06.m4",  0x0400, 0x0400, CRC(3d2f007b) SHA1(7f4b6f3f08be8c886af3e2ccd3c0d93ae54d4649) )
	ROM_LOAD( "tf07.m12", 0x0800, 0x0400, CRC(0b1d01c4) SHA1(8680602fecd412e5136e1107618a2e0a59b37d08) )
	ROM_LOAD( "tf08.m5",  0x0c00, 0x0400, CRC(5cbc7dfd) SHA1(1a054dc72d25615ea6f903f6da8108033514fd1f) )

	ROM_REGION( 0x0100, "proms", ROMREGION_INVERT )
	ROM_LOAD_NIB_LOW ( "tf09.m13", 0x0000, 0x0100, CRC(b0fc4b80) SHA1(c6029f6d912275aa65302ca97281e10ccbf63159) )
	ROM_LOAD_NIB_HIGH( "tf10.m6",  0x0000, 0x0100, CRC(7b30b15d) SHA1(e9826a107b209e18d891ead341eda3d4523ce195) )
ROM_END


GAME( 1979, fgoal,  0,     fgoal, fgoal, driver_device, 0, ROT90, "Taito", "Field Goal (set 1)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, fgoala, fgoal, fgoal, fgoal, driver_device, 0, ROT90, "Taito", "Field Goal (set 2)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
