// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************************************************

Optomax V image analyser, from AMS (Analytical Measuring Systems Ltd.).
The machine internally uses a CUBE EuroBEEB Single Board Computer (PCBs B6 and B7).

Back panel:
 - Camera input connector (DIN).
 - Camera video connector (3 x BNC).
 - VCR connector (BNC).
 - TGB monitor connector (DIN)
 - Printer connector (DB 26)
 - Data transfer serial connector (DB9).
 - Data transfer parallel connector (DB25).
 - Aux 1 connector (DIN).
 - Two sync mode switches (internal/external and interlace/non interlace).

Front panel:
 - Monitor (Philips M24-306GH/ED).
 - Control panel with the following layout:
 ______________________________________________________________________________________________
|           _____                                                    FIELD/                   |
|          /     \                     FRAME           TITLE          FEATURE         V (F1)  |
|         | DIAL  |        SET                                                                |
|         |       |       DETECTOR            CURRENT            INITIATE             W (F2)  |
|          \_____/                      EDIT          CALIBRATE         HOLD                  |
|                                                                                     X (F3)  |
|                                                                                             |
|     PREVIOUS    NEXT       A         B (7)   C (8)   D (9)     E (PRINT) F (ABORT)  Y (F4)  |
|                       (DETECT NULL)                                                         |
|             UP           G (HE)      H (4)   I (5)   J (6)     K (SPACE)     L      Z (F5)  |
|                                                                         (SEND DATA)         |
|      RIGHT HOME  LEFT    M (MR)      N (1)   O (2)   P (3)      Q (EXP)      R      / (F6)  |
|                                                                        (LOAD MATRIX)        |
|            DOWN           GREEN      S (0)   T (·)   U (DEL)     ENTER     GREEN    , (F7)  |
|_____________________________________________________________________________________________|


Six PCBs connected with two backplanes (Motorola VME 9 conns and a custom one with 13 conns):

OSD PCB (B6) (EURO-BEEB 65, CUBE EuroBEEB Single Board Computer PCB from Control Universal Ltd.)
 - Rockwell R6551AP ACIA
 - Rockwell R6522AP VIA
 - Toshiba TC5565PL SRAM
 - Xtal 1.8432
 - Hitachi HN613128PB05 Acorn BASIC Mask ROM (BASIC)
 - 2764 EPROM
 - M-3002-16PI Real Time Clock (RTC)
 - 2 x Signetics 82S147N PROMs
 - Rockwell R6502AP CPU
   __________________________________________
 _|                                          |_
| |__________________________________________| |
| Xtal                ________________________ |
| ___   ___________  | Rockwell              | |
||  |  |_82S147N__|  | R6502AP               | |
||  |   ___________  |_______________________| |
||  |  |_82S147N__|                            |
||_<-M-3002-16PI             _________________ |
|                           | EPROM          | |
|              __________   | M0             | |
|             |M74HC132B1   |________________| |
|              __________    _________________ |
|             |SN74S00N_|   | MASK (BASIC)   | |
|                           | M1             | |
|  _____                    |________________| |
| |    |                     _________________ |
| |BATT|                    | EMPTY          | |
| |2.4V|       __________   | M3             | |
| |____|      |_74F04N__|   |________________| |
| __________   __________    _________________ |
||AM26LS30PC  |AM26LS32PC   | TC5565PL-15    | |
|                  ..       |                | |
| Xtal 1.8432 MHz  ..       |________________| |
| _________________   ________________________ |
|| Rockwell       |  | Rockwell              | |
|| R6551AP        |  | R6522AP               | |
||________________|  |_______________________| |
|   _______      ___________________           |
|__| DIN  |_____||||||||||||||||||||___________|
   |______|


 TELETEXT PCB (B5) (CUBE TELETEXT Colour Video Interface from Control Universal Ltd.)
(this PCB lacks the buzzer present on other revisions)
 - Hitachi HD68B45SP CRT Controller
 - Xtal 6.000
 - 2 x NEC uPD2114LC SRAM
 - SAA 5050 Teletext Character Generator
 - Rockwell R6522-33 VIA
       _____________________
 _____|                     |__________________
|     |_____________________|                  |
|      __________     ____________________     |
|     |SN74LS174N    | Rockwell          |     |
|                    | R6522AP           |     |
|      __________    |___________________|     |
|     |_74LS86N_|      _________________       |
|                     |                |       |
|      __________     | SAA 5050       |       |
|     |_74LS04N_|     |________________|       |
|      __________   __________   __________    |
|     |_74LS04N_|  |_74LS92N_|  |uPD2114LC|    |
|        Xtal       __________   __________    |
|        6 MHz     |74LS245N_|  |uPD2114LC|    |
|      __________                              |
|     |_74LS00N_|     ____________________     |
|                    | Hitachi            |    |
|                    | HD46505SP-2        |    |
|                    |____________________|    |
|      __________   __________   __________    |
|     |74LS138N_|  |74LS157N_|  |74LS157N_|    |
|      __________   __________   __________    |
|     |74LS139N_|  |74LS136N_|  |74LS157N_|    |
|  __________________________________________  |
|_|                                          |_|
  |__________________________________________|


