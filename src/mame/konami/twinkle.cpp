// license:BSD-3-Clause
// copyright-holders:smf, R. Belmont
/*

Twinkle System

driver by smf and R. Belmont

TODO:

dvd check for bmiidx, bmiidxa, bmiidxc & bmiidxca
 - 1st style (all variants, including club kit) connected over the mini-din8 (CN7) and dvd check requires RTS->CTS
 - substream and above switched to standard serial connection (CN5)
The first 128k of RF5C400 bank 0 is uploaded by the 68000, the rest is unused. It may be using 16J & 18J
emulate dvd player and video mixing
16seg led font
The SPU has an IRQ2 handler that doesn't do anything, find out if or to what it is connected.


Konami Twinkle Hardware Overview
Konami 1997-2002

The following games are known to exist on this hardware (there may be more)
                                                                  Video CD      Security
Game Title                            Year     Program CD       6/7/8 use DVD   Dongle      HDD label
-----------------------------------------------------------------------------------------------------
Ugoke! Purimucho                      1997     GQ751?           (not used)     ?           (not used)

beatmania IIDX (English)              1999     GQ863 A01        GQ863 A04      863 A02!    863 HDD A01
beatmania IIDX (Japanese)             1999     GQ863-JA B01     GQ863 A04      863 A02!    863 HDD A01
beatmania IIDX + DDR Club Kit         1999     896 JA ABM       GQ863 A04      863 A02!    863 HDD A01
                                             + 896 JA A01
beatmania IIDX + DDR Club Kit(newer)  1999     896 JA BBM       GQ863 A04      863 A02!    863 HDD A01
                                             + 896 JA A01
beatmania IIDX Substream              1999     GC983 A01        GC983 A04      983 A02!    983 HDD A01
beatmania IIDX Substream (Asia)       1999     GC983-AA A01     GC983 A04      983A A02!   983 HDD A01
beatmania IIDX Club Version 2         1999     GE984 A01(BM)    GC983 A04      984 A02!    983 HDD A01
                                             + GE984 A01(DDR)
beatmania IIDX 2nd Style              1999     GC985 A01        GC985 A04      985         985 HDD A01
beatstage II 2nd Style                1999     ?                ?              ?           ?
beatmania IIDX 3rd Style              2000     GC992-JA A01     GC992-JA A04   992         992 HDD A01
beatmania IIDX 3rd Style(newer)       2000     GC992-JA B01     GC992-JA A04   992         992 HDD A01
beatmania IIDX 3rd Style(newest)      2000     GC992-JA C01     GC992-JA A04   992         992 HDD A01
beatmania IIDX 4th Style              2000     A03 JA A01       A03 JA A02     A03         A03 JA A03
beatmania IIDX 5th Style              2001     A17 JA A01       A17 JA A02     A17         A17 JA A03
beatmania IIDX 6th Style              2001     B4U JA A01       B4U JA A02     B4U         B4U JA A03
beatmania IIDX 6th Style(newer)       2001     B4U JA B01       B4U JA A02     B4U         B4U JA A03
beatmania IIDX 7th Style              2002     B44 JA A01       B44 JA A02     B44         B44 JA A03
beatmania IIDX 7th Style(newer)       2002     B44 JA B01       B44 JA A02     B44         B44 JA A03
beatmania IIDX 8th Style              2002     C44 JA A01       C44 JA A02     C44         C44 JA A03

* = Not dumped.
! = Dumped partially, see security note below.
? = Code unknown.

All versions of IIDX before 2nd Style use a security dongle containing a X76F041. It contains three
8 byte passwords for writing to the dongle, reading from the dongle and reading/writing configuration
registers. Without the password, you cannot perform the desired action. Further, the chip contains
a feature where a series of bad passwords locks all actions permanently until the chip is reset. The
only way around this, according to the datasheet, is to submit the correct password or wipe the entire
chip and reprogram it.

All four secure enclaves making the 512 bytes of data present on the dongle are protected by the read
password which must be present in some form in the game binary and as such is trivial to recover.
Additionally, tracing the code reveals unused functions for writing dongles and setting configuration
registers. Unfortunately, the write and config passwords appear scrubbed from the binaries. However,
the intended configuration register valuse are still present in the binaries. Thus, we can at least
infer the read password as well as the config register values, and use the read password to dump the
512 bytes contained in the secure enclave. The data sheet specifies the reset sequence. With this data,
we can reconstruct a dongle dump that is missing only the write and config passwords since it is not
presently possible to read them. Further, a brute-force attack seems impossible due to the config
register defaults being set to 3 password failures before lockout. All of the dumps taken in this
manner are labelled "bad dump" since they are incomplete.

Dongles for 2nd Style and newer do not suffer this problem as they use a serial EEPROM with no
protection. They can be read out and contain a serial number that matches the printed serial on the
outside of the dongle as well as game-specific bytes to tie the dongle to the specific mix.

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
      MC141685 - Motorola MC141685 low cost 3CH D/A converter
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
      SM5875   - Nippon Precision Circuits SM5875 2-channel D/A converter (SSOP24)
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


Ugoke! Purimucho notes:
Ugoke! Purimucho's schematic shows TWINKLE/SUB GQ751PWB(B1) as the subboard instead of the SUB2 used by beatmania IIDX
"VENDER GQ751PWB(C2)" for vendor box hardware
Has an LCD with a touch panel overlayed on it
Also includes a camera and printer (Mitsubishi CP700A, used to print pictures for keyholders)

*/

#include "emu.h"

#include "bus/ata/ataintf.h"
#include "bus/nscsi/cd.h"
#include "bus/rs232/xvd701.h"
#include "cpu/m68000/m68000.h"
#include "cpu/psx/psx.h"
#include "machine/fdc37c665gt.h"
#include "machine/i2cmem.h"
#include "machine/mb8421.h"
#include "machine/ncr53c90.h"
#include "machine/ram.h"
#include "machine/rtc65271.h"
#include "machine/watchdog.h"
#include "machine/x76f041.h"
#include "sound/cdda.h"
#include "sound/rf5c400.h"
#include "sound/spu.h"
#include "video/psx.h"

#include "screen.h"
#include "speaker.h"

#include "endianness.h"

#include "bmiidx.lh"


namespace {

class twinkle_state : public driver_device
{
public:
	twinkle_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ncr53cf96(*this, "scsi:7:ncr53cf96"),
		m_ata(*this, "ata"),
		m_dpram(*this, "dpram"),
		m_waveram(*this, "rfsnd"),
		m_in(*this, "IN%u", 0U),
		m_led_displays(*this, "led%u", 0U),
		m_spotlights(*this, "spotlight%u", 0U),
		m_main_leds(*this, "main_led%u", 0U),
		m_key_leds(*this, "key%u-%u", 1U, 1U),
		m_spu_leds(*this, "spu_led%u", 0U),
		m_player_lamps(*this, "%up", 1U),
		m_vefx_lamp(*this, "vefx"),
		m_effect_lamp(*this, "effect"),
		m_credit_lamp(*this, "credit"),
		m_neon_lamp(*this, "neonlamp"),
		m_unknown_outputs(*this, "unknown%u", 1U),
		m_spu_ata_dma(0),
		m_spu_ata_dmarq(0),
		m_wave_bank(0)
	{
	}

