// license:BSD-3-Clause
// copyright-holders:Yves
/*     vlc.c
Multi-games from VLC Nevada 1995
CGA monitor 15Khz 60hz
// CPU    CLOCK use Crystal 16.000MHZ
// VIDEO  CLOCK use Crystal 33.000MHZ

    - 1x MC68000  (CPU)  8mhz
    - 1x MC68B45P (CRTC)
    - 1x AY3-8912 (sound)
    - 1x M4262B oki (RTC CLOCK)
    - 3x MC68681  (2 x UART)
    - 4x 27C512
    - 2x 62256 NVRAM  + batt  (connect in 16bits)
    - 2x 6264  BACKUP + batt  (connect in  8bits)
    - 1x MODEM XE1214 from XECOM (300/1200 Baud)



on MAME 0.145u4
Yves
Todo :
Video Section
Security PAL U35 decoding
In/Out interface Board communication (included mechanical METER + some switch Door open ,etc..)
JCM Bill acceptor communication (probably ID-003 protocol)
PRINTER communication (Star Printer )

******************************************************
******************************************************
SECURITY SYSTEM

ADDRESS A40000..A40001
U35 PAL16R8  SECURITY
PAL is connect on the BUS D15..D8
Write Clock 8 bits in D15..D8
Read  OE    8 bit  on D15..D8

U50 4Bit D Flop Register is connect on D3..D0
this one seem to have 2 use.
1. keep track of the PAL16R8 state
2. DOOR SWITCH is connect on U50 Clear pin (D3..D0 LOW = Door Open)
Write Clock 4 bits in D-Flop Register
Read  OE    4 bits on D3..D0


Boot Section is locate in NVRAM. Interrupts Pointers are changed on the fly.
seem to check hardware WDT ,Power Failure , interrupt system,etc..  before game start.

INT7 seem to control POWER FAILURE ,WDT.
INT7 initialisation is needed to boot the game.

******************************************************
******************************************************
VIDEO REGISTER of 6845

BOOT:FE1A82 00A5 0000           CRTC_TYPE1:     dc.l unk_A50000
BOOT:FE1A86 0000                                dc.b   0
BOOT:FE1A87 0000                                dc.b   0
BOOT:FE1A88 0000                                dc.b   0
BOOT:FE1A89 0000                                dc.b   0
BOOT:FE1A8A 0000                                dc.b   0
BOOT:FE1A8B 0007                                dc.b   7
BOOT:FE1A8C 0000                                dc.b   0
BOOT:FE1A8D 001F                                dc.b $1F
BOOT:FE1A8E 001F                                dc.b $1F
BOOT:FE1A8F 0002                                dc.b   2
BOOT:FE1A90 0020                                dc.b $20
BOOT:FE1A91 0003                                dc.b   3
BOOT:FE1A92 0023                                dc.b $23 * #
BOOT:FE1A93 001F                                dc.b $1F
BOOT:FE1A94 002A                                dc.b $2A * *
BOOT:FE1A95 0000                                dc.b   0
BOOT:FE1A96 00A5 8000           CRTC_TYPE2:     dc.l unk_A58000
BOOT:FE1A9A 00FF                                dc.b $FF
BOOT:FE1A9B 0000                                dc.b   0
BOOT:FE1A9C 0000                                dc.b   0
BOOT:FE1A9D 0000                                dc.b   0
BOOT:FE1A9E 0000                                dc.b   0
BOOT:FE1A9F 0007                                dc.b   7
BOOT:FE1AA0 0000                                dc.b   0
BOOT:FE1AA1 001F                                dc.b $1F
BOOT:FE1AA2 001F                                dc.b $1F
BOOT:FE1AA3 0002                                dc.b   2
BOOT:FE1AA4 0020                                dc.b $20
BOOT:FE1AA5 0006                                dc.b   6
BOOT:FE1AA6 0033                                dc.b $33 * 3
BOOT:FE1AA7 002D                                dc.b $2D * -
BOOT:FE1AA8 0040                                dc.b $40 * @
BOOT:FE1AA9 0000                                dc.b   0


BOOT:FE19F0                     INI_CRTC6845:
BOOT:FE19F0 48E7 8140                           movem.l d0/d7/a1,-(sp)
BOOT:FE19F4 2258                                movea.l (a0)+,a1        * type1 = 0xA50000, type2 = 0xA80000
BOOT:FE19F6 50D1                                st      (a1)
BOOT:FE19F8 11D8 0435                           move.b  (a0)+,(ScreenMode)
BOOT:FE19FC 4278 0412                           clr.w   (word_412)
BOOT:FE1A00 2248                                movea.l a0,a1
BOOT:FE1A02 3E3C 0FFF                           move.w  #$FFF,d7
BOOT:FE1A06 307C 0000                           movea.w #0,a0
BOOT:FE1A0A 6100 0108                           bsr     set_vid_D0_A0   * D0 = 0x800020
BOOT:FE1A0A                                                             * A0 = 0xB00000
BOOT:FE1A0E 7E0D                                moveq   #$D,d7
BOOT:FE1A10
BOOT:FE1A10                     LOOP:
BOOT:FE1A10 13C7 0090 0001                      move.b  d7,(CRTC_ADR)
BOOT:FE1A16 13D9 0090 8001                      move.b  (a1)+,(CRTC_DAT)
BOOT:FE1A1C 51CF FFF2                           dbf     d7,LOOP
BOOT:FE1A20 4CDF 0281                           movem.l (sp)+,d0/d7/a1
BOOT:FE1A24 4E75                                rts


    *** MC6845 Initialization ***
                Htotal   Hdisp   HsyncPos HsyncW   Vtotal  VtotalAdj Vdisp  VsyncPos InterMode MaxScanAdr CurStart CurEnd StartAdrH StartAdrL CurH  CurL  LightPenH LightPenL
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    register:   00       01      02       03       04      05        06     07       08        09         10       11     12        13        14    15    16        17
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------
nevada TYPE1 :  42       31      35       03       32      02        31     31       00        07         00       00     00        00        00    00    00        00
nevada TYPE2 :  64       45      51       06       32      02        31     31       00        07         00       00     00        00        00    00    00        00

*/


