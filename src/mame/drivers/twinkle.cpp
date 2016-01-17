// license:BSD-3-Clause
// copyright-holders:smf, R. Belmont
/*

Twinkle System

driver by smf and R. Belmont

TODO:

dvd check for bmiidx, bmiidxa, bmiidxc & bmiidxca
finish sound board emulation and remove response hle
emulate dvd player and video mixing
16seg led font


Konami Twinkle Hardware Overview
Konami 1999-2002

The following games are known to exist on this hardware (there may be more)
                                                                  Video CD      Security
Game Title                            Year     Program CD       6/7/8 use DVD   Dongle      HDD label
-----------------------------------------------------------------------------------------------------
beatmania IIDX (English)              1999     GQ863 A01        GQ863 A04      *863 A02     ?
beatmania IIDX (Japanese)             1999     GQ863-JA B01     GQ863 A04      *863 A02     ?
beatmania IIDX + DDR Club Kit         1999     896 JA ABM       *?             *?           ?
beatmania IIDX + DDR Club Kit(newer)  1999     896 JA BBM       *?             *?           ?
beatmania IIDX Substream              1999     *?               GC983 A04      *?           ?
beatmania IIDX Club Version 2         1999     GE984 A01(BM)    *?             *984 A02     ?
                                             + GE984 A01(DDR)
beatmania IIDX 2nd Style              1999     GC985 A01        GC985 A04      *           *985 HDD A01
beatmania IIDX 3rd Style              2000     GC992-JA A01     GC992-JA A04   *           *992 HDD A01
beatmania IIDX 3rd Style(newer)       2000     GC992-JA C01     GC992-JA A04   *           *992 HDD A01
beatmania IIDX 4th Style              2000     A03 JA A01       A03 JA A02     *A03        A03 JA A03
beatmania IIDX 5th Style              2001     A17 JA A01       A17 JA A02     *           *A17 JA A03
beatmania IIDX 6th Style              2001     B4U JA A01       B4U JA A02     *           B4U JA A03
beatmania IIDX 6th Style(newer)       2001     B4U JA B01       B4U JA A02     *           B4U JA A03
beatmania IIDX 7th Style              2002     B44 JA A01       B44 JA A02     *           B44 JA A03
beatmania IIDX 8th Style              2002     C44 JA A01       C44 JA A02     *C44        C44 JA A03

* = Not dumped.
? = Code unknown.

Where there are multiple revisions of the program cd, it has been assumed that the video and hdd are the same.

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
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsicd.h"
#include "bus/rs232/xvd701.h"
#include "machine/am53cf96.h"
#include "machine/ataintf.h"
#include "machine/fdc37c665gt.h"
#include "machine/i2cmem.h"
#include "machine/rtc65271.h"
#include "machine/x76f041.h"
#include "sound/spu.h"
#include "sound/cdda.h"
#include "sound/rf5c400.h"

class twinkle_state : public driver_device
{
public:
	twinkle_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_am53cf96(*this, "am53cf96"),
		m_ata(*this, "ata"),
		m_waveram(*this, "rfsnd"),
		m_spu_ata_dma(0),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu")
	{
	}

	required_device<am53cf96_device> m_am53cf96;
	required_device<ata_interface_device> m_ata;
	required_region_ptr<UINT16> m_waveram;

	UINT16 m_spu_ctrl;      // SPU board control register
	UINT8 m_spu_shared[0x400];  // SPU/PSX shared dual-ported RAM
	UINT32 m_spu_ata_dma;
	int m_spu_ata_dmarq;

	int m_io_offset;
	int m_output_last[ 0x100 ];
	int m_last_io_offset;
	UINT8 m_sector_buffer[ 4096 ];
	DECLARE_WRITE8_MEMBER(twinkle_io_w);
	DECLARE_READ8_MEMBER(twinkle_io_r);
	DECLARE_WRITE16_MEMBER(twinkle_output_w);
	DECLARE_WRITE16_MEMBER(serial_w);
	DECLARE_WRITE8_MEMBER(shared_psx_w);
	DECLARE_READ8_MEMBER(shared_psx_r);
	DECLARE_WRITE16_MEMBER(twinkle_spu_ctrl_w);
	DECLARE_WRITE16_MEMBER(spu_ata_dma_low_w);
	DECLARE_WRITE16_MEMBER(spu_ata_dma_high_w);
	DECLARE_READ16_MEMBER(twinkle_waveram_r);
	DECLARE_WRITE16_MEMBER(twinkle_waveram_w);
	DECLARE_READ16_MEMBER(shared_68k_r);
	DECLARE_WRITE16_MEMBER(shared_68k_w);
	DECLARE_READ16_MEMBER(unk_68k_r);
	DECLARE_WRITE_LINE_MEMBER(spu_ata_irq);
	DECLARE_WRITE_LINE_MEMBER(spu_ata_dmarq);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;

	int m_serial_shift;
	int m_serial_bits;
	int m_serial_cs;
	int m_serial_clock;

	int m_output_shift;
	int m_output_bits;
	int m_output_cs;
	int m_output_clock;
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

WRITE8_MEMBER(twinkle_state::twinkle_io_w)
{
	switch( offset )
	{
	case 0:
		if( m_output_last[ m_io_offset ] != data )
		{
			m_output_last[ m_io_offset ] = data;

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
				output().set_indexed_value( "led", ( m_io_offset - 7 ) / 8, asciicharset[ ( data ^ 0xff ) & 0x7f ] );
				break;

			case 0x87:
				output().set_indexed_value( "spotlight", 0, ( ~data >> 3 ) & 1 );
				output().set_indexed_value( "spotlight", 1, ( ~data >> 2 ) & 1 );
				output().set_indexed_value( "spotlight", 2, ( ~data >> 1 ) & 1 );
				output().set_indexed_value( "spotlight", 3, ( ~data >> 0 ) & 1 );
				output().set_indexed_value( "spotlight", 4, ( ~data >> 4 ) & 1 );
				output().set_indexed_value( "spotlight", 5, ( ~data >> 5 ) & 1 );
				output().set_indexed_value( "spotlight", 6, ( ~data >> 6 ) & 1 );
				output().set_indexed_value( "spotlight", 7, ( ~data >> 7 ) & 1 );
				break;

			case 0x8f:
				output().set_value( "neonlamp", ( ~data >> 0 ) & 1 );
				output().set_value( "unknown1", ( ~data >> 1 ) & 1 );
				output().set_value( "unknown2", ( ~data >> 2 ) & 1 );

				if( ( data & 0xf8 ) != 0xf8 )
				{
					printf("%02x = %02x\n", m_io_offset, data );
				}
				break;

			default:
				printf( "unknown io %02x = %02x\n", m_io_offset, data );
				break;
			}
		}
		break;

	case 1:
		m_io_offset = data;
		break;
	}
}

READ8_MEMBER(twinkle_state::twinkle_io_r)
{
	UINT8 data = 0;

	switch( offset )
	{
	case 0:
		switch( m_io_offset )
		{
			case 0x07:
				data = ioport( "IN0" )->read();
				break;

			case 0x0f:
				data = ioport( "IN1" )->read();
				break;

			case 0x17:
				data = ioport( "IN2" )->read();
				break;

			case 0x1f:
				data = ioport( "IN3" )->read();
				break;

			case 0x27:
				data = ioport( "IN4" )->read();
				break;

			case 0x2f:
				data = ioport( "IN5" )->read();
				break;

			default:
				if( m_last_io_offset != m_io_offset )
				{
					m_last_io_offset = m_io_offset;
				}
				break;
		}
		break;

	case 1:
		/* led status? 1100 */
		break;
	}

	return data;
}