PCU PCB (PME 68-1B) (B4)
 - Signetics SCN68000 CPU
 - 16 x TMM41256P RAM
 - 2 x 27256 EPROM
 - 2 x 27128 EPROM
 - 3 x Hitachi HD46850P ACIA
 - AMPAL16L8 (labeled 0545)
 - PAL16L6C (labeled 0544)
 - 2 x PAL20L10C (labeled 0686 and 0620)
 - PAL12L10C (labeled 0546)
 - MM58167AN Real Time Clock (RTC)
 - MC68230L8 Parallel Interface / Timer 8MHz
 - Xtal 1.8432 MHz
 - MC14411P Bit Rate Generator
 - Xtal 32 MHz
      _____________________________________________________________________
     |    ________   _______________________________    __________        |
    _|_  |       |  |                              |   |SN74LS645-1N      |
 RES SW| |       |  |                              |    __________       _|_
    _|_  |       |  |  16 x TMM41256P-15           |   |SN74LS645-1N    |   |
 ABT SW| |       |  |                              |     ::::::         |   |
     |   |SCN68000  |______________________________|    __________      |   |
HTL LED  |       |  _______________  _______________   |SN74LS645-1N    |   |
    _|_  |       | | EPROM        | | EPROM        |    __________      |   |
   |   | |       | |              | |              |   |SN74LS645-1N    |   |
   |P3 | |       | |______________| |______________|    __________      |   |
   |DB25 |       |  _______________  _______________   |SN74LS645-1N    |   |
   |   | |       | | EPROM        | | EPROM        |    __________      |   |
   |___| |       | |              | |              |   |SN74LS645-1N    |   |
     |   |_______| |______________| |______________|    __________      |   |
    _|_  ..                           ::::::::         |SN74LS645-1N    |   |
   |   | ..   ____________   __________  ::::..         __________      |   |
   |P4 | ..  | HD46850P   | |SN74LS641_|  __________   |AMPAL16L8|      |   |
   |DB25 ..  |            | .. :::::::   |_74F74PC_|    __________      |___|
   |   | ..  |____________|                            |_74F241PC|        |
   |___| ..     __________   __________   __________    __________        |
     |         |_74S240N_|  |_PAL16L6_|  |PAL20l10_|   |SN74LS175N        |
    _|_  .. __________  __________  __________  __________  ___________   |
   |   | ..|_74S240N_| |SN74LS260N |SN74LS148N |SN74LS200N |MM58167AN |   |
   |P5 | .. __________  __________  __________  __________ |          |   |
   |SB25 ..|SN74LS393N |_74F74PC_| |SN74LS688N |_74F37N__| |__________|   |
   |   | .. ________    __________  :::::::::  ..... ::                  _|_
   |___| ..|SN74LS56P  |_74F241PC|       __________            .        |   |
     |      __________  __________      |PAL20L10_|            :        |   |
     |     |SN75188N_| |DM74LS01N|            ________________________  |   |
     |      ______  ______   __________      |                       |  |   |
     |     |     | |     |  |PAL12L10_|      | MC68230L8             |  |   |
     |     | 2 x HD46850P|   __________  ... |_______________________|  |   |
     |     |     | |     |  |SN75189N_| :::          Xtal               |   |
     |     |     | |     |   __________   _________1.8432 MHz_________  |   |
     | ..  |_____| |_____|  |SN75188N_|  |SN74LS556P       |SN74LS175N  |   |
     | ..  ______  ___  __________  __________  __________  __________  |   |
     | .. |     | |  | |_74F74PC_| |_SN74S08N| |_74F74PC_| |SN74LS32N|  |   |
     MC14411P-> | |  |  __________  __________  __________  __________  |   |
     | .. SN75189N->_| |_DL6311__| |_74F37N__| |_NE556N__| |_EP8304__|  |   |
     |    |     |       __________  __________  __________              |   |
     |    |_____|Xtal  |_74F74PC_| |_74F74PC_| |SN74LS56P|   ..         |___|
     |___________32 MHz___________________________________________________|


