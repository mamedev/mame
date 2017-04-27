// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
	Space Cyclone
	
	3 board stack? - the PCB pictures make it very difficult to figure much out
	
	seems a bit like 8080bw hardware but with extra sprites and a z80 cpu?
	maybe an evolution of Polaris etc.?
	
	
	there's an MB14241 near the Sprite ROM
	
	1x 8DSW, 1x 4DSW
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "screen.h"
#include "speaker.h"
#include "machine/mb14241.h"

class scyclone_state : public driver_device
{
public:
	scyclone_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_mainram(*this, "mainram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_mainram;

	DECLARE_WRITE8_MEMBER(scyclone_port06_w);
	DECLARE_READ8_MEMBER(scyclone_port01_r);	
	DECLARE_READ8_MEMBER(scyclone_port03_r);

	/* video-related */
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_scyclone(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	
	DECLARE_DRIVER_INIT(scyclone);
	
	INTERRUPT_GEN_MEMBER(irq);
};



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void scyclone_state::video_start()
{
}

uint32_t scyclone_state::screen_update_scyclone(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int count = 0;
	
	for (int y=0;y<256;y++)
	{
		for (int x=0;x<256/8;x++)
		{
			uint8_t video_data = m_mainram[count];
			
			for (int i=0;i<8;i++)
			{
				pen_t pen = ((video_data>>i) & 0x01) ? rgb_t::white() : rgb_t::black();
				bitmap.pix32(y, x*8+i) = pen;
			}

			count++;
		}	
	}	
		
	return 0;
}



static ADDRESS_MAP_START( scyclone_map, AS_PROGRAM, 8, scyclone_state )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x4000, 0x5fff) AM_RAM	AM_SHARE("mainram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( scyclone_iomap, AS_IO, 8, scyclone_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_READ(scyclone_port01_r)	
	AM_RANGE(0x03, 0x03) AM_READ(scyclone_port03_r) AM_WRITENOP
	AM_RANGE(0x04, 0x04) AM_WRITENOP
	AM_RANGE(0x05, 0x05) AM_WRITENOP
	AM_RANGE(0x06, 0x06) AM_WRITE(scyclone_port06_w)
	AM_RANGE(0x08, 0x08) AM_WRITENOP
	AM_RANGE(0x09, 0x09) AM_WRITENOP
	AM_RANGE(0x0a, 0x0a) AM_WRITENOP	
	AM_RANGE(0x0e, 0x0e) AM_WRITENOP
	AM_RANGE(0x0f, 0x0f) AM_WRITENOP
	AM_RANGE(0x40, 0x40) AM_WRITENOP
	AM_RANGE(0x80, 0x80) AM_WRITENOP	
ADDRESS_MAP_END


static ADDRESS_MAP_START( scyclone_sub_map, AS_PROGRAM, 8, scyclone_state )
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x0800, 0x1fff) AM_ROM // maybe
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	
	AM_RANGE(0x3000, 0x3005) AM_RAM	// comms?
ADDRESS_MAP_END

static ADDRESS_MAP_START( scyclone_sub_iomap, AS_IO, 8, scyclone_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


static INPUT_PORTS_START( scyclone )
INPUT_PORTS_END


WRITE8_MEMBER(scyclone_state::scyclone_port06_w)
{

}

READ8_MEMBER(scyclone_state::scyclone_port01_r)
{
	// TILT is in here at least
	return 0x00;
}


READ8_MEMBER(scyclone_state::scyclone_port03_r)
{
	return machine().rand();
}

static const gfx_layout tiles32x32_layout =
{
	32,32,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 256+0, 256+1, 256+2, 256+3, 512+0, 512+1, 512+2, 512+3, 768+0, 768+1, 768+2, 768+3, 1024+0, 1024+1, 1024+2, 1024+3, 1280+0, 1280+1, 1280+2, 1280+3, 1536+0, 1536+1, 1536+2, 1536+3, 1792+0, 1792+1, 1792+2, 1792+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8, 16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8, 24*8, 25*8, 26*8, 27*8, 28*8, 29*8, 30*8, 31*8 },
	8*32*8
};

static GFXDECODE_START( scyclone )
	GFXDECODE_ENTRY( "gfx1", 0, tiles32x32_layout, 0, 16 )
GFXDECODE_END



void scyclone_state::machine_start()
{
}

void scyclone_state::machine_reset()
{
}

INTERRUPT_GEN_MEMBER(scyclone_state::irq)
{
	// CPU runs in IM0
	m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7);   /* RST 10h */
}


