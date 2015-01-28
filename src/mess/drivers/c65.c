// license: MAME
// copyright-holders: Angelo Salese
/***************************************************************************

C=65 / C=64DX (c) 1991 Commodore

Attempt at rewriting the driver ...

TODO:
- I need to subtract border color to -1 in order to get blue color (-> register is 6 and blue color is 5 in palette array).
  Also top-left logo seems to draw wrong palette for entries 4,5,6,7. CPU core bug?

Note:
- VIC-4567 will be eventually be added via compile switch, once that I
  get the hang of the system (and checking where the old code fails 
  eventually)

***************************************************************************/


#include "emu.h"
#include "cpu/m6502/m4510.h"

#define MAIN_CLOCK XTAL_3_5MHz

class c65_state : public driver_device
{
public:
	c65_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_screen(*this, "screen"),
			m_palette(*this, "palette"),
			m_workram(*this, "wram"),
			m_palred(*this, "redpal"),
			m_palgreen(*this, "greenpal"),
			m_palblue(*this, "bluepal"),
			m_dmalist(*this, "dmalist"),
			m_cram(*this, "cram"),
			m_gfxdecode(*this, "gfxdecode")
	{ }

	// devices
	required_device<m4510_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT8> m_workram;
	required_shared_ptr<UINT8> m_palred;
	required_shared_ptr<UINT8> m_palgreen;
	required_shared_ptr<UINT8> m_palblue;
	required_shared_ptr<UINT8> m_dmalist;
	required_shared_ptr<UINT8> m_cram;
	required_device<gfxdecode_device> m_gfxdecode;

	UINT8 *m_iplrom;

	
	DECLARE_READ8_MEMBER(vic4567_dummy_r);
	DECLARE_WRITE8_MEMBER(vic4567_dummy_w);
	DECLARE_WRITE8_MEMBER(PalRed_w);
	DECLARE_WRITE8_MEMBER(PalGreen_w);
	DECLARE_WRITE8_MEMBER(PalBlue_w);
	DECLARE_WRITE8_MEMBER(DMAgic_w);
	DECLARE_READ8_MEMBER(CIASelect_r);
	DECLARE_WRITE8_MEMBER(CIASelect_w);
	
	DECLARE_READ8_MEMBER(dummy_r);
	
	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_PALETTE_INIT(c65);
	DECLARE_DRIVER_INIT(c65);
	DECLARE_DRIVER_INIT(c65pal);
	
	INTERRUPT_GEN_MEMBER(vic3_vblank_irq);
protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
private:
	UINT8 m_VIC2_IRQPend, m_VIC2_IRQMask;
	/* 0x20: border color (TODO: different thread?) */
	UINT8 m_VIC2_EXTColor;
	/* 0x30: banking + PAL + EXT SYNC */
	UINT8 m_VIC3_ControlA;
	/* 0x31: video modes */
	UINT8 m_VIC3_ControlB;
	void PalEntryFlush(UINT8 offset);
	void DMAgicExecute(address_space &space,UINT32 address);
	int inner_x_char(int xoffs);
	int inner_y_char(int yoffs);
};

void c65_state::video_start()
{
}

// TODO: inline?
int c65_state::inner_x_char(int xoffs)
{
	return xoffs>>3;
}

int c65_state::inner_y_char(int yoffs)
{
	return yoffs>>3;
}

UINT32 c65_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int y,x;
	int border_color = m_VIC2_EXTColor & 0xf;
	
	// TODO: border area
	for(y=0;y<m_screen->height();y++)
	{
		for(x=0;x<m_screen->width();x++)
		{
			//int, xi,yi,xm,ym,dot_x;
			int xi = inner_x_char(x);
			int yi = inner_y_char(y);
			int xm = 7 - (x & 7);
			int ym = (y & 7);
			UINT8 tile = m_workram[xi+yi*80+0x800];
			UINT8 attr = m_cram[xi+yi*80];
			if(attr & 0xf0)
				attr = machine().rand() & 0xf;
			
			int enable_dot = ((m_iplrom[(tile<<3)+ym+0xd000] >> xm) & 1);
						
			//if(cliprect.contains(x, y))
			bitmap.pix16(y, x) = m_palette->pen((enable_dot) ? attr & 0xf : border_color);

			
			//gfx->opaque(bitmap,cliprect,tile,0,0,0,x*8,y*8);
		}
	}

	return 0;
}

