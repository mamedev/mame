// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*  Konami Hornet System
    Driver by Ville Linde


***************************************************************************

Konami 'Hornet' Hardware, Konami, 1997-2000
Hardware info by Guru
Last updated: 22nd April 2021
-----------------------------

Known games on this hardware include....

Game                             (C)      Year
----------------------------------------------
Gradius 4 : Fukkatsu             Konami   1998
NBA Play by Play                 Konami   1998
Teraburst                        Konami   1998
Thrill Drive                     Konami   1999
Silent Scope                     Konami   1999
Silent Scope 2                   Konami   2000


Quick hardware overview:

GN715 CPU Board:
----------------
IBM PowerPC 403GA at 32MHz (main CPU)
Motorola MC68EC000 at 16MHz (sound CPU)
Konami K056800 (MIRAC), sound system interface
Ricoh RF5c400 sound chip

GN715 GFX Board:
----------------
Analog Devices ADSP-21062 SHARC DSP at 36MHz
Konami 0000037122 (2D Tilemap)
Konami 0000033906 (PCI bridge)
3DFX 500-0003-03 (Voodoo) FBI with 2MB RAM
3DFX 500-0004-02 (Voodoo) TMU with 4MB RAM

GQ871 GFX Board:
----------------
Analog Devices ADSP-21062 SHARC DSP at 36MHz
Konami 0000037122 (2D Tilemap)
Konami 0000033906 (PCI bridge)
3DFX 500-0009-01 (Voodoo 2) FBI with 2MB RAM
3DFX 500-0010-01 (Voodoo 2) TMU with 4MB RAM


Specific game hardware configurations:
-------------------------------------

Game              KONAMI ID  CPU PCB    GFX Board(s)  notes
----------------------------------------------------------------------
Gradius 4         GX837      GN715(A)   GN715(B)
NBA Play By Play  GX778      GN715(A)   GN715(B)
Teraburst         GX715      GN715(A)   GN715(B)      GN680(E) I/O board
Thrill Drive      GE713UF    GN715(A)   GN715(B)      GN676-PWB(H)A LAN PCB
Silent Scope      GQ830      GN715(A)   2x GN715(B)
Silent Scope      GQ830      GN715(A)   2x GQ871(B)
Silent Scope 2    GQ931      GN715(A)   2x GQ871(B)   GQ931(H) LAN PCB
Silent Scope 2    GQ931      GN715(A)   2x GN715(B)   GQ931(H) LAN PCB


PCB Layouts
-----------

Top Board
GN715 PWB(A)A
|--------------------------------------------------------------|
| SP485CS CN10       CN11        CN9          JP8 JP9 JP10 JP11|
|CN19                                                    PAL1  |
|CN21       JP13 PAL2             68EC000          EPROM.7S    |
|   NE5532       PAL3                                      CN12|
|           JP12  JP16    DRM1M4SJ8                        CN13|
|   NE5532                            MASKROM.9P    MASKROM.9T |
|     SM5877 JP15         RF5C400                              |
|CN18                                 MASKROM.12P   MASKROM.12T|
|     SM5877     16.9344MHz                                    |
|CN14            SRAM256K             MASKROM.14P   MASKROM.14T|
|                                                              |
|CN16            SRAM256K             MASKROM.16P   MASKROM.16T|
|  ADC12138                                                    |
|             056800            JP5                            |
|                               JP4                            |
|                      MACH111  JP3                |---------| |
|   TEST_SW                         EPROM.22P      |         | |
|CN1                   DRAM16X16                   |PPC403GA | |
|                                   EPROM.25P      |         | |
|                                                  |         | |
|                      DRAM16X16    EPROM.27P      |---------| |
| 4AK16                                                     JP6|
|                                                              |
|CN3                                                           |
| 0038323  PAL4                                       7.3728MHz|
| E9825    058232           CN2                                |
|                                                     50.000MHz|
|    RESET_SW               CN5                    JP1  JP2    |
|M48T58Y-70PC1  CN4                 CN6               64.000MHz|
|--------------------------------------------------------------|
Notes:
      DRM1M4SJ8 - Fujitsu 81C4256 256Kx4 DRAM (SOJ24)
       SRAM256K - Cypress CY7C199 32kx8 SRAM (SOJ28)
      DRAM16X16 - Fujitsu 8118160A-60 16megx16 DRAM (SOJ42)
  0038323 E9825 - SOIC8 (Secured PIC?). I've seen a similar chip in the security cart of System573
  M48T58Y-70PC1 - ST Timekeeper RAM
        RF5C400 - Ricoh RF5C400 PCM 32Ch, 44.1 kHz Stereo, 3D Effect Spatializer, clock input 16.9344MHz
         056800 - Konami Custom (QFP80)
         058232 - Konami Custom Ceramic Package (SIL14)
       ADC12138 - National Semiconductor ADC12138 A/D Converter, 12-bit + Serial I/O With MUX (SOP28)
        MACH111 - AMD MACH111 CPLD (Stamped 'N676A1', PLCC44)
        68EC000 - Motorola MC68EC000, running at 16.0MHz (64/4)
       PPC403GA - IBM PowerPC 403GA CPU, clock input 32.0MHz (QFP160)
       SM5877AM - Nippon Precision Circuits 3rd Order 2-Channel D/A Converter (SOIC24)
          4AK16 - Hitachi 4AK16 Silicon N-Channel Power MOS FET Array (SIL10)
       NE5532AN - Philips, Dual Low-Noise High-Speed Audio OP Amp (DIP8)
        SP485CS - Sipex SP485CS Low Power Half Duplex RS485 Transceiver (DIP8)
           PAL1 - AMD PALCE16V8 (stamped 'N676A4', DIP20)
           PAL2 - AMD PALCE16V8 (stamped 'N676A2', DIP20)
           PAL3 - AMD PALCE16V8 (stamped 'N676A3', DIP20)
           PAL4 - AMD PALCE16V8 (stamped 'N676A5', DIP20)
            JP1 -       25M O O-O 32M
            JP2 -       25M O O-O 32M
            JP3 -        RW O O O RO
            JP4 - PROG  32M O O-O 16M
            JP5 - DATA  32M O-O O 16M
            JP6 - BOOT   16 O-O O 32
            JP7 - SRC DOUT2 O O-O 0
            JP8 -   64M&32M O-O O 16M
            JP9 -       64M O O-O 32M&16M
           JP10 -   64M&32M O-O O 16M
           JP11 -       64M O O-O 32M&16M
           JP12 -   through O-O O SP
           JP13 -   through O-O O SP
           JP14 - WDT       O O
           JP15 -      MONO O-O O SURR
           JP16 -      HIGH O O O MID (N/C LOW)
    CN1 to  CN3 - Multi-pin Flat Cable Connector
            CN4 - Multi-pin Connector for Network PCB
            CN5 - Multi-pin Flat Cable Connector
            CN6 - 96-Pin To Lower PCB, Joining Connector
    CN7 to  CN8 - Not used
    CN9 to CN11 - 6-Pin Power Connectors
           CN19 - USB Connector
           CN21 - 5-Pin Analog Controls Connector (Tied to USB Connector via the Filter Board)
           CN18 - RCA Mono Audio OUT
    CN14 & CN16 - RCA Stereo Audio OUT


ROM Usage
---------
             |------------------------------- ROM Locations ---------------------------------------|
Game         27P     25P     22P     16P     14P     12P     9P      16T     14T     12T  9T  7S
----------------------------------------------------------------------------------------------------
Gradius 4    837C01  -       -       837A09  837A10  -       778A12  837A04  837A05  -    -   837A08
NBA P/Play   778A01  -       -       778A09  778A10  778A11  778A12  778A04  778A05  -    -   778A08
Teraburst    -       715l02  715l03  715A09  715A10  -       778A12  715A04  715A05  -    -   715A08
Thrill Drive 713AB01 -       -       713A09  713A10  -       -       713A04  713A05  -    -   713A08
S/Scope      830B01  -       -       830A09  830A10  -       -       -       -       -    -   830A08
S/Scope 2    931D01  -       -       931A09  931A10  931A11  -       931A04  -       -    -   931A08


Bottom Board
GN715 PWB(B)A
|--------------------------------------------------------------|
|CN4        CN2    CN8               CN6                    CN5|
|JP1                 |---------|              4M_EDO   4M_EDO  |
|                    |         |     |----------|              |
|  4M_EDO 4M_EDO     | TEXELFX |     |          |              |
|                    |         |     | PIXELFX  |       4M_EDO |
|  4M_EDO 4M_EDO     |         |     |          |       4M_EDO |
|                    |---------|     |          | |--------|   |
|  4M_EDO 4M_EDO                     |----------| |KONAMI  |   |
|CN3                                50MHz JP7     |33906   |   |
|  4M_EDO 4M_EDO                          JP6     |        |   |
|                       256KSRAM 256KSRAM         |--------|   |
|CN7                                                           |
|         AV9170                     1MSRAM 1MSRAM             |
| MC44200                                                      |
|                       256KSRAM 256KSRAM                      |
|                                    1MSRAM 1MSRAM             |
|  |-------|                                    MASKROM.24U    |
|  |KONAMI |  MACH111  |-------------|              MASKROM.24V|
|  |37122  |           |ANALOG       |   1MSRAM 1MSRAM         |
|  |       |           |DEVICES      |                         |
|  |-------|       JP5 |ADSP-21062   |   36.00MHz              |
|1MSRAM                |SHARC        |   1MSRAM 1MSRAM         |
|                      |             |                         |
|1MSRAM                |             |                         |
|           256KSRAM   |-------------|          MASKROM.32U    |
|1MSRAM     256KSRAM                                MASKROM.32V|
|           256KSRAM     PAL1  PAL2            JP4             |
|1MSRAM                                                        |
|           JP2                 CN1            JP3             |
|--------------------------------------------------------------|
Notes:
      4M_EDO - Silicon Magic SM81C256K16CJ-35 EDO DRAM 66MHz (SOJ40)
      1MSRAM - Cypress CY7C109-25VC 1Meg SRAM (SOJ32)
    256KSRAM - Winbond W24257AJ-15 256K SRAM (SOJ28)
     TEXELFX - 3DFX 500-0004-02 BD0665.1 TMU (QFP208)
     PIXELFX - 3DFX 500-0003-03 F001701.1 FBI (QFP240)
  0000037122 - Konami Custom (QFP208)
   MC44200FT - Motorola MC44200FT 3 Channel Video D/A Converter (QFP44)
     MACH111 - AMD MACH111 CPLD (Stamped 'N715B1', PLCC44)
      AV9170 - Integrated Circuit Systems Inc. Clock Multiplier (SOIC8)
        PAL1 - AMD PALCE16V8 (stamped 'N676B4', DIP20)
        PAL2 - AMD PALCE16V8 (stamped 'N676B5', DIP20)
         JP1 -    SCR O O-O TWN
         JP2 - MASTER O-O O SLAVE
         JP3 -    16M O O-O 32M
         JP4 -    32M O-O O 16M
         JP5 -  ASYNC O O-O SYNC
         JP6 -    DSP O O-O ADCK
         JP7 -    MCK O-O O SCK
         CN1 - 96 Pin To Lower PCB, Joining Connector
         CN2 - 8-Pin RGB OUT
         CN3 - 15-Pin DSUB VGA Video MAIN OUT
         CN4 - 6-Pin Power Connector
         CN5 - 4-Pin Power Connector
         CN6 - 2-Pin Connector (Not Used)
         CN7 - 15-Pin DSUB VGA Video MAIN OUT
         CN8 - 6-Pin Connector (Not Used)

ROM Usage
---------
             |------ ROM Locations -------|
Game         24U     24V     32U     32V
-------------------------------------------
Gradius 4    837A13  837A15  837A14  837A16
NBA P/Play   778A13  778A15  778A14  778A16
Teraburst    715A13  715A15  778A14  715A16
Thrill Drive 713A13  -       713A14  -
S/Scope      830A13  -       830A14  -
S/Scope 2    -       -       -       -      (no ROMs, not used)


Teraburst uses a different variation of the I/O board used in Operation: Thunder Hurricane (see gticlub.cpp). Analog inputs are controlled by
two CCD cameras, one from each gun. This specific variation uses a K056800 which normally acts as a sound interface controller. Perhaps this
either sends analog inputs to the main pcb or isn't used at all. No network connection is involved in this setup as this board directly connects
to the main pcb via joining connector.

GN680 PWB(E)403381B
|------------------------------------------|
|CN11  CN12    CN8      CN9    CN10  DSW(4)|
|                 NRPS11     NRPS11        |
|                                          |
|                        LM1881   LM1881   |
|                                          |
|LED(x4)                                   |
|                                          |
|           68EC000FN16  8464              |
|    RESET_SW            8464              |
|32MHz                           715A17.20K|
|8464                 PAL(002962)          |
|      056800         PAL(002961)          |
|   PAL(056787A)      PAL(002960)          |
|   CN1                                    |
|------------------------------------------|
Notes:
  68EC000 @ 16MHz (32/2)
  CN11/12 - Power connectors
  CN8/9   - 6-pin analog control connectors (to CCD cameras)
  CN1     - Lower joining connector to main pcb
  NRPS11  - Idec NRPS11 PC Board circuit protector
  LM1881  - Video sync separator (DIP8)
  056800  - Konami Custom (QFP80)


LAN PCB: GQ931 PWB(H)      (C) 1999 Konami
------------------------------------------

2 x LAN ports, LANC(1) & LANC(2)
1 x 32.0000MHz Oscillator

     HYC2485S  SMC ARCNET Media Transceiver, RS485 5Mbps-2.5Mbps
8E   931A19    Konami 32meg masked ROM, ROM0 (compressed GFX data)
6E   931A20    Konami 32meg masked ROM, ROM1 (compressed GFX data)
12F  XC9536    Xilinx  CPLD,  44 pin PLCC, Konami no. Q931H1
12C  XCS10XL   Xilinx  FPGA, 100 pin PQFP, Konami no. 4C
12B  CY7C199   Cypress 32kx8 SRAM
8B   AT93C46   Atmel 1K serial EEPROM, 8 pin SOP
16G  DS2401    Dallas Silicon Serial Number IC, 6 pin SOP

Note: This PCB does more than just networking. The serial eeprom is used as a means to prevent region change.
The timekeeper region has to match the serial eeprom. The two mask roms serve as GFX roms as the game "downloads"
the data from those two roms.


GFX PCB: GQ871 PWB(B)A (C) 1999 Konami
--------------------------------------

There are no ROMs on the two GFX PCBs, all sockets are empty. They are located on the LAN PCB.
Prior to the game starting there is a message saying downloading data.

Jumpers set on GFX PCB to main monitor:
4A   set to TWN (twin GFX PCBs)
37D  set to Master

Jumpers set on GFX PCB to scope monitor:
4A   set to TWN (twin GFX PCBs)
37D  set to Slave

1x 64.0000MHz
1x 36.0000MHz  (to 27L, ADSP)

21E  AV9170           ICS, Clock synchroniser and multiplier
27L  ADSP-21062       Analog Devices SHARC ADSP, 512k flash, Konami no. 022M16C
15T  0000033906       Konami Custom, 160 pin PQFP
19R  W241024AI-20     Winbond, 1Meg SRAM
22R  W241024AI-20     Winbond, 1Meg SRAM
25R  W241024AI-20     Winbond, 1Meg SRAM
29R  W241024AI-20     Winbond, 1Meg SRAM
19P  W241024AI-20     Winbond, 1Meg SRAM
22P  W241024AI-20     Winbond, 1Meg SRAM
25P  W241024AI-20     Winbond, 1Meg SRAM
29P  W241024AI-20     Winbond, 1Meg SRAM
18N  W24257AJ-15      Winbond, 256K SRAM
14N  W24257AJ-15      Winbond, 256K SRAM
18M  W24257AJ-15      Winbond, 256K SRAM
14M  W24257AJ-15      Winbond, 256K SRAM
28D  000037122        Konami Custom, 208 pin PQFP
33E  W24257AJ-15      Winbond, 256K SRAM
33D  W24257AJ-15      Winbond, 256K SRAM
33C  W24257AJ-15      Winbond, 256K SRAM
27A  W241024AI-20     Winbond, 1Meg SRAM
30A  W241024AI-20     Winbond, 1Meg SRAM
32A  W241024AI-20     Winbond, 1Meg SRAM
35A  W241024AI-20     Winbond, 1Meg SRAM
7K   500-0010-01      3DFX, Texture processor
16F  SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
13F  SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
9F   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
5F   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
16D  SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
13D  SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
9D   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
5D   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
9P   500-0009-01      3DFX, Pixel processor
10U  SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
7U   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
3S   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
3R   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
27G  XC9536           Xilinx, CPLD, Konami no. Q830B1
21C  MC44200FT        Motorola, 3 Channel video D/A converter

***************************************************************************
*/

#include "emu.h"

#include "k037122.h"
#include "konami_gn676_lan.h"
#include "konppc.h"
#include "konppc_jvshost.h"
#include "windy2.h"

#include "cpu/m68000/m68000.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/sharc/sharc.h"
#include "machine/adc1213x.h"
#include "machine/ds2401.h"
#include "machine/eepromser.h"
#include "machine/k033906.h"
#include "machine/timekpr.h"
#include "machine/watchdog.h"
#include "machine/x76f041.h"
#include "sound/k056800.h"
#include "sound/rf5c400.h"
#include "video/voodoo_2.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "layout/generic.h"

#define LOG_SYSREG (1 << 1)
#define LOG_COMM   (1 << 2)

#define LOG_ALL (LOG_SYSREG | LOG_COMM)

#define VERBOSE (0)

#include "logmacro.h"

#define LOGSYSREG(...) LOGMASKED(LOG_SYSREG, __VA_ARGS__)
#define LOGCOMM(...)   LOGMASKED(LOG_COMM, __VA_ARGS__)

namespace {

class hornet_state : public driver_device
{
public:
	hornet_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_workram(*this, "workram"),
		m_sharc_dataram(*this, "sharc%u_dataram", 0U),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k056800(*this, "k056800"),
		m_dsp(*this, "dsp%u", 1U),
		m_k037122(*this, "k037122_%u", 0U),
		m_adc12138(*this, "adc12138"),
		m_adc12138_sscope(*this, "adc12138_sscope"),
		m_konppc(*this, "konppc"),
		m_x76f041(*this, "security_eeprom"),
		m_voodoo(*this, "voodoo%u", 0U),
		m_watchdog(*this, "watchdog"),
		m_jvs_host(*this, "jvs_host"),
		m_k033906(*this, "k033906_%u", 1U),
		m_gn676_lan(*this, "gn676_lan"),
		m_cgboard_bank(*this, "cgboard_%u_bank", 0U),
		m_in(*this, "IN%u", 0U),
		m_dsw(*this, "DSW"),
		m_eepromout(*this, "EEPROMOUT"),
		m_analog(*this, "ANALOG%u", 1U),
		m_pcb_digit(*this, "pcbdigit%u", 0U),
		m_cg_view(*this, "cg_view")
	{ }

	void hornet(machine_config &config);
	void hornet_x76(machine_config &config);
	void hornet_lan(machine_config &config);
	void nbapbp(machine_config &config);
	void sscope(machine_config &config);
	void sscope_voodoo2(machine_config& config);

	void init_hornet();
	void init_sscope();
	void init_gradius4();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// TODO: Needs verification on real hardware
	static const int m_sound_timer_usec = 2800;

	required_shared_ptr<uint32_t> m_workram;
	optional_shared_ptr_array<uint32_t, 2> m_sharc_dataram;
	required_device<ppc4xx_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k056800_device> m_k056800;
	optional_device_array<adsp21062_device, 2> m_dsp;
	optional_device_array<k037122_device, 2> m_k037122;
	required_device<adc12138_device> m_adc12138;
	optional_device<adc12138_device> m_adc12138_sscope;
	required_device<konppc_device> m_konppc;
	optional_device<x76f041_device> m_x76f041;
	optional_device_array<generic_voodoo_device, 2> m_voodoo;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<konppc_jvs_host_device> m_jvs_host;
	optional_device_array<k033906_device, 2> m_k033906;
	optional_device<konami_gn676_lan_device> m_gn676_lan;
	optional_memory_bank_array<2> m_cgboard_bank;
	required_ioport_array<3> m_in;
	required_ioport m_dsw;
	optional_ioport m_eepromout;
	optional_ioport_array<5> m_analog;
	output_finder<2> m_pcb_digit;
	memory_view m_cg_view;

	emu_timer *m_sound_irq_timer = nullptr;

	bool m_sndres = false;

