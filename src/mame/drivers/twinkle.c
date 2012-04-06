/*

Twinkle System

driver by smf and R. Belmont

TODO:

sound (IDE DMA, finish comms)
dvd
hard drive
16seg led font



Konami Twinkle Hardware Overview
Konami 1999-2002

The following games are known to exist on this hardware (there may be more)
                                                                  Video CD     Security
Game Title                       Year            Program CD     6/7/8 use DVD  Dongle      HDD label
-----------------------------------------------------------------------------------------------------
beatmania IIDX (English)       - Konami 1999     GQ863 A01        GQ863 A04    863 A02     Possibly same as Japan version?
beatmania IIDX (Japanese)      - Konami 1999     GQ863-JA B01     GQ863 A04     "   "      C44 JA A03*
beatmania IIDX + DDR Club Kit  - Konami 1999     896 JA ABM       ?            ?           ?
beatmania IIDX Substream       - Konami 1999     ?                GC983 A04    ?           Dumped#
beatmania IIDX Club Version 2  - Konami 1999     GE984 A01(BM)    ?            984 A02     ?
                                               + GE984 A01(DDR)
beatmania IIDX 2nd Style       - Konami 1999     GC985 A01        GC985 A04    ?           ?
beatmania IIDX 3rd Style       - Konami 2000     GC992-JA A01     GC992-JA A04 ?           ?
beatmania IIDX 3rd Style(newer)- Konami 2000     GC992-JA C01     GC992-JA A04 ?           ?
beatmania IIDX 4th Style       - Konami 2000     A03 JA A01       A03 JA A02   A03         A03 JA A03
beatmania IIDX 5th Style       - Konami 2001     A17 JA A01       A17 JA A02   ?           ?
beatmania IIDX 6th Style       - Konami 2001     B4U JA A01       B4U JA A02   ?           B4U JA A03
beatmania IIDX 6th Style(newer)- Konami 2001     B4U JA B01       B4U JA A02   ?           B4U JA A03
beatmania IIDX 7th Style       - Konami 2002     B44 JA A01       B44 JA A02   ?           ?
beatmania IIDX 8th Style       - Konami 2002     C44 JA A01       ?            C44         ?

? = Undumped pieces.
# = Dumped but code unknown.
* = Came with beatmania IIDX main board but might be for 8th Style (i.e. game C44)?
If you can help, please contact us at http://guru.mameworld.info or http://mamedev.org/contact.html


The Konami Twinkle hardware basically consists of the following parts....
3 PCBs sandwiched together in a metal box
SCSI CDROM drive (for main program CD)
External DVD player (plays VCD)
CD disc (for main program)
Security dongle
IDE HDD (for audio)
VCD disc (for video)

The top board appears to be the main CPU/graphics board.
The middle board appears to be for the video output/overlay.
The bottom board appears to be for sound.


PCB Layouts
-----------

(Top)

TWINKLE/MAIN
GQ751 PWB(A2)0000039085
|-----------------------------------------------------------------------------|
|CN11      CN1           CN8                   CN7      CN5                   |
|PQ30RV21      32MHz                                                          |
|      53CF96-2.13M              24MHz            TD62083         ADM238LJR   |
|SW                                   SMC                         ADM238LJR   |
|  LTC1232.10P          M66011FP      FDC37C665GT.12G                         |
|                                                           MACH111.11C DSW(8)|
|                                                                             |
|                                                              RTC-65271.9B   |
|                          |----------|                                       |
|                          |          |                        863A03.7B      |
|                          |SONY      |          D481850GF-A12.8E             |
|              KM48V514.8L |CXD8561Q  |                                       |
|                          |@8J       |                                       |
|              KM48V514.7L |----------|          D481850GF-A12.6E             |
|                      67.737MHz                                              |
|              KM48V514.6L                                                    |
|                     53.693MHz|----------|                                   |
|              KM48V514.5L     |          |                                   |
|                              |SONY      |      KM416V256BT-7.4E          LED|
|              KM48V514.4L     |CXD8530CQ |                                LED|
|                              |@4J       |                                LED|
|78L05         KM48V514.3L     |----------|                                LED|
|                                          |-------|           TD62083     LED|
|     MC44200  KM48V514.2L                 |SONY   |           TD62083     LED|
|                                          |CXD2925|                       LED|
|              KM48V514.1L                 |-------|                       LED|
|     CN12                          CNx     @3F                     CN9    LED|
|-----------------------------------------------------------------------------|
Notes:
      CN1        - 50 pin SCSI connector for CDROM drive
      CN5        - 9 pin DSUB RS-232 connector
      CN7        - Panel/controls connector
      CN8        - RJ45 network connector
      CN9        - 40 pin flat cable I/O connector
      CNx        - Security card connector. The security cart contains only
                   one IC, a ST 2402W 2kbit serial I2C bus EEPROM (TSSOP8)
                   It appears only the first 12 bytes are actually used in this chip. The earlier games
                   (possibly from BMIIDX 2nd Style and before) use a similar IC but it is not directly
                   readable like the 2402W. The chip is covered with epoxy so the actual type is unknown.
      CN11       - DC power input connector
      CN12       - 15 pin DSUB connector (computer-generated video graphics output)
      SW         - Reset or test switch
      MACH111    - AMD MACH111 high-performance electrically erasable CMOS programmable logic device, stamped '38471'
      53CF96     - Symbios Logic 53CF96 SCSI chip
      PQ30RV21   - Sharp PQ30RV21 voltage regulator
      TD62083    - Toshiba TD62083 8ch darlington sink driver
      ADM238LJR  - Analog Devices ADM238LJR 5V CMOS RS-232 driver/receiver
      LTC1232    - Linear Technology Corporation LTC1232 microprocessor supervisory circuit (DIP8)
      M66011FP   - Renesas M66011FP serial bus controller
      FDC37C665GT- SMC FDC37C665/666GT high-performance multi-mode parallel port super I/O floppy disk controller
      RTC-65271  - Epson Toyocom RTC-65271 real-time clock
      D481850GF  - NEC D481850GF-A12 128k x 32Bit x 2 Banks SGRAM (QFP100)
      CXD2925Q   - Sony CXD2925Q SPU (QFP100)
      CXD8561Q   - Sony CXD8561Q GPU (QFP208)
      CXD8530CQ  - Sony CXD8530CQ R3000-based CPU (QFP208)
      MC44200FT  - Motorola MC44200FT Triple 8-bit Video DAC (QFP44)
      KM48V514   - Samsung Electronics KM48V514BJ-6 512kx8 EDO DRAM (SOJ28)
      KM416V256  - Samsung Electronics KM416V256BT-7 256kx16 DRAM (TSOP44/40)
      863A03.7B  - 27C040 EPROM (DIP32)


(Middle)

TWINKLE/SUB2
GQ860 PWB(A1)0000053591
|-----------------------------------------------------------------------------|
|  CN1                     CN11           RCA   CN7           RCA             |
|                                                                             |
|                             MC141685  MC141685      AD817  |--------| AD817 |
|                                            AD724JR         |Bt812KPF|       |
|                                                14.3182MHz  |VIDEO   |       |
|                                                            |DECODER |       |
|                                                            |--------|       |
|                                                                   26.8465MHz|
|                                                                             |
|                                                            D42280GU-30.11B  |
|                                                            D42280GU-30.11A  |
|                                                            D42280GU-30.9B   |
|                                                            D42280GU-30.9A   |
|                 M5118165B-60J.9G                           D42280GU-30.7B   |
|                                                            D42280GU-30.7A   |
|                                                                             |
|                 M5118165B-60J.7G                                            |
|                                                                             |
|                                                                             |
|                                                                             |
|                                                                             |
|                                                                             |
|                                       XC9572.5D                             |
|                                                         XC9572.5B  XC9572.5A|
|                                                                             |
|                       XC9572.2F                                    XC9536.3A|
|                 XC9436.2G    XC9536.2E                                      |
|-----------------------------------------------------------------------------|
Notes:
      CN1      - DC power input connector
      CN7      - 5 pin plug for connection and control of external DVD player for background video
      CN11     - 15 pin DSUB connector
      RCA      - Yellow RCA connectors for video (input and/or output?) from external DVD player
      MC141685 - Motorola MC141685 low cost 3CH D/A convertor
      AD817    - Analog Devices 18V high speed low power wide supply range amplifier
      AD724JR  - Analog Devices 6V 800mW 250MHz RGB to NTSC/PAL encoder
      Bt812KPF - Conexant Systems Inc. Bt812KPF NTSC/PAL to RGB/YCrCb decoder / video codec (QFP160)
      D42280GU - NEC uPD42280 high-speed field buffer with 256kx8 FIFO memory (SOP28)
      M5118165B- Oki Semiconductor M5118165B-60J 1M x 16 EDO DRAM
      XC9572   - Xilinx XC9572 in-system-programmable CPLD (PLCC44)
      XC9536   - Xilinx XC9536 in-system-programmable CPLD (PLCC44)


(Bottom)

TWINKLE/SPU
GQ863 PWB(A2)0000057606
|-----------------------------------------------------------------------------|
|  CN5    7805       CN7   CN1                GM76C8128CLLFW70.18J            |
|                                             GM76C8128CLLFW70.16J      DSW(8)|
|               SM5875  SM5875 |-------|                                      |
|SW                            |RICOH  |                                      |
|                              |RF5C400|           HY5117404BJ-60.15J         |
|                |-------|     |-------|           HY5117404BJ-60.15F         |
|                |M65851 |      @16L               HY5117404BJ-60.15C         |
|                |@14R   |                         HY5117404BJ-60.14J         |
|                |-------|    33.8688MHz           HY5117404BJ-60.14F         |
|                                                  HY5117404BJ-60.14C         |
|                                                  HY5117404BJ-60.13J         |
|                                                  HY5117404BJ-60.13F         |
|                                                  HY5117404BJ-60.13C         |
|                               CY7C131.10M        HY5117404BJ-60.12J         |
|                                                  HY5117404BJ-60.12F         |
|                                                  HY5117404BJ-60.12C         |
|                                                                             |
|                                                                             |
|                                                                          LED|
|                                                                          LED|
|                               XC9572.6L                                  LED|
|                                                                          LED|
|       LTC1232.5W         XC9536.4N  XC9536.4K                            LED|
|                32MHz                                                     LED|
|GM76C8128CLLFW70.4Y  68000-16.3S                                          LED|
|GM76C8128CLLFW70.4W                                                       LED|
|863A05.2X                                                        CN4         |
|-----------------------------------------------------------------------------|
Notes:
      CN1      - RCA left/right audio output
      CN4      - 40 pin flat cable connector for HDD data cable
      CN5      - DC power input connector
      CN7      - RCA left/right audio output
      SM5875   - Nippon Precision Circuits SM5875 2-channel D/A convertor (SSOP24)
      RF5C400  - Ricoh RF5C400 PCM 32Ch, 44.1 kHz Stereo, 3D Effect Spatializer, clock input 16.9344MHz [33.8688/2]
      M65851   - Mitsubishi M65851 single chip karaoke sound processor IC (QFP80)
      HY5117404- Hyundai Semiconductor HY5117404BJ-60 4M x 4-Bit CMOS EDO DRAM
      CY7C131  - Cypress Semiconductor CY7C131 1kx8 dual-port static RAM
      XC9572   - Xilinx XC9572 in-system-programmable CPLD (PLCC44)
      XC9536   - Xilinx XC9536 in-system-programmable CPLD (PLCC44)
      LTC1232  - Linear Technology Corporation LTC1232 microprocessor supervisory circuit (DIP8)
      68000    - Clock input 16.000MHz [32/2]
      GM76C8128- LG GM76C8128CLLFW70 128kx8 low power CMOS static RAM
      863A05.2X- 27C4096 EPROM (DIP40)


*/