PCB (B3) (labeled "9000-0025 SS 1-1")
 - HM6116LP-3
 - MK48Z02B-20 NVRAM
 - 2 x HM6264LP-15
 - 3 x 27128 EPROM
 - Fujitsu MB7124H
         _______________________________________              _______________________________________
      __|                                       |____________|                                       |_
     |  |_______________________________________|            |_______________________________________| |
     |   ...                               :::::::                                                     |
     |    _______  __________  __________  __________  __________  __________  __________  __________  |
     |   |_P232_| |CD74HCT08E |SN74LS645| |SN74LS682N |SN74LS645N |SN74LS645N |SN74LS645N |SN74LS645N  |
     |             ______      ______     __________   ______    ______    ______    ______    ______  |
     |            |     |     |     |    |PC74HCT157P | <-HM6264LP-15 |   |     |   |     |   |     |  |
     |   HM6116LP-3->   |     |     |     __________  |     |   | <-HM6264LP-15 |   |     |   | <-EPROM|
     |            |  MK48Z02B-20->  |    |CD74HCT157E |     |   |     |   | <-EPROM |     |   |     |  |
     |            |     |     |     |     __________  |     |   |     |   |     |   | <-EPROM |     |  |
     |            |_____|     |_____|    |CD74HCT157E |_____|   |_____|   |____ |   |_____|   |_____|  |
     |  _________  __________  __________  __________  __________  __________  __________  __________  |
     | CD74HCT32E PC74HCT138P PC74HCT157P  CD74HCT04E PC74HCT164P CD74HCT273E |_MB7124H_|  CD74HCT32E  |
     |  ::::.                                                                                          |
     |  _________  __________  __________  __________  __________  __________  __________  __________  |
     | |__8429__| PC74HCT139P PC74HCT139P CD74HCT174E  CD74HCT74E  SN74LS645N PC74HCT374P  SN74LS645N  |
     |  _________  __________  __________     ::::                                                     |
     | CD74HCT04E  CD74HCT74E  CD74HCT08E                                           _____________      |
     |_____________________________________________________________________________|             |_____|
                                                                                   |_____________|

PCB MEASUREMENT (B2) (labeled "MEASUREMENT CIRCUIT 9000-0024")
 - AM2149-45DC
 - 12 x TMS4416-15NL
 - 2 x Intel P8254
 - Motorola MC68230P10
 - 3 x AM27S21APC PROM
 - Fujitsu MB7124H PROM


PCB VIDEO (B1) (labeled "OPTOMAX VIDEO PCB 9000-0022-2/2")
 - Xtal (unknown frequency, labeled "Xtal Hy-O 2562-50 GE01S")
 - 2716 EPROM
 - 2 x Toshiba TC5565PL-15
 - Motorola MC68230P8
                   ___     ___     ___     ___
                  |   |   |   |   |   |   |   |
   _______________|   |___|   |___|   |___|   |________________________________
  |               |___|   |___|   |___|   |___|                               |
  |                           4 x BNC                                         |
 _|_                ······    __________  __________  __________    ······    |
|   |                        |_74128N__| |PC74HCT04P |PC74HCT74P              |
|   |                         __________  __________  __________  __________  |
|   |                        |HEF4046BP| |SN74LS221N |PC74HCT74P |I1-524-5_|  |
|   |                         ________    __________                          |
|   |                        |CA3140E|   |SN74LS221N                          |
|   |                                     __________                          |
|   |                                    |PC74HCT32P                          |
|   |  _________                          __________                          |
|   | |SN74LS05N                         |PC74HCT08P                          |
|   |                         __________  __________  __________  ________    |
|   |                        |MC74HC04_| |SN74LS123N |_LM361N__| |CA3240E|    |
|   |                                                                         |
|   |                         __________                          __________  |
|   |                        |_LM361N__|                         |I1-201-5_|  |
|___|              _______    __________                                      |
  |               |CA3240E|  |_LM361N__|                                      |
  |    _________  __________  __________                                      |
 _|_ CD74HCT151E |CD74HCT86E |_LM361N__|                          __________  |
|   |             ________                                       |I1-201-5_|  |
|   |            |CA3240E|                                        ________    |
|   |  _________  __________  __________                         |CA3140E|    |
|   | PC74HCT08P |AD7528KN_| |PC74HCT04P                                      |
|   |        ___                          __________  __________  __________  |
|   |       |  |  ..   ________          |_ZN448E__| |__DAC08__| |CD74HCT00E  |
| SN74LS682N-> |  ..  |       |                       __________  __________  |
|   |       |__|  ..  | MC68230P8                    PC74HCT374P |SN74LS221N  |
|   |        ___      |       |                                               |
|   |       |  |      |       |         ____________  __________  __________  |
|   DS75451N->_|      |       |        |TC5565PL-15| CD74HCT393E |SN74LS221N  |
|   |        ___      |       |        |___________|                          |
|   |       |  |      |       |                       __________  __________  |
| SN74LS645N-> |      |       |         ____________ |PC74HCT74P |CD74HCT00E  |
|___|       |__|      |_______|        |TC5565PL-15|  __________  __________  |
  |   BATT             _____           |___________| CD74HCT4040E SN74LS221N  |
  |   __________  _____________  __________           __________              |
  |   PC74HCT32P | EPROM      | CD74HCT4040E         CD74HCT4040E             |
  |              |____________|                                               |
  |   __________   __________    __________           __________              |
  |  |PC74HCT74P  |PC74HCT74P   |CD74HCT00E          |SN74LS682N              |
  |   __________   __________    __________           __________              |
  |  |_ZNA134H_|  |SN74LC221N   |PC74HCT157P         |PC74LS157P              |
  |                                                                           |
  |   Xtal (labeled "Hy-O 2562-50 GE01S")                                     |
  |___________________________________________________________________________|

*********************************************************************************************************/

#include "emu.h"

