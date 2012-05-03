/*
       Disco Boy

Similar to mitchell.c / egghunt.c .. clearly derived from that hardware

TODO:
- move sound HW into proper file (it's 99% IDENTICAL to yunsung8.c)
- ADPCM has sound volume issues, it's either too loud or too quiet;

PCB Layout
----------

SOFT ART CO. 1993 NO - 1021
PROGRAM NO-93-01-14-0024
+------------------------------------------+
| YM3014 YM3812 10MHz            5.u94     |
|       6116                    6.u124     |
|       1.u45                    7.u95     |
| M5205 2.u28                   8.u125     |
|J     Z8400B                          6116|
|A          6116          6116 TPC1020AFN  |
|M          6116                       6116|
|M    DSW2                                 |
|A DSW1                          6264      |
|      6264                       u80  u81 |
|12MHz  u2                       u50  u5   |
|      u18                        u78  u79 |
|    Z0840004PSC                 u46  u49  |
+------------------------------------------+

Notes:
  Zilog Z0840004PSC (Z80 cpu, main program CPU)
  Goldstar Z8400B PS (Z80 cpu, sound CPU)
  Yamaha YM3014/YM3812 (rebadged as 83142/5A12)
  OKI M5205
  TI TPC1020AFN-084C
  10.000MHz & 12.000MHz OSCs

*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/msm5205.h"
#include "sound/3812intf.h"



class discoboy_state : public driver_device
{
public:
	discoboy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* video-related */
	UINT8    m_ram_bank;
	UINT8    m_gfxbank;
	UINT8    m_port_00;
	int      m_adpcm;
	UINT8    m_toggle;

	/* devices */
	device_t *m_audiocpu;

	/* memory */
	UINT8    m_ram_1[0x800];
	UINT8    m_ram_2[0x800];
	UINT8    m_ram_3[0x1000];
	UINT8    m_ram_4[0x1000];
	UINT8    m_ram_att[0x800];
	DECLARE_WRITE8_MEMBER(rambank_select_w);
	DECLARE_WRITE8_MEMBER(discoboy_port_00_w);
	DECLARE_WRITE8_MEMBER(discoboy_port_01_w);
	DECLARE_WRITE8_MEMBER(discoboy_port_03_w);
	DECLARE_WRITE8_MEMBER(discoboy_port_06_w);
	DECLARE_WRITE8_MEMBER(rambank_w);
	DECLARE_READ8_MEMBER(rambank_r);
	DECLARE_READ8_MEMBER(rambank2_r);
	DECLARE_WRITE8_MEMBER(rambank2_w);
	DECLARE_READ8_MEMBER(discoboy_ram_att_r);
	DECLARE_WRITE8_MEMBER(discoboy_ram_att_w);
	DECLARE_READ8_MEMBER(discoboy_port_06_r);
	DECLARE_WRITE8_MEMBER(yunsung8_adpcm_w);
};



static VIDEO_START( discoboy )
{
}

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	discoboy_state *state = machine.driver_data<discoboy_state>();
	int flipscreen = 0;
	int offs, sx, sy;

	for (offs = 0x1000 - 0x40; offs >= 0; offs -= 0x20)
	{
		int code = state->m_ram_4[offs];
		int attr = state->m_ram_4[offs + 1];
		int color = attr & 0x0f;
		sx = state->m_ram_4[offs + 3] + ((attr & 0x10) << 4);
		sy = ((state->m_ram_4[offs + 2] + 8) & 0xff) - 8;
		code += (attr & 0xe0) << 3;

		if (code >= 0x400)
		{
			if ((state->m_gfxbank & 0x30) == 0x00)
			{
				code = 0x400 + (code & 0x3ff);
			}
			else if ((state->m_gfxbank & 0x30) == 0x10)
			{
				code = 0x400 + (code & 0x3ff) + 0x400;
			}
			else if ((state->m_gfxbank & 0x30) == 0x20)
			{
				code = 0x400 + (code & 0x3ff) + 0x800;
			}
			else if ((state->m_gfxbank & 0x30) == 0x30)
			{
				code = 0x400 + (code & 0x3ff) + 0xc00;
			}
			else
			{
				code = machine.rand();
			}
		}

		drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
				 code,
				 color,
				 flipscreen,0,
				 sx,sy,15);
	}
}