	void twinklex(machine_config &config);
	void twinklex2(machine_config &config);
	void twinklei(machine_config &config);
	void twinkle(machine_config &config);
	void twinkle_dvd_type1(machine_config &config);
	void twinkle_dvd_type2(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void twinkle_io_w(offs_t offset, uint8_t data);
	uint8_t twinkle_io_r(offs_t offset);
	void twinkle_videomixer_w(offs_t offset, uint16_t data);
	void led_w(uint16_t data);
	void key_led_w(uint16_t data);
	void serial_w(uint16_t data);
	void twinkle_spu_ctrl_w(uint16_t data);
	void spu_ata_dma_low_w(uint16_t data);
	void spu_ata_dma_high_w(uint16_t data);
	uint16_t twinkle_waveram_r(offs_t offset);
	void twinkle_waveram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void spu_led_w(uint16_t data);
	void spu_wavebank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void spu_ata_irq(int state);
	void spu_ata_dmarq(int state);
	void scsi_dma_read(uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size);
	void scsi_dma_write(uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size);
	void scsi_drq(int state);

	void main_map(address_map &map);
	void rf5c400_map(address_map &map);
	void sound_map(address_map &map);

	TIMER_CALLBACK_MEMBER(scsi_dma_transfer);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<ncr53cf96_device> m_ncr53cf96;
	required_device<ata_interface_device> m_ata;
	required_device<cy7c131_device> m_dpram;
	required_shared_ptr<uint16_t> m_waveram;

	required_ioport_array<6> m_in;

	output_finder<9> m_led_displays;
	output_finder<8> m_spotlights;
	output_finder<9> m_main_leds;
	output_finder<2, 7> m_key_leds;
	output_finder<8> m_spu_leds;
	output_finder<2> m_player_lamps;
	output_finder<> m_vefx_lamp;
	output_finder<> m_effect_lamp;
	output_finder<> m_credit_lamp;
	output_finder<> m_neon_lamp;
	output_finder<4> m_unknown_outputs;

	emu_timer *m_dma_timer = nullptr;

	uint16_t m_spu_ctrl = 0;      // SPU board control register
	uint32_t m_spu_ata_dma = 0;
	int m_spu_ata_dmarq = 0;
	uint32_t m_wave_bank = 0;

	uint32_t *m_dma_data_ptr = nullptr;
	uint32_t m_dma_offset = 0;
	int32_t m_dma_size = 0;
	bool m_dma_is_write = false;
	bool m_dma_requested = false;

	int m_io_offset = 0;
	int m_output_last[ 0x100 ]{};

	int m_serial_shift = 0;
	int m_serial_bits = 0;
	int m_serial_cs = 0;
	int m_serial_clock = 0;

	int m_output_shift = 0;
	int m_output_bits = 0;
	int m_output_cs = 0;
	int m_output_clock = 0;
};

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


static const uint16_t asciicharset[]=
{
// 0
	0, // (default on boot)
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
// 0x10
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
// 0x20
	0, // space
	LED_D2 | LED_A2 | LED_I | LED_J, // !
	LED_I | LED_B, // "
	0, // #
	0, // $
	0, // %
	LED_A1 | LED_A2 | LED_D1 | LED_D2 | LED_H | LED_J | LED_M | LED_K | LED_C, // &
	0, // '
	LED_J | LED_K, // (
	LED_H | LED_M, // )
	LED_G1 | LED_G2 | LED_H | LED_I | LED_J | LED_K | LED_L | LED_M, // *
	0, // +
	0, // ,
	LED_G1 | LED_G2, // -
	0, // .
	LED_J | LED_M, // /
// 0x30
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_E | LED_F | LED_J | LED_M, // 0
	LED_B | LED_C, // 1
	LED_A1 | LED_A2 | LED_B | LED_D1 | LED_D2 | LED_E | LED_G1 | LED_G2, // 2
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_G1 | LED_G2, // 3
	LED_B | LED_C | LED_F | LED_G1 | LED_G2 , // 4
	LED_A1 | LED_A2 | LED_C | LED_D1 | LED_D2 | LED_F | LED_G1 | LED_G2, // 5
	LED_A1 | LED_A2 | LED_C | LED_D1 | LED_D2 | LED_E | LED_F | LED_G1 | LED_G2, // 6
	LED_A1 | LED_A2 | LED_B | LED_C | LED_F, // 7
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_E | LED_F | LED_G1 | LED_G2, // 8
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_F | LED_G1 | LED_G2, // 9
	0, // :
	0, // ;
	0, // <
	0, // =
	0, // >
	LED_A1 | LED_A2 | LED_J | LED_D2, // ?
// 0x40
	0, // @
	LED_A1 | LED_A2 | LED_B | LED_C | LED_E | LED_F | LED_G1 | LED_G2, // A
	LED_A1 | LED_A2 | LED_D1 | LED_D2 | LED_E | LED_F | LED_G1 | LED_J | LED_K, // B
	LED_A1 | LED_A2 | LED_D1 | LED_D2 | LED_E | LED_F, // C
	LED_B | LED_C | LED_D1 | LED_D2 | LED_E | LED_G1 | LED_G2, // D
	LED_A1 | LED_A2 | LED_D1 | LED_D2 | LED_E | LED_F | LED_G1 | LED_G2, // E
	LED_A1 | LED_A2 | LED_E | LED_F | LED_G1 | LED_G2, // F
	LED_A1 | LED_A2 | LED_C | LED_D1 | LED_D2 | LED_E | LED_F | LED_G2, // G
	LED_B | LED_C | LED_E | LED_F | LED_G1 | LED_G2, // H
	LED_A1 | LED_A2 | LED_D1 | LED_D2 | LED_I | LED_L, // I
	LED_A1 | LED_A2 | LED_I | LED_L | LED_D2, // J
	LED_E | LED_F | LED_G1 | LED_J | LED_K, // K
	LED_D1 | LED_D2 | LED_E | LED_F, // L
	LED_B | LED_C | LED_E | LED_F | LED_H | LED_J, // M
	LED_B | LED_C | LED_E | LED_F | LED_H | LED_K, // N
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_E | LED_F, // O
// 0x50
	LED_A1 | LED_A2 | LED_B | LED_E | LED_F | LED_G1 | LED_G2, // P
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_E | LED_F | LED_K, // Q
	LED_A1 | LED_A2 | LED_B | LED_E | LED_F | LED_K | LED_G1 | LED_G2, // R
	LED_A1 | LED_A2 | LED_C | LED_D1 | LED_D2 | LED_F | LED_G1 | LED_G2, // S
	LED_A1 | LED_A2 | LED_I | LED_L, // T
	LED_B | LED_C | LED_D1 | LED_D2 | LED_E | LED_F, // U
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
// 0x60
	0, // `
	0, // a
	0, // b
	0, // c
	0, // d
	0, // e
	0, // f
	LED_A1 | LED_A2 | LED_B | LED_C | LED_D1 | LED_D2 | LED_E | LED_F | LED_G1 | LED_G2 | LED_H | LED_I | LED_J | LED_K | LED_L | LED_M, // g (16 seg led test)
	0, // h
	0, // i
	0, // j
	0, // k
	0, // l
	LED_D2, // m (".")
	0, // n
	0, // o
// 0x70
	0, // p
	LED_J, // q ("'")
	0, // r
	0, // s
	0, // t
	LED_M, // u (",")
	0, // v
	0, // w
	0, // x
	0, // y
	0, // z
	0, // {
	0, // |
	0, // }
	0, // ~
	0, //
};

void twinkle_state::machine_start()
{
	m_dma_timer = timer_alloc(FUNC(twinkle_state::scsi_dma_transfer), this);

	m_led_displays.resolve();
	m_spotlights.resolve();
	m_main_leds.resolve();
	m_key_leds.resolve();
	m_spu_leds.resolve();
	m_player_lamps.resolve();
	m_vefx_lamp.resolve();
	m_effect_lamp.resolve();
	m_credit_lamp.resolve();
	m_neon_lamp.resolve();
	m_unknown_outputs.resolve();

	save_item(NAME(m_spu_ctrl));
	save_item(NAME(m_spu_ata_dma));
	save_item(NAME(m_spu_ata_dmarq));
	save_item(NAME(m_wave_bank));

	save_item(NAME(m_dma_offset));
	save_item(NAME(m_dma_size));
	save_item(NAME(m_dma_is_write));
	save_item(NAME(m_dma_requested));

	save_item(NAME(m_io_offset));
	save_item(NAME(m_output_last));

	save_item(NAME(m_serial_shift));
	save_item(NAME(m_serial_bits));
	save_item(NAME(m_serial_cs));
	save_item(NAME(m_serial_clock));

	save_item(NAME(m_output_shift));
	save_item(NAME(m_output_bits));
	save_item(NAME(m_output_cs));
	save_item(NAME(m_output_clock));
}

void twinkle_state::machine_reset()
{
	m_dma_timer->adjust(attotime::never);
	m_dma_data_ptr = nullptr;
	m_dma_offset = 0;
	m_dma_size = 0;
	m_dma_requested = m_dma_is_write = false;
}


void twinkle_state::twinkle_io_w(offs_t offset, uint8_t data)
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
				if (data != 0xff)
				{
					osd_printf_warning("unknown io %02x = %02x\n", m_io_offset, data);
				}
				break;

