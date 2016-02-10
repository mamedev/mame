// license:BSD-3-Clause
// copyright-holders:David Haywood, Angelo Salese
/******************************************************************************

Hit Poker (c) 1997 Accept LTD

preliminary driver by Angelo Salese & David Haywood
Many thanks to Olivier Galibert for the CPU identify effort ;-)

TODO:
- CPU core bugs?
- Protection controls inputs;
- Understand & fix EEPROM emulation;
- Hangs during attract mode, eeprom or protection?
- complete video HW (unknown bits and hblank);
- 24Khz monitor isn't supported, it changes the resolution to 648 x 480 and
  changes the register 9 (raster lines x character lines) from 7 to 0xf.
- sound (I've heard something reasonable during tests, so it could be OK);

================================================================================

'Hit Poker'?

cpu hd46505SP (HD6845SP) <- ha, ha, ha... --"

other : ZC407615CFN (infralink)

chrystal : no idea

ram km6264BL X3
TMM 2018 X2
DALLAS REAL TIME CLK DS17487-5
SOUND YM2149F
DIP 1X4

============================================================================

Some debug tricks (let's test this CPU as more as possible):
- set a bp at 1185, the "bad crc 000" msg is caused by this routine.
  (kludged to work)
- set a bp at 121f then pc=1223
- set a bp at 3a50 then pc+=2, it should now enter into attract mode

***************************************************************************/


#include "emu.h"
#include "cpu/mc68hc11/mc68hc11.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
#include "machine/nvram.h"