#define MASTER_CLOCK    XTAL_16MHz
#define MASTER_CPU            ((MASTER_CLOCK)/2)    // 8mhz
#define SOUND_CLOCK           ((MASTER_CLOCK) /8)   // 2mhz

#define VIDEO_CLOCK           XTAL_33MHz
#define MC6845_CLOCK          ((VIDEO_CLOCK)/4/16)  // 0.515625 MHZ


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"
#include "video/mc6845.h"
#include "sound/ay8910.h"
#include "machine/msm6242.h"
#include "machine/microtch.h"


/***************************************************************************

                                 General

***************************************************************************/


class nevada_state : public driver_device
{
public:
	nevada_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_duart18_68681(*this, "duart18_68681"),
		m_duart39_68681(*this, "duart39_68681"),
		m_duart40_68681(*this, "duart40_68681"),
		m_maincpu(*this,"maincpu"),
		m_microtouch(*this,"microtouch"),
		m_nvram(*this,"nvram"),
		m_ram62256(*this, "ram62256"),
		m_backup(*this, "backup")
		{ }

	required_device<mc68681_device> m_duart18_68681;
	required_device<mc68681_device> m_duart39_68681;
	required_device<mc68681_device> m_duart40_68681;

	required_device<cpu_device> m_maincpu;
	optional_device<microtouch_device> m_microtouch;
	required_device<nvram_device> m_nvram;

	required_shared_ptr<UINT16> m_ram62256;
	required_shared_ptr<UINT16> m_backup;

	void nvram_init(nvram_device &nvram, void *data, size_t size);

	UINT16  m_datA40000;

		//UINT8* m_videoram;
		//UINT8* m_colorram;

