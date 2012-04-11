/***************************************************************************
IMOLA GP by Alberici

todo:
- document remaining dips
- bogus colors
- need better mappings for accelerator/shifter
- eliminate need for master-cpu communications hack
========================================
www.andys-arcade.com

Dumped by Andy Welburn on a windy and rainy day 07/07/04

Possibly has clk/dir type steering.

Shows RB BO ITALY on the title screen and is a top-down driving game,
a lot like monaco GP, it even has stages where you have headlights.
Board colour, screening, track patterns, and most importantly
component type and colour of sockets indicates to me a pcb made in
the same factory as 'Sidam' and some 'Olympia' games. There is no
manufacturer name, no game name, all i see is : AA20/80 etched
on the underside of the pcb.

I have had this pcb for a number of years, i always thought it was
some sort of pinball logic pcb so didn't treat it too well. When it
came to clearing out my boxes of junk i took another look at it, and
it was the bank of 4116 rams that made me take a closer look.

I hooked it up and saw some video on my scope, then it died.
The +12v had shorted.. Suspecting the godamn tantalum capacitors
(often short out for no reason) i found a shorted one, removed
it and away we went. It had separate H + V sync, so i loaded
a 74ls08 into a spare ic space and AND'ed the two signals to get
composite, voila, i got a stable picture. The colours aren't right,
and maybe the video isn't quite right either, but it worked enough
for me to realise i had never seen a game like it, so i dumped it.

I couldn't get any sound out of it, could be broken, or not
hooked up right, i would suspect the latter is the case.


Hardware :
==========
2x  z80's
1x  AY-3-8910
1x  8255
2   pairs of 2114 RAM (512 bytes each)
16x 4116 RAM (2k each)
4x  2716 (ROM)
12x 2708 (ROM)


ROMS layout:
------------
(add .bin to the end to get the filenames)
YC,YD,YA and YB are all 2716 eproms, everything else is 2708's.

(tabulation used here, see photo for clarity)

YC  YD      XX
YA  YB
            XC
        XI  XM
        XY  XD
        XP  XS
        XA  XE
        XR  XO

Andy Welburn
www.andys-arcade.com

Known issues:
- the 8255 is not hooked up
***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/8255ppi.h"
#include "sound/ay8910.h"


#define HLE_COM


class imolagp_state : public driver_device
{
public:
	imolagp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_slave_workram(*this, "slave_workram"){ }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_slave_workram; // used only ifdef HLE_COM

#ifdef HLE_COM
	UINT8 m_mComData[0x100];
	int   m_mComCount;

#else
	UINT8 m_mLatchedData[2];
#endif

	UINT8 m_control;
	UINT8 m_scroll;
	UINT8 m_steerlatch;
	int   m_draw_mode;
	int   m_oldsteer;

	/* devices */
	device_t *m_slavecpu;

	/* memory */
	UINT8  m_videoram[3][0x4000];

	DECLARE_WRITE8_MEMBER(transmit_data_w);
	DECLARE_READ8_MEMBER(trigger_slave_nmi_r);
	DECLARE_READ8_MEMBER(receive_data_r);
	DECLARE_WRITE8_MEMBER(imola_ledram_w);
	DECLARE_READ8_MEMBER(steerlatch_r);
	DECLARE_WRITE8_MEMBER(screenram_w);
	DECLARE_READ8_MEMBER(imola_slave_port05r);
	DECLARE_READ8_MEMBER(imola_slave_port06r);
	DECLARE_READ8_MEMBER(imola_slave_port81r);
	DECLARE_READ8_MEMBER(imola_slave_port82r);
	DECLARE_WRITE8_MEMBER(vreg_control_w);
	DECLARE_WRITE8_MEMBER(vreg_data_w);
};


#ifdef HLE_COM

WRITE8_MEMBER(imolagp_state::transmit_data_w)
{
	m_mComData[m_mComCount++] = data;
}

