// license:BSD-3-Clause
// copyright-holders:Yves
/*     vlc.cpp
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
INT7 initialization is needed to boot the game.

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


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68681.h"
#include "machine/microtch.h"
#include "machine/msm6242.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

#define MASTER_CLOCK    XTAL(16'000'000)
#define MASTER_CPU      ((MASTER_CLOCK)/2)    // 8mhz
#define SOUND_CLOCK     ((MASTER_CLOCK) /8)   // 2mhz

#define VIDEO_CLOCK     XTAL(33'000'000)
#define MC6845_CLOCK    ((VIDEO_CLOCK)/4/16)  // 0.515625 MHZ


/***************************************************************************

                                 General

***************************************************************************/


class nevada_state : public driver_device
{
public:
	nevada_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_duart(*this, {"duart18", "duart40", "duart39"}),
		m_rtc(*this, "rtc"),
		m_maincpu(*this, "maincpu"),
		m_microtouch(*this, "microtouch"),
		m_nvram(*this, "nvram"),
		m_ram62256(*this, "ram62256"),
		m_backup(*this, "backup"),
		m_vram(*this, "vram"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void nevada(machine_config &config);

	void init_nevada();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device_array<mc68681_device, 3> m_duart;
	required_device<msm6242_device> m_rtc;

	required_device<cpu_device> m_maincpu;
	optional_device<microtouch_device> m_microtouch;
	required_device<nvram_device> m_nvram;

	required_shared_ptr<uint16_t> m_ram62256;
	required_shared_ptr<uint16_t> m_backup;
	required_shared_ptr<uint16_t> m_vram;
	required_device<gfxdecode_device> m_gfxdecode;


	//uint8_t* m_videoram = nullptr;
	//uint8_t* m_colorram = nullptr;

	tilemap_t *m_tilemap = nullptr;
	uint16_t  m_datA40000 = 0U;

	void nvram_init(nvram_device &nvram, void *data, size_t size);
	uint32_t screen_update_nevada(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void nevada_palette(palette_device &palette) const;

	template<int N> uint8_t duart_r(offs_t offset);
	template<int N> void duart_w(offs_t offset, uint8_t data);
	uint8_t rtc_r(offs_t offset);
	void rtc_w(offs_t offset, uint8_t data);
	uint16_t io_board_r();
	void io_board_w(uint16_t data);
	void io_board_x(uint16_t data);
	uint16_t nevada_sec_r();
	void nevada_sec_w(uint16_t data);
	void vram_w(offs_t offset, uint16_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void nevada_map(address_map &map) ATTR_COLD;
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
static const uint8_t pal35[256] = {
	0x11, 0x42, 0x5b, 0xca, 0x19, 0x42, 0x5b, 0xca, 0x38, 0x63, 0x3a, 0x63, 0x3a, 0x63, 0x3a, 0x63,
	0xd3, 0x08, 0x5b, 0xca, 0x19, 0xca, 0x19, 0xca, 0x18, 0xeb, 0x18, 0xeb, 0x18, 0xeb, 0x18, 0xeb,
	0xd3, 0xca, 0x5b, 0xcc, 0x5b, 0xcc, 0x5b, 0xcc, 0xba, 0x63, 0x38, 0x65, 0x38, 0x65, 0x38, 0x65,
	0xd1, 0xca, 0x5b, 0xc8, 0x5b, 0xc8, 0x5b, 0xc8, 0x9a, 0xeb, 0x1a, 0xed, 0x1a, 0xed, 0x1a, 0xed,
	0x0c, 0x65, 0xf0, 0x00, 0x64, 0xf5, 0x04, 0x65, 0xb8, 0x25, 0x20, 0x20, 0x24, 0x24, 0x24, 0x24,
	0xf0, 0x00, 0xf8, 0x00, 0xfc, 0x00, 0xfc, 0x00, 0xb8, 0x3d, 0xfc, 0x19, 0xfc, 0x19, 0xfc, 0x19,
	0x44, 0xf9, 0xc4, 0xf9, 0xc4, 0xfd, 0xc4, 0xfd, 0xfc, 0xfd, 0xfc, 0xfd, 0xfc, 0xfd, 0xfc, 0xfd,
	0xc0, 0xd9, 0xc8, 0xd9, 0xc8, 0xd9, 0xc8, 0xd9, 0xd8, 0xd9, 0xd8, 0xd9, 0xd8, 0xd9, 0xd8, 0xd9,
	0x0c, 0x44, 0xfb, 0x04, 0x67, 0xd4, 0x0c, 0x44, 0xba, 0x24, 0x22, 0x02, 0x26, 0x06, 0x26, 0x06,
	0xfb, 0x00, 0xfb, 0x00, 0xef, 0x10, 0xce, 0x18, 0xef, 0x18, 0xef, 0x18, 0xff, 0x18, 0xff, 0x18,
	0x44, 0xf8, 0xc6, 0xd8, 0xce, 0xdc, 0xce, 0xdc, 0xef, 0x9e, 0x67, 0xb8, 0x67, 0xbc, 0x67, 0xbc,
	0xc6, 0xd8, 0xca, 0xd8, 0xca, 0xd8, 0xca, 0xd8, 0xcb, 0x9a, 0xef, 0x9a, 0xff, 0xd8, 0xdb, 0xd8,
	0x66, 0xf4, 0x00, 0x64, 0xba, 0x25, 0x22, 0x22, 0x26, 0x26, 0x26, 0x26, 0xf2, 0x00, 0xfa, 0x00,
	0xfa, 0x00, 0xfa, 0x00, 0xba, 0x3d, 0xfa, 0x19, 0xfa, 0x19, 0xfa, 0x19, 0x44, 0xf9, 0xc2, 0xf0,
	0xc2, 0xf4, 0xc2, 0xf4, 0xfa, 0xf4, 0xfa, 0xf4, 0xfa, 0xf4, 0xfa, 0xf4, 0xc2, 0xd0, 0xca, 0xd0,
	0xca, 0xd0, 0xca, 0xd0, 0xda, 0xd0, 0xda, 0xd0, 0xda, 0xd0, 0xda, 0xd0, 0x08, 0x63, 0xd3, 0x08 };


/***************************************************************************/
/********************   VIDEO SECTION   ************************************/
/***************************************************************************/

static const gfx_layout charlayout =
{
	/* Todo  , just for sample */

	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 5*4, 4*4, 7*4, 6*4, 1*4, 0*4, 3*4, 2*4 },
	{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 },
	8*8*4
};

/***************************************************************************/

void nevada_state::vram_w(offs_t offset, uint16_t data)
{
// Todo, Just for sample

	m_vram[offset] = data;
	m_tilemap->mark_tile_dirty(offset / 2);

}

/***************************************************************************/
static GFXDECODE_START( gfx_nevada )
	/* Todo  , just for sample */
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout,   0, 8 )
GFXDECODE_END

/***************************************************************************/

TILE_GET_INFO_MEMBER( nevada_state::get_bg_tile_info )
{
	//int attr = m_colorram[tile_index];
	int code = m_vram[tile_index*2+1];
	//int bank = (attr & 0x02) >> 1;
	//int color = (attr & 0x3c) >> 2;

	tileinfo.set(0, code, 0, 0);

}


/***************************************************************************/
void nevada_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(nevada_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 31,31);
}

/***************************************************************************/
uint32_t nevada_state::screen_update_nevada(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

/***************************************************************************/
void nevada_state::nevada_palette(palette_device &palette) const
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
    Interrupt 4 (autovectored)

    U39 MC68681 RS232 UART SIDEA = Printer J3 (rs232)
    U39 MC68681 RS232 UART SIDEB = Player Tracking Interface J2 (not used)
    Interrupt 3 (autovectored)

    68681 DUART <-> Microtouch touch screen controller communication
    U40 MC68681 RS232 UART  SIDEA = TouchScreen J1 (RS232)
    U40 MC68681 RS232 UART  SIDEB = JCM Bill Acceptor (RS422)
    Interrupt 5 (autovectored)
***************************************************************************/

template<int N>
uint8_t nevada_state::duart_r(offs_t offset)
{
	return m_duart[N]->read(offset >> 3);
}

template<int N>
void nevada_state::duart_w(offs_t offset, uint8_t data)
{
	m_duart[N]->write(offset >> 3, data);
}

/***************************************************************************/
/*********************    RTC SECTION       ********************************/
/***************************************************************************/

uint8_t nevada_state::rtc_r(offs_t offset)
{
	return m_rtc->read(offset >> 3);
}

void nevada_state::rtc_w(offs_t offset, uint8_t data)
{
	m_rtc->write(offset >> 3, data);
}


/***************************************************************************/
uint16_t nevada_state::io_board_r()
{
	// IO board Serial communication 0xA00000
	return 1;
}
/***************************************************************************/
void nevada_state::io_board_w(uint16_t data)
{
	// IO board Serial communication 0xA00000 on bit0
}
/***************************************************************************/
void nevada_state::io_board_x(uint16_t data)
{
	// IO board Serial communication 0xA80000  on bit15
}

/***************************************************************************/
uint16_t nevada_state::nevada_sec_r()
{
//  D3..D0 = DOOR OPEN or Track STATE of PAL35
	uint16_t res;
		/* UPPER byte is use for input in PAL35 */
			// 74LS173 $bits Register used LOWER bits D3..D0 for PAL35 state and DOOR LOGIC SWITCH
			res = pal35[m_datA40000 >> 8];
			res = res << 8;
			res = res | (m_datA40000 & 0x00FF);

	return res;
}
/***************************************************************************/
void nevada_state::nevada_sec_w(uint16_t data)
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
void nevada_state::nevada_map(address_map &map)
{
	map(0x00000000, 0x0000ffff).ram().share("ram62256");
	map(0x00010000, 0x00021fff).ram().share("backup");
	map(0x00900001, 0x00900001).w("crtc", FUNC(mc6845_device::address_w));
	map(0x00908001, 0x00908001).w("crtc", FUNC(mc6845_device::register_w));
	map(0x00a00000, 0x00a00001).rw(FUNC(nevada_state::io_board_r), FUNC(nevada_state::io_board_w));
	map(0x00a08000, 0x00a08001).w(FUNC(nevada_state::io_board_x));
	map(0x00a10000, 0x00a10001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x00a20001, 0x00a20001).w("aysnd", FUNC(ay8910_device::address_w));
	map(0x00a28001, 0x00a28001).w("aysnd", FUNC(ay8910_device::data_w));
	map(0x00a30000, 0x00a30001).select(0xf0).rw(FUNC(nevada_state::rtc_r), FUNC(nevada_state::rtc_w)).umask16(0x00ff);
	map(0x00a40000, 0x00a40001).rw(FUNC(nevada_state::nevada_sec_r), FUNC(nevada_state::nevada_sec_w));
	map(0x00b00000, 0x00b03fff).ram().w(FUNC(nevada_state::vram_w)).share("vram");
	map(0x00b10000, 0x00b10001).select(0xf0).rw(FUNC(nevada_state::duart_r<1>), FUNC(nevada_state::duart_w<1>)).umask16(0x00ff); // Lower byte
	map(0x00b20000, 0x00b20001).select(0xf0).rw(FUNC(nevada_state::duart_r<2>), FUNC(nevada_state::duart_w<2>)).umask16(0x00ff); // Lower byte
	map(0x00e00000, 0x00e00001).select(0xf0).rw(FUNC(nevada_state::duart_r<0>), FUNC(nevada_state::duart_w<0>)).umask16(0xff00); // Upper byte
	map(0x00fa0000, 0x00fbffff).ram();  // not used
	map(0x00fc0000, 0x00ffffff).rom();  // ROM ext + ROM boot
}


/***************************************************************************/

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

void nevada_state::machine_start()
{
	m_nvram->set_base(m_ram62256, 0x1000);
}

/***************************************************************************/

/*************************
*     Machine Driver     *
*************************/

void nevada_state::nevada(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, MASTER_CPU);
	m_maincpu->set_addrmap(AS_PROGRAM, &nevada_state::nevada_map);

	WATCHDOG_TIMER(config, "watchdog").set_time(attotime::from_msec(150));   /* 150ms Ds1232 TD to Ground */

	NVRAM(config, "nvram").set_custom_handler(FUNC(nevada_state::nvram_init));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size((42+1)*8, (32+1)*8);                  /* From MC6845 init, registers 00 & 04 (programmed with value-1). */
	screen.set_visarea(0*8, 31*8-1, 0*8, 31*8-1);    /* From MC6845 init, registers 01 & 06. */
	screen.set_screen_update(FUNC(nevada_state::screen_update_nevada));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_nevada);
	PALETTE(config, "palette", FUNC(nevada_state::nevada_palette), 256);