	UINT16* m_videoram;
	tilemap_t *m_bg_tilemap;
	virtual void video_start() override;
	UINT32 screen_update_nevada(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_PALETTE_INIT(nevada);

	DECLARE_WRITE_LINE_MEMBER(duart18_irq_handler);
	DECLARE_WRITE_LINE_MEMBER(duart39_irq_handler);
	DECLARE_WRITE_LINE_MEMBER(duart40_irq_handler);
	DECLARE_WRITE_LINE_MEMBER(nevada_rtc_irq);
	DECLARE_READ16_MEMBER(io_board_r);
	DECLARE_WRITE16_MEMBER(io_board_w);
	DECLARE_WRITE16_MEMBER (io_board_x);
	DECLARE_READ16_MEMBER( nevada_sec_r );
	DECLARE_WRITE16_MEMBER( nevada_sec_w );

	DECLARE_MACHINE_START(nevada);
	DECLARE_DRIVER_INIT(nevada);
};

/*
need further test on PAL16R8 (FlipFlop Trick with other bits ?)
this DUMP is done writing 0 to 255 and reading output (probably not the good way.)

PAL is connected on the UPPER byte D15..D8
Address A40000..A40001
2 Type of PAL (one for game, the other is to set game to fabric default)

there is a 74LS173 on the LOWER byte that used bit D3..D0
funny thing , the DOOR ACCESS Switch is connected on the CLEAR  PIN of this 4bits register
so when D3..D0 are LOW , DOOR is OPEN
*/
static const UINT8 pal35[256] = {
0x11, 0x42, 0x5B, 0xCA, 0x19, 0x42, 0x5B, 0xCA, 0x38, 0x63, 0x3A, 0x63, 0x3A, 0x63, 0x3A, 0x63,
0xD3, 0x08, 0x5B, 0xCA, 0x19, 0xCA, 0x19, 0xCA, 0x18, 0xEB, 0x18, 0xEB, 0x18, 0xEB, 0x18, 0xEB,
0xD3, 0xCA, 0x5B, 0xCC, 0x5B, 0xCC, 0x5B, 0xCC, 0xBA, 0x63, 0x38, 0x65, 0x38, 0x65, 0x38, 0x65,
0xD1, 0xCA, 0x5B, 0xC8, 0x5B, 0xC8, 0x5B, 0xC8, 0x9A, 0xEB, 0x1A, 0xED, 0x1A, 0xED, 0x1A, 0xED,
0x0C, 0x65, 0xF0, 0x00, 0x64, 0xF5, 0x04, 0x65, 0xB8, 0x25, 0x20, 0x20, 0x24, 0x24, 0x24, 0x24,
0xF0, 0x00, 0xF8, 0x00, 0xFC, 0x00, 0xFC, 0x00, 0xB8, 0x3D, 0xFC, 0x19, 0xFC, 0x19, 0xFC, 0x19,
0x44, 0xF9, 0xC4, 0xF9, 0xC4, 0xFD, 0xC4, 0xFD, 0xFC, 0xFD, 0xFC, 0xFD, 0xFC, 0xFD, 0xFC, 0xFD,
0xC0, 0xD9, 0xC8, 0xD9, 0xC8, 0xD9, 0xC8, 0xD9, 0xD8, 0xD9, 0xD8, 0xD9, 0xD8, 0xD9, 0xD8, 0xD9,
0x0C, 0x44, 0xFB, 0x04, 0x67, 0xD4, 0x0C, 0x44, 0xBA, 0x24, 0x22, 0x02, 0x26, 0x06, 0x26, 0x06,
0xFB, 0x00, 0xFB, 0x00, 0xEF, 0x10, 0xCE, 0x18, 0xEF, 0x18, 0xEF, 0x18, 0xFF, 0x18, 0xFF, 0x18,
0x44, 0xF8, 0xC6, 0xD8, 0xCE, 0xDC, 0xCE, 0xDC, 0xEF, 0x9E, 0x67, 0xB8, 0x67, 0xBC, 0x67, 0xBC,
0xC6, 0xD8, 0xCA, 0xD8, 0xCA, 0xD8, 0xCA, 0xD8, 0xCB, 0x9A, 0xEF, 0x9A, 0xFF, 0xD8, 0xDB, 0xD8,
0x66, 0xF4, 0x00, 0x64, 0xBA, 0x25, 0x22, 0x22, 0x26, 0x26, 0x26, 0x26, 0xF2, 0x00, 0xFA, 0x00,
0xFA, 0x00, 0xFA, 0x00, 0xBA, 0x3D, 0xFA, 0x19, 0xFA, 0x19, 0xFA, 0x19, 0x44, 0xF9, 0xC2, 0xF0,
0xC2, 0xF4, 0xC2, 0xF4, 0xFA, 0xF4, 0xFA, 0xF4, 0xFA, 0xF4, 0xFA, 0xF4, 0xC2, 0xD0, 0xCA, 0xD0,
0xCA, 0xD0, 0xCA, 0xD0, 0xDA, 0xD0, 0xDA, 0xD0, 0xDA, 0xD0, 0xDA, 0xD0, 0x08, 0x63, 0xD3, 0x08
};


/***************************************************************************/
/********************   VIDEO SECTION   ************************************/
/***************************************************************************/

static const gfx_layout charlayout =
{
	/* Todo  , just for sample */

	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

/***************************************************************************/
/*
WRITE16_MEMBER( nevada_state:nevada_videoram_w )
{
// Todo, Just for sample

    m_videoram[offset] = data;
    m_bg_tilemap->mark_tile_dirty(offset);

}
*/
/***************************************************************************/
static GFXDECODE_START( nevada )
	/* Todo  , just for sample */
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout,   0, 8 )
GFXDECODE_END

/***************************************************************************/
/*
static TILE_GET_INFO_MEMBER( nevada_state::get_bg_tile_info )
{
// Todo, Just for sample
    int attr = m_colorram[tile_index];
    int code = ((attr & 1) << 8) | m_videoram[tile_index];
    int bank = (attr & 0x02) >> 1;
    int color = (attr & 0x3c) >> 2;

    SET_TILE_INFO_MEMBER(bank, code, color, 0);

}
*/

/***************************************************************************/
void nevada_state::video_start()
{
// todo
/*
    m_bg_tilemap = tilemap_create(machine(), get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
*/
}

/***************************************************************************/
UINT32 nevada_state::screen_update_nevada(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Todo
/*
    m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
*/
	return 0;
}

/***************************************************************************/
PALETTE_INIT_MEMBER(nevada_state, nevada)
{
	// Palette init
}

/***************************************************************************/
/********************   NVRAM SECTION   ************************************/
/***************************************************************************/

void nevada_state::nvram_init(nvram_device &nvram, void *data, size_t size)
{
	memset(data, 0x00, size);
	if (memregion("defaults")->base())
		memcpy(data, memregion("defaults")->base(), memregion("defaults")->bytes());
}


/***************************************************************************

    U18 MC68681 RS232 UART  SIDEA = MODEM 1200 Baud
    U18 MC68681 RS232 UART  SIDEB = not used
    Interrupt 4
***************************************************************************/

WRITE_LINE_MEMBER(nevada_state::duart18_irq_handler)
{
	m_maincpu->set_input_line_and_vector(4, state, m_duart18_68681->get_irq_vector());
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************

    U39 MC68681 RS232 UART SIDEA = Printer J3 (rs232)
    U39 MC68681 RS232 UART SIDEB = Player Tracking Interface J2 (not used)
    Interrupt 3
***************************************************************************/

WRITE_LINE_MEMBER(nevada_state::duart39_irq_handler)
{
	m_maincpu->set_input_line_and_vector(3, state, m_duart39_68681->get_irq_vector());
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************

    68681 DUART <-> Microtouch touch screen controller communication
    U40 MC68681 RS232 UART  SIDEA = TouchScreen J1 (RS232)
    U40 MC68681 RS232 UART  SIDEB = JCM Bill Acceptor (RS422)
    Interrupt 5
***************************************************************************/

WRITE_LINE_MEMBER(nevada_state::duart40_irq_handler)
{
	m_maincpu->set_input_line_and_vector(5, state, m_duart40_68681->get_irq_vector());
}

/***************************************************************************/
/*********************    RTC SECTION       ********************************/
/***************************************************************************/
WRITE_LINE_MEMBER(nevada_state::nevada_rtc_irq)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ1, HOLD_LINE);  // rtc interrupt on INT1
}

/***************************************************************************/
/*********************    SOUND SECTION     ********************************/
/***************************************************************************/
#if 0
static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
//  DEVCB_INPUT_PORT("DSW1"),  /* not used */
//  DEVCB_INPUT_PORT("DSW2"),  /* not used */
	DEVCB_NULL, /* callback for display state changes */
	DEVCB_NULL, /* callback for cursor state changes */
	DEVCB_NULL,
	DEVCB_NULL
};
#endif

