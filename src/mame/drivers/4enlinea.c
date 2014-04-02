/*************************************************************************

  Cuatro en Linea.
  System I.
  1991, Compumatic

  Driver by David Haywood & Roberto Fresca.

**************************************************************************

  1x Z84C00HB6 CPU @ 8 MHz for program.
  1x Z84C00AB6 CPU @ 4 MHz for sound.
  1x AY-3-8910 A
  1x UMC UM487F (HCGA Controller)

  2x NEC D41464C (64K x 4-bit Dynamic NMOS RAM) for VRAM.
  1x UMC UM6264A (8K x 8-bit CMOS SRAM).

  2x 27512 EPROMS.
  1x X24C16P Serial EEPROM.

  1x GAL16V8AS

  1x ES2 CM3080 (unknown DIP-18 IC)
  1x ES2 9046 (unknown PLCC-84 IC)
  1x 8952 CM 32 (unknown DIP-40 IC)

  1x 16.0000 MHz crystal. ; Divided by 2 (through CM3080) for main CPU Z84C00HB6.
  1x 8.000 MHz crystal.   ; Divided by 2 for audio CPU Z84C00AB6.
  1x 14.31818 MHz crystal ; For HCGA controller.

  CN1: 1 x 8 connector.
  CN2: 1 x 8 connector.
  CN3: 2 x 5 connector.
  CN4: 2 x 5 connector.
  CN5: 2 x 5 connector.
  CN6: 2 x 28 Jamma connector.
  CN7: 1 x 20 connector.
  CN8: 1 x 4 connector.
  CN9: 1 x 4 connector.
  CN10: DB9 video out connector.
  CN11 1 x 2 bridge connector.

**************************************************************************

  UM487F HCGA Controller notes...

  The fact that there is a 14.318 MHz crystal tied to pin 65, just point
  that the video controller is working in CGA mode. MGA mode needs a
  16.257 MHz crystal instead, and tied to pin 64 (currently tied to GND).

  Also a signal of 8Mhz (shared with the program CPU is entering from the
  pin 1 (CLK) needed for clock the UM6845 mode.

  UM487F Access:

  Offsets are for sure the CGA mode. MGA mode has different ones.

  3D4h: -W  CRTC index register.
  3D5h: RW  CRTC data register.
  3D8h: -W  Mode control register.
  3D9h: -W  Color select register.
  3DAh: R-  Status register.
  3BFh: -W  Config register.

  Mode CTRL (3D8h): 0x6A / 0x62
  ----- bits -----
  7 6 5 4  3 2 1 0   For CGA Mode.
  - x x -  x - x -
  | | | |  | | | |
  | | | |  | | | '-- 40*25 text.
  | | | |  | | '---- Graphics.
  | | | |  | '------ Color Mode.
  | | | |  '-------- Enable Video.
  | | | '----------- 320x200 Graphics.
  | | '------------- Enable Blink.
  | '--------------- Enable Change Mode.
  '----------------- (not for CGA)

  Color Sel (3D9h): 00

  Index register (3D4h): 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
  Data register (3D5h):  38 28 2D 0A 7F 06 64 70 02 01 06 07 10 00 00 00

  Config Register (3BFh): 0x40
  (bit 6 active means CGA Mode)


  So... Screen size is set to 320x200.
  but...

  The embedded CRT controller is set to:
  Screen Total:   0x38+1 * 0x7F+1 = (57 * 128) chars.
  Screen Visible: 0x28 * 0x64 = (40 * 100) chars.

  NOTE: All (registers and offsets) match the CGA ISA card.
  Maybe we can find a workaround to hook the controller
  without the ISA bus.

**************************************************************************

  Custom IC's...

  8952 CM 32 pinouts and peripheral circuitry:

                               8952 CM 32
                            .------v------.
    74HC244 (IC9), PIN 08 --|01         40|-- 74HC244 (IC9), PIN 17
    74HC244 (IC9), PIN 06 --|02         39|-- 74HC244 (IC9), PIN 15
    74HC244 (IC9), PIN 04 --|03         38|-- 74HC244 (IC9), PIN 13
    74HC244 (IC9), PIN 02 --|04         37|-- 74HC244 (IC9), PIN 11
   74HC244 (IC10), PIN 08 --|05         36|-- 74HC244 (IC10), PIN 17
   74HC244 (IC10), PIN 06 --|06         35|-- 74HC244 (IC10), PIN 15
   74HC244 (IC10), PIN 04 --|07    8    34|-- 74HC244 (IC10), PIN 13
   74HC244 (IC10), PIN 02 --|08    9    33|-- 74HC244 (IC10), PIN 11
                            |      5      |
                         /--|09    2    32|--\
  To CN1 (through IC15) | --|10         31|-- | To CN1 (through IC15)
                        | --|11         30|-- |
                         \--|12    C    29|--/
                            |      M     |
                         /--|13         28|--\
  To CN2 (through IC14) | --|14    3    27|-- | To CN2 (through IC14)
                        | --|15    2    26|-- |
                         \--|16         25|--/
                            |             |
      To CN3 and ES2 9046 --|17         24|--\
      To CN3 and ES2 9046 --|18         23|-- > bridge to GND
                   To CN3 --|19         22|--/
                            |             |
                      GND --|20         21|-- VCC
                            '-------------'

                 74HC244 (IC9)                                 74HC244 (IC10)
                  .---v---.                                     .---v---.
   GAL (PIN 17) --|01   20|-- VCC                GAL (PIN 16) --|01   20|-- VCC
  8952 (PIN 04) --|02   19|-- GAL (PIN 17)      8952 (PIN 08) --|02   19|-- GAL (PIN 16)
  MAIN Z80 (D7) --|03   18|-- MAIN Z80 (D0)     MAIN Z80 (D7) --|03   18|-- MAIN Z80 (D0)
  8952 (PIN 03) --|04   17|-- 8952 (PIN 40)     8952 (PIN 07) --|04   17|-- 8952 (PIN 36)
  MAIN Z80 (D6) --|05   16|-- MAIN Z80 (D1)     MAIN Z80 (D6) --|05   16|-- MAIN Z80 (D1) 
  8952 (PIN 02) --|06   15|-- 8952 (PIN 39)     8952 (PIN 06) --|06   15|-- 8952 (PIN 35)
  MAIN Z80 (D5) --|07   14|-- MAIN Z80 (D2)     MAIN Z80 (D5) --|07   14|-- MAIN Z80 (D2)
  8952 (PIN 01) --|08   13|-- 8952 (PIN 38)     8952 (PIN 05) --|08   13|-- 8952 (PIN 34)
  MAIN Z80 (D4) --|09   12|-- MAIN Z80 (D3)     MAIN Z80 (D4) --|09   12|-- MAIN Z80 (D3) 
            GND --|10   11|-- 8952 (PIN 37)               GND --|10   11|-- 8952 (PIN 33)
                  '-------'                                     '-------'


  ES2 CM3080 pinouts and peripheral circuitry:

                    CM3080
                   .---v---.
             VCC --|01   18|-- VCC          .--------.
             N/C --|02   17|----------------+ 16 MHz |
             N/C --|03   16|----------------+  Xtal  |
             N/C --|04   15|-- CLK OUT --.  '--------'
             GND --|05   14|-- N/C       |
             GND --|06   13|-- N/C       '--+-- (8MHz) UM487F (PIN 01, CLK)
  MAIN Z80 (/M1) --|07   12|-- GND          +-- (8MHz) MAIN Z80 (PIN 06, CLK)
    GAL (PIN 11) --|08   11|-- VCC
             GND --|09   10|-- MAIN Z80 (/INT)
                   '-------'


  Notes:

  - Looks like the GAL is switching the different '8952 CM 32' outputs
     through the 74HC244 drivers to the Z80 data bus.
  - CN1, CN2 & CN3 are blind connectors.
  - 8952 pinouts to CN1 & CN2, are also passing through locations
     IC14 & IC15 (both are unpopulated from factory).
  - GAL is GAL16V8 at location IC4.
  - CM3080 pins 16 & 17 have a 1 Megohm resistor in parallel before connect the
     16 MHz. crystal.

**************************************************************************

  TODO:

  - Proper UM487F device emulation.
  - Interlaced video mode.
  - Sound.
  - More work...

*************************************************************************/

