// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

    Nintendo Super System

    driver by Angelo Salese, based off info from Noca$h

    TODO:
    - EEPROM doesn't save?
    - Fix sound CPU halt / reset lines, particularly needed by this to work
      correctly;
    - Fix continue behaviour, might be the same issue as the one above.
    - Various M50458 bits
    - OSD should actually super-impose with the SNES video somehow;

    Notes:
    - Multi-Cart BIOS works only with F-Zero, Super Tennis and Super Mario
      World;

***************************************************************************

Nintendo Super System Hardware Overview
Nintendo, 1991/1992

This system is basically a Super Nintendo with a timer.
The main board has 3 slots on it and can accept up to 3 plug-in carts. The player
can choose to play any of the available games, although I'm not sure why anyone
would have wanted to pay money to play these games when the home SNES was selling
well and most kids had one.... possibly one of the reasons it failed in the arcades.
The control panel was also just some SNES pads mounted into the arcade machine
control panel.... not very usable as-is, very cheaply presented and probably
another reason why it failed in the arcades.

PCB Layouts
-----------
NSS-01-CPU MADE IN JAPAN
(C) 1991 Nintendo
|----------------------------------------------------|
|           HA13001             AN5836               |
|     SL4          SL2  CL2          SL3             |
|-|                CL1  SL1                          |
  |         |--------------|                         |
|-|        14.31818MHz     |         SL5             |
|           |    M50458    |                         |
|   IR3P32A |              |          |-|  |-|  |-|  |
|J          | VC1          |          | |  | |  | |  |
|A          |              |          | |  | |  | |  |
|M          |     CN6      |          | |  | |  | |  |
|M          |-------------21.4772MHz  | |  | |  | |  |
|A             |-------|   |-------|  | |  | |  | |  |
|      84256   |S-PPU1 |   |S-CPU  |  | |  | |  | |  |
|              |5C77-01|   |5A22-02|  | |  | |  | |  |
|-|            |-------|   |-------|  | |  | |  | |  |
  |                                   | |  | |  | |  |
|-|            |-------|   |-------|  | |  | |  | |  |
|      84256   |S-PPU2 |   |S-WRAM |  | |  | |  | |  |
|CN4           |5C78-01|   |LH68120|  | |  | |  | |  |
|              |-------|   |-------|  | |  | |  | |  |
|                                     | |  | |  | |  |
|                                     | |  | |  | |  |
|CN3                                  | |  | |  | |  |
|                                     | |  | |  | |  |
|                             4MHz    |-|  |-|  |-|  |
|                                    CN11 CN12  CN13 |
|                             Z84C0006               |
|CN5                                      LH5168     |
|                                                    |
|                M6M80011                            |
|                                                    |
|                               S-3520               |
|CN2                       32.678kHz   *MM1026       |
|                               5.5V    NSS-C_IC14_02|
|----------------------------------------------------|
Notes:
      The main board has many surface mounted logic chips and transistors on the solder side of the PCB
      which are not documented here. The parts side comprises mainly RAM and custom Nintendo ICs

      IR3P32A       - Sharp IR3P32A Special Function TV Interface Circuit, Conversion of color diff sig. & lumin. to RGB (NDIP30)
      M50458        - Mitsubishi M50458-001SP On-Screen Display (OSD) Chip (NDIP32). Clock signal 14.31818MHz on pins 28 and 29
      VC1           - Potentiometer connected to M50458. Turning the pot changes the horizontal width of the OSD menu towards the right
                      side only. The left edge of the OSD menu does not move. So this is a right horizontal stretch adjustment pot.
      HA13001       - Hitachi Dual 5.5W Power Amplifier IC
      AN5836        - Matsushita AN5836 DC Volume and Tone Control IC (SIL12)
      84256         - Fujitsu MB84256-10L 32k x8 SRAM (SOP28)
      LH5168        - Sharp LH5168N-10L 8k x8 SRAM (SOP28)
      Z84C0006      - Zilog Z84C0006FEC Z80 CPU, clock input 4.000MHz (QFP44)
      M6M80011      - Mitsubishi M6M80011 64 x16 Serial EEPROM (DIP8). Pinout..... 1 CS, 2 CLK, 3 DATA IN, 4 DATA OUT, 5 VSS, 6 RESET, 7 RDY, 8 VCC
      S-3520        - Seiko Epson S-3520 Real Time Clock (SOIC14)
      5.5V          - 5.5 volt supercap
      MM1026        - Mitsumi Monolithic MM1026BF System Reset IC with Battery Backup (SOIC8)
                      * This IC is located on the solder side of the PCB
      VSync         - 60.0980Hz
      HSync         - 15.3844kHz
      NSS-C_IC14_02 - 27C256 EPROM (BIOS, DIP28)
      CN11/12/13    - 50 pin connectors for game carts
      CN2           - 10 pin connector
      CN3           - 13 pin connector
      CN4           - 8 pin connector
      CN5           - 7 pin connector
      CN6           - 24 pin connector for plug in custom sound module
      Custom IC's   - S-CPU (QFP100)
                      S-PPU1 (QFP100)
                      S-PPU2 (QFP100)
                      S-WRAM (SOP64)
      SL* / CL*       - Jumper pads
                      SL1 open
                      SL2 open
                      SL3 open
                      SL4 open
                      SL5 shorted
                      CL1 shorted
                      CL2 shorted

Custom Sound Module (plugs into CN6)
------------------------------------
Note - This board is encased in a metal shield which is soldered together.

MITSUMI ELEC CO. LTD.
(C) 1990 Nintendo Co. Ltd.
|-----------------------|
|  CN1                  |
|            JRC2904    |---|
|  |-------|                |
|  |S-SMP  |                |
|  |       |  D6376         |
|  |-------|         51832  |
|                           |
|  |-------|                |
|  |S-DSP  |                |
|  |       |     51832      |
|  |-------|                |
|---|                       |
    |                       |
    |-----------------------|
Notes:
      Note: Without this PCB, the board will boot up and work, displaying the game
      selection menu, but once the game tries to load attract mode, the PCB resets.

      JRC2904 - Japan Radio Co. JRC2904 Dual Low Power Op Amp (SOIC8)
      D6376   - NEC D6376 Audio 2-Channel 16-Bit D/A Converter (SOIC16)
      51832   - Toshiba TC51832FL-12 32k x8 SRAM (SOP28)
      CN1     - 24 pin connector to plug in custom sound module to main board
      S-SMP   - Stamped 'Nintendo S-SMP (M) SONY (C) Nintendo '89' custom sound chip (QFP80)
      S-DSP   - Stamped 'Nintendo S-DSP (M) (C) SONY '89' custom sound DSP (QFP80)