#include "bus/acorn/bus.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m68000/m68000.h"
#include "machine/6522via.h"
#include "machine/68230pit.h"
#include "machine/6850acia.h"
#include "machine/input_merger.h"
#include "machine/m3002.h"
#include "machine/mc14411.h"
#include "machine/mm58167.h"
#include "machine/mos6551.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"
#include "sound/adc.h"

#include "screen.h"


namespace {

class optomaxv_state : public driver_device
{
public:
	optomaxv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)

		// CPU PCB (B4)
		, m_maincpu(*this, "maincpu")
		, m_b4_acia(*this, "b4_acia%u", 0U)
		, m_b4_brg(*this, "b4_brg")
		, m_b4_rtc(*this, "b4_rtc")
		, m_b4_pit(*this, "b4_pit")
		, m_b4_brf(*this, "BRF")
		, m_b4_baud_p3(*this, "BAUD_P3")
		, m_b4_baud_p4(*this, "BAUD_P4")
		, m_b4_baud_p5(*this, "BAUD_P5")

		// OSD PCB (B6)
		, m_eurocpu(*this, "eurocpu")
		, m_b6_acia(*this, "b6_acia")
		, m_b6_rtc(*this, "b6_rtc")
		, m_b6_via(*this, "b6_via")
		, m_b6_bus(*this, "b6_bus")

		// PCB (B3)
		, m_b3_view(*this, "b3_view")
		, m_b3_ram(*this, "b3_ram", 0x4000, ENDIANNESS_LITTLE)

		// Measurement PCB (B2)
		, m_b2_pit(*this, "b2_pit")
		, m_b2_pit8254(*this, "b2_pit8254_%u", 0U)

		// Video PCB (B1)
		, m_b1_pit(*this, "b1_pit")
	{ }

	void optomaxv(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// CPU PCB (B4)
	required_device<cpu_device> m_maincpu;
	required_device_array<acia6850_device, 3> m_b4_acia;
	required_device<mc14411_device> m_b4_brg;
	required_device<mm58167_device> m_b4_rtc;
	required_device<pit68230_device> m_b4_pit;
	required_ioport m_b4_brf;
	required_ioport m_b4_baud_p3;
	required_ioport m_b4_baud_p4;
	required_ioport m_b4_baud_p5;

	// OSD PCB (B6)
	required_device<m6502_device> m_eurocpu;
	required_device<mos6551_device> m_b6_acia;
	required_device<m3002_device> m_b6_rtc;
	required_device<via6522_device> m_b6_via;
	required_device<acorn_bus_device> m_b6_bus;

	// PCB (B3)
	memory_view m_b3_view;
	memory_share_creator<uint8_t> m_b3_ram;

	// Measurement PCB (B2)
	required_device<pit68230_device> m_b2_pit;
	required_device_array<pit8254_device, 2> m_b2_pit8254;

	// Video PCB (B1)
	required_device<pit68230_device> m_b1_pit;

	void mem_map(address_map &map) ATTR_COLD;
	void osd_map(address_map &map) ATTR_COLD;

	void write_acia_clocks(int id, int state);
	void write_f1_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F1, state); }
	void write_f3_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F3, state); }
	void write_f5_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F5, state); }
	void write_f7_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F7, state); }
	void write_f8_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F8, state); }
	void write_f9_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F9, state); }
	void write_f11_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F11, state); }
	void write_f13_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F13, state); }

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t map_r();
	void map_w(uint8_t data);

	uint8_t m_map = 0x00;
};


void optomaxv_state::mem_map(address_map &map)
{
	// Main CPU PCB (B4)

	map(0x000000, 0x07ffff).ram();
	map(0x000000, 0x000007).rom().region("eprom_sys", 0);
	map(0x080000, 0x087fff).rom().region("eprom_sys", 0);
	map(0x0a0000, 0x0affff).rom().region("eprom_usr", 0);
	map(0x0c0040, 0x0c0043).rw(m_b4_acia[0], FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);
	map(0x0c0080, 0x0c0083).rw(m_b4_acia[1], FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0xff00);
	map(0x0c0100, 0x0c0103).rw(m_b4_acia[2], FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);
	map(0x0c0400, 0x0c042f).rw(m_b4_rtc, FUNC(mm58167_device::read), FUNC(mm58167_device::write)).umask16(0x00ff);
	map(0x0e0000, 0x0e003f).rw(m_b4_pit, FUNC(pit68230_device::read), FUNC(pit68230_device::write)).umask16(0x00ff);

	// Measurement PCB (B2)

	map(0x100000, 0x117fff).ram();
	map(0x140000, 0x14003f).rw(m_b2_pit, FUNC(pit68230_device::read), FUNC(pit68230_device::write)).umask16(0xff00);
	//map(0x160000, 0x160007).rw(m_b2_pit8254, FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask16(0x00ff);
	//map(0x160008, 0x16000f).rw(m_b2_pit8254, FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask16(0x00ff);

	// VIDEO PCB (B1)

	// B1 2 x 8K static ram
	// B1 2K rom
	//map(0x200000, 0x201fff).ram();
	map(0x300000, 0x30003f).rw(m_b1_pit, FUNC(pit68230_device::read), FUNC(pit68230_device::write)).umask16(0x00ff);
}