/***************************************************************************/
READ16_MEMBER(nevada_state::io_board_r)
{
	// IO board Serial communication 0xA00000
	return 1;
}
/***************************************************************************/
WRITE16_MEMBER(nevada_state::io_board_w)
{
	// IO board Serial communication 0xA00000 on bit0
}
/***************************************************************************/
WRITE16_MEMBER(nevada_state::io_board_x)
{
	// IO board Serial communication 0xA80000  on bit15
}

/***************************************************************************/
READ16_MEMBER(nevada_state::nevada_sec_r )
{
//  D3..D0 = DOOR OPEN or Track STATE of PAL35
	UINT16 res;
		/* UPPER byte is use for input in PAL35 */
			// 74LS173 $bits Register used LOWER bits D3..D0 for PAL35 state and DOOR LOGIC SWITCH
			res = pal35[m_datA40000 >> 8];
			res = res << 8;
			res = res | (m_datA40000 & 0x00FF);

	return res;
}
/***************************************************************************/
WRITE16_MEMBER(nevada_state::nevada_sec_w )
{
	// 74LS173 $bits Register used LOWER bits D3..D0 for DOOR LOGIC SWITCH
	m_datA40000 = data | 0x00f0;     // since D7..D4 are not used and are connected to PULLUP
//  popmessage("WRITE %04x %04x  ",datA40000,data);
}