	uint8_t sysreg_r(offs_t offset);
	void sysreg_w(offs_t offset, uint8_t data);
	void soundtimer_en_w(uint16_t data);
	void soundtimer_count_w(uint16_t data);
	double adc12138_input_callback(uint8_t input);
	void jamma_jvs_w(uint8_t data);

	template <uint8_t Which> uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(sound_irq);

	void hornet_map(address_map &map) ATTR_COLD;
	void hornet_lan_map(address_map &map) ATTR_COLD;
	void sscope_map(address_map &map) ATTR_COLD;
	template <unsigned Board> void sharc_map(address_map &map) ATTR_COLD;
	void sound_memmap(address_map &map) ATTR_COLD;
};

// with GN680 I/O board
class terabrst_state : public hornet_state
{
public:
	terabrst_state(const machine_config &mconfig, device_type type, const char *tag) :
		hornet_state(mconfig, type, tag),
		m_gn680(*this, "gn680")
	{ }

	void terabrst(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_gn680;

	uint16_t m_gn680_latch = 0;
	uint16_t m_gn680_ret0 = 0;
	uint16_t m_gn680_ret1 = 0;
	uint16_t m_gn680_check = 0;
	uint16_t m_gn680_reg0e = 0;

	uint16_t gun_r(offs_t offset);
	void gun_w(offs_t offset, uint16_t data);
	void gn680_sysctrl(uint16_t data);
	uint16_t gn680_latch_r();
	void gn680_latch_w(offs_t offset, uint16_t data);

	void terabrst_map(address_map &map) ATTR_COLD;
	void gn680_memmap(address_map &map) ATTR_COLD;
};

// with GQ931 LAN board
class sscope2_state : public hornet_state
{
public:
	sscope2_state(const machine_config &mconfig, device_type type, const char *tag) :
		hornet_state(mconfig, type, tag),
		m_lan_eeprom(*this, "lan_eeprom"),
		m_lan_ds2401(*this, "lan_serial_id"),
		m_comm_board_rom(*this, "comm_board"),
		m_comm_bank(*this, "comm_bank")
	{ }

	void sscope2(machine_config &config);
	void sscope2_voodoo1(machine_config& config);

	void init_sscope2();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<eeprom_serial_93cxx_device> m_lan_eeprom;
	required_device<ds2401_device> m_lan_ds2401;
	required_region_ptr<uint32_t> m_comm_board_rom;
	required_memory_bank m_comm_bank;

	void comm1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void comm_rombank_w(uint32_t data);
	uint32_t comm0_unk_r(offs_t offset, uint32_t mem_mask = ~0);
	uint8_t comm_eeprom_r();
	void comm_eeprom_w(uint8_t data);

	void sscope2_map(address_map &map) ATTR_COLD;
};


template <uint8_t Which>
uint32_t hornet_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_voodoo[Which]->update(bitmap, cliprect);

	m_k037122[Which]->tile_draw(screen, bitmap, cliprect);

	return 0;
}

/*****************************************************************************/

uint8_t hornet_state::sysreg_r(offs_t offset)
{
	uint8_t r = 0;

	switch (offset)
	{
		case 0: // I/O port 0
			r = m_in[0]->read();
			break;
		case 1: // I/O port 1
			r = m_in[1]->read();
			if (m_adc12138_sscope)
			{
				r &= ~7;
				r |= m_adc12138_sscope->do_r() | (m_adc12138_sscope->eoc_r() << 2);
			}
			break;
		case 2: // I/O port 2
			r = m_in[2]->read();
			break;

		case 3: // I/O port 3
			/*
			    0x80 = JVSINIT (JAMMA I/F SENSE)
			    0x40 = COMMST
			    0x20 = GSENSE
			    0x08 = EEPDO (EEPROM DO)
			    0x04 = ADEOC (ADC EOC)
			    0x02 = ADDOR (ADC DOR)
			    0x01 = ADDO (ADC DO)
			*/
			r = 0x70;
			r |= m_jvs_host->sense() << 7;
			if (m_x76f041)
				r |= m_x76f041->read_sda() << 3;
			r |= m_adc12138->do_r() | (m_adc12138->eoc_r() << 2);
			break;

		case 4: // I/O port 4 - DIP switches
			r = m_dsw->read();
			break;

		default:
			break;
	}
	return r;
}

void hornet_state::sysreg_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0: // 7seg LEDs on PCB
		case 1:
			m_pcb_digit[offset] = bitswap<8>(~data,7,0,1,2,3,4,5,6) & 0x7f;
			break;

		case 2: // Parallel data register
			LOGSYSREG("Parallel data = %02X\n", data);

			if (m_adc12138_sscope)
			{
				m_adc12138_sscope->cs_w(BIT(data, 4));
				m_adc12138_sscope->conv_w(BIT(data, 3));
				m_adc12138_sscope->di_w(BIT(data, 5));
				m_adc12138_sscope->sclk_w(BIT(data, 7));
			}
			break;

		case 3: // System Register 0
			/*
			    0x80 = EEPWEN (EEPROM write enable)
			    0x40 = EEPCS (EEPROM CS)
			    0x20 = EEPSCL (EEPROM SCL?)
			    0x10 = EEPDT (EEPROM data) / JVSTXEN (for Gradius 4)
			    0x08 = JVSTXEN / LAMP3 (something about JAMMA interface)
			    0x04 = LAMP2
			    0x02 = LAMP1
			    0x01 = LAMP0

			    The bit used for JVSTXEN changes between 3 and 4 based on the lower 2 bits of IN2.
			    If m_in[2]->read() & 3 != 0, bit 4 is used. Otherwise, bit 3 is used.
			*/
			if (m_x76f041)
				m_x76f041->write_cs(BIT(data, 6));

			LOGSYSREG("System register 0 = %02X\n", data);
			break;

		case 4: // System Register 1
		{
			/*
			    0x80 = SNDRES (sound reset)
			    0x40 = COMRES (COM reset)
			    0x20 = COINRQ2 (EEPROM SCL?)
			    0x10 = COINRQ1 (EEPROM data)
			    0x08 = ADCS (ADC CS)
			    0x04 = ADCONV (ADC CONV)
			    0x02 = ADDI (ADC DI)
			    0x01 = ADDSCLK (ADC SCLK)
			*/

			// Set FPGA into a state to accept new firmware
			if (m_gn676_lan)
				m_gn676_lan->reset_fpga_state(BIT(data, 6));

			if (m_x76f041)
			{
				// HACK: Figure out a way a better way to differentiate between what device it wants to talk to here.
				// I haven't seen a combination of both x76 + adc usage in the available Hornet library so this hack
				// works but there may be a proper way differentiate the two.
				// Not emulating the x76 results in NBA Play By Play becoming regionless/bugged, and not emulating the
				// ADC results in Silent Scope boot looping.
				m_x76f041->write_rst(BIT(data, 2));
				m_x76f041->write_sda(BIT(data, 1));
				m_x76f041->write_scl(BIT(data, 0));
			}
			else
			{
				m_adc12138->cs_w(BIT(data, 3));
				m_adc12138->conv_w(BIT(data, 2));
				m_adc12138->di_w(BIT(data, 1));
				m_adc12138->sclk_w(BIT(data, 0));
			}

			bool const sndres = BIT(data, 7);
			m_audiocpu->set_input_line(INPUT_LINE_RESET, sndres ? CLEAR_LINE : ASSERT_LINE);
			if (sndres != m_sndres)
			{
				// clear interrupts when reset line is triggered
				m_audiocpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
			}

			m_sndres = sndres;

			LOGSYSREG("System register 1 = %02X\n", data);
			break;
		}

		case 5: // Sound Control Register
			/*
			    0x80 = MODE1
			    0x40 = MUTE1
			    0x20 = DEEN1
			    0x10 = ATCK1
			    0x08 = MODE0
			    0x04 = MUTE0
			    0x02 = DEEN0
			    0x01 = ATCK0
			*/
			LOGSYSREG("Sound control register = %02X\n", data);
			break;

		case 6: // WDT Register
			/*
			    0x80 = WDTCLK
			*/
			m_watchdog->reset_line_w(BIT(data, 7));
			break;

		case 7: // CG Control Register
			/*
			    0x80 = EXRES1?
			    0x40 = EXRES0?
			    0x20 = EXID1
			    0x10 = EXID0
			    0x0C = 0x00 = 24kHz, 0x04 = 31kHz, 0x0c = 15kHz
			    0x01 = EXRGB
			*/

			// TODO: The IRQ1 clear line is causing Silent Scope's screen to not update
			// at the correct rate. Bits 6 and 7 always seem to be set even if an IRQ
			// hasn't been called so they don't appear to be responsible for clearing IRQs,
			// and ends up clearing IRQs out of turn.
			// The IRQ0 clear bit is also questionable but games run too fast and crash without it.
			// if (BIT(data, 7))
			//  m_maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
			if (BIT(data, 6))
				m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);

			m_konppc->set_cgboard_id((data >> 4) & 3);
			m_cg_view.select(m_konppc->get_cgboard_id() ? 1 : 0);
			break;
	}
}

/*****************************************************************************/

uint8_t sscope2_state::comm_eeprom_r()
{
	uint8_t r = 0;
	r |= (m_lan_eeprom->do_read() & 1) << 1;
	r |= m_lan_ds2401->read() & 1;
	return r;
}

void sscope2_state::comm_eeprom_w(uint8_t data)
{
	m_eepromout->write(data, 0xff);
	m_lan_ds2401->write(BIT(data, 4));
}

void sscope2_state::comm1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGCOMM("comm1_w: %08X = %08X & %08X\n", offset, data, mem_mask);
}

void sscope2_state::comm_rombank_w(uint32_t data)
{
	m_comm_bank->set_entry((data >> 24) & 0x7f);
}

uint32_t sscope2_state::comm0_unk_r(offs_t offset, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGCOMM("comm0_unk_r: %08X & %08X\n", offset, mem_mask);
	return 0xffffffff;
}


uint16_t terabrst_state::gun_r(offs_t offset)
{
	uint16_t r = 0;

	// TODO: Replace this with proper emulation of a CCD camera
	// so the GN680's program can handle inputs normally.
	// TODO: Check if this works when skip post is disabled (currently causes game to boot loop)
	if (m_gn680_reg0e == 0 && (offset == 0 || offset == 1))
	{
		// All values are offset so that the range in-game is
		// +/- 280 on the X and +/- 220 on the Y axis.

		// Parts of Player 2's Y axis value is included with every read,
		// so it doesn't have its own index for reading.
		int16_t const p2y = (int16_t)m_analog[3].read_safe(0) - 220;

		r = m_gn680_check;

		switch (m_gn680_latch & 3)
		{
			case 1:
				r |= ((int16_t)m_analog[0].read_safe(0) - 281) & 0x7ff;
				r |= (p2y & 0x700) << 4;
				break;
			case 2:
				r |= ((int16_t)m_analog[1].read_safe(0) - 220) & 0x7ff;
				r |= (p2y & 0xf0) << 7;
				break;
			case 3:
				r |= ((int16_t)m_analog[2].read_safe(0) - 280) & 0x7ff;
				r |= (p2y & 0x0f) << 11;
				break;
		}

		switch (offset)
		{
			case 0:
				r = (r >> 8) & 0xff;
				break;
			case 1:
				r &= 0xff;
				if (!machine().side_effects_disabled())
					m_gn680_check ^= 0x8000; // Must be in sync with the game every read or the update will be rejected
				break;
		}
	}
	else
	{
		if (offset == 0)
			r = m_gn680_ret0;
		else if (offset == 1)
			r = m_gn680_ret1;
	}

	return r;
}

void terabrst_state::gun_w(offs_t offset, uint16_t data)
{
	if (offset == 0)
	{
		m_gn680_latch = data;
		m_gn680->set_input_line(M68K_IRQ_6, HOLD_LINE);
	}
	else if (offset == 0x0e/2)
	{
		// Always set to 0 when reading the gun inputs
		m_gn680_reg0e = data;
	}
}

/******************************************************************/

TIMER_CALLBACK_MEMBER(hornet_state::sound_irq)
{
	m_audiocpu->set_input_line(M68K_IRQ_1, ASSERT_LINE);
}


void hornet_state::soundtimer_en_w(uint16_t data)
{
	if (BIT(data, 0))
	{
		// Reset and disable timer
		m_sound_irq_timer->adjust(attotime::from_usec(m_sound_timer_usec));
		m_sound_irq_timer->enable(false);
	}
	else
	{
		// Enable timer
		m_sound_irq_timer->enable(true);
	}
}

void hornet_state::soundtimer_count_w(uint16_t data)
{
	// Reset the count
	m_sound_irq_timer->adjust(attotime::from_usec(m_sound_timer_usec));
	m_audiocpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
}

/*****************************************************************************/

void hornet_state::hornet_map(address_map &map)
{
	map(0x00000000, 0x003fffff).ram().share(m_workram);
	map(0x74000000, 0x7407ffff).view(m_cg_view);
	m_cg_view[0](0x74000000, 0x740000ff).rw(m_k037122[0], FUNC(k037122_device::reg_r), FUNC(k037122_device::reg_w));
	m_cg_view[0](0x74020000, 0x7403ffff).rw(m_k037122[0], FUNC(k037122_device::sram_r), FUNC(k037122_device::sram_w));
	m_cg_view[0](0x74040000, 0x7407ffff).rw(m_k037122[0], FUNC(k037122_device::char_r), FUNC(k037122_device::char_w));
	map(0x78000000, 0x7800ffff).rw(m_konppc, FUNC(konppc_device::cgboard_dsp_shared_r_ppc), FUNC(konppc_device::cgboard_dsp_shared_w_ppc));
	map(0x780c0000, 0x780c0003).rw(m_konppc, FUNC(konppc_device::cgboard_dsp_comm_r_ppc), FUNC(konppc_device::cgboard_dsp_comm_w_ppc));
	map(0x7d000000, 0x7d00ffff).r(FUNC(hornet_state::sysreg_r));
	map(0x7d010000, 0x7d01ffff).w(FUNC(hornet_state::sysreg_w));
	map(0x7d020000, 0x7d021fff).rw("m48t58", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write));  // M48T58Y RTC/NVRAM
	map(0x7d030000, 0x7d03000f).rw(m_k056800, FUNC(k056800_device::host_r), FUNC(k056800_device::host_w));
	map(0x7e000000, 0x7e7fffff).rom().region("datarom", 0);
	map(0x7f000000, 0x7f3fffff).rom().region("prgrom", 0);
	map(0x7fc00000, 0x7fffffff).rom().region("prgrom", 0);
}

void hornet_state::hornet_lan_map(address_map &map)
{
	hornet_map(map);

	map(0x7d040000, 0x7d04ffff).rw(m_gn676_lan, FUNC(konami_gn676_lan_device::lanc1_r), FUNC(konami_gn676_lan_device::lanc1_w));
	map(0x7d050000, 0x7d05ffff).rw(m_gn676_lan, FUNC(konami_gn676_lan_device::lanc2_r), FUNC(konami_gn676_lan_device::lanc2_w));
}

void terabrst_state::terabrst_map(address_map &map)
{
	hornet_map(map);
	map(0x74080000, 0x7408000f).rw(FUNC(terabrst_state::gun_r), FUNC(terabrst_state::gun_w));
}

void hornet_state::sscope_map(address_map &map)
{
	hornet_map(map);

	m_cg_view[1](0x74000000, 0x740000ff).rw(m_k037122[1], FUNC(k037122_device::reg_r), FUNC(k037122_device::reg_w));
	m_cg_view[1](0x74020000, 0x7403ffff).rw(m_k037122[1], FUNC(k037122_device::sram_r), FUNC(k037122_device::sram_w));
	m_cg_view[1](0x74040000, 0x7407ffff).rw(m_k037122[1], FUNC(k037122_device::char_r), FUNC(k037122_device::char_w));
}

void sscope2_state::sscope2_map(address_map &map)
{
	sscope_map(map);
	map(0x7d040004, 0x7d040007).rw(FUNC(sscope2_state::comm_eeprom_r), FUNC(sscope2_state::comm_eeprom_w));
	map(0x7d042000, 0x7d043fff).ram();                 // COMM BOARD 0
	map(0x7d044000, 0x7d044007).r(FUNC(sscope2_state::comm0_unk_r));
	map(0x7d048000, 0x7d048003).w(FUNC(sscope2_state::comm1_w));
	map(0x7d04a000, 0x7d04a003).w(FUNC(sscope2_state::comm_rombank_w));
	map(0x7d050000, 0x7d05ffff).bankr(m_comm_bank);   // COMM BOARD 1
}

/*****************************************************************************/

void hornet_state::sound_memmap(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();     // Work RAM
	map(0x200000, 0x200fff).rw("rfsnd", FUNC(rf5c400_device::rf5c400_r), FUNC(rf5c400_device::rf5c400_w));      // Ricoh RF5C400
	map(0x300000, 0x30001f).rw(m_k056800, FUNC(k056800_device::sound_r), FUNC(k056800_device::sound_w)).umask16(0x00ff);
	map(0x480000, 0x480001).nopw();
	map(0x4c0000, 0x4c0001).nopw();
	map(0x500000, 0x500001).w(FUNC(hornet_state::soundtimer_en_w)).nopr();
	map(0x600000, 0x600001).w(FUNC(hornet_state::soundtimer_count_w)).nopr();
}

/*****************************************************************************/

void terabrst_state::gn680_sysctrl(uint16_t data)
{
	// bit 15 = watchdog toggle
	// lower 4 bits = LEDs?
}

uint16_t terabrst_state::gn680_latch_r()
{
	if (!machine().side_effects_disabled())
		m_gn680->set_input_line(M68K_IRQ_6, CLEAR_LINE);

	return m_gn680_latch;
}

void terabrst_state::gn680_latch_w(offs_t offset, uint16_t data)
{
	if (offset)
	{
		m_gn680_ret1 = data;
	}
	else
	{
		m_gn680_ret0 = data;
	}
}

// WORD at 30000e: IRQ 4 tests bits 6 and 7, IRQ5 tests bits 4 and 5
// (vsync and hsync status for each of the two screens?)

void terabrst_state::gn680_memmap(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x200000, 0x203fff).ram();
	map(0x300000, 0x300001).w(FUNC(terabrst_state::gn680_sysctrl));
	map(0x314000, 0x317fff).ram();
	map(0x400000, 0x400003).rw(FUNC(terabrst_state::gn680_latch_r), FUNC(terabrst_state::gn680_latch_w));
	map(0x400008, 0x400009).nopw();    // writes 0001 00fe each time IRQ 6 triggers
}

/*****************************************************************************/

template <unsigned Board>
void hornet_state::sharc_map(address_map &map)
{
	map(0x0400000, 0x041ffff).rw(m_konppc, FUNC(konppc_device::cgboard_shared_sharc_r<Board>), FUNC(konppc_device::cgboard_shared_sharc_w<Board>));
	map(0x0500000, 0x05fffff).ram().share(m_sharc_dataram[Board]).lr32(NAME([this](offs_t offset) { return m_sharc_dataram[Board][offset] & 0xffff; }));
	map(0x1400000, 0x14fffff).ram();
	map(0x2400000, 0x27fffff).m(m_voodoo[Board], FUNC(generic_voodoo_device::core_map));
	map(0x3400000, 0x34000ff).rw(m_konppc, FUNC(konppc_device::cgboard_comm_sharc_r<Board>), FUNC(konppc_device::cgboard_comm_sharc_w<Board>));
	map(0x3500000, 0x35000ff).rw(m_konppc, FUNC(konppc_device::cgboard_k033906_r<Board>), FUNC(konppc_device::cgboard_k033906_w<Board>));
	map(0x3600000, 0x37fffff).bankr(m_cgboard_bank[Board]);
}

/*****************************************************************************/