WRITE16_MEMBER(twinkle_state::twinkle_output_w)
{
	switch( offset )
	{
	case 0x00:
		/* offset */
		break;
	case 0x04:
		/* data */
		break;
	case 0x08:
		/* ?? */
		break;
	case 0x10:
		{
			int clock = (data >> 0) & 1;
			int _do = (data >> 1) & 1;
			int cs = (data >> 2) & 1;

			//printf( "output do=%d clock=%d cs=%d (remaining %02x)\n", _do, clock, cs, data & 0xfff8 );

			if (!cs && m_output_cs)
			{
				m_output_shift = 0;
				m_output_bits = 0;
			}

			if (clock && !m_output_clock && m_output_bits < 8)
			{
				m_output_shift <<= 1;
				m_output_shift |= _do;
				m_output_bits++;

				if (m_output_bits == 8)
				{
					//printf( "output %02x\n", m_output_shift );

					m_output_bits = 0;
					m_output_shift = 0;
				}
			}

			m_output_cs = cs;
			m_output_clock = clock;
		}
		break;
	case 0x18:
		/* ?? */
		break;
	case 0x30:
		/* ?? */
		break;
	case 0x48:
		/* ?? */
		break;
	}
}

WRITE16_MEMBER(twinkle_state::serial_w)
{
	int _do = ( data >> 4 ) & 1;
	int clock = ( data >> 5 ) & 1;
	int cs = ( data >> 6 ) & 1;

	//printf( "serial_w do=%d clock=%d cs=%d (remaining %02x)\n", _do, clock, cs, data & 0xff8f );

	if (!cs && m_serial_cs)
	{
		m_serial_shift = 0;
		m_serial_bits = 0;
	}

	if (clock && !m_serial_clock && m_serial_bits < 8)
	{
		m_serial_shift <<= 1;
		m_serial_shift |= _do;
		m_serial_bits++;

		if (m_serial_bits == 8)
		{
			//printf( "serial %02x\n", m_serial_shift );
		}
	}

	m_serial_cs = cs;
	m_serial_clock = clock;
}