			case 0x37:
				m_player_lamps[0] = BIT(~data, 0);
				m_player_lamps[1] = BIT(~data, 1);
				m_vefx_lamp = BIT(~data, 2);
				m_effect_lamp = BIT(~data, 3);
				m_credit_lamp = BIT(~data, 4);

				if ((data & 0xe0) != 0xe0)
				{
					osd_printf_warning("unknown io %02x = %02x\n", m_io_offset, data);
				}
				break;

			case 0x3f:
			case 0x47:
			case 0x4f:
			case 0x57:
			case 0x5f:
			case 0x67:
			case 0x6f:
			case 0x77:
			case 0x7f:
				m_led_displays[(m_io_offset - 0x3f) / 8] = asciicharset[(data ^ 0xff) & 0x7f];
				break;

			case 0x87:
				m_spotlights[0] = BIT(~data, 3);
				m_spotlights[1] = BIT(~data, 2);
				m_spotlights[2] = BIT(~data, 1);
				m_spotlights[3] = BIT(~data, 0);
				m_spotlights[4] = BIT(~data, 4);
				m_spotlights[5] = BIT(~data, 5);
				m_spotlights[6] = BIT(~data, 6);
				m_spotlights[7] = BIT(~data, 7);
				break;

			case 0x8f:
				m_neon_lamp = BIT(~data, 0);
				m_unknown_outputs[0] = BIT(~data, 1);
				m_unknown_outputs[1] = BIT(~data, 2);

