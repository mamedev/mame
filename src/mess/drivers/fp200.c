/***************************************************************************

	Casio FP-200 driver


	TODO:
	- What's the LCDC type? Custom?

***************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"

#define MAIN_CLOCK XTAL_5MHz

class fp200_state : public driver_device
{
public:
	fp200_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	UINT8 m_io_type;
	UINT8 *m_chargen;

	struct{
		UINT8 x;
		UINT8 y;
		UINT8 status;
		UINT8 *vram;
		UINT8 *attr;
	}m_lcd;
	void write_lcd_attr(UINT16 X, UINT16 Y,UINT8 data);
	void write_lcd_vram(UINT16 X, UINT16 Y,UINT8 data);

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(fp200_io_r);
	DECLARE_WRITE8_MEMBER(fp200_io_w);
	DECLARE_READ8_MEMBER(fp200_lcd_r);
	DECLARE_WRITE8_MEMBER(fp200_lcd_w);

	DECLARE_WRITE_LINE_MEMBER(sod_w);
	DECLARE_READ_LINE_MEMBER(sid_r);

protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
	virtual void palette_init();
};

void fp200_state::video_start()
{
	m_lcd.vram = auto_alloc_array_clear(machine(), UINT8, 20*64);
	m_lcd.attr = auto_alloc_array_clear(machine(), UINT8, 20*64);
}

UINT32 fp200_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	for(int y=cliprect.min_y;y<cliprect.max_y;y++)
	{
		for(int x=cliprect.min_x;x<cliprect.max_x;x++)
		{
			if(m_lcd.attr[x/8+y*20] == 0x60)
			{
				UINT8 vram,pix;

				vram = m_lcd.vram[x/8+y*20];
				pix = ((m_chargen[vram*8+(x & 7)]) >> (7-(y & 7))) & 1;
				bitmap.pix16(y,x) = pix;
			}
		}
	}

	return 0;
}


/*
[8]	SSSS --YY Status code (?) / upper part of Y address
[9] YYYY XXXX lower part of Y address / X address
*/
READ8_MEMBER(fp200_state::fp200_lcd_r)
{
	UINT8 res;

	res = 0;

	switch(offset)
	{
		case 1:
			printf("%d %d -> (L)\n",m_lcd.x,m_lcd.y);
			break;
		case 2:
			printf("%d %d -> (R)\n",m_lcd.x,m_lcd.y);
			break;
		case 8:
			res =  (m_lcd.status & 0xf) << 4;
			res |= (m_lcd.y & 0x30) >> 4;
			break;
		case 9:
			res =  (m_lcd.y & 0xf) << 4;
			res |= (m_lcd.x & 0xf);
			break;
	}


	return res;
}

void fp200_state::write_lcd_attr(UINT16 X, UINT16 Y,UINT8 data)
{
	UINT16 base_offs;

	for(int yi=0;yi<8;yi++)
	{
		base_offs = X+(Y+yi)*20;

		if(base_offs >= 20*64)
			return;

		m_lcd.attr[base_offs] = data;
	}
}

void fp200_state::write_lcd_vram(UINT16 X, UINT16 Y,UINT8 data)
{
	UINT16 base_offs;

	for(int yi=0;yi<8;yi++)
	{
		base_offs = X+(Y+yi)*20;

		if(base_offs >= 20*64)
			return;

		m_lcd.vram[base_offs] = data;
	}
}

WRITE8_MEMBER(fp200_state::fp200_lcd_w)
{
	switch(offset)
	{
		case 1:
			printf("%d %d -> %02x (%c) (L %02x)\n",m_lcd.x,m_lcd.y,data,data,m_lcd.status);
			if(m_lcd.status == 0xb)
				write_lcd_attr(m_lcd.x,m_lcd.y,data);
			else if(m_lcd.status == 1)
				write_lcd_vram(m_lcd.x,m_lcd.y,data);
			break;
		case 2:
			printf("%d %d -> %02x (%c) (R %02x)\n",m_lcd.x + 10,m_lcd.y,data,data,m_lcd.status);
			if(m_lcd.status == 0xb)
				write_lcd_attr(m_lcd.x + 10,m_lcd.y,data);
			else if(m_lcd.status == 1)
				write_lcd_vram(m_lcd.x + 10,m_lcd.y,data);
			break;
		case 8:
			m_lcd.status = (data & 0xf0) >> 4;
			if(m_lcd.status == 0x0b)
				m_lcd.y = (m_lcd.y & 0xf) | ((data & 3) << 4);
			break;
		case 9:
			m_lcd.y = (m_lcd.y & 0x30) | ((data & 0xf0) >> 4);
			m_lcd.x = data & 0xf;
			break;
	}
}