/***************************************************************************/

/*
DOOR Switch desc
1 x FRONT DOOR SW
1 x REAR DOOR SW
1 x COIN DOOR SW
1 x TOP DOOR SW
1 x COMPUTER DOOR SW

Interrupt Vector
INT1  RTC MSM6242
INT2  nc
INT3  UART U39
INT4  UART U18
INT5  UART U40
INT6  nc
INT7  Seem to be used for confirmation of Ds1232 WDT work (need futher check)

U40 MC68681 RS232 UART  SIDEA = TouchScreen J1 (RS232)
U40 MC68681 RS232 UART  SIDEB = JCM Bill Acceptor (RS422)

U39 MC68681 RS232 UART SIDEA = Printer J3 (rs232)
U39 MC68681 RS232 UART SIDEB = Player Tracking Interface J2 (not used)

U18 MC68681 RS232 UART SIDEA = MODEM LOW SPEED 1200 BAUD from XECOM

U18 MC68681 PIN4  IP1  from U16 (75HC189 pin6) from PIN2 J90 UNKNOWN !
U18 MC68681 PIN36 IP2  ACCESS DOOR SWITCH
U18 MC68681 PIN2  IP3  LOW Battery Detector for ACCESS DOOR SWITCH
U18 MC68681 PIN39 IP4  from U12 75174  UNKNOWN !
U18 MC68681 PIN38 IP5  from U8 smart Battery (not used)

U39 MC68681 PIN7  IP0  Printer Data Terminal Ready
U39 MC68681 PIN4  IP1  COIN INPUT
U39 MC68681 PIN2  IP3  COIN INPUT
U39 MC68681 Pin38 IP5  JCM Bill Acceptor (J4-1,J4-2)
U39 MC68681 PIN29 OP0  Printer Clear To Send

U40 MC68681 PIN4  IP1  COIN INPUT
U40 MC68681 PIN2  IP3  COIN INPUT
U40 MC68681 PIN38 IP5  from U51 DS1260 smart Battery POWER FAIL FLAG

U40 MC68681 Pin12 OP1  JCM Bill Acceptor  (Enable Comm. U34)
U40 MC68681 Pin27 OP4  JCM Bill Acceptor  (J4-6, J4-7 Control)

// missing address for :
// external I/O board communication via PAL23

    A23 A22 A21 A20  | A19 A18 A17 A16 | A15 A14 A13 A12 | A11 A10 A9 A8 | A7 A6 A5 A4 | A3 A2 A1 xx
    Memory Map (generic)
    --------------------
    00000000 0000FFFF  NVRAM  (only 0..FFFF vector and jump)  16bits
    00010000 00011FFF  BACKUP1                                 8bits
    00020000 00021FFF  BACKUP2                                 8bits
    00B00000 00B01FFF  VIDEO RAM                               8bits
    00FA0000 00FBFFFF  Not used (Expension board  SRAM)
    00FC0000 00FDFFFF  PROGRAM                                16bits
    00FE0000 00FFFFFF  BOOTLOADER                             16bits

    00010001
    00014001
    00020001
    00024001
    00900001 6845  CS  CRTC U8  RS=LOW   ADDRESS REGISTER   (A15 ON RS)  Data bus on D7..D0
    00980001 6845  CS  CRTC U8  RS=HIGH  DATA   REGISTER    (A15 ON RS)
    00A00001           I/O Board Communication
    00A08001           I/O Board Communication
    00A10000 WDT    STROBE DS1232 WatchDog Controller (this address reset Strobe of Ds1232)
    00A20001 AY8912 BDIR  AUDIO  Data bus on D7..D0
    00A28001 AY8912 BC1   AUDIO
    00A300x1 6242  CS  RTC  U41 A0..A3   (A4..A7 ON A0..A3)  Data bus on D3..D0
    00A4000x PAL   CS  READ   U35 SECURITY PAL16R8           Data bus on D15..D8
    00A4000x PAL   CLK WRITE  U35 SECURITY PAL16R8
    00A4000x       CLK WRITE  U50 74LS173  for DOOR LOGIC SW on D3..D0
    00A70000 ???
    00B100x0 68681 CS  UART U40 RS1..RS4 REGISTER (A4..A7 ON RS1..RS4)  Data bus on D7..D0
    00B200x0 68681 CS  UART U39 RS1..RS4 REGISTER (A4..A7 ON RS1..RS4)  Data bus on D7..D0
    00E000x0 68681 CS  UART U18 RS1..RS4 REGISTER (A4..A7 ON RS1..RS4)  Data bus on D15..D8
*/
/***************************************************************************/
static ADDRESS_MAP_START( nevada_map, AS_PROGRAM, 16,nevada_state )
	AM_RANGE(0x00000000, 0x0000ffff) AM_RAM AM_SHARE("ram62256")
	AM_RANGE(0x00010000, 0x00021fff) AM_RAM AM_SHARE("backup")
	AM_RANGE(0x00900000, 0x00900001) AM_DEVWRITE8("crtc",mc6845_device, address_w,0x00ff )
	AM_RANGE(0x00908000, 0x00908001) AM_DEVWRITE8("crtc",mc6845_device,register_w,0x00ff )
	AM_RANGE(0x00a00000, 0x00a00001) AM_READWRITE(io_board_r,io_board_w)
	AM_RANGE(0x00a08000, 0x00a08001) AM_WRITE(io_board_x)
	AM_RANGE(0x00a10000, 0x00a10001) AM_WRITE(watchdog_reset16_w )
	AM_RANGE(0x00a20000, 0x00a20001) AM_DEVWRITE8("aysnd", ay8910_device, address_w, 0x00ff)
	AM_RANGE(0x00a28000, 0x00a28001) AM_DEVWRITE8("aysnd", ay8910_device, data_w, 0x00ff)
	AM_RANGE(0x00a30000, 0x00A300ff) AM_DEVREADWRITE8("rtc",msm6242_device, read, write, 0x00ff)
	AM_RANGE(0x00a40000, 0x00A40001) AM_READWRITE( nevada_sec_r, nevada_sec_w)
		//AM_RANGE(0x00b00000, 0x00b01fff) AM_RAM_WRITE(nevada_videoram_w) AM_BASE_MEMBER(nevada_state, m_videoram)
			AM_RANGE(0x00b00000, 0x00b01fff) AM_RAM // Video
	AM_RANGE(0x00b10000, 0x00b100ff) AM_DEVREADWRITE8( "duart40_68681", mc68681_device, read, write, 0x00ff ) // Lower byte
	AM_RANGE(0x00b20000, 0x00b200ff) AM_DEVREADWRITE8( "duart39_68681", mc68681_device, read, write, 0x00ff ) // Lower byte
	AM_RANGE(0x00e00000, 0x00e000ff) AM_DEVREADWRITE8( "duart18_68681", mc68681_device, read, write, 0xff00 ) // Upper byte
	AM_RANGE(0x00fa0000, 0x00fbffff) AM_RAM  // not used
	AM_RANGE(0x00fc0000, 0x00ffffff) AM_ROM  // ROM ext + ROM boot