READ8_MEMBER(c65_state::vic4567_dummy_r)
{
	UINT8 res;

	res=0xff;
	switch(offset)
	{
		case 0x11:
			res = (m_screen->vpos() & 0x100) >> 1;
			return res;
		case 0x12:
			res = (m_screen->vpos() & 0xff);
			return res;
		case 0x20:
			return m_VIC2_EXTColor;
		case 0x19:
			return m_VIC2_IRQPend;
		case 0x30:
			return m_VIC3_ControlA;
		case 0x31:
			return m_VIC3_ControlB;
	}
	
	if(!space.debugger_access())
		printf("%02x\n",offset); // TODO: PC
	return res;
}

WRITE8_MEMBER(c65_state::vic4567_dummy_w)
{
	switch(offset)
	{
		case 0x19:
			m_VIC2_IRQPend = data & 0x8f;
			break;
		case 0x1a:
			m_VIC2_IRQMask = data & 0xf;
			break;
		case 0x20:
			m_VIC2_EXTColor = data & 0xf;
			break;
		/* KEY register, handles vic-iii and vic-ii modes via two consecutive writes 
		  0xa5 -> 0x96 vic-iii mode
          any other write vic-ii mode
		  */
		//case 0x2f: break;
		case 0x30: 
			if((data & 0xfe) != 0x64)
				printf("CONTROL A %02x\n",data); 
			m_VIC3_ControlA = data;
			break;
		case 0x31:
			printf("CONTROL B %02x\n",data); 
			m_VIC3_ControlB = data;
			break;
		default:
			if(!space.debugger_access())
				printf("%02x %02x\n",offset,data); 
			break;
	}

}

void c65_state::PalEntryFlush(UINT8 offset)
{
	m_palette->set_pen_color(offset, pal4bit(m_palred[offset]), pal4bit(m_palgreen[offset]), pal4bit(m_palblue[offset]));
}

WRITE8_MEMBER(c65_state::PalRed_w)
{
	m_palred[offset] = data;
	PalEntryFlush(offset);
}

WRITE8_MEMBER(c65_state::PalGreen_w)
{
	m_palgreen[offset] = data;
	PalEntryFlush(offset);
}

WRITE8_MEMBER(c65_state::PalBlue_w)
{
	m_palblue[offset] = data;
	PalEntryFlush(offset);
}

void c65_state::DMAgicExecute(address_space &space,UINT32 address)
{
	UINT8 cmd;// = space.read_byte(address++);
	UINT16 length; //= space.read_byte(address++);
	UINT32 src, dst;
	static const char *const dma_cmd_string[] =
	{
		"COPY",                 // 0
		"MIX",
		"SWAP",
		"FILL"
	};
	cmd = space.read_byte(address++);
	length = space.read_byte(address++);
	length|=(space.read_byte(address++)<<8);
	src = space.read_byte(address++);
	src|=(space.read_byte(address++)<<8);
	src|=(space.read_byte(address++)<<16);
	dst = space.read_byte(address++);
	dst|=(space.read_byte(address++)<<8);
	dst|=(space.read_byte(address++)<<16);

	if(cmd & 0xfc)
		printf("%02x\n",cmd & 0xfc);
	switch(cmd & 3)
	{
		case 0: // copy - TODO: untested
		{
				if(length != 1)
					printf("DMAgic %s %02x -> %08x %04x (CHAIN=%s)\n",dma_cmd_string[cmd & 3],src,dst,length,cmd & 4 ? "yes" : "no");
				UINT32 SourceIndex;
				UINT32 DestIndex;
				UINT16 SizeIndex;
				SourceIndex = src & 0xfffff;
				DestIndex = dst & 0xfffff;
				SizeIndex = length;
				do
				{
					space.write_byte(DestIndex++,space.read_byte(SourceIndex++));
					SizeIndex--;
				}while(SizeIndex != 0);

			return;
		}
		case 3: // fill
			{
				/* TODO: upper bits of source */
				printf("DMAgic %s %02x -> %08x %04x (CHAIN=%s)\n",dma_cmd_string[cmd & 3],src & 0xff,dst,length,cmd & 4 ? "yes" : "no");
				UINT8 FillValue;
				UINT32 DestIndex;
				UINT16 SizeIndex;
				FillValue = src & 0xff;
				DestIndex = dst & 0xfffff;
				SizeIndex = length;
				do
				{
					space.write_byte(DestIndex++,FillValue);
					SizeIndex--;
				}while(SizeIndex != 0);
			}
			return;
	}
	printf("DMAgic %s %08x %08x %04x (CHAIN=%s)\n",dma_cmd_string[cmd & 3],src,dst,length,cmd & 4 ? "yes" : "no");
}