/*
Annoyingly the i/o map uses the SOD to access different devices, so we need trampolines.
SOD = 0
0x10 - 0x1f Timer control (RPC05 RTC)
0x20 - 0x2f AUTO-POWER OFF
0x40 - 0x4f Device Code
0x80 - 0xff FDD (unknown type)
SOD = 1
0x00 - 0x0f LCD control.
0x10 - 0x1f I/O control
0x20 - 0x2f Keyboard
0x40 - 0x4f MT.RS-232C control
0x80 - 0x8f Printer (Centronics)
*/
READ8_MEMBER(fp200_state::fp200_io_r)
{
	UINT8 res;

	if(m_io_type == 0)
	{
		res = 0;
		logerror("Unemulated I/O read %02x (%02x)\n",offset,m_io_type);
	}
	else
	{
		switch(offset & 0xf0)
		{
			//case 0x00: return;
			case 0x00: res = fp200_lcd_r(space, offset & 0xf); break;
			default: res = 0; logerror("Unemulated I/O read %02x (%02x)\n",offset,m_io_type); break;
		}
	}

	return res;
}

WRITE8_MEMBER(fp200_state::fp200_io_w)
{
	if(m_io_type == 0)
	{
		switch(offset & 0xf0)
		{
			default:logerror("Unemulated I/O write %02x (%02x) <- %02x\n",offset,m_io_type,data); break;
		}
	}
	else
	{
		switch(offset & 0xf0)
		{
			case 0x00: fp200_lcd_w(space, offset & 0xf,data); break;
			default:logerror("Unemulated I/O write %02x (%02x) <- %02x\n",offset,m_io_type,data); break;
		}
	}
}

static ADDRESS_MAP_START( fp200_map, AS_PROGRAM, 8, fp200_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_RAM
//  0xa000, 0xffff exp RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( fp200_io, AS_IO, 8, fp200_state )
	AM_RANGE(0x00, 0xff) AM_READWRITE(fp200_io_r,fp200_io_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( fp200 )
	/* dummy active high structure */
	PORT_START("SYSA")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	/* dummy active low structure */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
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

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ RGN_FRAC(0,1) },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	8*8
};

static GFXDECODE_START( fp200 )
	GFXDECODE_ENTRY( "chargen", 0, charlayout,     0, 1 )
GFXDECODE_END


void fp200_state::machine_start()
{
	UINT8 *raw_gfx = machine().root_device().memregion("raw_gfx")->base();
	m_chargen = memregion("chargen")->base();

	for(int i=0;i<0x800;i++)
	{
		m_chargen[i] = raw_gfx[BITSWAP16(i,15,14,13,12,11,6,5,4,3,10,9,8,7,2,1,0)];
	}
}

void fp200_state::machine_reset()
{
}


void fp200_state::palette_init()
{
	palette_set_color_rgb(machine(), 0, 0xa0, 0xa8, 0xa0);
	palette_set_color_rgb(machine(), 1, 0x30, 0x38, 0x10);
}

WRITE_LINE_MEMBER( fp200_state::sod_w )
{
	m_io_type = state;
}

READ_LINE_MEMBER( fp200_state::sid_r )
{
	/* TODO: key mods */
	return 0;
}

static I8085_CONFIG( cpu_config )
{
	DEVCB_NULL,             /* STATUS changed callback */
	DEVCB_NULL,             /* INTE changed callback */
	DEVCB_DRIVER_LINE_MEMBER(fp200_state, sid_r),   /* SID changed callback (I8085A only) */
	DEVCB_DRIVER_LINE_MEMBER(fp200_state, sod_w)    /* SOD changed callback (I8085A only) */
};

static MACHINE_CONFIG_START( fp200, fp200_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8085A,MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(fp200_map)
	MCFG_CPU_IO_MAP(fp200_io)
	MCFG_CPU_CONFIG(cpu_config)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(fp200_state, screen_update)
	MCFG_SCREEN_SIZE(20*8, 8*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 20*8-1, 0*8, 8*8-1)

	MCFG_GFXDECODE(fp200)

	MCFG_PALETTE_LENGTH(2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( fp200 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "fp200rom.bin", 0x0000, 0x8000, CRC(dba6e41b) SHA1(c694fa19172eb56585a9503997655bcf9d369c34) )

	ROM_REGION( 0x800, "raw_gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "chr.bin", 0x0000, 0x800, CRC(2e6501a5) SHA1(6186e25feabe6db851ee7d61dad11e182a6d3a4a) )

	ROM_REGION( 0x800, "chargen", ROMREGION_ERASE00 )
ROM_END

GAME( 1982, fp200,  0,   fp200,  fp200, driver_device,  0,       ROT0, "Casio",      "FP-200", GAME_IS_SKELETON )