#include "emu.h"
#include "cpu/psx/psx.h"
#include "cpu/m68000/m68000.h"
#include "video/psx.h"
#include "includes/psx.h"
#include "machine/am53cf96.h"
#include "machine/rtc65271.h"
#include "machine/i2cmem.h"
#include "machine/idectrl.h"
#include "sound/spu.h"
#include "sound/cdda.h"
#include "sound/rf5c400.h"

class twinkle_state : public psx_state
{
public:
	twinkle_state(const machine_config &mconfig, device_type type, const char *tag)
		: psx_state(mconfig, type, tag) { }

	UINT16 m_spu_ctrl;		// SPU board control register
	UINT8 m_spu_shared[0x400];	// SPU/PSX shared dual-ported RAM
	UINT32 m_unknown;

	int m_io_offset;
	int m_output_last[ 0x100 ];
	int m_last_io_offset;
	UINT8 m_sector_buffer[ 4096 ];
	DECLARE_WRITE32_MEMBER(twinkle_io_w);
	DECLARE_READ32_MEMBER(twinkle_io_r);
	DECLARE_WRITE32_MEMBER(twinkle_output_w);
	DECLARE_WRITE32_MEMBER(serial_w);
	DECLARE_WRITE32_MEMBER(shared_psx_w);
	DECLARE_READ32_MEMBER(shared_psx_r);
	DECLARE_WRITE16_MEMBER(twinkle_spu_ctrl_w);
	DECLARE_READ16_MEMBER(twinkle_waveram_r);
	DECLARE_WRITE16_MEMBER(twinkle_waveram_w);
	DECLARE_READ16_MEMBER(shared_68k_r);
	DECLARE_WRITE16_MEMBER(shared_68k_w);
};