static SCREEN_UPDATE_IND16( discoboy )
{
	discoboy_state *state = screen.machine().driver_data<discoboy_state>();
	UINT16 x, y;
	int i;
	int count = 0;

	for (i = 0; i < 0x800; i += 2)
	{
		UINT16 pal;
		int r, g, b;
		pal = state->m_ram_1[i] | (state->m_ram_1[i + 1] << 8);

		b = ((pal >> 0) & 0xf) << 4;
		g = ((pal >> 4) & 0xf) << 4;
		r = ((pal >> 8) & 0xf) << 4;

		palette_set_color(screen.machine(), i / 2, MAKE_RGB(r, g, b));
	}

	for (i = 0; i < 0x800; i += 2)
	{
		UINT16 pal;
		int r,g,b;
		pal = state->m_ram_2[i] | (state->m_ram_2[i + 1] << 8);

		b = ((pal >> 0) & 0xf) << 4;
		g = ((pal >> 4) & 0xf) << 4;
		r = ((pal >> 8) & 0xf) << 4;

		palette_set_color(screen.machine(), (i / 2) + 0x400, MAKE_RGB(r, g, b));
	}

	bitmap.fill(0x3ff, cliprect);

	for (y = 0; y < 32; y++)
	{
		for (x = 0; x < 64; x++)
		{
			UINT16 tileno = state->m_ram_3[count] | (state->m_ram_3[count + 1] << 8);

			if (tileno > 0x2000)
			{
				if ((state->m_gfxbank & 0x40) == 0x40)
					tileno = 0x2000 + (tileno & 0x1fff) + 0x2000;
				else
					tileno = 0x2000 + (tileno & 0x1fff) + 0x0000;
			}

			drawgfx_opaque(bitmap, cliprect, screen.machine().gfx[1], tileno, state->m_ram_att[count / 2], 0, 0, x*8, y*8);
			count += 2;
		}
	}

	draw_sprites(screen.machine(), bitmap, cliprect);

	return 0;
}

#ifdef UNUSED_FUNCTION
void discoboy_setrombank( running_machine &machine, UINT8 data )
{
	UINT8 *ROM = machine.root_device().memregion("maincpu")->base();
	data &= 0x2f;
	space->machine().root_device().membank("bank1")->set_base(&ROM[0x6000 + (data * 0x1000)] );
}
#endif

WRITE8_MEMBER(discoboy_state::rambank_select_w)
{
	m_ram_bank = data;
	if (data &= 0x83) logerror("rambank_select_w !!!!!");
}

WRITE8_MEMBER(discoboy_state::discoboy_port_00_w)
{
	if (data & 0xfe) logerror("unk discoboy_port_00_w %02x\n",data);
	m_port_00 = data;
}

WRITE8_MEMBER(discoboy_state::discoboy_port_01_w)
{

	// 00 10 20 30 during gameplay  1,2,3 other times?? title screen bit 0x40 toggle
	//printf("unk discoboy_port_01_w %02x\n",data);
	// discoboy gfxbank
	m_gfxbank = data & 0xf0;

	membank("bank1")->set_entry(data & 0x07);
}

WRITE8_MEMBER(discoboy_state::discoboy_port_03_w)// sfx? (to sound cpu)
{
	//  printf("unk discoboy_port_03_w %02x\n", data);
	//  device_set_input_line(m_audiocpu, INPUT_LINE_NMI, HOLD_LINE);
	soundlatch_byte_w(space, 0, data);
	device_set_input_line(m_audiocpu, 0, HOLD_LINE);
}

WRITE8_MEMBER(discoboy_state::discoboy_port_06_w)
{
	//printf("unk discoboy_port_06_w %02x\n",data);
	if (data != 0) logerror("port 06!!!! %02x\n",data);
}


WRITE8_MEMBER(discoboy_state::rambank_w)
{

	if (m_ram_bank & 0x20)
		m_ram_2[offset] = data;
	else
		m_ram_1[offset] = data;
}

READ8_MEMBER(discoboy_state::rambank_r)
{

	if (m_ram_bank & 0x20)
		return m_ram_2[offset];
	else
		return m_ram_1[offset];
}

READ8_MEMBER(discoboy_state::rambank2_r)
{

	if (m_port_00 == 0x00)
		return m_ram_3[offset];
	else if (m_port_00 == 0x01)
		return m_ram_4[offset];
	else
		printf("unk rb2_r\n");

	return machine().rand();
}

WRITE8_MEMBER(discoboy_state::rambank2_w)
{

	if (m_port_00 == 0x00)
		m_ram_3[offset] = data;
	else if (m_port_00 == 0x01)
		m_ram_4[offset] = data;
	else
		printf("unk rb2_w\n");
}

READ8_MEMBER(discoboy_state::discoboy_ram_att_r)
{
	return m_ram_att[offset];
}

WRITE8_MEMBER(discoboy_state::discoboy_ram_att_w)
{
	m_ram_att[offset] = data;
}

static ADDRESS_MAP_START( discoboy_map, AS_PROGRAM, 8, discoboy_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc7ff) AM_READWRITE(rambank_r, rambank_w)
	AM_RANGE(0xc800, 0xcfff) AM_READWRITE(discoboy_ram_att_r, discoboy_ram_att_w)
	AM_RANGE(0xd000, 0xdfff) AM_READWRITE(rambank2_r, rambank2_w)
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END