void optomaxv_state::osd_map(address_map &map)
{
	// OSD PCB (B6)

	map(0x0000, 0x1fff).ram();                           // M3
	map(0x8000, 0xbfff).rom().region("b6_rom", 0x0000);  // M1
	map(0xe000, 0xffff).rom().region("b6_rom", 0x6000);  // M0
	map(0xd000, 0xdfff).lrw8(                    // I/O Block
		NAME([this](offs_t offset) { return m_b6_bus->read(offset | 0xd000); }),
		NAME([this](offs_t offset, uint8_t data) { m_b6_bus->write(offset | 0xd000, data); })
	);
	map(0xfe00, 0xfeff).lrw8(                   // I/O Page
		NAME([this](offs_t offset) { return m_b6_bus->read(offset | 0xfe00); }),
		NAME([this](offs_t offset, uint8_t data) { m_b6_bus->write(offset | 0xfe00, data); })
	);
	map(0xfe00, 0xfe0f).rw(m_b6_via, FUNC(via6522_device::read), FUNC(via6522_device::write));
	map(0xfe10, 0xfe17).rw(m_b6_acia, FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0xfe18, 0xfe1f).rw(m_b6_rtc, FUNC(m3002_device::read), FUNC(m3002_device::write));
	map(0xfe30, 0xfe30).nopw(); // usually ROM banking of region 0x8000-0xBFFF but unlikely to be relevant in this machine.

	// PCB (B3)

	map(0x2000, 0x7fff).rom().region("b3_rom", 0x2000);
	map(0x2000, 0x7fff).view(m_b3_view);
	m_b3_view[0](0x2000, 0x2fff).rom().region("b3_rom", 0xa000);
	m_b3_view[1](0x3000, 0x3fff).rom().region("b3_rom", 0xb000);
	m_b3_view[1](0x4000, 0x7fff).ram().share("b3_ram");
	map(0xc000, 0xc7ff).ram().share("nvram");
	map(0xc800, 0xcfff).ram();
	map(0xdc00, 0xdc00).rw(FUNC(optomaxv_state::map_r), FUNC(optomaxv_state::map_w));
}


void optomaxv_state::machine_start()
{
	save_item(NAME(m_map));
}

void optomaxv_state::machine_reset()
{
	// Set up the BRG divider. RSA is a jumper setting and RSB is always set High
	m_b4_brg->rsa_w(m_b4_brf->read() == 0x80 ? ASSERT_LINE : CLEAR_LINE);
	m_b4_brg->rsb_w(ASSERT_LINE);

	// Disable all configured timers, only enabling the used ones
	m_b4_brg->timer_disable_all();
	m_b4_brg->timer_enable((mc14411_device::timer_id) m_b4_baud_p3->read(), true);
	m_b4_brg->timer_enable((mc14411_device::timer_id) m_b4_baud_p4->read(), true);
	m_b4_brg->timer_enable((mc14411_device::timer_id) m_b4_baud_p5->read(), true);

	// Default memory map
	m_b3_view.disable();
}


uint8_t optomaxv_state::map_r()
{
	return m_map;
}

void optomaxv_state::map_w(uint8_t data)
{
	m_map = data;

	switch (m_map)
	{
	case 0x00:
		m_b3_view.disable();
		break;
	case 0x40:
		m_b3_view.select(0);
		break;
	case 0x80:
		m_b3_view.select(1);
		break;
	default:
		logerror("%s map_w: unknown %02x\n", machine().describe_context(), data);
		break;
	}
}


void optomaxv_state::write_acia_clocks(int id, int state)
{
	if (id == m_b4_baud_p3->read())
	{
		m_b4_acia[0]->write_txc(state);
		m_b4_acia[0]->write_rxc(state);
	}
	if (id == m_b4_baud_p4->read())
	{
		m_b4_acia[1]->write_txc(state);
		m_b4_acia[1]->write_rxc(state);
	}
	if (id == m_b4_baud_p5->read())
	{
		m_b4_acia[2]->write_txc(state);
		m_b4_acia[2]->write_rxc(state);
	}
}


uint32_t optomaxv_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