/* RTC */

#define LED_A1 0x0001
#define LED_A2 0x0002
#define LED_B 0x0004
#define LED_C 0x0008
#define LED_D1 0x0010
#define LED_D2 0x0020
#define LED_E 0x0040
#define LED_F 0x0080
#define LED_G1 0x0100
#define LED_G2 0x0200
#define LED_H 0x2000
#define LED_I 0x0400
#define LED_J 0x4000
#define LED_K 0x8000
#define LED_L 0x0800
#define LED_M 0x1000

//   A1  A2
//  F H I J B
//   G1  G2
//  E M L K C
//   D2  D1


static const UINT16 asciicharset[]=
{
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_E | LED_F | LED_J | LED_M, // 0
	LED_B | LED_C, // 1
	LED_A1 | LED_A2 | LED_B | LED_D1 | LED_D2 | LED_E | LED_G1  | LED_G2, // 2
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_G2, // 3
	LED_B | LED_C | LED_F | LED_G1 | LED_G2 , // 4
	LED_A1 | LED_A2 | LED_D1 | LED_D2 | LED_F | LED_G1 | LED_K, // 5
	LED_A1 | LED_A2 | LED_C | LED_D1 | LED_D2 | LED_E | LED_F | LED_G1 | LED_G2, // 6
	LED_A1 | LED_A2 | LED_B | LED_C, // 7
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_E | LED_F, // 8
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_F, // 9
	LED_A1 | LED_A2 | LED_B | LED_C | LED_E | LED_F, // A
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_G2 | LED_I | LED_L, // B
	LED_A1 | LED_A2 | LED_D1 | LED_D2 | LED_E | LED_F, // C
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_I | LED_L, // D
	LED_A1 | LED_A2 | LED_D1 | LED_D2 | LED_E | LED_F | LED_G1 | LED_G2, // E
	LED_A1 | LED_A2 | LED_E | LED_F | LED_G1, // F
// 16
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
// 32
	0, // space
	0, // !
	0, // "
	0, // #
	0, // $
	0, // %
	0, // &
	0, // '
	0, // (
	0, // )
	0, // *
	0, // +
	0, // ,
	0, // -
	0, // .
	0, // /
// 48
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_E | LED_F | LED_J | LED_M, // 0
	LED_B | LED_C, // 1
	LED_A1 | LED_A2 | LED_B | LED_D1 | LED_D2 | LED_E | LED_G1  | LED_G2, // 2
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_G2, // 3
	LED_B | LED_C | LED_F | LED_G1 | LED_G2 , // 4
	LED_A1 | LED_A2 | LED_D1 | LED_D2 | LED_F | LED_G1 | LED_K, // 5
	LED_A1 | LED_A2 | LED_C | LED_D1 | LED_D2 | LED_E | LED_F, // 6
	LED_A1 | LED_A2 | LED_B | LED_C, // 7
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_E | LED_F, // 8
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_F, // 9
	0, // :
	0, // ;
	0, // <
	0, // =
	0, // >
	0, // ?
// 64
	0, // @
	LED_A1 | LED_A2 | LED_B | LED_C | LED_E | LED_F, // A
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_G2 | LED_I | LED_L, // B
	LED_A1 | LED_A2 | LED_D1 | LED_D2 | LED_E | LED_F, // C
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_I | LED_L, // D
	LED_A1 | LED_A2 | LED_D1 | LED_D2 | LED_E | LED_F | LED_G1 | LED_G2, // E
	LED_A1 | LED_A2 | LED_E | LED_F | LED_G1, // F
	LED_A1 | LED_A2 | LED_C | LED_D1 | LED_D2 | LED_E | LED_F | LED_G2, // G
	LED_B | LED_C | LED_E | LED_F | LED_G1 | LED_G2, // H
	LED_I | LED_L, // I
	LED_B | LED_C | LED_D1 | LED_D2 | LED_E, // J
	LED_E | LED_F | LED_G1 | LED_J | LED_K, // K
	LED_D1 | LED_D2 | LED_E | LED_F, // L
	LED_B | LED_C | LED_E | LED_F | LED_H | LED_J, // M
	LED_B | LED_C | LED_E | LED_F | LED_H | LED_K, // N
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_E | LED_F, // O
	LED_A1 | LED_A2 | LED_B | LED_E | LED_F, // P
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_E | LED_F | LED_K, // Q
	LED_A1 | LED_A2 | LED_B | LED_E | LED_F | LED_K, // R
	LED_A1 | LED_A2 | LED_C | LED_D1 | LED_D2 | LED_F | LED_G1 | LED_G2, // S
	LED_A1 | LED_A2 | LED_I | LED_L, // T
	LED_B | LED_C | LED_D1 | LED_D2 | LED_E | LED_F, // O
	LED_E | LED_F | LED_M | LED_J, // V
	LED_B | LED_C | LED_E | LED_F | LED_M | LED_K, // W
	LED_H | LED_J | LED_K | LED_M, // X
	LED_H | LED_J | LED_L, // Y
	LED_A1 | LED_A2 | LED_D1 | LED_D2 | LED_J | LED_M, // Z
	0, // [
	0, // "\"
	0, // ]
	0, // ^
	0, // _
	0, // `
	LED_A1 | LED_A2 | LED_B | LED_C | LED_E | LED_F, // A
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_G2 | LED_I | LED_L, // B
	LED_A1 | LED_A2 | LED_D1 | LED_D2 | LED_E | LED_F, // C
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_I | LED_L, // D
	LED_A1 | LED_A2 | LED_D1 | LED_D2 | LED_E | LED_F | LED_G1 | LED_G2, // E
	LED_A1 | LED_A2 | LED_E | LED_F | LED_G1, // F
	LED_A1 | LED_A2 | LED_C | LED_D1 | LED_D2 | LED_E | LED_F | LED_G2, // G
	LED_B | LED_C | LED_E | LED_F | LED_G1 | LED_G2, // H
	LED_I | LED_L, // I
	LED_B | LED_C | LED_D1 | LED_D2 | LED_E, // J
	LED_E | LED_F | LED_G1 | LED_J | LED_K, // K
	LED_D1 | LED_D2 | LED_E | LED_F, // L
	LED_B | LED_C | LED_E | LED_F | LED_H | LED_J, // M
	LED_B | LED_C | LED_E | LED_F | LED_H | LED_K, // N
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_E | LED_F, // O
	LED_A1 | LED_A2 | LED_B | LED_E | LED_F, // P
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_E | LED_F | LED_K, // Q
	LED_A1 | LED_A2 | LED_B | LED_E | LED_F | LED_K, // R
	LED_A1 | LED_A2 | LED_C | LED_D1 | LED_D2 | LED_F | LED_G1 | LED_G2, // S
	LED_A1 | LED_A2 | LED_I | LED_L, // T
	LED_B | LED_C | LED_D1 | LED_D2 | LED_E | LED_F, // O
	LED_E | LED_F | LED_M | LED_J, // V
	LED_B | LED_C | LED_E | LED_F | LED_M | LED_K, // W
	LED_H | LED_J | LED_K | LED_M, // X
	LED_H | LED_J | LED_L, // Y
	LED_A1 | LED_A2 | LED_D1 | LED_D2 | LED_J | LED_M, // Z
	0, // {
	0, // |
	0, // }
	0, // ~
	0, //
};