READ8_MEMBER(discoboy_state::discoboy_port_06_r)
{
	return 0x00;
}

static ADDRESS_MAP_START( io_map, AS_IO, 8, discoboy_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("DSWA") AM_WRITE(discoboy_port_00_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("SYSTEM") AM_WRITE(discoboy_port_01_w)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("P1")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("P2") AM_WRITE(discoboy_port_03_w)
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSWB")
	AM_RANGE(0x06, 0x06) AM_READWRITE(discoboy_port_06_r, discoboy_port_06_w) // ???
	AM_RANGE(0x07, 0x07) AM_WRITE(rambank_select_w) // 0x20 is palette bank bit.. others?
ADDRESS_MAP_END

/* Sound */

static WRITE8_DEVICE_HANDLER( yunsung8_sound_bankswitch_w )
{
	/* Note: this is bit 5 on yunsung8.c */
	msm5205_reset_w(device, (data & 0x08) >> 3);

	device->machine().root_device().membank("sndbank")->set_entry(data & 0x07);

	if (data != (data & (~0x0f)))
		logerror("%s: Bank %02X\n", device->machine().describe_context(), data);
}

WRITE8_MEMBER(discoboy_state::yunsung8_adpcm_w)
{

	/* Swap the nibbles */
	m_adpcm = ((data & 0xf) << 4) | ((data >> 4) & 0xf);
}

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, discoboy_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("sndbank")
	AM_RANGE(0xe000, 0xe000) AM_DEVWRITE_LEGACY("msm",yunsung8_sound_bankswitch_w)
	AM_RANGE(0xe400, 0xe400) AM_WRITE(yunsung8_adpcm_w)
	AM_RANGE(0xec00, 0xec01) AM_DEVWRITE_LEGACY("ymsnd", ym3812_w)
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xf800) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( discoboy )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SWA:6,7,8")
	PORT_DIPSETTING(	0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(	0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x02, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(	0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(	0x08, "Every 150000" )
	PORT_DIPSETTING(	0x00, "Every 300000" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(	0x10, "3" )
	PORT_DIPSETTING(	0x00, "4" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWA:2,3")
	PORT_DIPSETTING(	0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(	0x20, DEF_STR( Hardest ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWA:1" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("DSWB")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SWB:8" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SWB:7" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SWB:6" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SWB:5" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SWB:4" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SWB:3" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWB:2" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4,RGN_FRAC(1,2),  RGN_FRAC(0,2)+4,RGN_FRAC(0,2) },
	{ 0, 1, 2, 3, 8, 9, 10, 11, 256, 257, 258, 259, 264,265,266,267  },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	16*32
};

static const gfx_layout tiles8x8_layout2 =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(1,4), RGN_FRAC(0,4),RGN_FRAC(3,4),RGN_FRAC(2,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( discoboy )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0x000, 128 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout2, 0x000, 128 )
GFXDECODE_END

static MACHINE_START( discoboy )
{
	discoboy_state *state = machine.driver_data<discoboy_state>();

	state->m_audiocpu = machine.device("audiocpu");

	state->save_item(NAME(state->m_ram_bank));
	state->save_item(NAME(state->m_port_00));
	state->save_item(NAME(state->m_gfxbank));
	state->save_item(NAME(state->m_adpcm));
	state->save_item(NAME(state->m_toggle));
}

static MACHINE_RESET( discoboy )
{
	discoboy_state *state = machine.driver_data<discoboy_state>();

	state->m_ram_bank = 0;
	state->m_port_00 = 0;
	state->m_gfxbank = 0;
	state->m_adpcm = 0x80;
	state->m_toggle = 0;
}

static void yunsung8_adpcm_int( device_t *device )
{
	discoboy_state *state = device->machine().driver_data<discoboy_state>();

	msm5205_data_w(device, state->m_adpcm >> 4);
	state->m_adpcm <<= 4;

	state->m_toggle ^= 1;
}

static const msm5205_interface yunsung8_msm5205_interface =
{
	yunsung8_adpcm_int,	/* interrupt function */
	MSM5205_S96_4B		/* 4KHz, 4 Bits */
};

static MACHINE_CONFIG_START( discoboy, discoboy_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,12000000/2)		 /* 6 MHz? */
	MCFG_CPU_PROGRAM_MAP(discoboy_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80,10000000/2)		 /* 5 MHz? */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_PERIODIC_INT(nmi_line_pulse,32*60)

	MCFG_MACHINE_START( discoboy )
	MCFG_MACHINE_RESET( discoboy )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(8*8, 512-1-8*8, 0+8, 256-1-8)
	MCFG_SCREEN_UPDATE_STATIC(discoboy)

	MCFG_GFXDECODE(discoboy)
	MCFG_PALETTE_LENGTH(0x1000)

	MCFG_VIDEO_START(discoboy)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM3812, 2500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.6)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.6)

	MCFG_SOUND_ADD("msm", MSM5205, XTAL_400kHz) // ???? unknown
	MCFG_SOUND_CONFIG(yunsung8_msm5205_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.80)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.80)
MACHINE_CONFIG_END


ROM_START( discoboy )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "u2",  0x00000, 0x10000, CRC(44a4fefa) SHA1(29b74bb739afffb7baefb5ed4da09cdb1559b011) )
	ROM_LOAD( "u18", 0x10000, 0x20000, CRC(88d1282d) SHA1(1f11dad0f577198c54a1dc182ba7502e398b998f) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "2.u28",  0x00000, 0x10000, CRC(7c2ed174) SHA1(ace209dc4cc7a4ffca062842defd84cefc5b10d2) )
	ROM_LOAD( "1.u45",  0x10000, 0x10000, CRC(c266c6df) SHA1(f76e38ded43f56a486cf6569c679ddb57a4165fb) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "5.u94",   0x00000, 0x10000, CRC(dbd20836) SHA1(d97651626b1dc16b93f8aed28bac19fd177e626f) )
	ROM_LOAD( "6.u124",  0x10000, 0x40000, CRC(e20d41f8) SHA1(792294a34840867072bc484d6f3cae3502c8bc28) )
	ROM_LOAD( "7.u95",   0x80000, 0x10000, CRC(1d5617a2) SHA1(6b6bd50c1984748dc8bf6600431d9bb6fe443873) )
	ROM_LOAD( "8.u125",  0x90000, 0x40000, CRC(30be1340) SHA1(e4765b75c8f774c6f7f7b5496a50c33ee3950550) )

	ROM_REGION( 0x100000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "u80",   0x00000, 0x10000, CRC(4cc642ae) SHA1(2a59ebc8ab27bf7c3c1aa389ea32fb01d5cfdce8) )
	ROM_LOAD( "u50",   0x10000, 0x20000, CRC(1557ca92) SHA1(5a0afbeede6f0ae1c75bdec446132c673aeb0fe7) )
	ROM_LOAD( "u81",   0x40000, 0x10000, CRC(9e04274e) SHA1(70c28212b242335353e6dd48b7eb176146bec457) )
	ROM_LOAD( "u5",    0x50000, 0x20000, CRC(a07df669) SHA1(7f09b2508b9bffed7a4cd191f707af3c0c2a1de2) )
	ROM_LOAD( "u78",   0x80000, 0x10000, CRC(04571f70) SHA1(afdc7d84f7804c2ced413d13e6985a05f841e79e) )
	ROM_LOAD( "u46",   0x90000, 0x20000, CRC(764ffde4) SHA1(637df403a6ac73456892add3f2403a92afb67f19) )
	ROM_LOAD( "u79",   0xc0000, 0x10000, CRC(646f0f83) SHA1(d5cd050872d4b8c2fc89c3c0f434b1d66e5f1c59) )
	ROM_LOAD( "u49",   0xd0000, 0x20000, CRC(0b6c0d8d) SHA1(820a12c84af4fd5a04e1eca3cbace0002d3024b6) )
ROM_END


static DRIVER_INIT( discoboy )
{
	discoboy_state *state = machine.driver_data<discoboy_state>();
	UINT8 *ROM = state->memregion("maincpu")->base();
	UINT8 *AUDIO = state->memregion("audiocpu")->base();

	memset(state->m_ram_1, 0, sizeof(state->m_ram_1));
	memset(state->m_ram_2, 0, sizeof(state->m_ram_2));
	memset(state->m_ram_att,0, sizeof(state->m_ram_att));
	memset(state->m_ram_3, 0, sizeof(state->m_ram_3));
	memset(state->m_ram_4, 0, sizeof(state->m_ram_4));

	state->save_item(NAME(state->m_ram_1));
	state->save_item(NAME(state->m_ram_2));
	state->save_item(NAME(state->m_ram_att));
	state->save_item(NAME(state->m_ram_3));
	state->save_item(NAME(state->m_ram_4));

	state->membank("bank1")->configure_entries(0, 8, &ROM[0x10000], 0x4000);
	state->membank("bank1")->set_entry(0);
	state->membank("sndbank")->configure_entries(0, 8, &AUDIO[0x00000], 0x4000);
	state->membank("sndbank")->set_entry(0);
}


GAME( 1993, discoboy,  0,    discoboy, discoboy, discoboy, ROT270, "Soft Art Co.", "Disco Boy", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