				if( ( data & 0xf8 ) != 0xf8 )
				{
					osd_printf_warning("unknown io %02x = %02x\n", m_io_offset, data);
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

uint8_t twinkle_state::twinkle_io_r(offs_t offset)
{
	uint8_t data = 0;

	switch( offset )
	{
	case 0:
		switch( m_io_offset )
		{
			case 0x07:
				data = m_in[0]->read();
				break;

			case 0x0f:
				data = m_in[1]->read();
				break;

			case 0x17:
				data = m_in[2]->read();
				break;

			case 0x1f:
				data = m_in[3]->read();
				break;

			case 0x27:
				data = m_in[4]->read();
				break;

			case 0x2f:
				data = m_in[5]->read();
				break;

			default:
				osd_printf_warning("unknown io 0x%02x\n", m_io_offset);
				break;
		}
		break;

	case 1:
		/* led status? 1100 */
		break;
	}

	return data;
}

void twinkle_state::twinkle_videomixer_w(offs_t offset, uint16_t data)
{
	// Bt812 NTSC/PAL to RGB/YCrCb Decoder chip
	switch( offset )
	{
	case 0x00:
		/*
		    Address Register offset
		    0x00 Command Register 0, Input Select Register
		    0x01 Reserved
		    0x02 Command Register 2, Status Register
		    0x03 Command Register 3, Output Format Register
		    0x04 Command Register 4, Operation Mode Select Register
		    0x05 Command Register 5, Input Format Register
		    0x06 Command Register 6, Clock Definition Register
		    0x07 Command Register 7, Video Timing Definition Register
		    0x08 Brightness Adjust Register (range: -64 to +63)
		    0x09 Contrast Adjust Register (range: 0 to 198.44%)
		    0x0a Saturation Adjust Register (range: 0 to 198.44%)
		    0x0b Hue Adjust Register (range: -45 to +44.3)
		    0x0c HCLOCK Low Register
		    0x0d HCLOCK High Register
		    0x0e HDELAY Low Register
		    0x0f HDELAY High Register
		    0x10 ACTIVE_PIXELS Low Register
		    0x11 ACTIVE_PIXELS High Register
		    0x12 VDELAY Low Register
		    0x13 VDELAY High Register
		    0x14 ACTIVE_LINES Low Register
		    0x15 ACTIVE_LINES High Register
		    0x16 P (subcarrier freq) Register 0
		    0x17 P (subcarrier freq) Register 1
		    0x18 P (subcarrier freq) Register 2
		    0x19 AGC Delay Register
		    0x1a Burst Delay Register
		    0x1b Sample Rate Conversion Low Register
		    0x1c Sample Rate Conversion High Register
		    0x1d Command Register 1D, Video Timing Polarity Register
		    0x1e-0xfe Reserved
		    0xff Software Reset
		*/
		break;
	case 0x04:
		/*
		    Register data
		    Uses offset given in the address register offset command.

		    Game initialized values:
		    HCLOCK 853
		    HDELAY 126
		    ACTIVE_PIXELS 706
		    VDELAY 22
		    ACTIVE_LINES 16
		*/
		break;
	case 0x08:
		// Status bits?
		// Seen values:
		// 0x01 - ?
		// 0x02 - Perform overlay mixing
		// 0x08 - ?
		break;
	case 0x10:
		{
			// Always writes 0x214 and 0x128 here?
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
		// Always 0x69?
		break;
	case 0x30:
		// Always 0x10?
		break;
	case 0x48:
		// 0x20 - Powered on?
		break;
	}
}

void twinkle_state::led_w(uint16_t data)
{
	m_main_leds[0] = BIT(~data, 0);
	m_main_leds[1] = BIT(~data, 1);
	m_main_leds[2] = BIT(~data, 2);
	m_main_leds[3] = BIT(~data, 3);
	m_main_leds[4] = BIT(~data, 4);
	m_main_leds[5] = BIT(~data, 5);
	m_main_leds[6] = BIT(~data, 6);
	m_main_leds[7] = BIT(~data, 7);
	m_main_leds[8] = BIT(~data, 8);

	if ((data & 0xfe00) != 0xfe00)
	{
		osd_printf_warning("led_w unknown %04x\n", data);
	}
}

void twinkle_state::key_led_w(uint16_t data)
{
	// words are written using a byte write
	m_key_leds[0][0] = BIT(data, 0);
	m_key_leds[0][1] = BIT(data, 1);
	m_key_leds[0][2] = BIT(data, 2);
	m_key_leds[0][3] = BIT(data, 3);
	m_key_leds[0][4] = BIT(data, 4);
	m_key_leds[0][5] = BIT(data, 5);
	m_key_leds[0][6] = BIT(data, 6);
	m_key_leds[1][0] = BIT(data, 7);
	m_key_leds[1][1] = BIT(data, 8);
	m_key_leds[1][2] = BIT(data, 9);
	m_key_leds[1][3] = BIT(data, 10);
	m_key_leds[1][4] = BIT(data, 11);
	m_key_leds[1][5] = BIT(data, 12);
	m_key_leds[1][6] = BIT(data, 13);
	m_unknown_outputs[2] = BIT(data, 14);
	m_unknown_outputs[3] = BIT(data, 15);
}

void twinkle_state::serial_w(uint16_t data)
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

void twinkle_state::main_map(address_map &map)
{
	map(0x1f000000, 0x1f0007ff).rw(m_dpram, FUNC(cy7c131_device::right_r), FUNC(cy7c131_device::right_w)).umask32(0x00ff00ff);
	map(0x1f200000, 0x1f20001f).m(m_ncr53cf96, FUNC(ncr53cf96_device::map)).umask16(0x00ff);
	map(0x1f20a01c, 0x1f20a01f).nopw(); /* scsi? */
	map(0x1f210000, 0x1f2107ff).rw("fdc37c665gt", FUNC(fdc37c665gt_device::read), FUNC(fdc37c665gt_device::write)).umask32(0x00ff00ff);
	map(0x1f218000, 0x1f218000).w("watchdog", FUNC(watchdog_timer_device::reset_w)); /* LTC1232 */
	map(0x1f220000, 0x1f220003).w(FUNC(twinkle_state::twinkle_io_w)).umask32(0x00ff00ff);
	map(0x1f220004, 0x1f220007).r(FUNC(twinkle_state::twinkle_io_r)).umask32(0x00ff00ff);
	map(0x1f230000, 0x1f230001).w(FUNC(twinkle_state::led_w));
	map(0x1f240000, 0x1f240003).portr("IN6");
	map(0x1f250000, 0x1f250001).w(FUNC(twinkle_state::key_led_w));
	map(0x1f260000, 0x1f260001).w(FUNC(twinkle_state::serial_w));
	map(0x1f270000, 0x1f270003).portw("OUTSEC");
	map(0x1f280000, 0x1f280003).portr("INSEC");
	map(0x1f290000, 0x1f29007f).rw("rtc", FUNC(rtc65271_device::rtc_r), FUNC(rtc65271_device::rtc_w)).umask32(0x00ff00ff);
	map(0x1f2a0000, 0x1f2a007f).rw("rtc", FUNC(rtc65271_device::xram_r), FUNC(rtc65271_device::xram_w)).umask32(0x00ff00ff);
	map(0x1f2b0000, 0x1f2b00ff).w(FUNC(twinkle_state::twinkle_videomixer_w));
}

/* SPU board */

void twinkle_state::spu_ata_irq(int state)
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
void twinkle_state::twinkle_spu_ctrl_w(uint16_t data)
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

void twinkle_state::spu_ata_dma_low_w(uint16_t data)
{
	m_spu_ata_dma = (m_spu_ata_dma & ~0xffff) | data;
}

void twinkle_state::spu_ata_dma_high_w(uint16_t data)
{
	m_spu_ata_dma = (m_spu_ata_dma & 0xffff) | ((uint32_t)data << 16);
	//printf("DMA now %x\n", m_spu_ata_dma);
}

void twinkle_state::spu_ata_dmarq(int state)
{
	if (m_spu_ata_dmarq != state)
	{
		m_spu_ata_dmarq = state;

		if (m_spu_ata_dmarq)
		{
			m_ata->write_dmack(ASSERT_LINE);

			while (m_spu_ata_dmarq)
			{
				uint16_t data = m_ata->read_dma();
				//printf("spu_ata_dmarq %08x %04x\n", m_spu_ata_dma * 2, data);
				m_waveram[m_wave_bank+m_spu_ata_dma] = data; //(data >> 8) | (data << 8);
				m_spu_ata_dma++;
				// bp 4a0e ;bmiidx4 checksum
				// bp 4d62 ;bmiidx4 dma
			}

			m_ata->write_dmack(CLEAR_LINE);
		}
	}
}

void twinkle_state::spu_wavebank_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//printf("%x to wavebank_w, mask %04x\n", data, mem_mask);

	// banks are fairly clearly 8MB, so there's 3 of them in the 24 MB of RAM.
	// the games load up the full 24MB of RAM 8 MB at a time, first to bank 1,
	// then to bank 2, and finally to bank 3.
	//
	// neither the 68k nor DMA access wave RAM when the bank is 0.
	m_wave_bank = data * (4*1024*1024);
}

uint16_t twinkle_state::twinkle_waveram_r(offs_t offset)
{
	return m_waveram[offset+m_wave_bank];
}

void twinkle_state::twinkle_waveram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_waveram[offset+m_wave_bank]);
}

void twinkle_state::spu_led_w(uint16_t data)
{
	// upper 8 bits are occasionally written as all zeros
	m_spu_leds[0] = BIT(~data, 0);
	m_spu_leds[1] = BIT(~data, 1);
	m_spu_leds[2] = BIT(~data, 2);
	m_spu_leds[3] = BIT(~data, 3);
	m_spu_leds[4] = BIT(~data, 4);
	m_spu_leds[5] = BIT(~data, 5);
	m_spu_leds[6] = BIT(~data, 6);
	m_spu_leds[7] = BIT(~data, 7);
}

void twinkle_state::sound_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x13ffff).ram();
	map(0x200000, 0x200001).portr("SPU_DSW");
	map(0x220000, 0x220001).w(FUNC(twinkle_state::spu_led_w));
	map(0x230000, 0x230003).w(FUNC(twinkle_state::twinkle_spu_ctrl_w));
	map(0x240000, 0x240003).w(FUNC(twinkle_state::spu_ata_dma_low_w)).nopr();
	map(0x250000, 0x250003).w(FUNC(twinkle_state::spu_ata_dma_high_w)).nopr();
	map(0x260000, 0x260001).w(FUNC(twinkle_state::spu_wavebank_w)).nopr();
	map(0x280000, 0x2807ff).rw(m_dpram, FUNC(cy7c131_device::left_r), FUNC(cy7c131_device::left_w)).umask16(0x00ff);
	map(0x300000, 0x30000f).rw(m_ata, FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w));
	// 34000E = ???
	map(0x34000e, 0x34000f).nopw();
	map(0x400000, 0x400fff).rw("rfsnd", FUNC(rf5c400_device::rf5c400_r), FUNC(rf5c400_device::rf5c400_w));
	map(0x800000, 0xffffff).rw(FUNC(twinkle_state::twinkle_waveram_r), FUNC(twinkle_state::twinkle_waveram_w));
}