READ8_MEMBER(imolagp_state::trigger_slave_nmi_r)
{
	return 0;
}

READ8_MEMBER(imolagp_state::receive_data_r)
{
	return 0;
}

#else
/* the master cpu transmits data to the slave CPU one word at a time using a rapid sequence of triggered NMIs
 * the slave cpu pauses as it enters its irq, awaiting this burst of data
 */
WRITE8_MEMBER(imolagp_state::transmit_data_w)
{
	m_mLatchedData[offset] = data;
}
READ8_MEMBER(imolagp_state::trigger_slave_nmi_r)
{
	device_set_input_line(m_slave, INPUT_LINE_NMI, PULSE_LINE);
	return 0;
}

READ8_MEMBER(imolagp_state::receive_data_r)
{
	return m_mLatchedData[offset];
}
#endif

static void initialize_colors( running_machine &machine )
{
	/* optional runtime remapping of colors */
	static const UINT8 color[0x10][3] =
	{ /* wrong! need color-accurate screenshots to fix */
		{ 0x44,0x44,0x00 },
		{ 0x7f,0xff,0xff },
		{ 0xff,0x00,0x7f }, /* opp car */
		{ 0x00,0x00,0xff }, /* road(2) */
		{ 0xff,0xff,0x00 },
		{ 0x00,0x88,0x00 }, /* grass */
		{ 0x7f,0x00,0x00 },
		{ 0x00,0xff,0xff }, /* house front */
		{ 0xff,0x00,0x00 }, /* player car body */
		{ 0xff,0x00,0x00 }, /* house roof right */
		{ 0xff,0x7f,0x00 },
		{ 0xff,0x00,0xff }, /* house roof left */
		{ 0xff,0xff,0xff }, /* rescue truck trim */
		{ 0xff,0xff,0x00 }, /* roadside grass */
		{ 0xff,0xff,0xff }, /* driveway, headlight */
		{ 0xff,0xff,0xff }  /* house crease, door */
	};
	int i;
	for (i = 0; i < 0x10; i++)
	{
		palette_set_color_rgb( machine, i * 2 + 0, 0, 0, 0 );
		palette_set_color_rgb( machine, i * 2 + 1, color[i][0], color[i][1], color[i][2] );
	}
}

static VIDEO_START( imolagp )
{
	imolagp_state *state = machine.driver_data<imolagp_state>();

	memset(state->m_videoram, 0, sizeof(state->m_videoram));
	state->save_item(NAME(state->m_videoram));

	initialize_colors(machine);
}


static SCREEN_UPDATE_IND16( imolagp )
{
	imolagp_state *state = screen.machine().driver_data<imolagp_state>();
	int scroll2 = state->m_scroll ^ 0x03;
	int pass;
	for (pass = 0; pass < 2; pass++)
	{
		int i;
		const UINT8 *source = state->m_videoram[pass * 2];

		for (i = 0; i < 0x4000; i++)
		{
			int pen;
			int y = (i / 0x40);
			int x = (i & 0x3f) * 4 - scroll2;
			UINT16 *dest = &bitmap.pix16(y & 0xff);
			int data = source[i];
			if (data || pass == 0)
			{
				int color = (data & 0xf0) >> 3;
				data &= 0x0f;
				pen = ((data >> 3) & 1); dest[(x + 3) & 0xff] = color | pen;
				pen = ((data >> 2) & 1); dest[(x + 2) & 0xff] = color | pen;
				pen = ((data >> 1) & 1); dest[(x + 1) & 0xff] = color | pen;
				pen = ((data >> 0) & 1); dest[(x + 0) & 0xff] = color | pen;
			}
		}
	}
	return 0;
}

