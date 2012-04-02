/****************************************************************************

Tug Boat
6502 hooked up + preliminary video by MooglyGuy

TODO:
- verify connections of the two PIAs. I only hooked up a couple of ports but
  there are more.
- check how the score is displayed. I'm quite sure that tugboat_score_w is
  supposed to access videoram scanning it by columns (like btime_mirrorvideoram_w),
  but the current implementation is a big kludge, and it still looks wrong.
- colors might not be entirely accurate
  Suspect berenstn is using the wrong color PROM.

the problem which caused the controls not to work
---
There's counter at $000b, counting up from $ff to 0 or from $fe to 0 (initial value depends
on game level). It's increased in main loop, and used for game flow control (scrolling speed , controls  etc).
Every interrupt, when (counter&3)!=0 , there's a check for left/right inputs .
But when init val was $ff (2nd level),  the condition 'counter&3!=0' was
always false - counter was reloaded and incremented before interrupt occurs

****************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6821pia.h"
#include "sound/ay8910.h"


class tugboat_state : public driver_device
{
public:
	tugboat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_ram;
	UINT8 m_hd46505_0_reg[18];
	UINT8 m_hd46505_1_reg[18];
	int m_reg0;
	int m_reg1;
	int m_ctrl;
};


/*  there isn't the usual resistor array anywhere near the color prom,
    just four 1k resistors. */