WRITE32_MEMBER(twinkle_state::twinkle_io_w)
{


	if( ACCESSING_BITS_16_23 )
	{
		m_io_offset = ( data >> 16 ) & 0xff;
	}
	if( ACCESSING_BITS_0_7 )
	{
		if( m_output_last[ m_io_offset ] != ( data & 0xff ) )
		{
			m_output_last[ m_io_offset ] = ( data & 0xff );

			switch( m_io_offset )
			{
				/* ? */
			case 0x07:
			case 0x0f:
			case 0x17:
			case 0x1f:
			case 0x27:
			case 0x2f:
			case 0x37:

				/* led */
			case 0x3f:
			case 0x47:
			case 0x4f:
			case 0x57:
			case 0x5f:
			case 0x67:
			case 0x6f:
			case 0x77:
			case 0x7f:
				output_set_indexed_value( "led", ( m_io_offset - 7 ) / 8, asciicharset[ ( data ^ 0xff ) & 0x7f ] );
				break;

			case 0x87:
				output_set_indexed_value( "spotlight", 0, ( ~data >> 3 ) & 1 );
				output_set_indexed_value( "spotlight", 1, ( ~data >> 2 ) & 1 );
				output_set_indexed_value( "spotlight", 2, ( ~data >> 1 ) & 1 );
				output_set_indexed_value( "spotlight", 3, ( ~data >> 0 ) & 1 );
				output_set_indexed_value( "spotlight", 4, ( ~data >> 4 ) & 1 );
				output_set_indexed_value( "spotlight", 5, ( ~data >> 5 ) & 1 );
				output_set_indexed_value( "spotlight", 6, ( ~data >> 6 ) & 1 );
				output_set_indexed_value( "spotlight", 7, ( ~data >> 7 ) & 1 );
				break;

			case 0x8f:
				output_set_value( "neonlamp", ~data & 1 );

				if( ( data & 0xfe ) != 0xfe )
				{
					printf("%02x = %02x\n", m_io_offset, data & 0xff );
				}
				break;

			default:
				printf( "unknown io %02x = %02x\n", m_io_offset, data & 0xff );
				break;
			}
		}
	}
}

READ32_MEMBER(twinkle_state::twinkle_io_r)
{

	UINT32 data = 0;

	if( ACCESSING_BITS_0_7 )
	{
		switch( m_io_offset )
		{
			case 0x07:
				data |= input_port_read( machine(), "IN0" );
				break;

			case 0x0f:
				data |= input_port_read( machine(), "IN1" );
				break;

			case 0x17:
				data |= input_port_read( machine(), "IN2" );
				break;

			case 0x1f:
				data |= input_port_read( machine(), "IN3" );
				break;

			case 0x27:
				data |= input_port_read( machine(), "IN4" );
				break;

			case 0x2f:
				data |= input_port_read( machine(), "IN5" );
				break;

			default:
				if( m_last_io_offset != m_io_offset )
				{
					m_last_io_offset = m_io_offset;
				}

				break;
		}
	}

	if( ACCESSING_BITS_8_15 )
	{
		/* led status? 1100 */
	}

	return data;
}

WRITE32_MEMBER(twinkle_state::twinkle_output_w)
{
	switch( offset )
	{
	case 0x00:
		/* offset */
		break;
	case 0x02:
		/* data */
		break;
	case 0x04:
		/* ?? */
		break;
	case 0x08:
		/* bit 0 = clock?? */
		/* bit 1 = data?? */
		/* bit 2 = reset?? */
		break;
	case 0x0c:
		/* ?? */
		break;
	case 0x15:
		/* ?? */
		break;
	case 0x24:
		/* ?? */
		break;
	}
}