static INPUT_PORTS_START(optomaxv)
	PORT_START("BRF")
	PORT_CONFNAME(0x80, 0x00, "Baud Rate Factor") // RSA pin on MC14411
	PORT_CONFSETTING(0x00, "1x (Lo)")
	PORT_CONFSETTING(0x80, "4x (Hi)")

	PORT_START("BAUD_P3")
	PORT_CONFNAME(0x0f, 0x00, "P3 Host Baud Lo/Hi") // F1-Fx pins on MC14411
	PORT_CONFSETTING(mc14411_device::TIMER_F1,  "9600/38400") // RSA=1x/16x
	PORT_CONFSETTING(mc14411_device::TIMER_F3,  "4800/19200")
	PORT_CONFSETTING(mc14411_device::TIMER_F5,  "2400/9600")
	PORT_CONFSETTING(mc14411_device::TIMER_F7,  "1200/4800")
	PORT_CONFSETTING(mc14411_device::TIMER_F8,  "600/2400")
	PORT_CONFSETTING(mc14411_device::TIMER_F9,  "300/1200")
	PORT_CONFSETTING(mc14411_device::TIMER_F11, "150/600")
	PORT_CONFSETTING(mc14411_device::TIMER_F13, "110/440")

	PORT_START("BAUD_P4")
	PORT_CONFNAME(0x0f, 0x00, "P4 Terminal Baud Lo/Hi") // F1-Fx pins on MC14411
	PORT_CONFSETTING(mc14411_device::TIMER_F1,  "9600/38400") // RSA=1x/16x
	PORT_CONFSETTING(mc14411_device::TIMER_F3,  "4800/19200")
	PORT_CONFSETTING(mc14411_device::TIMER_F5,  "2400/9600")
	PORT_CONFSETTING(mc14411_device::TIMER_F7,  "1200/4800")
	PORT_CONFSETTING(mc14411_device::TIMER_F8,  "600/2400")
	PORT_CONFSETTING(mc14411_device::TIMER_F9,  "300/1200")
	PORT_CONFSETTING(mc14411_device::TIMER_F11, "150/600")
	PORT_CONFSETTING(mc14411_device::TIMER_F13, "110/440")

	PORT_START("BAUD_P5")
	PORT_CONFNAME(0x0f, 0x00, "P5 Remote Baud Lo/Hi") // F1-Fx pins on MC14411
	PORT_CONFSETTING(mc14411_device::TIMER_F1,  "9600/38400") // RSA=1x/16x
	PORT_CONFSETTING(mc14411_device::TIMER_F3,  "4800/19200")
	PORT_CONFSETTING(mc14411_device::TIMER_F5,  "2400/9600")
	PORT_CONFSETTING(mc14411_device::TIMER_F7,  "1200/4800")
	PORT_CONFSETTING(mc14411_device::TIMER_F8,  "600/2400")
	PORT_CONFSETTING(mc14411_device::TIMER_F9,  "300/1200")
	PORT_CONFSETTING(mc14411_device::TIMER_F11, "150/600")
	PORT_CONFSETTING(mc14411_device::TIMER_F13, "110/440")
INPUT_PORTS_END


static DEVICE_INPUT_DEFAULTS_START(terminal)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END