static PALETTE_INIT( tugboat )
{
	int i;


	for (i = 0;i < machine.total_colors();i++)
	{
		int r,g,b,brt;


		brt = ((color_prom[i] >> 3) & 0x01) ? 0xff : 0x80;

		r = brt * ((color_prom[i] >> 0) & 0x01);
		g = brt * ((color_prom[i] >> 1) & 0x01);
		b = brt * ((color_prom[i] >> 2) & 0x01);

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}



/* see mc6845.c. That file is only a placeholder, I process the writes here
   because I need the start_addr register to handle scrolling */
static WRITE8_HANDLER( tugboat_hd46505_0_w )
{
	tugboat_state *state = space->machine().driver_data<tugboat_state>();
	if (offset == 0) state->m_reg0 = data & 0x0f;
	else if (state->m_reg0 < 18) state->m_hd46505_0_reg[state->m_reg0] = data;
}
static WRITE8_HANDLER( tugboat_hd46505_1_w )
{
	tugboat_state *state = space->machine().driver_data<tugboat_state>();
	if (offset == 0) state->m_reg1 = data & 0x0f;
	else if (state->m_reg1 < 18) state->m_hd46505_1_reg[state->m_reg1] = data;
}


static WRITE8_HANDLER( tugboat_score_w )
{
	tugboat_state *state = space->machine().driver_data<tugboat_state>();
      if (offset>=0x8) state->m_ram[0x291d + 32*offset + 32*(1-8)] = data ^ 0x0f;
      if (offset<0x8 ) state->m_ram[0x291d + 32*offset + 32*9] = data ^ 0x0f;
}

static void draw_tilemap(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect,
		int addr,int gfx0,int gfx1,int transparency)
{
	tugboat_state *state = machine.driver_data<tugboat_state>();
	int x,y;

	for (y = 0;y < 32;y++)
	{
		for (x = 0;x < 32;x++)
		{
			int code = (state->m_ram[addr + 0x400] << 8) | state->m_ram[addr];
			int color = (code & 0x3c00) >> 10;
			int rgn;

			code &=0x3ff;
			rgn = gfx0;

			if (code > 0x1ff)
			{
				code &= 0x1ff;
				rgn = gfx1;
			}

			drawgfx_transpen(bitmap,cliprect,machine.gfx[rgn],
					code,
					color,
					0,0,
					8*x,8*y,
					transparency ? 7 : -1);

			addr = (addr & 0xfc00) | ((addr + 1) & 0x03ff);
		}
	}
}

static SCREEN_UPDATE_IND16( tugboat )
{
	tugboat_state *state = screen.machine().driver_data<tugboat_state>();
	int startaddr0 = state->m_hd46505_0_reg[0x0c]*256 + state->m_hd46505_0_reg[0x0d];
	int startaddr1 = state->m_hd46505_1_reg[0x0c]*256 + state->m_hd46505_1_reg[0x0d];


	draw_tilemap(screen.machine(), bitmap,cliprect,startaddr0,0,1,FALSE);
	draw_tilemap(screen.machine(), bitmap,cliprect,startaddr1,2,3,TRUE);
	return 0;
}



static READ8_DEVICE_HANDLER( tugboat_input_r )
{
	tugboat_state *state = device->machine().driver_data<tugboat_state>();
	if (~state->m_ctrl & 0x80)
		return input_port_read(device->machine(), "IN0");
	else if (~state->m_ctrl & 0x40)
		return input_port_read(device->machine(), "IN1");
	else if (~state->m_ctrl & 0x20)
		return input_port_read(device->machine(), "IN2");
	else if (~state->m_ctrl & 0x10)
		return input_port_read(device->machine(), "IN3");
	else
		return input_port_read(device->machine(), "IN4");
}

static READ8_DEVICE_HANDLER( tugboat_ctrl_r )
{
	tugboat_state *state = device->machine().driver_data<tugboat_state>();
	return state->m_ctrl;
}

static WRITE8_DEVICE_HANDLER( tugboat_ctrl_w )
{
	tugboat_state *state = device->machine().driver_data<tugboat_state>();
	state->m_ctrl = data;
}

static const pia6821_interface pia0_intf =
{
	DEVCB_HANDLER(tugboat_input_r),		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

static const pia6821_interface pia1_intf =
{
	DEVCB_INPUT_PORT("DSW"),			/* port A in */
	DEVCB_HANDLER(tugboat_ctrl_r),		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_HANDLER(tugboat_ctrl_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

static TIMER_CALLBACK( interrupt_gen )
{
	cputag_set_input_line(machine, "maincpu", 0, HOLD_LINE);
	machine.scheduler().timer_set(machine.primary_screen->frame_period(), FUNC(interrupt_gen));
}

static MACHINE_RESET( tugboat )
{
	machine.scheduler().timer_set(machine.primary_screen->time_until_pos(30*8+4), FUNC(interrupt_gen));
}


static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, tugboat_state )
	AM_RANGE(0x0000, 0x01ff) AM_RAM AM_BASE(m_ram)
	AM_RANGE(0x1060, 0x1061) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_data_w)
	AM_RANGE(0x10a0, 0x10a1) AM_WRITE_LEGACY(tugboat_hd46505_0_w)	/* scrolling is performed changing the start_addr register (0C/0D) */
	AM_RANGE(0x10c0, 0x10c1) AM_WRITE_LEGACY(tugboat_hd46505_1_w)
	AM_RANGE(0x11e4, 0x11e7) AM_DEVREADWRITE("pia0", pia6821_device, read, write)
	AM_RANGE(0x11e8, 0x11eb) AM_DEVREADWRITE("pia1", pia6821_device, read, write)
	//AM_RANGE(0x1700, 0x1fff) AM_RAM
	AM_RANGE(0x18e0, 0x18ef) AM_WRITE_LEGACY(tugboat_score_w)
	AM_RANGE(0x2000, 0x2fff) AM_RAM	/* tilemap RAM */
	AM_RANGE(0x4000, 0x7fff) AM_ROM
	AM_RANGE(0xfff0, 0xffff) AM_ROM	/* vectors */
ADDRESS_MAP_END


static INPUT_PORTS_START( tugboat )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( noahsark )
	PORT_INCLUDE( tugboat )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( tugboat )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0x80, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout, 0x80, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, charlayout, 0x00, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, charlayout, 0x00, 16 )
GFXDECODE_END


static MACHINE_CONFIG_START( tugboat, tugboat_state )
	MCFG_CPU_ADD("maincpu", M6502, 2000000)	/* 2 MHz ???? */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MCFG_MACHINE_RESET(tugboat)

	MCFG_PIA6821_ADD("pia0", pia0_intf)
	MCFG_PIA6821_ADD("pia1", pia1_intf)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(32*8,32*8)
	MCFG_SCREEN_VISIBLE_AREA(1*8,31*8-1,2*8,30*8-1)
	MCFG_SCREEN_UPDATE_STATIC(tugboat)

	MCFG_GFXDECODE(tugboat)
	MCFG_PALETTE_LENGTH(256)

	MCFG_PALETTE_INIT(tugboat)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 2000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)
MACHINE_CONFIG_END