#define MAIN_CLOCK           XTAL_16MHz
#define SEC_CLOCK            XTAL_8MHz
#define HCGA_CLOCK           XTAL_14_31818MHz

#define PRG_CPU_CLOCK        MAIN_CLOCK /2		/* 8 MHz. (measured) */
#define SND_CPU_CLOCK        SEC_CLOCK /2       /* 4 MHz. (measured) */
#define SND_AY_CLOCK         SEC_CLOCK /4       /* 2 MHz. (measured) */
#define CRTC_CLOCK           SEC_CLOCK /2       /* 8 MHz. (measured) */


#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "sound/ay8910.h"

class _4enlinea_state : public driver_device
{
public:
	_4enlinea_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_ay(*this, "aysnd"),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")  { }


	required_device<ay8910_device> m_ay;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_videoram2;

	DECLARE_WRITE8_MEMBER(crtc_config_w);
	DECLARE_WRITE8_MEMBER(crtc_mode_ctrl_w);
	DECLARE_WRITE8_MEMBER(crtc_colormode_w);
	DECLARE_READ8_MEMBER(crtc_status_r);
	DECLARE_READ8_MEMBER(unk_e000_r);
	DECLARE_READ8_MEMBER(unk_e001_r);

	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(_4enlinea);
	UINT32 screen_update_4enlinea(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_WRITE8_MEMBER(vram2_w);

};


/***********************************
*          Video Hardware          *
***********************************/

void _4enlinea_state::video_start()
{
	m_gfxdecode->gfx(0)->set_source(m_videoram);
}

UINT32 _4enlinea_state::screen_update_4enlinea(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
/* note: chars are 16*12 pixels */

	int offset = 0;
	int offset2 = 0;

	for (int y = 0; y < 200; y++)
	{
		UINT16* dstptr_bitmap = &bitmap.pix16(y);

		for (int x = 0; x < 320; x += 4)
		{
			UINT8 pix;

			if (y & 1) pix = m_videoram2[offset2++];
			else pix = m_videoram[offset++];

			dstptr_bitmap[x + 3] = (pix >> 0) & 0x3;
			dstptr_bitmap[x + 2] = (pix >> 2) & 0x3;
			dstptr_bitmap[x + 1] = (pix >> 4) & 0x3;
			dstptr_bitmap[x + 0] = (pix >> 6) & 0x3;
		}
	}

	return 0;
}


WRITE8_MEMBER(_4enlinea_state::vram_w)
{
	m_videoram[offset] = data;
//	m_gfxdecode->gfx(0)->mark_dirty(offset/16);
}

WRITE8_MEMBER(_4enlinea_state::vram2_w)
{
	m_videoram2[offset] = data;
//	m_gfxdecode->gfx(0)->mark_dirty(offset/16);
}

WRITE8_MEMBER(_4enlinea_state::crtc_config_w)
{
/* Bit 6 enables the CGA mode, otherwise is MGA */
	if(data & 0x40)
	{
		logerror("CRTC config mode (3BFh): CGA\n");
	}
	else
	{
		logerror("CRTC config mode (3BFh): MGA\n");
	}
}

WRITE8_MEMBER(_4enlinea_state::crtc_mode_ctrl_w)
{
/* Bit 3 enables/disables the video (see the notes above) */
	logerror("CRTC mode control (3D8h): %02x\n", data);
}

WRITE8_MEMBER(_4enlinea_state::crtc_colormode_w)
{
	logerror("CRTC color mode (3D9h): %02x\n", data);
}

READ8_MEMBER(_4enlinea_state::crtc_status_r)
{
/*----- bits -----
  7 6 5 4  3 2 1 0   For CGA Mode.
  x x x x  - - - -  (bits 4-5-6-7 are unused)
           | | | |
           | | | '-- 0: Display active period.
           | | |     1: Non-display period.
           | | |
           | | '---- 0: Light pen reset.
           | |       1: Light pen set.
           | |
           | '------ 0: Light pen switch off.
           |         1: Light pen switch on.
           |
           '-------- 0: Non-vertical sync period.
                     1: Vertical sync period.

*/
	return (m_screen->vpos() >= 200) ? 0x80 : 0x00;    // bit 7 is suppossed to be unused in CGA mode
}

READ8_MEMBER(_4enlinea_state::unk_e000_r)
{
	logerror("read e000\n");
//	return (machine().rand() & 0xff);
	return 0xff;
}

READ8_MEMBER(_4enlinea_state::unk_e001_r)
{
	logerror("read e001\n");
	return (machine().rand() & 0xff);	// after 30 seconds, random strings and gfx appear on the screen.
//	return (machine().rand() & 0x0f);	// after 30 seconds, random gfx appear on the screen.
}


/***********************************
*      Memory Map Information      *
***********************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, _4enlinea_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_RAM_WRITE(vram_w) AM_SHARE("videoram")		// even lines
	AM_RANGE(0xa000, 0xbfff) AM_RAM_WRITE(vram2_w) AM_SHARE("videoram2")	// odd lines
	AM_RANGE(0xc000, 0xdfff) AM_RAM

	AM_RANGE(0xe000, 0xe000) AM_READ(unk_e000_r)
	AM_RANGE(0xe001, 0xe001) AM_READ(unk_e001_r)

	AM_RANGE(0xe002, 0xe3ff) AM_RAM	// bad... just temporary to allow writes for debug purposes.

ADDRESS_MAP_END


static ADDRESS_MAP_START( main_portmap, AS_IO, 8, _4enlinea_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3ff)

	AM_RANGE(0x3d4, 0x3d4) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x3d5, 0x3d5) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0x3d8, 0x3d8) AM_WRITE(crtc_mode_ctrl_w)
	AM_RANGE(0x3d9, 0x3d9) AM_WRITE(crtc_colormode_w)
	AM_RANGE(0x3da, 0x3da) AM_READ(crtc_status_r)
	AM_RANGE(0x3bf, 0x3bf) AM_WRITE(crtc_config_w)

ADDRESS_MAP_END



static ADDRESS_MAP_START( audio_map, AS_PROGRAM, 8, _4enlinea_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( audio_portmap, AS_IO, 8, _4enlinea_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


/***********************************
*           Input Ports            *
***********************************/

static INPUT_PORTS_START( 4enlinea )

/*  Player 1 & 2 ports are tied to both AY-3-8910 ports.
    Coin 1 & 2 are tied to the big ES2 9046 CPLD/FPGA,
    so... It's a mystery to figure out.
*/
	PORT_START("IN-P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY  PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY  PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY  PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY  PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )                   PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )                   PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN-P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY  PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY  PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY  PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY  PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )                   PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )                   PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/***********************************
*         Graphics Layouts         *
***********************************/

static const gfx_layout charlayout =
{
	8,8,
	0x4000/16,
	2,
	{ 0, 1 },
	{ 0, 2, 4, 6, 8, 10, 12, 14 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};

/****************************************
*      Graphics Decode Information      *
****************************************/

static GFXDECODE_START( 4enlinea )
	GFXDECODE_ENTRY( NULL, 0, charlayout, 0, 1 )
GFXDECODE_END


/****************************************
*          Machine Start/Reset          *
****************************************/

void _4enlinea_state::machine_start()
{

}

void _4enlinea_state::machine_reset()
{

}


/**********************************
*         CRTC Interface          *
**********************************/

static MC6845_ON_UPDATE_ADDR_CHANGED(crtc_addr)
{

}

static MC6845_INTERFACE( mc6845_intf )
{
	false,           /* show border area */
	0,0,0,0,         /* visarea adjustment */
	8,               /* number of pixels per video memory address */
	NULL,            /* before pixel update callback */
	NULL,            /* row update callback */
	NULL,            /* after pixel update callback */
	DEVCB_NULL,      /* callback for display state changes */
	DEVCB_NULL,      /* callback for cursor state changes */
	DEVCB_NULL,      /* HSYNC callback */
	DEVCB_NULL,      /* VSYNC callback */
	crtc_addr        /* update address callback */
};


/***********************************
*         Sound Interface          *
***********************************/

static const ay8910_interface ay8910_intf =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("IN-P2"),
	DEVCB_INPUT_PORT("IN-P1"),
	DEVCB_NULL,
	DEVCB_NULL
};


/***********************************
*         Machine Drivers          *
***********************************/

static MACHINE_CONFIG_START( 4enlinea, _4enlinea_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, PRG_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", _4enlinea_state, nmi_line_pulse)
	MCFG_CPU_PERIODIC_INT_DRIVER(_4enlinea_state, irq0_line_hold, 4*60)

	MCFG_CPU_ADD("audiocpu", Z80, SND_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(audio_map)
	MCFG_CPU_IO_MAP(audio_portmap)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(_4enlinea_state, screen_update_4enlinea)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", 4enlinea)
	MCFG_PALETTE_ADD("palette", 256)

/*  6845 clock is a guess, since it's a UM6845R embedded in the UM487F.
    CRTC_CLOCK is 8MHz, entering for pin 1 of UM487F. This clock is used
    only for UM6845R embedded mode. The frequency divisor is unknown.

	CRTC_CLOCK / 4.0 = 66.961296 Hz.
	CRTC_CLOCK / 4.5 = 59.521093 Hz.
    CRTC_CLOCK / 5.0 = 53.569037 Hz.
*/
//	MCFG_MC6845_ADD("crtc", MC6845, "screen", CRTC_CLOCK / 2, mc6845_intf)	// seems that MC6845 doesn't support the game mode
	MCFG_MC6845_ADD("crtc", R6545_1, "screen", CRTC_CLOCK / 4.5, mc6845_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, SND_AY_CLOCK)
	MCFG_SOUND_CONFIG(ay8910_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


/***********************************
*             Rom Load             *
***********************************/

ROM_START( 4enlinea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cuatro_en_linea_27c256__cicplay-2.ic6",  0x0000, 0x8000, CRC(f8f14bf8) SHA1(e48fbedbd1b9be6fb56a0f65db80eddbedb487c7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cuatro_en_linea_27c256__cicplay-1.ic19", 0x0000, 0x8000, CRC(307a57a3) SHA1(241329d919ec43d0eeb1dad0a4db6cf6de06e7e1) )

	ROM_REGION( 0x0800, "eeprom", 0 )   /* default serial EEPROM */
	ROM_LOAD( "cuatro_en_linea_x24c16p__nosticker.ic17", 0x000, 0x800, CRC(21f81f5a) SHA1(00b10eee5af1ca79ced2878f4be4cac2bb8d26a0) )

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "cuatro_en_linea_gal16v8as__nosticker.ic04", 0x000, 0x117, CRC(094edf29) SHA1(428a2f6568ac1032833ee0c65fa8304967a58607) )
ROM_END


/***********************************
*           Game Drivers           *
***********************************/

/*    YEAR  NAME       PARENT   MACHINE   INPUT     STATE          INIT   ROT    COMPANY       FULLNAME          FLAGS  */
GAME( 1991, 4enlinea,  0,       4enlinea, 4enlinea, driver_device, 0,     ROT0, "Compumatic", "Cuatro en Linea", GAME_NOT_WORKING )