void twinkle_state::rf5c400_map(address_map &map)
{
	map(0x0000000, 0x1ffffff).ram().share("rfsnd");
}

// SCSI

void twinkle_state::scsi_dma_read(uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size)
{
	m_dma_data_ptr = p_n_psxram;
	m_dma_offset = n_address;
	m_dma_size = n_size * 4;
	m_dma_is_write = false;
	m_dma_timer->adjust(attotime::zero);
}

void twinkle_state::scsi_dma_write(uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size)
{
	m_dma_data_ptr = p_n_psxram;
	m_dma_offset = n_address;
	m_dma_size = n_size * 4;
	m_dma_is_write = true;
	m_dma_timer->adjust(attotime::zero);
}

TIMER_CALLBACK_MEMBER(twinkle_state::scsi_dma_transfer)
{
	// TODO: Figure out proper DMA timings
	while (m_dma_requested && m_dma_data_ptr != nullptr && m_dma_size > 0)
	{
		if (m_dma_is_write)
			m_ncr53cf96->dma_w(util::little_endian_cast<const uint8_t>(m_dma_data_ptr)[m_dma_offset]);
		else
			util::little_endian_cast<uint8_t>(m_dma_data_ptr)[m_dma_offset] = m_ncr53cf96->dma_r();

		m_dma_offset++;
		m_dma_size--;
	}
}

void twinkle_state::scsi_drq(int state)
{
	if (!m_dma_requested && state)
		m_dma_timer->adjust(attotime::zero);

	m_dma_requested = state;
}


void twinkle_state::twinkle(machine_config &config)
{
	/* basic machine hardware */
	CXD8530CQ(config, m_maincpu, XTAL(67'737'600));
	m_maincpu->set_addrmap(AS_PROGRAM, &twinkle_state::main_map);
	m_maincpu->subdevice<psxdma_device>("dma")->install_read_handler(5, psxdma_device::read_delegate(&twinkle_state::scsi_dma_read, this));
	m_maincpu->subdevice<psxdma_device>("dma")->install_write_handler(5, psxdma_device::write_delegate(&twinkle_state::scsi_dma_write, this));
	m_maincpu->subdevice<ram_device>("ram")->set_default_size("4M");

	M68000(config, m_audiocpu, 32000000/2);    /* 16.000 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &twinkle_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(twinkle_state::irq1_line_assert), attotime::from_hz(60));

	WATCHDOG_TIMER(config, "watchdog").set_time(attotime::from_msec(1200)); /* check TD pin on LTC1232 */

	CY7C131(config, m_dpram); // or IDT7130 at some PCBs
	m_dpram->intl_callback().set_inputline(m_audiocpu, M68K_IRQ_4);

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:4").option_set("cdrom", NSCSI_XM5401).machine_config(
			[](device_t *device)
			{
				device->subdevice<cdda_device>("cdda")->add_route(0, "^^speakerleft", 1.0);
				device->subdevice<cdda_device>("cdda")->add_route(1, "^^speakerright", 1.0);
			});
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr53cf96", NCR53CF96).clock(32_MHz_XTAL/2).machine_config(
			[this](device_t *device)
			{
				ncr53cf96_device &adapter = downcast<ncr53cf96_device &>(*device);
				adapter.irq_handler_cb().set(":maincpu:irq", FUNC(psxirq_device::intin10));
				adapter.drq_handler_cb().set(*this, FUNC(twinkle_state::scsi_drq));
			});

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, true);
	m_ata->irq_handler().set(FUNC(twinkle_state::spu_ata_irq));
	m_ata->dmarq_handler().set(FUNC(twinkle_state::spu_ata_dmarq));

	RTC65271(config, "rtc", 0);

	FDC37C665GT(config, "fdc37c665gt", XTAL(24'000'000));

	/* video hardware */
	CXD8561Q(config, "gpu", XTAL(53'693'175), 0x200000, subdevice<psxcpu_device>("maincpu")).set_screen("screen");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	//all twinkle cabinets use anamorphic widescreen displays
	screen.set_physical_aspect(16, 9);

	/* sound hardware */
	SPEAKER(config, "speakerleft").front_left();
	SPEAKER(config, "speakerright").front_right();

	spu_device &spu(SPU(config, "spu", XTAL(67'737'600)/2, subdevice<psxcpu_device>("maincpu")));
	spu.add_route(0, "speakerleft", 0.75);
	spu.add_route(1, "speakerright", 0.75);

	rf5c400_device &rf5c400(RF5C400(config, "rfsnd", XTAL(33'868'800)/2));
	rf5c400.set_addrmap(0, &twinkle_state::rf5c400_map);
	rf5c400.add_route(0, "speakerleft", 1.0);
	rf5c400.add_route(1, "speakerright", 1.0);
}

void twinkle_state::twinkle_dvd_type1(machine_config &config)
{
	// All versions before beatmania IIDX 2nd style
	//   - maincpu:sio1 (PSX SIO1) is used for the DVD player
	//   - fdc37c665gt:uart2 is used for network/session play
	rs232_port_device &rs232(RS232_PORT(config, "rs232_dvd", 0));
	rs232.option_add("xvd701", JVC_XVD701);
	rs232.set_default_option("xvd701");

	// TODO: CTS isn't implemented in SIO but it's required to fix the DVD player error for very early IIDX games
	auto sio1 = subdevice<psxsio1_device>("maincpu:sio1");
	rs232.rxd_handler().set(*sio1, FUNC(psxsio1_device::write_rxd));
	rs232.dsr_handler().set(*sio1, FUNC(psxsio1_device::write_dsr));
	//rs232.cts_handler().set(*sio1, FUNC(psxsio1_device::write_cts));
	sio1->txd_handler().set(rs232, FUNC(rs232_port_device::write_txd));
	sio1->dtr_handler().set(rs232, FUNC(rs232_port_device::write_dtr));
	sio1->rts_handler().set(rs232, FUNC(rs232_port_device::write_rts));

	ns16550_device &uart(*subdevice<ns16550_device>("fdc37c665gt:uart2"));
	rs232_port_device &rs232_network(RS232_PORT(config, "rs232_network", default_rs232_devices, nullptr));
	uart.out_tx_callback().set(rs232_network, FUNC(rs232_port_device::write_txd));
	uart.out_dtr_callback().set(rs232_network, FUNC(rs232_port_device::write_dtr));
	uart.out_rts_callback().set(rs232_network, FUNC(rs232_port_device::write_rts));
	rs232_network.rxd_handler().set(uart, FUNC(ns16550_device::rx_w));
	rs232_network.dcd_handler().set(uart, FUNC(ns16550_device::dcd_w));
	rs232_network.dsr_handler().set(uart, FUNC(ns16550_device::dsr_w));
	rs232_network.ri_handler().set(uart, FUNC(ns16550_device::ri_w));
	rs232_network.cts_handler().set(uart, FUNC(ns16550_device::cts_w));
}