WRITE8_MEMBER(imolagp_state::imola_ledram_w)
{
	data &= 0xf;

	switch (offset)
	{
	case 0x00: output_set_value("score1000", data); break;
	case 0x01: output_set_value("score100", data); break;
	case 0x02: output_set_value("score10", data); break;
	case 0x03: output_set_value("score1", data); break;
	case 0x04: output_set_value("time10", data ); break;
	case 0x05: output_set_value("time1", data ); break;

	case 0x08: output_set_value("hs5_10", data); break;
	case 0x09: output_set_value("hs5_1", data); break;

	case 0x10: output_set_value("hs4_1000", data); break;
	case 0x11: output_set_value("hs4_100", data); break;
	case 0x12: output_set_value("hs4_10", data); break;
	case 0x13: output_set_value("hs4_1", data); break;
	case 0x14: output_set_value("hs5_1000", data); break;
	case 0x15: output_set_value("hs5_100", data); break;

	case 0x0a: output_set_value("numplays10", data ); break;
	case 0x0b: output_set_value("numplays1", data ); break;
	case 0x0c: output_set_value("credit10", data); break;
	case 0x0d: output_set_value("credit1", data); break;

	case 0x18: output_set_value("hs2_10", data); break;
	case 0x19: output_set_value("hs2_1", data); break;
	case 0x1a: output_set_value("hs3_1000", data); break;
	case 0x1b: output_set_value("hs3_100", data); break;
	case 0x1c: output_set_value("hs3_10", data); break;
	case 0x1d: output_set_value("hs3_1", data); break;

	case 0x20: output_set_value("hs1_1000", data); break;
	case 0x21: output_set_value("hs1_100", data); break;
	case 0x22: output_set_value("hs1_10", data); break;
	case 0x23: output_set_value("hs1_1", data); break;
	case 0x24: output_set_value("hs2_1000", data); break;
	case 0x25: output_set_value("hs2_100", data); break;

	default:
		break;
	}
}

READ8_MEMBER(imolagp_state::steerlatch_r)
{
	return m_steerlatch;
}

WRITE8_MEMBER(imolagp_state::screenram_w)
{ /* ?! */
	switch (m_draw_mode)
	{
	case 0x82:
	case 0x81:
	case 0x05:
		m_videoram[1][offset] = data;
		break;
	case 0x06:
		m_videoram[0][offset] = data;
		break;
	default:
		break;
	}
}

READ8_MEMBER(imolagp_state::imola_slave_port05r)
{
	memcpy(m_videoram[2], m_videoram[1], 0x4000); /* hack! capture before sprite plane is erased */
	m_draw_mode = 0x05;
	return 0;
}

READ8_MEMBER(imolagp_state::imola_slave_port06r)
{
	m_draw_mode = 0x06;
	return 0;
}

READ8_MEMBER(imolagp_state::imola_slave_port81r)
{
	m_draw_mode = 0x81;
	memcpy(m_videoram[2], m_videoram[1], 0x4000); /* hack! capture before sprite plane is erased */
	return 0;
}

READ8_MEMBER(imolagp_state::imola_slave_port82r)
{
	m_draw_mode = 0x82;
	return 0;
}

WRITE8_MEMBER(imolagp_state::vreg_control_w)
{
	m_control = data;
}

WRITE8_MEMBER(imolagp_state::vreg_data_w)
{
	switch (m_control)
	{
	case 0x0e:
		m_scroll = data;
		break;
	case 0x07: /* always 0xff? */
	case 0x0f: /* 0xff or 0x00 */
	default:
		logerror("vreg[0x%02x]:=0x%02x\n", m_control, data);
		break;
	}
}