ADDRESS_MAP_END


/***************************************************************************/
static ADDRESS_MAP_START( nevada_iomap, AS_IO, 8, nevada_state )
// todo

ADDRESS_MAP_END
/*
U18 MC68681 PIN4  IP1  from U16 (75HC189 pin6) from PIN2 J90 UNKNOWN !
U18 MC68681 PIN36 IP2  ACCESS DOOR SWITCH
U18 MC68681 PIN2  IP3  LOW Battery Detector for ACCESS DOOR SWITCH
U18 MC68681 PIN39 IP4  from U12 75174  UNKNOWN !
U18 MC68681 PIN38 IP5  from U8 smart Battery (not used)

U39 MC68681 PIN7  IP0  Printer Data Terminal Ready
U39 MC68681 PIN4  IP1  COIN INPUT
U39 MC68681 PIN2  IP3  COIN INPUT
U39 MC68681 Pin38 IP5  JCM Bill Acceptor (J4-1,J4-2)
U39 MC68681 PIN29 OP0  Printer Clear To Send
U39 MC68681 PIN27 OP4  ROM CHAR A14

U40 MC68681 PIN4  IP1  COIN INPUT
U40 MC68681 PIN2  IP3  COIN INPUT
U40 MC68681 PIN38 IP5  from U51 DS1260 smart Battery POWER FAIL FLAG

U40 MC68681 Pin12 OP1  JCM Bill Acceptor  (Enable Comm. U34)
U40 MC68681 Pin27 OP4  JCM Bill Acceptor  (J4-6, J4-7 Control)

*/
/***************************************************************************/
static INPUT_PORTS_START( nevada )
	PORT_START("DSW1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_BUTTON1  ) PORT_NAME("U16 pin6")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_BUTTON2  ) PORT_NAME("DOOR SW1")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_BUTTON3  ) PORT_NAME("LOW BATT on DOOR ACCESS")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_BUTTON4  ) PORT_NAME("U12 ??")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_BUTTON5  ) PORT_NAME("LOW BATT U8")
	PORT_START("DSW2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_BUTTON6  ) PORT_NAME("PRINTER READY")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_COIN1    )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_COIN2    )
	PORT_START("DSW3")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_COIN3    )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_COIN4    )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_BUTTON7  ) PORT_NAME("LOW BATT U51")