void optomaxv_state::optomaxv(machine_config &config)
{
	// Main CPU PCB (B4)

	M68000(config, m_maincpu, 32_MHz_XTAL / 4); // Signetics SCN68000
	m_maincpu->set_addrmap(AS_PROGRAM, &optomaxv_state::mem_map);

	ACIA6850(config, m_b4_acia[0], 0); // Hitachi HD46850P (Host)
	m_b4_acia[0]->txd_handler().set("rs232host", FUNC(rs232_port_device::write_txd));
	m_b4_acia[0]->rts_handler().set("rs232host", FUNC(rs232_port_device::write_rts));
	m_b4_acia[0]->irq_handler().set_inputline(m_maincpu, M68K_IRQ_2);

	rs232_port_device &rs232host(RS232_PORT(config, "rs232host", default_rs232_devices, nullptr));
	rs232host.rxd_handler().set(m_b4_acia[0], FUNC(acia6850_device::write_rxd));
	rs232host.cts_handler().set(m_b4_acia[0], FUNC(acia6850_device::write_cts));

	ACIA6850(config, m_b4_acia[1], 0); // Hitachi HD46850P (Terminal)
	m_b4_acia[1]->txd_handler().set("rs232term", FUNC(rs232_port_device::write_txd));
	m_b4_acia[1]->rts_handler().set("rs232term", FUNC(rs232_port_device::write_rts));
	m_b4_acia[1]->irq_handler().set_inputline(m_maincpu, M68K_IRQ_4);

	rs232_port_device &rs232term(RS232_PORT(config, "rs232term", default_rs232_devices, "terminal"));
	rs232term.rxd_handler().set(m_b4_acia[1], FUNC(acia6850_device::write_rxd));
	rs232term.cts_handler().set(m_b4_acia[1], FUNC(acia6850_device::write_cts));
	rs232term.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	ACIA6850(config, m_b4_acia[2], 0); // Hitachi HD46850P (Remote)
	m_b4_acia[2]->txd_handler().set("rs232remt", FUNC(rs232_port_device::write_txd));
	m_b4_acia[2]->rts_handler().set("rs232remt", FUNC(rs232_port_device::write_rts));
	m_b4_acia[2]->irq_handler().set_inputline(m_maincpu, M68K_IRQ_3);

	rs232_port_device &rs232remt(RS232_PORT(config, "rs232remt", default_rs232_devices, nullptr));
	rs232remt.rxd_handler().set(m_b4_acia[2], FUNC(acia6850_device::write_rxd));
	rs232remt.cts_handler().set(m_b4_acia[2], FUNC(acia6850_device::write_cts));

	MC14411(config, m_b4_brg, 1.8432_MHz_XTAL); // Motorola MC14411P
	m_b4_brg->out_f<1>().set(FUNC(optomaxv_state::write_f1_clock));
	m_b4_brg->out_f<3>().set(FUNC(optomaxv_state::write_f3_clock));
	m_b4_brg->out_f<5>().set(FUNC(optomaxv_state::write_f5_clock));
	m_b4_brg->out_f<7>().set(FUNC(optomaxv_state::write_f7_clock));
	m_b4_brg->out_f<8>().set(FUNC(optomaxv_state::write_f8_clock));
	m_b4_brg->out_f<9>().set(FUNC(optomaxv_state::write_f9_clock));
	m_b4_brg->out_f<11>().set(FUNC(optomaxv_state::write_f11_clock));
	m_b4_brg->out_f<13>().set(FUNC(optomaxv_state::write_f13_clock));

	MM58167(config, m_b4_rtc, 32.768_kHz_XTAL); // National Semiconductor MM58167AN
	m_b4_rtc->irq().set_inputline(m_maincpu, M68K_IRQ_6);

	PIT68230(config, m_b4_pit, 32_MHz_XTAL / 4); // Motorola MC68230L8
	m_b4_pit->port_irq_callback().set_inputline(m_maincpu, M68K_IRQ_5);

	// OSD PCB (B6) (CUBE EuroBEEB System CPU board)

	M6502(config, m_eurocpu, 2'000'000); // Rockwell R6502AP
	m_eurocpu->set_addrmap(AS_PROGRAM, &optomaxv_state::osd_map);

	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set_inputline(m_eurocpu, M6502_IRQ_LINE);

	MOS6522(config, m_b6_via, 2'000'000); // Rockwell R6522AP
	m_b6_via->irq_handler().set("irqs", FUNC(input_merger_device::in_w<0>));

	MOS6551(config, m_b6_acia, 0); // Rockwell R6551AP
	m_b6_acia->set_xtal(1.8432_MHz_XTAL);
	m_b6_acia->irq_handler().set("irqs", FUNC(input_merger_device::in_w<1>));
	m_b6_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_b6_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "keyboard"));
	rs232.rxd_handler().set(m_b6_acia, FUNC(mos6551_device::write_rxd));
	rs232.dsr_handler().set(m_b6_acia, FUNC(mos6551_device::write_dsr));
	rs232.cts_handler().set(m_b6_acia, FUNC(mos6551_device::write_cts));

	M3002(config, m_b6_rtc, 32.768_kHz_XTAL); // uEM M-3002-16PI Real Time Clock

	ACORN_BUS(config, m_b6_bus, 1'000'000);
	m_b6_bus->out_irq_callback().set("irqs", FUNC(input_merger_device::in_w<2>));
	m_b6_bus->out_nmi_callback().set_inputline(m_eurocpu, M6502_NMI_LINE);

	// Teletext PCB (B5) (CUBE EuroBEEB System Teletext Video Card)

	ACORN_BUS_SLOT(config, "slot1", m_b6_bus, eurocube_bus_devices, "teletext").set_fixed(true);

	// PCB (B3)

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // Mostek MK48Z02B-20 ZeroPower

	// Measurement PCB (B2)

	PIT68230(config, m_b2_pit, 32_MHz_XTAL / 4); // Motorola MC68230P8, unknown xtal
	PIT8254(config, m_b2_pit8254[0]); // Intel P8254
	PIT8254(config, m_b2_pit8254[1]); // Intel P8254

	// VIDEO PCB (B1)

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.screen_vblank().set(m_b1_pit, FUNC(pit68230_device::h2_w));
	screen.set_size(1024, 768);
	screen.set_visarea(0, 704-1, 0, 560-1);
	screen.set_screen_update(FUNC(optomaxv_state::screen_update));

	PIT68230(config, m_b1_pit, 32_MHz_XTAL / 4); // Motorola MC68230P8, unknown xtal

	ZN449(config, "adc", 0); // ZN448
}