WRITE32_MEMBER(twinkle_state::serial_w)
{
/*
    int _do = ( data >> 4 ) & 1;
    int clock = ( data >> 5 ) & 1;
    int reset = ( data >> 6 ) & 1;

    printf( "serial_w do=%d clock=%d reset=%d\n", _do, clock, reset );
*/
}

WRITE32_MEMBER(twinkle_state::shared_psx_w)
{


	if (mem_mask == 0xff)
	{
		m_spu_shared[offset*2] = data;
//      printf("shared_psx_w: %x to %x (%x), mask %x (PC=%x)\n", data, offset, offset*2, mem_mask, cpu_get_pc(&space.device()));
	}
	else if (mem_mask == 0xff0000)
	{
		m_spu_shared[(offset*2)+1] = data;
//      printf("shared_psx_w: %x to %x (%x), mask %x (PC=%x)\n", data, offset, (offset*2)+1, mem_mask, cpu_get_pc(&space.device()));
	}
	else
	{
		fatalerror("shared_psx_w: Unknown mask %x\n", mem_mask);
	}
}

READ32_MEMBER(twinkle_state::shared_psx_r)
{

	UINT32 result;

	result = m_spu_shared[offset*2] | m_spu_shared[(offset*2)+1]<<16;

//  printf("shared_psx_r: @ %x (%x %x), mask %x = %x (PC=%x)\n", offset, offset*2, (offset*2)+1, mem_mask, result, cpu_get_pc(&space.device()));

	result = 0;	// HACK to prevent the games from freezing while we sort out the rest of the 68k's boot sequence

	return result;
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 32, twinkle_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM	AM_SHARE("share1") /* ram */
	AM_RANGE(0x1f000000, 0x1f0007ff) AM_READWRITE(shared_psx_r, shared_psx_w)
	AM_RANGE(0x1f200000, 0x1f20001f) AM_READWRITE_LEGACY(am53cf96_r, am53cf96_w)
	AM_RANGE(0x1f20a01c, 0x1f20a01f) AM_WRITENOP /* scsi? */
	AM_RANGE(0x1f210400, 0x1f2107ff) AM_READNOP
	AM_RANGE(0x1f218000, 0x1f218003) AM_WRITE(watchdog_reset32_w) /* LTC1232 */
	AM_RANGE(0x1f220000, 0x1f220003) AM_WRITE(twinkle_io_w)
	AM_RANGE(0x1f220004, 0x1f220007) AM_READ(twinkle_io_r)
	AM_RANGE(0x1f230000, 0x1f230003) AM_WRITENOP
	AM_RANGE(0x1f240000, 0x1f240003) AM_READ_PORT("IN6")
	AM_RANGE(0x1f250000, 0x1f250003) AM_WRITENOP
	AM_RANGE(0x1f260000, 0x1f260003) AM_WRITE(serial_w)
	AM_RANGE(0x1f270000, 0x1f270003) AM_WRITE_PORT("OUTSEC")
	AM_RANGE(0x1f280000, 0x1f280003) AM_READ_PORT("INSEC")
	AM_RANGE(0x1f290000, 0x1f29007f) AM_DEVREADWRITE8("rtc", rtc65271_device, rtc_r, rtc_w, 0x00ff00ff)
	AM_RANGE(0x1f2a0000, 0x1f2a007f) AM_DEVREADWRITE8("rtc", rtc65271_device, xram_r, xram_w, 0x00ff00ff)
	AM_RANGE(0x1f2b0000, 0x1f2b00ff) AM_WRITE(twinkle_output_w)
	AM_RANGE(0x1fc00000, 0x1fc7ffff) AM_ROM AM_SHARE("share2") AM_REGION("user1", 0) /* bios */
	AM_RANGE(0x80000000, 0x803fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0x9fc00000, 0x9fc7ffff) AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xa0000000, 0xa03fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0xbfc00000, 0xbfc7ffff) AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xfffe0130, 0xfffe0133) AM_WRITENOP
ADDRESS_MAP_END

/* SPU board */

static void ide_interrupt(device_t *device, int state_)
{
	twinkle_state *state = device->machine().driver_data<twinkle_state>();

	if ((state_) && (state->m_spu_ctrl & 0x0400))
	{
		cputag_set_input_line(device->machine(), "audiocpu", M68K_IRQ_6, ASSERT_LINE);
	}
}

static READ16_DEVICE_HANDLER( twinkle_ide_r )
{
	if (offset == 0)
	{
		return ide_controller_r(device, offset+0x1f0, 2);
	}
	else
	{
		return ide_controller_r(device, offset+0x1f0, 1);
	}
}

static WRITE16_DEVICE_HANDLER( twinkle_ide_w )
{
	ide_controller_w(device, offset+0x1f0, 1, data);
}

/*
    System control register (Konami always has one)

    bit 7  = write 0 to ack IRQ 1, write 1 to enable (IRQ 1 appears to be vblank)
    bit 8  = write 0 to ack IRQ 2, write 1 to enable (IRQ 2 appears to be DMA completion)
    bit 9  = write 0 to ack IRQ 4, write 1 to enable (IRQ 4 appears to be "command sent", unsure how the MIPS causes it yet however)
    bit 10 = write 0 to ack IRQ 6, write 1 to enable (IRQ 6 is IDE)
    bit 11 = watchdog toggle?

    Other bits unknown.
*/
WRITE16_MEMBER(twinkle_state::twinkle_spu_ctrl_w)
{


	if ((!(data & 0x0080)) && (m_spu_ctrl & 0x0080))
	{
		device_set_input_line(&space.device(), M68K_IRQ_1, CLEAR_LINE);
	}
	else if ((!(data & 0x0100)) && (m_spu_ctrl & 0x0100))
	{
		device_set_input_line(&space.device(), M68K_IRQ_2, CLEAR_LINE);
	}
	else if ((!(data & 0x0200)) && (m_spu_ctrl & 0x0200))
	{
		device_set_input_line(&space.device(), M68K_IRQ_4, CLEAR_LINE);
	}
	else if ((!(data & 0x0400)) && (m_spu_ctrl & 0x0400))
	{
		device_set_input_line(&space.device(), M68K_IRQ_6, CLEAR_LINE);
	}

	m_spu_ctrl = data;
}