Game Carts
----------
There are 3 types of carts. The carts have only a few components on them, including some
ROMs/sockets, a few logic chips/resistors/caps, a DIPSW8 block, one unknown DIP8 chip
(with it's surface scratched) and some solder-jumper pads to config the ROM types.
The unknown DIP8 chip is different per game also. There is a sticker on top with a 2-digit
game code (I.E. MW/AT/L3 etc). The unknown DIP8 chip is used for protection.

NSS-01-ROM-A
|-------------------------------------------------------|
|     SL3  CL1                                          |
|     CL3  SL1  IC1_PRG_ROM     IC3_INST_ROM       IC4  |
|CL4  SL2  SL5                                          |
|SL4  CL2  CL5                                          |
|--|                                                 |--|
   |-------------------------------------------------|
Notes:
      IC1 - Program ROM
      IC2 - Instruction ROM
      IC4 - Unknown DIP8 chip

Game Name            IC1             IC1 Type    IC3            IC3 Type    Jumpers
-------------------------------------------------------------------------------------------------------
Super Mario World    NSS-MW-0_PRG    LH534J      NSS-R_IC3_MW   27C256      CL1 CL2 CL3 CL4 CL5 - Short
                                                                            SL1 SL2 SL3 SL4 SL5 - Open
Super Mario World    NSS-R__IC1__MW  ?LH534J?    NSS-R_IC3_MW   27C256      CL1 CL2 CL3 CL4 CL5 - Short
                                                                            SL1 SL2 SL3 SL4 SL5 - Open
Super Tennis         NSS-ST-0        LH534J      NSS-R_IC3_ST   27C256      CL1 CL2 CL3 CL4 CL5 - Short
                                                                            SL1 SL2 SL3 SL4 SL5 - Open
Super Soccer         NSS-R__IC1__FS  TC574000    NSS-R_IC3_FS   27C256      CL1 CL2 CL3 CL4 CL5 - Open
                                                                            SL1 SL2 SL3 SL4 SL5 - Short
-------------------------------------------------------------------------------------------------------
Note - By setting the jumpers to 'Super Soccer', the other 2 games can use standard EPROMs if required.

LH534J is a 512k x 8-bit (4MBit) MaskROM with a non-standard pinout.
An adapter can be made easily to read them as a 27C040.

  Sharp LH534J           Common 27C040
    +--\/--+               +--\/--+
A17 |1   32| +5V       VPP |1   32| +5V
A18 |2   31| /OE       A16 |2   31| A18
A15 |3   30| NC        A15 |3   30| A17
A12 |4   29| A14       A12 |4   29| A14
 A7 |5   28| A13       A7  |5   28| A13
 A6 |6   27| A8        A6  |6   27| A8
 A5 |7   26| A9        A5  |7   26| A9
 A4 |8   25| A11       A4  |8   25| A11
 A3 |9   24| A16       A3  |9   24| OE/
 A2 |10  23| A10       A2  |10  23| A10
 A1 |11  22| /CE       A1  |11  22| CE/,PGM/
 A0 |12  21| D7        A0  |12  21| D7
 D0 |13  20| D6        D0  |13  20| D6
 D1 |14  19| D5        D1  |14  19| D5
 D2 |15  18| D4        D2  |15  18| D4
GND |16  17| D3        GND |16  17| D3
    +------+               +------+

NSS-01-ROM-B
|-------------------------------------------------------|
|  BAT1  SL3 CL1 CL2 SL5 CL6 CL7                        |
|        CL3 SL1 SL2 CL5 SL6 SL7                        |
|                 CL4 SL4                               |
|                   IC1                                 |
|                                                       |
|                                                       |
|                   IC2_PRG_ROM      IC7_INST_ROM  IC9  |
|                                                       |
|                                                       |
|--|                                                 |--|
   |-------------------------------------------------|
Notes:
      Battery is populated on this board, type CR2032 3V coin battery
      IC1 - LH5168 8k x8 SRAM (DIP28)
      IC2 - Program ROM
      IC7 - Instruction ROM
      IC9 - Unknown DIP8 chip

Game Name            IC2             IC2 Type    IC7            IC7 Type    Jumpers
---------------------------------------------------------------------------------------------------------------
F-Zero               NSS-FZ-0        LH534J      NSS-R_IC3_FZ   27C256      CL1 CL2 CL3 CL4 CL5 CL6 CL7 - Short
                                                                            SL1 SL2 SL3 SL4 SL5 SL6 SL7 - Open
---------------------------------------------------------------------------------------------------------------

NSS-01-ROM-C
|-------------------------------------------------------|
|  BAT1 CL19 SL22   IC1_SRAM                    DIPSW8  |
|       CL18 SL21                                       |
|       CL17 SL20                                       |
|       CL15 SL16   IC2_PRG_ROM-1                       |
|       SL12                                            |
|       CL13 SL14                                       |
|       CL5  SL11                                       |
|       CL6  SL10   IC3_PRG_ROM-0    IC8_INST_ROM  IC10 |
|       CL3  SL9                                        |
|       CL4  SL8                                        |
|   SL1 CL2  SL7                                        |
|--|                                                 |--|
   |-------------------------------------------------|
Notes:
      Battery is not populated on this board for any games
      IC1   - 6116 2k x8 SRAM, not populated (DIP24)
      IC2/3 - Program ROM
      IC8   - Instruction ROM
      IC10  - Unknown DIP8 chip

Game Name             IC2            IC2 Type   IC3            IC3 Type    IC8            IC8 Type    Jumpers
---------------------------------------------------------------------------------------------------------------------------------------------------------
Actraiser             NSS-R_IC2_AR   TC574000   NSS-R_IC3_AR   TC574000    NSS-R_IC8_AR   27C256      CL2 CL3 CL4 CL5 CL6 CL12 CL13 CL15 CL17 CL18 CL19 - Short
                                                                                                SL1 SL7 SL8 SL9 SL10 SL11 SL12 SL14 SL16 SL20 SL21 SL22 - Open
Addams Family         NSS-R_IC2_AF   TC574000   NSS-R_IC3_AF   TC574000    NSS-R_IC8_AF   27C256      All games use the above jumper configuration.
Amazing Tennis        NSS-R_IC2_AT   TC574000   NSS-R_IC3_AT   TC574000    NSS-R_IC8_AT   27C256
Irem Skins Game       NSS-R_IC2_MT   TC574000   NSS-R_IC3_MT   TC574000    NSS-R_IC8_MT   27C256
Lethal Weapon         NSS-R_IC2_L3   TC574000   NSS-R_IC3_L3   TC574000    NSS-R_IC8_L3   27C256
NCAA Basketball       NSS-R_IC2_DU   TC574000   NSS-R_IC3_DU   TC574000    NSS-R_IC8_DU   27C256
Robocop 3             NSS-R_IC2_R3   TC574000   NSS-R_IC3_R3   TC574000    NSS-R_IC8_R3   27C256
Super Mario All-Stars NSS-R_IC2_4M   HN62318    NSS-R_IC3_4M   HN62318     NSS-R_IC8_4M   27C256
---------------------------------------------------------------------------------------------------------------------------------------------------------

NSS-01-ROM-C
Sticker - NSS-X1-ROM-C (this is just a ROM-C board with a sticker over the top)
                       (the differences being the SRAM and battery are populated)
|-------------------------------------------------------|
|  BAT1 CL19 SL22   IC1_SRAM                    DIPSW8  |
|       CL18 SL21                                       |
|       CL17 SL20                                       |
|       CL15 SL16   IC2_PRG_ROM-1                       |
|       SL12                                            |
|       CL13 SL14                                       |
|       CL5  SL11                                       |
|       CL6  SL10   IC3_PRG_ROM-0    IC8_INST_ROM  IC10 |
|       CL3  SL9                                        |
|       CL4  SL8                                        |
|   SL1 CL2  SL7                                        |
|--|                                                 |--|
   |-------------------------------------------------|
Notes:
      Battery is populated on this board, type CR2032 3V coin battery
      IC1   - 6116 2k x8 SRAM, populated (DIP24)
      IC2/3 - Program ROM
      IC8   - Instruction ROM
      IC10  - Unknown DIP8 chip

Game Name    IC2            IC2 Type   IC3            IC3 Type    IC8           IC8 Type    Jumpers
-----------------------------------------------------------------------------------------------------------------------------------------------------
Contra III   CONTRA_III_1   TC574000   CONTRA_III_0   TC574000    GAME1_NSSU    27C256      CL2 CL3 CL4 CL5 CL6 CL12 CL13 CL15 CL17 CL18 CL19 - Short
                                                                                            SL1 SL7 SL8 SL9 SL10 SL11 SL12 SL14 SL16 SL20 SL21 SL22 - Open
-----------------------------------------------------------------------------------------------------------------------------------------------------

***************************************************************************/

#include "emu.h"
#include "snes.h"

#include "cpu/z80/z80.h"
#include "machine/m6m80011ap.h"
#include "machine/s3520cf.h"
#include "machine/rp5h01.h"
#include "video/m50458.h"
#include "emupal.h"
#include "layout/generic.h"
#include "speaker.h"


namespace {

class nss_state : public snes_state
{
public:
	nss_state(const machine_config &mconfig, device_type type, const char *tag)
		: snes_state(mconfig, type, tag)
		, m_bioscpu(*this, "bios")
		, m_m50458(*this, "m50458")
		, m_s3520cf(*this, "s3520cf")
		, m_rp5h01(*this, "rp5h01")
		, m_wram_wp(*this, "wram_wp")
		, m_eepromout(*this, "EEPROMOUT")
		, m_serial1_data1(*this, "SERIAL1_DATA1")
		, m_rtc_osd(*this, "RTC_OSD")
	{ }

	void nss(machine_config &config);

	void init_nss();

	int game_over_flag_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_bioscpu;
	required_device<m50458_device> m_m50458;
	required_device<s3520cf_device> m_s3520cf;
	required_device<rp5h01_device> m_rp5h01;
	required_shared_ptr<uint8_t> m_wram_wp;
	required_ioport m_eepromout;
	required_ioport m_serial1_data1;
	required_ioport m_rtc_osd;

	uint8_t m_wram_wp_flag = 0;
	bool m_nmi_enable = false;
	uint8_t m_cart_sel = 0;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void ram_wp_w(offs_t offset, uint8_t data);
	uint8_t nss_prot_r();
	void nss_prot_w(uint8_t data);

	uint8_t port_00_r();
	void port_00_w(uint8_t data);
	void port_01_w(uint8_t data);
	void port_02_w(uint8_t data);
	void port_03_w(uint8_t data);
	void port_04_w(uint8_t data);
	void port_07_w(uint8_t data);

	void nss_vblank_irq(int state);
	void bios_io_map(address_map &map) ATTR_COLD;
	void bios_map(address_map &map) ATTR_COLD;
	void snes_map(address_map &map) ATTR_COLD;
	void spc_map(address_map &map) ATTR_COLD;
};



uint32_t nss_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	m_m50458->screen_update(screen,bitmap,cliprect);
	return 0;
}



void nss_state::snes_map(address_map &map)
{
	map(0x000000, 0x7dffff).rw(FUNC(nss_state::snes_r_bank1), FUNC(nss_state::snes_w_bank1));
	map(0x7e0000, 0x7fffff).ram().share("wram");                 /* 8KB Low RAM, 24KB High RAM, 96KB Expanded RAM */
	map(0x800000, 0xffffff).rw(FUNC(nss_state::snes_r_bank2), FUNC(nss_state::snes_w_bank2));    /* Mirror and ROM */
}

void nss_state::spc_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("aram");
}


