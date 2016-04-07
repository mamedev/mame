// license:BSD-3-Clause
// copyright-holders:David Haywood

/*

 CPUs
QTY 	Type 	clock 	position 	function
1x 	MC68HC000FN10 		u3 	16/32-bit Microprocessor - main
1x 	u6295 		u98 	4-Channel Mixing ADCPM Voice Synthesis LSI - sound
1x 	HA17358 		u101 	Dual Operational Amplifier - sound
1x 	TDA2003 		u104 	Audio Amplifier - sound
1x 	oscillator 	12.000MHz 	osc1 	
ROMs
QTY 	Type 	position 	status
2x 	M27C1001 	2,3 	dumped
1x 	M27C2001 	1 	dumped
3x 	M27C4001 	4,5,6 	dumped
RAMs
QTY 	Type 	position
11x 	LH52B256-10PLL 	u16a,u17a,u27,u28,u29,u30,u39,u40,u74,u75,u76
PLDs
QTY 	Type 	position 	status
1x 	ATF20V8B-15PC 	u37 	read protected
2x 	A40MX04-F-PL84 	u83,u86 	read protected
Others

1x 28x2 JAMMA edge connector
1x pushbutton (SW1)
1x trimmer (volume) (VR1)
4x 8x2 switches DIP (SW1,SW2,SW3,SW4)
1x battery 3.6V (BT1)
Notes

PCB silkscreened: "MADE IN TAIWAN YONSHI PCB NO-006F" 

*/



#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"


class jungleyo_state : public driver_device
{
public:
	jungleyo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	/* memory pointers */

	/* video-related */
	virtual void video_start() override;
	UINT32 screen_update_jungleyo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};

void jungleyo_state::video_start()
{
}

UINT32 jungleyo_state::screen_update_jungleyo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


static ADDRESS_MAP_START( jungleyo_map, AS_PROGRAM, 16, jungleyo_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( jungleyo )
INPUT_PORTS_END

static const gfx_layout jungleyo_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64
};

static const gfx_layout jungleyo16_layout =
{
	8,32,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
	  8*64, 9*64,10*64,11*64,12*64,13*64,14*64,15*64,
	 16*64,17*64,18*64,19*64,20*64,21*64,22*64,23*64,
	 24*64,25*64,26*64,27*64,28*64,29*64,30*64,31*64 },
	8*64*4
};


static GFXDECODE_START( jungleyo )
	GFXDECODE_ENTRY( "reelgfx", 0, jungleyo16_layout,   0x0, 2  )
	GFXDECODE_ENTRY( "gfx2", 0, jungleyo_layout,   0x0, 2  )
	GFXDECODE_ENTRY( "gfx3", 0, jungleyo_layout,   0x0, 2  )
GFXDECODE_END


static MACHINE_CONFIG_START( jungleyo, jungleyo_state )

	MCFG_CPU_ADD("maincpu", M68000, XTAL_12MHz)
	MCFG_CPU_PROGRAM_MAP(jungleyo_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", jungleyo_state,  irq1_line_hold)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", jungleyo)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(jungleyo_state, screen_update_jungleyo)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x200)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", XTAL_12MHz/16, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.47)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.47)
MACHINE_CONFIG_END


ROM_START( jungleyo )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */ // encrypted?
	ROM_LOAD16_BYTE( "jungle_(record)_rom3_vi3.02.u15", 0x00001, 0x20000, CRC(7c9f431e) SHA1(fb3f90c4fe59c938f36b30c5fa3af227031e7d7a) )
	ROM_LOAD16_BYTE( "jungle_(record)_rom2_vi3.02.u14", 0x00000, 0x20000, CRC(f6a71260) SHA1(8e48cbb9d701ad968540244396820359afe97c28) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "jungle_rom1.u99", 0x00000, 0x40000, CRC(05ef5b85) SHA1(ca7584646271c6adc7880eca5cf43a412340c522) )

	ROM_REGION( 0x80000, "reelgfx", 0 )
	ROM_LOAD( "jungle_rom4.u58", 0x000000, 0x80000, CRC(2f37da94) SHA1(6479e3bcff665316903964286d72df9822c05485) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "jungle_rom5.u59", 0x000000, 0x80000, CRC(0ccd9b94) SHA1(f209c7e15967be2e43be018aca89edd0c311503e) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "jungle_rom6.u60", 0x000000, 0x80000, CRC(caab8eb2) SHA1(472ca9f396d7c01a1bd03485581cfae677a3b365) )
ROM_END


GAME( 1999, jungleyo,    0,        jungleyo,    jungleyo, driver_device,    0, ROT0,  "Yonshi", "Jungle (VI3.02)", MACHINE_NOT_WORKING )