WRITE8_MEMBER(twinkle_state::shared_psx_w)
{
//  printf("shared_psx_w: %04x, %04x, %04x\n", offset, data, mem_mask);

	m_spu_shared[offset] = data;

	if (offset == 0x03fe && data == 0xff)
	{
//      printf("spu command %02x %02x\n", m_spu_shared[1], m_spu_shared[3]);

		m_audiocpu->set_input_line(M68K_IRQ_4, HOLD_LINE);
	}
}

READ8_MEMBER(twinkle_state::shared_psx_r)
{
	UINT32 result = m_spu_shared[offset];

	//printf("shared_psx_r: %04x, %04x, %04x\n", offset, result, mem_mask);

	return result;
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 32, twinkle_state )
	AM_RANGE(0x1f000000, 0x1f0007ff) AM_READWRITE8(shared_psx_r, shared_psx_w, 0x00ff00ff)
	AM_RANGE(0x1f200000, 0x1f20001f) AM_DEVREADWRITE8("am53cf96", am53cf96_device, read, write, 0x00ff00ff)
	AM_RANGE(0x1f20a01c, 0x1f20a01f) AM_WRITENOP /* scsi? */
	AM_RANGE(0x1f210000, 0x1f2107ff) AM_DEVREADWRITE8("fdc37c665gt", fdc37c665gt_device, read, write, 0x00ff00ff)
	AM_RANGE(0x1f218000, 0x1f218003) AM_WRITE8(watchdog_reset_w, 0x000000ff) /* LTC1232 */
	AM_RANGE(0x1f220000, 0x1f220003) AM_WRITE8(twinkle_io_w, 0x00ff00ff)
	AM_RANGE(0x1f220004, 0x1f220007) AM_READ8(twinkle_io_r, 0x00ff00ff)
	AM_RANGE(0x1f230000, 0x1f230003) AM_WRITENOP
	AM_RANGE(0x1f240000, 0x1f240003) AM_READ_PORT("IN6")
	AM_RANGE(0x1f250000, 0x1f250003) AM_WRITENOP
	AM_RANGE(0x1f260000, 0x1f260003) AM_WRITE16(serial_w, 0x0000ffff)
	AM_RANGE(0x1f270000, 0x1f270003) AM_WRITE_PORT("OUTSEC")
	AM_RANGE(0x1f280000, 0x1f280003) AM_READ_PORT("INSEC")
	AM_RANGE(0x1f290000, 0x1f29007f) AM_DEVREADWRITE8("rtc", rtc65271_device, rtc_r, rtc_w, 0x00ff00ff)
	AM_RANGE(0x1f2a0000, 0x1f2a007f) AM_DEVREADWRITE8("rtc", rtc65271_device, xram_r, xram_w, 0x00ff00ff)
	AM_RANGE(0x1f2b0000, 0x1f2b00ff) AM_WRITE16(twinkle_output_w, 0xffffffff)
ADDRESS_MAP_END

/* SPU board */

WRITE_LINE_MEMBER(twinkle_state::spu_ata_irq)
{
	if ((state) && (m_spu_ctrl & 0x0400))
	{
		m_audiocpu->set_input_line(M68K_IRQ_6, ASSERT_LINE);
	}
}

/*
    System control register (Konami always has one)

    bit 7  = write 0 to ack IRQ 1, write 1 to enable (IRQ 1 appears to be an RF5C400-related timer, or some free-running timing source)
    bit 8  = write 0 to ack IRQ 2, write 1 to enable (IRQ 2 appears to be DMA completion)
    bit 9  = write 0 to ack IRQ 4, write 1 to enable (IRQ 4 is "command available")
    bit 10 = write 0 to ack IRQ 6, write 1 to enable (IRQ 6 is the ATA IRQ)
    bit 11 = watchdog toggle

    Other bits unknown.
*/
WRITE16_MEMBER(twinkle_state::twinkle_spu_ctrl_w)
{
	if ((!(data & 0x0080)) && (m_spu_ctrl & 0x0080))
	{
		m_audiocpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
	}
	else if ((!(data & 0x0100)) && (m_spu_ctrl & 0x0100))
	{
		m_audiocpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);
	}
	else if ((!(data & 0x0200)) && (m_spu_ctrl & 0x0200))
	{
		m_audiocpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);
	}
	else if ((!(data & 0x0400)) && (m_spu_ctrl & 0x0400))
	{
		m_audiocpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
	}

	m_spu_ctrl = data;
}