void twinkle_state::twinkle_dvd_type2(machine_config &config)
{
	// All versions starting from beatmania IIDX 2nd style
	//   - fdc37c665gt:uart2 is used for the DVD player
	//   - maincpu:sio1 (PSX SIO1) is used for network/session play
	rs232_port_device &rs232(RS232_PORT(config, "rs232_dvd", 0));
	rs232.option_add("xvd701", JVC_XVD701);
	// rs232.option_add("xvs1100", JVC_XVS1100); // 8th mix only
	rs232.set_default_option("xvd701");

	auto sio1 = subdevice<psxsio1_device>("maincpu:sio1");
	rs232_port_device &rs232_network(RS232_PORT(config, "rs232_network", default_rs232_devices, nullptr));
	sio1->txd_handler().set(rs232_network, FUNC(rs232_port_device::write_txd));
	sio1->dtr_handler().set(rs232_network, FUNC(rs232_port_device::write_dtr));
	rs232_network.rxd_handler().set(*sio1, FUNC(psxsio1_device::write_rxd));
	rs232_network.dsr_handler().set(*sio1, FUNC(psxsio1_device::write_dsr));

	ns16550_device &uart(*subdevice<ns16550_device>("fdc37c665gt:uart2"));
	uart.out_tx_callback().set(rs232, FUNC(rs232_port_device::write_txd));
	uart.out_dtr_callback().set(rs232, FUNC(rs232_port_device::write_dtr));
	uart.out_rts_callback().set(rs232, FUNC(rs232_port_device::write_rts));
	rs232.rxd_handler().set(uart, FUNC(ns16550_device::rx_w));
	rs232.dcd_handler().set(uart, FUNC(ns16550_device::dcd_w));
	rs232.dsr_handler().set(uart, FUNC(ns16550_device::dsr_w));
	rs232.ri_handler().set(uart, FUNC(ns16550_device::ri_w));
	rs232.cts_handler().set(uart, FUNC(ns16550_device::cts_w));
}

void twinkle_state::twinklex(machine_config &config)
{
	twinkle(config);
	twinkle_dvd_type1(config);
	X76F041(config, "security");
}

void twinkle_state::twinklex2(machine_config &config)
{
	twinkle(config);
	twinkle_dvd_type2(config);
	X76F041(config, "security");
}