/* NSS specific */
/*
Notes of interest:

nss_smw

bp 2914 onward is a crc check with the Instruction ROM
c0fe - c0ff pointers to the checksum

bp 6b9e EEPROM write
bp 6bf9 EEPROM read
8080 - 8081 EEPROM checksummed value

8700 - 870c  EEPROM2 checksummed value (0x8707 value and'ed with 0x03)
bp 6f8d check the EEPROM2 results
870d EEPROM2 result of checksum

New notes:

0x3a7 is the protection check

0x8000 work RAM is for cart 1
0x8100 work RAM is for cart 2
0x8200 work RAM is for cart 3

*/
/*
noca$h info @ nocash.emubase.de/fullsnes.htm
map
0x0000 - 0x7fff BIOS
0x8000 - 0x8fff RAM
0x9000 - 0x9fff RAM with write protection
0xa000          EEPROM Read
0xc000 - 0xdfff instruction ROM
0xe000          EEPROM Write
0xe000 - 0xffff PROM Input & Output & Program Code (protection RP5H01, used also in earlier Nintendo systems)
Data Write:
  7-5  Unknown/unused
  4    PROM Test Mode (0=Low=6bit Address, 1=High=7bit Address)
  3    PROM Clock     (0=Low, 1=High) ;increment address on 1-to-0 transition
  2-1  Unknown/unused
  0    PROM Address Reset (0=High=Reset to zero, 1=Low=No Change)

Data Read and Opcode Fetch:

  7-5  Always set (MSBs of RST Opcode)
  4    PROM Counter Out (0=High=One, 1=Low=Zero) ;PROM Address Bit5
  3    PROM Data Out    (0=High=One, 1=Low=Zero)
  2-0  Always set (LSBs of RST Opcode)

i/o
Input
0x00 Joypad
0x01 Front-Panel Buttons and Game Over Flag
  7   From SNES Port 4016h.W.Bit2 (0=Game Over Flag, 1=Normal) (Inverted!)
0x02 Coin and Service Button Inputs
  7-3 Unknown/unused (maybe the (unused) Test button hides here)
  2   Service Button (1=Pressed: Add Credit; with INST button: Config)
  1   Coin Input 2   (1=Coin inserted in coin-slot 2)
  0   Coin Input 1   (1=Coin inserted in coin-slot 1)
0x03 RTC
Output
0x00/0x80 NMI Control and RAM protect
0x01/0x81 Unknown and Slot Select
0x02/0x82 RTC and OSD
0x03/0x83 Unknown and LED control
0x84 Coin Counter Outputs
0x05 Unknown
0x07 SNES Watchdog / Acknowledge SNES Joypad Read Flag

SNES part:
0x4100 DSW
0x4016 bit 0 Joypad Strobe?
0x4016 bit 2 Game Over Flag

*/