class hitpoker_state : public driver_device
{
public:
	hitpoker_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_sys_regs(*this, "sys_regs"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	required_shared_ptr<UINT8> m_sys_regs;

	UINT8 m_pic_data;
	std::unique_ptr<UINT8[]> m_videoram;
	std::unique_ptr<UINT8[]> m_paletteram;
	std::unique_ptr<UINT8[]> m_colorram;
	UINT8 m_eeprom_data[0x1000];
	UINT16 m_eeprom_index;

	DECLARE_READ8_MEMBER(hitpoker_vram_r);
	DECLARE_WRITE8_MEMBER(hitpoker_vram_w);
	DECLARE_READ8_MEMBER(hitpoker_cram_r);
	DECLARE_WRITE8_MEMBER(hitpoker_cram_w);
	DECLARE_READ8_MEMBER(hitpoker_paletteram_r);
	DECLARE_WRITE8_MEMBER(hitpoker_paletteram_w);
	DECLARE_READ8_MEMBER(rtc_r);
	DECLARE_WRITE8_MEMBER(eeprom_offset_w);
	DECLARE_WRITE8_MEMBER(eeprom_w);
	DECLARE_READ8_MEMBER(eeprom_r);
	DECLARE_READ8_MEMBER(hitpoker_pic_r);
	DECLARE_WRITE8_MEMBER(hitpoker_pic_w);
	DECLARE_DRIVER_INIT(hitpoker);
	virtual void video_start() override;
	UINT32 screen_update_hitpoker(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(hitpoker_irq);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


#define CRTC_CLOCK XTAL_3_579545MHz

void hitpoker_state::video_start()
{
	m_videoram = std::make_unique<UINT8[]>(0x35ff);
	m_paletteram = std::make_unique<UINT8[]>(0x1000);
	m_colorram = std::make_unique<UINT8[]>(0x2000);
}

UINT32 hitpoker_state::screen_update_hitpoker(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int count = 0;
	int y,x;

	bitmap.fill(0, cliprect);

	for (y=0;y<31;y++)
	{
		for (x=0;x<81;x++) //it's probably 80 + 1 global line attribute at the start of each line
		{
			int tile,color,gfx_bpp;

			tile = (((m_videoram[count]<<8)|(m_videoram[count+1])) & 0x3fff);
			gfx_bpp = (m_colorram[count] & 0x80)>>7; //flag between 4 and 8 bpp
			color = gfx_bpp ? ((m_colorram[count] & 0x70)>>4) : (m_colorram[count] & 0xf);

			m_gfxdecode->gfx(gfx_bpp)->opaque(bitmap,cliprect,tile,color,0,0,x*8,y*8);

			count+=2;
		}
	}

	return 0;
}

READ8_MEMBER(hitpoker_state::hitpoker_vram_r)
{
	UINT8 *ROM = memregion("maincpu")->base();

	if(m_pic_data & 0x10)
		return m_videoram[offset];
	else
		return ROM[offset+0x8000];
}

WRITE8_MEMBER(hitpoker_state::hitpoker_vram_w)
{
//  UINT8 *ROM = memregion("maincpu")->base();

//  if(m_sys_regs[0x00] & 0x10)
	m_videoram[offset] = data;
}

READ8_MEMBER(hitpoker_state::hitpoker_cram_r)
{
	UINT8 *ROM = memregion("maincpu")->base();

	if(m_pic_data & 0x10)
		return m_colorram[offset];
	else
		return ROM[offset+0xc000];
}

WRITE8_MEMBER(hitpoker_state::hitpoker_cram_w)
{
	m_colorram[offset] = data;
}

READ8_MEMBER(hitpoker_state::hitpoker_paletteram_r)
{
	UINT8 *ROM = memregion("maincpu")->base();

	if(m_pic_data & 0x10)
		return m_paletteram[offset];
	else
		return ROM[offset+0xe000];
}

WRITE8_MEMBER(hitpoker_state::hitpoker_paletteram_w)
{
	int r,g,b,datax;
	m_paletteram[offset] = data;
	offset>>=1;
	datax=256*m_paletteram[offset*2]+m_paletteram[offset*2+1];

	/* RGB565 */
	b = ((datax)&0xf800)>>11;
	g = ((datax)&0x07e0)>>5;
	r = ((datax)&0x001f)>>0;

	m_palette->set_pen_color(offset, pal5bit(r), pal6bit(g), pal5bit(b));
}

READ8_MEMBER(hitpoker_state::rtc_r)
{
	return 0x80; //kludge it for now
}


/* tests 0x180, what EEPROM is this one??? it seems to access up to 4KB */
WRITE8_MEMBER(hitpoker_state::eeprom_offset_w)
{
	if (offset == 0)
		m_eeprom_index = (m_eeprom_index & 0xf00) | (data & 0xff);
	else
		m_eeprom_index = (m_eeprom_index & 0x0ff) | (data << 8 & 0xf00);
}

WRITE8_MEMBER(hitpoker_state::eeprom_w)
{
	// is 0xbe53 the right address?
	m_eeprom_data[m_eeprom_index] = data;
}

READ8_MEMBER(hitpoker_state::eeprom_r)
{
	/*** hack to make it boot ***/
	int ret = ((m_eeprom_index & 0x1f) == 0x1f) ? 1 : 0;
	m_eeprom_index++;
	return ret;
	/*** ***/

	// FIXME: never executed
	//return m_eeprom_data[m_eeprom_index & 0xfff];
}

READ8_MEMBER(hitpoker_state::hitpoker_pic_r)
{
//  logerror("R\n");

	if(offset == 0)
	{
		if(space.device().safe_pc() == 0x3143 ||
			space.device().safe_pc() == 0x314e ||
			space.device().safe_pc() == 0x3164 ||
			space.device().safe_pc() == 0x3179)
			return m_pic_data;

		return (m_pic_data & 0x7f) | (m_pic_data & 0x40 ? 0x80 : 0x00);
	}

	return m_sys_regs[offset];
}

WRITE8_MEMBER(hitpoker_state::hitpoker_pic_w)
{
	if(offset == 0)
		m_pic_data = (data & 0xff);// | (data & 0x40) ? 0x80 : 0x00;
//  logerror("%02x W\n",data);
	m_sys_regs[offset] = data;
}

#if 0
READ8_MEMBER(hitpoker_state::test_r)
{
	return machine().rand();
}
#endif

/* overlap empty rom addresses */
static ADDRESS_MAP_START( hitpoker_map, AS_PROGRAM, 8, hitpoker_state )
	AM_RANGE(0x0000, 0x00ff) AM_RAM // stack ram
	AM_RANGE(0x1000, 0x103f) AM_RAM // internal I/O
	AM_RANGE(0x8000, 0xb5ff) AM_READWRITE(hitpoker_vram_r,hitpoker_vram_w)
	AM_RANGE(0xb600, 0xbdff) AM_RAM
	AM_RANGE(0xbe0a, 0xbe0a) AM_READ_PORT("IN0")
	AM_RANGE(0xbe0c, 0xbe0c) AM_READ_PORT("IN2") //irq ack?
	AM_RANGE(0xbe0d, 0xbe0d) AM_READ(rtc_r)
	AM_RANGE(0xbe0e, 0xbe0e) AM_READ_PORT("IN1")
	AM_RANGE(0xbe50, 0xbe51) AM_WRITE(eeprom_offset_w)
	AM_RANGE(0xbe53, 0xbe53) AM_READWRITE(eeprom_r, eeprom_w)
	AM_RANGE(0xbe80, 0xbe80) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xbe81, 0xbe81) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0xbe90, 0xbe91) AM_DEVREADWRITE("aysnd", ay8910_device, data_r, address_data_w)
	AM_RANGE(0xbea0, 0xbea0) AM_READ_PORT("VBLANK") //probably other bits as well
//  AM_RANGE(0xbe00, 0xbeff) AM_READ(test_r)
	AM_RANGE(0xc000, 0xdfff) AM_READWRITE(hitpoker_cram_r,hitpoker_cram_w)
	AM_RANGE(0xe000, 0xefff) AM_READWRITE(hitpoker_paletteram_r,hitpoker_paletteram_w)
	AM_RANGE(0x0000, 0xbdff) AM_ROM
	AM_RANGE(0xbf00, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( hitpoker_io, AS_IO, 8, hitpoker_state )
	AM_RANGE(MC68HC11_IO_PORTA, MC68HC11_IO_PORTA) AM_READWRITE(hitpoker_pic_r,hitpoker_pic_w) AM_SHARE("sys_regs")
ADDRESS_MAP_END

static INPUT_PORTS_START( hitpoker )
	PORT_START("VBLANK")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) //resets the game?
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) //"permanent ram initialized" if combined with the reset switch
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "H-Blank" ) //scanline counter probably
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Monitor" ) //a JP probably
	PORT_DIPSETTING(    0x40, "15KHz" )
	PORT_DIPSETTING(    0x00, "24KHz" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout hitpoker_layout_4bpp =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,4,8,12 },
	{ 0,1,2,3,16,17,18,19 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static const gfx_layout hitpoker_layout_8bpp =
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{ RGN_FRAC(1,2)+0,RGN_FRAC(1,2)+4,RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+12,0,4,8,12 },
	{ 0,1,2,3,16,17,18,19 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static GFXDECODE_START( hitpoker )
	GFXDECODE_ENTRY( "gfx1", 0, hitpoker_layout_4bpp,   0, 0x100  )
	GFXDECODE_ENTRY( "gfx1", 0, hitpoker_layout_8bpp,   0, 8  )
GFXDECODE_END

INTERRUPT_GEN_MEMBER(hitpoker_state::hitpoker_irq)
{
	device.execute().set_input_line(MC68HC11_IRQ_LINE, HOLD_LINE);
}

static MACHINE_CONFIG_START( hitpoker, hitpoker_state )
	MCFG_CPU_ADD("maincpu", MC68HC11,1000000)
	MCFG_CPU_PROGRAM_MAP(hitpoker_map)
	MCFG_CPU_IO_MAP(hitpoker_io)
	MCFG_MC68HC11_CONFIG(0, 0x100, 0x01)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", hitpoker_state,  hitpoker_irq)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not accurate
	MCFG_SCREEN_SIZE(648, 480) //setted by the CRTC
	MCFG_SCREEN_VISIBLE_AREA(0, 648-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(hitpoker_state, screen_update_hitpoker)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_MC6845_ADD("crtc", H46505, "screen", CRTC_CLOCK/2)  /* hand tuned to get ~60 fps */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", hitpoker)
	MCFG_PALETTE_ADD("palette", 0x800)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", YM2149, 1500000)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW2"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(hitpoker_state,hitpoker)
{
	UINT8 *ROM = memregion("maincpu")->base();

	// init nvram
	machine().device<nvram_device>("nvram")->set_base(m_eeprom_data, sizeof(m_eeprom_data));

	ROM[0x1220] = 0x01; //patch eeprom write?
	ROM[0x1221] = 0x01;
	ROM[0x1222] = 0x01;

	ROM[0x10c6] = 0x01;
	ROM[0x10c7] = 0x01; //patch the checksum routine
}

ROM_START( hitpoker )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u4.bin",         0x00000, 0x10000, CRC(0016497a) SHA1(017320bfe05fea8a48e26a66c0412415846cee7c) )

	ROM_REGION( 0x10000, "pic", 0 )
	ROM_LOAD( "pic",            0x00000, 0x1000, NO_DUMP ) // unknown type

	ROM_REGION( 0x100000, "gfx1", 0 ) // tile 0x4c8 seems to contain something non-gfx related, could be tilemap / colour data, check!
	ROM_LOAD16_BYTE( "u42.bin",         0x00001, 0x40000, CRC(cbe56fec) SHA1(129bfd10243eaa7fb6a087f96de90228e6030353) )
	ROM_LOAD16_BYTE( "u43.bin",         0x00000, 0x40000, CRC(6c0d4283) SHA1(04a4fd82f5cc0ed9f548e490ac67d287227073c3) )
	ROM_LOAD16_BYTE( "u44.bin",         0x80001, 0x40000, CRC(e23d5f30) SHA1(ca8855301528aa4eeff40cb820943b4268f8596e) ) // the 'adult images' are 8bpp
	ROM_LOAD16_BYTE( "u45.bin",         0x80000, 0x40000, CRC(e65b3e52) SHA1(c0c1a360a4a1823bf71c0a4105ff41f4102862e8) ) //  the first part of these 2 is almost empty as the standard gfx are 4bpp
ROM_END

GAME( 1997, hitpoker,  0,    hitpoker, hitpoker, hitpoker_state,  hitpoker, ROT0, "Accept Ltd.", "Hit Poker (Bulgaria)", MACHINE_NOT_WORKING )