READ16_MEMBER(twinkle_state::twinkle_waveram_r)
{
	UINT16 *waveram = (UINT16 *)machine().region("rfsnd")->base();

	return waveram[offset];
}

WRITE16_MEMBER(twinkle_state::twinkle_waveram_w)
{
	UINT16 *waveram = (UINT16 *)machine().region("rfsnd")->base();

	COMBINE_DATA(&waveram[offset]);
}

READ16_MEMBER(twinkle_state::shared_68k_r)
{


//  printf("shared_68k_r: @ %x, mask %x\n", offset, mem_mask);

	return m_spu_shared[offset];
}

WRITE16_MEMBER(twinkle_state::shared_68k_w)
{


//  printf("shared_68k_w: %x to %x, mask %x\n", data, offset, mem_mask);

	m_spu_shared[offset] = data & 0xff;
}

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 16, twinkle_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x13ffff) AM_RAM
	// 220000 = LEDs?
	AM_RANGE(0x230000, 0x230003) AM_WRITE(twinkle_spu_ctrl_w)
	// 240000 = top 16 bits of DMA address?
	// 250000 = write to initiate DMA?
	// 260000 = ???
	AM_RANGE(0x280000, 0x280fff) AM_READWRITE(shared_68k_r, shared_68k_w )
	AM_RANGE(0x300000, 0x30000f) AM_DEVREADWRITE_LEGACY("ide", twinkle_ide_r, twinkle_ide_w)
	// 34000E = ???
	AM_RANGE(0x400000, 0x400fff) AM_DEVREADWRITE_LEGACY("rfsnd", rf5c400_r, rf5c400_w)
	AM_RANGE(0x800000, 0xffffff) AM_READWRITE(twinkle_waveram_r, twinkle_waveram_w )	// 8 MB window wave RAM
ADDRESS_MAP_END

/* SCSI */

static void scsi_dma_read( twinkle_state *state, UINT32 n_address, INT32 n_size )
{
	UINT32 *p_n_psxram = state->m_p_n_psxram;
	int i;
	int n_this;

	while( n_size > 0 )
	{
		if( n_size > sizeof( state->m_sector_buffer ) / 4 )
		{
			n_this = sizeof( state->m_sector_buffer ) / 4;
		}
		else
		{
			n_this = n_size;
		}
		if( n_this < 2048 / 4 )
		{
			/* non-READ commands */
			am53cf96_read_data( n_this * 4, state->m_sector_buffer );
		}
		else
		{
			/* assume normal 2048 byte data for now */
			am53cf96_read_data( 2048, state->m_sector_buffer );
			n_this = 2048 / 4;
		}
		n_size -= n_this;

		i = 0;
		while( n_this > 0 )
		{
			p_n_psxram[ n_address / 4 ] =
				( state->m_sector_buffer[ i + 0 ] << 0 ) |
				( state->m_sector_buffer[ i + 1 ] << 8 ) |
				( state->m_sector_buffer[ i + 2 ] << 16 ) |
				( state->m_sector_buffer[ i + 3 ] << 24 );
			n_address += 4;
			i += 4;
			n_this--;
		}
	}
}

static void scsi_dma_write( twinkle_state *state, UINT32 n_address, INT32 n_size )
{
	UINT32 *p_n_psxram = state->m_p_n_psxram;
	int i;
	int n_this;

	while( n_size > 0 )
	{
		if( n_size > sizeof( state->m_sector_buffer ) / 4 )
		{
			n_this = sizeof( state->m_sector_buffer ) / 4;
		}
		else
		{
			n_this = n_size;
		}
		n_size -= n_this;

		i = 0;
		while( n_this > 0 )
		{
			state->m_sector_buffer[ i + 0 ] = ( p_n_psxram[ n_address / 4 ] >> 0 ) & 0xff;
			state->m_sector_buffer[ i + 1 ] = ( p_n_psxram[ n_address / 4 ] >> 8 ) & 0xff;
			state->m_sector_buffer[ i + 2 ] = ( p_n_psxram[ n_address / 4 ] >> 16 ) & 0xff;
			state->m_sector_buffer[ i + 3 ] = ( p_n_psxram[ n_address / 4 ] >> 24 ) & 0xff;
			n_address += 4;
			i += 4;
			n_this--;
		}

		am53cf96_write_data( n_this * 4, state->m_sector_buffer );
	}
}

static void scsi_irq(running_machine &machine)
{
	psx_irq_set(machine, 0x400);
}

static const SCSIConfigTable dev_table =
{
	1, /* 1 SCSI device */
	{
		{ SCSI_ID_4, "cdrom0", SCSI_DEVICE_CDROM } /* SCSI ID 4, using CHD 0, and it's a CD-ROM */
	}
};

static const struct AM53CF96interface scsi_intf =
{
	&dev_table,		/* SCSI device table */
	&scsi_irq,		/* command completion IRQ */
};

static DRIVER_INIT( twinkle )
{
	psx_driver_init(machine);
	am53cf96_init(machine, &scsi_intf);

	device_t *i2cmem = machine.device("security");
	i2cmem_e0_write( i2cmem, 0 );
	i2cmem_e1_write( i2cmem, 0 );
	i2cmem_e2_write( i2cmem, 0 );
	i2cmem_wc_write( i2cmem, 0 );
}

static MACHINE_RESET( twinkle )
{
	/* also hook up CDDA audio to the CD-ROM drive */
	cdda_set_cdrom(machine.device("cdda"), am53cf96_get_device(SCSI_ID_4));
}

static void spu_irq(device_t *device, UINT32 data)
{
	if (data)
	{
		psx_irq_set(device->machine(), 1<<9);
	}
}

static const i2cmem_interface i2cmem_interface =
{
	I2CMEM_SLAVE_ADDRESS, 0, 0x100
};

static const rtc65271_interface twinkle_rtc =
{
	DEVCB_NULL
};