static MACHINE_CONFIG_START( scyclone, scyclone_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,8000000) // MOSTEK Z80-CPU   ? MHz  (there's also a 9.987MHz XTAL)
	MCFG_CPU_PROGRAM_MAP(scyclone_map)
	MCFG_CPU_IO_MAP(scyclone_iomap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", scyclone_state,  irq)

	// sound ?
	MCFG_CPU_ADD("subcpu", Z80,5000000/2) // LH0080 Z80-CPU SHARP  ? MHz   (5Mhz XTAL on this sub-pcb)
	MCFG_CPU_PROGRAM_MAP(scyclone_sub_map)
	MCFG_CPU_IO_MAP(scyclone_sub_iomap)

	/* add shifter */
	MCFG_MB14241_ADD("mb14241")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 32, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(scyclone_state, screen_update_scyclone)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", scyclone)
	MCFG_PALETTE_ADD("palette", 0x100)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
    // 2x SN76477N
MACHINE_CONFIG_END

ROM_START( scyclone )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "DE07.1H.2716", 0x0000, 0x0800, CRC(fa9c93a3) SHA1(701a3196ee67533d69f1264e14085d04dfb1b915) ) 
	ROM_LOAD( "DE08.3H.2716", 0x0800, 0x0800, CRC(bdff3e77) SHA1(e3fc6840348fa6df834cb1fb8b3832db05b5c585) ) 
	ROM_LOAD( "DE09.4H.2716", 0x1000, 0x0800, BAD_DUMP CRC(2fd791a0) SHA1(41b7d42bef338c0a0f19efcbf1a49a75aaab5993) ) 
	ROM_LOAD( "DE10.5H.2716", 0x1800, 0x0800, CRC(71301be6) SHA1(70c1983d256a6a9e18c7a15b93f5e3781d884fdb) ) 
	ROM_LOAD( "DE11.6H.2716", 0x2000, 0x0800, CRC(d241db51) SHA1(53c6489d715baf5f03032a7ac367a6af64bce040) ) 
	ROM_LOAD( "DE12.7H.2716", 0x2800, 0x0800, CRC(208e3e9a) SHA1(07f0e6c5417584eef171d8dcad83d741f88c9348) ) 

	ROM_REGION( 0x2000, "subcpu", 0 )
	ROM_LOAD( "DE18.IC1.2716", 0x0000, 0x0800, CRC(182a5c24) SHA1(0ba29b6b3e7ec97c0b0748a6e60bb1a737dc55fa) ) 
	// maybe load here based on board layout
	ROM_LOAD( "DE04.IC9.2716",  0x0800, 0x0800, CRC(098269ac) SHA1(138af858bbbd73380086f971bcdd52ae47e2b667) ) 
	ROM_LOAD( "DE05.IC14.2716", 0x1000, 0x0800, CRC(878f68b2) SHA1(2c2fa1b9053664ec26452ab0b35e2ae550601cc9) ) 
	ROM_LOAD( "DE06.IC19.2716", 0x1800, 0x0800, CRC(0b1ead90) SHA1(38322b39f4420408c223cdbed3b75692a3d70746) ) 

	ROM_REGION( 0x0800, "gfx1", 0 ) // 32x32x2 sprites?
	ROM_LOAD( "DE01.11C.2716", 0x0000, 0x0800, CRC(bf770730) SHA1(1d0f9235b0618e3f4dd6db47efbdf92e2b00f5f6) )
	
	// undumped proms (DE02 and DE15 on the sprite board?, something in the sound section? DE17 on the main board? DE16 somewhere)
ROM_END

DRIVER_INIT_MEMBER(scyclone_state,scyclone)
{
	// this code jumped to here does not look good at all, infinite loop corrupting memory, see below
	// should it never happen or is there a rom problem or rom mapping problem?
	uint8_t *rom = memregion("maincpu")->base();
	rom[0x11d] = 0x00;
	rom[0x11e] = 0x00;
	rom[0x11f] = 0x00;

	/*
	code ends up trashing the stack

	0116: FE 09         cp   $09
	0118: 3E 02         ld   a,$02
	011A: D2 79 14      jp   nc,$1479
	011D: C3 82 14      jp   $1482  -- this jump
	0120: CD 6A 19      call $196A

	-- doesn't look like it's really meant to go here?
	147F: 18 CD         jr   $144E
	1481: 4A            ld   c,d
	1482: 14            inc  d  -- to here
	1483: CD F1 18      call $18F1
	1486: C3 D8 00      jp   $00D8
	1489: 2E C4         ld   l,$C4

	18EE: CD EA 10      call $10EA
	18F1: 3E 04         ld   a,$04 -- to here
	18F3: CD 82 14      call $1482 -- back to where we just came from...
	18F6: 3E 04         ld   a,$04
	18F8: C3 79 14      jp   $1479
	18FB: CD 28 2B      call $2B28

	*/

}		
	
GAME( 1980, scyclone,  0,    scyclone, scyclone, scyclone_state,  scyclone, ROT270, "Taito Corporation", "Space Cyclone", MACHINE_NOT_WORKING )
