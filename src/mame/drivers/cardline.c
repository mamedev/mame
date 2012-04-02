
/************************************
 Card Line
 driver by Tomasz Slanina
 analog[at]op[dot]pl


 SIEMENS 80C32 (main cpu)
 MC6845P
 GM76C88 x3 (8K x 8 RAM)
 K-665 9546 (OKI 6295)
 STARS B2072 9629 (qfp ASIC)
 XTAL 12 MHz
 XTAL  4 MHz

***********************************/


#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/okim6295.h"

#include "cardline.lh"


class cardline_state : public driver_device
{
public:
	cardline_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_video;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	int m_var;
};



#define DRAW_TILE(machine, offset, transparency) drawgfx_transpen(bitmap, cliprect, (machine).gfx[0],\
					(state->m_videoram[index+offset] | (state->m_colorram[index+offset]<<8))&0x3fff,\
					(state->m_colorram[index+offset]&0x80)>>7,\
					0,0,\
					x<<3, y<<3,\
					transparency?transparency:(UINT32)-1);

static SCREEN_UPDATE_IND16( cardline )
{
	cardline_state *state = screen.machine().driver_data<cardline_state>();
	int x,y;
	bitmap.fill(0, cliprect);
	for(y=0;y<32;y++)
	{
		for(x=0;x<64;x++)
		{
			int index=y*64+x;
			if(state->m_video&1)
			{
				DRAW_TILE(screen.machine(),0,0);
				DRAW_TILE(screen.machine(),0x800,1);
			}

			if(state->m_video&2)
			{
				DRAW_TILE(screen.machine(),0x1000,0);
				DRAW_TILE(screen.machine(),0x1800,1);
			}
		}
	}
	return 0;
}

static WRITE8_HANDLER(vram_w)
{
	cardline_state *state = space->machine().driver_data<cardline_state>();
	offset+=0x1000*((state->m_video&2)>>1);
	state->m_videoram[offset]=data;
}

static WRITE8_HANDLER(attr_w)
{
	cardline_state *state = space->machine().driver_data<cardline_state>();
	offset+=0x1000*((state->m_video&2)>>1);
	state->m_colorram[offset]=data;
}

static WRITE8_HANDLER(video_w)
{
	cardline_state *state = space->machine().driver_data<cardline_state>();
	state->m_video=data;
}

static READ8_HANDLER(unk_r)
{
	cardline_state *state = space->machine().driver_data<cardline_state>();
	state->m_var^=0x10;
	//printf("var %d\n",state->m_var);
	return state->m_var;
}

static WRITE8_HANDLER(lamps_w)
{
	/* button lamps 1-8 (collect, card 1-5, bet, start) */
	output_set_lamp_value(5,(data >> 0) & 1);
	output_set_lamp_value(0,(data >> 1) & 1);
	output_set_lamp_value(1,(data >> 2) & 1);
	output_set_lamp_value(2,(data >> 3) & 1);
	output_set_lamp_value(3,(data >> 4) & 1);
	output_set_lamp_value(4,(data >> 5) & 1);
	output_set_lamp_value(6,(data >> 6) & 1);
	output_set_lamp_value(7,(data >> 7) & 1);
}

static ADDRESS_MAP_START( mem_prg, AS_PROGRAM, 8, cardline_state )
	AM_RANGE(0x0000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mem_io, AS_IO, 8, cardline_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2003, 0x2003) AM_READ_PORT("IN0")
	AM_RANGE(0x2005, 0x2005) AM_READ_PORT("IN1")
	AM_RANGE(0x2006, 0x2006) AM_READ_PORT("DSW")
	AM_RANGE(0x2007, 0x2007) AM_WRITE_LEGACY(lamps_w)
	AM_RANGE(0x2008, 0x2008) AM_NOP
	AM_RANGE(0x2080, 0x213f) AM_NOP
	AM_RANGE(0x2400, 0x2400) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x2800, 0x2801) AM_NOP
	AM_RANGE(0x2840, 0x2840) AM_NOP
	AM_RANGE(0x2880, 0x2880) AM_NOP
	AM_RANGE(0x3003, 0x3003) AM_NOP
	AM_RANGE(0xc000, 0xdfff) AM_WRITE_LEGACY(vram_w) AM_BASE(m_videoram)
	AM_RANGE(0xe000, 0xffff) AM_WRITE_LEGACY(attr_w) AM_BASE(m_colorram)
	/* Ports */
	AM_RANGE(MCS51_PORT_P1, MCS51_PORT_P1) AM_READWRITE_LEGACY(unk_r, video_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( cardline )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Collect")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Card 1 / Double-Up")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Card 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Card 3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Card 4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Card 5 / Winning Plan")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Bet")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Unknown1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bookkeeping Info") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_L) PORT_NAME("Payout 2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Unknown2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Payout")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Unknown3")

	PORT_START("DSW")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf5, IP_ACTIVE_HIGH, IPT_SPECIAL ) // h/w status bits
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7  },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*8*8
};

static GFXDECODE_START( cardline )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 2 )
GFXDECODE_END

static PALETTE_INIT(cardline)
{
	int i,r,g,b,data;
	int bit0,bit1,bit2;
	for (i = 0;i < machine.total_colors();i++)
	{
		data=color_prom[i];

		/* red component */
		bit0 = (data >> 5) & 0x01;
		bit1 = (data >> 6) & 0x01;
		bit2 = (data >> 7) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (data >> 2) & 0x01;
		bit1 = (data >> 3) & 0x01;
		bit2 = (data >> 4) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (data >> 0) & 0x01;
		bit1 = (data >> 1) & 0x01;
		b = 0x55 * bit0 + 0xaa * bit1;
		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

static MACHINE_CONFIG_START( cardline, cardline_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80C32,12000000)
	MCFG_CPU_PROGRAM_MAP(mem_prg)
	MCFG_CPU_IO_MAP(mem_io)
	//MCFG_CPU_VBLANK_INT("screen", irq1_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 35*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_STATIC(cardline)

	MCFG_GFXDECODE(cardline)
	MCFG_PALETTE_LENGTH(512)
	MCFG_PALETTE_INIT(cardline)

	MCFG_DEFAULT_LAYOUT(layout_cardline)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( cardline )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dns0401.u23",   0x0000, 0x10000, CRC(5bbaf5c1) SHA1(70972a744c5981b01a46799a7fd1b0a600489264) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "u38cll01.u38",   0x000001, 0x80000, CRC(12f62496) SHA1(b89eaf09e76c5c42588bf9c8c23190347635cc83) )
	ROM_LOAD16_BYTE( "u39cll01.u39",   0x000000, 0x80000, CRC(fcfa703e) SHA1(9230ad9df02140f3a6c38b24558548a888b23412) )

	ROM_REGION( 0x40000,  "oki", 0 ) // OKI samples
	ROM_LOAD( "3a.u3",   0x0000, 0x40000, CRC(9fa543c5) SHA1(a22396cb341ca4a3f0dd23719620a219c91e0e9d) )

	ROM_REGION( 0x0200,  "proms", 0 )
	ROM_LOAD( "82s147.u33",   0x0000, 0x0200, CRC(a3b95911) SHA1(46850ea38950cdccbc2ad91d968218ac964c0eb5) )

ROM_END

GAME( 199?, cardline,  0,       cardline,  cardline,  0, ROT0, "Veltmeijer", "Card Line" , 0)