static MACHINE_CONFIG_START( twinkle, twinkle_state )
	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", CXD8530CQ, XTAL_67_7376MHz )
	MCFG_CPU_PROGRAM_MAP( main_map )

	MCFG_PSX_DMA_CHANNEL_READ( "maincpu", 5, psx_dma_read_delegate( FUNC( scsi_dma_read ), (twinkle_state *) owner ) )
	MCFG_PSX_DMA_CHANNEL_WRITE( "maincpu", 5, psx_dma_write_delegate( FUNC( scsi_dma_write ), (twinkle_state *) owner ) )

	MCFG_CPU_ADD("audiocpu", M68000, 32000000/2)	/* 16.000 MHz */
	MCFG_CPU_PROGRAM_MAP( sound_map )

	MCFG_WATCHDOG_TIME_INIT(attotime::from_msec(1200)) /* check TD pin on LTC1232 */

	MCFG_MACHINE_RESET( twinkle )
	MCFG_I2CMEM_ADD("security",i2cmem_interface)

	MCFG_IDE_CONTROLLER_ADD("ide", ide_interrupt, ide_devices, "hdd", NULL)
	MCFG_RTC65271_ADD("rtc", twinkle_rtc)

	/* video hardware */
	MCFG_PSXGPU_ADD( "maincpu", "gpu", CXD8561Q, 0x200000, XTAL_53_693175MHz )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("speakerleft", "speakerright")

	MCFG_SPU_ADD( "spu", XTAL_67_7376MHz/2, &spu_irq )
	MCFG_SOUND_ROUTE( 0, "speakerleft", 0.75 )
	MCFG_SOUND_ROUTE( 1, "speakerright", 0.75 )

	MCFG_SOUND_ADD("rfsnd", RF5C400, 32000000/2)
	MCFG_SOUND_ROUTE(0, "speakerleft", 1.0)
	MCFG_SOUND_ROUTE(1, "speakerright", 1.0)

	MCFG_SOUND_ADD( "cdda", CDDA, 0 )
	MCFG_SOUND_ROUTE( 0, "speakerleft", 1.0 )
	MCFG_SOUND_ROUTE( 1, "speakerright", 1.0 )
MACHINE_CONFIG_END

static INPUT_PORTS_START( twinkle )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON8) PORT_NAME("VEFX")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON9) PORT_NAME("Effect")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE) PORT_NAME(DEF_STR(Test))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X) PORT_PLAYER(2) PORT_NAME("2P Table") PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("IN2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X) PORT_PLAYER(1) PORT_NAME("1P Table") PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("IN3")
	PORT_BIT( 0x0f, 0x00, IPT_AD_STICK_Y) PORT_NAME("Volume 1") PORT_MINMAX(0x00,0x0f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0)
	PORT_BIT( 0xf0, 0x00, IPT_AD_STICK_Y) PORT_NAME("Volume 2") PORT_MINMAX(0x00,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0)

	PORT_START("IN4")
	PORT_BIT( 0x0f, 0x00, IPT_AD_STICK_Y) PORT_NAME("Volume 3") PORT_MINMAX(0x00,0x0f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0)
	PORT_BIT( 0xf0, 0x00, IPT_AD_STICK_Y) PORT_NAME("Volume 4") PORT_MINMAX(0x00,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0)

	PORT_START("IN5")
	PORT_BIT( 0x0f, 0x00, IPT_AD_STICK_Y) PORT_NAME("Volume 5") PORT_MINMAX(0x00,0x0f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0)

	PORT_START("IN6")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1) PORT_NAME("P1-1 (SW1)" )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1) PORT_NAME("P1-2 (SW2)" )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(1) PORT_NAME("P1-3 (SW3)" )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(1) PORT_NAME("P1-4 (SW4)" )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_PLAYER(1) PORT_NAME("P1-5 (SW5)" )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_PLAYER(1) PORT_NAME("P1-6 (SW6)" )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON7) PORT_PLAYER(1) PORT_NAME("P1-7 (SW7)" )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2) PORT_NAME("P2-1 (SW8)" )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2) PORT_NAME("P2-2 (SW9)" )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(2) PORT_NAME("P2-3 (SW10)" )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(2) PORT_NAME("P2-4 (SW11)" )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_PLAYER(2) PORT_NAME("P2-5 (SW12)" )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_PLAYER(2) PORT_NAME("P2-6 (SW13)" )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON7) PORT_PLAYER(2) PORT_NAME("P2-7 (SW14)" )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("OUTSEC")
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("security", i2cmem_scl_write)
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("security", i2cmem_sda_write)

	PORT_START("INSEC")
	PORT_BIT( 0x00001000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_READ_LINE_DEVICE("security", i2cmem_sda_read)
INPUT_PORTS_END

#define TWINKLE_BIOS	\
	ROM_REGION32_LE( 0x080000, "user1", 0 )\
	ROM_LOAD( "863a03.7b",    0x000000, 0x080000, CRC(81498f73) SHA1(3599b40a5872eab3a00d345287635355fcb25a71) )\
\
	ROM_REGION32_LE( 0x080000, "audiocpu", 0 )\
	ROM_LOAD16_WORD_SWAP( "863a05.2x",    0x000000, 0x080000, CRC(6f42a09e) SHA1(cab5209f90f47b9ee6e721479913ad74e3ba84b1) )\
\
	ROM_REGION(0x1800000, "rfsnd", ROMREGION_ERASE00)

ROM_START( gq863 )
	TWINKLE_BIOS
ROM_END

ROM_START( bmiidx )
	TWINKLE_BIOS

	DISK_REGION( "cdrom0" )	// program
	DISK_IMAGE_READONLY("863jaa01", 0, SHA1(aee12de1dc5dd44e5bf7b62133ed695b80999390) )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY("863jaa04", 0, SHA1(8f6a0d2e191153032c9388b5298d8ee531b22a41) )

	DISK_REGION( "drive_0" )
	DISK_IMAGE_READONLY("c44jaa03", 0, SHA1(53e9bd25d1674a04aeec81c0224b4e4e44af802a) )	// was part of a 1st mix machine, but "c44" indicates 8th mix?
ROM_END

ROM_START( bmiidx3 )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "992j.pd",      0x000000, 0x000100, BAD_DUMP CRC(51f24913) SHA1(574b555e3d0c234011198d218d7ae5e95091acb1) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "992jaa01", 0, BAD_DUMP SHA1(7e5389735dff379bb286ba3744edf59b7dfcc74b) )
//  DISK_IMAGE_READONLY( "992jaahd", 1, NO_DUMP )
//  DISK_IMAGE_READONLY( "992jaa02", 2, NO_DUMP )
ROM_END