void nss_state::ram_wp_w(offs_t offset, uint8_t data)
{
	if (m_wram_wp_flag)
		m_wram_wp[offset] = data;
}

uint8_t nss_state::nss_prot_r()
{
	int data = 0xe7;

	if (m_cart_sel == 0)
	{
		data |= ((~m_rp5h01->counter_r()) << 4) & 0x10;  /* D4 */
		data |= (m_rp5h01->data_r() << 3) & 0x08;        /* D3 */
	}

	return data;
}

void nss_state::nss_prot_w(uint8_t data)
{
	if (m_cart_sel == 0)
	{
		m_rp5h01->test_w(data & 0x10);     /* D4 */
		m_rp5h01->clock_w(data & 0x08);    /* D3 */
		m_rp5h01->cs_w(~data & 0x01);
	}

	m_eepromout->write(data, 0xff);
}


void nss_state::bios_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram();
	map(0x9000, 0x9fff).ram().w(FUNC(nss_state::ram_wp_w)).share("wram_wp");
	map(0xa000, 0xa000).portr("EEPROMIN");
	map(0xc000, 0xdfff).rom().region("ibios_rom", 0x6000);
	map(0xe000, 0xffff).rw(FUNC(nss_state::nss_prot_r), FUNC(nss_state::nss_prot_w));
}

uint8_t nss_state::port_00_r()
{
/*
    x--- ----   SNES Watchdog (0=SNES did read Joypads, 1=Didn't do so) (ack via 07h.W)
    -x-- ----   Vblank or Vsync or so       (0=What, 1=What?)
    --x- ----   Button "Joypad Button B?"   (0=Released, 1=Pressed)
    ---x ----   Button "Joypad Button A"    (0=Released, 1=Pressed)
    ---- x---   Button "Joypad Down"        (0=Released, 1=Pressed)
    ---- -x--   Button "Joypad Up"          (0=Released, 1=Pressed)
    ---- --x-   Button "Joypad Left"        (0=Released, 1=Pressed)
    ---- ---x   Button "Joypad Right"       (0=Released, 1=Pressed)
*/
	uint8_t res;

	res = (m_joy_flag) << 7;
	// TODO: reads from SNES screen output, correct?
	res|= (m_screen->vblank() & 1) << 6;
	res|= (BIT(m_serial1_data1->read(), 15) << 5);
	res|= (BIT(m_serial1_data1->read(),  7) << 4);
	res|= (BIT(m_serial1_data1->read(), 10) << 3);
	res|= (BIT(m_serial1_data1->read(), 11) << 2);
	res|= (BIT(m_serial1_data1->read(),  9) << 1);
	res|= (BIT(m_serial1_data1->read(),  8) << 0);

	return res;
}

void nss_state::port_00_w(uint8_t data)
{
/*
    xxxx ---- Unknown/unused      (should be always 0)
    ---- x--- Maybe SNES CPU/PPU reset (usually same as Port 01h.W.Bit1)
    ---- -x-- RAM at 9000h-9FFFh  (0=Disable/Protect, 1=Enable/Unlock)
    ---- --x- Looks like maybe somehow NMI Related ?    ;\or one of these is PC10-style
    ---- ---x Looks like NMI Enable                     ;/hardware-watchdog reload?
*/
	m_wram_wp_flag = (data & 4) >> 2;
	m_nmi_enable = data & 1;
	if (!m_nmi_enable)
		m_bioscpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

}

void nss_state::port_01_w(uint8_t data)
{
/*
    x--- ---- Maybe SNES Joypad Enable? (0=Disable/Demo, 1=Enable/Game)
    -x-- ---- Unknown/unused        (should be always 0)
    --x- ---- SNES Sound Mute       (0=Normal, 1=Mute) (for optional mute in demo mode)
    ---x ---- Unknown  ;from INST-ROM flag! (Lo/HiROM, 2-player, zapper, volume or so?)
    ---- xx-- Slot Select        (0..2 for Slot 1..3) (mapping to both SNES and Z80)
    ---- --x- Maybe SNES CPU pause?  (cleared on deposit coin to continue) (1=Run)
    ---- ---x Maybe SNES CPU/PPU reset?   (0=Reset, 1=Run)
*/
	m_input_disabled = BIT(data, 7) ^ 1;
	m_s_dsp->set_volume((data & 0x20) ? 0.0 : 100.0);

	m_cart_sel = (data & 0xc) >> 2;

	m_maincpu->set_input_line(INPUT_LINE_HALT, (data & 2) ? CLEAR_LINE : ASSERT_LINE);
	m_soundcpu->set_input_line(INPUT_LINE_HALT, (data & 2) ? CLEAR_LINE : ASSERT_LINE);
	m_maincpu->set_input_line(INPUT_LINE_RESET, (data & 1) ? CLEAR_LINE : ASSERT_LINE);
	m_soundcpu->set_input_line(INPUT_LINE_RESET, (data & 1) ? CLEAR_LINE : ASSERT_LINE);
	/* also reset the device */
	if (!(data & 1))
		m_s_dsp->reset();
}