INPUT_PORTS_END

/***************************************************************************/
/*************************
*     Machine start      *
*************************/

MACHINE_START_MEMBER(nevada_state, nevada)
{
	m_nvram->set_base(m_ram62256, 0x1000);
}

/***************************************************************************/

/*************************
*     Machine Driver     *
*************************/

static MACHINE_CONFIG_START( nevada, nevada_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", M68000, MASTER_CPU)
	MCFG_CPU_PROGRAM_MAP(nevada_map)
	MCFG_CPU_IO_MAP(nevada_iomap)  //0x10000 0x20000

	MCFG_WATCHDOG_TIME_INIT(attotime::from_msec(150))   /* 150ms Ds1232 TD to Ground */

	MCFG_MACHINE_START_OVERRIDE(nevada_state, nevada)

	MCFG_NVRAM_ADD_CUSTOM_DRIVER("nvram", nevada_state, nvram_init)

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE((42+1)*8, (32+1)*8)                  /* From MC6845 init, registers 00 & 04 (programmed with value-1). */
	MCFG_SCREEN_VISIBLE_AREA(0*8, 31*8-1, 0*8, 31*8-1)    /* From MC6845 init, registers 01 & 06. */
	MCFG_SCREEN_UPDATE_DRIVER(nevada_state, screen_update_nevada)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", nevada)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(nevada_state, nevada)

	MCFG_MC6845_ADD("crtc", MC6845, "screen", MC6845_CLOCK)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8912, SOUND_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)

	MCFG_MC68681_ADD( "duart18_68681", XTAL_3_6864MHz )  // UARTA = Modem 1200Baud
	MCFG_MC68681_IRQ_CALLBACK(WRITELINE(nevada_state, duart18_irq_handler))
	MCFG_MC68681_INPORT_CALLBACK(IOPORT("DSW1"))

	MCFG_MC68681_ADD( "duart39_68681", XTAL_3_6864MHz )  // UARTA = Printer
	MCFG_MC68681_IRQ_CALLBACK(WRITELINE(nevada_state, duart39_irq_handler))
	MCFG_MC68681_INPORT_CALLBACK(IOPORT("DSW2"))

	MCFG_MC68681_ADD( "duart40_68681", XTAL_3_6864MHz )  // UARTA = Touch , UARTB = Bill Acceptor
	MCFG_MC68681_IRQ_CALLBACK(WRITELINE(nevada_state, duart40_irq_handler))
	MCFG_MC68681_A_TX_CALLBACK(DEVWRITELINE("microtouch", microtouch_device, rx))
	MCFG_MC68681_INPORT_CALLBACK(IOPORT("DSW3"))

	MCFG_MICROTOUCH_ADD( "microtouch", 9600, DEVWRITELINE("duart40_68681", mc68681_device, rx_a_w) )

	/* devices */
	MCFG_DEVICE_ADD("rtc", MSM6242, XTAL_32_768kHz)
	MCFG_MSM6242_OUT_INT_HANDLER(WRITELINE(nevada_state, nevada_rtc_irq))