WRITE16_MEMBER(twinkle_state::spu_ata_dma_low_w)
{
	m_spu_ata_dma = (m_spu_ata_dma & ~0xffff) | data;
}

WRITE16_MEMBER(twinkle_state::spu_ata_dma_high_w)
{
	m_spu_ata_dma = (m_spu_ata_dma & 0xffff) | (data << 16);
}

WRITE_LINE_MEMBER(twinkle_state::spu_ata_dmarq)
{
	if (m_spu_ata_dmarq != state)
	{
		m_spu_ata_dmarq = state;

		if (m_spu_ata_dmarq)
		{
			m_ata->write_dmack(ASSERT_LINE);

			while (m_spu_ata_dmarq)
			{
				UINT16 data = m_ata->read_dma();
				//printf("spu_ata_dmarq %08x %04x\n", m_spu_ata_dma * 2, data);
				//waveram[m_spu_ata_dma++] = (data >> 8) | (data << 8);
				// bp 4a0e ;bmiidx4 checksum
				// bp 4d62 ;bmiidx4 dma

				// $$$HACK - game DMAs nothing useful to 0x400000 but all sound plays are 0x400000 or above
				//           so limit sound RAM to 4MB (there's 6 MB on the board) and let the 5c400's address masking
				//           work for us until we figure out what's actually going on.
				if (m_spu_ata_dma < 0x200000)
				{
					m_waveram[m_spu_ata_dma++] = data;
				}
			}

			m_ata->write_dmack(CLEAR_LINE);
		}
	}
}

READ16_MEMBER(twinkle_state::twinkle_waveram_r)
{
	return m_waveram[offset];
}

WRITE16_MEMBER(twinkle_state::twinkle_waveram_w)
{
	COMBINE_DATA(&m_waveram[offset]);
}

READ16_MEMBER(twinkle_state::shared_68k_r)
{
	UINT16 result = m_spu_shared[offset];

//  printf("shared_68k_r: %04x, %04x, %04x\n", offset, result, mem_mask);

	return result;
}

WRITE16_MEMBER(twinkle_state::shared_68k_w)
{
//  printf("shared_68k_w: %04x, %04x, %04x\n", offset, data, mem_mask);

	m_spu_shared[offset] = data & 0xff;
}

READ16_MEMBER(twinkle_state::unk_68k_r)
{
	return 0xffff;  // must return 0xff for 68000 POST to complete properly
}

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 16, twinkle_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x13ffff) AM_RAM
	AM_RANGE(0x200000, 0x200001) AM_READ(unk_68k_r)
	// 220000 = LEDs?
	AM_RANGE(0x230000, 0x230003) AM_WRITE(twinkle_spu_ctrl_w)
	AM_RANGE(0x240000, 0x240003) AM_WRITE(spu_ata_dma_low_w)
	AM_RANGE(0x250000, 0x250003) AM_WRITE(spu_ata_dma_high_w)
	// 260000 = ???
	AM_RANGE(0x280000, 0x280fff) AM_READWRITE(shared_68k_r, shared_68k_w)
	AM_RANGE(0x300000, 0x30000f) AM_DEVREADWRITE("ata", ata_interface_device, read_cs0, write_cs0)
	// 34000E = ???
	AM_RANGE(0x34000e, 0x34000f) AM_WRITENOP
	AM_RANGE(0x400000, 0x400fff) AM_DEVREADWRITE("rfsnd", rf5c400_device, rf5c400_r, rf5c400_w)
	AM_RANGE(0x800000, 0xbfffff) AM_READWRITE(twinkle_waveram_r, twinkle_waveram_w )
	AM_RANGE(0xfe0000, 0xffffff) AM_RAM // ...and the RAM test checks this last 128k (mirror of the work RAM at 0x100000?)
ADDRESS_MAP_END

/* SCSI */

static void scsi_dma_read( twinkle_state *state, UINT32 *p_n_psxram, UINT32 n_address, INT32 n_size )
{
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
			state->m_am53cf96->dma_read_data( n_this * 4, state->m_sector_buffer );
		}
		else
		{
			/* assume normal 2048 byte data for now */
			state->m_am53cf96->dma_read_data( 2048, state->m_sector_buffer );
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

static void scsi_dma_write( twinkle_state *state, UINT32 *p_n_psxram, UINT32 n_address, INT32 n_size )
{
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

		state->m_am53cf96->dma_write_data( n_this * 4, state->m_sector_buffer );
	}
}