static INPUT_PORTS_START( hornet )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) // greyed out in gradius4 test mode, but does work
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x80, 0x00, "Skip Post" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Disable Machine Init" ) PORT_DIPLOCATION("SW:2") // Having this on disables the analog controls in terabrst, sscope and sscope2 and enables usage with JAMMA inputs
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) ) // in addition, this disables the wheel motor test in thrilldg**
	PORT_DIPNAME( 0x20, 0x20, "DIP3" ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIP4" ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP5" ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIP6" ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP7" ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, "DIP8" ) PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( gradius4 )
	PORT_INCLUDE( hornet )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x80, 0x00, "Skip Post" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPNAME( 0x40, 0x40, "Screen Flip (H)" ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Screen Flip (V)" ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIP4" ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP5" ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Harness" ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING( 0x04, "JVS" )
	PORT_DIPSETTING( 0x00, "JAMMA" )
	PORT_DIPNAME( 0x02, 0x02, "DIP7" ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, "Monitor Type" ) PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING( 0x01, "24KHz" )
	PORT_DIPSETTING( 0x00, "15KHz" )
INPUT_PORTS_END

static INPUT_PORTS_START(nbapbp)
	PORT_INCLUDE(gradius4)

	PORT_MODIFY("DSW")
	PORT_DIPNAME(0x02, 0x02, "Cabinet Type") PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(0x02, "2 Player")
	PORT_DIPSETTING(0x00, "4 Player")
INPUT_PORTS_END

static INPUT_PORTS_START(terabrst)
	PORT_INCLUDE(hornet)

	PORT_MODIFY("IN0")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1) PORT_NAME("P1 Trigger")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1) PORT_NAME("P1 Bomb")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(1) PORT_NAME("P1 Temp Cursor Speedup")

	PORT_MODIFY("IN1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2) PORT_NAME("P2 Trigger")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2) PORT_NAME("P2 Bomb")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(2) PORT_NAME("P2 Temp Cursor Speedup")

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x40, 0x40, "Disable Machine Init" ) PORT_DIPLOCATION("SW:2") // Default DIPSW2 to OFF
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	// Ranges picked to allow the cursor to go just off screen because the acceptable range is too wide
	PORT_START("ANALOG1") // P1 Gun X
	PORT_BIT( 0x7ff, 280, IPT_LIGHTGUN_X ) PORT_MINMAX(0, 560) PORT_SENSITIVITY(35) PORT_KEYDELTA(1) PORT_PLAYER(1)

	PORT_START("ANALOG2") // P1 Gun Y
	PORT_BIT( 0x7ff, 220, IPT_LIGHTGUN_Y ) PORT_MINMAX(0, 440) PORT_SENSITIVITY(35) PORT_KEYDELTA(1) PORT_PLAYER(1)

	PORT_START("ANALOG3") // P2 Gun X
	PORT_BIT( 0x7ff, 280, IPT_LIGHTGUN_X ) PORT_MINMAX(0, 560) PORT_SENSITIVITY(35) PORT_KEYDELTA(1) PORT_PLAYER(2)

	PORT_START("ANALOG4") // P2 Gun Y
	PORT_BIT( 0x7ff, 220, IPT_LIGHTGUN_Y ) PORT_MINMAX(0, 440) PORT_SENSITIVITY(35) PORT_KEYDELTA(1) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( sscope )
	PORT_INCLUDE( hornet )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Gun Trigger")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x40, 0x40, "Disable Machine Init" ) PORT_DIPLOCATION("SW:2") // Default DIPSW2 to OFF
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START("ANALOG1") // Gun Yaw
	PORT_BIT( 0x7ff, 0x400, IPT_AD_STICK_X ) PORT_MINMAX(0x000, 0x7ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(20) PORT_CENTERDELTA(0)

	PORT_START("ANALOG2") // Gun Pitch
	PORT_BIT( 0x7ff, 0x3ff, IPT_AD_STICK_Y ) PORT_MINMAX(0x000, 0x7ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(20) PORT_CENTERDELTA(0) PORT_INVERT
INPUT_PORTS_END

static INPUT_PORTS_START( sscope2 )
	PORT_INCLUDE( sscope )

	// LAN board EEPROM
	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("lan_eeprom", FUNC(eeprom_serial_93cxx_device::di_write))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("lan_eeprom", FUNC(eeprom_serial_93cxx_device::clk_write))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("lan_eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))
INPUT_PORTS_END

static INPUT_PORTS_START( thrilld )
	PORT_INCLUDE( hornet )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Gear Shift Up/1st")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Gear Shift Down/2nd")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Gear Shift Left/3rd")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Gear Shift Right/4th")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("IN2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ANALOG1")
	PORT_BIT(0x7ff, 0x3ff, IPT_PADDLE) PORT_NAME("Steering Wheel") PORT_MINMAX(0x000, 0x7ff) PORT_SENSITIVITY(80) PORT_KEYDELTA(50)

	PORT_START("ANALOG2")
	PORT_BIT(0x7ff, 0x000, IPT_PEDAL) PORT_NAME("Gas Pedal") PORT_MINMAX(0x000, 0x7ff) PORT_SENSITIVITY(80) PORT_KEYDELTA(50)

	PORT_START("ANALOG3")
	PORT_BIT(0x7ff, 0x000, IPT_PEDAL2) PORT_NAME("Brake Pedal") PORT_MINMAX(0x000, 0x7ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(60)
INPUT_PORTS_END

static INPUT_PORTS_START( thrilld_gp )
	PORT_INCLUDE( thrilld )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Gear Shift Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Gear Shift Down")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( thrilld_gk )
	PORT_INCLUDE( thrilld )

	PORT_START("ANALOG4")
	PORT_BIT(0x7ff, 0x000, IPT_PEDAL3) PORT_NAME("Handbrake Lever") PORT_MINMAX(0x000, 0x7ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(60)
INPUT_PORTS_END

static INPUT_PORTS_START( thrilld_gn )
	PORT_INCLUDE( thrilld_gk )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Gear Shift Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Gear Shift Down")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Gear Shift Left")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Gear Shift Right")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( thrilld_gm )
	PORT_INCLUDE( thrilld_gk )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Gear Shift Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Gear Shift Down")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( thrilld_ge )
	PORT_INCLUDE( thrilld_gk )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Gear Shift Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Gear Shift Down")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Gear Shift Left")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Gear Shift Right")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ANALOG5")
	PORT_BIT(0x7ff, 0x000, IPT_PEDAL) PORT_NAME("Clutch Pedal") PORT_MINMAX(0x000, 0x7ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(60) PORT_PLAYER(2)
INPUT_PORTS_END

/* PowerPC interrupts

    IRQ0:   Vblank CG Board 0
    IRQ1:   Vblank CG Board 1
    IRQ2:   LANC (GQ931(H) board), Teraburst (usage unknown)
    DMA0
    NMI:    SCI

*/


void hornet_state::machine_start()
{
	m_pcb_digit.resolve();

	// set conservative DRC options
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);

	// configure fast RAM regions for DRC
	m_maincpu->ppcdrc_add_fastram(0x00000000, 0x003fffff, false, m_workram);

	m_sound_irq_timer = timer_alloc(FUNC(hornet_state::sound_irq), this);

	save_item(NAME(m_sndres));
}

void terabrst_state::machine_start()
{
	hornet_state::machine_start();

	save_item(NAME(m_gn680_latch));
	save_item(NAME(m_gn680_ret0));
	save_item(NAME(m_gn680_ret1));
	save_item(NAME(m_gn680_check));
	save_item(NAME(m_gn680_reg0e));
}

void sscope2_state::machine_start()
{
	hornet_state::machine_start();

	m_comm_bank->configure_entries(0, m_comm_board_rom.bytes() / 0x10000, &m_comm_board_rom[0], 0x10000);
}

void hornet_state::machine_reset()
{
	m_dsp[0]->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	if (m_dsp[1].found())
		m_dsp[1]->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	if (memregion("cgboard_0"))
	{
		m_cgboard_bank[0]->set_entry(0);
		if (m_cgboard_bank[1])
			m_cgboard_bank[1]->set_entry(0);
	}
}

void terabrst_state::machine_reset()
{
	hornet_state::machine_reset();

	m_gn680_check = 0x8000;
}

void sscope2_state::machine_reset()
{
	hornet_state::machine_reset();

	m_comm_bank->set_entry(0);
}

double hornet_state::adc12138_input_callback(uint8_t input)
{
	if (input < m_analog.size())
	{
		int const value = m_analog[input].read_safe(0);
		return (double)(value) / 2047.0;
	}

	return 0.0;
}

void hornet_state::hornet(machine_config &config)
{
	// basic machine hardware
	PPC403GA(config, m_maincpu, XTAL(64'000'000) / 2);   // PowerPC 403GA 32MHz
	// The default serial clock used by the ppc4xx code results in JVS comm at 57600 baud,
	// so set serial clock to 7.3728MHz (xtal on PCB) to allow for 115200 baud.
	// With the slower clock rate the in and out rx ptr slowly desyncs (does not read
	// last byte sometimes) on frequent large responses and eventually fatal errors with
	// the message "ppc4xx_spu_rx_data: buffer overrun!".
	m_maincpu->set_serial_clock(XTAL(7'372'800));
	m_maincpu->set_addrmap(AS_PROGRAM, &hornet_state::hornet_map);

	M68000(config, m_audiocpu, XTAL(64'000'000) / 4);    // 16MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &hornet_state::sound_memmap);

	ADSP21062(config, m_dsp[0], XTAL(36'000'000));
	m_dsp[0]->set_boot_mode(adsp21062_device::BOOT_MODE_EPROM);
	m_dsp[0]->set_addrmap(AS_DATA, &hornet_state::sharc_map<0>);

	config.set_maximum_quantum(attotime::from_hz(6000));

	WATCHDOG_TIMER(config, m_watchdog);

//  PCB description at top doesn't mention any EEPROM on the base board...
//  EEPROM_93C46_16BIT(config, "eeprom");

	VOODOO_1(config, m_voodoo[0], XTAL(50'000'000));
	m_voodoo[0]->set_fbmem(2);
	m_voodoo[0]->set_tmumem(4,0);
	m_voodoo[0]->set_status_cycles(1000); // optimization to consume extra cycles when polling status
	m_voodoo[0]->set_screen("screen");
	m_voodoo[0]->set_cpu(m_dsp[0]);
	m_voodoo[0]->vblank_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_voodoo[0]->stall_callback().set(m_dsp[0], FUNC(adsp21062_device::write_stall));

	K033906(config, m_k033906[0], 0, m_voodoo[0]);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	// default 24KHz parameter in both 037122 and voodoo, input clock correct? (58~Hz Vsync, 50MHz/3 or 64MHz/4?)
	screen.set_raw(XTAL(64'000'000) / 4, 644, 41, 41 + 512, 428, 27, 27 + 384);
	screen.set_screen_update(FUNC(hornet_state::screen_update<0>));

	K037122(config, m_k037122[0], 0);
	m_k037122[0]->set_screen("screen");

	K056800(config, m_k056800, XTAL(16'934'400));
	m_k056800->int_callback().set_inputline(m_audiocpu, M68K_IRQ_2);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	RF5C400(config, "rfsnd", XTAL(16'934'400))  // value from Guru readme, gives 44100 Hz sample rate
		.add_route(0, "lspeaker", 1.0)
		.add_route(1, "rspeaker", 1.0);

	M48T58(config, "m48t58", 0);

	ADC12138(config, m_adc12138, 0);
	m_adc12138->set_ipt_convert_callback(FUNC(hornet_state::adc12138_input_callback));

	KONPPC(config, m_konppc, 0);
	m_konppc->set_dsp_tag(0, m_dsp[0]);
	m_konppc->set_k033906_tag(0, m_k033906[0]);
	m_konppc->set_voodoo_tag(0, m_voodoo[0]);
	m_konppc->set_texture_bank_tag(0, m_cgboard_bank[0]);
	m_konppc->set_num_boards(1);
	m_konppc->set_cgboard_type(konppc_device::CGBOARD_TYPE_HORNET);

	KONPPC_JVS_HOST(config, m_jvs_host, 0);
	m_jvs_host->output_callback().set([this](uint8_t c) { m_maincpu->ppc4xx_spu_receive_byte(c); });
}

void hornet_state::hornet_x76(machine_config &config)
{
	hornet(config);
	X76F041(config, m_x76f041);
}

void hornet_state::hornet_lan(machine_config &config)
{
	hornet(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &hornet_state::hornet_lan_map);

	KONAMI_GN676A_LAN(config, m_gn676_lan, 0);
}

void hornet_state::nbapbp(machine_config &config)
{
	hornet_x76(config);

	// The official recommended settings in the manual for 4 player mode is cabinet type 4 player + harness JAMMA
	// with the 2L6B panel dipswitch settings on the Windy2 board.
	// NOTE: Harness JVS + cabinet 4 player will work with a second JVS I/O device hooked up, but then
	// the official recommended setting of cabinet type 4 player + harness JAMMA breaks.
	KONAMI_WINDY2_JVS_IO_2L6B_PANEL(config, "windy2_jvsio", 0, m_jvs_host);
}

void terabrst_state::terabrst(machine_config &config) //todo: add K056800 from I/O board
{
	hornet(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &terabrst_state::terabrst_map);

	M68000(config, m_gn680, XTAL(32'000'000) / 2);   // 16MHz
	m_gn680->set_addrmap(AS_PROGRAM, &terabrst_state::gn680_memmap);
}

void hornet_state::sscope(machine_config &config)
{
	hornet(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &hornet_state::sscope_map);

	ADSP21062(config, m_dsp[1], XTAL(36'000'000));
	m_dsp[1]->set_boot_mode(adsp21062_device::BOOT_MODE_EPROM);
	m_dsp[1]->set_addrmap(AS_DATA, &hornet_state::sharc_map<1>);

	m_k037122[0]->set_screen("lscreen");

	K037122(config, m_k037122[1], 0); // unknown input clock
	m_k037122[1]->set_screen("rscreen");

	m_voodoo[0]->set_screen("lscreen");

	VOODOO_1(config, m_voodoo[1], XTAL(50'000'000));
	m_voodoo[1]->set_fbmem(2);
	m_voodoo[1]->set_tmumem(4, 0);
	m_voodoo[1]->set_status_cycles(1000); // optimization to consume extra cycles when polling status
	m_voodoo[1]->set_screen("rscreen");
	m_voodoo[1]->set_cpu(m_dsp[1]);
	m_voodoo[1]->vblank_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ1);
	m_voodoo[1]->stall_callback().set(m_dsp[1], FUNC(adsp21062_device::write_stall));

	K033906(config, m_k033906[1], 0, m_voodoo[1]);

	// video hardware
	config.device_remove("screen");

	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	// default 24KHz parameter in both 037122 and voodoo, input clock correct? (58~Hz Vsync, 50MHz/3 or 64MHz/4?)
	lscreen.set_raw(XTAL(64'000'000) / 4, 644, 41, 41 + 512, 428, 27, 27 + 384);
	lscreen.set_screen_update(FUNC(hornet_state::screen_update<0>));

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER)); // for scope
	// scope screen is 15khz, verified default parameter in both 037122 and voodoo, input clock correct? (60~Hz Vsync, 50MHz/3 or 64MHz/4?)
	rscreen.set_raw(XTAL(64'000'000) / 4, 1017, 106, 106 + 768, 262, 17, 17 + 236);
	rscreen.set_screen_update(FUNC(hornet_state::screen_update<1>));

	// Comes from the GQ830-PWB(J) board
	ADC12138(config, m_adc12138_sscope, 0);
	m_adc12138_sscope->set_ipt_convert_callback(FUNC(hornet_state::adc12138_input_callback));

	m_konppc->set_dsp_tag(1, m_dsp[1]);
	m_konppc->set_k033906_tag(1, m_k033906[1]);
	m_konppc->set_voodoo_tag(1, m_voodoo[1]);
	m_konppc->set_texture_bank_tag(1, m_cgboard_bank[1]);
	m_konppc->set_num_boards(2);
}

void hornet_state::sscope_voodoo2(machine_config& config)
{
	sscope(config);

	VOODOO_2(config.replace(), m_voodoo[0], voodoo_2_device::NOMINAL_CLOCK);
	m_voodoo[0]->set_fbmem(2);
	m_voodoo[0]->set_tmumem(4,0);
	m_voodoo[0]->set_status_cycles(1000); // optimization to consume extra cycles when polling status
	m_voodoo[0]->set_screen("lscreen");
	m_voodoo[0]->set_cpu(m_dsp[0]);
	m_voodoo[0]->vblank_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_voodoo[0]->stall_callback().set(m_dsp[0], FUNC(adsp21062_device::write_stall));

	VOODOO_2(config.replace(), m_voodoo[1], voodoo_2_device::NOMINAL_CLOCK);
	m_voodoo[1]->set_fbmem(2);
	m_voodoo[1]->set_tmumem(4,0);
	m_voodoo[1]->set_status_cycles(1000); // optimization to consume extra cycles when polling status
	m_voodoo[1]->set_screen("rscreen");
	m_voodoo[1]->set_cpu(m_dsp[1]);
	m_voodoo[1]->vblank_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ1);
	m_voodoo[1]->stall_callback().set(m_dsp[1], FUNC(adsp21062_device::write_stall));

	m_k033906[0]->set_pciid(0x0002121a); // PCI Vendor ID (0x121a = 3dfx), Device ID (0x0002 = Voodoo 2)
	m_k033906[1]->set_pciid(0x0002121a); // PCI Vendor ID (0x121a = 3dfx), Device ID (0x0002 = Voodoo 2)
}

void sscope2_state::sscope2(machine_config &config)
{
	sscope_voodoo2(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sscope2_state::sscope2_map);

	DS2401(config, m_lan_ds2401);
	EEPROM_93C46_16BIT(config, m_lan_eeprom);
}

void sscope2_state::sscope2_voodoo1(machine_config& config)
{
	sscope(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sscope2_state::sscope2_map);

	DS2401(config, m_lan_ds2401);
	EEPROM_93C46_16BIT(config, m_lan_eeprom);
}

/*****************************************************************************/

void hornet_state::jamma_jvs_w(uint8_t data)
{
	bool const accepted = m_jvs_host->write(data);
	if (accepted)
		m_jvs_host->read();
}

/*****************************************************************************/

void hornet_state::init_hornet()
{
	m_cgboard_bank[0]->configure_entries(0, 2, memregion("cgboard_0")->base(), 0x800000);

	m_maincpu->ppc4xx_spu_set_tx_handler(write8smo_delegate(*this, FUNC(hornet_state::jamma_jvs_w)));

	//m_dsp[0]->enable_recompiler();
}

void hornet_state::init_gradius4()
{
	init_hornet();

	m_dsp[0]->enable_recompiler();
}

void hornet_state::init_sscope()
{
	m_cgboard_bank[0]->configure_entries(0, 2, memregion("cgboard_0")->base(), 0x800000);
	m_cgboard_bank[1]->configure_entries(0, 2, memregion("cgboard_0")->base(), 0x800000);

	m_maincpu->ppc4xx_spu_set_tx_handler(write8smo_delegate(*this, FUNC(hornet_state::jamma_jvs_w)));
}

void sscope2_state::init_sscope2() //fixme: eventually set sscope2 to load gfx roms from the comm board
{
	m_cgboard_bank[0]->configure_entries(0, 2, memregion("cgboard_0")->base(), 0x800000);
	m_cgboard_bank[1]->configure_entries(0, 2, memregion("cgboard_0")->base(), 0x800000);

	m_maincpu->ppc4xx_spu_set_tx_handler(write8smo_delegate(*this, FUNC(sscope2_state::jamma_jvs_w)));
}

/*****************************************************************************/