WRITE8_MEMBER(c65_state::DMAgic_w)
{
	m_dmalist[offset] = data;
	if(offset == 0)
		DMAgicExecute(space,(m_dmalist[0])|(m_dmalist[1]<<8)|(m_dmalist[2]<<16));
}

READ8_MEMBER(c65_state::CIASelect_r)
{
	if(m_VIC3_ControlA & 1)
		return m_cram[offset];
	else
	{
		// CIA at 0xdc00
	}

	return 0xff;
}

WRITE8_MEMBER(c65_state::CIASelect_w)
{
	if(m_VIC3_ControlA & 1)
		m_cram[offset] = data;
	else
	{
		// CIA at 0xdc00
	}
	
}

READ8_MEMBER(c65_state::dummy_r)
{
	return 0;
}

static ADDRESS_MAP_START( c65_map, AS_PROGRAM, 8, c65_state )
	AM_RANGE(0x00000, 0x07fff) AM_RAM AM_SHARE("wram") // TODO: bank
	AM_RANGE(0x0c800, 0x0cfff) AM_ROM AM_REGION("maincpu", 0xc800)
	AM_RANGE(0x0d000, 0x0d07f) AM_READWRITE(vic4567_dummy_r,vic4567_dummy_w) // 0x0d000, 0x0d07f VIC-4567
	AM_RANGE(0x0d080, 0x0d081) AM_READ(dummy_r) // 0x0d080, 0x0d09f FDC
	// 0x0d0a0, 0x0d0ff Ram Expansion Control (REC)
	AM_RANGE(0x0d100, 0x0d1ff) AM_RAM_WRITE(PalRed_w) AM_SHARE("redpal")// 0x0d100, 0x0d1ff Red Palette
	AM_RANGE(0x0d200, 0x0d2ff) AM_RAM_WRITE(PalGreen_w) AM_SHARE("greenpal") // 0x0d200, 0x0d2ff Green Palette
	AM_RANGE(0x0d300, 0x0d3ff) AM_RAM_WRITE(PalBlue_w) AM_SHARE("bluepal") // 0x0d300, 0x0d3ff Blue Palette
	// 0x0d400, 0x0d4*f Right SID
	// 0x0d440, 0x0d4*f Left  SID
	// 0x0d600, 0x0d6** UART
	AM_RANGE(0x0d700, 0x0d702) AM_WRITE(DMAgic_w) AM_SHARE("dmalist") // 0x0d700, 0x0d7** DMAgic
	//AM_RANGE(0x0d703, 0x0d703) AM_READ(DMAgic_r)
	// 0x0d800, 0x0d8** Color matrix
	AM_RANGE(0x0d800, 0x0dfff) AM_READWRITE(CIASelect_r,CIASelect_w) AM_SHARE("cram")
	// 0x0dc00, 0x0dc** CIA-1
	// 0x0dd00, 0x0dd** CIA-2
	// 0x0de00, 0x0de** Ext I/O Select 1
	// 0x0df00, 0x0df** Ext I/O Select 2 (RAM window?)
	AM_RANGE(0x0e000, 0x0ffff) AM_ROM AM_REGION("maincpu",0x0e000)
	AM_RANGE(0x10000, 0x1f7ff) AM_RAM 
	AM_RANGE(0x1f800, 0x1ffff) AM_RAM // VRAM attributes
	AM_RANGE(0x20000, 0x3ffff) AM_ROM AM_REGION("maincpu",0)