static ADDRESS_MAP_START( readport_master, AS_IO, 8, imolagp_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(trigger_slave_nmi_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( imolagp_master, AS_PROGRAM, 8, imolagp_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x2800, 0x2800) AM_READ_PORT("2800")	/* gas */
	AM_RANGE(0x2802, 0x2802) AM_READ(steerlatch_r) AM_WRITENOP
	/*  AM_RANGE(0x2803, 0x2803) ? */
	AM_RANGE(0x3000, 0x3000) AM_WRITE(vreg_control_w)
	AM_RANGE(0x37f0, 0x37f0) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_w)
	/*  AM_RANGE(0x37f7, 0x37f7) ? */
	AM_RANGE(0x3800, 0x3800) AM_WRITE(vreg_data_w)
	AM_RANGE(0x3810, 0x3810) AM_DEVWRITE_LEGACY("aysnd", ay8910_data_w)
	AM_RANGE(0x4000, 0x4000) AM_READ_PORT("DSWA")	/* DSWA */
	AM_RANGE(0x5000, 0x50ff) AM_WRITE(imola_ledram_w)
	AM_RANGE(0x47ff, 0x4800) AM_WRITE(transmit_data_w)
	AM_RANGE(0x6000, 0x6000) AM_READ_PORT("DSWB")	/* DSWB */
ADDRESS_MAP_END

static ADDRESS_MAP_START( readport_slave, AS_IO, 8, imolagp_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x05,0x05) AM_READ(imola_slave_port05r)
	AM_RANGE(0x06,0x06) AM_READ(imola_slave_port06r)
	AM_RANGE(0x81,0x81) AM_READ(imola_slave_port81r)
	AM_RANGE(0x82,0x82) AM_READ(imola_slave_port82r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( imolagp_slave, AS_PROGRAM, 8, imolagp_state )
	AM_RANGE(0x0000, 0x03ff) AM_ROM
	AM_RANGE(0x0800, 0x0bff) AM_ROM
	AM_RANGE(0x1000, 0x13ff) AM_ROM
	AM_RANGE(0x1c00, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM AM_SHARE("slave_workram")
	AM_RANGE(0x9fff, 0xa000) AM_READ(receive_data_r)
	AM_RANGE(0xc000, 0xffff) AM_WRITE(screenram_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( imolagp )
	PORT_START("DSWA") /* 0x4000 */
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Unknown ) ) /* ? */
	PORT_DIPSETTING(    0x00, "DSWA-0" )
	PORT_DIPSETTING(    0x08, "DSWA-1" )
	PORT_DIPSETTING(    0x10, "DSWA-2" )
	PORT_DIPSETTING(    0x18, "DSWA-3" )
	PORT_DIPNAME( 0xe0, 0x00, DEF_STR( Test ) )
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x20, "TEST A" )
	PORT_DIPSETTING(    0x40, "TEST C" )
	PORT_DIPSETTING(    0x60, "TEST D" )
	PORT_DIPSETTING(    0x80, "Memory" )
	PORT_DIPSETTING(    0xa0, "Color Test" )
	PORT_DIPSETTING(    0xc0, "Grid Test" )
	PORT_DIPSETTING(    0xe0, DEF_STR( Unused ) )

	PORT_START("DSWB") /* 0x6000 */
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Unknown ) ) /* ? */
	PORT_DIPSETTING(    0x00, "DSWB-0" )
	PORT_DIPSETTING(    0x08, "DSWB-1" )
	PORT_DIPSETTING(    0x10, "DSWB-2" )
	PORT_DIPSETTING(    0x18, "DSWB-3" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "DSWB-0" )
	PORT_DIPSETTING(    0x20, "DSWB-1" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("2800") /* 2800 */ /* speed: 08 00 0F 1C 0F 00 1E 3D */
//  PORT_DIPNAME( 0x03, 0x03, "Pedal" )
//  PORT_DIPSETTING( 0x01, "STOPPED" )
//  PORT_DIPSETTING( 0x00, "SLOW" )
//  PORT_DIPSETTING( 0x02, "MEDIUM" )
//  PORT_DIPSETTING( 0x03, "FAST" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
//  PORT_DIPNAME( 0x04, 0x04, "Stick Shift" )
//  PORT_DIPSETTING( 0x0, DEF_STR(Low) )
//  PORT_DIPSETTING( 0x4, DEF_STR(High) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("2802") /* 2802 */
	PORT_BIT( 0x0f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_PLAYER(1)
INPUT_PORTS_END

/***************************************************************************/


static TIMER_DEVICE_CALLBACK ( imolagp_nmi_cb )
{
	imolagp_state *state = timer.machine().driver_data<imolagp_state>();

	{
		int newsteer = input_port_read(timer.machine(), "2802") & 0xf;
		if (newsteer != state->m_oldsteer)
		{
			if (state->m_steerlatch == 0)
				state->m_steerlatch = 0x03;
			else if ((newsteer - state->m_oldsteer) & 0x8)
			{
				state->m_steerlatch = ((state->m_steerlatch << 1) | (state->m_steerlatch >> 3)) & 0xf;
				state->m_oldsteer = (state->m_oldsteer - 1) & 0xf;
			}
			else
			{
				state->m_oldsteer = (state->m_oldsteer + 1) & 0xf;
			}
			device_set_input_line(state->m_maincpu, INPUT_LINE_NMI, PULSE_LINE);
		}
	}
}

static INTERRUPT_GEN( vblank_irq )
{
	imolagp_state *state = device->machine().driver_data<imolagp_state>();

#ifdef HLE_COM
	memcpy(&state->m_slave_workram[0x80], state->m_mComData, state->m_mComCount);
	state->m_mComCount = 0;
#endif
	device_set_input_line(device, 0, HOLD_LINE);
} /* master_interrupt */


static const ppi8255_interface ppi8255_intf =
{
	DEVCB_NULL,		/* Port A read */
	DEVCB_NULL,		/* Port B read */
	DEVCB_NULL,		/* Port C read */
	DEVCB_NULL,		/* Port A write */
	DEVCB_NULL,		/* Port B write */
	DEVCB_NULL		/* Port C write */
};


static MACHINE_START( imolagp )
{
	imolagp_state *state = machine.driver_data<imolagp_state>();

	state->m_slavecpu = machine.device("slave");

	state->save_item(NAME(state->m_control));
	state->save_item(NAME(state->m_scroll));
	state->save_item(NAME(state->m_steerlatch));
	state->save_item(NAME(state->m_draw_mode));
	state->save_item(NAME(state->m_oldsteer));
#ifdef HLE_COM
	state->save_item(NAME(state->m_mComData));
	state->save_item(NAME(state->m_mComCount));
#else
	state->save_item(NAME(state->m_mLatchedData));
#endif
}

static MACHINE_RESET( imolagp )
{
	imolagp_state *state = machine.driver_data<imolagp_state>();

	state->m_control = 0;
	state->m_scroll = 0;
	state->m_steerlatch = 0;
	state->m_draw_mode = 0;
	state->m_oldsteer = 0;
#ifdef HLE_COM
	state->m_mComCount = 0;
	memset(state->m_mComData, 0, 0x100);
#else
	state->m_mLatchedData[0] = 0;
	state->m_mLatchedData[1] = 0;
#endif
}

static MACHINE_CONFIG_START( imolagp, imolagp_state )

	MCFG_CPU_ADD("maincpu", Z80,8000000) /* ? */
	MCFG_CPU_PROGRAM_MAP(imolagp_master)
	MCFG_CPU_IO_MAP(readport_master)
	MCFG_CPU_VBLANK_INT("screen",vblank_irq)
	MCFG_TIMER_ADD_PERIODIC("pot_irq", imolagp_nmi_cb, attotime::from_hz(60*3))

	MCFG_CPU_ADD("slave", Z80,8000000) /* ? */
	MCFG_CPU_PROGRAM_MAP(imolagp_slave)
	MCFG_CPU_IO_MAP(readport_slave)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_MACHINE_START(imolagp)
	MCFG_MACHINE_RESET(imolagp)

	MCFG_PPI8255_ADD( "ppi8255", ppi8255_intf )

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(256,256)
	MCFG_SCREEN_VISIBLE_AREA(0+64-16,255,0+16,255)
	MCFG_SCREEN_UPDATE_STATIC(imolagp)

	MCFG_PALETTE_LENGTH(0x20)
	MCFG_VIDEO_START(imolagp)
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, 2000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

ROM_START( imolagp )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code */
	ROM_LOAD( "yd.bin", 0x0000, 0x800, CRC(5eb61bb7) SHA1(b897ecc7fa9aa1ae4e095d22d16a901b9d439a8e) )
	ROM_LOAD( "yc.bin", 0x0800, 0x800, CRC(f7468a3b) SHA1(af1664e30b732b3d5321e76659961af3ebeb1237) )
	ROM_LOAD( "yb.bin", 0x1000, 0x800, CRC(9f21506e) SHA1(6b46ff4815b8a02b190ec13e067f9a6687980774) )
	ROM_LOAD( "ya.bin", 0x1800, 0x800, CRC(23fbcf14) SHA1(e8b5a9b01f715356c14aa41dbc9ca26732d3a4e4) )

	ROM_REGION( 0x10000, "slave", 0 ) /* Z80 code */
	ROM_LOAD( "xx.bin", 0x0000, 0x400, CRC(059d6294) SHA1(38f075753e7a9fcabb857e5587e8a5966052cbcd) )
	ROM_LOAD( "xm.bin", 0x0800, 0x400, CRC(64ebb7de) SHA1(fc5477bbedf44e93a578a71d2ff376f6f0b51a71) ) // ? gfx: B
	ROM_LOAD( "xc.bin", 0x1000, 0x400, CRC(397fd1f3) SHA1(e6b927933847ddcdbbcbeb5e5f37fea063356b24) )
	ROM_LOAD( "xi.bin", 0x1c00, 0x400, CRC(ef54efa2) SHA1(c8464f11ccfd9eaf9aefb2cd3ac2b9e8bc2d11b6) ) // contains bitmap for "R.B."
	ROM_LOAD( "xy.bin", 0x2000, 0x400, CRC(fea8e31e) SHA1(f85eac74d32ebd28170b466b136faf21a8ab220f) )
	ROM_LOAD( "xd.bin", 0x2400, 0x400, CRC(0c601fc9) SHA1(e655f292b502a14068f5c35428001f8ceedf3637) )
	ROM_LOAD( "xs.bin", 0x2800, 0x400, CRC(5d15ac52) SHA1(b4f97854018f72e4086c7d830d1b312aea1420a7) )
	ROM_LOAD( "xa.bin", 0x2c00, 0x400, CRC(a95f5461) SHA1(2645fb93bc4ad5354eef5a385fa94021fb7291dc) ) // ? car - good?
	ROM_LOAD( "xp.bin", 0x3000, 0x400, CRC(4b6d63ef) SHA1(16f9e31e588b989f5259ab59c0a3a2c7787f3a16) ) // ? gfx: AEIOSXTDNMVGYRPL
	ROM_LOAD( "xo.bin", 0x3400, 0x400, CRC(c1d7f67c) SHA1(2ddfe9e59e323cd041fd760531b9e15ccd050058) ) // ? gfx: C
	ROM_LOAD( "xr.bin", 0x3800, 0x400, CRC(8a8667aa) SHA1(53f34b6c5327d4398de644d7f318d460da56c2de) ) // ? gfx: sign+explosion
	ROM_LOAD( "xe.bin", 0x3c00, 0x400, CRC(e0e81120) SHA1(14a77dfd069be342df4dbb1b747443c6d121d3fe) ) // ? car+misc
ROM_END


/*    YEAR, NAME,    PARENT,   MACHINE, INPUT,   INIT,    MONITOR, COMPANY,     FULLNAME */
GAME( 1981, imolagp, 0,        imolagp, imolagp, 0,       ROT90,   "Alberici", "Imola Grand Prix", GAME_WRONG_COLORS | GAME_SUPPORTS_SAVE )