static MACHINE_CONFIG_FRAGMENT( cdrom_config )
	MCFG_DEVICE_MODIFY( "cdda" )
	MCFG_SOUND_ROUTE( 0, "^^^^speakerleft", 1.0 )
	MCFG_SOUND_ROUTE( 1, "^^^^speakerright", 1.0 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( twinkle, twinkle_state )
	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", CXD8530CQ, XTAL_67_7376MHz )
	MCFG_CPU_PROGRAM_MAP( main_map )

	MCFG_RAM_MODIFY("maincpu:ram")
	MCFG_RAM_DEFAULT_SIZE("4M")

	MCFG_PSX_DMA_CHANNEL_READ( "maincpu", 5, psx_dma_read_delegate( FUNC( scsi_dma_read ), (twinkle_state *) owner ) )
	MCFG_PSX_DMA_CHANNEL_WRITE( "maincpu", 5, psx_dma_write_delegate( FUNC( scsi_dma_write ), (twinkle_state *) owner ) )

	MCFG_CPU_ADD("audiocpu", M68000, 32000000/2)    /* 16.000 MHz */
	MCFG_CPU_PROGRAM_MAP( sound_map )
	MCFG_CPU_PERIODIC_INT_DRIVER(twinkle_state, irq1_line_assert, 60)
	MCFG_CPU_PERIODIC_INT_DRIVER(twinkle_state, irq2_line_assert, 60)

	MCFG_WATCHDOG_TIME_INIT(attotime::from_msec(1200)) /* check TD pin on LTC1232 */

	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "cdrom", SCSICD, SCSI_ID_4)
	MCFG_SLOT_OPTION_MACHINE_CONFIG("cdrom", cdrom_config)

	MCFG_DEVICE_ADD("am53cf96", AM53CF96, 0)
	MCFG_LEGACY_SCSI_PORT("scsi")
	MCFG_AM53CF96_IRQ_HANDLER(DEVWRITELINE("maincpu:irq", psxirq_device, intin10))

	MCFG_ATA_INTERFACE_ADD("ata", ata_devices, "hdd", nullptr, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(twinkle_state, spu_ata_irq))
	MCFG_ATA_INTERFACE_DMARQ_HANDLER(WRITELINE(twinkle_state, spu_ata_dmarq))

	MCFG_DEVICE_ADD("rtc", RTC65271, 0)

	MCFG_DEVICE_ADD("fdc37c665gt", FDC37C665GT, XTAL_24MHz)

	MCFG_DEVICE_ADD("rs232", RS232_PORT, 0)
	MCFG_SLOT_OPTION_ADD("xvd701", JVC_XVD701)