ADDRESS_MAP_END



static INPUT_PORTS_START( c65 )
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


void c65_state::machine_start()
{
	m_iplrom = memregion("maincpu")->base();

	save_pointer(NAME(m_cram.target()), 0x800);
}

void c65_state::machine_reset()
{
}


PALETTE_INIT_MEMBER(c65_state, c65)
{
	// TODO: initial state? 
}

static const gfx_layout charlayout =
{
	8,8,
	0x1000/8,
	1,
	{ RGN_FRAC(0,1) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( c65 )
	GFXDECODE_ENTRY( "maincpu", 0xd000, charlayout,     0, 16 ) // another identical copy is at 0x9000
GFXDECODE_END

INTERRUPT_GEN_MEMBER(c65_state::vic3_vblank_irq)
{
	//if(m_VIC2_IRQMask & 1)
	//	m_maincpu->set_input_line(M4510_IRQ_LINE,HOLD_LINE);
}

static MACHINE_CONFIG_START( c65, c65_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M4510,MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(c65_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen",c65_state,vic3_vblank_irq)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(c65_state, screen_update)
//  MCFG_SCREEN_SIZE(32*8, 32*8)
//  MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_RAW_PARAMS(MAIN_CLOCK, 910, 0, 640, 525, 0, 200) // mods needed
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", c65)

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_INIT_OWNER(c65_state, c65)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( c65 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "910111", "V0.9.910111" )
	ROMX_LOAD( "910111.bin", 0x0000, 0x20000, CRC(c5d8d32e) SHA1(71c05f098eff29d306b0170e2c1cdeadb1a5f206), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "910523", "V0.9.910523" )
	ROMX_LOAD( "910523.bin", 0x0000, 0x20000, CRC(e8235dd4) SHA1(e453a8e7e5b95de65a70952e9d48012191e1b3e7), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "910626", "V0.9.910626" )
	ROMX_LOAD( "910626.bin", 0x0000, 0x20000, CRC(12527742) SHA1(07c185b3bc58410183422f7ac13a37ddd330881b), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "910828", "V0.9.910828" )
	ROMX_LOAD( "910828.bin", 0x0000, 0x20000, CRC(3ee40b06) SHA1(b63d970727a2b8da72a0a8e234f3c30a20cbcb26), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 4, "911001", "V0.9.911001" )
	ROMX_LOAD( "911001.bin", 0x0000, 0x20000, CRC(0888b50f) SHA1(129b9a2611edaebaa028ac3e3f444927c8b1fc5d), ROM_BIOS(5) )
ROM_END

ROM_START( c64dx )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "910429.bin", 0x0000, 0x20000, CRC(b025805c) SHA1(c3b05665684f74adbe33052a2d10170a1063ee7d) )
ROM_END

DRIVER_INIT_MEMBER(c65_state,c65)
{
//	m_dma.version = 2;
//	c65_common_driver_init();
}

DRIVER_INIT_MEMBER(c65_state,c65pal)
{
//	m_dma.version = 1;
//	c65_common_driver_init();
//	m_pal = 1;
}

COMP( 1991, c65,    0,      0,      c65,    c65, c65_state, c65,    "Commodore Business Machines",  "Commodore 65 Development System (Prototype, NTSC)", GAME_NOT_WORKING )
COMP( 1991, c64dx,  c65,    0,      c65,    c65, c65_state, c65pal, "Commodore Business Machines",  "Commodore 64DX Development System (Prototype, PAL, German)", GAME_NOT_WORKING )