MACHINE_CONFIG_END

/***************************************************************************/
ROM_START( nevada )
	ROM_REGION( 0x1000000, "maincpu", 0 )   /* 2 x 27C512 */
	ROM_LOAD16_BYTE( "u9even.bin" ,     0xfc0000, 0x010000, CRC(adb207bd) SHA1(3e3509b78fdf32785f92cb21272694673d25c563) )  // program fc0000..fdffff
	ROM_LOAD16_BYTE( "u10odd.bin" ,     0xfc0001, 0x010000, CRC(a79778d7) SHA1(6ff969f09d9781479360bca3403b927099ad6481) )

	ROM_LOAD16_BYTE( "u31even.bin",     0xfe0000, 0x010000, CRC(c9779f30) SHA1(5310b3d8b5e887313ce8059bd72d0730a295074f) )  // Boot fe0000..ffffff
	ROM_LOAD16_BYTE( "u32odd.bin" ,     0xfe0001, 0x010000, CRC(51035ed1) SHA1(66cbf582cdf34cf3dde30cf8a99bbced4af1ce6f) )

	ROM_REGION16_BE( 0x0010000, "defaults", 0 ) /* 2 x 62256  NVRAM */
	ROM_LOAD16_BYTE( "u30nv_even.bin",  0x000000, 0x008000, CRC(11f5c663) SHA1(f447fd59010bc7fbbda321c5aaf13e23c2aebd40) )  // NVRAM even  (RESET + Vector table in NVRAM)
	ROM_LOAD16_BYTE( "u33nv_odd.bin" ,  0x000001, 0x008000, CRC(20623da2) SHA1(2dd31a96f0a3454855cd975e8ee95e43316344e0) )  // NVRAM odd

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "u34char.bin",            0x00000  , 0x08000, CRC(6f1c6953) SHA1(e8db3b1d3fc3ec1c3dca155517553bd0000a2249) )  /* Characters */

		/*
		BACKUP RAM
		PAL DUMP
		*/
ROM_END

/***************************************************************************/
/*************************
*      Driver Init       *
*************************/
DRIVER_INIT_MEMBER(nevada_state,nevada)
{
	UINT16 *ROM = (UINT16 *)memregion("maincpu")->base();

	memset(m_backup,0x00,m_backup.bytes()); // temp

	/* Patch for WDT test with int Level7 */
	/* PATCH FE0086   4278 0414 Clrf $0414 */
	/* this skip the test for the WDT */
//  ROM[0xFE0086/2] = 0x4278;
//  ROM[0xFE0088/2] = 0x0414;

	// Skip PAL SECURITY
	ROM[0xFE0248/2] = 0x4E71; // nop
	ROM[0xFE05D0/2] = 0x4E71; // nop
	ROM[0xFE05D8/2] = 0x6014; // bra
	ROM[0xFE0606/2] = 0x600A; // bra
//  ROM[0xFE18B4/2] = 0x4E71; // nop


}
/***************************************************************************/

/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME     PARENT MACHINE INPUT   INIT    ROT    COMPANY    FULLNAME             FLAGS... */

GAME( 1995, nevada,  0,     nevada, nevada, nevada_state, nevada, ROT0, "VLC Inc.", "VLC Nevada",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