void nss_state::port_02_w(uint8_t data)
{
/*
    x--- ----  OSD Clock ?       (usually same as Bit6)  ;\Chip Select when Bit6=Bit7 ?
    -x-- ----  OSD Clock ?       (usually same as Bit7)  ;/
    --x- ----  OSD Data Out      (0=Low=Zero, 1=High=One)
    ---x ----  OSD Special       (?)  ... or just /CS ? (or software index DC3F/DD3F?)
    ---- x---  RTC /CLK          (0=Low=Clock,  1=High=Idle)              ;S-3520
    ---- -x--  RTC Data Out      (0=Low=Zero,   1=High=One)
    ---- --x-  RTC Direction     (0=Low=Write,  1=High=Read)
    ---- ---x  RTC /CS           (0=Low/Select, 1=High/No)
*/
//  printf("%02x\n",data & 0xf);
	m_rtc_osd->write(data, 0xff);
}

void nss_state::port_03_w(uint8_t data)
{
/*
    x--- ----     Layer SNES Enable?             (used by token proc, see 7A46h) SNES?
    -x-- ----     Layer OSD Enable?
    --xx ---- Unknown/unused (should be always 0)
    ---- x---   LED Instructions (0=Off, 1=On)  ;-glows in demo (prompt for INST button)
    ---- -x--   LED Game 3       (0=Off, 1=On)  ;\
    ---- --x-   LED Game 2       (0=Off, 1=On)  ; blinked when enough credits inserted
    ---- ---x   LED Game 1       (0=Off, 1=On)  ;/

*/
//  popmessage("%02x",data);
}

void nss_state::port_04_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, (data >> 0) & 1);
	machine().bookkeeping().coin_counter_w(1, (data >> 1) & 1);
}

void nss_state::port_07_w(uint8_t data)
{
	m_joy_flag = 1;
}

void nss_state::bios_io_map(address_map &map)
{
	map.global_mask(0x7);
	map(0x00, 0x00).r(FUNC(nss_state::port_00_r)).w(FUNC(nss_state::port_00_w));
	map(0x01, 0x01).portr("FP").w(FUNC(nss_state::port_01_w));
	map(0x02, 0x02).portr("SYSTEM").w(FUNC(nss_state::port_02_w));
	map(0x03, 0x03).portr("RTC").w(FUNC(nss_state::port_03_w));
	map(0x04, 0x04).w(FUNC(nss_state::port_04_w));
	map(0x07, 0x07).w(FUNC(nss_state::port_07_w));
}

void nss_state::machine_start()
{
	snes_state::machine_start();

	m_is_nss = 1;

	save_item(NAME(m_wram_wp_flag));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_cart_sel));
}


int nss_state::game_over_flag_r()
{
	return m_game_over_flag;
}

static INPUT_PORTS_START( snes )
	PORT_START("SYSTEM")
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("FP")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(nss_state::game_over_flag_r))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON13 ) PORT_NAME("Restart Button")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_NAME("Page Up Button")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("Page Down Button")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Instructions Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Game 3 Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Game 2 Button")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Game 1 Button")

	PORT_START("EEPROMIN")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("m6m80011ap", FUNC(m6m80011ap_device::read_bit))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("m6m80011ap", FUNC(m6m80011ap_device::ready_line))
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("EEPROMOUT")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("m6m80011ap", FUNC(m6m80011ap_device::set_clock_line))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH,IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("m6m80011ap", FUNC(m6m80011ap_device::write_bit))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("m6m80011ap", FUNC(m6m80011ap_device::set_cs_line))

	PORT_START("RTC_OSD")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("m50458", FUNC(m50458_device::set_clock_line))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("m50458", FUNC(m50458_device::write_bit))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("m50458", FUNC(m50458_device::set_cs_line))
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("s3520cf", FUNC(s3520cf_device::set_clock_line))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("s3520cf", FUNC(s3520cf_device::write_bit))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("s3520cf", FUNC(s3520cf_device::set_dir_line))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("s3520cf", FUNC(s3520cf_device::set_cs_line))

	PORT_START("RTC")
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("s3520cf", FUNC(s3520cf_device::read_bit))

	PORT_START("SERIAL1_DATA1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Button B") PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Button Y") PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("P1 Select")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("P1 Start")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Button A") PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 Button X") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P1 Button L") PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Button R") PORT_PLAYER(1)
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SERIAL2_DATA1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Button B") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 Button Y") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("P2 Select")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("P2 Start")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Button A") PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P2 Button X") PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P2 Button L") PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Button R") PORT_PLAYER(2)
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SERIAL1_DATA2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SERIAL2_DATA2")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hard )  )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x00, "Time limit per level?" ) // taken from the scan of nss_adam
	PORT_DIPSETTING(    0x10, "104 sec." )
	PORT_DIPSETTING(    0x20, "112 sec." )
	PORT_DIPSETTING(    0x00, "120 sec." )
	PORT_DIPSETTING(    0x30, "? sec." )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

