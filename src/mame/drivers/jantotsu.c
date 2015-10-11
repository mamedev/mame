// license:BSD-3-Clause
// copyright-holders:David Haywood, Angelo Salese
/*******************************************************************************************

4nin-uchi Mahjong Jantotsu (c) 1983 Sanritsu

driver by David Haywood and Angelo Salese

Notes:
-The 1st/2nd Player tiles on hand are actually shown on different screen sides.The Service
 Mode is for adjusting these screens (to not let the human opponent to read your tiles).

TODO:
-According to the flyer, color bitplanes might be wrong on the A-N mahjong charset, might be
 a BTANB however...
-I need schematics / pcb photos (component + solder sides) to understand if the background
 color is hard-wired to the DIP-Switches or there's something else wrong.

============================================================================================
Debug cheats:

c01b-c028 player-1 tiles
c02b-c038 "right" computer tiles
c03b-c048 "up" computer tiles / player-2 tiles
c04b-c058 "left" computer tiles

============================================================================================
MSM samples table (I'm assuming that F is left-right players, M2 is up / player 2 and M1 is player 1):
start |end   |sample    |possible data triggers
------------------------|----------------------
jat-43 (0x0000-0x1fff)
0x0000|0x06ff|rii'chi M1| 0x00
0x0700|0x0bff|rii'chi M2| 0x07
0x0c00|0x0fff|nagare?   |
0x1000|0x13ff|ron M     |
0x1400|0x17ff|kore?     | 0x14
0x1800|0x1bff|kan M1    | 0x18
0x1c00|0x1fff|kan M2    |
jat-42 (0x4000-0x5fff)
0x4000|0x42ff|koi?      |
0x4300|0x4ef0|ippatsu M |
0x5000|0x52ff|pon M     |
0x5300|0x57ff|tsumo M2  | 0x53
0x5800|0x5dff|rii'chi F | 0x58
jat-41 (0x8000-0x9fff)
0x8000|0x87ff|ippatsu F |
0x8800|0x8fff|ryutoku?  | 0x88
0x9000|0x95ff|otoyo?    | 0x90
0x9600|0x9eff|yatta     | 0x96
jat-40 (0xc000-0xdfff)
0xc000|0xc8ff|odaii?    | 0xc0
0xc900|0xccff|tsumo F   | 0xc9
0xcd00|0xcfff|tsumo M1  |
0xd000|0xd3ff|ron F     | 0xd0
0xd400|0xd7ff|pon F     | 0xd4
0xd800|0xdbff|kan F     |
0xdc00|0xdfff|chi       |
------------------------------------------------


============================================================================================

4nin-uchi mahjong Jantotsu
(c)1983 Sanritsu

C2-00159_A

CPU: Z80
Sound: SN76489ANx2 MSM5205
OSC: 18.432MHz

ROMs:
JAT-00.2B (2764)
JAT-01.3B
JAT-02.4B
JAT-03.2E
JAT-04.3E
JAT-05.4E

JAT-40.6B
JAT-41.7B
JAT-42.8B
JAT-43.9B

JAT-60.10P (7051) - color PROM

This game requires special control panel.
Standard mahjong control panel + these 3 buttons.
- 9syu nagare
- Continue Yes
- Continue No

dumped by sayu

*******************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "sound/msm5205.h"

#define MAIN_CLOCK XTAL_18_432MHz

class jantotsu_state : public driver_device
{
public:
	jantotsu_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_adpcm(*this, "adpcm") ,
		m_palette(*this, "palette"){ }

	/* sound-related */
	UINT32   m_adpcm_pos;
	UINT8    m_adpcm_idle;
	int      m_adpcm_data;
	UINT8    m_adpcm_trigger;

	/* misc */
	UINT8    m_mux_data;

	/* video-related */
	UINT8    m_vram_bank;
	UINT8    m_col_bank;
	UINT8    m_display_on;
	UINT8    m_bitmap[0x8000];
	DECLARE_READ8_MEMBER(jantotsu_bitmap_r);
	DECLARE_WRITE8_MEMBER(jantotsu_bitmap_w);
	DECLARE_WRITE8_MEMBER(bankaddr_w);
	DECLARE_READ8_MEMBER(jantotsu_mux_r);
	DECLARE_WRITE8_MEMBER(jantotsu_mux_w);
	DECLARE_READ8_MEMBER(jantotsu_dsw2_r);
	DECLARE_WRITE8_MEMBER(jan_adpcm_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(jantotsu);
	UINT32 screen_update_jantotsu(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(jan_adpcm_int);
	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_adpcm;
	required_device<palette_device> m_palette;
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/

void jantotsu_state::video_start()
{
	save_item(NAME(m_bitmap));
}

UINT32 jantotsu_state::screen_update_jantotsu(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x, y, i;
	int count = 0;
	UINT8 pen_i;

	if(!m_display_on)
		return 0;

	for (y = 0; y < 256; y++)
	{
		for (x = 0; x < 256; x += 8)
		{
			UINT8 color;

			for (i = 0; i < 8; i++)
			{
				color = m_col_bank;

				for(pen_i = 0;pen_i<4;pen_i++)
					color |= (((m_bitmap[count + pen_i*0x2000]) >> (7 - i)) & 1) << pen_i;

				if (cliprect.contains(x + i, y))
					bitmap.pix32(y, x + i) = m_palette->pen(color);
			}

			count++;
		}
	}

	return 0;
}

/* banked vram */
READ8_MEMBER(jantotsu_state::jantotsu_bitmap_r)
{
	return m_bitmap[offset + ((m_vram_bank & 3) * 0x2000)];
}

WRITE8_MEMBER(jantotsu_state::jantotsu_bitmap_w)
{
	m_bitmap[offset + ((m_vram_bank & 3) * 0x2000)] = data;
}

WRITE8_MEMBER(jantotsu_state::bankaddr_w)
{
	m_vram_bank = ((data & 0xc0) >> 6);

	m_display_on = (data & 2);

	/* bit 0 is unknown */
	if(data & 0x3c)
		logerror("I/O port $07 write trips %02x\n",data);
}

PALETTE_INIT_MEMBER(jantotsu_state, jantotsu)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int bit0, bit1, bit2, r, g, b;
	int i;

	for (i = 0; i < 0x20; ++i)
	{
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[0] >> 3) & 0x01;
		bit1 = (color_prom[0] >> 4) & 0x01;
		bit2 = (color_prom[0] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = (color_prom[0] >> 6) & 0x01;
		bit2 = (color_prom[0] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}

/*************************************
 *
 *  Memory handlers
 *
 *************************************/

/*Multiplexer is mapped as 6-bits reads,bits 6 & 7 are always connected to the coin mechs.*/
READ8_MEMBER(jantotsu_state::jantotsu_mux_r)
{
	const char *const portnames[] = { "PL1_1", "PL1_2", "PL1_3", "PL1_4",
										"PL2_1", "PL2_2", "PL2_3", "PL2_4" };
	UINT8 i,res;

	//  printf("%02x\n", m_mux_data);
	res = ioport("COINS")->read();

	for(i=0;i<8;i++)
	{
		if((~m_mux_data) & (1 << i))
			res |= ioport(portnames[i])->read();
	}

	return res;
}

WRITE8_MEMBER(jantotsu_state::jantotsu_mux_w)
{
	m_mux_data = data;
}

/*If bits 6 & 7 doesn't return 0x80,the game hangs until this bit is set,
  so I'm guessing that these bits can't be read by the z80 at all but directly
  hard-wired to the video chip. However I need the schematics / pcb snaps and/or
  a side-by-side test (to know if the background colors really works) to be sure. */
READ8_MEMBER(jantotsu_state::jantotsu_dsw2_r)
{
	return (ioport("DSW2")->read() & 0x3f) | 0x80;
}

WRITE8_MEMBER(jantotsu_state::jan_adpcm_w)
{
	switch (offset)
	{
		case 0:
			m_adpcm_pos = (data & 0xff) * 0x100;
			m_adpcm_idle = 0;
			m_adpcm->reset_w(0);
			/* I don't think that this will ever happen, it's there just to be sure
			   (i.e. I'll probably never do a "nagare" in my entire life ;-) ) */
			if(data & 0x20)
				popmessage("ADPCM called with data = %02x, contact MAMEdev", data);
//          printf("%02x 0\n", data);
			break;
		/*same write as port 2? MSM sample ack? */
		case 1:
//          m_adpcm_idle = 1;
//          m_adpcm->reset_w(1);
//          printf("%02x 1\n", data);
			break;
	}
}

WRITE_LINE_MEMBER(jantotsu_state::jan_adpcm_int)
{
	if (m_adpcm_pos >= 0x10000 || m_adpcm_idle)
	{
		//m_adpcm_idle = 1;
		m_adpcm->reset_w(1);
		m_adpcm_trigger = 0;
	}
	else
	{
		UINT8 *ROM = memregion("adpcm")->base();

		m_adpcm_data = ((m_adpcm_trigger ? (ROM[m_adpcm_pos] & 0x0f) : (ROM[m_adpcm_pos] & 0xf0) >> 4));
		m_adpcm->data_w(m_adpcm_data & 0xf);
		m_adpcm_trigger ^= 1;
		if (m_adpcm_trigger == 0)
		{
			m_adpcm_pos++;
			if ((ROM[m_adpcm_pos] & 0xff) == 0x70)
				m_adpcm_idle = 1;
		}
	}
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( jantotsu_map, AS_PROGRAM, 8, jantotsu_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_READWRITE(jantotsu_bitmap_r, jantotsu_bitmap_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( jantotsu_io, AS_IO, 8, jantotsu_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("DSW1") AM_DEVWRITE("sn1", sn76489a_device, write)
	AM_RANGE(0x01, 0x01) AM_READ(jantotsu_dsw2_r) AM_DEVWRITE("sn2", sn76489a_device, write)
	AM_RANGE(0x02, 0x03) AM_WRITE(jan_adpcm_w)
	AM_RANGE(0x04, 0x04) AM_READWRITE(jantotsu_mux_r, jantotsu_mux_w)
	AM_RANGE(0x07, 0x07) AM_WRITE(bankaddr_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( jantotsu )
	PORT_START("COINS")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x00, "SW1:2" )
	PORT_DIPNAME( 0x1c, 0x10, "Player vs. CPU timer decrement speed") PORT_DIPLOCATION("SW1:3,4,5") //in msecs I suppose
	PORT_DIPSETTING(    0x00, "30")
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x04, "70")
	PORT_DIPSETTING(    0x0c, "90" )
	PORT_DIPSETTING(    0x10, "110" )
	PORT_DIPSETTING(    0x18, "130" )
	PORT_DIPSETTING(    0x14, "150" )
	PORT_DIPSETTING(    0x1c, "170" )
	PORT_DIPNAME( 0xe0, 0x80, "Player vs. Player timer decrement speed" ) PORT_DIPLOCATION("SW1:6,7,8") //in msecs I suppose
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x40, "120" )
	PORT_DIPSETTING(    0x20, "140" )
	PORT_DIPSETTING(    0x60, "160" )
	PORT_DIPSETTING(    0x80, "180" )
	PORT_DIPSETTING(    0xc0, "200" )
	PORT_DIPSETTING(    0xa0, "220" )
	PORT_DIPSETTING(    0xe0, "240" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "SW2:4" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Play BGM") PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x40, "Background Color" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0xc0, "Purple" )
	PORT_DIPSETTING(    0x80, "Green" )
	PORT_DIPSETTING(    0x40, "Blue" )
	PORT_DIPSETTING(    0x00, "Black" )

	PORT_START("PL1_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Mahjong Nagare") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Continue Button Yes") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Continue Button No") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("PL1_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )

	PORT_START("PL1_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_L )

	PORT_START("PL1_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_F )

	PORT_START("PL2_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Mahjong Nagare") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Continue Button Yes") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Continue Button No") PORT_PLAYER(2)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)

	PORT_START("PL2_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)

	PORT_START("PL2_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void jantotsu_state::machine_start()
{
	save_item(NAME(m_vram_bank));
	save_item(NAME(m_mux_data));
	save_item(NAME(m_adpcm_pos));
	save_item(NAME(m_adpcm_idle));
	save_item(NAME(m_adpcm_data));
	save_item(NAME(m_adpcm_trigger));
}

void jantotsu_state::machine_reset()
{
	/*Load hard-wired background color.*/
	m_col_bank = (ioport("DSW2")->read() & 0xc0) >> 3;

	m_vram_bank = 0;
	m_mux_data = 0;
	m_adpcm_pos = 0;
	m_adpcm_idle = 1;
	m_adpcm_data = 0;
	m_adpcm_trigger = 0;
}

static MACHINE_CONFIG_START( jantotsu, jantotsu_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,MAIN_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(jantotsu_map)
	MCFG_CPU_IO_MAP(jantotsu_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", jantotsu_state,  nmi_line_pulse)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) //not accurate
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 16, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(jantotsu_state, screen_update_jantotsu)
	MCFG_SCREEN_ORIENTATION(ROT270)

	MCFG_PALETTE_ADD("palette", 0x20)
	MCFG_PALETTE_INIT_OWNER(jantotsu_state, jantotsu)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn1", SN76489A, MAIN_CLOCK/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("sn2", SN76489A, MAIN_CLOCK/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("adpcm", MSM5205, XTAL_384kHz)
	MCFG_MSM5205_VCLK_CB(WRITELINE(jantotsu_state, jan_adpcm_int))  /* interrupt function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S64_4B)  /* 6 KHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( jantotsu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jat-00.2b", 0x00000, 0x02000, CRC(bed10f86) SHA1(c5ac845b32fa295b0ff205b1401bcc071d604d6e) )
	ROM_LOAD( "jat-01.3b", 0x02000, 0x02000, CRC(cc9312e9) SHA1(b08d38cc58d92378305c015c5a001b4da072188b) )
	ROM_LOAD( "jat-02.4b", 0x04000, 0x02000, CRC(292969f1) SHA1(e7cb93b67296e84ea012fcceb72df712c7e0f135) )
	ROM_LOAD( "jat-03.2e", 0x06000, 0x02000, CRC(7452ff63) SHA1(b258674e77eee5548a065419a3092877957dec66) )
	ROM_LOAD( "jat-04.3e", 0x08000, 0x02000, CRC(734e029f) SHA1(75aa13397847b4db32c41aaa6ff2ac82f16bd7a2) )
	ROM_LOAD( "jat-05.4e", 0x0a000, 0x02000, CRC(1a725e1a) SHA1(1d39d607850f47b9389f41147d4570da8814f639) )

	ROM_REGION( 0x10000, "adpcm", ROMREGION_ERASE00 )
	ROM_LOAD( "jat-43.9b", 0x00000, 0x02000, CRC(3c1d843c) SHA1(7a836e66cad4e94916f0d80a439efde49306a0e1) )
	ROM_LOAD( "jat-42.8b", 0x04000, 0x02000, CRC(3ac3efbf) SHA1(846faea7c7c01fb7500aa33a70d4b54e878c0e41) )
	ROM_LOAD( "jat-41.7b", 0x08000, 0x02000, CRC(ce08ed71) SHA1(8554e5e7ec178f57bed5fbdd5937e3a35f72c454) )
	ROM_LOAD( "jat-40.6b", 0x0c000, 0x02000, CRC(2275253e) SHA1(64e9415faf2775c6b9ab497dce7fda8c4775192e) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "jat-60.10p", 0x00, 0x20,  CRC(65528ae0) SHA1(6e3bf27d10ec14e3c6a494667b03b68726fcff14) )
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1983, jantotsu,  0,    jantotsu, jantotsu, driver_device,  0, ROT270, "Sanritsu", "4nin-uchi Mahjong Jantotsu", MACHINE_SUPPORTS_SAVE )