ROM_START( tugboat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u7.bin", 0x5000, 0x1000, CRC(e81d7581) SHA1(c76327e3b027a5a2af69f8cfafa1f828ad0ebdb1) )
	ROM_LOAD( "u8.bin", 0x6000, 0x1000, CRC(7525de06) SHA1(0722c7a0b89c55162227173679ffbe398ca350a2) )
	ROM_LOAD( "u9.bin", 0x7000, 0x1000, CRC(aa4ae687) SHA1(a212eed5d04d6197aa3484ff36059fd7998604a6) )
	ROM_RELOAD(         0xf000, 0x1000 )	/* for the vectors */

	ROM_REGION( 0x1800, "gfx1", ROMREGION_INVERT  )
	ROM_LOAD( "u67.bin",  0x0000, 0x0800, CRC(601c425b) SHA1(13ed54ba1307ba3f779293d88c19d0c0f2d91a96) )
	ROM_FILL(             0x0800, 0x0800, 0xff )
	ROM_FILL(             0x1000, 0x0800, 0xff )

	ROM_REGION( 0x3000, "gfx2", ROMREGION_INVERT  )
	ROM_LOAD( "u68.bin", 0x0000, 0x1000, CRC(d5835182) SHA1(f67c8f93e0d7dd1bf8e3a98756719d386c133d1c) )
	ROM_LOAD( "u69.bin", 0x1000, 0x1000, CRC(e6d25878) SHA1(de9096ef3108d031049be1e7f2c5e346d0bc0df1) )
	ROM_LOAD( "u70.bin", 0x2000, 0x1000, CRC(34ce2850) SHA1(8883126627ed8a1d2c3bed2a3d169ce35eafc8a3) )

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_LOAD( "u168.bin", 0x0000, 0x0800, CRC(279042fd) SHA1(1361fff1bc532251bbd36b7b60776c2cc137cfba) )	/* labeled u-167 */
	ROM_RELOAD(         0x0800, 0x0800 )
	ROM_RELOAD(         0x1000, 0x0800 )

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_LOAD( "u170.bin", 0x0000, 0x0800, CRC(64d9f4d7) SHA1(3ff7fc099023512c33ec4583e91e6cbab903e7a8) )	/* labeled u-168 */
	ROM_LOAD( "u169.bin", 0x0800, 0x0800, CRC(1a636296) SHA1(bcb18d714328ba3db2d16d74c47a985c16a0bbe2) )	/* labeled u-169 */
	ROM_LOAD( "u167.bin", 0x1000, 0x0800, CRC(b9c9b4f7) SHA1(6685d580ae150d7c67bac2786ee4b7a2c28eddc3) )	/* labeled u-170 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "nt2_u128.clr", 0x0000, 0x0100, CRC(236672bf) SHA1(57482d0a23223ef7b211045ad28d3e41e90f961e) )
ROM_END


ROM_START( noahsark )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u6.bin", 0x4000, 0x1000, CRC(3579eeac) SHA1(f54435ac6b31cf81342de83965cf8a8503b26eb8) )
	ROM_LOAD( "u7.bin", 0x5000, 0x1000, CRC(64b0afae) SHA1(1fcc17490d1290565be38a817f783604bcefb8be) )
	ROM_LOAD( "u8.bin", 0x6000, 0x1000, CRC(02d53f62) SHA1(e51a583a548b4bdaf43d376d5d276325ee448d49) )
	ROM_LOAD( "u9.bin", 0x7000, 0x1000, CRC(d425b61c) SHA1(a8d9562435cc910916df4cd7e958468d88ff92e7) )
	ROM_RELOAD(         0xf000, 0x1000 )	/* for the vectors */

	ROM_REGION( 0x1800, "gfx1", ROMREGION_INVERT  )
	ROM_LOAD( "u67.bin",  0x0000, 0x0800, CRC(1a77605b) SHA1(8c25750f94895f5820ad4f1fa4ae1ea70ee0aee2) )
	ROM_FILL(             0x0800, 0x0800, 0xff )
	ROM_FILL(             0x1000, 0x0800, 0xff )

	ROM_REGION( 0x3000, "gfx2", ROMREGION_INVERT  )
	ROM_LOAD( "u68.bin", 0x0000, 0x1000, CRC(6a66eac8) SHA1(3a13c2f5ef45cdd8b8b5db07d8c1417a3304723a) )
	ROM_LOAD( "u69.bin", 0x1000, 0x1000, CRC(fa2c279c) SHA1(332fcfcfe605c4132114399c32932507b16752e5) )
	ROM_LOAD( "u70.bin", 0x2000, 0x1000, CRC(dcabc7c5) SHA1(68abfdedea518e3a5c90f9f72173e8c05e190535) )

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_LOAD( "u168.bin", 0x0000, 0x0800, CRC(7fc7280f) SHA1(93bf46e421b580edf81177db85cb220073761c57) )	/* labeled u-167 */
	ROM_RELOAD(         0x0800, 0x0800 )
	ROM_RELOAD(         0x1000, 0x0800 )

	ROM_REGION( 0x3000, "gfx4", 0 )
	ROM_LOAD( "u170.bin", 0x0000, 0x1000, CRC(ba36641c) SHA1(df206dc4b6f2da7b60bdaa72c8175de928a630a4) )	/* labeled u-168 */
	ROM_LOAD( "u169.bin", 0x1000, 0x1000, CRC(68c58207) SHA1(e09f9f8b5f1071fbf8a4883f75f296ec4bc0eca1) )	/* labeled u-169 */
	ROM_LOAD( "u167.bin", 0x2000, 0x1000, CRC(76f16c5b) SHA1(a8a8f0ad7dcc57c2bf518fc5e2509ed8fb87f403) )	/* labeled u-170 */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "u128.bin", 0x0000, 0x0100, CRC(816784bd) SHA1(47181f4a6ab35c46796ca1d8c130b76f404c188d) )