#if SNES_LAYER_DEBUG
	PORT_START("DEBUG1")
	PORT_CONFNAME( 0x03, 0x00, "Select BG1 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x01, "BG1B (lower) only" )
	PORT_CONFSETTING(    0x02, "BG1A (higher) only" )
	PORT_CONFNAME( 0x0c, 0x00, "Select BG2 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x04, "BG2B (lower) only" )
	PORT_CONFSETTING(    0x08, "BG2A (higher) only" )
	PORT_CONFNAME( 0x30, 0x00, "Select BG3 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x10, "BG3B (lower) only" )
	PORT_CONFSETTING(    0x20, "BG3A (higher) only" )
	PORT_CONFNAME( 0xc0, 0x00, "Select BG4 priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x40, "BG4B (lower) only" )
	PORT_CONFSETTING(    0x80, "BG4A (higher) only" )

	PORT_START("DEBUG2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 1") PORT_CODE(KEYCODE_1_PAD) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 2") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 3") PORT_CODE(KEYCODE_3_PAD) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle BG 4") PORT_CODE(KEYCODE_4_PAD) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Objects") PORT_CODE(KEYCODE_5_PAD) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Main/Sub") PORT_CODE(KEYCODE_6_PAD) PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Color Math") PORT_CODE(KEYCODE_7_PAD) PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Windows") PORT_CODE(KEYCODE_8_PAD) PORT_TOGGLE

	PORT_START("DEBUG3")
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mosaic") PORT_CODE(KEYCODE_9_PAD) PORT_TOGGLE
	PORT_CONFNAME( 0x70, 0x00, "Select OAM priority" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x10, "OAM0 only" )
	PORT_CONFSETTING(    0x20, "OAM1 only" )
	PORT_CONFSETTING(    0x30, "OAM2 only" )
	PORT_CONFSETTING(    0x40, "OAM3 only" )
	PORT_CONFNAME( 0x80, 0x00, "Draw sprite in reverse order" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DEBUG4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 0 draw") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 1 draw") PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 2 draw") PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 3 draw") PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 4 draw") PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 5 draw") PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 6 draw") PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Toggle Mode 7 draw") PORT_TOGGLE
#endif
INPUT_PORTS_END


void nss_state::nss_vblank_irq(int state)
{
	if (state && m_nmi_enable)
		m_bioscpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void nss_state::machine_reset()
{
	snes_state::machine_reset();

	/* start with both CPUs disabled */
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	/* reset the security chip */
	m_rp5h01->enable_w(1);
	m_rp5h01->enable_w(0);
	m_rp5h01->reset_w(0);
	m_rp5h01->reset_w(1);

	m_game_over_flag = 1;
	m_joy_flag = 1;
}

void nss_state::nss(machine_config &config)
{
	/* base snes hardware */
	_5A22(config, m_maincpu, MCLK_NTSC);   /* 2.68Mhz, also 3.58Mhz */
	m_maincpu->set_addrmap(AS_PROGRAM, &nss_state::snes_map);

	// runs at 24.576 MHz / 12 = 2.048 MHz
	S_SMP(config, m_soundcpu, XTAL(24'576'000) / 12);
	m_soundcpu->set_addrmap(AS_DATA, &nss_state::spc_map);
	m_soundcpu->dsp_io_read_callback().set(m_s_dsp, FUNC(s_dsp_device::dsp_io_r));
	m_soundcpu->dsp_io_write_callback().set(m_s_dsp, FUNC(s_dsp_device::dsp_io_w));

	config.set_perfect_quantum(m_maincpu);

	/* nss hardware */
	Z80(config, m_bioscpu, 4000000);
	m_bioscpu->set_addrmap(AS_PROGRAM, &nss_state::bios_map);
	m_bioscpu->set_addrmap(AS_IO, &nss_state::bios_io_map);

	M50458(config, m_m50458, 4000000, "osd"); /* TODO: correct clock */
	S3520CF(config, m_s3520cf); /* RTC */
	RP5H01(config, m_rp5h01, 0);
	M6M80011AP(config, "m6m80011ap");

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	S_DSP(config, m_s_dsp, XTAL(24'576'000) / 12);
	m_s_dsp->set_addrmap(0, &nss_state::spc_map);
	m_s_dsp->add_route(0, "lspeaker", 1.00);
	m_s_dsp->add_route(1, "rspeaker", 1.00);

	/* video hardware */
	/* TODO: the screen should actually superimpose, but for the time being let's just separate outputs */
	config.set_default_layout(layout_dualhsxs);

	// SNES PPU
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(DOTCLK_NTSC * 2, SNES_HTOTAL * 2, 0, SNES_SCR_WIDTH * 2, SNES_VTOTAL_NTSC, 0, SNES_SCR_HEIGHT_NTSC);
	m_screen->set_video_attributes(VIDEO_VARIABLE_WIDTH);
	m_screen->set_screen_update(FUNC(snes_state::screen_update));
	m_screen->screen_vblank().set(FUNC(nss_state::nss_vblank_irq));

	SNES_PPU(config, m_ppu, MCLK_NTSC);
	m_ppu->open_bus_callback().set([this] { return snes_open_bus_r(); }); // lambda because overloaded function name
	m_ppu->set_screen("screen");

	// NSS
	screen_device &osd(SCREEN(config, "osd", SCREEN_TYPE_RASTER));
	osd.set_refresh_hz(60);
	osd.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	osd.set_size(288+22, 216+22);
	osd.set_visarea(0, 288-1, 0, 216-1);
	osd.set_screen_update(FUNC(nss_state::screen_update));
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

#define NSS_BIOS \
	ROM_REGION(0x8000,         "bios",  0)      /* BIOS CPU */ \
	ROM_SYSTEM_BIOS( 0, "single", "Nintendo Super System (Single Cart BIOS)" ) \
	ROMX_LOAD("nss-ic14.02.ic14", 0x00000, 0x8000, CRC(e06cb58f) SHA1(62f507e91a2797919a78d627af53f029c7d81477), ROM_BIOS(0) )   /* BIOS */ \
	ROM_SYSTEM_BIOS( 1, "multi", "Nintendo Super System (Multi Cart BIOS)" ) \
	ROMX_LOAD("nss-c.ic14"  , 0x00000, 0x8000, CRC(a8e202b3) SHA1(b7afcfe4f5cf15df53452dc04be81929ced1efb2), ROM_BIOS(1) )   /* BIOS */ \
	ROM_SYSTEM_BIOS( 2, "single3", "Nintendo Super System (Single Cart BIOS v3, hack?)" ) \
	ROMX_LOAD("nss-v3.ic14" , 0x00000, 0x8000, CRC(ac385b53) SHA1(e3942f9d508c3c8074c3c3941376c37ca68b8e54), ROM_BIOS(2) )   /* BIOS */


ROM_START( nss )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", ROMREGION_ERASEFF )

	/* instruction / data rom for BIOS */
	ROM_REGION( 0x8000, "ibios_rom", ROMREGION_ERASEFF )
ROM_END

ROM_START( nss_actr )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "act-rais.ic3", 0x00000, 0x80000, CRC(c9f788c2) SHA1(fba2331fd5bcbe51d74115528fd3a9becf072e8d) )
	ROM_LOAD( "act-rais.ic2", 0x80000, 0x80000, CRC(4df9cc63) SHA1(3e98d9693d60d125a1257ba79701f27bda688261) )

	/* instruction / data rom for BIOS */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "act-rais.ic8", 0x0000, 0x8000, CRC(08b38ce6) SHA1(4cbb7fd28d98ffef0f17747201625883af954e3a) )

	ROM_REGION( 0x10, "rp5h01", 0 )
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(4b74ac55) SHA1(51ea71b06367b4956a4b737385e2d4d15bd43980) )
ROM_END