//  MCFG_SLOT_OPTION_ADD("xvs1100", JVC_XVS1100) // 8th mix only
	MCFG_SLOT_DEFAULT_OPTION("xvd701")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("fdc37c665gt:uart2", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("fdc37c665gt:uart2", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("fdc37c665gt:uart2", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("fdc37c665gt:uart2", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("fdc37c665gt:uart2", ins8250_uart_device, cts_w))

	MCFG_DEVICE_MODIFY("fdc37c665gt:uart2")
	MCFG_INS8250_OUT_TX_CB(DEVWRITELINE("^rs232", rs232_port_device, write_txd))
	MCFG_INS8250_OUT_DTR_CB(DEVWRITELINE("^rs232", rs232_port_device, write_dtr))
	MCFG_INS8250_OUT_RTS_CB(DEVWRITELINE("^rs232", rs232_port_device, write_rts))

	/* video hardware */
	MCFG_PSXGPU_ADD( "maincpu", "gpu", CXD8561Q, 0x200000, XTAL_53_693175MHz )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("speakerleft", "speakerright")

	MCFG_SPU_ADD( "spu", XTAL_67_7376MHz/2 )
	MCFG_SOUND_ROUTE( 0, "speakerleft", 0.75 )
	MCFG_SOUND_ROUTE( 1, "speakerright", 0.75 )

	MCFG_RF5C400_ADD("rfsnd", 32000000/2)
	MCFG_SOUND_ROUTE(0, "speakerleft", 1.0)
	MCFG_SOUND_ROUTE(1, "speakerright", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( twinklex, twinkle )
	MCFG_X76F041_ADD( "security" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( twinklei, twinkle )
	MCFG_I2CMEM_ADD( "security" )
	MCFG_I2CMEM_DATA_SIZE( 0x100 )
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
	PORT_START("INSEC")
INPUT_PORTS_END

static INPUT_PORTS_START( twinklex )
	PORT_INCLUDE( twinkle )

	PORT_MODIFY("OUTSEC")
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("security", x76f041_device, write_scl)
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("security", x76f041_device, write_sda)
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("security", x76f041_device, write_cs)

	PORT_MODIFY("INSEC")
	PORT_BIT( 0x00001000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("security", x76f041_device, read_sda)
INPUT_PORTS_END

static INPUT_PORTS_START( twinklei )
	PORT_INCLUDE( twinkle )

	PORT_MODIFY("OUTSEC")
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("security", i2cmem_device, write_scl)
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("security", i2cmem_device, write_sda)

	PORT_MODIFY("INSEC")
	PORT_BIT( 0x00001000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_READ_LINE_DEVICE_MEMBER("security", i2cmem_device, read_sda)
INPUT_PORTS_END

#define TWINKLE_BIOS    \
	ROM_REGION32_LE( 0x080000, "maincpu:rom", 0 )\
	ROM_LOAD( "863a03.7b",    0x000000, 0x080000, CRC(81498f73) SHA1(3599b40a5872eab3a00d345287635355fcb25a71) )\
\
	ROM_REGION32_LE( 0x080000, "audiocpu", 0 )\
	ROM_LOAD16_WORD_SWAP( "863a05.2x",    0x000000, 0x080000, CRC(6f42a09e) SHA1(cab5209f90f47b9ee6e721479913ad74e3ba84b1) )\
\
	ROM_REGION16_LE(0x400000, "rfsnd", ROMREGION_ERASE00)

ROM_START( gq863 )
	TWINKLE_BIOS
ROM_END

ROM_START( bmiidx )
	TWINKLE_BIOS

	ROM_REGION( 0x224, "security", 0 )
	ROM_LOAD( "863a02", 0x000000, 0x000224, BAD_DUMP CRC(7b2a429b) SHA1(f710d19c7b900a58584c07ab8fd3ab7b9f0121d7) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" ) // program
	DISK_IMAGE_READONLY( "gq863-jab01", 0, SHA1(331f80b40ed560c7e017621b7daeeb8275d92b9a) )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY( "gq863a04", 0, SHA1(25359f0eaff3749a6194a6b9d93f6aec67d94819) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY( "863hdda01", 0, SHA1(0b8dbf1c9caf4abf965dbc6e1a8e6329d48b1c90) )
ROM_END

ROM_START( bmiidxa )
	TWINKLE_BIOS

	ROM_REGION( 0x224, "security", 0 )
	ROM_LOAD( "863a02", 0x000000, 0x000224, BAD_DUMP CRC(7b2a429b) SHA1(f710d19c7b900a58584c07ab8fd3ab7b9f0121d7) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" ) // program
	DISK_IMAGE_READONLY( "gq863a01", 0, SHA1(07fc467f6500504729becbaf77dabc093a134e65) )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY( "gq863a04", 0, SHA1(25359f0eaff3749a6194a6b9d93f6aec67d94819) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY( "863hdda01", 0, SHA1(0b8dbf1c9caf4abf965dbc6e1a8e6329d48b1c90) )
ROM_END

ROM_START( bmiidx2 )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "985a02", 0x000000, 0x000100, BAD_DUMP CRC(a35143a9) SHA1(1c0feeab60d9dc50dc4b9a2f3dac73ca619e74b0) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "gc985a01", 0, SHA1(0b783f11317f64552ebf3323459139529e7f315f) )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY( "gc985a04", 0, SHA1(4306417f61eb1ea92894d288cdb7c385eb4610f2) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY( "985hdda01", 0, NO_DUMP )
ROM_END

ROM_START( bmiidx3 )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "992a02", 0x000000, 0x000100, BAD_DUMP CRC(51f24913) SHA1(574b555e3d0c234011198d218d7ae5e95091acb1) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "gc992-jac01", 0, SHA1(c02d6e58439be678ec0d7171eae2dfd53a21acc7) )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY( "gc992-jaa04", 0, SHA1(66d0b9ac793ff3fdddd0aa2aa5f2809d0c295944) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY( "992hdda01", 0, NO_DUMP )
ROM_END

ROM_START( bmiidx3a )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "992a02", 0x000000, 0x000100, BAD_DUMP CRC(51f24913) SHA1(574b555e3d0c234011198d218d7ae5e95091acb1) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "gc992-jaa01", 0, BAD_DUMP SHA1(7e5389735dff379bb286ba3744edf59b7dfcc74b) )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY( "gc992-jaa04", 0, SHA1(66d0b9ac793ff3fdddd0aa2aa5f2809d0c295944) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY( "992hdda01", 0, NO_DUMP )
ROM_END

ROM_START( bmiidx4 )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "a03", 0x000000, 0x000100, BAD_DUMP CRC(8860cfb6) SHA1(85a5b27f24d4baa7960e692b91c0cf3dc5388e72) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "a03jaa01", 0, SHA1(f54fc778c2187ccd950402a159babef956b71492) )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY( "a03jaa02", 0, SHA1(d6f01d666e8de285a02215f7ef987073e2b25019) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY( "a03jaa03", 0, SHA1(a9814c60d2ed98b8c4f6e11ea762518a1712e7b5) )
ROM_END

ROM_START( bmiidx5 )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "a17", 0x000000, 0x000100, BAD_DUMP CRC(9428afb0) SHA1(ba907d3361256b022583d6a42fe223e90590e3c6) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "a17jaa01", 0, SHA1(5ac46973b42b2c66ae63297d1a7fd69b33ef4d1d) )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY( "a17jaa02", 0, SHA1(cc24a4c3f5e7c77dbeee7db94c0cc8a330e2b51b) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY( "a17jaa03", 0, NO_DUMP )
ROM_END

ROM_START( bmiidx6 )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "b4u", 0x000000, 0x000100, BAD_DUMP CRC(0ab15633) SHA1(df004ff41f35b16089f69808ccf53a5e5cc13ac3) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "b4ujab01", 0, SHA1(aaae77f473c4a44ce6838da3ef6dab27e4afa0e4) )

	DISK_REGION( "cdrom1" ) // DVD
	DISK_IMAGE_READONLY( "b4ujaa02", 0, SHA1(70c85f6df6f21b96c02e4eefc224593edcaf9e63) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY( "b4ujaa03", 0, SHA1(cfcbdfab157a864cbd4ac83247be5d62218f5b72) )
ROM_END

ROM_START( bmiidx6a )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "b4u", 0x000000, 0x000100, BAD_DUMP CRC(0ab15633) SHA1(df004ff41f35b16089f69808ccf53a5e5cc13ac3) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "b4ujaa01", 0, BAD_DUMP SHA1(d8f5d56b8728bea761dc4cdbc04851094d276bd6) )

	DISK_REGION( "cdrom1" ) // DVD
	DISK_IMAGE_READONLY( "b4ujaa02", 0, SHA1(70c85f6df6f21b96c02e4eefc224593edcaf9e63) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY( "b4ujaa03", 0, SHA1(cfcbdfab157a864cbd4ac83247be5d62218f5b72) )
ROM_END

ROM_START( bmiidx7 )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "b44", 0x000000, 0x000100, BAD_DUMP CRC(5baf4761) SHA1(aa7e07eb2cada03b85bdf11ac6a3de65f4253eef) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "b44jaa01", 0, SHA1(57fb0312d8102e959658e48a97e46aa16e592b60) )

	DISK_REGION( "cdrom1" ) // DVD
	DISK_IMAGE_READONLY( "b44jaa02", 0, SHA1(a45726d99025f4d824ec143ef92957c76c08a13a) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY( "b44jaa03", 0, SHA1(1adb8e4874e26e8ccd9822e6f9dd12f6e6f8af05) )
ROM_END

ROM_START( bmiidx8 )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "c44", 0x000000, 0x000100, BAD_DUMP CRC(04c22349) SHA1(d1cb78911cb1ca660d393a81ed3ed07b24c51525) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "c44jaa01", 0, BAD_DUMP SHA1(8b544c81bc56b19e4aa1649e68824811d6d51ce5) )

	DISK_REGION( "cdrom1" ) // DVD
	DISK_IMAGE_READONLY( "c44jaa02", 0, SHA1(f4c454a6360c507a122888d5bc3311eed5ce083b) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY( "c44jaa03", 0, SHA1(14df5039a4f5a648f1a2d12a35c16f56d0f9cd28) )
ROM_END

ROM_START( bmiidxc )
	TWINKLE_BIOS

	ROM_REGION( 0x224, "security", 0 )
	ROM_LOAD( "896a02", 0x000000, 0x000224, BAD_DUMP CRC(7b2a429b) SHA1(f710d19c7b900a58584c07ab8fd3ab7b9f0121d7) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "896jabbm", 0, BAD_DUMP SHA1(117ae4c876207bbaf9e8fe0fdf5bb161155c1bdb) )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY( "896jaa04", 0, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY( "863hdda01", 0, SHA1(0b8dbf1c9caf4abf965dbc6e1a8e6329d48b1c90) )
ROM_END

ROM_START( bmiidxca )
	TWINKLE_BIOS

	ROM_REGION( 0x224, "security", 0 )
	ROM_LOAD( "896a02", 0x000000, 0x000224, BAD_DUMP CRC(7b2a429b) SHA1(f710d19c7b900a58584c07ab8fd3ab7b9f0121d7) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "896jaabm", 0, SHA1(ea7205f86543d9273efcc226666ab530c32b23c1) )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY( "896jaa04", 0, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY( "863hdda01", 0, SHA1(0b8dbf1c9caf4abf965dbc6e1a8e6329d48b1c90) )
ROM_END

ROM_START( bmiidxs )
	TWINKLE_BIOS

	ROM_REGION( 0x224, "security", 0 )
	ROM_LOAD( "983a02", 0x000000, 0x000224, NO_DUMP )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "gc983a01", 0, NO_DUMP )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY( "gc983a04", 0, SHA1(73454f2acb5a1e6b9e21140eb7b93a4827072d63) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY( "983hdda01", 0, SHA1(bcbbf55acf8bebc5773ffc5769420a0129f4da57) )
ROM_END

ROM_START( bmiidxc2 )
	TWINKLE_BIOS

	ROM_REGION( 0x224, "security", 0 )
	ROM_LOAD( "984a02", 0x000000, 0x000224, BAD_DUMP CRC(5b08e1ef) SHA1(d43ad5d958313ccb2420246621d9180230b4782d) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "ge984a01(bm)", 0, SHA1(03b083ba09652dfab6f328000c3c9de2a7a4e618) )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY( "ge984a04", 0, NO_DUMP )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE_READONLY( "983hdda01", 0, SHA1(bcbbf55acf8bebc5773ffc5769420a0129f4da57) )
ROM_END

GAME( 1999, gq863,    0,       twinkle,  twinkle,  driver_device, 0,        ROT0, "Konami", "Twinkle System", MACHINE_IS_BIOS_ROOT )

GAME( 1999, bmiidx,   gq863,   twinklex, twinklex, driver_device, 0,        ROT0, "Konami", "beatmania IIDX (863 JAB)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1999, bmiidxa,  bmiidx,  twinklex, twinklex, driver_device, 0,        ROT0, "Konami", "beatmania IIDX (863 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1999, bmiidxc,  gq863,   twinklex, twinklex, driver_device, 0,        ROT0, "Konami", "beatmania IIDX with DDR 2nd Club Version (896 JAB)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1999, bmiidxca, bmiidxc, twinklex, twinklex, driver_device, 0,        ROT0, "Konami", "beatmania IIDX with DDR 2nd Club Version (896 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1999, bmiidxs,  gq863,   twinklex, twinklex, driver_device, 0,        ROT0, "Konami", "beatmania IIDX Substream (983 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1999, bmiidxc2, gq863,   twinklex, twinklex, driver_device, 0,        ROT0, "Konami", "beatmania IIDX Substream with DDR 2nd Club Version 2 (984 A01 BM)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1999, bmiidx2,  gq863,   twinklei, twinklei, driver_device, 0,        ROT0, "Konami", "beatmania IIDX 2nd style (GC985 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 2000, bmiidx3,  gq863,   twinklei, twinklei, driver_device, 0,        ROT0, "Konami", "beatmania IIDX 3rd style (GC992 JAC)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 2000, bmiidx3a, bmiidx3, twinklei, twinklei, driver_device, 0,        ROT0, "Konami", "beatmania IIDX 3rd style (GC992 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 2000, bmiidx4,  gq863,   twinklei, twinklei, driver_device, 0,        ROT0, "Konami", "beatmania IIDX 4th style (GCA03 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 2001, bmiidx5,  gq863,   twinklei, twinklei, driver_device, 0,        ROT0, "Konami", "beatmania IIDX 5th style (GCA17 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 2001, bmiidx6,  gq863,   twinklei, twinklei, driver_device, 0,        ROT0, "Konami", "beatmania IIDX 6th style (GCB4U JAB)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 2001, bmiidx6a, bmiidx6, twinklei, twinklei, driver_device, 0,        ROT0, "Konami", "beatmania IIDX 6th style (GCB4U JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 2002, bmiidx7,  gq863,   twinklei, twinklei, driver_device, 0,        ROT0, "Konami", "beatmania IIDX 7th style (GCB44 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 2002, bmiidx8,  gq863,   twinklei, twinklei, driver_device, 0,        ROT0, "Konami", "beatmania IIDX 8th style (GCC44 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