	mc6845_device &crtc(MC6845(config, "crtc", MC6845_CLOCK));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	AY8912(config, "aysnd", SOUND_CLOCK).add_route(ALL_OUTPUTS, "mono", 0.75);

	MC68681(config, m_duart[0],  XTAL(3'686'400));  // UARTA = Modem 1200Baud
	m_duart[0]->irq_cb().set_inputline(m_maincpu, M68K_IRQ_4);
	m_duart[0]->inport_cb().set_ioport("DSW1");

	MC68681(config, m_duart[1],  XTAL(3'686'400));  // UARTA = Printer
	m_duart[1]->irq_cb().set_inputline(m_maincpu, M68K_IRQ_3);
	m_duart[1]->inport_cb().set_ioport("DSW2");

	MC68681(config, m_duart[2],  XTAL(3'686'400));  // UARTA = Touch , UARTB = Bill Acceptor
	m_duart[2]->irq_cb().set_inputline(m_maincpu, M68K_IRQ_5);
	m_duart[2]->a_tx_cb().set(m_microtouch, FUNC(microtouch_device::rx));
	m_duart[2]->inport_cb().set_ioport("DSW3");

	MICROTOUCH(config, m_microtouch, 9600).stx().set(m_duart[1], FUNC(mc68681_device::rx_a_w));

	/* devices */
	MSM6242(config, m_rtc, XTAL(32'768));
	m_rtc->out_int_handler().set_inputline(m_maincpu, M68K_IRQ_1);  // rtc interrupt on INT1
}

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
void nevada_state::init_nevada()
{
	uint16_t *ROM = (uint16_t *)memregion("maincpu")->base();

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

} // Anonymous namespace


/*************************
*      Game Drivers      *
*************************/

//    YEAR  NAME     PARENT MACHINE INPUT   STATE         INIT         ROT   COMPANY     FULLNAME             FLAGS
GAME( 1995, nevada,  0,     nevada, nevada, nevada_state, init_nevada, ROT0, "VLC Inc.", "VLC Nevada",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