void twinkle_state::twinklei(machine_config &config)
{
	twinkle(config);
	twinkle_dvd_type2(config);
	I2C_M24C02(config, "security", 0); // M24C02-W
}

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

	PORT_START("SPU_DSW")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "SPU DSW:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "SPU DSW:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "SPU DSW:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SPU DSW:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SPU DSW:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SPU DSW:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "SPU DSW:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "SPU DSW:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( twinklex )
	PORT_INCLUDE( twinkle )

	PORT_MODIFY("OUTSEC")
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("security", x76f041_device, write_scl)
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("security", x76f041_device, write_sda)
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("security", x76f041_device, write_cs)

	PORT_MODIFY("INSEC")
	PORT_BIT( 0x00001000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("security", x76f041_device, read_sda)
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
	ROM_LOAD16_WORD_SWAP( "863a05.2x",    0x000000, 0x080000, CRC(6f42a09e) SHA1(cab5209f90f47b9ee6e721479913ad74e3ba84b1) )

ROM_START( gq863 )
	TWINKLE_BIOS
ROM_END

ROM_START( bmiidx )
	TWINKLE_BIOS

	ROM_REGION( 0x224, "security", 0 )
	ROM_LOAD( "863a02", 0x000000, 0x000224, BAD_DUMP CRC(078be99f) SHA1(7def88d18a9250a8e4b54a51bf663161676cd9be) )

	DISK_REGION( "scsi:4:cdrom" ) // program
	DISK_IMAGE_READONLY( "gq863-jab01", 0, SHA1(331f80b40ed560c7e017621b7daeeb8275d92b9a) )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY( "gq863a04", 0, SHA1(25359f0eaff3749a6194a6b9d93f6aec67d94819) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE_READONLY( "863hdda01", 0, SHA1(0b8dbf1c9caf4abf965dbc6e1a8e6329d48b1c90) )
ROM_END

ROM_START( bmiidxa )
	TWINKLE_BIOS

	ROM_REGION( 0x224, "security", 0 )
	ROM_LOAD( "863a02", 0x000000, 0x000224, BAD_DUMP CRC(078be99f) SHA1(7def88d18a9250a8e4b54a51bf663161676cd9be) )

	DISK_REGION( "scsi:4:cdrom" ) // program
	DISK_IMAGE_READONLY( "gq863a01", 0, SHA1(07fc467f6500504729becbaf77dabc093a134e65) )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY( "gq863a04", 0, SHA1(25359f0eaff3749a6194a6b9d93f6aec67d94819) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE_READONLY( "863hdda01", 0, SHA1(0b8dbf1c9caf4abf965dbc6e1a8e6329d48b1c90) )
ROM_END

ROM_START( bmiidx2 )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "985a02", 0x000000, 0x000100, CRC(059c1f99) SHA1(f094a12c9a56d351667746a765804c94cb3f96bb) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "gc985a01", 0, SHA1(0b783f11317f64552ebf3323459139529e7f315f) )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY( "gc985a04", 0, SHA1(4306417f61eb1ea92894d288cdb7c385eb4610f2) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE_READONLY( "985hdda01", 0, SHA1(a5b9ec9a5afa38f36af529d3aea5355ea7d022ca) )
ROM_END

ROM_START( bmiidx3 )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "992a02", 0x000000, 0x000100, CRC(92520992) SHA1(c4a47dd0e805807df0f086fd9602c000fe2baa61) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "gc992-jac01", 0, SHA1(c02d6e58439be678ec0d7171eae2dfd53a21acc7) )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY( "gc992-jaa04", 0, SHA1(66d0b9ac793ff3fdddd0aa2aa5f2809d0c295944) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE_READONLY( "992hdda01", 0, SHA1(c3936ae9368b23a3a6a876b668cac67a4c9d1287) )
ROM_END

ROM_START( bmiidx3b )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "992a02", 0x000000, 0x000100, CRC(92520992) SHA1(c4a47dd0e805807df0f086fd9602c000fe2baa61) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "gc992-jab01", 0, SHA1(7a5bc08e5723c2705fb7954a8bb727f96514ec2e) )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY( "gc992-jaa04", 0, SHA1(66d0b9ac793ff3fdddd0aa2aa5f2809d0c295944) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE_READONLY( "992hdda01", 0, SHA1(c3936ae9368b23a3a6a876b668cac67a4c9d1287) )
ROM_END

ROM_START( bmiidx3a )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "992a02", 0x000000, 0x000100, CRC(92520992) SHA1(c4a47dd0e805807df0f086fd9602c000fe2baa61) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "gc992-jaa01", 0, SHA1(d86dab5c8130859e73a3e2936b7fd6231e1c025d) )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY( "gc992-jaa04", 0, SHA1(66d0b9ac793ff3fdddd0aa2aa5f2809d0c295944) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE_READONLY( "992hdda01", 0, SHA1(c3936ae9368b23a3a6a876b668cac67a4c9d1287) )
ROM_END

ROM_START( bmiidx4 )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "a03", 0x000000, 0x000100, CRC(9ccf71f9) SHA1(01c4060ac6e2cdc156c034797ea2e305cbbc31ef) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "a03jaa01", 0, SHA1(f54fc778c2187ccd950402a159babef956b71492) )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY( "a03jaa02", 0, SHA1(d6f01d666e8de285a02215f7ef987073e2b25019) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE_READONLY( "a03jaa03", 0, SHA1(a9814c60d2ed98b8c4f6e11ea762518a1712e7b5) )
ROM_END

ROM_START( bmiidx5 )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "a17", 0x000000, 0x000100, CRC(70ae9983) SHA1(de0b9ce8ca09e794a35722737958fa7ee6eef6ce) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "a17jaa01", 0, SHA1(5ac46973b42b2c66ae63297d1a7fd69b33ef4d1d) )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY( "a17jaa02", 0, SHA1(cc24a4c3f5e7c77dbeee7db94c0cc8a330e2b51b) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE_READONLY( "a17jaa03", 0, SHA1(f8c9b1af4ad15bb9cc37f0d234949a4342f1ca34) )
ROM_END

ROM_START( bmiidx6 )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "b4u", 0x000000, 0x000100, CRC(5c134715) SHA1(2bc40388c5e2d54e99e9fdfd63216820b5c939c0) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "b4ujab01", 0, SHA1(aaae77f473c4a44ce6838da3ef6dab27e4afa0e4) )

	DISK_REGION( "cdrom1" ) // DVD
	DISK_IMAGE_READONLY( "b4ujaa02", 0, SHA1(70c85f6df6f21b96c02e4eefc224593edcaf9e63) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE_READONLY( "b4ujaa03", 0, SHA1(cfcbdfab157a864cbd4ac83247be5d62218f5b72) )
ROM_END

ROM_START( bmiidx6a )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "b4u", 0x000000, 0x000100, CRC(5c134715) SHA1(2bc40388c5e2d54e99e9fdfd63216820b5c939c0) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "b4ujaa01", 0, SHA1(22589f2a2b421e910d8dc3d98b375d7939a94921) )

	DISK_REGION( "cdrom1" ) // DVD
	DISK_IMAGE_READONLY( "b4ujaa02", 0, SHA1(70c85f6df6f21b96c02e4eefc224593edcaf9e63) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE_READONLY( "b4ujaa03", 0, SHA1(cfcbdfab157a864cbd4ac83247be5d62218f5b72) )
ROM_END

ROM_START( bmiidx7 )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "b44", 0x000000, 0x000100, CRC(fb6aaa40) SHA1(7a93a8b69d71c2e448d176a6a9d175a01bd8a5f1) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "b44jab01", 0, SHA1(f04411b2c7a671dc9544635b187db7a5f3eae6aa) )

	DISK_REGION( "cdrom1" ) // DVD
	DISK_IMAGE_READONLY( "b44jaa02", 0, SHA1(a45726d99025f4d824ec143ef92957c76c08a13a) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE_READONLY( "b44jaa03", 0, SHA1(1adb8e4874e26e8ccd9822e6f9dd12f6e6f8af05) )
ROM_END

ROM_START( bmiidx7a )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "b44", 0x000000, 0x000100, CRC(fb6aaa40) SHA1(7a93a8b69d71c2e448d176a6a9d175a01bd8a5f1) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "b44jaa01", 0, SHA1(57fb0312d8102e959658e48a97e46aa16e592b60) )

	DISK_REGION( "cdrom1" ) // DVD
	DISK_IMAGE_READONLY( "b44jaa02", 0, SHA1(a45726d99025f4d824ec143ef92957c76c08a13a) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE_READONLY( "b44jaa03", 0, SHA1(1adb8e4874e26e8ccd9822e6f9dd12f6e6f8af05) )
ROM_END

ROM_START( bmiidx8 )
	TWINKLE_BIOS

	ROM_REGION( 0x100, "security", 0 )
	ROM_LOAD( "c44", 0x000000, 0x000100, CRC(3afc8048) SHA1(36cf01288a5ca4f03060de44a89472650e9f8dcc) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "c44jaa01", 0, SHA1(dd2aeb925182ed75ade510ca3a0f913d667a2be2) )

	DISK_REGION( "cdrom1" ) // DVD
	DISK_IMAGE_READONLY( "c44jaa02", 0, SHA1(f4c454a6360c507a122888d5bc3311eed5ce083b) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE_READONLY( "c44jaa03", 0, SHA1(14df5039a4f5a648f1a2d12a35c16f56d0f9cd28) )
ROM_END

ROM_START( bmiidxc )
	TWINKLE_BIOS

	ROM_REGION( 0x224, "security", 0 )
	ROM_LOAD( "863a02", 0x000000, 0x000224, BAD_DUMP CRC(078be99f) SHA1(7def88d18a9250a8e4b54a51bf663161676cd9be) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "896jabbm", 0, SHA1(09fb638bc5b3e64af13ae3df66ba25e490440946) )

	DISK_REGION( "cdrom1" ) // video CD, same as bmiidx
	DISK_IMAGE_READONLY( "gq863a04", 0, SHA1(25359f0eaff3749a6194a6b9d93f6aec67d94819) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE_READONLY( "863hdda01", 0, SHA1(0b8dbf1c9caf4abf965dbc6e1a8e6329d48b1c90) )
ROM_END

ROM_START( bmiidxca )
	TWINKLE_BIOS

	ROM_REGION( 0x224, "security", 0 )
	ROM_LOAD( "863a02", 0x000000, 0x000224, BAD_DUMP CRC(078be99f) SHA1(7def88d18a9250a8e4b54a51bf663161676cd9be) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "896jaabm", 0, SHA1(ea7205f86543d9273efcc226666ab530c32b23c1) )

	DISK_REGION( "cdrom1" ) // video CD, same as bmiidx
	DISK_IMAGE_READONLY( "gq863a04", 0, SHA1(25359f0eaff3749a6194a6b9d93f6aec67d94819) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE_READONLY( "863hdda01", 0, SHA1(0b8dbf1c9caf4abf965dbc6e1a8e6329d48b1c90) )
ROM_END

ROM_START( bmiidxs )
	TWINKLE_BIOS

	ROM_REGION( 0x224, "security", 0 )
	ROM_LOAD( "983a02", 0x000000, 0x000224, BAD_DUMP CRC(6a6ace82) SHA1(1e1373f40c469c117316c03db414d9984567dd42) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "gc983a01", 0, SHA1(7a80380f9c18c7da9643e0b9954ad8367eda5948) )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY( "gc983a04", 0, SHA1(73454f2acb5a1e6b9e21140eb7b93a4827072d63) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE_READONLY( "983hdda01", 0, SHA1(bcbbf55acf8bebc5773ffc5769420a0129f4da57) )
ROM_END

ROM_START( bmiidxsa )
	TWINKLE_BIOS

	ROM_REGION( 0x224, "security", 0 )
	ROM_LOAD( "983aa02", 0x000000, 0x000224, BAD_DUMP CRC(bcc8965c) SHA1(e152d19a92544212e321a332c6e6678d623dab21) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "gc983aa,a01", 0, SHA1(9ef5725fc79a7f4f524ef93849af42b2758102cd) )

	DISK_REGION( "cdrom1" ) // video CD
	DISK_IMAGE_READONLY( "gc983a04", 0, SHA1(73454f2acb5a1e6b9e21140eb7b93a4827072d63) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE_READONLY( "983hdda01", 0, SHA1(bcbbf55acf8bebc5773ffc5769420a0129f4da57) )
ROM_END

ROM_START( bmiidxc2 )
	TWINKLE_BIOS

	ROM_REGION( 0x224, "security", 0 )
	ROM_LOAD( "984a02", 0x000000, 0x000224, BAD_DUMP CRC(786db814) SHA1(722c709d95d54cd519856ddea64b9176ef191b0d) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "ge984a01,bm", 0, SHA1(03b083ba09652dfab6f328000c3c9de2a7a4e618) )

	DISK_REGION( "cdrom1" ) // video CD, same as bmiidxs
	DISK_IMAGE_READONLY( "gc983a04", 0, SHA1(73454f2acb5a1e6b9e21140eb7b93a4827072d63) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE_READONLY( "983hdda01", 0, SHA1(bcbbf55acf8bebc5773ffc5769420a0129f4da57) )
ROM_END

} // anonymous namespace


GAMEL( 1999, gq863,    0,       twinkle,  twinkle,  twinkle_state, empty_init, ROT0, "Konami", "Twinkle System", MACHINE_IS_BIOS_ROOT, layout_bmiidx )

GAMEL( 1999, bmiidx,   gq863,   twinklex, twinklex, twinkle_state, empty_init, ROT0, "Konami", "beatmania IIDX (863 JAB)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_bmiidx )
GAMEL( 1999, bmiidxa,  bmiidx,  twinklex, twinklex, twinkle_state, empty_init, ROT0, "Konami", "beatmania IIDX (863 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_bmiidx )
GAMEL( 1999, bmiidxc,  gq863,   twinklex, twinklex, twinkle_state, empty_init, ROT0, "Konami", "beatmania IIDX with DDR 2nd Club Version (896 JAB)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_bmiidx )
GAMEL( 1999, bmiidxca, bmiidxc, twinklex, twinklex, twinkle_state, empty_init, ROT0, "Konami", "beatmania IIDX with DDR 2nd Club Version (896 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING, layout_bmiidx )
GAMEL( 1999, bmiidxs,  gq863,   twinklex2,twinklex, twinkle_state, empty_init, ROT0, "Konami", "beatmania IIDX Substream (983 JAA)", MACHINE_IMPERFECT_SOUND, layout_bmiidx )
GAMEL( 1999, bmiidxsa, bmiidxs, twinklex2,twinklex, twinkle_state, empty_init, ROT0, "Konami", "beatmania IIDX Substream (983-AA JAA)", MACHINE_IMPERFECT_SOUND, layout_bmiidx )
GAMEL( 1999, bmiidxc2, gq863,   twinklex2,twinklex, twinkle_state, empty_init, ROT0, "Konami", "beatmania IIDX Substream with DDR 2nd Club Version 2 (984 A01 BM)", MACHINE_IMPERFECT_SOUND, layout_bmiidx )
GAMEL( 1999, bmiidx2,  gq863,   twinklei, twinklei, twinkle_state, empty_init, ROT0, "Konami", "beatmania IIDX 2nd style (GC985 JAA)", MACHINE_IMPERFECT_SOUND, layout_bmiidx )
GAMEL( 2000, bmiidx3,  gq863,   twinklei, twinklei, twinkle_state, empty_init, ROT0, "Konami", "beatmania IIDX 3rd style (GC992 JAC)", MACHINE_IMPERFECT_SOUND, layout_bmiidx )
GAMEL( 2000, bmiidx3b, bmiidx3, twinklei, twinklei, twinkle_state, empty_init, ROT0, "Konami", "beatmania IIDX 3rd style (GC992 JAB)", MACHINE_IMPERFECT_SOUND, layout_bmiidx )
GAMEL( 2000, bmiidx3a, bmiidx3, twinklei, twinklei, twinkle_state, empty_init, ROT0, "Konami", "beatmania IIDX 3rd style (GC992 JAA)", MACHINE_IMPERFECT_SOUND, layout_bmiidx )
GAMEL( 2000, bmiidx4,  gq863,   twinklei, twinklei, twinkle_state, empty_init, ROT0, "Konami", "beatmania IIDX 4th style (GCA03 JAA)", MACHINE_IMPERFECT_SOUND, layout_bmiidx )
GAMEL( 2001, bmiidx5,  gq863,   twinklei, twinklei, twinkle_state, empty_init, ROT0, "Konami", "beatmania IIDX 5th style (GCA17 JAA)", MACHINE_IMPERFECT_SOUND, layout_bmiidx )
GAMEL( 2001, bmiidx6,  gq863,   twinklei, twinklei, twinkle_state, empty_init, ROT0, "Konami", "beatmania IIDX 6th style (GCB4U JAB)", MACHINE_IMPERFECT_SOUND, layout_bmiidx )
GAMEL( 2001, bmiidx6a, bmiidx6, twinklei, twinklei, twinkle_state, empty_init, ROT0, "Konami", "beatmania IIDX 6th style (GCB4U JAA)", MACHINE_IMPERFECT_SOUND, layout_bmiidx )
GAMEL( 2002, bmiidx7,  gq863,   twinklei, twinklei, twinkle_state, empty_init, ROT0, "Konami", "beatmania IIDX 7th style (GCB44 JAB)", MACHINE_IMPERFECT_SOUND, layout_bmiidx )
GAMEL( 2002, bmiidx7a, bmiidx7, twinklei, twinklei, twinkle_state, empty_init, ROT0, "Konami", "beatmania IIDX 7th style (GCB44 JAA)", MACHINE_IMPERFECT_SOUND, layout_bmiidx )
GAMEL( 2002, bmiidx8,  gq863,   twinklei, twinklei, twinkle_state, empty_init, ROT0, "Konami", "beatmania IIDX 8th style (GCC44 JAA)", MACHINE_IMPERFECT_SOUND, layout_bmiidx )