ROM_START( nss_con3 )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "contra3.ic3", 0x00000, 0x80000, CRC(33b03501) SHA1(c7f4835d5ec4983e487b00f0b4c49fede2f03b9c) )
	ROM_LOAD( "contra3.ic2", 0x80000, 0x80000, CRC(2f3e3b5b) SHA1(0186b92f022701f6ae29984252e6d346acf6550b) )

	/* instruction / data rom for BIOS */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "contra3.ic8", 0x0000, 0x8000, CRC(0fbfa23b) SHA1(e7a1a78a58c64297e7b9623350ec57aed8035a4f) )

	ROM_REGION( 0x10, "rp5h01", ROMREGION_ERASE00 )
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(e97e4b00) SHA1(70e8382a93137353f5d0b905db2e9af50c52ce0b) )
ROM_END

ROM_START( nss_adam )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "addams.ic3", 0x00000, 0x80000, CRC(44643930) SHA1(a45204b2eb13c6befca30d130061b5b8ba054270) )
	ROM_LOAD( "addams.ic2", 0x80000, 0x80000, CRC(6196adcf) SHA1(a450f278a37d5822f607aa3631831a461e8b147e) )

	/* instruction / data rom for BIOS */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "addams.ic8", 0x0000, 0x8000, CRC(57c7f72c) SHA1(2e3642b4b5438f6c535d6d1eb668e1663062cf78) )

	ROM_REGION( 0x10, "rp5h01", 0 )
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(154d10c2) SHA1(6829e149c341b753ee9bc72055c0634db4e81884) )
ROM_END

ROM_START( nss_aten )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "amtennis.ic3", 0x00000, 0x80000, CRC(aeabaf2a) SHA1(b355e0a322b57454e767785a49c14d4c7f492488) )
	ROM_LOAD( "amtennis.ic2", 0x80000, 0x80000, CRC(7738c5f2) SHA1(eb0089e9724c7b3834d9f6c47b92f5a1bb26fc77) )

	/* instruction / data rom for BIOS */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "amtennis.ic8", 0x0000, 0x8000, CRC(d2cd3926) SHA1(49fc253b1b9497ef1374c7db0bd72c163ffb07e7) )

	ROM_REGION( 0x10, "rp5h01", 0 )
	ROM_LOAD( "security.prm", 0x00, 0x10,CRC(3e640fa2) SHA1(ac530610a9d4979f070d5f57dfd4886c530aa20f) )
ROM_END

ROM_START( nss_rob3 )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "robocop3.ic3", 0x00000, 0x80000, CRC(60916c42) SHA1(462d9645210a58bfd5204bd209eae2cdadb4493e) )
	ROM_LOAD( "robocop3.ic2", 0x80000, 0x80000, CRC(a94e1b56) SHA1(7403d70504310ad5949a3b45b4a1e71e7d2bce77) )

	/* instruction / data rom for BIOS */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "robocop3.ic8", 0x0000, 0x8000, CRC(90d13c51) SHA1(6751dab14b7d178350ac333f07dd2c3852e4ae23) )

	ROM_REGION( 0x10, "rp5h01", 0 )
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(eb9a75de) SHA1(58f028c3f28eb4155215f4e154323e01e0fd4297) )
ROM_END

ROM_START( nss_ncaa )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "ncaa.ic3", 0x00000, 0x80000, CRC(ef49ad8c) SHA1(4c40f3746b995b53f006434b9ccec06d8fe16e1f) )
	ROM_LOAD( "ncaa.ic2", 0x80000, 0x80000, CRC(83ef6936) SHA1(8e0f38c763861e33684c6ddb742385b0522af78a) )

	/* instruction / data rom for BIOS */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "ncaa.ic8", 0x0000, 0x8000, CRC(b9fa28d5) SHA1(bc538bcff5c19eae4becc6582b5c111d287b76fa) )

	ROM_REGION( 0x10, "rp5h01", 0 )
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(a2e9ad5b) SHA1(a41f82451fc185f8e989a0d4f38700dc7813bb50) )
ROM_END

ROM_START( nss_skin )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "skins.ic3", 0x00000, 0x80000, CRC(ee1bb84d) SHA1(549ad9319e94a5d75cd4af017e63ea93ab407c87) )
	ROM_LOAD( "skins.ic2", 0x80000, 0x80000, CRC(365fd19e) SHA1(f60d7ac39fe83fb98730e73fbef410c90a4ff35b) )

	/* instruction / data rom for BIOS */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "skins.ic8", 0x0000, 0x8000, CRC(9f33d5ce) SHA1(4d279ad3665bd94c7ca9cb2778572bed42c5b298) )

	ROM_REGION( 0x10, "rp5h01", 0 )
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(86f8cd1d) SHA1(d567d194058568f4ae32b7726e433918b06bca54) )
ROM_END

ROM_START( nss_lwep )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "nss-lw.ic3", 0x00000, 0x80000, CRC(32564666) SHA1(bf371218fa303ce95eab09fb6017a522071dcd7e) )
	ROM_LOAD( "nss-lw.ic2", 0x80000, 0x80000, CRC(86365042) SHA1(f818024c6f858fd2780396b6c83d3a37a97fa08a) )

	/* instruction / data rom for BIOS */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "nss-lw.ic8", 0x0000, 0x8000, CRC(1acc1d5d) SHA1(4c8b100ac5847915aaf3b5bfbcb4f632606c97de) )

	ROM_REGION( 0x10, "rp5h01", 0 )
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(e9755c14) SHA1(d8dbebf3536dcbd18c50ba11a6b729dc7085f74b) )
ROM_END

ROM_START( nss_ssoc )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "s-soccer.ic1", 0x00000, 0x80000,  CRC(70b7f50e) SHA1(92856118528995e3a0b7d22340d440bef5fd61ac) )

	/* instruction / data rom for BIOS */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "s-soccer.ic3", 0x0000, 0x8000, CRC(c09211c3) SHA1(b274a57f93ae0a8774664df3d3615fb7dbecfa2e) )

	ROM_REGION( 0x10, "rp5h01", 0 )
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(e41c4204) SHA1(529ca7df78ecf154a095dc1b627783c43c817a45) )
ROM_END