ROM_START(sscope)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("830d01.27p", 0x200000, 0x200000, CRC(de9b3dfa) SHA1(660652a5f745cb04687481c3626d8a43cd169193) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", ROMREGION_ERASE00)   // Data roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)       // CG Board texture roms
	ROM_LOAD32_WORD( "830a14.32u", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.24u", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ua", 0x000000, 0x002000, BAD_DUMP CRC(458900fb) SHA1(ae2f5477e3999ecce5199fc4a53c5ddf78c4406d) ) // hand built
ROM_END

ROM_START(sscopee)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("830d01.27p", 0x200000, 0x200000, CRC(de9b3dfa) SHA1(660652a5f745cb04687481c3626d8a43cd169193) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", ROMREGION_ERASE00)   // Data roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)       // CG Board texture roms
	ROM_LOAD32_WORD( "830a14.32u", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.24u", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ea", 0x000000, 0x002000, BAD_DUMP CRC(7d94272c) SHA1(ef0b3e5d4fcf3cec71caa8e48776f71f850f3b09) ) // hand built
ROM_END

ROM_START(sscopea)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("830d01.27p", 0x200000, 0x200000, CRC(de9b3dfa) SHA1(660652a5f745cb04687481c3626d8a43cd169193) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", ROMREGION_ERASE00)   // Data roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)       // CG Board texture roms
	ROM_LOAD32_WORD( "830a14.32u", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.24u", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_aa", 0x000000, 0x002000, BAD_DUMP CRC(e8f7ac69) SHA1(93df4d8cc6ae376460873e4f3a95dc3921e5690e) ) // hand built
ROM_END

ROM_START(sscopeuc)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("830c01.27p", 0x200000, 0x200000, CRC(87682449) SHA1(6ccaa5bac86e947e01a6aae568a75f002421fe5b) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", ROMREGION_ERASE00)   // Data roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)       // CG Board texture roms
	ROM_LOAD32_WORD( "830a14.32u", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.24u", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ua", 0x000000, 0x002000, BAD_DUMP CRC(458900fb) SHA1(ae2f5477e3999ecce5199fc4a53c5ddf78c4406d) ) // hand built
ROM_END

ROM_START(sscopeec)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("830c01.27p", 0x200000, 0x200000, CRC(87682449) SHA1(6ccaa5bac86e947e01a6aae568a75f002421fe5b) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", ROMREGION_ERASE00)   // Data roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)       // CG Board texture roms
	ROM_LOAD32_WORD( "830a14.32u", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.24u", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ea", 0x000000, 0x002000, BAD_DUMP CRC(7d94272c) SHA1(ef0b3e5d4fcf3cec71caa8e48776f71f850f3b09) ) // hand built
ROM_END

ROM_START(sscopeac)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("830c01.27p", 0x200000, 0x200000, CRC(87682449) SHA1(6ccaa5bac86e947e01a6aae568a75f002421fe5b) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", ROMREGION_ERASE00)   // Data roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)       // CG Board texture roms
	ROM_LOAD32_WORD( "830a14.32u", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.24u", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_aa", 0x000000, 0x002000, BAD_DUMP CRC(e8f7ac69) SHA1(93df4d8cc6ae376460873e4f3a95dc3921e5690e) ) // hand built
ROM_END

ROM_START(sscopeub)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("830b01.27p", 0x200000, 0x200000, CRC(3b6bb075) SHA1(babc134c3a20c7cdcaa735d5f1fd5cab38667a14) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", ROMREGION_ERASE00)   // Data roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)       // CG Board texture roms
	ROM_LOAD32_WORD( "830a14.32u", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.24u", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ua", 0x000000, 0x002000, BAD_DUMP CRC(458900fb) SHA1(ae2f5477e3999ecce5199fc4a53c5ddf78c4406d) ) // hand built
ROM_END

ROM_START(sscopeeb)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("830b01.27p", 0x200000, 0x200000, CRC(3b6bb075) SHA1(babc134c3a20c7cdcaa735d5f1fd5cab38667a14) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", ROMREGION_ERASE00)   // Data roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)       // CG Board texture roms
	ROM_LOAD32_WORD( "830a14.32u", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.24u", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ea", 0x000000, 0x002000, BAD_DUMP CRC(7d94272c) SHA1(ef0b3e5d4fcf3cec71caa8e48776f71f850f3b09) ) // hand built
ROM_END

ROM_START(sscopejb)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("830b01.27p", 0x200000, 0x200000, CRC(3b6bb075) SHA1(babc134c3a20c7cdcaa735d5f1fd5cab38667a14) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", ROMREGION_ERASE00)   // Data roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)       // CG Board texture roms
	ROM_LOAD32_WORD( "830a14.32u", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.24u", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ja", 0x000000, 0x002000, BAD_DUMP CRC(325465c5) SHA1(24524a8eed8f0aa45881bddf65a8fa8ba5270eb1) ) // hand built
ROM_END

ROM_START(sscopeab)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("830b01.27p", 0x200000, 0x200000, CRC(3b6bb075) SHA1(babc134c3a20c7cdcaa735d5f1fd5cab38667a14) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", ROMREGION_ERASE00)   // Data roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)       // CG Board texture roms
	ROM_LOAD32_WORD( "830a14.32u", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.24u", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_aa", 0x000000, 0x002000, BAD_DUMP CRC(e8f7ac69) SHA1(93df4d8cc6ae376460873e4f3a95dc3921e5690e) ) // hand built
ROM_END

ROM_START(sscopeua)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("830a01.27p", 0x200000, 0x200000, CRC(39e353f1) SHA1(569b06969ae7a690f6d6e63cc3b5336061663a37) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", ROMREGION_ERASE00)   // Data roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)       // CG Board texture roms
	ROM_LOAD32_WORD( "830a14.32u", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.24u", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ua", 0x000000, 0x002000, BAD_DUMP CRC(458900fb) SHA1(ae2f5477e3999ecce5199fc4a53c5ddf78c4406d) ) // hand built
ROM_END

ROM_START(sscopeea)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("830a01.27p", 0x200000, 0x200000, CRC(39e353f1) SHA1(569b06969ae7a690f6d6e63cc3b5336061663a37) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", ROMREGION_ERASE00)   // Data roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)       // CG Board texture roms
	ROM_LOAD32_WORD( "830a14.32u", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.24u", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ea", 0x000000, 0x002000, BAD_DUMP CRC(7d94272c) SHA1(ef0b3e5d4fcf3cec71caa8e48776f71f850f3b09) ) // hand built
ROM_END

ROM_START(sscopeja)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("830a01.27p", 0x200000, 0x200000, CRC(39e353f1) SHA1(569b06969ae7a690f6d6e63cc3b5336061663a37) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", ROMREGION_ERASE00)   // Data roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)       // CG Board texture roms
	ROM_LOAD32_WORD( "830a14.32u", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.24u", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ja", 0x000000, 0x002000, BAD_DUMP CRC(325465c5) SHA1(24524a8eed8f0aa45881bddf65a8fa8ba5270eb1) ) // hand built
ROM_END

ROM_START(sscopeaa)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("830a01.27p", 0x200000, 0x200000, CRC(39e353f1) SHA1(569b06969ae7a690f6d6e63cc3b5336061663a37) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", ROMREGION_ERASE00)   // Data roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)       // CG Board texture roms
	ROM_LOAD32_WORD( "830a14.32u", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.24u", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_aa", 0x000000, 0x002000, BAD_DUMP CRC(e8f7ac69) SHA1(93df4d8cc6ae376460873e4f3a95dc3921e5690e) ) // hand built
ROM_END

ROM_START(sscopevd2)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("830d01.27p", 0x200000, 0x200000, CRC(de9b3dfa) SHA1(660652a5f745cb04687481c3626d8a43cd169193) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", ROMREGION_ERASE00)   // Data roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)       // CG Board texture roms
	ROM_LOAD32_WORD( "830a14.32u", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.24u", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ua", 0x000000, 0x002000, BAD_DUMP CRC(458900fb) SHA1(ae2f5477e3999ecce5199fc4a53c5ddf78c4406d) ) // hand built
ROM_END

ROM_START(sscopeevd2)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("830d01.27p", 0x200000, 0x200000, CRC(de9b3dfa) SHA1(660652a5f745cb04687481c3626d8a43cd169193) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", ROMREGION_ERASE00)   // Data roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)       // CG Board texture roms
	ROM_LOAD32_WORD( "830a14.32u", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.24u", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ea", 0x000000, 0x002000, BAD_DUMP CRC(7d94272c) SHA1(ef0b3e5d4fcf3cec71caa8e48776f71f850f3b09) ) // hand built
ROM_END

ROM_START(sscopeavd2)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("830d01.27p", 0x200000, 0x200000, CRC(de9b3dfa) SHA1(660652a5f745cb04687481c3626d8a43cd169193) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", ROMREGION_ERASE00)   // Data roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)       // CG Board texture roms
	ROM_LOAD32_WORD( "830a14.32u", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.24u", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_aa", 0x000000, 0x002000, BAD_DUMP CRC(e8f7ac69) SHA1(93df4d8cc6ae376460873e4f3a95dc3921e5690e) ) // hand built
ROM_END

ROM_START(sscopeucvd2)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("830c01.27p", 0x200000, 0x200000, CRC(87682449) SHA1(6ccaa5bac86e947e01a6aae568a75f002421fe5b) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", ROMREGION_ERASE00)   // Data roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)       // CG Board texture roms
	ROM_LOAD32_WORD( "830a14.32u", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.24u", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ua", 0x000000, 0x002000, BAD_DUMP CRC(458900fb) SHA1(ae2f5477e3999ecce5199fc4a53c5ddf78c4406d) ) // hand built
ROM_END

ROM_START(sscopeecvd2)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("830c01.27p", 0x200000, 0x200000, CRC(87682449) SHA1(6ccaa5bac86e947e01a6aae568a75f002421fe5b) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", ROMREGION_ERASE00)   // Data roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)       // CG Board texture roms
	ROM_LOAD32_WORD( "830a14.32u", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.24u", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ea", 0x000000, 0x002000, BAD_DUMP CRC(7d94272c) SHA1(ef0b3e5d4fcf3cec71caa8e48776f71f850f3b09) ) // hand built
ROM_END

ROM_START(sscopeacvd2)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("830c01.27p", 0x200000, 0x200000, CRC(87682449) SHA1(6ccaa5bac86e947e01a6aae568a75f002421fe5b) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", ROMREGION_ERASE00)   // Data roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)       // CG Board texture roms
	ROM_LOAD32_WORD( "830a14.32u", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.24u", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_aa", 0x000000, 0x002000, BAD_DUMP CRC(e8f7ac69) SHA1(93df4d8cc6ae376460873e4f3a95dc3921e5690e) ) // hand built
ROM_END

ROM_START(sscopeubvd2)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("830b01.27p", 0x200000, 0x200000, CRC(3b6bb075) SHA1(babc134c3a20c7cdcaa735d5f1fd5cab38667a14) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", ROMREGION_ERASE00)   // Data roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)       // CG Board texture roms
	ROM_LOAD32_WORD( "830a14.32u", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.24u", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ua", 0x000000, 0x002000, BAD_DUMP CRC(458900fb) SHA1(ae2f5477e3999ecce5199fc4a53c5ddf78c4406d) ) // hand built
ROM_END

ROM_START(sscope2)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931d01.27p", 0x200000, 0x200000, CRC(4065fde6) SHA1(84f2dedc3e8f61651b22c0a21433a64993e1b9e2) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04.16t", 0x000000, 0x200000, CRC(4f5917e6) SHA1(a63a107f1d6d9756e4ab0965d72ea446f0692814) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, CRC(f7c40218) SHA1(5021089803024a6f552e5c9d42b905e804b9d904) )

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401_gk830.16g", 0x000000, 0x000008, BAD_DUMP CRC(bae36d0b) SHA1(4dd5915888d5718356b40bbe897f2470e410176a) ) // hand built

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD( "at93c46.8g", 0x000000, 0x000080, BAD_DUMP CRC(cc63c213) SHA1(fb20e56fb73a887dc7b6db49efd1f8a18b959152) ) // hand built
ROM_END

ROM_START(sscope2e)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931d01.27p", 0x200000, 0x200000, CRC(4065fde6) SHA1(84f2dedc3e8f61651b22c0a21433a64993e1b9e2) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04.16t", 0x000000, 0x200000, CRC(4f5917e6) SHA1(a63a107f1d6d9756e4ab0965d72ea446f0692814) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ea", 0x000000, 0x002000, BAD_DUMP CRC(832f9148) SHA1(42a8cc9436eaa79b5bab242692e18c3807f6af74) ) // hand built

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401.16g", 0x000000, 0x000008, CRC(908da6dd) SHA1(f7c1a2ebe05f4bc403a6154d724f8f6f6eeeff15) )

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD16_WORD_SWAP( "at93c46_ea.8g", 0x000000, 0x000080, BAD_DUMP CRC(b6da86a4) SHA1(3a6570ac25748fb5e6b8a0dd6b832ee2d463cc7b) ) // hand built
ROM_END

ROM_START(sscope2j)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931d01.27p", 0x200000, 0x200000, CRC(4065fde6) SHA1(84f2dedc3e8f61651b22c0a21433a64993e1b9e2) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04.16t", 0x000000, 0x200000, CRC(4f5917e6) SHA1(a63a107f1d6d9756e4ab0965d72ea446f0692814) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ja", 0x000000, 0x002000, BAD_DUMP CRC(d16ac629) SHA1(92c65a67ef201912e4f81d896126b045c5cc2072) ) // hand built

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401.16g", 0x000000, 0x000008, CRC(908da6dd) SHA1(f7c1a2ebe05f4bc403a6154d724f8f6f6eeeff15) )

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD16_WORD_SWAP( "at93c46_ja.8g", 0x000000, 0x000080, BAD_DUMP CRC(6613c091) SHA1(101a15afc27d5b4b5e846dc6823c14656132b26b) ) // hand built
ROM_END

ROM_START(sscope2a)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931d01.27p", 0x200000, 0x200000, CRC(4065fde6) SHA1(84f2dedc3e8f61651b22c0a21433a64993e1b9e2) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04.16t", 0x000000, 0x200000, CRC(4f5917e6) SHA1(a63a107f1d6d9756e4ab0965d72ea446f0692814) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_aa", 0x000000, 0x002000, BAD_DUMP CRC(164c1a0d) SHA1(9f7e6cc1acae114aa97d9ed435661bf9c8b845c5) ) // hand built

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401.16g", 0x000000, 0x000008, CRC(908da6dd) SHA1(f7c1a2ebe05f4bc403a6154d724f8f6f6eeeff15) )

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD16_WORD_SWAP( "at93c46_aa.8g", 0x000000, 0x000080, BAD_DUMP CRC(026b0ea5) SHA1(5ab63b88caeb9dc53732b1a432f884d85bcc222c) ) // hand built
ROM_END

ROM_START(sscope2uc)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931c01.27p", 0x200000, 0x200000, CRC(653ba4d9) SHA1(29c1c1d5088e6ba7fa5cfa63b5975f47b54602ee) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04_c.16t", 0x000000, 0x200000, CRC(a05446e3) SHA1(67aef3cfe217223aea53dbc5cccd8d706eae8864) ) // Only 2 bytes are different from the other 931a04.16t (CRC 4f5917e6) and both are only off by 1 bit. Bad dump?

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, CRC(f7c40218) SHA1(5021089803024a6f552e5c9d42b905e804b9d904) )

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401_gk830.16g", 0x000000, 0x000008, BAD_DUMP CRC(bae36d0b) SHA1(4dd5915888d5718356b40bbe897f2470e410176a) ) // hand built

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD( "at93c46.8g", 0x000000, 0x000080, BAD_DUMP CRC(cc63c213) SHA1(fb20e56fb73a887dc7b6db49efd1f8a18b959152) ) // hand built
ROM_END

ROM_START(sscope2ec)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931c01.27p", 0x200000, 0x200000, CRC(653ba4d9) SHA1(29c1c1d5088e6ba7fa5cfa63b5975f47b54602ee) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04_c.16t", 0x000000, 0x200000, CRC(a05446e3) SHA1(67aef3cfe217223aea53dbc5cccd8d706eae8864) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ea", 0x000000, 0x002000, BAD_DUMP CRC(832f9148) SHA1(42a8cc9436eaa79b5bab242692e18c3807f6af74) ) // hand built

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401.16g", 0x000000, 0x000008, CRC(908da6dd) SHA1(f7c1a2ebe05f4bc403a6154d724f8f6f6eeeff15) )

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD16_WORD_SWAP( "at93c46_ea.8g", 0x000000, 0x000080, BAD_DUMP CRC(b6da86a4) SHA1(3a6570ac25748fb5e6b8a0dd6b832ee2d463cc7b) ) // hand built
ROM_END

ROM_START(sscope2jc)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931c01.27p", 0x200000, 0x200000, CRC(653ba4d9) SHA1(29c1c1d5088e6ba7fa5cfa63b5975f47b54602ee) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04_c.16t", 0x000000, 0x200000, CRC(a05446e3) SHA1(67aef3cfe217223aea53dbc5cccd8d706eae8864) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ja", 0x000000, 0x002000, BAD_DUMP CRC(d16ac629) SHA1(92c65a67ef201912e4f81d896126b045c5cc2072) ) // hand built

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401.16g", 0x000000, 0x000008, CRC(908da6dd) SHA1(f7c1a2ebe05f4bc403a6154d724f8f6f6eeeff15) )

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD16_WORD_SWAP( "at93c46_ja.8g", 0x000000, 0x000080, BAD_DUMP CRC(6613c091) SHA1(101a15afc27d5b4b5e846dc6823c14656132b26b) ) // hand built
ROM_END

ROM_START(sscope2ac)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931c01.27p", 0x200000, 0x200000, CRC(653ba4d9) SHA1(29c1c1d5088e6ba7fa5cfa63b5975f47b54602ee) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04_c.16t", 0x000000, 0x200000, CRC(a05446e3) SHA1(67aef3cfe217223aea53dbc5cccd8d706eae8864) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_aa", 0x000000, 0x002000, BAD_DUMP CRC(164c1a0d) SHA1(9f7e6cc1acae114aa97d9ed435661bf9c8b845c5) ) // hand built

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401.16g", 0x000000, 0x000008, CRC(908da6dd) SHA1(f7c1a2ebe05f4bc403a6154d724f8f6f6eeeff15) )

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD16_WORD_SWAP( "at93c46_aa.8g", 0x000000, 0x000080, BAD_DUMP CRC(026b0ea5) SHA1(5ab63b88caeb9dc53732b1a432f884d85bcc222c) ) // hand built
ROM_END

ROM_START(sscope2ub)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931b01.27p", 0x200000, 0x200000, CRC(deb036b7) SHA1(12280aa4e37c3492e5192d630c26e758d08744dd) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04.16t", 0x000000, 0x200000, CRC(4f5917e6) SHA1(a63a107f1d6d9756e4ab0965d72ea446f0692814) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, CRC(f7c40218) SHA1(5021089803024a6f552e5c9d42b905e804b9d904) )

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401_gk830.16g", 0x000000, 0x000008, BAD_DUMP CRC(bae36d0b) SHA1(4dd5915888d5718356b40bbe897f2470e410176a) ) // hand built

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD( "at93c46.8g", 0x000000, 0x000080, BAD_DUMP CRC(cc63c213) SHA1(fb20e56fb73a887dc7b6db49efd1f8a18b959152) ) // hand built
ROM_END

ROM_START(sscope2eb)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931b01.27p", 0x200000, 0x200000, CRC(deb036b7) SHA1(12280aa4e37c3492e5192d630c26e758d08744dd) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04.16t", 0x000000, 0x200000, CRC(4f5917e6) SHA1(a63a107f1d6d9756e4ab0965d72ea446f0692814) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ea", 0x000000, 0x002000, BAD_DUMP CRC(832f9148) SHA1(42a8cc9436eaa79b5bab242692e18c3807f6af74) ) // hand built

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401.16g", 0x000000, 0x000008, CRC(908da6dd) SHA1(f7c1a2ebe05f4bc403a6154d724f8f6f6eeeff15) )

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD16_WORD_SWAP( "at93c46_ea.8g", 0x000000, 0x000080, BAD_DUMP CRC(b6da86a4) SHA1(3a6570ac25748fb5e6b8a0dd6b832ee2d463cc7b) ) // hand built
ROM_END

ROM_START(sscope2jb)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931b01.27p", 0x200000, 0x200000, CRC(deb036b7) SHA1(12280aa4e37c3492e5192d630c26e758d08744dd) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04.16t", 0x000000, 0x200000, CRC(4f5917e6) SHA1(a63a107f1d6d9756e4ab0965d72ea446f0692814) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ja", 0x000000, 0x002000, BAD_DUMP CRC(d16ac629) SHA1(92c65a67ef201912e4f81d896126b045c5cc2072) ) // hand built

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401.16g", 0x000000, 0x000008, CRC(908da6dd) SHA1(f7c1a2ebe05f4bc403a6154d724f8f6f6eeeff15) )

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD16_WORD_SWAP( "at93c46_ja.8g", 0x000000, 0x000080, BAD_DUMP CRC(6613c091) SHA1(101a15afc27d5b4b5e846dc6823c14656132b26b) ) // hand built
ROM_END

ROM_START(sscope2ab)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931b01.27p", 0x200000, 0x200000, CRC(deb036b7) SHA1(12280aa4e37c3492e5192d630c26e758d08744dd) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04.16t", 0x000000, 0x200000, CRC(4f5917e6) SHA1(a63a107f1d6d9756e4ab0965d72ea446f0692814) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_aa", 0x000000, 0x002000, BAD_DUMP CRC(164c1a0d) SHA1(9f7e6cc1acae114aa97d9ed435661bf9c8b845c5) ) // hand built

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401.16g", 0x000000, 0x000008, CRC(908da6dd) SHA1(f7c1a2ebe05f4bc403a6154d724f8f6f6eeeff15) )

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD16_WORD_SWAP( "at93c46_aa.8g", 0x000000, 0x000080, BAD_DUMP CRC(026b0ea5) SHA1(5ab63b88caeb9dc53732b1a432f884d85bcc222c) ) // hand built
ROM_END

ROM_START(sscope2vd1)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931d01.27p", 0x200000, 0x200000, CRC(4065fde6) SHA1(84f2dedc3e8f61651b22c0a21433a64993e1b9e2) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04.16t", 0x000000, 0x200000, CRC(4f5917e6) SHA1(a63a107f1d6d9756e4ab0965d72ea446f0692814) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, CRC(f7c40218) SHA1(5021089803024a6f552e5c9d42b905e804b9d904) )

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401_gk830.16g", 0x000000, 0x000008, BAD_DUMP CRC(bae36d0b) SHA1(4dd5915888d5718356b40bbe897f2470e410176a) ) // hand built

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD( "at93c46.8g", 0x000000, 0x000080, BAD_DUMP CRC(cc63c213) SHA1(fb20e56fb73a887dc7b6db49efd1f8a18b959152) ) // hand built
ROM_END

ROM_START(sscope2evd1)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931d01.27p", 0x200000, 0x200000, CRC(4065fde6) SHA1(84f2dedc3e8f61651b22c0a21433a64993e1b9e2) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04.16t", 0x000000, 0x200000, CRC(4f5917e6) SHA1(a63a107f1d6d9756e4ab0965d72ea446f0692814) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ea", 0x000000, 0x002000, BAD_DUMP CRC(832f9148) SHA1(42a8cc9436eaa79b5bab242692e18c3807f6af74) ) // hand built

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401.16g", 0x000000, 0x000008, CRC(908da6dd) SHA1(f7c1a2ebe05f4bc403a6154d724f8f6f6eeeff15) )

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD16_WORD_SWAP( "at93c46_ea.8g", 0x000000, 0x000080, BAD_DUMP CRC(b6da86a4) SHA1(3a6570ac25748fb5e6b8a0dd6b832ee2d463cc7b) ) // hand built
ROM_END

ROM_START(sscope2jvd1)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931d01.27p", 0x200000, 0x200000, CRC(4065fde6) SHA1(84f2dedc3e8f61651b22c0a21433a64993e1b9e2) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04.16t", 0x000000, 0x200000, CRC(4f5917e6) SHA1(a63a107f1d6d9756e4ab0965d72ea446f0692814) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ja", 0x000000, 0x002000, BAD_DUMP CRC(d16ac629) SHA1(92c65a67ef201912e4f81d896126b045c5cc2072) ) // hand built

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401.16g", 0x000000, 0x000008, CRC(908da6dd) SHA1(f7c1a2ebe05f4bc403a6154d724f8f6f6eeeff15) )

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD16_WORD_SWAP( "at93c46_ja.8g", 0x000000, 0x000080, BAD_DUMP CRC(6613c091) SHA1(101a15afc27d5b4b5e846dc6823c14656132b26b) ) // hand built
ROM_END

ROM_START(sscope2avd1)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931d01.27p", 0x200000, 0x200000, CRC(4065fde6) SHA1(84f2dedc3e8f61651b22c0a21433a64993e1b9e2) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04.16t", 0x000000, 0x200000, CRC(4f5917e6) SHA1(a63a107f1d6d9756e4ab0965d72ea446f0692814) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_aa", 0x000000, 0x002000, BAD_DUMP CRC(164c1a0d) SHA1(9f7e6cc1acae114aa97d9ed435661bf9c8b845c5) ) // hand built

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401.16g", 0x000000, 0x000008, CRC(908da6dd) SHA1(f7c1a2ebe05f4bc403a6154d724f8f6f6eeeff15) )

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD16_WORD_SWAP( "at93c46_aa.8g", 0x000000, 0x000080, BAD_DUMP CRC(026b0ea5) SHA1(5ab63b88caeb9dc53732b1a432f884d85bcc222c) ) // hand built
ROM_END

ROM_START(sscope2ucvd1)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931c01.27p", 0x200000, 0x200000, CRC(653ba4d9) SHA1(29c1c1d5088e6ba7fa5cfa63b5975f47b54602ee) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04_c.16t", 0x000000, 0x200000, CRC(a05446e3) SHA1(67aef3cfe217223aea53dbc5cccd8d706eae8864) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, CRC(f7c40218) SHA1(5021089803024a6f552e5c9d42b905e804b9d904) )

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401_gk830.16g", 0x000000, 0x000008, BAD_DUMP CRC(bae36d0b) SHA1(4dd5915888d5718356b40bbe897f2470e410176a) ) // hand built

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD( "at93c46.8g", 0x000000, 0x000080, BAD_DUMP CRC(cc63c213) SHA1(fb20e56fb73a887dc7b6db49efd1f8a18b959152) ) // hand built
ROM_END

ROM_START(sscope2ecvd1)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931c01.27p", 0x200000, 0x200000, CRC(653ba4d9) SHA1(29c1c1d5088e6ba7fa5cfa63b5975f47b54602ee) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04_c.16t", 0x000000, 0x200000, CRC(a05446e3) SHA1(67aef3cfe217223aea53dbc5cccd8d706eae8864) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ea", 0x000000, 0x002000, BAD_DUMP CRC(832f9148) SHA1(42a8cc9436eaa79b5bab242692e18c3807f6af74) ) // hand built

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401.16g", 0x000000, 0x000008, CRC(908da6dd) SHA1(f7c1a2ebe05f4bc403a6154d724f8f6f6eeeff15) )

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD16_WORD_SWAP( "at93c46_ea.8g", 0x000000, 0x000080, BAD_DUMP CRC(b6da86a4) SHA1(3a6570ac25748fb5e6b8a0dd6b832ee2d463cc7b) ) // hand built
ROM_END

ROM_START(sscope2jcvd1)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931c01.27p", 0x200000, 0x200000, CRC(653ba4d9) SHA1(29c1c1d5088e6ba7fa5cfa63b5975f47b54602ee) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04_c.16t", 0x000000, 0x200000, CRC(a05446e3) SHA1(67aef3cfe217223aea53dbc5cccd8d706eae8864) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ja", 0x000000, 0x002000, BAD_DUMP CRC(d16ac629) SHA1(92c65a67ef201912e4f81d896126b045c5cc2072) ) // hand built

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401.16g", 0x000000, 0x000008, CRC(908da6dd) SHA1(f7c1a2ebe05f4bc403a6154d724f8f6f6eeeff15) )

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD16_WORD_SWAP( "at93c46_ja.8g", 0x000000, 0x000080, BAD_DUMP CRC(6613c091) SHA1(101a15afc27d5b4b5e846dc6823c14656132b26b) ) // hand built
ROM_END

ROM_START(sscope2acvd1)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931c01.27p", 0x200000, 0x200000, CRC(653ba4d9) SHA1(29c1c1d5088e6ba7fa5cfa63b5975f47b54602ee) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04_c.16t", 0x000000, 0x200000, CRC(a05446e3) SHA1(67aef3cfe217223aea53dbc5cccd8d706eae8864) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_aa", 0x000000, 0x002000, BAD_DUMP CRC(164c1a0d) SHA1(9f7e6cc1acae114aa97d9ed435661bf9c8b845c5) ) // hand built

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401.16g", 0x000000, 0x000008, CRC(908da6dd) SHA1(f7c1a2ebe05f4bc403a6154d724f8f6f6eeeff15) )

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD16_WORD_SWAP( "at93c46_aa.8g", 0x000000, 0x000080, BAD_DUMP CRC(026b0ea5) SHA1(5ab63b88caeb9dc53732b1a432f884d85bcc222c) ) // hand built
ROM_END

ROM_START(sscope2ubvd1)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931b01.27p", 0x200000, 0x200000, CRC(deb036b7) SHA1(12280aa4e37c3492e5192d630c26e758d08744dd) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04.16t", 0x000000, 0x200000, CRC(4f5917e6) SHA1(a63a107f1d6d9756e4ab0965d72ea446f0692814) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, CRC(f7c40218) SHA1(5021089803024a6f552e5c9d42b905e804b9d904) )

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401_gk830.16g", 0x000000, 0x000008, BAD_DUMP CRC(bae36d0b) SHA1(4dd5915888d5718356b40bbe897f2470e410176a) ) // hand built

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD( "at93c46.8g", 0x000000, 0x000080, BAD_DUMP CRC(cc63c213) SHA1(fb20e56fb73a887dc7b6db49efd1f8a18b959152) ) // hand built
ROM_END

ROM_START(sscope2ebvd1)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931b01.27p", 0x200000, 0x200000, CRC(deb036b7) SHA1(12280aa4e37c3492e5192d630c26e758d08744dd) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04.16t", 0x000000, 0x200000, CRC(4f5917e6) SHA1(a63a107f1d6d9756e4ab0965d72ea446f0692814) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ea", 0x000000, 0x002000, BAD_DUMP CRC(832f9148) SHA1(42a8cc9436eaa79b5bab242692e18c3807f6af74) ) // hand built

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401.16g", 0x000000, 0x000008, CRC(908da6dd) SHA1(f7c1a2ebe05f4bc403a6154d724f8f6f6eeeff15) )

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD16_WORD_SWAP( "at93c46_ea.8g", 0x000000, 0x000080, BAD_DUMP CRC(b6da86a4) SHA1(3a6570ac25748fb5e6b8a0dd6b832ee2d463cc7b) ) // hand built
ROM_END

ROM_START(sscope2jbvd1)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931b01.27p", 0x200000, 0x200000, CRC(deb036b7) SHA1(12280aa4e37c3492e5192d630c26e758d08744dd) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04.16t", 0x000000, 0x200000, CRC(4f5917e6) SHA1(a63a107f1d6d9756e4ab0965d72ea446f0692814) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_ja", 0x000000, 0x002000, BAD_DUMP CRC(d16ac629) SHA1(92c65a67ef201912e4f81d896126b045c5cc2072) ) // hand built

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401.16g", 0x000000, 0x000008, CRC(908da6dd) SHA1(f7c1a2ebe05f4bc403a6154d724f8f6f6eeeff15) )

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD16_WORD_SWAP( "at93c46_ja.8g", 0x000000, 0x000080, BAD_DUMP CRC(6613c091) SHA1(101a15afc27d5b4b5e846dc6823c14656132b26b) ) // hand built
ROM_END

ROM_START(sscope2abvd1)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP("931b01.27p", 0x200000, 0x200000, CRC(deb036b7) SHA1(12280aa4e37c3492e5192d630c26e758d08744dd) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP("931a04.16t", 0x000000, 0x200000, CRC(4f5917e6) SHA1(a63a107f1d6d9756e4ab0965d72ea446f0692814) )

	ROM_REGION32_BE(0x800000, "comm_board", 0)   // Comm board roms
	ROM_LOAD("931a19.8e", 0x000000, 0x400000, CRC(0417b528) SHA1(ebd7f06b83256b94784de164f9d0642bfb2c94d4) )
	ROM_LOAD("931a20.6e", 0x400000, 0x400000, CRC(d367a4c9) SHA1(8bf029841d9d3be20dea0423240bfec825477a1d) )

	ROM_REGION(0x1000000, "cgboard_0", ROMREGION_ERASE00)    // CG Board texture roms

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP("931a08.7s", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION16_LE(0xc00000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "931a09.16p",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.14p",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.12p",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_aa", 0x000000, 0x002000, BAD_DUMP CRC(164c1a0d) SHA1(9f7e6cc1acae114aa97d9ed435661bf9c8b845c5) ) // hand built

	ROM_REGION(0x8, "lan_serial_id", 0)     // LAN Board DS2401
	ROM_LOAD( "ds2401.16g", 0x000000, 0x000008, CRC(908da6dd) SHA1(f7c1a2ebe05f4bc403a6154d724f8f6f6eeeff15) )

	ROM_REGION16_BE(0x80, "lan_eeprom", 0)       // LAN Board AT93C46
	ROM_LOAD16_WORD_SWAP( "at93c46_aa.8g", 0x000000, 0x000080, BAD_DUMP CRC(026b0ea5) SHA1(5ab63b88caeb9dc53732b1a432f884d85bcc222c) ) // hand built
ROM_END

ROM_START(gradius4)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP( "837c01.27p",   0x200000, 0x200000, CRC(ce003123) SHA1(15e33997be2c1b3f71998627c540db378680a7a1) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP( "837a04.16t",   0x000000, 0x200000, CRC(18453b59) SHA1(3c75a54d8c09c0796223b42d30fb3867a911a074) )
	ROM_LOAD32_WORD_SWAP( "837a05.14t",   0x000002, 0x200000, CRC(77178633) SHA1(ececdd501d0692390325c8dad6dbb068808a8b26) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", 0)  // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "837a14.32u",   0x000002, 0x400000, CRC(ff1b5d18) SHA1(7a38362170133dcc6ea01eb62981845917b85c36) )
	ROM_LOAD32_WORD_SWAP( "837a13.24u",   0x000000, 0x400000, CRC(d86e10ff) SHA1(6de1179d7081d9a93ab6df47692d3efc190c38ba) )
	ROM_LOAD32_WORD_SWAP( "837a16.32v",   0x800002, 0x400000, CRC(bb7a7558) SHA1(8c8cc062793c2dcfa72657b6ea0813d7223a0b87) )
	ROM_LOAD32_WORD_SWAP( "837a15.24v",   0x800000, 0x400000, CRC(e0620737) SHA1(c14078cdb44f75c7c956b3627045d8494941d6b4) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP( "837a08.7s",    0x000000, 0x080000, CRC(c3a7ff56) SHA1(9d8d033277d560b58da151338d14b4758a9235ea) )

	ROM_REGION16_LE(0x800000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "837a09.16p",   0x000000, 0x400000, CRC(fb8f3dc2) SHA1(69e314ac06308c5a24309abc3d7b05af6c0302a8) )
	ROM_LOAD( "837a10.14p",   0x400000, 0x400000, CRC(1419cad2) SHA1(a6369a5c29813fa51e8246d0c091736f32994f3d) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_jac", 0x000000, 0x002000, BAD_DUMP CRC(935f9d05) SHA1(c3a787dff1b2ac4942858ffa1574405db01292b6) ) // hand built

	ROM_REGION( 0x0000224, "security_eeprom", 0 )
	ROM_LOAD( "security_eeprom_ja", 0x000000, 0x000224, BAD_DUMP CRC(76b57192) SHA1(da510e389c26e1b3f9bba09f34450225a9a0a6ff) ) // hand built
ROM_END

ROM_START(gradius4u)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP( "837c01.27p",   0x200000, 0x200000, CRC(ce003123) SHA1(15e33997be2c1b3f71998627c540db378680a7a1) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP( "837a04.16t",   0x000000, 0x200000, CRC(18453b59) SHA1(3c75a54d8c09c0796223b42d30fb3867a911a074) )
	ROM_LOAD32_WORD_SWAP( "837a05.14t",   0x000002, 0x200000, CRC(77178633) SHA1(ececdd501d0692390325c8dad6dbb068808a8b26) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", 0)  // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "837a14.32u",   0x000002, 0x400000, CRC(ff1b5d18) SHA1(7a38362170133dcc6ea01eb62981845917b85c36) )
	ROM_LOAD32_WORD_SWAP( "837a13.24u",   0x000000, 0x400000, CRC(d86e10ff) SHA1(6de1179d7081d9a93ab6df47692d3efc190c38ba) )
	ROM_LOAD32_WORD_SWAP( "837a16.32v",   0x800002, 0x400000, CRC(bb7a7558) SHA1(8c8cc062793c2dcfa72657b6ea0813d7223a0b87) )
	ROM_LOAD32_WORD_SWAP( "837a15.24v",   0x800000, 0x400000, CRC(e0620737) SHA1(c14078cdb44f75c7c956b3627045d8494941d6b4) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP( "837a08.7s",    0x000000, 0x080000, CRC(c3a7ff56) SHA1(9d8d033277d560b58da151338d14b4758a9235ea) )

	ROM_REGION16_LE(0x800000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "837a09.16p",   0x000000, 0x400000, CRC(fb8f3dc2) SHA1(69e314ac06308c5a24309abc3d7b05af6c0302a8) )
	ROM_LOAD( "837a10.14p",   0x400000, 0x400000, CRC(1419cad2) SHA1(a6369a5c29813fa51e8246d0c091736f32994f3d) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_uac", 0x000000, 0x002000, BAD_DUMP CRC(cc8986c1) SHA1(a32bc175acae48bede7a97629e215ab4fb6954c6) ) // hand built

	ROM_REGION( 0x0000224, "security_eeprom", 0 )
	ROM_LOAD( "security_eeprom_ua", 0x000000, 0x000224, BAD_DUMP CRC(386b1464) SHA1(41bace7acad17f37a934ca001ac7b92f45aabce9) ) // hand built
ROM_END

ROM_START(gradius4a)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP( "837c01.27p",   0x200000, 0x200000, CRC(ce003123) SHA1(15e33997be2c1b3f71998627c540db378680a7a1) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP( "837a04.16t",   0x000000, 0x200000, CRC(18453b59) SHA1(3c75a54d8c09c0796223b42d30fb3867a911a074) )
	ROM_LOAD32_WORD_SWAP( "837a05.14t",   0x000002, 0x200000, CRC(77178633) SHA1(ececdd501d0692390325c8dad6dbb068808a8b26) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", 0)  // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "837a14.32u",   0x000002, 0x400000, CRC(ff1b5d18) SHA1(7a38362170133dcc6ea01eb62981845917b85c36) )
	ROM_LOAD32_WORD_SWAP( "837a13.24u",   0x000000, 0x400000, CRC(d86e10ff) SHA1(6de1179d7081d9a93ab6df47692d3efc190c38ba) )
	ROM_LOAD32_WORD_SWAP( "837a16.32v",   0x800002, 0x400000, CRC(bb7a7558) SHA1(8c8cc062793c2dcfa72657b6ea0813d7223a0b87) )
	ROM_LOAD32_WORD_SWAP( "837a15.24v",   0x800000, 0x400000, CRC(e0620737) SHA1(c14078cdb44f75c7c956b3627045d8494941d6b4) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP( "837a08.7s",    0x000000, 0x080000, CRC(c3a7ff56) SHA1(9d8d033277d560b58da151338d14b4758a9235ea) )

	ROM_REGION16_LE(0x800000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "837a09.16p",   0x000000, 0x400000, CRC(fb8f3dc2) SHA1(69e314ac06308c5a24309abc3d7b05af6c0302a8) )
	ROM_LOAD( "837a10.14p",   0x400000, 0x400000, CRC(1419cad2) SHA1(a6369a5c29813fa51e8246d0c091736f32994f3d) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_aac", 0x000000, 0x002000, BAD_DUMP CRC(7977736d) SHA1(149ae7bc4987362f928a6c0c1e9671c2396ac811) ) // hand built

	ROM_REGION( 0x0000224, "security_eeprom", 0 )
	ROM_LOAD( "security_eeprom_aa", 0x000000, 0x000224, BAD_DUMP CRC(6fd2e9ea) SHA1(90a90c8173c595f20efcf2525697b87989d2b67f) ) // hand built
ROM_END

ROM_START(gradius4ja)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP( "837a01.27p",   0x200000, 0x200000, CRC(6083ed08) SHA1(42d4dc78a94b235ae4ea5934641528eb776dcdde) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP( "837a04.16t",   0x000000, 0x200000, CRC(18453b59) SHA1(3c75a54d8c09c0796223b42d30fb3867a911a074) )
	ROM_LOAD32_WORD_SWAP( "837a05.14t",   0x000002, 0x200000, CRC(77178633) SHA1(ececdd501d0692390325c8dad6dbb068808a8b26) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", 0)  // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "837a14.32u",   0x000002, 0x400000, CRC(ff1b5d18) SHA1(7a38362170133dcc6ea01eb62981845917b85c36) )
	ROM_LOAD32_WORD_SWAP( "837a13.24u",   0x000000, 0x400000, CRC(d86e10ff) SHA1(6de1179d7081d9a93ab6df47692d3efc190c38ba) )
	ROM_LOAD32_WORD_SWAP( "837a16.32v",   0x800002, 0x400000, CRC(bb7a7558) SHA1(8c8cc062793c2dcfa72657b6ea0813d7223a0b87) )
	ROM_LOAD32_WORD_SWAP( "837a15.24v",   0x800000, 0x400000, CRC(e0620737) SHA1(c14078cdb44f75c7c956b3627045d8494941d6b4) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP( "837a08.7s",    0x000000, 0x080000, CRC(c3a7ff56) SHA1(9d8d033277d560b58da151338d14b4758a9235ea) )

	ROM_REGION16_LE(0x800000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "837a09.16p",   0x000000, 0x400000, CRC(fb8f3dc2) SHA1(69e314ac06308c5a24309abc3d7b05af6c0302a8) )
	ROM_LOAD( "837a10.14p",   0x400000, 0x400000, CRC(1419cad2) SHA1(a6369a5c29813fa51e8246d0c091736f32994f3d) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_jaa", 0x000000, 0x002000, BAD_DUMP CRC(264dc314) SHA1(1800b93063a6b3d424c329e124acc30814eb7ef0) ) // hand built

	ROM_REGION( 0x0000224, "security_eeprom", 0 )
	ROM_LOAD( "security_eeprom_ja", 0x000000, 0x000224, BAD_DUMP CRC(76b57192) SHA1(da510e389c26e1b3f9bba09f34450225a9a0a6ff) ) // hand built
ROM_END

ROM_START(gradius4ua)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP( "837a01.27p",   0x200000, 0x200000, CRC(6083ed08) SHA1(42d4dc78a94b235ae4ea5934641528eb776dcdde) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP( "837a04.16t",   0x000000, 0x200000, CRC(18453b59) SHA1(3c75a54d8c09c0796223b42d30fb3867a911a074) )
	ROM_LOAD32_WORD_SWAP( "837a05.14t",   0x000002, 0x200000, CRC(77178633) SHA1(ececdd501d0692390325c8dad6dbb068808a8b26) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", 0)  // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "837a14.32u",   0x000002, 0x400000, CRC(ff1b5d18) SHA1(7a38362170133dcc6ea01eb62981845917b85c36) )
	ROM_LOAD32_WORD_SWAP( "837a13.24u",   0x000000, 0x400000, CRC(d86e10ff) SHA1(6de1179d7081d9a93ab6df47692d3efc190c38ba) )
	ROM_LOAD32_WORD_SWAP( "837a16.32v",   0x800002, 0x400000, CRC(bb7a7558) SHA1(8c8cc062793c2dcfa72657b6ea0813d7223a0b87) )
	ROM_LOAD32_WORD_SWAP( "837a15.24v",   0x800000, 0x400000, CRC(e0620737) SHA1(c14078cdb44f75c7c956b3627045d8494941d6b4) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP( "837a08.7s",    0x000000, 0x080000, CRC(c3a7ff56) SHA1(9d8d033277d560b58da151338d14b4758a9235ea) )

	ROM_REGION16_LE(0x800000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "837a09.16p",   0x000000, 0x400000, CRC(fb8f3dc2) SHA1(69e314ac06308c5a24309abc3d7b05af6c0302a8) )
	ROM_LOAD( "837a10.14p",   0x400000, 0x400000, CRC(1419cad2) SHA1(a6369a5c29813fa51e8246d0c091736f32994f3d) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_uaa", 0x000000, 0x002000, BAD_DUMP CRC(799bd8d0) SHA1(c69b5bb99657c2fdb71049ba1db0075a024fe8ff) ) // hand built

	ROM_REGION( 0x0000224, "security_eeprom", 0 )
	ROM_LOAD( "security_eeprom_ua", 0x000000, 0x000224, BAD_DUMP CRC(386b1464) SHA1(41bace7acad17f37a934ca001ac7b92f45aabce9) ) // hand built
ROM_END

ROM_START(gradius4aa)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP( "837a01.27p",   0x200000, 0x200000, CRC(6083ed08) SHA1(42d4dc78a94b235ae4ea5934641528eb776dcdde) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP( "837a04.16t",   0x000000, 0x200000, CRC(18453b59) SHA1(3c75a54d8c09c0796223b42d30fb3867a911a074) )
	ROM_LOAD32_WORD_SWAP( "837a05.14t",   0x000002, 0x200000, CRC(77178633) SHA1(ececdd501d0692390325c8dad6dbb068808a8b26) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", 0)  // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "837a14.32u",   0x000002, 0x400000, CRC(ff1b5d18) SHA1(7a38362170133dcc6ea01eb62981845917b85c36) )
	ROM_LOAD32_WORD_SWAP( "837a13.24u",   0x000000, 0x400000, CRC(d86e10ff) SHA1(6de1179d7081d9a93ab6df47692d3efc190c38ba) )
	ROM_LOAD32_WORD_SWAP( "837a16.32v",   0x800002, 0x400000, CRC(bb7a7558) SHA1(8c8cc062793c2dcfa72657b6ea0813d7223a0b87) )
	ROM_LOAD32_WORD_SWAP( "837a15.24v",   0x800000, 0x400000, CRC(e0620737) SHA1(c14078cdb44f75c7c956b3627045d8494941d6b4) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP( "837a08.7s",    0x000000, 0x080000, CRC(c3a7ff56) SHA1(9d8d033277d560b58da151338d14b4758a9235ea) )

	ROM_REGION16_LE(0x800000, "rfsnd", 0)        // PCM sample roms
	ROM_LOAD( "837a09.16p",   0x000000, 0x400000, CRC(fb8f3dc2) SHA1(69e314ac06308c5a24309abc3d7b05af6c0302a8) )
	ROM_LOAD( "837a10.14p",   0x400000, 0x400000, CRC(1419cad2) SHA1(a6369a5c29813fa51e8246d0c091736f32994f3d) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_aaa", 0x000000, 0x002000, BAD_DUMP CRC(cc652d7c) SHA1(238086ba9eac26e6ae4217e085859e40f98a5050) ) // hand built

	ROM_REGION( 0x0000224, "security_eeprom", 0 )
	ROM_LOAD( "security_eeprom_aa", 0x000000, 0x000224, BAD_DUMP CRC(6fd2e9ea) SHA1(90a90c8173c595f20efcf2525697b87989d2b67f) ) // hand built
ROM_END

ROM_START(nbapbp) // only the PowerPC program rom present in the archive
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP( "778b01.27p",   0x200000, 0x200000, CRC(8dca96b5) SHA1(7dfa38c4be6c3547ee9c7ad104282510e205ab37) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP( "778a04.16t",   0x000000, 0x400000, CRC(62c70132) SHA1(405aed149fc51e0adfa3ace3c644e47d53cf1ee3) )
	ROM_LOAD32_WORD_SWAP( "778a05.14t",   0x000002, 0x400000, CRC(03249803) SHA1(f632a5f1dfa0a8500407214df0ec8d98ce09bc2b) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", 0)  // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "778a14.32u",   0x000002, 0x400000, CRC(db0c278d) SHA1(bb9884b6cdcdb707fff7e56e92e2ede062abcfd3) )
	ROM_LOAD32_WORD_SWAP( "778a13.24u",   0x000000, 0x400000, CRC(47fda9cc) SHA1(4aae01c1f1861b4b12a3f9de6b39eb4d11a9736b) )
	ROM_LOAD32_WORD_SWAP( "778a16.32v",   0x800002, 0x400000, CRC(6c0f46ea) SHA1(c6b9fbe14e13114a91a5925a0b46496260539687) )
	ROM_LOAD32_WORD_SWAP( "778a15.24v",   0x800000, 0x400000, CRC(d176ad0d) SHA1(2be755dfa3f60379d396734809bbaaaad49e0db5) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP( "778a08.7s",    0x000000, 0x080000, CRC(6259b4bf) SHA1(d0c38870495c9a07984b4b85e736d6477dd44832) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "778a09.16p",   0x000000, 0x400000, CRC(e8c6fd93) SHA1(dd378b67b3b7dd932e4b39fbf4321e706522247f) )
	ROM_LOAD( "778a10.14p",   0x400000, 0x400000, CRC(c6a0857b) SHA1(976734ba56460fcc090619fbba043a3d888c4f4e) )
	ROM_LOAD( "778a11.12p",   0x800000, 0x400000, CRC(40199382) SHA1(bee268adf9b6634a4f6bb39278ecd02f2bdcb1f4) )
	ROM_LOAD( "778a12.9p",    0xc00000, 0x400000, CRC(27d0c724) SHA1(48e48cbaea6db0de8c3471a2eda6faaa16eed46e) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, BAD_DUMP CRC(3cff1b1d) SHA1(bed0fc657a785be0c69bb21ad52365635c83d751) ) // hand built

	ROM_REGION( 0x0000224, "security_eeprom", 0 )
	ROM_LOAD( "security_eeprom_uab",  0x000000, 0x000224, BAD_DUMP CRC(bdef9a1f) SHA1(9f47efeac272362be0e1a999c71df6b07ed76e8d) ) // hand built
ROM_END

ROM_START(nbapbpa) // only the PowerPC program rom present in the archive
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP( "778b01.27p",   0x200000, 0x200000, CRC(8dca96b5) SHA1(7dfa38c4be6c3547ee9c7ad104282510e205ab37) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP( "778a04.16t",   0x000000, 0x400000, CRC(62c70132) SHA1(405aed149fc51e0adfa3ace3c644e47d53cf1ee3) )
	ROM_LOAD32_WORD_SWAP( "778a05.14t",   0x000002, 0x400000, CRC(03249803) SHA1(f632a5f1dfa0a8500407214df0ec8d98ce09bc2b) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", 0)  // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "778a14.32u",   0x000002, 0x400000, CRC(db0c278d) SHA1(bb9884b6cdcdb707fff7e56e92e2ede062abcfd3) )
	ROM_LOAD32_WORD_SWAP( "778a13.24u",   0x000000, 0x400000, CRC(47fda9cc) SHA1(4aae01c1f1861b4b12a3f9de6b39eb4d11a9736b) )
	ROM_LOAD32_WORD_SWAP( "778a16.32v",   0x800002, 0x400000, CRC(6c0f46ea) SHA1(c6b9fbe14e13114a91a5925a0b46496260539687) )
	ROM_LOAD32_WORD_SWAP( "778a15.24v",   0x800000, 0x400000, CRC(d176ad0d) SHA1(2be755dfa3f60379d396734809bbaaaad49e0db5) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP( "778a08.7s",    0x000000, 0x080000, CRC(6259b4bf) SHA1(d0c38870495c9a07984b4b85e736d6477dd44832) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "778a09.16p",   0x000000, 0x400000, CRC(e8c6fd93) SHA1(dd378b67b3b7dd932e4b39fbf4321e706522247f) )
	ROM_LOAD( "778a10.14p",   0x400000, 0x400000, CRC(c6a0857b) SHA1(976734ba56460fcc090619fbba043a3d888c4f4e) )
	ROM_LOAD( "778a11.12p",   0x800000, 0x400000, CRC(40199382) SHA1(bee268adf9b6634a4f6bb39278ecd02f2bdcb1f4) )
	ROM_LOAD( "778a12.9p",    0xc00000, 0x400000, CRC(27d0c724) SHA1(48e48cbaea6db0de8c3471a2eda6faaa16eed46e) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, BAD_DUMP CRC(3cff1b1d) SHA1(bed0fc657a785be0c69bb21ad52365635c83d751) ) // hand built

	ROM_REGION( 0x0000224, "security_eeprom", 0 )
	ROM_LOAD( "security_eeprom_aab",  0x000000, 0x000224, BAD_DUMP CRC(1c392865) SHA1(dea5067eb5f8b10680c3b38897f6f42353ae7ac0) ) // hand built
ROM_END

ROM_START(nbapbpj) // only the PowerPC program rom present in the archive
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP( "778b01.27p",   0x200000, 0x200000, CRC(8dca96b5) SHA1(7dfa38c4be6c3547ee9c7ad104282510e205ab37) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP( "778a04.16t",   0x000000, 0x400000, CRC(62c70132) SHA1(405aed149fc51e0adfa3ace3c644e47d53cf1ee3) )
	ROM_LOAD32_WORD_SWAP( "778a05.14t",   0x000002, 0x400000, CRC(03249803) SHA1(f632a5f1dfa0a8500407214df0ec8d98ce09bc2b) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", 0)  // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "778a14.32u",   0x000002, 0x400000, CRC(db0c278d) SHA1(bb9884b6cdcdb707fff7e56e92e2ede062abcfd3) )
	ROM_LOAD32_WORD_SWAP( "778a13.24u",   0x000000, 0x400000, CRC(47fda9cc) SHA1(4aae01c1f1861b4b12a3f9de6b39eb4d11a9736b) )
	ROM_LOAD32_WORD_SWAP( "778a16.32v",   0x800002, 0x400000, CRC(6c0f46ea) SHA1(c6b9fbe14e13114a91a5925a0b46496260539687) )
	ROM_LOAD32_WORD_SWAP( "778a15.24v",   0x800000, 0x400000, CRC(d176ad0d) SHA1(2be755dfa3f60379d396734809bbaaaad49e0db5) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP( "778a08.7s",    0x000000, 0x080000, CRC(6259b4bf) SHA1(d0c38870495c9a07984b4b85e736d6477dd44832) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "778a09.16p",   0x000000, 0x400000, CRC(e8c6fd93) SHA1(dd378b67b3b7dd932e4b39fbf4321e706522247f) )
	ROM_LOAD( "778a10.14p",   0x400000, 0x400000, CRC(c6a0857b) SHA1(976734ba56460fcc090619fbba043a3d888c4f4e) )
	ROM_LOAD( "778a11.12p",   0x800000, 0x400000, CRC(40199382) SHA1(bee268adf9b6634a4f6bb39278ecd02f2bdcb1f4) )
	ROM_LOAD( "778a12.9p",    0xc00000, 0x400000, CRC(27d0c724) SHA1(48e48cbaea6db0de8c3471a2eda6faaa16eed46e) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, BAD_DUMP CRC(3cff1b1d) SHA1(bed0fc657a785be0c69bb21ad52365635c83d751) ) // hand built

	ROM_REGION( 0x0000224, "security_eeprom", 0 )
	ROM_LOAD( "security_eeprom_jab",  0x000000, 0x000224, BAD_DUMP CRC(66604175) SHA1(b8eb176697ba9dd3fcd274455570cd362d78180f) ) // hand built
ROM_END

ROM_START(nbapbpua)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP( "778a01.27p",   0x200000, 0x200000, CRC(e70019ce) SHA1(8b187b6e670fdc88771da08a56685cd621b139dc) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP( "778a04.16t",   0x000000, 0x400000, CRC(62c70132) SHA1(405aed149fc51e0adfa3ace3c644e47d53cf1ee3) )
	ROM_LOAD32_WORD_SWAP( "778a05.14t",   0x000002, 0x400000, CRC(03249803) SHA1(f632a5f1dfa0a8500407214df0ec8d98ce09bc2b) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", 0)  // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "778a14.32u",   0x000002, 0x400000, CRC(db0c278d) SHA1(bb9884b6cdcdb707fff7e56e92e2ede062abcfd3) )
	ROM_LOAD32_WORD_SWAP( "778a13.24u",   0x000000, 0x400000, CRC(47fda9cc) SHA1(4aae01c1f1861b4b12a3f9de6b39eb4d11a9736b) )
	ROM_LOAD32_WORD_SWAP( "778a16.32v",   0x800002, 0x400000, CRC(6c0f46ea) SHA1(c6b9fbe14e13114a91a5925a0b46496260539687) )
	ROM_LOAD32_WORD_SWAP( "778a15.24v",   0x800000, 0x400000, CRC(d176ad0d) SHA1(2be755dfa3f60379d396734809bbaaaad49e0db5) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP( "778a08.7s",    0x000000, 0x080000, CRC(6259b4bf) SHA1(d0c38870495c9a07984b4b85e736d6477dd44832) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "778a09.16p",   0x000000, 0x400000, CRC(e8c6fd93) SHA1(dd378b67b3b7dd932e4b39fbf4321e706522247f) )
	ROM_LOAD( "778a10.14p",   0x400000, 0x400000, CRC(c6a0857b) SHA1(976734ba56460fcc090619fbba043a3d888c4f4e) )
	ROM_LOAD( "778a11.12p",   0x800000, 0x400000, CRC(40199382) SHA1(bee268adf9b6634a4f6bb39278ecd02f2bdcb1f4) )
	ROM_LOAD( "778a12.9p",    0xc00000, 0x400000, CRC(27d0c724) SHA1(48e48cbaea6db0de8c3471a2eda6faaa16eed46e) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, BAD_DUMP CRC(3cff1b1d) SHA1(bed0fc657a785be0c69bb21ad52365635c83d751) ) // hand built

	ROM_REGION( 0x0000224, "security_eeprom", 0 )
	ROM_LOAD( "security_eeprom_uaa",  0x000000, 0x000224, BAD_DUMP CRC(bd8c9d3b) SHA1(2f9a84923a219f1b746fb247209aec498b80e1f4) ) // hand built
ROM_END

ROM_START(nbapbpaa)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP( "778a01.27p",   0x200000, 0x200000, CRC(e70019ce) SHA1(8b187b6e670fdc88771da08a56685cd621b139dc) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP( "778a04.16t",   0x000000, 0x400000, CRC(62c70132) SHA1(405aed149fc51e0adfa3ace3c644e47d53cf1ee3) )
	ROM_LOAD32_WORD_SWAP( "778a05.14t",   0x000002, 0x400000, CRC(03249803) SHA1(f632a5f1dfa0a8500407214df0ec8d98ce09bc2b) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", 0)  // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "778a14.32u",   0x000002, 0x400000, CRC(db0c278d) SHA1(bb9884b6cdcdb707fff7e56e92e2ede062abcfd3) )
	ROM_LOAD32_WORD_SWAP( "778a13.24u",   0x000000, 0x400000, CRC(47fda9cc) SHA1(4aae01c1f1861b4b12a3f9de6b39eb4d11a9736b) )
	ROM_LOAD32_WORD_SWAP( "778a16.32v",   0x800002, 0x400000, CRC(6c0f46ea) SHA1(c6b9fbe14e13114a91a5925a0b46496260539687) )
	ROM_LOAD32_WORD_SWAP( "778a15.24v",   0x800000, 0x400000, CRC(d176ad0d) SHA1(2be755dfa3f60379d396734809bbaaaad49e0db5) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP( "778a08.7s",    0x000000, 0x080000, CRC(6259b4bf) SHA1(d0c38870495c9a07984b4b85e736d6477dd44832) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "778a09.16p",   0x000000, 0x400000, CRC(e8c6fd93) SHA1(dd378b67b3b7dd932e4b39fbf4321e706522247f) )
	ROM_LOAD( "778a10.14p",   0x400000, 0x400000, CRC(c6a0857b) SHA1(976734ba56460fcc090619fbba043a3d888c4f4e) )
	ROM_LOAD( "778a11.12p",   0x800000, 0x400000, CRC(40199382) SHA1(bee268adf9b6634a4f6bb39278ecd02f2bdcb1f4) )
	ROM_LOAD( "778a12.9p",    0xc00000, 0x400000, CRC(27d0c724) SHA1(48e48cbaea6db0de8c3471a2eda6faaa16eed46e) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, BAD_DUMP CRC(3cff1b1d) SHA1(bed0fc657a785be0c69bb21ad52365635c83d751) ) // hand built

	ROM_REGION( 0x0000224, "security_eeprom", 0 )
	ROM_LOAD( "security_eeprom_aaa",  0x000000, 0x000224, BAD_DUMP CRC(1c5a2f41) SHA1(00225338d2fb8ae3c91e3cf4c3526323c4df74b2) ) // hand built
ROM_END

ROM_START(nbapbpja)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP( "778a01.27p",   0x200000, 0x200000, CRC(e70019ce) SHA1(8b187b6e670fdc88771da08a56685cd621b139dc) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP( "778a04.16t",   0x000000, 0x400000, CRC(62c70132) SHA1(405aed149fc51e0adfa3ace3c644e47d53cf1ee3) )
	ROM_LOAD32_WORD_SWAP( "778a05.14t",   0x000002, 0x400000, CRC(03249803) SHA1(f632a5f1dfa0a8500407214df0ec8d98ce09bc2b) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", 0)  // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "778a14.32u",   0x000002, 0x400000, CRC(db0c278d) SHA1(bb9884b6cdcdb707fff7e56e92e2ede062abcfd3) )
	ROM_LOAD32_WORD_SWAP( "778a13.24u",   0x000000, 0x400000, CRC(47fda9cc) SHA1(4aae01c1f1861b4b12a3f9de6b39eb4d11a9736b) )
	ROM_LOAD32_WORD_SWAP( "778a16.32v",   0x800002, 0x400000, CRC(6c0f46ea) SHA1(c6b9fbe14e13114a91a5925a0b46496260539687) )
	ROM_LOAD32_WORD_SWAP( "778a15.24v",   0x800000, 0x400000, CRC(d176ad0d) SHA1(2be755dfa3f60379d396734809bbaaaad49e0db5) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP( "778a08.7s",    0x000000, 0x080000, CRC(6259b4bf) SHA1(d0c38870495c9a07984b4b85e736d6477dd44832) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "778a09.16p",   0x000000, 0x400000, CRC(e8c6fd93) SHA1(dd378b67b3b7dd932e4b39fbf4321e706522247f) )
	ROM_LOAD( "778a10.14p",   0x400000, 0x400000, CRC(c6a0857b) SHA1(976734ba56460fcc090619fbba043a3d888c4f4e) )
	ROM_LOAD( "778a11.12p",   0x800000, 0x400000, CRC(40199382) SHA1(bee268adf9b6634a4f6bb39278ecd02f2bdcb1f4) )
	ROM_LOAD( "778a12.9p",    0xc00000, 0x400000, CRC(27d0c724) SHA1(48e48cbaea6db0de8c3471a2eda6faaa16eed46e) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, BAD_DUMP CRC(3cff1b1d) SHA1(bed0fc657a785be0c69bb21ad52365635c83d751) ) // hand built

	ROM_REGION( 0x0000224, "security_eeprom", 0 )
	ROM_LOAD( "security_eeprom_jaa",  0x000000, 0x000224, BAD_DUMP CRC(c1177c6f) SHA1(bf00a94f6c7e97e6109157e7147426ec13acd497) ) // hand built
ROM_END

ROM_START(nbaatw) // only the PowerPC program rom present in the archive
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP( "778b01.27p",   0x200000, 0x200000, CRC(8dca96b5) SHA1(7dfa38c4be6c3547ee9c7ad104282510e205ab37) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP( "778a04.16t",   0x000000, 0x400000, CRC(62c70132) SHA1(405aed149fc51e0adfa3ace3c644e47d53cf1ee3) )
	ROM_LOAD32_WORD_SWAP( "778a05.14t",   0x000002, 0x400000, CRC(03249803) SHA1(f632a5f1dfa0a8500407214df0ec8d98ce09bc2b) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", 0)  // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "778a14.32u",   0x000002, 0x400000, CRC(db0c278d) SHA1(bb9884b6cdcdb707fff7e56e92e2ede062abcfd3) )
	ROM_LOAD32_WORD_SWAP( "778a13.24u",   0x000000, 0x400000, CRC(47fda9cc) SHA1(4aae01c1f1861b4b12a3f9de6b39eb4d11a9736b) )
	ROM_LOAD32_WORD_SWAP( "778a16.32v",   0x800002, 0x400000, CRC(6c0f46ea) SHA1(c6b9fbe14e13114a91a5925a0b46496260539687) )
	ROM_LOAD32_WORD_SWAP( "778a15.24v",   0x800000, 0x400000, CRC(d176ad0d) SHA1(2be755dfa3f60379d396734809bbaaaad49e0db5) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP( "778a08.7s",    0x000000, 0x080000, CRC(6259b4bf) SHA1(d0c38870495c9a07984b4b85e736d6477dd44832) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "778a09.16p",   0x000000, 0x400000, CRC(e8c6fd93) SHA1(dd378b67b3b7dd932e4b39fbf4321e706522247f) )
	ROM_LOAD( "778a10.14p",   0x400000, 0x400000, CRC(c6a0857b) SHA1(976734ba56460fcc090619fbba043a3d888c4f4e) )
	ROM_LOAD( "778a11.12p",   0x800000, 0x400000, CRC(40199382) SHA1(bee268adf9b6634a4f6bb39278ecd02f2bdcb1f4) )
	ROM_LOAD( "778a12.9p",    0xc00000, 0x400000, CRC(27d0c724) SHA1(48e48cbaea6db0de8c3471a2eda6faaa16eed46e) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, BAD_DUMP CRC(3cff1b1d) SHA1(bed0fc657a785be0c69bb21ad52365635c83d751) ) // hand built

	ROM_REGION( 0x0000224, "security_eeprom", 0 )
	ROM_LOAD( "security_eeprom_eab",  0x000000, 0x000224, BAD_DUMP CRC(7ebc2518) SHA1(e906ef6d32b35be801bbba8f450910e9bf75876f) ) // hand built
ROM_END

ROM_START(nbaatwa)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD16_WORD_SWAP( "778a01.27p",   0x200000, 0x200000, CRC(e70019ce) SHA1(8b187b6e670fdc88771da08a56685cd621b139dc) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP( "778a04.16t",   0x000000, 0x400000, CRC(62c70132) SHA1(405aed149fc51e0adfa3ace3c644e47d53cf1ee3) )
	ROM_LOAD32_WORD_SWAP( "778a05.14t",   0x000002, 0x400000, CRC(03249803) SHA1(f632a5f1dfa0a8500407214df0ec8d98ce09bc2b) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", 0)  // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "778a14.32u",   0x000002, 0x400000, CRC(db0c278d) SHA1(bb9884b6cdcdb707fff7e56e92e2ede062abcfd3) )
	ROM_LOAD32_WORD_SWAP( "778a13.24u",   0x000000, 0x400000, CRC(47fda9cc) SHA1(4aae01c1f1861b4b12a3f9de6b39eb4d11a9736b) )
	ROM_LOAD32_WORD_SWAP( "778a16.32v",   0x800002, 0x400000, CRC(6c0f46ea) SHA1(c6b9fbe14e13114a91a5925a0b46496260539687) )
	ROM_LOAD32_WORD_SWAP( "778a15.24v",   0x800000, 0x400000, CRC(d176ad0d) SHA1(2be755dfa3f60379d396734809bbaaaad49e0db5) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP( "778a08.7s",    0x000000, 0x080000, CRC(6259b4bf) SHA1(d0c38870495c9a07984b4b85e736d6477dd44832) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "778a09.16p",   0x000000, 0x400000, CRC(e8c6fd93) SHA1(dd378b67b3b7dd932e4b39fbf4321e706522247f) )
	ROM_LOAD( "778a10.14p",   0x400000, 0x400000, CRC(c6a0857b) SHA1(976734ba56460fcc090619fbba043a3d888c4f4e) )
	ROM_LOAD( "778a11.12p",   0x800000, 0x400000, CRC(40199382) SHA1(bee268adf9b6634a4f6bb39278ecd02f2bdcb1f4) )
	ROM_LOAD( "778a12.9p",    0xc00000, 0x400000, CRC(27d0c724) SHA1(48e48cbaea6db0de8c3471a2eda6faaa16eed46e) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, BAD_DUMP CRC(3cff1b1d) SHA1(bed0fc657a785be0c69bb21ad52365635c83d751) ) // hand built

	ROM_REGION( 0x0000224, "security_eeprom", 0 )
	ROM_LOAD( "security_eeprom_eaa",  0x000000, 0x000224, BAD_DUMP CRC(7edf223c) SHA1(9af4fd4f042d65edb1294e76e02ab41881f1ee28) ) // hand built
ROM_END

ROM_START(terabrst)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD32_WORD_SWAP( "715l02.25p",   0x000000, 0x200000, CRC(79586f19) SHA1(8dcfed5d101ebe49d958a7a38d5472323f75dd1d) )
	ROM_LOAD32_WORD_SWAP( "715l03.22p",   0x000002, 0x200000, CRC(c193021e) SHA1(c934b7c4bdab0ceff0f1699fcf2fb7d90e2e8962) )

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP( "715a04.16t",   0x000000, 0x200000, CRC(00d9567e) SHA1(fe372399ad0ae89d557c93c3145b38e3ed0f714d) )
	ROM_LOAD32_WORD_SWAP( "715a05.14t",   0x000002, 0x200000, CRC(462d53bf) SHA1(0216a84358571de6791365c69a1fa8fe2784148d) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", ROMREGION_ERASE00)  // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "715a14.32u",   0x000002, 0x400000, CRC(bbb36be3) SHA1(c828d0af0546db02e87afe68423b9447db7c7e51) )
	ROM_LOAD32_WORD_SWAP( "715a13.24u",   0x000000, 0x400000, CRC(dbff58a1) SHA1(f0c60bb2cbf268cfcbdd65606ebb18f1b4839c0e) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP( "715a08.7s",    0x000000, 0x080000, CRC(3aa2f4a5) SHA1(bb43e5f5ef4ac51f228d4d825be66d3c720d51ea) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "715a09.16p",   0x000000, 0x400000, CRC(65845866) SHA1(d2a63d0deef1901e6fa21b55c5f96e1f781dceda) )
	ROM_LOAD( "715a10.14p",   0x400000, 0x400000, CRC(294fe71b) SHA1(ac5fff5627df1cee4f1e1867377f208b34334899) )

	ROM_REGION(0x20000, "gn680", 0)     // 68K Program
	ROM_LOAD16_WORD_SWAP( "715a17.20k",    0x000000, 0x020000, CRC(f0b7ba0c) SHA1(863b260824b0ae2f890ba84d1c9a8f436891b1ff) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "715uel_m48t58y.35d", 0x000000, 0x002000, CRC(57322db4) SHA1(59cb8cd6ab446bf8781e3dddf902a4ff2484068e) )

	ROM_REGION( 0x0000224, "security_eeprom", 0 )
	ROM_LOAD( "security_eeprom", 0x000000, 0x000224, NO_DUMP ) // Unused?
ROM_END

ROM_START(terabrstj)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD32_WORD_SWAP( "715l02.25p",   0x000000, 0x200000, CRC(79586f19) SHA1(8dcfed5d101ebe49d958a7a38d5472323f75dd1d) )
	ROM_LOAD32_WORD_SWAP( "715l03.22p",   0x000002, 0x200000, CRC(c193021e) SHA1(c934b7c4bdab0ceff0f1699fcf2fb7d90e2e8962) )

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP( "715a04.16t",   0x000000, 0x200000, CRC(00d9567e) SHA1(fe372399ad0ae89d557c93c3145b38e3ed0f714d) )
	ROM_LOAD32_WORD_SWAP( "715a05.14t",   0x000002, 0x200000, CRC(462d53bf) SHA1(0216a84358571de6791365c69a1fa8fe2784148d) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", ROMREGION_ERASE00)  // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "715a14.32u",   0x000002, 0x400000, CRC(bbb36be3) SHA1(c828d0af0546db02e87afe68423b9447db7c7e51) )
	ROM_LOAD32_WORD_SWAP( "715a13.24u",   0x000000, 0x400000, CRC(dbff58a1) SHA1(f0c60bb2cbf268cfcbdd65606ebb18f1b4839c0e) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP( "715a08.7s",    0x000000, 0x080000, CRC(3aa2f4a5) SHA1(bb43e5f5ef4ac51f228d4d825be66d3c720d51ea) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "715a09.16p",   0x000000, 0x400000, CRC(65845866) SHA1(d2a63d0deef1901e6fa21b55c5f96e1f781dceda) )
	ROM_LOAD( "715a10.14p",   0x400000, 0x400000, CRC(294fe71b) SHA1(ac5fff5627df1cee4f1e1867377f208b34334899) )

	ROM_REGION(0x20000, "gn680", 0)     // 68K Program
	ROM_LOAD16_WORD_SWAP( "715a17.20k",    0x000000, 0x020000, CRC(f0b7ba0c) SHA1(863b260824b0ae2f890ba84d1c9a8f436891b1ff) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_jel", 0x000000, 0x002000, BAD_DUMP CRC(bcf8610f) SHA1(b52e4ca707cf36f16fb3ba29a8a8f5dc4a42be7b) ) // hand built

	ROM_REGION( 0x0000224, "security_eeprom", 0 )
	ROM_LOAD( "security_eeprom", 0x000000, 0x000224, NO_DUMP ) // Unused?
ROM_END

ROM_START(terabrsta)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD32_WORD_SWAP( "715l02.25p",   0x000000, 0x200000, CRC(79586f19) SHA1(8dcfed5d101ebe49d958a7a38d5472323f75dd1d) )
	ROM_LOAD32_WORD_SWAP( "715l03.22p",   0x000002, 0x200000, CRC(c193021e) SHA1(c934b7c4bdab0ceff0f1699fcf2fb7d90e2e8962) )

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP( "715a04.16t",   0x000000, 0x200000, CRC(00d9567e) SHA1(fe372399ad0ae89d557c93c3145b38e3ed0f714d) )
	ROM_LOAD32_WORD_SWAP( "715a05.14t",   0x000002, 0x200000, CRC(462d53bf) SHA1(0216a84358571de6791365c69a1fa8fe2784148d) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", ROMREGION_ERASE00)  // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "715a14.32u",   0x000002, 0x400000, CRC(bbb36be3) SHA1(c828d0af0546db02e87afe68423b9447db7c7e51) )
	ROM_LOAD32_WORD_SWAP( "715a13.24u",   0x000000, 0x400000, CRC(dbff58a1) SHA1(f0c60bb2cbf268cfcbdd65606ebb18f1b4839c0e) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP( "715a08.7s",    0x000000, 0x080000, CRC(3aa2f4a5) SHA1(bb43e5f5ef4ac51f228d4d825be66d3c720d51ea) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "715a09.16p",   0x000000, 0x400000, CRC(65845866) SHA1(d2a63d0deef1901e6fa21b55c5f96e1f781dceda) )
	ROM_LOAD( "715a10.14p",   0x400000, 0x400000, CRC(294fe71b) SHA1(ac5fff5627df1cee4f1e1867377f208b34334899) )

	ROM_REGION(0x20000, "gn680", 0)     // 68K Program
	ROM_LOAD16_WORD_SWAP( "715a17.20k",    0x000000, 0x020000, CRC(f0b7ba0c) SHA1(863b260824b0ae2f890ba84d1c9a8f436891b1ff) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_hel", 0x000000, 0x002000, BAD_DUMP CRC(1bf1278d) SHA1(40d437eb7428a42c0d8eb47cbcebc95ff8dc1767) ) // hand built

	ROM_REGION( 0x0000224, "security_eeprom", 0 )
	ROM_LOAD( "security_eeprom", 0x000000, 0x000224, NO_DUMP ) // Unused?
ROM_END

ROM_START(terabrstua)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD32_WORD_SWAP( "715a02.25p",   0x000000, 0x200000, CRC(070c48b3) SHA1(066cefbd34d8f6476083417471114f782bef97fb) )
	ROM_LOAD32_WORD_SWAP( "715a03.22p",   0x000002, 0x200000, CRC(f77d242f) SHA1(7680e4abcccd549b3f6d1d245f64631fab57e80d) )

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP( "715a04.16t",   0x000000, 0x200000, CRC(00d9567e) SHA1(fe372399ad0ae89d557c93c3145b38e3ed0f714d) )
	ROM_LOAD32_WORD_SWAP( "715a05.14t",   0x000002, 0x200000, CRC(462d53bf) SHA1(0216a84358571de6791365c69a1fa8fe2784148d) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", ROMREGION_ERASE00)  // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "715a14.32u",   0x000002, 0x400000, CRC(bbb36be3) SHA1(c828d0af0546db02e87afe68423b9447db7c7e51) )
	ROM_LOAD32_WORD_SWAP( "715a13.24u",   0x000000, 0x400000, CRC(dbff58a1) SHA1(f0c60bb2cbf268cfcbdd65606ebb18f1b4839c0e) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP( "715a08.7s",    0x000000, 0x080000, CRC(3aa2f4a5) SHA1(bb43e5f5ef4ac51f228d4d825be66d3c720d51ea) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "715a09.16p",   0x000000, 0x400000, CRC(65845866) SHA1(d2a63d0deef1901e6fa21b55c5f96e1f781dceda) )
	ROM_LOAD( "715a10.14p",   0x400000, 0x400000, CRC(294fe71b) SHA1(ac5fff5627df1cee4f1e1867377f208b34334899) )

	ROM_REGION(0x20000, "gn680", 0)     // 68K Program
	ROM_LOAD16_WORD_SWAP( "715a17.20k",    0x000000, 0x020000, CRC(f0b7ba0c) SHA1(863b260824b0ae2f890ba84d1c9a8f436891b1ff) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_uaa", 0x000000, 0x002000, BAD_DUMP CRC(60509b6a) SHA1(5938587770bdf5569c8b4c7413967869bddfcf84) ) // hand built

	ROM_REGION( 0x0000224, "security_eeprom", 0 )
	ROM_LOAD( "security_eeprom", 0x000000, 0x000224, NO_DUMP ) // Unused?
ROM_END

ROM_START(terabrstja)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD32_WORD_SWAP( "715a02.25p",   0x000000, 0x200000, CRC(070c48b3) SHA1(066cefbd34d8f6476083417471114f782bef97fb) )
	ROM_LOAD32_WORD_SWAP( "715a03.22p",   0x000002, 0x200000, CRC(f77d242f) SHA1(7680e4abcccd549b3f6d1d245f64631fab57e80d) )

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP( "715a04.16t",   0x000000, 0x200000, CRC(00d9567e) SHA1(fe372399ad0ae89d557c93c3145b38e3ed0f714d) )
	ROM_LOAD32_WORD_SWAP( "715a05.14t",   0x000002, 0x200000, CRC(462d53bf) SHA1(0216a84358571de6791365c69a1fa8fe2784148d) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", ROMREGION_ERASE00)  // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "715a14.32u",   0x000002, 0x400000, CRC(bbb36be3) SHA1(c828d0af0546db02e87afe68423b9447db7c7e51) )
	ROM_LOAD32_WORD_SWAP( "715a13.24u",   0x000000, 0x400000, CRC(dbff58a1) SHA1(f0c60bb2cbf268cfcbdd65606ebb18f1b4839c0e) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP( "715a08.7s",    0x000000, 0x080000, CRC(3aa2f4a5) SHA1(bb43e5f5ef4ac51f228d4d825be66d3c720d51ea) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "715a09.16p",   0x000000, 0x400000, CRC(65845866) SHA1(d2a63d0deef1901e6fa21b55c5f96e1f781dceda) )
	ROM_LOAD( "715a10.14p",   0x400000, 0x400000, CRC(294fe71b) SHA1(ac5fff5627df1cee4f1e1867377f208b34334899) )

	ROM_REGION(0x20000, "gn680", 0)     // 68K Program
	ROM_LOAD16_WORD_SWAP( "715a17.20k",    0x000000, 0x020000, CRC(f0b7ba0c) SHA1(863b260824b0ae2f890ba84d1c9a8f436891b1ff) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_jaa", 0x000000, 0x002000, BAD_DUMP CRC(ac54bdf9) SHA1(0139d29db112f9581a94091c2fac008e5c9f855d) ) // hand built

	ROM_REGION( 0x0000224, "security_eeprom", 0 )
	ROM_LOAD( "security_eeprom", 0x000000, 0x000224, NO_DUMP ) // Unused?
ROM_END

ROM_START(terabrstaa)
	ROM_REGION32_BE(0x400000, "prgrom", 0)   // PowerPC program
	ROM_LOAD32_WORD_SWAP( "715a02.25p",   0x000000, 0x200000, CRC(070c48b3) SHA1(066cefbd34d8f6476083417471114f782bef97fb) )
	ROM_LOAD32_WORD_SWAP( "715a03.22p",   0x000002, 0x200000, CRC(f77d242f) SHA1(7680e4abcccd549b3f6d1d245f64631fab57e80d) )

	ROM_REGION32_BE(0x800000, "datarom", 0)   // Data roms
	ROM_LOAD32_WORD_SWAP( "715a04.16t",   0x000000, 0x200000, CRC(00d9567e) SHA1(fe372399ad0ae89d557c93c3145b38e3ed0f714d) )
	ROM_LOAD32_WORD_SWAP( "715a05.14t",   0x000002, 0x200000, CRC(462d53bf) SHA1(0216a84358571de6791365c69a1fa8fe2784148d) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", ROMREGION_ERASE00)  // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "715a14.32u",   0x000002, 0x400000, CRC(bbb36be3) SHA1(c828d0af0546db02e87afe68423b9447db7c7e51) )
	ROM_LOAD32_WORD_SWAP( "715a13.24u",   0x000000, 0x400000, CRC(dbff58a1) SHA1(f0c60bb2cbf268cfcbdd65606ebb18f1b4839c0e) )

	ROM_REGION(0x80000, "audiocpu", 0)      // 68K Program
	ROM_LOAD16_WORD_SWAP( "715a08.7s",    0x000000, 0x080000, CRC(3aa2f4a5) SHA1(bb43e5f5ef4ac51f228d4d825be66d3c720d51ea) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0)       // PCM sample roms
	ROM_LOAD( "715a09.16p",   0x000000, 0x400000, CRC(65845866) SHA1(d2a63d0deef1901e6fa21b55c5f96e1f781dceda) )
	ROM_LOAD( "715a10.14p",   0x400000, 0x400000, CRC(294fe71b) SHA1(ac5fff5627df1cee4f1e1867377f208b34334899) )

	ROM_REGION(0x20000, "gn680", 0)     // 68K Program
	ROM_LOAD16_WORD_SWAP( "715a17.20k",    0x000000, 0x020000, CRC(f0b7ba0c) SHA1(863b260824b0ae2f890ba84d1c9a8f436891b1ff) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "m48t58y-70pc1_haa", 0x000000, 0x002000, BAD_DUMP CRC(960b864e) SHA1(9f6d7b81689777b98c0e1b6ac41135604da48429) ) // hand built

	ROM_REGION( 0x0000224, "security_eeprom", 0 )
	ROM_LOAD( "security_eeprom", 0x000000, 0x000224, NO_DUMP ) // Unused?
ROM_END

ROM_START(thrilldgeu) // GE713UF sticker, does not have the chip at 2G since it uses the rev A network board
	ROM_REGION32_BE(0x400000, "prgrom", ROMREGION_ERASEFF) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("713ab01.27p", 0x200000, 0x200000, CRC(a005d728) SHA1(8e265b1bb3adb7db2d342d3c0e3361a7174cb54d) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", ROMREGION_ERASE00) // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "713a13.24u", 0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.32u", 0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "ge713uf_m48t58y.35d", 0x000000, 0x002000, CRC(39f521d4) SHA1(2a1a574eb5830d40f5db87464785a159a5ab252d) ) // hand built
ROM_END

ROM_START(thrilldgnj)
	ROM_REGION32_BE(0x400000, "prgrom", ROMREGION_ERASEFF) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("713ab01.27p", 0x200000, 0x200000, CRC(a005d728) SHA1(8e265b1bb3adb7db2d342d3c0e3361a7174cb54d) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", ROMREGION_ERASE00) // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "713a13.24u", 0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.32u", 0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gn713ja_m48t58y.35d", 0x000000, 0x002000, CRC(4496250d) SHA1(d433dc63afb3fd1c0ce772a62bec0e026d0e278c) ) // hand built
ROM_END

ROM_START(thrilldgmj)
	ROM_REGION32_BE(0x400000, "prgrom", ROMREGION_ERASEFF) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("713ab01.27p", 0x200000, 0x200000, CRC(a005d728) SHA1(8e265b1bb3adb7db2d342d3c0e3361a7174cb54d) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", ROMREGION_ERASE00) // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "713a13.24u", 0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.32u", 0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gm713ja_m48t58y.35d", 0x000000, 0x002000, CRC(96792a3f) SHA1(f05e9f83fc655da2b300e2a17b71a52b7933cfb2) ) // hand built
ROM_END

ROM_START(thrilldgpj)
	ROM_REGION32_BE(0x400000, "prgrom", ROMREGION_ERASEFF) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("713ab01.27p", 0x200000, 0x200000, CRC(a005d728) SHA1(8e265b1bb3adb7db2d342d3c0e3361a7174cb54d) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", ROMREGION_ERASE00) // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "713a13.24u", 0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.32u", 0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gp713ja_m48t58y.35d", 0x000000, 0x002000, CRC(2a5011ef) SHA1(9bac6d95d2f035ad1fd1b91810be198fa6e4c7ce) ) // hand built
ROM_END

ROM_START(thrilldgej)
	ROM_REGION32_BE(0x400000, "prgrom", ROMREGION_ERASEFF) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("713ab01.27p", 0x200000, 0x200000, CRC(a005d728) SHA1(8e265b1bb3adb7db2d342d3c0e3361a7174cb54d) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", ROMREGION_ERASE00) // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "713a13.24u", 0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.32u", 0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "ge713ja_m48t58y.35d", 0x000000, 0x002000, CRC(f484975b) SHA1(027097109a850d38376b79e5a5f844d357967f2c) ) // hand built
ROM_END

ROM_START(thrilldgke)
	ROM_REGION32_BE(0x400000, "prgrom", ROMREGION_ERASEFF) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("713ab01.27p", 0x200000, 0x200000, CRC(a005d728) SHA1(8e265b1bb3adb7db2d342d3c0e3361a7174cb54d) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", ROMREGION_ERASE00) // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "713a13.24u", 0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.32u", 0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gk713ea_m48t58y.35d", 0x000000, 0x002000, CRC(3e77a0b9) SHA1(fdbd0b72447cb8077ae614864452152223fd2b57) ) // hand built
ROM_END

ROM_START(thrilldgkee)
	ROM_REGION32_BE(0x400000, "prgrom", ROMREGION_ERASEFF) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("713ab01.27p", 0x200000, 0x200000, CRC(a005d728) SHA1(8e265b1bb3adb7db2d342d3c0e3361a7174cb54d) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", ROMREGION_ERASE00) // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "713a13.24u", 0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.32u", 0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gk713ee_m48t58y.35d", 0x000000, 0x002000, CRC(ac6a7648) SHA1(73d06b219c2d6859a565d1b0d3dea79268cc3795) ) // hand built
ROM_END

ROM_START(thrilldgkk)
	ROM_REGION32_BE(0x400000, "prgrom", ROMREGION_ERASEFF) // PowerPC program roms
	ROM_LOAD16_WORD_SWAP("713ab01.27p", 0x200000, 0x200000, CRC(a005d728) SHA1(8e265b1bb3adb7db2d342d3c0e3361a7174cb54d) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "datarom", 0) // Data roms
	ROM_LOAD32_WORD_SWAP("713a04.16t", 0x000000, 0x200000, CRC(c994aaa8) SHA1(d82b9930a11e5384ad583684a27c95beec03cd5a) )
	ROM_LOAD32_WORD_SWAP("713a05.14t", 0x000002, 0x200000, CRC(6f1e6802) SHA1(91f8a170327e9b4ee6a64aee0c106b981a317e69) )

	ROM_REGION32_BE(0x1000000, "cgboard_0", ROMREGION_ERASE00) // CG Board texture roms
	ROM_LOAD32_WORD_SWAP( "713a13.24u", 0x000000, 0x400000, CRC(b795c66b) SHA1(6e50de0d5cc444ffaa0fec7ada8c07f643374bb2) )
	ROM_LOAD32_WORD_SWAP( "713a14.32u", 0x000002, 0x400000, CRC(5275a629) SHA1(16fadef06975f0f3625cac8f84e2e77ed7d75e15) )

	ROM_REGION(0x80000, "audiocpu", 0) // 68k program roms
	ROM_LOAD16_WORD_SWAP( "713a08.7s", 0x000000, 0x080000, CRC(6a72a825) SHA1(abeac99c5343efacabcb0cdff6d34f9f967024db) )

	ROM_REGION16_LE(0x1000000, "rfsnd", 0) // PCM sample roms
	ROM_LOAD( "713a09.16p", 0x000000, 0x400000, CRC(058f250a) SHA1(63b8e60004ec49009633e86b4992c00083def9a8) )
	ROM_LOAD( "713a10.14p", 0x400000, 0x400000, CRC(27f9833e) SHA1(1540f00d2571ecb81b914c553682b67fca94bbbd) )

	ROM_REGION(0x2000, "m48t58", 0)
	ROM_LOAD( "gk713ka_m48t58y.35d", 0x000000, 0x002000, CRC(688f5164) SHA1(0617e2e2a0078854e2344412f8c4fbee22452ba1) ) // hand built
ROM_END


} // Anonymous namespace


/*************************************************************************/

GAME(  1998, gradius4,   0,        hornet_x76, gradius4, hornet_state, init_gradius4, ROT0, "Konami", "Gradius IV: Fukkatsu (ver JAC)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, gradius4u,  gradius4, hornet_x76, gradius4, hornet_state, init_gradius4, ROT0, "Konami", "Gradius IV (ver UAC)",           MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, gradius4a,  gradius4, hornet_x76, gradius4, hornet_state, init_gradius4, ROT0, "Konami", "Gradius IV (ver AAC)",           MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, gradius4ja, gradius4, hornet_x76, gradius4, hornet_state, init_gradius4, ROT0, "Konami", "Gradius IV: Fukkatsu (ver JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, gradius4ua, gradius4, hornet_x76, gradius4, hornet_state, init_gradius4, ROT0, "Konami", "Gradius IV (ver UAA)",           MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, gradius4aa, gradius4, hornet_x76, gradius4, hornet_state, init_gradius4, ROT0, "Konami", "Gradius IV (ver AAA)",           MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME(  1998, nbapbp,   0,      nbapbp, nbapbp, hornet_state, init_hornet, ROT0, "Konami", "NBA Play By Play (ver UAB)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, nbapbpa,  nbapbp, nbapbp, nbapbp, hornet_state, init_hornet, ROT0, "Konami", "NBA Play By Play (ver AAB)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, nbapbpj,  nbapbp, nbapbp, nbapbp, hornet_state, init_hornet, ROT0, "Konami", "NBA Play By Play (ver JAB)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, nbapbpua, nbapbp, nbapbp, nbapbp, hornet_state, init_hornet, ROT0, "Konami", "NBA Play By Play (ver UAA)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, nbapbpaa, nbapbp, nbapbp, nbapbp, hornet_state, init_hornet, ROT0, "Konami", "NBA Play By Play (ver AAA)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, nbapbpja, nbapbp, nbapbp, nbapbp, hornet_state, init_hornet, ROT0, "Konami", "NBA Play By Play (ver JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, nbaatw,   nbapbp, nbapbp, nbapbp, hornet_state, init_hornet, ROT0, "Konami", "NBA All The Way (ver EAB)",  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, nbaatwa,  nbapbp, nbapbp, nbapbp, hornet_state, init_hornet, ROT0, "Konami", "NBA All The Way (ver EAA)",  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME(  1998, terabrst,   0,        terabrst,   terabrst, terabrst_state, init_hornet, ROT0, "Konami", "Teraburst (1998/07/17 ver UEL)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, terabrstj,  terabrst, terabrst,   terabrst, terabrst_state, init_hornet, ROT0, "Konami", "Teraburst (1998/07/17 ver JEL)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, terabrsta,  terabrst, terabrst,   terabrst, terabrst_state, init_hornet, ROT0, "Konami", "Teraburst (1998/07/17 ver HEL)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
// A revision set won't boot due to issues with the cgboard/konppc.
// All instances of the hanging I can find involve the 0x780c0003 register not returning how the game expected (checks against bit 7 and/or bit 6 and loops while non-zero, some kind of state?).
// You can patch the following values to get the game to boot with poor performance.
// For manual patching you can breakpoint on 800060c8 and then make changes in memory when breakpoint is hit to get around the checksum check issues.
// 80008a70: 40820090 -> 38600000
// 80002540: 4082fff8 -> 81810048
// 80040a88: 4082ffe8 -> 38603e80
GAME(  1998, terabrstua, terabrst, terabrst,   terabrst, terabrst_state, init_hornet, ROT0, "Konami", "Teraburst (1998/02/25 ver UAA)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, terabrstja, terabrst, terabrst,   terabrst, terabrst_state, init_hornet, ROT0, "Konami", "Teraburst (1998/02/25 ver JAA)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, terabrstaa, terabrst, terabrst,   terabrst, terabrst_state, init_hornet, ROT0, "Konami", "Teraburst (1998/02/25 ver HAA)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

// identifies as NWK-LC system
// heavy GFX glitches, fails wheel motor test, for now it's possible to get in game by switching "SW:2" to on
// notable version differences:
// GN713JA handbrake, 5+R shifter, no clutch
// GM713JA handbrake, up/down shifter, no clutch
// GP713JA no handbrake, up/down shifter, no clutch
// GE713JA handbrake, 5+R shifter, clutch
// GE713UF no handbrake, no clutch. settings configurable on boot: brake pedal, shifter (up/down, 4 pos, 5+R), steering motor type (A, W, H types)
// GK713EA no clutch. settings configurable on boot: handbrake lever, shifter (up/down, 4 pos, 5+R), shifter display position (right/left)
// GK713EE no clutch. settings configurable on boot: handbrake lever, shifter (up/down, 4 pos, 5+R), shifter display position (right/left)
// GK713K* no handbrake, up/down shifter, no clutch, English only version of GP713JA, supposed Korean release
GAME(  1998, thrilldgeu,  thrilld,  hornet_lan, thrilld,    hornet_state, init_hornet, ROT0, "Konami", "Thrill Drive (ver GE713UFB)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, thrilldgnj,  thrilld,  hornet_lan, thrilld_gn, hornet_state, init_hornet, ROT0, "Konami", "Thrill Drive (ver GN713JAB)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, thrilldgmj,  thrilld,  hornet_lan, thrilld_gm, hornet_state, init_hornet, ROT0, "Konami", "Thrill Drive (ver GM713JAB)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, thrilldgpj,  thrilld,  hornet_lan, thrilld_gp, hornet_state, init_hornet, ROT0, "Konami", "Thrill Drive (ver GP713JAB)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, thrilldgej,  thrilld,  hornet_lan, thrilld_ge, hornet_state, init_hornet, ROT0, "Konami", "Thrill Drive (ver GE713JAB)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, thrilldgke,  thrilld,  hornet_lan, thrilld_gk, hornet_state, init_hornet, ROT0, "Konami", "Thrill Drive (ver GK713EAB)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, thrilldgkee, thrilld,  hornet_lan, thrilld_gk, hornet_state, init_hornet, ROT0, "Konami", "Thrill Drive (ver GK713EEB)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1998, thrilldgkk,  thrilld,  hornet_lan, thrilld_gp, hornet_state, init_hornet, ROT0, "Konami", "Thrill Drive (ver GK713K*B)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

// Revisions C and D removed Japanese region support but introduced Voodoo 2 support.
GAMEL( 1999, sscope,   0,      sscope, sscope, hornet_state, init_sscope, ROT0, "Konami", "Silent Scope (ver UAD, Ver 1.33)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 1999, sscopee,  sscope, sscope, sscope, hornet_state, init_sscope, ROT0, "Konami", "Silent Scope (ver EAD, Ver 1.33)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 1999, sscopea,  sscope, sscope, sscope, hornet_state, init_sscope, ROT0, "Konami", "Silent Scope (ver AAD, Ver 1.33)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 1999, sscopeuc, sscope, sscope, sscope, hornet_state, init_sscope, ROT0, "Konami", "Silent Scope (ver UAC, Ver 1.30)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 1999, sscopeec, sscope, sscope, sscope, hornet_state, init_sscope, ROT0, "Konami", "Silent Scope (ver EAC, Ver 1.30)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 1999, sscopeac, sscope, sscope, sscope, hornet_state, init_sscope, ROT0, "Konami", "Silent Scope (ver AAC, Ver 1.30)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 1999, sscopeub, sscope, sscope, sscope, hornet_state, init_sscope, ROT0, "Konami", "Silent Scope (ver UAB, Ver 1.20)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 1999, sscopeeb, sscope, sscope, sscope, hornet_state, init_sscope, ROT0, "Konami", "Silent Scope (ver EAB, Ver 1.20)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 1999, sscopeab, sscope, sscope, sscope, hornet_state, init_sscope, ROT0, "Konami", "Silent Scope (ver AAB, Ver 1.20)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 1999, sscopejb, sscope, sscope, sscope, hornet_state, init_sscope, ROT0, "Konami", "Silent Scope (ver JAB, Ver 1.20)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 1999, sscopeua, sscope, sscope, sscope, hornet_state, init_sscope, ROT0, "Konami", "Silent Scope (ver UAA, Ver 1.00)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 1999, sscopeea, sscope, sscope, sscope, hornet_state, init_sscope, ROT0, "Konami", "Silent Scope (ver EAA, Ver 1.00)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 1999, sscopeaa, sscope, sscope, sscope, hornet_state, init_sscope, ROT0, "Konami", "Silent Scope (ver AAA, Ver 1.00)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 1999, sscopeja, sscope, sscope, sscope, hornet_state, init_sscope, ROT0, "Konami", "Silent Scope (ver JAA, Ver 1.00)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
// This version of Silent Scope runs on GQ871 video boards (Voodoo 2 instead of Voodoo 1). Only revisions C and D of the available program ROMs have support for Voodoo 2.
GAMEL( 1999, sscopevd2,   sscope, sscope_voodoo2, sscope, hornet_state, init_sscope, ROT0, "Konami", "Silent Scope (ver UAD, Ver 1.33, GQ871 Voodoo 2 video board)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 1999, sscopeevd2,  sscope, sscope_voodoo2, sscope, hornet_state, init_sscope, ROT0, "Konami", "Silent Scope (ver EAD, Ver 1.33, GQ871 Voodoo 2 video board)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 1999, sscopeavd2,  sscope, sscope_voodoo2, sscope, hornet_state, init_sscope, ROT0, "Konami", "Silent Scope (ver AAD, Ver 1.33, GQ871 Voodoo 2 video board)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 1999, sscopeucvd2, sscope, sscope_voodoo2, sscope, hornet_state, init_sscope, ROT0, "Konami", "Silent Scope (ver UAC, Ver 1.30, GQ871 Voodoo 2 video board)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 1999, sscopeecvd2, sscope, sscope_voodoo2, sscope, hornet_state, init_sscope, ROT0, "Konami", "Silent Scope (ver EAC, Ver 1.30, GQ871 Voodoo 2 video board)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 1999, sscopeacvd2, sscope, sscope_voodoo2, sscope, hornet_state, init_sscope, ROT0, "Konami", "Silent Scope (ver AAC, Ver 1.30, GQ871 Voodoo 2 video board)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_dualhsxs )

GAMEL( 2000, sscope2,   0,       sscope2, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Dark Silhouette (ver UAD, Ver 1.03)",  MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2e,  sscope2, sscope2, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Fatal Judgement (ver EAD, Ver 1.03)",  MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2j,  sscope2, sscope2, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Innocent Sweeper (ver JAD, Ver 1.03)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2a,  sscope2, sscope2, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Innocent Sweeper (ver AAD, Ver 1.03)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2uc, sscope2, sscope2, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Dark Silhouette (ver UAC, Ver 1.02)",  MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2ec, sscope2, sscope2, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Fatal Judgement (ver EAC, Ver 1.02)",  MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2jc, sscope2, sscope2, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Innocent Sweeper (ver JAC, Ver 1.02)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2ac, sscope2, sscope2, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Innocent Sweeper (ver AAC, Ver 1.02)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2ub, sscope2, sscope2, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Dark Silhouette (ver UAB, Ver 1.01)",  MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2eb, sscope2, sscope2, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Fatal Judgement (ver EAB, Ver 1.01)",  MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2jb, sscope2, sscope2, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Innocent Sweeper (ver JAB, Ver 1.01)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2ab, sscope2, sscope2, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Innocent Sweeper (ver AAB, Ver 1.01)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
// These versions of Silent Scope 2 run on GN715 video boards (Voodoo 1 instead of Voodoo 2)
GAMEL( 2000, sscope2vd1,   sscope2, sscope2_voodoo1, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Dark Silhouette (ver UAD, Ver 1.03, GN715 Voodoo 1 video board)",  MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2evd1,  sscope2, sscope2_voodoo1, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Fatal Judgement (ver EAD, Ver 1.03, GN715 Voodoo 1 video board)",  MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2jvd1,  sscope2, sscope2_voodoo1, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Innocent Sweeper (ver JAD, Ver 1.03, GN715 Voodoo 1 video board)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2avd1,  sscope2, sscope2_voodoo1, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Innocent Sweeper (ver AAD, Ver 1.03, GN715 Voodoo 1 video board)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2ucvd1, sscope2, sscope2_voodoo1, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Dark Silhouette (ver UAC, Ver 1.02, GN715 Voodoo 1 video board)",  MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2ecvd1, sscope2, sscope2_voodoo1, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Fatal Judgement (ver EAC, Ver 1.02, GN715 Voodoo 1 video board)",  MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2jcvd1, sscope2, sscope2_voodoo1, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Innocent Sweeper (ver JAC, Ver 1.02, GN715 Voodoo 1 video board)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2acvd1, sscope2, sscope2_voodoo1, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Innocent Sweeper (ver AAC, Ver 1.02, GN715 Voodoo 1 video board)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2ubvd1, sscope2, sscope2_voodoo1, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Dark Silhouette (ver UAB, Ver 1.01, GN715 Voodoo 1 video board)",  MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2ebvd1, sscope2, sscope2_voodoo1, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Fatal Judgement (ver EAB, Ver 1.01, GN715 Voodoo 1 video board)",  MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2jbvd1, sscope2, sscope2_voodoo1, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Innocent Sweeper (ver JAB, Ver 1.01, GN715 Voodoo 1 video board)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
GAMEL( 2000, sscope2abvd1, sscope2, sscope2_voodoo1, sscope2, sscope2_state, init_sscope2, ROT0, "Konami", "Silent Scope 2 : Innocent Sweeper (ver AAB, Ver 1.01, GN715 Voodoo 1 video board)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN, layout_dualhsxs )