ROM_END


ROM_START( berenstn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u6.bin", 0x4000, 0x1000, CRC(e45275a2) SHA1(d788bc5a69b3cdb2596a3b354371ff88d39f6d46) )
	ROM_LOAD( "u7.bin", 0x5000, 0x1000, CRC(1984d787) SHA1(c13959c9be075400e9d1668b5404bc73f6db5fe4) )
	ROM_LOAD( "u8.bin", 0x6000, 0x1000, CRC(0c4d53b7) SHA1(45bd847fdb7bbfbe53d750003024ef3454faa6e6) )
	ROM_LOAD( "u9.bin", 0x7000, 0x1000, CRC(7e058e57) SHA1(e9506fa4ec693abf0dc4e4cbfd4b93bdbcfc9ba4) )
	ROM_RELOAD(         0xf000, 0x1000 )	/* for the vectors */

	ROM_REGION( 0x1800, "gfx1", ROMREGION_INVERT  )
	ROM_LOAD( "u67.bin",  0x0000, 0x0800, CRC(1a77605b) SHA1(8c25750f94895f5820ad4f1fa4ae1ea70ee0aee2) )
	ROM_FILL(             0x0800, 0x0800, 0xff )
	ROM_FILL(             0x1000, 0x0800, 0xff )

	ROM_REGION( 0x3000, "gfx2", ROMREGION_INVERT  )
	ROM_LOAD( "u68.bin", 0x0000, 0x1000, CRC(21bf375f) SHA1(52bc81a4f289a96edfab034445bcf639b1524ada) )
	ROM_LOAD( "u69.bin", 0x1000, 0x1000, CRC(9dc770f6) SHA1(5dc16fac72d68b521dbb415935f5e7f682c26d7f) )
	ROM_LOAD( "u70.bin", 0x2000, 0x1000, CRC(a810bd45) SHA1(8be531529174c5d4b4f164bd2397116b9d5350db) )

	ROM_REGION( 0x1800, "gfx3", 0 )
	ROM_LOAD( "u167.bin", 0x0000, 0x0800, CRC(7fc7280f) SHA1(93bf46e421b580edf81177db85cb220073761c57) )
	ROM_RELOAD(         0x0800, 0x0800 )
	ROM_RELOAD(         0x1000, 0x0800 )

	ROM_REGION( 0x3000, "gfx4", 0 )
	ROM_LOAD( "u168.bin", 0x0000, 0x0800, CRC(af532ba3) SHA1(b196e294eaf4c25549278fd040b1dad2799e18d5) )
	ROM_LOAD( "u169.bin", 0x1000, 0x0800, CRC(07b6e660) SHA1(c755f63cc7c566e200fc11199bfac06a7e8f89e4) )
	ROM_LOAD( "u170.bin", 0x2000, 0x0800, CRC(73261eff) SHA1(19edd6957fceb3df12fd29cd5e156a5eb1c70710) )

	ROM_REGION( 0x0100, "proms", 0 ) /* Same as Tugboat but is this actually correct? */
	ROM_LOAD( "n.t.2-031j.24s10", 0x0000, 0x0100, CRC(236672bf) SHA1(57482d0a23223ef7b211045ad28d3e41e90f961e) )
ROM_END


GAME( 1982, tugboat,  0, tugboat, tugboat,  0, ROT90, "Enter-Tech, Ltd.", "Tugboat",    GAME_IMPERFECT_GRAPHICS )
GAME( 1983, noahsark, 0, tugboat, noahsark, 0, ROT90, "Enter-Tech, Ltd.", "Noah's Ark", GAME_IMPERFECT_GRAPHICS )
GAME( 1984, berenstn, 0, tugboat, noahsark, 0, ROT90, "Enter-Tech, Ltd.", "The Berenstain Bears in Big Paw's Cave", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_COLORS )