ROM_START( nss_smw )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "nss-mw-0_prg.ic1", 0x000000, 0x80000, CRC(c46766f2) SHA1(06a6efc246c6fdb83efab1d402d61d2179a84494) )
	// Note: this rom appears with 2 variations: LH534J mask rom with "NSS-MW-0 // PRG" silkscreen
	// or ?LH534J mask rom? with "NSS-R // IC1 // MW" label sticker

	/* instruction / data rom for BIOS */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "nss-r__ic3__mw.ic3", 0x0000, 0x8000, CRC(f2c5466e) SHA1(e116f01342fcf359498ed8750741c139093b1fb2) )

	ROM_REGION( 0x10, "rp5h01", 0 )
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(fd700dca) SHA1(6805cefb1856c3498b7a7a049c0d09858afac47c) ) // Labeled "MW" on yellow sticker
ROM_END

ROM_START( nss_fzer )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "nss-fz-0.ic2", 0x000000, 0x100000, CRC(e9b3cdf1) SHA1(ab616eecd292b94ca74c55446bddd23e9dc3e3bb) )

	/* instruction / data rom for BIOS */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "fz.ic7", 0x0000, 0x8000, CRC(48ae570d) SHA1(934f9fec47dcf9e49936388968d2db50c69950da) )

	ROM_REGION( 0x10, "rp5h01", 0 )
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(9650a7d0) SHA1(59d57ab2720cff3a24105a7250560c41def45acc) )
ROM_END

ROM_START( nss_sten )
	NSS_BIOS
	ROM_REGION( 0x100000, "user3", 0 )
	ROM_LOAD( "nss-st-0.ic1", 0x000000, 0x100000, CRC(f131611f) SHA1(0797936e1fc9e705cd7e029097fc013a58e69002) )

	/* instruction / data rom for BIOS */
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "st.ic3", 0x0000, 0x8000, CRC(8880596e) SHA1(ec6d68fc2f51f7d94f496cd72cf898db65324542) )

	ROM_REGION( 0x10, "rp5h01", 0 )
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(2fd8475b) SHA1(38af97734649b90e0ea74cb1daeaa431e4295eb9) )
ROM_END

ROM_START( nss_smas )
	NSS_BIOS
	ROM_REGION( 0x200000, "user3", 0 ) // believed bad dumps as they are 0x00 filled in ranges 0x40000-0x7ffff and 0xc0000-0xfffff. Comparison with the SNES ROMs seems to confirm the suspects.
	ROM_LOAD( "nss-r ic3 4m.ic3", 0x000000, 0x100000, BAD_DUMP CRC(e071405a) SHA1(e2a4b849c225a637d1f7481f24d9162da29c4905) )
	ROM_LOAD( "nss-r ic3 4m.ic2", 0x100000, 0x100000, BAD_DUMP CRC(a46eed6e) SHA1(b9ac1f1d1f8aa5276238a65003c25a3916a4a0c2) )

	// instruction / data rom for BIOS
	ROM_REGION( 0x8000, "ibios_rom", 0 )
	ROM_LOAD( "nss-r ic3 4m.ic8", 0x0000, 0x8000, CRC(bb46c507) SHA1(e753043b70541a4298d0af21781ecc2640a4d554) )

	ROM_REGION( 0x10, "rp5h01", 0 )
	ROM_LOAD( "security.prm", 0x00, 0x10, NO_DUMP )
ROM_END

void nss_state::init_nss()
{
	uint8_t *PROM = memregion("rp5h01")->base();

	for (int i = 0; i < 0x10; i++)
		PROM[i] = bitswap<8>(PROM[i],0,1,2,3,4,5,6,7) ^ 0xff;

	init_snes();
}

} // Anonymous namespace


GAME( 199?, nss,       0,     nss,      snes, nss_state, init_snes, ROT0, "Nintendo",                    "Nintendo Super System BIOS", MACHINE_IS_BIOS_ROOT )
GAME( 1992, nss_actr,  nss,   nss,      snes, nss_state, init_nss,  ROT0, "Enix",                        "Act Raiser (Nintendo Super System)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
GAME( 1992, nss_adam,  nss,   nss,      snes, nss_state, init_nss,  ROT0, "Ocean",                       "The Addams Family (Nintendo Super System)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
GAME( 1992, nss_aten,  nss,   nss,      snes, nss_state, init_nss,  ROT0, "Absolute Entertainment Inc.", "David Crane's Amazing Tennis (Nintendo Super System)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1992, nss_con3,  nss,   nss,      snes, nss_state, init_nss,  ROT0, "Konami",                      "Contra 3: The Alien Wars (Nintendo Super System)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1992, nss_lwep,  nss,   nss,      snes, nss_state, init_nss,  ROT0, "Ocean",                       "Lethal Weapon (Nintendo Super System)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1992, nss_ncaa,  nss,   nss,      snes, nss_state, init_nss,  ROT0, "Sculptured Software Inc.",    "NCAA Basketball (Nintendo Super System)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1992, nss_rob3,  nss,   nss,      snes, nss_state, init_nss,  ROT0, "Ocean",                       "Robocop 3 (Nintendo Super System)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1992, nss_skin,  nss,   nss,      snes, nss_state, init_nss,  ROT0, "Irem",                        "Skins Game (Nintendo Super System)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // can't start
GAME( 1992, nss_ssoc,  nss,   nss,      snes, nss_state, init_nss,  ROT0, "Human Inc.",                  "Super Soccer (Nintendo Super System)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1991, nss_smw,   nss,   nss,      snes, nss_state, init_nss,  ROT0, "Nintendo",                    "Super Mario World (Nintendo Super System)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1991, nss_fzer,  nss,   nss,      snes, nss_state, init_nss,  ROT0, "Nintendo",                    "F-Zero (Nintendo Super System)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1991, nss_sten,  nss,   nss,      snes, nss_state, init_nss,  ROT0, "Nintendo",                    "Super Tennis (Nintendo Super System)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1993, nss_smas,  nss,   nss,      snes, nss_state, init_nss,  ROT0, "Nintendo",                    "Super Mario All-Stars (Nintendo Super System)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // bad dump