ROM_START(optomaxv)
	ROM_REGION16_BE(0x8000, "eprom_sys", 0)
	ROM_LOAD16_BYTE( "b4_hn4827128g.j24",               0x0000, 0x4000, CRC(a6399550) SHA1(602b7152a5a5cbe1f8f598e6622d2bdfccd5d322) )
	ROM_LOAD16_BYTE( "b4_hn4827128g.j40",               0x0001, 0x4000, CRC(b85f37f7) SHA1(dc532ba0af735233c5a7308a5fbc90be746ee08c) )

	ROM_REGION16_BE(0x10000, "eprom_usr", 0)
	ROM_LOAD16_BYTE( "b4_68up-12-01-sd-1-86_27256.j41", 0x0000, 0x8000, CRC(a3196db7) SHA1(06773dea886908673d9def406d4985a6eef71d0c) )
	ROM_LOAD16_BYTE( "b4_68lw-12-01-sd-1-86_27256.j25", 0x0001, 0x8000, CRC(c0a88d38) SHA1(836a616bb6df84e3e6dfb1d42cacd592f6c6b4e6) )

	ROM_REGION(0x8000, "b6_rom", 0)
	ROM_LOAD( "b6_hn613128pb05.m1",                     0x0000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281) ) // Acorn BASIC, on OSD PCB
	ROM_LOAD( "b6_abmon4-abci-12-12_hn482764g.m0",      0x6000, 0x2000, CRC(8f9ce214) SHA1(37fca4a5184025ed034acd5f20a4614163246779) ) // On OSD PCB

	ROM_REGION(0x10000, "b3_rom", 0)
	ROM_LOAD( "b3_6sr01-04-4477-1-86_hn4827128g.ic24",  0x0000, 0x4000, CRC(e9110243) SHA1(c96f6f526b0b1c36971f7a0fc03a6e57d02213e5) ) // On PCB (B3)
	ROM_LOAD( "b3_6sr02-04-8140-1-86_d27128d.ic23",     0x4000, 0x4000, CRC(58c04246) SHA1(7eb0b276206f546b34bc5f32e05d755f1a700294) ) // On PCB (B3)
	ROM_LOAD( "b3_6sr03-04-b03f-1-86_hn4827128g.ic22",  0x8000, 0x4000, CRC(705f0f4d) SHA1(5c836694d736b2d2a268e3a8acdfb76fde6698c4) ) // On PCB (B3)

	ROM_REGION(0x0800, "video", 0)
	ROM_LOAD( "b1_9000-vs-1.1_d2716d.ic46",             0x0000, 0x0800, CRC(ebaefb94) SHA1(ca6d194926a98b846443ce7393e3b44d3e5199f9) ) // On Video PCB (B1)

	ROM_REGION(0x0200, "proms", 0)
	ROM_LOAD( "b6_sp007mp_82s147n.ic9",                 0x0000, 0x0200, CRC(35aaa7a3) SHA1(ebc977ff748a19cd0e9d0626cf7cf97d07656f80) ) // On OSD PCB (B6), for CPU address decoding
	ROM_LOAD( "b6_502_82s147.ic10",                     0x0000, 0x0200, CRC(401fa579) SHA1(e6320f70da9dfed0daae47af7b6cf9f3a62313b2) ) // On OSD PCB (B6), for CPU address decoding. Same as the standard EuroBEEB
	ROM_LOAD( "b3_b515c1-1.ic15",                       0x0000, 0x0200, CRC(c4b02b5f) SHA1(e7b3363974b8a1b61169f543a672dff37e8e0e11) ) // On PCB (B3)
	ROM_LOAD( "b2_cpi-sl-1.3_am27s21a.ic29",            0x0000, 0x0100, CRC(897071f9) SHA1(912154fd24d3601bcfd7fbd61be5c1ade62c12f3) ) // On Measurement PCB (B2)
	ROM_LOAD( "b2_hb-11.2_mb7124h.ic6",                 0x0000, 0x0200, CRC(78bab798) SHA1(f5b88db41efed9c540801c367047d608fb086094) ) // On Measurement PCB (B2)
	ROM_LOAD( "b2_hgb-22.2_am27s21a.ic7",               0x0000, 0x0100, CRC(5c3f4be5) SHA1(1ead926a5c71232c75f20673fe0a7c36ff4480bb) ) // On Measurement PCB (B2)
	ROM_LOAD( "b2_hgw-1.2_am27s21a.ic8",                0x0000, 0x0100, CRC(b45f4d48) SHA1(7cfe7b19efc7d034a5795a99dda347ae742c904d) ) // On Measurement PCB (B2)

	ROM_REGION(0x0117, "plds", 0)
	ROM_LOAD( "b4_0544_pal16l6.j22",                    0x00000, 0x00054, CRC(7d325ea4) SHA1(723a9938b7e3a0edf38261d7b6349efe0443d2e0) ) // On main CPU PCB (B4)
	ROM_LOAD( "b4_0545_pal16l8.j66",                    0x00000, 0x00104, CRC(f1837b78) SHA1(8eb40c7320bd626ec6037662234c5befba88e116) ) // On main CPU PCB (B4)
	ROM_LOAD( "b4_0546_pal12l10c.j17",                  0x00000, 0x00040, CRC(b669fd4a) SHA1(f122719c62e797a1c514389d8ea013e86c8b2040) ) // On main CPU PCB (B4)
	ROM_LOAD( "b4_0620_pal20l10c.j76",                  0x00000, 0x000cc, CRC(51e963ee) SHA1(81f0ad7e2505d31eb00be6742146c80e5631f3f2) ) // On main CPU PCB (B4)
	ROM_LOAD( "b4_0686_pal20l10c.j54",                  0x00000, 0x000cc, CRC(7cf9018b) SHA1(dcbfb718ab076935eb74452ba97a505d0bd34546) ) // On main CPU PCB (B4)
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY  FULLNAME     FLAGS
SYST( 1986, optomaxv,  0,      0,      optomaxv, optomaxv, optomaxv_state, empty_init, "AMS",   "Optomax V", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING ) // Hardware from 1985, software on ROM from 1986.