ROM_START( bmiidx4 )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "a03j.pd",      0x000000, 0x000100, CRC(8860cfb6) SHA1(85a5b27f24d4baa7960e692b91c0cf3dc5388e72) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "a03jaa01", 0, BAD_DUMP SHA1(2a587b5524bac6f03d26b55247a0acd22aad6c3a) )
//  DISK_IMAGE_READONLY( "a03jaahd", 1, NO_DUMP )
//  DISK_IMAGE_READONLY( "a03jaa02", 2, NO_DUMP )
ROM_END

ROM_START( bmiidx6 )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "b4uj.pd",      0x000000, 0x000100, BAD_DUMP CRC(0ab15633) SHA1(df004ff41f35b16089f69808ccf53a5e5cc13ac3) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "b4ujaa01", 0, BAD_DUMP SHA1(d8f5d56b8728bea761dc4cdbc04851094d276bd6) )
//  DISK_IMAGE_READONLY( "b4ujaahd", 1, NO_DUMP )
//  DISK_IMAGE_READONLY( "b4ujaa02", 2, NO_DUMP )
ROM_END

ROM_START( bmiidx7 )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "b44j.pd",      0x000000, 0x000100, BAD_DUMP CRC(5baf4761) SHA1(aa7e07eb2cada03b85bdf11ac6a3de65f4253eef) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "b44jaa01", 0, BAD_DUMP SHA1(a21610f3dc090e39e125d063442ed877fa056146) )
//  DISK_IMAGE_READONLY( "b44jaahd", 1, NO_DUMP )
//  DISK_IMAGE_READONLY( "b44jaa02", 2, NO_DUMP )
ROM_END

ROM_START( bmiidx8 )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "c44j.pd",      0x000000, 0x000100, BAD_DUMP CRC(04c22349) SHA1(d1cb78911cb1ca660d393a81ed3ed07b24c51525) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "c44jaa01", 0, BAD_DUMP SHA1(8b544c81bc56b19e4aa1649e68824811d6d51ce5) )
//  DISK_IMAGE_READONLY( "c44jaahd", 1, NO_DUMP )
//  DISK_IMAGE_READONLY( "c44jaa02", 2, NO_DUMP )
ROM_END

ROM_START( bmiidxc )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "896j.pd",      0x000000, 0x000100, BAD_DUMP CRC(1e5caf37) SHA1(75b378662b651cb322e41564d3bae68cc9edadc5) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "896jaabm", 0, SHA1(af008e5bcf18da4e9aea752a712c843e37a74be5) )
//  DISK_IMAGE_READONLY( "abmjaahd", 1, NO_DUMP )
//  DISK_IMAGE_READONLY( "abmjaa02", 2, NO_DUMP )
ROM_END

ROM_START( bmiidxc2 )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "984j.pd",      0x000000, 0x000100, BAD_DUMP CRC(213843e5) SHA1(5571db155a60fa4087dd996af48e8e27fc1c518c) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "984a01bm", 0, SHA1(d9b7d74a72a76e4e9cf7725e0fb8dafcc1c87187) )
//  DISK_IMAGE_READONLY( "abmjaahd", 1, NO_DUMP )
//  DISK_IMAGE_READONLY( "abmjaa02", 2, NO_DUMP )
ROM_END

ROM_START( bmiidxca )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "896j.pd",      0x000000, 0x000100, BAD_DUMP CRC(1e5caf37) SHA1(75b378662b651cb322e41564d3bae68cc9edadc5) )

	DISK_REGION( "cdrom0" )
	DISK_IMAGE_READONLY( "896jabbm", 0, SHA1(117ae4c876207bbaf9e8fe0fdf5bb161155c1bdb) )
//  DISK_IMAGE_READONLY( "abmjaahd", 1, NO_DUMP )
//  DISK_IMAGE_READONLY( "abmjaa02", 2, NO_DUMP )
ROM_END

GAME( 1999, gq863,    0,       twinkle, twinkle, twinkle, ROT0, "Konami", "Twinkle System", GAME_IS_BIOS_ROOT )

/* VCD */
GAME( 1999, bmiidx,   gq863,   twinkle, twinkle, twinkle, ROT0, "Konami", "beatmania IIDX (863 JAA)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING )
/* find out what these use for security */
GAME( 1999, bmiidxc,  gq863,   twinkle, twinkle, twinkle, ROT0, "Konami", "beatmania IIDX with DDR 2nd Club Version (896 JAB)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING  )
GAME( 1999, bmiidxca, bmiidxc, twinkle, twinkle, twinkle, ROT0, "Konami", "beatmania IIDX with DDR 2nd Club Version (896 JAA)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING  )
/* 1999 - beatmania IIDX substream */
GAME( 1999, bmiidxc2, gq863,   twinkle, twinkle, twinkle, ROT0, "Konami", "beatmania IIDX Substream 2 with DDR 2nd Club Version (984 A01 BM)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING  )

/* 1999 - beatmania IIDX 2nd style */
/* these use i2c for security */
GAME( 2000, bmiidx3,  gq863,   twinkle, twinkle, twinkle, ROT0, "Konami", "beatmania IIDX 3rd style (GC992 JA)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING  )
GAME( 2000, bmiidx4,  gq863,   twinkle, twinkle, twinkle, ROT0, "Konami", "beatmania IIDX 4th style (GCA03 JA)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING  )
/* 2001 - beatmania IIDX 5th style */

/* DVD */
GAME( 2001, bmiidx6,  gq863,   twinkle, twinkle, twinkle, ROT0, "Konami", "beatmania IIDX 6th style (GCB4U JA)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING  )
GAME( 2002, bmiidx7,  gq863,   twinkle, twinkle, twinkle, ROT0, "Konami", "beatmania IIDX 7th style (GCB44 JA)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING  )
GAME( 2002, bmiidx8,  gq863,   twinkle, twinkle, twinkle, ROT0, "Konami", "beatmania IIDX 8th style (GCC44 JA)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING  )
