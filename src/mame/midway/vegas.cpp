// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Atari/Midway Vegas hardware games

    driver by Aaron Giles

    Games supported:
        * Gauntlet Legends [200MHz R5000, 8MB RAM, Vegas + Vegas SIO + Voodoo 2, 2-TMU * 4MB]
        * Tenth Degree/Juko Threat [200MHz R5000, 8MB RAM, Vegas + Vegas SIO + Voodoo 2, 2-TMU * 4MB]
        * Gauntlet Dark Legacy [200/250MHz R5000, 8/32MB RAM, Vegas/Durango + Vegas SIO + Voodoo 2, 2-TMU * 4MB]
        * War: Final Assault [200/250MHz R5000, 8/32MB RAM, Vegas/Durango + Vegas SIO + Voodoo 2, 2-TMU * 4MB]
        * NBA Showtime (Gold)/Sportstation [200/250MHz R5000, 8/32MB RAM, Vegas/Durango + Vegas SIO + Voodoo Banshee, 16MB]
        * Road Burners [250MHz QED5271, 32MB RAM, Durango + DSIO + Voodoo 2, 2-TMU * 4MB]
        * San Francisco Rush 2049 [250MHz RM7000, 32MB RAM, Durango + Denver + Voodoo 3, 16MB]
        * CART Fury Championship Racing [250MHz RM7000, 32MB RAM, Durango + Denver + Voodoo 3, 16MB]

    Known bugs:
        * Tournament Editions not working yet

***************************************************************************

    Interrupt summary:

                        __________
    UART clear-to-send |          |
    -------(0x2000)--->|          |
                       |          |
    UART data ready    |          |
    -------(0x1000)--->|          |                     __________
                       |          |   VSYNC            |          |
    Main-to-sound empty|  IOASIC  |   -------(0x20)--->|          |
    -------(0x0080)--->|          |                    |          |
                       |          |   Ethernet         |          |   SIO Summary
    Sound-to-main full |          |   -------(0x10)--->|   SIO    |--------------+
    -------(0x0040)--->|          |                    |          |              |
                       |          |                    |          |              |
    Sound FIFO empty   |          |   IOASIC Summary   |          |              |
    -------(0x0008)--->|          |----------(0x04)--->|          |              |
                       |__________|                    |__________|              |
                                      NSS/Hi-Link  (0x08)                        |
                                      A2D IRQ      (0x02)                        |
                                      SIO Watchdog (0x01)                        |
 +-------------------------------------------------------------------------------+
 |
 |                      __________                      __________
 |  IDE Controller     |          |                    |          |
 |  -------(0x0800)--->|          |                    |          |
 |                     |          |----------(IRQ5)--->|          |
 |  SIO Summary        |          |                    |          |
 +---------(0x0400)--->|          |----------(IRQ4)--->|          |
                       |          |                    |          |
    Timer 2            |          |----------(IRQ3)--->|   CPU    |
    -------(0x0040)--->|   NILE   |                    |          |
                       |          |----------(IRQ2)--->|          |
    Timer 3            |          |                    |          |
    -------(0x0020)--->|          |----------(IRQ1)--->|          |
                       |          |                    |          |
    UART Transmit      |          |----------(IRQ0)--->|          |
    -------(0x0010)--->|          |                    |__________|
                       |__________|

***************************************************************************

 PCB Summary:

    Three boards per game. CPU board, sound I/O board, and a video board.

    CPU boards:
        Vegas    - R5000 @ 200-250MHz, 8MB RAM
        Vegas32  - R5000 @ 200-250MHz, 32MB RAM
        Durango  - RM7000 or RM5271 @ 250-300MHz, 8-32MB RAM

    Sound I/O boards:
        Vegas SIO  - ADSP2104 @ 16MHz, boot ROM, 4MB RAM
        Deluxe SIO - ADSP2181 @ 32MHz, no ROM, 4MB RAM
        Denver SIO - ADSP2181 @ 33MHz, no ROM, 4MB RAM

    Video boards:
        Voodoo 2
        Voodoo Banshee
        Voodoo 3


***************************************************************************

 Gauntlet Legends info:


 CPU PCB:  Vegas CPU  Midway no.5770-15563-06
 --------------------------------------------

 U16   33.3333MHz Oscillator (to U2 uPD82157N7-002)
 U9   100.0000MHz Oscillator (to U10 MPC948)

 U1   79RV5000-200      IDT 64-bit Processor     TBGA
 U2   uPD82157N7-002    NEC                      TBGA (huge)
 U3   uPD4516161ag5-A10 NEC 1Megx16 SDRAM        TSOP
 U4   uPD4516161ag5-A10 NEC 1Megx16 SDRAM        TSOP
 U5   uPD4516161ag5-A10 NEC 1Megx16 SDRAM        TSOP
 U6   uPD4516161ag5-A10 NEC 1Megx16 SDRAM        TSOP
 U13  EPM7032LC44-15T   Altera PLD               Midway no.A-22544
 U11  93LC46B           Microchip EEPROM         Midway no.A-22545
 U18  27C4001           Boot ROM
 U10  MPC948            Motorola low voltage clock distribution chip
 U21  PC1646U2          CMD EIDE Controller


 Sound I/O PCB: Vegas 7-7-7 SIO-4 PCB  Midway no.5770-15534-04
 -------------------------------------------------------------

 U1   33.3333MHz  Oscillator
 Y1   20.0000MHz  XTAL       (to U24 SMC91C94QFP)
 Y2    4.0000MHz  XTAL       (to U37 PIC micro)
 Y3   16.0000MHz  XTAL       (to U14 ADSP)

 U28  M4T28-BR12SH1    ST Timer Keeper Snap Hat RAM
 U14  ADSP2104         Analog Devices DSP
 U8   EPF6016TC144-2   Altera FLEX 6000 PLD    144 pin TQFP
 U11  5410-14589-00    Midway Custom           164 pin QFP
 U34  5410-14590-00    Midway Custom            80 pin PQFP
 U18  ADC0848CCN       NSC ADC
 U24  SMC91C94QFP      SMC ISA/PCMCIA Ethernet & Modem Controller
 U5   AD1866R          Analog Devices dual 16-bit audio DAC
 U31  IS61C256AH-15J   ISSI 32kx8 SRAM
 U32  IS61C256AH-15J   ISSI 32kx8 SRAM
 U33  IS61C256AH-15J   ISSI 32kx8 SRAM
 U25  TMS418160ADZ     TI 1048576x16 DRAM
 U26  TMS418160ADZ     TI 1048576x16 DRAM
 U37  PIC16C57C        Microchip PIC  Atari no.322 Gauntlet 27"
 U44  27C256           Vegas SIO Audio Boot ROM
 U1   AM7201-35JC      AMD opamp
 U2   AM7201-35JC      AMD opamp


 GFX Card: Quantum 3D - Obsidian2 PCI (3DFx Voodoo2)
 ---------------------------------------------------

 U1    500-0009-01      3DFX Pixel processor    256 pin PQFP
 U26   500-0010-01      3DFX Texture processor  208 pin PQFP
 U2    500-0010-01      3DFX Texture processor  208 pin PQFP
 U20   ICS5342-3        ICS DAC
 U21   XC9572           Xilinx CPLD Firmware 546-0014-02
 U53   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U54   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U55   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U56   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U37   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U38   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U39   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U40   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U45   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U46   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U47   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U48   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U49   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U50   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U51   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U52   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U57   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U58   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U59   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U60   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg


***************************************************************************

 Gauntlet Dark Legacy Info:


 CPU board: Durango 5770-15982-00    sticker with RM5271 on it
 --------------------------------
 1x RM5271 CPU(assuming this is correct...don't want to remove the heatsink from my good board)
 1x medium sized BGA, has heatsink
 1x Atmel ATF1500A  PLD  A-22912
 4x MT48LC1M16AT RAM
 1x 93clc46b       label A-22911   config eeprom
 1x texas instruments 8CA00YF (don't know what it is)
 1x motorola MPC948 clock distribution chip
 100MHz crystal
 1x CMDPCI646U2 IDE controller
 1x 7segment LED display (cycles IOASIC if you try to load a game that doesn't match the PIC, spins during normal play)
 1x 232ACBN serial port controller
 other misc 74xxx parts
 Boot ROM 1.7

 Connectors:
   P1 4 pin marked "Reset/Interrupt"
   P2 20 pin marked "PCI Extend"
   P8B Standard PCI connector found on PC motherboards (although is custom mounted on the side of the PCB) Voodoo connects here
   P3 DB15 labeled "Video In" Voodoo connects here, but is not required if you connect directly to the Voodoo card.
   P4 Standard IDE connector marked "IDE HDD"
   P5 9 pin marked "Serial"
   Non-designated connector marked "EXP CONN" Standard PCI connector found on PC motherboards(custom mounted on the side of the PCB), SIO connects here



 SIO board: Vegas777 5770-15534-05
 ---------------------------------
 1x Midway 5410-14589-00 IO chip
 1x ADSP-2104  (16MHz crystal attached to clock pins)
 1x ADC0848CCN analog input chip
 1x ST M4t28-br12sh1  timekeeper
 1x Altera Flex epf6016tc144   pld
 1x Midway security PIC Gauntlet DL 27" 346xxxxxx (4MHz crystal attached)
 2x KM416C1200CJ-6  RAM
 1x SMSC LAN91C94 ethernet
 1x Midway 5410-14590-00 ???
 3x CY7C199-15VC  32Kx8 SRAM
 1x AD1866 DAC
 20MHz crystal attached to LAN chip
 other misc parts
 SIO ROM 1.0



 Voodoo card: Quantum3D Obsidian2 PCI 650-0818-03D
 -------------------------------------------------
 (SLI pin holes filled with solder, no connector, has no VGA input/passthrough connector, only VGA out)

 1x XC9572 label 546-0014-02
 20x silicon magic 100MHz   sm81c256k16cj-25   RAM (8MB for texture RAM, 2MB for framebuffer?)
 2x 3dfx 500-0010-01 texelfx
 1x 3dfx 500-0009-01 pixelfx
 1x 14.318MHz crystal
 1 x 3384q 10bit bus switch(i think)
 1x ICS GenDAC ICS5342-3
 other misc IC's


***************************************************************************

 Gauntlet Legends versus Gauntlet Dark Legacy


 CPU board: Vegas 5770-15563-06
 ------------------------------
 4x NEC D4516161A65-A10-9NF 8MB RAM  (other difference here...this PCB only has spots for these chips, the Durango has alternate positions for RAM, I assume for larger chips)
 1x 33.3333MHz crystal (in addition to the 100MHz crystal, on Durango this spot is present but unpopulated)
 1x Atmel ATF1500A  PLD A-22560 (replaces A-22912 on Durango)
 1x 93lc46b (A-22545 config EEPROM, dump is mostly FF and 00)
 Boot ROM "Gauntlet Update Boot v1.5 A-5343-30022-7"



 SIO board: Vegas777 5770-15534-05 (same as Legacy)
 --------------------------------------------------
 Security PIC "Gauntlet 27" 322xxxxxx"
 Sound ROM "Gauntlet 3D U44 Sound 1.0"
 2x TMS418160ADZ RAM instead of 2x KM416C1200CJ-6 on Legacy
 20MHz crystal attached to LAN chip
 1x Valor  SF1012 ethernet physical interface(also on other board)

 Connectors:
   P1 5 pin marked "Snd Line In"
   P5 5 pin marked "Snd Line Out"
   unmarked 4 pin PC style power connector(for hard drive)
   standard JAMMA connection
   P7 14 pin marked "PLYR4"
   P14 14 pin marked "PLYR3"
   P23 10 pin marked "Coin DR"
   P10 unmarked 10 pin connector (no idea)
   P21 99 pin unmarked -right on top of PCI edge connection to CPU board
   P15 20 pin marked "Aux Latched Out"
   P8 14 pin unmarked
   P2 11 pin marked "Gun 2 I/O"
   P4 11 pin marked "Gun 1 I/O"
   P 18 standard ethernet connector


**************************************************************************/

#include "emu.h"

#include "midwayic.h"

#include "dcs.h"

#include "bus/ata/hdd.h"
#include "bus/rs232/rs232.h"
#include "cpu/adsp2100/adsp2100.h"
#include "cpu/mips/mips3.h"
#include "machine/idectrl.h"
#include "machine/input_merger.h"
#include "machine/ins8250.h"
#include "machine/pci-ide.h"
#include "machine/pci.h"
#include "machine/smc91c9x.h"
#include "machine/timekpr.h"
#include "machine/vrc5074.h"
#include "video/voodoo_pci.h"

#include "screen.h"
#include "speaker.h"

#include "sf2049.lh"

#define LOG_TIMEKEEPER       (1U << 1)
#define LOG_TIMEKEEPER_LOCKS (1U << 2)
#define LOG_SIO              (1U << 3)
#define LOG_SIO_VERBOSE      (1U << 4)
#define LOG_WATCHDOG         (1U << 5)
#define LOG_UNKNOWN          (1U << 6)

#define VERBOSE (LOG_UNKNOWN)
#include "logmacro.h"


namespace {

/*************************************
 *
 *  Core constants
 *
 *************************************/

#define PCI_ID_NILE     "pci:00.0"
#define PCI_ID_VIDEO    "pci:03.0"
#define PCI_ID_IDE      "pci:05.0"

class vegas_state : public driver_device
{
public:
	vegas_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nile(*this, PCI_ID_NILE),
		m_timekeeper(*this, "timekeeper") ,
		m_ethernet(*this, "ethernet"),
		m_dcs(*this, "dcs"),
		m_ioasic(*this, "ioasic"),
		m_uart1(*this, "uart1"),
		m_uart2(*this, "uart2"),
		m_io_analog(*this, "AN.%u", 0U),
		m_io_8way(*this, "8WAY_P%u", 1U),
		m_io_49way_x(*this, "49WAYX_P%u", 1U),
		m_io_49way_y(*this, "49WAYY_P%u", 1U),
		m_io_keypad(*this, "KEYPAD"),
		m_io_gearshift(*this, "GEAR"),
		m_io_system(*this, "SYSTEM"),
		m_io_dips(*this, "DIPS"),
		m_system_led(*this, "system_led"),
		m_wheel_driver(*this, "wheel"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void vegascore(machine_config &config);
	void vegas(machine_config &config);
	void vegas250(machine_config &config);
	void vegas32m(machine_config &config);
	void vegasban(machine_config &config);
	void vegasv3(machine_config &config);
	void denver(machine_config &config);

	void nbanfl(machine_config &config);
	void sf2049te(machine_config &config);
	void sf2049se(machine_config &config);
	void nbashowt(machine_config &config);
	void gauntdl(machine_config &config);
	void sf2049(machine_config &config);
	void gauntleg(machine_config &config);
	void cartfury(machine_config &config);
	void tenthdeg(machine_config &config);
	void nbagold(machine_config &config);
	void roadburn(machine_config &config);
	void warfa(machine_config &config);

	void init_gauntleg();
	void init_cartfury();
	void init_tenthdeg();
	void init_nbashowt();
	void init_nbagold();
	void init_warfa();
	void init_roadburn();
	void init_sf2049te();
	void init_gauntdl();
	void init_nbanfl();
	void init_sf2049();
	void init_sf2049se();

	ioport_value i40_r();
	ioport_value gauntleg_p12_r();
	ioport_value gauntleg_p34_r();
	ioport_value keypad_r();
	ioport_value gearshift_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static constexpr unsigned SYSTEM_CLOCK = 100'000'000;

	required_device<mips3_device> m_maincpu;
	required_device<vrc5074_device> m_nile;
	required_device<m48t37_device> m_timekeeper;
	required_device<smc91c94_device> m_ethernet;
	required_device<dcs_audio_device> m_dcs;
	required_device<midway_ioasic_device> m_ioasic;
	optional_device<ns16550_device> m_uart1;
	optional_device<ns16550_device> m_uart2;

	optional_ioport_array<8> m_io_analog;
	optional_ioport_array<4> m_io_8way;

	optional_ioport_array<4> m_io_49way_x;
	optional_ioport_array<4> m_io_49way_y;

	optional_ioport m_io_keypad;
	optional_ioport m_io_gearshift;
	optional_ioport m_io_system;
	optional_ioport m_io_dips;
	output_finder<> m_system_led;
	output_finder<1> m_wheel_driver;
	output_finder<16> m_lamps;

	static const uint8_t translate49[7];

	uint8_t m_a2d_shift = 0;
	int8_t m_wheel_force = 0;
	int m_wheel_offset = 0;
	bool m_wheel_calibrated = false;
	uint8_t m_vblank_state = 0;
	uint8_t m_cpuio_data[4] = { };
	uint8_t m_sio_reset_ctrl = 0;
	uint8_t m_sio_irq_enable = 0;
	uint8_t m_sio_irq_state = 0;
	uint8_t m_duart_irq_state = 0;
	uint8_t m_sio_led_state = 0;
	uint8_t m_pending_analog_read = 0;
	uint8_t m_cmos_unlocked = 0;
	uint8_t m_dcs_idma_cs = 0;
	uint32_t m_i40_data = 0;
	uint32_t m_keypad_select = 0;
	uint32_t m_gear = 0;

	void duart_irq_cb(int state);
	void vblank_assert(int state);

	void update_sio_irqs();

	void watchdog_reset(int state);
	void watchdog_irq(int state);
	void timekeeper_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t timekeeper_r(offs_t offset, uint32_t mem_mask = ~0);
	void reset_sio(void);
	uint8_t sio_r(offs_t offset);
	void sio_w(offs_t offset, uint8_t data);
	void cpu_io_w(offs_t offset, uint8_t data);
	uint8_t cpu_io_r(offs_t offset);
	uint32_t analog_port_r(offs_t offset, uint32_t mem_mask = ~0);
	void analog_port_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void asic_fifo_w(uint32_t data);
	uint32_t ethernet_r(offs_t offset, uint32_t mem_mask = ~0);
	void ethernet_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void dcs3_fifo_full_w(uint32_t data);
	void ethernet_interrupt(int state);
	void ioasic_irq(int state);
	uint32_t unknown_r(offs_t offset, uint32_t mem_mask = ~0);
	uint8_t parallel_r(offs_t offset);
	void parallel_w(offs_t offset, uint8_t data);
	void mpsreset_w(offs_t offset, uint8_t data);
	void i40_w(uint32_t data);

	void wheel_board_w(uint32_t data);

	std::string sioIRQString(uint8_t data);

	void vegas_cs2_map(address_map &map) ATTR_COLD;
	void vegas_cs3_map(address_map &map) ATTR_COLD;
	void vegas_cs4_map(address_map &map) ATTR_COLD;
	void vegas_cs5_map(address_map &map) ATTR_COLD;
	void vegas_cs6_map(address_map &map) ATTR_COLD;
	void vegas_cs7_map(address_map &map) ATTR_COLD;
	void vegas_cs8_map(address_map &map) ATTR_COLD;

	static void hdd_config(device_t *device);
};

/*************************************
 *
 *  Machine init
 *
 *************************************/

void vegas_state::machine_start()
{
	/* set the fastest DRC options, but strict verification */
	/* need to check the current options since some drivers add options in their init */
	uint32_t new_options = m_maincpu->mips3drc_get_options();
	new_options |= MIPS3DRC_FASTEST_OPTIONS | MIPS3DRC_STRICT_VERIFY;
	m_maincpu->mips3drc_set_options(new_options);

	m_system_led.resolve();
	m_wheel_driver.resolve();
	m_lamps.resolve();

	/* register for save states */
	save_item(NAME(m_vblank_state));
	save_item(NAME(m_cpuio_data));
	save_item(NAME(m_sio_reset_ctrl));
	save_item(NAME(m_sio_irq_enable));
	save_item(NAME(m_sio_irq_state));
	save_item(NAME(m_duart_irq_state));
	save_item(NAME(m_sio_led_state));
	save_item(NAME(m_pending_analog_read));
	save_item(NAME(m_cmos_unlocked));
	save_item(NAME(m_i40_data));
	save_item(NAME(m_keypad_select));
	save_item(NAME(m_gear));
	save_item(NAME(m_wheel_calibrated));

	/* identify our sound board */
	if (m_dcs->get_rev() == dcs_audio_device::REV_DSIO) {
		m_dcs_idma_cs = 6;
		LOGMASKED(LOG_SIO, "Found dsio\n");
	}
	else if (m_dcs->get_rev() == dcs_audio_device::REV_DENV) {
		m_dcs_idma_cs = 7;
		LOGMASKED(LOG_SIO, "Found denver\n");
	}
	else {
		m_dcs_idma_cs = 0;
		LOGMASKED(LOG_SIO, "Did not find dcs2 sound board\n");
	}

	m_cmos_unlocked = 0;
}


void vegas_state::machine_reset()
{
	m_dcs->reset_w(0);
	m_dcs->reset_w(1);

	// Clear CPU IO registers
	std::fill(std::begin(m_cpuio_data), std::end(m_cpuio_data), 0);

	// Clear SIO registers
	reset_sio();
	m_duart_irq_state = 0;
	m_i40_data = 0;
	m_keypad_select = 0;
	m_gear = 1;
	m_wheel_force = 0;
	m_wheel_offset = 0;
	m_wheel_calibrated = false;
}

/*************************************
*  Watchdog interrupts
*************************************/
#define WD_IRQ 0x1
void vegas_state::watchdog_irq(int state)
{
	if (state && !(m_sio_irq_state & WD_IRQ)) {
		LOGMASKED(LOG_WATCHDOG, "%s: vegas_state::watchdog_irq state = %i\n", machine().describe_context(), state);
		m_sio_irq_state |= WD_IRQ;
		update_sio_irqs();
	}
	else if (!state && (m_sio_irq_state & WD_IRQ)) {
		LOGMASKED(LOG_WATCHDOG, "%s: vegas_state::watchdog_irq state = %i\n", machine().describe_context(), state);
		m_sio_irq_state &= ~WD_IRQ;
		update_sio_irqs();
	}
}

/*************************************
*  Watchdog Reset
*************************************/
void vegas_state::watchdog_reset(int state)
{
	if (state) {
		LOGMASKED(LOG_WATCHDOG, "vegas_state::watchdog_reset!!!\n");
		machine().schedule_soft_reset();
	}
}

/*************************************
 *
 *  Timekeeper access
 *
 *************************************/

void vegas_state::timekeeper_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (m_cmos_unlocked) {
		if (ACCESSING_BITS_0_7)
			m_timekeeper->write(offset * 4 + 0, data >> 0);
		if (ACCESSING_BITS_8_15)
			m_timekeeper->write(offset * 4 + 1, data >> 8);
		if (ACCESSING_BITS_16_23)
			m_timekeeper->write(offset * 4 + 2, data >> 16);
		if (ACCESSING_BITS_24_31)
			m_timekeeper->write(offset * 4 + 3, data >> 24);
		if (offset*4 >= 0x7ff0)
			LOGMASKED(LOG_TIMEKEEPER, "%s timekeeper_w(%04X & %08X) = %08X\n", machine().describe_context(), offset*4, mem_mask, data);
		m_cmos_unlocked = 0;
	}
	else
		LOGMASKED(LOG_TIMEKEEPER_LOCKS, "%s: timekeeper_w(%04X,%08X & %08X) without CMOS unlocked\n", machine().describe_context(), offset, data, mem_mask);
}


uint32_t vegas_state::timekeeper_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0xffffffff;
	if (ACCESSING_BITS_0_7)
		result = (result & ~0x000000ff) | (m_timekeeper->read(offset * 4 + 0) << 0);
	if (ACCESSING_BITS_8_15)
		result = (result & ~0x0000ff00) | (m_timekeeper->read(offset * 4 + 1) << 8);
	if (ACCESSING_BITS_16_23)
		result = (result & ~0x00ff0000) | (m_timekeeper->read(offset * 4 + 2) << 16);
	if (ACCESSING_BITS_24_31)
		result = (result & ~0xff000000) | (m_timekeeper->read(offset * 4 + 3) << 24);
	if (offset * 4 >= 0x7ff0) {
		// Initial RTC check expects reads to the RTC to take some time
		m_maincpu->eat_cycles(30);
		LOGMASKED(LOG_TIMEKEEPER, "%s: timekeeper_r(%04X & %08X) = %08X\n", machine().describe_context(), offset * 4, mem_mask, result);
	}
	return result;
}



/*************************************
 *
 *  SIO interrupts
 *
 *************************************/
std::string vegas_state::sioIRQString(uint8_t data)
{
	std::string sioBitDef[6] = { "SIO_WD", "A2D", "IOASIC", "NSS", "ETHER", "VSYNC" };
	std::string sioBitSel = "";
	for (int i = 0; i < 6; i++)
		if (data & (1 << i)) sioBitSel += " " + sioBitDef[i];
	return sioBitSel;

}

void vegas_state::update_sio_irqs()
{
	// Duart shares IRQ with SIO
	if ((m_sio_irq_state & m_sio_irq_enable) || m_duart_irq_state) {
		m_nile->pci_intr_c(ASSERT_LINE);
	}
	else {
		m_nile->pci_intr_c(CLEAR_LINE);
	}

	std::string sioEnable = sioIRQString(m_sio_irq_enable);
	std::string sioState = sioIRQString(m_sio_irq_state);
	LOGMASKED(LOG_SIO, "update_sio_irqs: irq_enable: %02x %s irq_state: %02x %s\n", m_sio_irq_enable, sioEnable, m_sio_irq_state, sioState);
}

void vegas_state::duart_irq_cb(int state)
{
	// Duart shares IRQ with SIO
	if (state ^ m_duart_irq_state) {
		m_duart_irq_state = state;
		update_sio_irqs();
	}
}

void vegas_state::vblank_assert(int state)
{
	LOGMASKED(LOG_SIO, "vblank_assert: m_sio_reset_ctrl: %04x state: %d\n", m_sio_reset_ctrl, state);
	// latch on the correct polarity transition
	if ((m_sio_irq_enable & 0x20) && ((state && !(m_sio_reset_ctrl & 0x10)) || (!state && (m_sio_reset_ctrl & 0x10)))) {
		m_sio_irq_state |= 0x20;
		update_sio_irqs();
	}
}


void vegas_state::ioasic_irq(int state)
{
	if (state)
		m_sio_irq_state |= 0x04;
	else
		m_sio_irq_state &= ~0x04;
	update_sio_irqs();
}

void vegas_state::ethernet_interrupt(int state)
{
	if (state)
		m_sio_irq_state |= 0x10;
	else
		m_sio_irq_state &= ~0x10;
	update_sio_irqs();
}

void vegas_state::reset_sio()
{
	m_sio_reset_ctrl = 0;
	m_sio_irq_enable = 0;
	m_sio_irq_state = 0;
	m_sio_led_state = 0;
	update_sio_irqs();
}

uint8_t vegas_state::sio_r(offs_t offset)
{
	uint32_t result = 0x0;
	int index = offset >> 12;
	switch (index) {
	case 0:
		// Reset Control:  Bit 0=>Reset IOASIC, Bit 1=>Reset NSS Connection, Bit 2=>Reset SMC, Bit 3=>Reset VSYNC, Bit 4=>VSYNC Polarity
		result = m_sio_reset_ctrl;
		// Hack for fpga programming finished
		m_cpuio_data[3] |= 0x1;
		break;
	case 1:
		// Interrupt Enable
		result = m_sio_irq_enable;
		LOGMASKED(LOG_SIO, "%s: sio_r: INTR ENABLE 0x%02x %s\n", machine().describe_context(), result, sioIRQString(result).c_str());
		break;
	case 2:
		// Interrupt Cause
		result = m_sio_irq_state & m_sio_irq_enable;
		LOGMASKED(LOG_SIO, "%s: sio_r: INTR CAUSE 0x%02x %s\n", machine().describe_context(), result, sioIRQString(result).c_str());
		//m_sio_irq_state &= ~0x02;
		break;
	case 3:
		// Interrupt Status
		result = m_sio_irq_state;
		LOGMASKED(LOG_SIO, "%s: sio_r: INTR STATUS 0x%02x %s\n", machine().describe_context(), result, sioIRQString(result).c_str());
		break;
	case 4:
		// LED
		result = m_sio_led_state;
		break;
	case 5:
	{
		result = 0x00;
		switch (offset & 0x7) {
		case 0:
			// Gun 1 H Low
			break;
		case 1:
			// Gun 1 H High
			// P1 magic = ~0x10;
			// P1 fight = ~0x20;
			result = ~m_io_8way[0]->read() & 0x30;
			//result = 0x70;
			break;
		case 2:
			// Gun 1 V Low
			break;
		case 3:
			// Gun 1 V High
			// P1 run   = 0x08
			// P2 magic = 0x10
			// P2 fight = 0x20
			// P2 run   = 0x40
			result = (((m_io_8way[0]->read() & 0x40) >> 3) | ((m_io_8way[1]->read() & 0x7000) >> 8));
			//result = ~0x7c;
			break;
		case 4:
			// Gun 2 H Low
			break;
		case 5:
			// Gun 2 H High
			result = ~m_io_8way[2]->read() & 0x30;
			break;
		case 6:
			// Gun 2 V Low
			break;
		case 7:
			// Gun 2 V High
			result = (((m_io_8way[2]->read() & 0x40) >> 3) | ((m_io_8way[3]->read() & 0x7000) >> 8));
			break;
		}
		LOGMASKED(LOG_SIO, "%s: sio_r: offset: %08x index: %d result: %02X\n", machine().describe_context(), offset, index, result);
		break;
	}
	}
	if (index < 0x1 || index > 0x4)
		LOGMASKED(LOG_SIO, "%s: sio_r: offset: %08x index: %d result: %02X\n", machine().describe_context(), offset, index, result);
	return result;
}


void vegas_state::sio_w(offs_t offset, uint8_t data)
{
	// Bit 0 of data is used to program the 6016 FPGA in programming mode (m_cpio_data[3](Bit 0)==0)
	if (m_cpuio_data[3] & 0x1) {
		int index = offset >> 12;
		switch (index) {
		case 0:
			LOGMASKED(LOG_SIO, "sio_w: Reset Control offset: %08x index: %d data: %02X\n", offset, index, data);
			// Reset Control:  Bit 0=>Reset IOASIC, Bit 1=>Reset NSS Connection, Bit 2=>Reset SMC, Bit 3=>Reset VSYNC, Bit 4=>VSYNC Polarity
			/* bit 0 is used to reset the IOASIC */
			if (!(data & (1 << 0)))
				m_ioasic->ioasic_reset();
			m_dcs->reset_w(data & 0x01);
			if ((data & (1 << 2)) && !(m_sio_reset_ctrl & (1 << 2))) {
				logerror("sio_w: Ethernet reset\n");
				m_ethernet->reset();
			}
			/* toggle bit 3 low to reset the VBLANK */
			if (!(data & (1 << 3)))
			{
				m_sio_irq_state &= ~0x20;
				update_sio_irqs();
			}
			m_sio_reset_ctrl = data;
			break;
		case 1:
			// Interrupt Enable
			// Bit 0 => SIO Watchdog
			// Bit 1 => A/D Converter
			// Bit 2 => IOASIC
			// Bit 3 => NSS / Hi-Link
			// Bit 4 => Ethernet
			// Bit 5 => Vsync
			LOGMASKED(LOG_SIO, "sio_w: Interrupt Enable 0x%02x %s\n", data, sioIRQString(data).c_str());
			m_sio_irq_enable = data;
			update_sio_irqs();
			break;
		case 4:
			// LED
			LOGMASKED(LOG_SIO, "sio_w: LED offset: %08x index: %d data: %02X\n", offset, index, data);
			m_sio_led_state = data;
			break;
		case 6:
			// CMOS Unlock
			m_cmos_unlocked = 1;
			break;
		case 7:
			// Watchdog
			m_timekeeper->watchdog_write();
			LOGMASKED(LOG_SIO_VERBOSE, "sio_w: Watchdog: %08x index: %d data: %02X\n", offset, index, data);
			//m_maincpu->eat_cycles(100);
			break;
		}
	}
}

/*************************************
 *
 *  CPU IO accesses
 *
 *************************************/

void vegas_state::cpu_io_w(offs_t offset, uint8_t data)
{
	// 0: system LED
	// 1: PLD Config / Clock Gen
	// 2: PLD Status / Jammma Serial Sense Bit 7:4=>Revision, Bit 1=>Busy, Bit 0=>Config Done
	// 3: System Reset Bit 0=>enable sio, Bit 1=>enable ide, Bit 2=>enable PCI
	m_cpuio_data[offset] = data;
	switch (offset) {
	case 0: {
		m_system_led = ~data & 0xff;
		char digit = 'U';
		switch (data & 0xff) {
		case 0xc0: digit = '0'; break;
		case 0xf9: digit = '1'; break;
		case 0xa4: digit = '2'; break;
		case 0xb0: digit = '3'; break;
		case 0x99: digit = '4'; break;
		case 0x92: digit = '5'; break;
		case 0x82: digit = '6'; break;
		case 0xf8: digit = '7'; break;
		case 0x80: digit = '8'; break;
		case 0x90: digit = '9'; break;
		case 0x88: digit = 'A'; break;
		case 0x83: digit = 'B'; break;
		case 0xc6: digit = 'C'; break;
		case 0xa7: digit = 'c'; break;
		case 0xa1: digit = 'D'; break;
		case 0x86: digit = 'E'; break;
		case 0x87: digit = 'F'; break;
		case 0x7f: digit = '.'; break;
		case 0xf7: digit = '_'; break;
		case 0xbf: digit = '|'; break;
		case 0xfe: digit = '-'; break;
		case 0xff: digit = 'Z'; break;
		}
		//popmessage("System LED: %c", digit);
		LOGMASKED(LOG_SIO, "%s: cpu_io_w System LED offset %X = %02X '%c'\n", machine().describe_context(), offset, data, digit);
		break;
	}
	case 1:
		m_cpuio_data[2] = (m_cpuio_data[2] & ~0x02) | ((m_cpuio_data[1] & 0x01) << 1) | (m_cpuio_data[1] & 0x01);
		if (!(data & 0x1)) {
			// Need to clear this register while programming SIO FPGA so that fpga config data doesn't register in sio_w
			m_cpuio_data[3] &= ~0x1;
			// Reset the SIO registers
			reset_sio();
		}
		LOGMASKED(LOG_SIO, "%s: cpu_io_w PLD Config offset %X = %02X\n", machine().describe_context(), offset, data);
		break;
	case 2:
		if (m_cpuio_data[3] & 0x1)
			LOGMASKED(LOG_SIO, "%s: cpu_io_w PLD Status / Jamma Serial Sense offset %X = %02X\n", machine().describe_context(), offset, data);
		break;
	case 3:
		// Bit 0: Enable SIO, Bit 1: Enable SIO_R0/IDE, Bit 2: Enable PCI
		LOGMASKED(LOG_SIO, "%s: cpu_io_w System Reset offset %X = %02X\n", machine().describe_context(), offset, data);
		break;
	default:
		LOGMASKED(LOG_UNKNOWN, "%s: cpu_io_w unknown offset %X = %02X\n", machine().describe_context(), offset, data);
		break;
	}
}

uint8_t vegas_state::cpu_io_r(offs_t offset)
{
	uint32_t result = 0;
	if (offset < 4)
		result = m_cpuio_data[offset];
	if (m_cpuio_data[3] & 0x1)
		LOGMASKED(LOG_SIO, "%s:cpu_io_r offset %X = %02X\n", machine().describe_context(), offset, result);
	return result;
}



/*************************************
 *
 *  Analog input handling
 *
 *************************************/

uint32_t vegas_state::analog_port_r(offs_t offset, uint32_t mem_mask)
{
	//logerror("%s: analog_port_r = %08X & %08X\n", machine().describe_context(), m_pending_analog_read, mem_mask);
	// Clear interrupt
	m_sio_irq_state &= ~0x02;
	if (m_sio_irq_enable & 0x02) {
		update_sio_irqs();
	}
	// TODO: Need to look at the proper shift value for sf2049
	return m_pending_analog_read << (m_a2d_shift << 1);
}


void vegas_state::analog_port_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t shift_data = data >> m_a2d_shift;
	int index = shift_data & 0x7;
	uint8_t currValue = m_io_analog[index].read_safe(0);
	if (!m_wheel_calibrated && ((m_wheel_force > 20) || (m_wheel_force < -20))) {
		if (m_wheel_force > 0 && m_wheel_offset < 128)
			m_wheel_offset++;
		else if (m_wheel_offset > -128)
			m_wheel_offset--;
		int tmpVal = int(currValue) + m_wheel_offset;
		if (tmpVal < m_io_analog[index]->field(0xff)->minval())
			m_pending_analog_read = m_io_analog[index]->field(0xff)->minval();
		else if (tmpVal > m_io_analog[index]->field(0xff)->maxval())
			m_pending_analog_read = m_io_analog[index]->field(0xff)->maxval();
		else
			m_pending_analog_read = tmpVal;
	}
	else {
		m_pending_analog_read = currValue;
	}
	// Declare calibration finished as soon as a SYSTEM button is hit
	if (!m_wheel_calibrated && ((~m_io_system->read()) & 0xffff)) {
		m_wheel_calibrated = true;
		//osd_printf_info("wheel calibration comlete wheel: %02x\n", currValue);
	}
	//logerror("analog_port_w: wheel_force: %i read: %i\n", m_wheel_force, m_pending_analog_read);
	//logerror("%s: analog_port_w = %08X & %08X index = %d\n", machine().describe_context(), data, mem_mask, index);
	if (m_sio_irq_enable & 0x02) {
		m_sio_irq_state |= 0x02;
		update_sio_irqs();
	}
}



/*************************************
 *
 *  Misc accesses
 *
 *************************************/

void vegas_state::asic_fifo_w(uint32_t data)
{
	m_ioasic->fifo_w(data);
}

uint32_t vegas_state::ethernet_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;
	if (ACCESSING_BITS_0_15)
		result |= m_ethernet->read(offset * 2 + 0, mem_mask);
	if (ACCESSING_BITS_16_31)
		result |= m_ethernet->read(offset * 2 + 1, mem_mask >> 16) << 16;
	//logerror("ethernet_r: offset %08x = %08x & %08x\n", offset, result, mem_mask);
	return result;
}


void vegas_state::ethernet_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_15)
		m_ethernet->write(offset * 2 + 0, data, mem_mask);
	if (ACCESSING_BITS_16_31)
		m_ethernet->write(offset * 2 + 1, data >> 16, mem_mask >> 16);
	//logerror("ethernet_w: offset %08x = %08x & %08x\n", offset, data, mem_mask);
}


void vegas_state::dcs3_fifo_full_w(uint32_t data)
{
	m_ioasic->fifo_full_w(data);
}

uint32_t vegas_state::unknown_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0xffffffff;
	if (1)
		logerror("unknown_r: offset: %08X data: %08X mask: %08X\n", offset * 4, result, mem_mask);
	return result;
}

/*************************************
* Parallel Port
*************************************/
uint8_t vegas_state::parallel_r(offs_t offset)
{
	uint8_t result = 0x7;
	logerror("%s: parallel_r %08x = %02x\n", machine().describe_context(), offset, result);
	return result;
}

void vegas_state::parallel_w(offs_t offset, uint8_t data)
{
	logerror("%s: parallel_w %08x = %02x\n", machine().describe_context(), offset, data);
}

/*************************************
* MPS Reset
*************************************/
void vegas_state::mpsreset_w(offs_t offset, uint8_t data)
{
	logerror("%s: mpsreset_w %08x = %02x\n", machine().describe_context(), offset, data);
}

/*************************************
* 49 Way translation matrix
*************************************/
const uint8_t vegas_state::translate49[7] = { 0x8, 0xc, 0xe, 0xf, 0x3, 0x1, 0x0 };

/*************************************
* Optical 49 Way Joystick I40 Board
*************************************/
void vegas_state::i40_w(uint32_t data)
{
	//printf("i40_w: data = %08x\n", data);
	//logerror("i40_w: data = %08x\n", data);
	m_i40_data = data;
}

ioport_value vegas_state::i40_r()
{
	if (m_io_dips->read() & 0x100) {
		// 8 way joysticks
		return m_io_8way[3]->read();
	}
	else {
		// 49 way joysticks via i40 adapter board
		int index = m_i40_data & 0xf;
		uint8_t data = 0;
		switch (index) {
		case 0:
			data = translate49[m_io_49way_x[0]->read() >> 4];
			break;
		case 1:
			data = translate49[m_io_49way_y[0]->read() >> 4];
			break;
		case 2:
			data = translate49[m_io_49way_x[1]->read() >> 4];
			break;
		case 3:
			data = translate49[m_io_49way_y[1]->read() >> 4];
			break;
		case 4:
			data = translate49[m_io_49way_x[2]->read() >> 4];
			break;
		case 5:
			data = translate49[m_io_49way_y[2]->read() >> 4];
			break;
		case 6:
			data = translate49[m_io_49way_x[3]->read() >> 4];
			break;
		case 7:
			data = translate49[m_io_49way_y[3]->read() >> 4];
			break;
		case 10:
		case 11:
		case 12:
			// I40 Detection
			data = ~index & 0xf;
			break;
		default:
			//logerror("%s: i40_r: select: %x index: %d data: %x\n", machine().describe_context(), m_i40_data, index, data);
			break;
		}
		//if (m_i40_data & 0x1000)
		//  printf("%s: i40_r: select: %x index: %d data: %x\n", machine().describe_context().c_str(), m_i40_data, index, data);
		//m_i40_data &= ~0x1000;
		return data;
	}
}

/*************************************
* Gauntlet Player 1 & 2 control read
*************************************/
ioport_value vegas_state::gauntleg_p12_r()
{
	if (m_io_dips->read() & 0x2000) {
		// 8 way joysticks
		return m_io_8way[1]->read() | m_io_8way[0]->read();
	}
	else {
		// 49 way joysticks
		return  (translate49[m_io_49way_x[1]->read() >> 4] << 12) | (translate49[m_io_49way_y[1]->read() >> 4] << 8) |
				(translate49[m_io_49way_x[0]->read() >> 4] << 4) |  (translate49[m_io_49way_y[0]->read() >> 4] << 0);
	}
}

/*************************************
* Gauntlet Player 3 & 4 control read
*************************************/
ioport_value vegas_state::gauntleg_p34_r()
{
	if (m_io_dips->read() & 0x2000) {
		// 8 way joysticks
		return m_io_8way[3]->read() | m_io_8way[2]->read();
	}
	else {
		// 49 way joysticks
		return  (translate49[m_io_49way_x[3]->read() >> 4] << 12) | (translate49[m_io_49way_y[3]->read() >> 4] << 8) |
				(translate49[m_io_49way_x[2]->read() >> 4] << 4) |  (translate49[m_io_49way_y[2]->read() >> 4] << 0);
	}
}

/*************************************
* Keypad
*************************************/
void vegas_state::wheel_board_w(uint32_t data)
{
	bool chip_select = BIT(data, 11);
	bool latch_clk = BIT(data, 10);
	uint8_t op = (data >> 8) & 0x3;
	uint8_t arg = data & 0xff;

	//logerror("wheel_board_w: data = %08x op: %02x arg: %02x\n", data, op, arg);

	if (chip_select && latch_clk) {
		switch (op) {
		case 0x0:
			m_wheel_driver[0] = arg; // target wheel angle. signed byte.
			m_wheel_force = int8_t(~arg);
			break;

		case 0x1:
			for (uint8_t bit = 0; bit < 8; bit++)
				m_lamps[bit] = BIT(arg, bit);

			// leader lamp bit is included in every write, for some reason.
			m_lamps[8] = BIT(data, 12);
			break;

		case 0x2:
			m_keypad_select = arg;
			break;
		}
	}
}

ioport_value vegas_state::keypad_r()
{
	int row_sel;
	for (row_sel=0; row_sel<4; row_sel++) {
		if (!(m_keypad_select & (1 << row_sel)))
			break;
	}
	if (row_sel <= 3) {
		uint32_t bits = m_io_keypad->read();
		bits >>= row_sel * 3;
		return bits & 0x7;
	}
	else
		return 0x7;
}

/*************************************
*
*  Gearshift
*
*************************************/
ioport_value vegas_state::gearshift_r()
{
	// Check for gear change and save gear selection
	uint32_t gear = m_io_gearshift->read();
	for (int i = 0; i < 4; i++) {
		if (gear & (1 << i))
			m_gear = 1 << i;
	}
	return m_gear;
}

/*************************************
 *
 * Common Input ports
 *
 *************************************/

static INPUT_PORTS_START( vegas_common )
	PORT_START("DIPS")
	PORT_DIPNAME( 0x0001, 0x0001, "Unknown0001" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Unknown0002" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Unknown0004" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Unknown0008" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Unknown0010" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Unknown0020" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Unknown0040" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Unknown0080" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "Unknown0100" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Unknown0200" )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Unknown0400" )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Unknown0800" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Unknown1000" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Unknown2000" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Unknown4000" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Unknown8000" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT )
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BILL1 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)

	PORT_START("AN.0") PORT_BIT( 0xff, 0x80, IPT_CUSTOM )
	PORT_START("AN.1") PORT_BIT( 0xff, 0x80, IPT_CUSTOM )
	PORT_START("AN.2") PORT_BIT( 0xff, 0x80, IPT_CUSTOM )
	PORT_START("AN.3") PORT_BIT( 0xff, 0x80, IPT_CUSTOM )
	PORT_START("AN.4") PORT_BIT( 0xff, 0x80, IPT_CUSTOM )
	PORT_START("AN.5") PORT_BIT( 0xff, 0x80, IPT_CUSTOM )
	PORT_START("AN.6") PORT_BIT( 0xff, 0x80, IPT_CUSTOM )
	PORT_START("AN.7") PORT_BIT( 0xff, 0x80, IPT_CUSTOM )

	PORT_START("8WAY_P1")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("8WAY_P2")
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("8WAY_P3")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("8WAY_P4")
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( vegas_analog )
	PORT_INCLUDE(vegas_common)

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start Button")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED  )
INPUT_PORTS_END

/*************************************
 *
 * Game specific Input ports
 *
 *************************************/

static INPUT_PORTS_START( gauntleg )
	PORT_INCLUDE(vegas_common)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0001, 0x0001, "PM Dump" )
	PORT_DIPSETTING(      0x0001, "Watchdog resets only" )
	PORT_DIPSETTING(      0x0000, "All resets" )
	PORT_DIPNAME( 0x0002, 0x0002, "Quantum 3dfx card rev" )
	PORT_DIPSETTING(      0x0002, "4 or later" )
	PORT_DIPSETTING(      0x0000, "3 or earlier" )
	PORT_DIPNAME( 0x0004, 0x0004, "DRAM" )
	PORT_DIPSETTING(      0x0004, "8MB" )
	PORT_DIPSETTING(      0x0000, "32MB" )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Test Mode" )
	PORT_DIPSETTING(      0x00c0, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, "Disk-based Test" )
	PORT_DIPSETTING(      0x0080, "EPROM-based Test" )
	PORT_DIPSETTING(      0x0000, "Interactive Diagnostics" )
	PORT_DIPNAME( 0x0800, 0x0800, "SIO Rev" )
	PORT_DIPSETTING(      0x0800, "1 or later")
	PORT_DIPSETTING(      0x0000, "0")
	PORT_DIPNAME( 0x1000, 0x1000, "Harness" )
	PORT_DIPSETTING(      0x1000, "JAMMA" )
	PORT_DIPSETTING(      0x0000, "Midway" )
	PORT_DIPNAME( 0x2000, 0x2000, "Joysticks" )
	PORT_DIPSETTING(      0x2000, "8-Way" )
	PORT_DIPSETTING(      0x0000, "49-Way" )
	PORT_DIPNAME( 0xc000, 0x4000, "Resolution" )
	PORT_DIPSETTING(      0xc000, "Standard Res 512x256" )
	PORT_DIPSETTING(      0x4000, "Medium Res 512x384" )
	PORT_DIPSETTING(      0x0000, "VGA Res 640x480" ) //VGA res not supported for gauntleg

	PORT_MODIFY("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_CUSTOM  ) PORT_CUSTOM_MEMBER(FUNC(vegas_state::gauntleg_p12_r))

	PORT_MODIFY("IN2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_CUSTOM  ) PORT_CUSTOM_MEMBER(FUNC(vegas_state::gauntleg_p34_r))

	PORT_MODIFY("8WAY_P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Fight")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Magic")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Turbo")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("8WAY_P2")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Fight")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Magic")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Turbo")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("8WAY_P3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_NAME("P3 Fight")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(3) PORT_NAME("P3 Magic")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("P3 Turbo")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("8WAY_P4")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(4) PORT_NAME("P4 Fight")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(4) PORT_NAME("P4 Magic")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4) PORT_NAME("P4 Turbo")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("49WAYX_P1")
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("49WAYY_P1")
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1) PORT_REVERSE

	PORT_START("49WAYX_P2")
	PORT_BIT(0xff, 0x38, IPT_AD_STICK_X) PORT_MINMAX(0x00, 0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("49WAYY_P2")
	PORT_BIT(0xff, 0x38, IPT_AD_STICK_Y) PORT_MINMAX(0x00, 0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2) PORT_REVERSE

	PORT_START("49WAYX_P3")
	PORT_BIT(0xff, 0x38, IPT_AD_STICK_X) PORT_MINMAX(0x00, 0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(3)

	PORT_START("49WAYY_P3")
	PORT_BIT(0xff, 0x38, IPT_AD_STICK_Y) PORT_MINMAX(0x00, 0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(3) PORT_REVERSE

	PORT_START("49WAYX_P4")
	PORT_BIT(0xff, 0x38, IPT_AD_STICK_X) PORT_MINMAX(0x00, 0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(4)

	PORT_START("49WAYY_P4")
	PORT_BIT(0xff, 0x38, IPT_AD_STICK_Y) PORT_MINMAX(0x00, 0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(4) PORT_REVERSE
INPUT_PORTS_END

static INPUT_PORTS_START( tenthdeg )
	PORT_INCLUDE(vegas_common)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0040, 0x0040, "Boot ROM Test" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0xc000, 0xc000, "Resolution" )
	PORT_DIPSETTING(      0xc000, "Standard Res 512x256" )
	PORT_DIPSETTING(      0x4000, "Medium Res 512x384" )
	PORT_DIPSETTING(      0x0000, "VGA Res 640x480" ) //VGA res not supported for this game

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("P1 Counter")

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Jab")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Strong")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Short")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Jab")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Strong")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Short")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Roundhouse")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Fierce")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 Forward")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Roundhouse")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Forward")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Fierce")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_NAME("P2 Counter")
	PORT_BIT( 0xe000, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


static INPUT_PORTS_START( warfa )
	PORT_INCLUDE(vegas_analog)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0001, 0x0001, "PM Dump" )
	PORT_DIPSETTING( 0x0001, "Watchdog resets only" )
	PORT_DIPSETTING( 0x0000, "All resets" )
	PORT_DIPNAME( 0x0002, 0x0002, "Quantum 3dfx card rev" )
	PORT_DIPSETTING( 0x0002, "4 or later" )
	PORT_DIPSETTING( 0x0000, "3 or earlier" )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Test Mode" )
	PORT_DIPSETTING(      0x00c0, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, "Disk-based Test" )
	PORT_DIPSETTING(      0x0080, "EPROM-based Test" )
	PORT_DIPSETTING(      0x0000, "Interactive Diagnostics" )
	PORT_DIPNAME( 0xc000, 0xc000, "Resolution" )
	PORT_DIPSETTING( 0xc000, "Standard Res 512x256" )
	PORT_DIPSETTING( 0x4000, "Medium Res 512x384" )
	PORT_DIPSETTING( 0x0000, "VGA Res 640x480" )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_J) PORT_NAME("Trigger")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_K) PORT_NAME("Discard")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_L) PORT_NAME("Jump")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_U) PORT_NAME("View")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_CODE(KEYCODE_W) PORT_NAME("Forward")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_CODE(KEYCODE_S) PORT_NAME("Back")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_CODE(KEYCODE_A) PORT_NAME("Dodge Left")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_CODE(KEYCODE_D) PORT_NAME("Dodge Right")
	PORT_BIT( 0xe000, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("AN.0")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_MODIFY("AN.1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( roadburn )
	PORT_INCLUDE(vegas_analog)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0003, 0x0003, "Test Mode" )
	PORT_DIPSETTING(      0x0003, "Off" )
	PORT_DIPSETTING(      0x0002, "Disk-based Test" )
	PORT_DIPSETTING(      0x0001, "EPROM-based Test" )
	PORT_DIPSETTING(      0x0000, "Interactive Diagnostics" )
	PORT_DIPNAME( 0x0300, 0x0000, "Resolution" )
	PORT_DIPSETTING(      0x0300, "Standard Res 512x256" )
	PORT_DIPSETTING(      0x0200, "Medium Res 512x384" )
	PORT_DIPSETTING(      0x0000, "VGA Res 640x480" )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Brake")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Music")
	PORT_BIT( 0xff40, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("AN.0")
	PORT_BIT( 0xff, 0x80, IPT_PEDAL ) PORT_NAME("Accel") PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(20)

	PORT_MODIFY("AN.1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_NAME("Steering") PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_MODIFY("AN.2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_NAME("Bank") PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)
INPUT_PORTS_END


static INPUT_PORTS_START( nbashowt )
	PORT_INCLUDE(vegas_common)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0001, 0x0000, "Coinage Source" )
	PORT_DIPSETTING(      0x0001, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x000e, "Mode 1" )
	PORT_DIPSETTING(      0x000c, "Mode 2" )
	PORT_DIPSETTING(      0x000a, "Mode 3" )
	PORT_DIPSETTING(      0x0008, "Mode 4" )
	PORT_DIPSETTING(      0x0006, "Mode 5")
	PORT_DIPSETTING(      0x0004, "Mode 6")
	PORT_DIPSETTING(      0x0002, "Mode 7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0030, 0x0030, "Currency Type" )
	PORT_DIPSETTING(      0x0030, DEF_STR( USA ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( French ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( German ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Select Game" )
	PORT_DIPSETTING(      0x0040, DEF_STR(Yes))
	PORT_DIPSETTING(      0x0000, DEF_STR(No))
	PORT_DIPNAME( 0x0080, 0x0000, "Game Powerup" )
	PORT_DIPSETTING(      0x0080, "NFL Blitz" )
	PORT_DIPSETTING(      0x0000, "NBA Showtime" )
	PORT_DIPNAME( 0x0100, 0x0000, "Joysticks" )
	PORT_DIPSETTING(      0x0100, "8-Way" )
	PORT_DIPSETTING(      0x0000, "49-Way" )
	PORT_DIPNAME( 0x0200, 0x0200, "Graphics Mode" )
	PORT_DIPSETTING(      0x0200, "Med Res" )
	PORT_DIPSETTING(      0x0000, "Low Res" )
	PORT_DIPUNUSED(       0x1c00, 0x1c00 )
	PORT_DIPNAME( 0x2000, 0x2000, "Number of Players" )
	PORT_DIPSETTING(      0x2000, "2" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x4000, 0x0000, "Power On Self Test" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Test Switch" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 A")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 B")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Turbo")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 A")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 B")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Turbo")

	PORT_MODIFY("IN2")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("P3 A")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_NAME("P3 B")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3) PORT_NAME("P3 Turbo")
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(vegas_state::i40_r))
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4) PORT_NAME("P4 A")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4) PORT_NAME("P4 B")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4) PORT_NAME("P4 Turbo")

	PORT_MODIFY("8WAY_P4")
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x8, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_8WAY

	PORT_START("49WAYX_P1")
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_X ) PORT_MINMAX(0x00, 0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("49WAYY_P1")
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_Y ) PORT_MINMAX(0x00, 0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("49WAYX_P2")
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_X ) PORT_MINMAX(0x00, 0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("49WAYY_P2")
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_Y ) PORT_MINMAX(0x00, 0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("49WAYX_P3")
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_X ) PORT_MINMAX(0x00, 0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(3)

	PORT_START("49WAYY_P3")
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_Y ) PORT_MINMAX(0x00, 0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(3)

	PORT_START("49WAYX_P4")
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_X ) PORT_MINMAX(0x00, 0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(4)

	PORT_START("49WAYY_P4")
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_Y ) PORT_MINMAX(0x00, 0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(4)
INPUT_PORTS_END


static INPUT_PORTS_START( sf2049 )
	PORT_INCLUDE(vegas_analog)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0003, 0x0003, "Test Mode" )
	PORT_DIPSETTING(      0x0003, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, "Disk-based Test" )
	PORT_DIPSETTING(      0x0001, "EPROM-based Test" )
	PORT_DIPSETTING(      0x0000, "Interactive Diagnostics" )
	PORT_DIPNAME( 0x0080, 0x0080, "PM Dump" )
	PORT_DIPSETTING(      0x0080, "Watchdog resets only" )
	PORT_DIPSETTING(      0x0000, "All resets" )
	PORT_DIPNAME( 0x0300, 0x0000, "Resolution" )
	PORT_DIPSETTING(      0x0300, "Standard Res 512x256" )
	PORT_DIPSETTING(      0x0200, "Medium Res 512x384" )
	PORT_DIPSETTING(      0x0000, "VGA Res 640x480" )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Abort")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Reverse")

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("View 1")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("View 2")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("View 3")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_NAME("Music")
	PORT_BIT( 0x0070, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(vegas_state::keypad_r))
	PORT_BIT( 0x0f00, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(vegas_state::gearshift_r))
	PORT_BIT( 0xf080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("GEAR")
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("1st Gear")
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("2nd Gear")
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("3rd Gear")
	PORT_BIT( 0x8, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("4th Gear")

	PORT_START("KEYPAD")
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK)) PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD)) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD)) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad #") PORT_CODE(KEYCODE_PLUS_PAD)

	PORT_MODIFY("AN.2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_NAME("Accel") PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_MODIFY("AN.3")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL3 ) PORT_NAME("Clutch") PORT_SENSITIVITY(25) PORT_KEYDELTA(100)

	PORT_MODIFY("AN.6")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_NAME("Brake") PORT_SENSITIVITY(25) PORT_KEYDELTA(100)

	PORT_MODIFY("AN.7")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_NAME("Steering") PORT_MINMAX(0x10, 0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)
INPUT_PORTS_END

static INPUT_PORTS_START( sf2049se )
	PORT_INCLUDE(sf2049)

	PORT_MODIFY("DIPS")
	PORT_DIPUNUSED( 0x001e, 0x001e )
	PORT_DIPNAME( 0x0020, 0x0020, "Console Enable" )
	PORT_DIPSETTING(      0x0020, DEF_STR(No))
	PORT_DIPSETTING(      0x0000, DEF_STR(Yes))
	PORT_DIPNAME( 0x00c0, 0x00c0, "Test Mode" )
	PORT_DIPSETTING(      0x00c0, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, "EPROM-based Test" )
	PORT_DIPSETTING(      0x0040, "Disk-based Test" )
	PORT_DIPSETTING(      0x0000, "Interactive Diagnostics" )
	PORT_DIPNAME( 0x0001, 0x0001, "PM Dump" )
	PORT_DIPSETTING(      0x0001, "Watchdog resets only" )
	PORT_DIPSETTING(      0x0000, "All resets" )
	PORT_DIPUNUSED( 0x1f00, 0x1f00 )
	PORT_DIPNAME( 0xc000, 0x8000, "Resolution" )
	PORT_DIPSETTING(      0xc000, "Standard Res 512x256" )
	PORT_DIPSETTING(      0x4000, "Medium Res 512x384" )
	PORT_DIPSETTING(      0x8000, "VGA Res 640x480" )
	PORT_DIPNAME( 0x2000, 0x2000, "Cabinet Type" )
	PORT_DIPSETTING(      0x2000, "Sit down cabinet" )
	PORT_DIPSETTING(      0x0000, "Upright cabinet" )
INPUT_PORTS_END

static INPUT_PORTS_START( cartfury )
	PORT_INCLUDE(vegas_analog)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0001, 0x0000, "Coinage Source" )
	PORT_DIPSETTING(      0x0001, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x000e, "Mode 1" )
	PORT_DIPSETTING(      0x000c, "Mode 2" )
	PORT_DIPSETTING(      0x000a, "Mode 3" )
	PORT_DIPSETTING(      0x0008, "Mode 4" )
	PORT_DIPSETTING(      0x0006, "Mode 5")
	PORT_DIPSETTING(      0x0004, "Mode 6")
	PORT_DIPSETTING(      0x0002, "Mode 7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0030, 0x0030, "Currency Type" )
	PORT_DIPSETTING(      0x0030, DEF_STR( USA ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( French ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( German ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Disable Brake" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Test Switch" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("View 1")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("View 2")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("View 3")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_NAME("Boost")
	PORT_BIT( 0x0f00, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(vegas_state::gearshift_r))
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("GEAR")
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("1st Gear")
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("2nd Gear")
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("3rd Gear")
	PORT_BIT( 0x8, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("4th Gear")

	PORT_MODIFY("AN.0")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_NAME("Steering") PORT_MINMAX(0x10, 0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_MODIFY("AN.1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_NAME("Accel") PORT_SENSITIVITY(25) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_MODIFY("AN.2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_NAME("Brake") PORT_SENSITIVITY(25) PORT_KEYDELTA(100) PORT_PLAYER(1)
INPUT_PORTS_END

/*************************************
*
*  Memory maps
*
*************************************/
void vegas_state::vegas_cs2_map(address_map &map)
{
	map(0x00000000, 0x00007003).rw(FUNC(vegas_state::sio_r), FUNC(vegas_state::sio_w));
}

void vegas_state::vegas_cs3_map(address_map &map)
{
	map(0x00000000, 0x00000003).rw(FUNC(vegas_state::analog_port_r), FUNC(vegas_state::analog_port_w));
//  map(0x00001000, 0x00001003).rw(FUNC(vegas_state::lcd_r), FUNC(vegas_state::lcd_w));
}

void vegas_state::vegas_cs4_map(address_map &map)
{
	map(0x00000000, 0x00007fff).rw(FUNC(vegas_state::timekeeper_r), FUNC(vegas_state::timekeeper_w));
}

void vegas_state::vegas_cs5_map(address_map &map)
{
	map(0x00000000, 0x00000003).rw(FUNC(vegas_state::cpu_io_r), FUNC(vegas_state::cpu_io_w));
	map(0x00100000, 0x001fffff).r(FUNC(vegas_state::unknown_r));
}

void vegas_state::vegas_cs6_map(address_map &map)
{
	map(0x00000000, 0x0000003f).rw(m_ioasic, FUNC(midway_ioasic_device::packed_r), FUNC(midway_ioasic_device::packed_w));
	map(0x00001000, 0x00001003).w(FUNC(vegas_state::asic_fifo_w));
	map(0x00003000, 0x00003003).w(FUNC(vegas_state::dcs3_fifo_full_w));  // if (m_dcs_idma_cs != 0)
	map(0x00005000, 0x00005003).w(m_dcs, FUNC(dcs_audio_device::dsio_idma_addr_w)); // if (m_dcs_idma_cs == 6)
	map(0x00007000, 0x00007003).rw(m_dcs, FUNC(dcs_audio_device::dsio_idma_data_r), FUNC(dcs_audio_device::dsio_idma_data_w)); // if (m_dcs_idma_cs == 6)
}

void vegas_state::vegas_cs7_map(address_map &map)
{
//  map(0x00000000, 0x00000003).rw(FUNC(vegas_state::nss_r), FUNC(vegas_state::nss_w));
	map(0x00001000, 0x0000100f).rw(FUNC(vegas_state::ethernet_r), FUNC(vegas_state::ethernet_w));
	map(0x00005000, 0x00005003).w(m_dcs, FUNC(dcs_audio_device::dsio_idma_addr_w)); // if (m_dcs_idma_cs == 7)
	map(0x00007000, 0x00007003).rw(m_dcs, FUNC(dcs_audio_device::dsio_idma_data_r), FUNC(dcs_audio_device::dsio_idma_data_w)); // if (m_dcs_idma_cs == 7)
}

void vegas_state::vegas_cs8_map(address_map &map)
{
	map(0x01000000, 0x0100001f).rw(m_uart1, FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w)).umask32(0x000000ff); // Serial ttyS01 (TL16C552 CS0)
	map(0x01400000, 0x0140001f).rw(m_uart2, FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w)).umask32(0x000000ff); // Serial ttyS02 (TL16C552 CS1)
	map(0x01800000, 0x0180001f).rw(FUNC(vegas_state::parallel_r), FUNC(vegas_state::parallel_w)).umask32(0x000000ff); // Parallel UART (TL16C552 CS2)
	map(0x01c00000, 0x01c00000).w(FUNC(vegas_state::mpsreset_w)); // MPS Reset
}

/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void vegas_state::vegascore(machine_config &config)
{
	// basic machine hardware
	R5000LE(config, m_maincpu, vegas_state::SYSTEM_CLOCK * 2);
	m_maincpu->set_icache_size(0x4000);
	m_maincpu->set_dcache_size(0x4000);
	m_maincpu->set_system_clock(vegas_state::SYSTEM_CLOCK);

	// PCI Bus Devices
	PCI_ROOT(config, "pci", 0);

	VRC5074(config, m_nile, SYSTEM_CLOCK, m_maincpu);
	m_nile->set_sdram_size(0, 0x00800000);
	m_nile->set_map(2, address_map_constructor(&vegas_state::vegas_cs2_map, "vegas_cs2_map", this), this);
	m_nile->set_map(3, address_map_constructor(&vegas_state::vegas_cs3_map, "vegas_cs3_map", this), this);
	m_nile->set_map(4, address_map_constructor(&vegas_state::vegas_cs4_map, "vegas_cs4_map", this), this);
	m_nile->set_map(5, address_map_constructor(&vegas_state::vegas_cs5_map, "vegas_cs5_map", this), this);
	m_nile->set_map(6, address_map_constructor(&vegas_state::vegas_cs6_map, "vegas_cs6_map", this), this);
	m_nile->set_map(7, address_map_constructor(&vegas_state::vegas_cs7_map, "vegas_cs7_map", this), this);

	ide_pci_device &ide(IDE_PCI(config, PCI_ID_IDE, 0, 0x10950646, 0x05, 0x0));
	ide.irq_handler().set(PCI_ID_NILE, FUNC(vrc5074_device::pci_intr_d));
	//ide.set_pif(0x8f);
	ide.subdevice<bus_master_ide_controller_device>("ide")->slot(0).set_option_machine_config("hdd", hdd_config);

	// video hardware
	voodoo_2_pci_device &voodoo(VOODOO_2_PCI(config, PCI_ID_VIDEO, 0, m_maincpu, "screen"));
	voodoo.set_fbmem(2);
	voodoo.set_tmumem(4, 4);
	voodoo.set_status_cycles(1000); // optimization to consume extra cycles when polling status
	subdevice<generic_voodoo_device>(PCI_ID_VIDEO":voodoo")->vblank_callback().set(FUNC(vegas_state::vblank_assert));

	M48T37(config, m_timekeeper);
	m_timekeeper->reset_cb().set(FUNC(vegas_state::watchdog_reset));
	m_timekeeper->irq_cb().set(FUNC(vegas_state::watchdog_irq));

	SMC91C94(config, m_ethernet, 0);
	m_ethernet->irq_handler().set(FUNC(vegas_state::ethernet_interrupt));

	// screen
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	// Screeen size and timing is re-calculated later in voodoo card
	screen.set_refresh_hz(57);
	screen.set_size(640, 480);
	screen.set_visarea(0, 640 - 1, 0, 480 - 1);
	screen.set_screen_update(PCI_ID_VIDEO, FUNC(voodoo_pci_device::screen_update));
}

void vegas_state::hdd_config(device_t *device)
{
	// Set the disk dma transfer speed
	dynamic_cast<ide_hdd_device *>(device)->set_dma_transfer_time(attotime::from_usec(15));
	// Allow ultra dma
	//uint16_t *identify_device = dynamic_cast<ide_hdd_device *>(device)->identify_device_buffer();
	//identify_device[88] = 0x7f;
}


void vegas_state::vegas(machine_config &config)
{
	vegascore(config);
}


void vegas_state::vegas250(machine_config &config)
{
	vegascore(config);
	m_maincpu->set_clock(vegas_state::SYSTEM_CLOCK*2.5);
}


void vegas_state::vegas32m(machine_config &config)
{
	vegas250(config);
	m_nile->set_sdram_size(0, 0x02000000);
}


void vegas_state::vegasban(machine_config &config)
{
	vegas32m(config);

	voodoo_banshee_pci_device &voodoo(VOODOO_BANSHEE_PCI(config.replace(), PCI_ID_VIDEO, 0, m_maincpu, "screen"));
	voodoo.set_fbmem(16);
	voodoo.set_status_cycles(1000); // optimization to consume extra cycles when polling status
	subdevice<generic_voodoo_device>(PCI_ID_VIDEO":voodoo")->vblank_callback().set(FUNC(vegas_state::vblank_assert));
}


void vegas_state::vegasv3(machine_config &config)
{
	vegas32m(config);

	RM7000LE(config.replace(), m_maincpu, vegas_state::SYSTEM_CLOCK * 2.5);
	m_maincpu->set_icache_size(0x4000);
	m_maincpu->set_dcache_size(0x4000);
	m_maincpu->set_system_clock(vegas_state::SYSTEM_CLOCK);

	voodoo_3_pci_device &voodoo(VOODOO_3_PCI(config.replace(), PCI_ID_VIDEO, 0, m_maincpu, "screen"));
	voodoo.set_fbmem(16);
	voodoo.set_status_cycles(1000); // optimization to consume extra cycles when polling status
	subdevice<generic_voodoo_device>(PCI_ID_VIDEO":voodoo")->vblank_callback().set(FUNC(vegas_state::vblank_assert));
}


void vegas_state::denver(machine_config &config)
{
	vegascore(config);
	RM7000LE(config.replace(), m_maincpu, vegas_state::SYSTEM_CLOCK * 2.5);

	m_maincpu->set_icache_size(0x4000);
	m_maincpu->set_dcache_size(0x4000);
	m_maincpu->set_system_clock(vegas_state::SYSTEM_CLOCK);
	m_nile->set_sdram_size(0, 0x02000000);
	m_nile->set_map(8, address_map_constructor(&vegas_state::vegas_cs8_map, "vegas_cs8_map", this), this);

	voodoo_3_pci_device &voodoo(VOODOO_3_PCI(config.replace(), PCI_ID_VIDEO, 0, m_maincpu, "screen"));
	voodoo.set_fbmem(16);
	voodoo.set_status_cycles(1000); // optimization to consume extra cycles when polling status
	subdevice<generic_voodoo_device>(PCI_ID_VIDEO":voodoo")->vblank_callback().set(FUNC(vegas_state::vblank_assert));

	// TL16C552 UART
	INPUT_MERGER_ANY_HIGH(config, "duart_irq").output_handler().set(FUNC(vegas_state::duart_irq_cb));

	NS16550(config, m_uart1, XTAL(1'843'200));
	m_uart1->out_tx_callback().set("ttys01", FUNC(rs232_port_device::write_txd));
	m_uart1->out_dtr_callback().set("ttys01", FUNC(rs232_port_device::write_dtr));
	m_uart1->out_rts_callback().set("ttys01", FUNC(rs232_port_device::write_rts));
	m_uart1->out_int_callback().set("duart_irq", FUNC(input_merger_device::in_w<0>));

	NS16550(config, m_uart2, XTAL(1'843'200));
	m_uart2->out_tx_callback().set("ttys02", FUNC(rs232_port_device::write_txd));
	m_uart2->out_dtr_callback().set("ttys02", FUNC(rs232_port_device::write_dtr));
	m_uart2->out_rts_callback().set("ttys02", FUNC(rs232_port_device::write_rts));
	m_uart2->out_int_callback().set("duart_irq", FUNC(input_merger_device::in_w<1>));

	rs232_port_device &ttys01(RS232_PORT(config, "ttys01", default_rs232_devices, nullptr));
	ttys01.rxd_handler().set(m_uart1, FUNC(ins8250_uart_device::rx_w));
	ttys01.dcd_handler().set(m_uart1, FUNC(ins8250_uart_device::dcd_w));
	ttys01.dsr_handler().set(m_uart1, FUNC(ins8250_uart_device::dsr_w));
	ttys01.ri_handler().set(m_uart1, FUNC(ins8250_uart_device::ri_w));
	ttys01.cts_handler().set(m_uart1, FUNC(ins8250_uart_device::cts_w));

	rs232_port_device &ttys02(RS232_PORT(config, "ttys02", default_rs232_devices, nullptr));
	ttys02.rxd_handler().set(m_uart2, FUNC(ins8250_uart_device::rx_w));
	ttys02.dcd_handler().set(m_uart2, FUNC(ins8250_uart_device::dcd_w));
	ttys02.dsr_handler().set(m_uart2, FUNC(ins8250_uart_device::dsr_w));
	ttys02.ri_handler().set(m_uart2, FUNC(ins8250_uart_device::ri_w));
	ttys02.cts_handler().set(m_uart2, FUNC(ins8250_uart_device::cts_w));
}

// Per driver configs

void vegas_state::gauntleg(machine_config &config)
{
	// Needs 250MHz MIPS or screen tearing occurs (See MT8064)
	// Firmware frequency detection seems to have a bug, console reports 220MHz for a 200MHz cpu and 260MHz for a 250MHz cpu
	vegas250(config);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DCS2_AUDIO_2104(config, m_dcs, 0);
	m_dcs->set_maincpu_tag(m_maincpu);
	m_dcs->set_dram_in_mb(4);
	m_dcs->set_polling_offset(0x0b5d);
	m_dcs->add_route(0, "rspeaker", 1.0);
	m_dcs->add_route(1, "lspeaker", 1.0);

	MIDWAY_IOASIC(config, m_ioasic, 0);
	m_ioasic->in_port_cb<0>().set_ioport("DIPS");
	m_ioasic->in_port_cb<1>().set_ioport("SYSTEM");
	m_ioasic->in_port_cb<2>().set_ioport("IN1");
	m_ioasic->in_port_cb<3>().set_ioport("IN2");
	m_ioasic->set_dcs_tag(m_dcs);
	m_ioasic->set_shuffle(midway_ioasic_device::SHUFFLE_CALSPEED);
	m_ioasic->set_upper(340); // 340=39", 322=27" others?
	m_ioasic->set_yearoffs(80);
	m_ioasic->irq_handler().set(FUNC(vegas_state::ioasic_irq));
	m_ioasic->set_auto_ack(1);
}

void vegas_state::gauntdl(machine_config &config)
{
	// Needs 250MHz MIPS or screen tearing occurs (See MT8064)
	vegas250(config);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DCS2_AUDIO_2104(config, m_dcs, 0);
	m_dcs->set_maincpu_tag(m_maincpu);
	m_dcs->set_dram_in_mb(4);
	m_dcs->set_polling_offset(0x0b5d);
	m_dcs->add_route(0, "rspeaker", 1.0);
	m_dcs->add_route(1, "lspeaker", 1.0);

	MIDWAY_IOASIC(config, m_ioasic, 0);
	m_ioasic->in_port_cb<0>().set_ioport("DIPS");
	m_ioasic->in_port_cb<1>().set_ioport("SYSTEM");
	m_ioasic->in_port_cb<2>().set_ioport("IN1");
	m_ioasic->in_port_cb<3>().set_ioport("IN2");
	m_ioasic->set_dcs_tag(m_dcs);
	m_ioasic->set_shuffle(midway_ioasic_device::SHUFFLE_GAUNTDL);
	m_ioasic->set_upper(346); // others?
	m_ioasic->set_yearoffs(80);
	m_ioasic->irq_handler().set(FUNC(vegas_state::ioasic_irq));
	m_ioasic->set_auto_ack(1);
}

void vegas_state::warfa(machine_config &config)
{
	vegas250(config);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DCS2_AUDIO_2104(config, m_dcs, 0);
	m_dcs->set_maincpu_tag(m_maincpu);
	m_dcs->set_dram_in_mb(4);
	m_dcs->set_polling_offset(0x0b5d);
	m_dcs->add_route(0, "rspeaker", 1.0);
	m_dcs->add_route(1, "lspeaker", 1.0);

	MIDWAY_IOASIC(config, m_ioasic, 0);
	m_ioasic->in_port_cb<0>().set_ioport("DIPS");
	m_ioasic->in_port_cb<1>().set_ioport("SYSTEM");
	m_ioasic->in_port_cb<2>().set_ioport("IN1");
	m_ioasic->in_port_cb<3>().set_ioport("IN2");
	m_ioasic->set_dcs_tag(m_dcs);
	m_ioasic->set_shuffle(midway_ioasic_device::SHUFFLE_MACE);
	m_ioasic->set_upper(337); // others?
	m_ioasic->set_yearoffs(80);
	m_ioasic->irq_handler().set(FUNC(vegas_state::ioasic_irq));
	m_ioasic->set_auto_ack(1);
}

void vegas_state::tenthdeg(machine_config &config)
{
	vegas(config);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DCS2_AUDIO_2115(config, m_dcs, 0);
	m_dcs->set_maincpu_tag(m_maincpu);
	m_dcs->set_dram_in_mb(4);
	m_dcs->set_polling_offset(0x0afb);
	m_dcs->add_route(0, "rspeaker", 1.0);
	m_dcs->add_route(1, "lspeaker", 1.0);

	MIDWAY_IOASIC(config, m_ioasic, 0);
	m_ioasic->in_port_cb<0>().set_ioport("DIPS");
	m_ioasic->in_port_cb<1>().set_ioport("SYSTEM");
	m_ioasic->in_port_cb<2>().set_ioport("IN1");
	m_ioasic->in_port_cb<3>().set_ioport("IN2");
	m_ioasic->set_dcs_tag(m_dcs);
	m_ioasic->set_shuffle(midway_ioasic_device::SHUFFLE_GAUNTDL);
	m_ioasic->set_upper(330); // others?
	m_ioasic->set_yearoffs(80);
	m_ioasic->irq_handler().set(FUNC(vegas_state::ioasic_irq));
	m_ioasic->set_auto_ack(1);
}

void vegas_state::roadburn(machine_config &config)
{
	vegas32m(config);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DCS2_AUDIO_DSIO(config, m_dcs, 0);
	m_dcs->set_maincpu_tag(m_maincpu);
	m_dcs->set_dram_in_mb(4);
	m_dcs->set_polling_offset(0x0ddd);
	m_dcs->add_route(0, "rspeaker", 1.0);
	m_dcs->add_route(1, "lspeaker", 1.0);

	MIDWAY_IOASIC(config, m_ioasic, 0);
	m_ioasic->in_port_cb<0>().set_ioport("DIPS");
	m_ioasic->in_port_cb<1>().set_ioport("SYSTEM");
	m_ioasic->in_port_cb<2>().set_ioport("IN1");
	m_ioasic->in_port_cb<3>().set_ioport("IN2");
	m_ioasic->set_dcs_tag(m_dcs);
	m_ioasic->set_shuffle(midway_ioasic_device::SHUFFLE_STANDARD);
	m_ioasic->set_upper(325); // others?
	m_ioasic->set_yearoffs(80);
	m_ioasic->irq_handler().set(FUNC(vegas_state::ioasic_irq));
	m_ioasic->set_auto_ack(1);
}

void vegas_state::nbashowt(machine_config &config)
{
	vegasban(config);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DCS2_AUDIO_2104(config, m_dcs, 0);
	m_dcs->set_maincpu_tag(m_maincpu);
	m_dcs->set_dram_in_mb(4);
	m_dcs->set_polling_offset(0x0b5d);
	m_dcs->add_route(0, "rspeaker", 1.0);
	m_dcs->add_route(1, "lspeaker", 1.0);

	MIDWAY_IOASIC(config, m_ioasic, 0);
	m_ioasic->in_port_cb<0>().set_ioport("DIPS");
	m_ioasic->in_port_cb<1>().set_ioport("SYSTEM");
	m_ioasic->in_port_cb<2>().set_ioport("IN1");
	m_ioasic->in_port_cb<3>().set_ioport("IN2");
	m_ioasic->set_dcs_tag(m_dcs);
	m_ioasic->set_shuffle(midway_ioasic_device::SHUFFLE_MACE);
	// 528 494 478 development pic, 487 NBA
	m_ioasic->set_upper(487); // or 478 or 487
	m_ioasic->set_yearoffs(80);
	m_ioasic->irq_handler().set(FUNC(vegas_state::ioasic_irq));
	//m_ioasic->set_auto_ack(1)
	m_ioasic->aux_output_handler().set(FUNC(vegas_state::i40_w));
}

void vegas_state::nbanfl(machine_config &config)
{
	vegasban(config);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DCS2_AUDIO_2104(config, m_dcs, 0);
	m_dcs->set_maincpu_tag(m_maincpu);
	m_dcs->set_dram_in_mb(4);
	m_dcs->set_polling_offset(0x0b5d);
	m_dcs->add_route(0, "rspeaker", 1.0);
	m_dcs->add_route(1, "lspeaker", 1.0);

	MIDWAY_IOASIC(config, m_ioasic, 0);
	m_ioasic->in_port_cb<0>().set_ioport("DIPS");
	m_ioasic->in_port_cb<1>().set_ioport("SYSTEM");
	m_ioasic->in_port_cb<2>().set_ioport("IN1");
	m_ioasic->in_port_cb<3>().set_ioport("IN2");
	m_ioasic->set_dcs_tag(m_dcs);
	m_ioasic->set_shuffle(midway_ioasic_device::SHUFFLE_BLITZ99);
	m_ioasic->set_upper(498); // or 478 or 487
	m_ioasic->set_yearoffs(80);
	m_ioasic->irq_handler().set(FUNC(vegas_state::ioasic_irq));
	//m_ioasic->set_auto_ack(1)
	m_ioasic->aux_output_handler().set(FUNC(vegas_state::i40_w));
}

void vegas_state::nbagold(machine_config &config)
{
	vegasban(config);

	QED5271LE(config.replace(), m_maincpu, vegas_state::SYSTEM_CLOCK * 2.5);
	m_maincpu->set_icache_size(32768);
	m_maincpu->set_dcache_size(32768);
	m_maincpu->set_system_clock(vegas_state::SYSTEM_CLOCK);
	m_nile->set_sdram_size(0, 0x00800000);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DCS2_AUDIO_2104(config, m_dcs, 0);
	m_dcs->set_maincpu_tag(m_maincpu);
	m_dcs->set_dram_in_mb(4);
	m_dcs->set_polling_offset(0x0b5d);
	m_dcs->add_route(0, "rspeaker", 1.0);
	m_dcs->add_route(1, "lspeaker", 1.0);

	MIDWAY_IOASIC(config, m_ioasic, 0);
	m_ioasic->in_port_cb<0>().set_ioport("DIPS");
	m_ioasic->in_port_cb<1>().set_ioport("SYSTEM");
	m_ioasic->in_port_cb<2>().set_ioport("IN1");
	m_ioasic->in_port_cb<3>().set_ioport("IN2");
	m_ioasic->set_dcs_tag(m_dcs);
	m_ioasic->set_shuffle(midway_ioasic_device::SHUFFLE_GAUNTDL);
	m_ioasic->set_upper(109); // 494 109 ???
	m_ioasic->set_yearoffs(80);
	m_ioasic->irq_handler().set(FUNC(vegas_state::ioasic_irq));
	//m_ioasic->set_auto_ack(1)
	m_ioasic->aux_output_handler().set(FUNC(vegas_state::i40_w));
}

void vegas_state::sf2049(machine_config &config)
{
	denver(config);

	SPEAKER(config, "flspeaker").front_left();
	SPEAKER(config, "frspeaker").front_right();
	SPEAKER(config, "rlspeaker").headrest_left();
	SPEAKER(config, "rrspeaker").headrest_right();
	SPEAKER(config, "subwoofer").backrest();

	DCS2_AUDIO_DENVER_5CH(config, m_dcs, 0);
	m_dcs->set_maincpu_tag(m_maincpu);
	m_dcs->set_dram_in_mb(8);
	m_dcs->set_polling_offset(0x872);
	m_dcs->add_route(0, "flspeaker", 1.0);
	m_dcs->add_route(1, "frspeaker", 1.0);
	m_dcs->add_route(2, "rlspeaker", 1.0);
	m_dcs->add_route(3, "rrspeaker", 1.0);
	m_dcs->add_route(4, "subwoofer", 1.0);

	MIDWAY_IOASIC(config, m_ioasic, 0);
	m_ioasic->in_port_cb<0>().set_ioport("DIPS");
	m_ioasic->in_port_cb<1>().set_ioport("SYSTEM");
	m_ioasic->in_port_cb<2>().set_ioport("IN1");
	m_ioasic->in_port_cb<3>().set_ioport("IN2");
	m_ioasic->set_dcs_tag(m_dcs);
	m_ioasic->set_shuffle(midway_ioasic_device::SHUFFLE_STANDARD);
	m_ioasic->set_upper(336); // others?
	m_ioasic->set_yearoffs(80);
	m_ioasic->irq_handler().set(FUNC(vegas_state::ioasic_irq));
	m_ioasic->set_auto_ack(1);
	m_ioasic->aux_output_handler().set(FUNC(vegas_state::wheel_board_w));
}

void vegas_state::sf2049se(machine_config &config)
{
	denver(config);

	SPEAKER(config, "flspeaker").front_left();
	SPEAKER(config, "frspeaker").front_right();
	SPEAKER(config, "rlspeaker").headrest_left();
	SPEAKER(config, "rrspeaker").headrest_right();
	SPEAKER(config, "subwoofer").backrest();

	DCS2_AUDIO_DENVER_5CH(config, m_dcs, 0);
	m_dcs->set_maincpu_tag(m_maincpu);
	m_dcs->set_dram_in_mb(8);
	m_dcs->set_polling_offset(0x872);
	m_dcs->add_route(0, "flspeaker", 1.0);
	m_dcs->add_route(1, "frspeaker", 1.0);
	m_dcs->add_route(2, "rlspeaker", 1.0);
	m_dcs->add_route(3, "rrspeaker", 1.0);
	m_dcs->add_route(4, "subwoofer", 1.0);

	MIDWAY_IOASIC(config, m_ioasic, 0);
	m_ioasic->in_port_cb<0>().set_ioport("DIPS");
	m_ioasic->in_port_cb<1>().set_ioport("SYSTEM");
	m_ioasic->in_port_cb<2>().set_ioport("IN1");
	m_ioasic->in_port_cb<3>().set_ioport("IN2");
	m_ioasic->set_dcs_tag(m_dcs);
	m_ioasic->set_shuffle(midway_ioasic_device::SHUFFLE_SFRUSHRK);
	m_ioasic->set_upper(352); // 352 336 others?
	m_ioasic->set_yearoffs(80);
	m_ioasic->irq_handler().set(FUNC(vegas_state::ioasic_irq));
	m_ioasic->set_auto_ack(1);
	m_ioasic->aux_output_handler().set(FUNC(vegas_state::wheel_board_w));
}

void vegas_state::sf2049te(machine_config &config)
{
	denver(config);

	SPEAKER(config, "flspeaker").front_left();
	SPEAKER(config, "frspeaker").front_right();
	SPEAKER(config, "rlspeaker").headrest_left();
	SPEAKER(config, "rrspeaker").headrest_right();
	SPEAKER(config, "subwoofer").backrest();

	DCS2_AUDIO_DENVER_5CH(config, m_dcs, 0);
	m_dcs->set_maincpu_tag(m_maincpu);
	m_dcs->set_dram_in_mb(8);
	m_dcs->set_polling_offset(0x872);
	m_dcs->add_route(0, "flspeaker", 1.0);
	m_dcs->add_route(1, "frspeaker", 1.0);
	m_dcs->add_route(2, "rlspeaker", 1.0);
	m_dcs->add_route(3, "rrspeaker", 1.0);
	m_dcs->add_route(4, "subwoofer", 1.0);

	MIDWAY_IOASIC(config, m_ioasic, 0);
	m_ioasic->in_port_cb<0>().set_ioport("DIPS");
	m_ioasic->in_port_cb<1>().set_ioport("SYSTEM");
	m_ioasic->in_port_cb<2>().set_ioport("IN1");
	m_ioasic->in_port_cb<3>().set_ioport("IN2");
	m_ioasic->set_dcs_tag(m_dcs);
	m_ioasic->set_shuffle(midway_ioasic_device::SHUFFLE_SFRUSHRK);
	m_ioasic->set_upper(348); // others?
	m_ioasic->set_yearoffs(80);
	m_ioasic->irq_handler().set(FUNC(vegas_state::ioasic_irq));
	m_ioasic->set_auto_ack(1);
	m_ioasic->aux_output_handler().set(FUNC(vegas_state::wheel_board_w));
}

void vegas_state::cartfury(machine_config &config)
{
	vegasv3(config);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DCS2_AUDIO_2104(config, m_dcs, 0);
	m_dcs->set_maincpu_tag(m_maincpu);
	m_dcs->set_dram_in_mb(4);
	m_dcs->set_polling_offset(0x0b5d);
	m_dcs->add_route(0, "rspeaker", 1.0);
	m_dcs->add_route(1, "lspeaker", 1.0);

	MIDWAY_IOASIC(config, m_ioasic, 0);
	m_ioasic->in_port_cb<0>().set_ioport("DIPS");
	m_ioasic->in_port_cb<1>().set_ioport("SYSTEM");
	m_ioasic->in_port_cb<2>().set_ioport("IN1");
	m_ioasic->in_port_cb<3>().set_ioport("IN2");
	m_ioasic->set_dcs_tag(m_dcs);
	m_ioasic->set_shuffle(midway_ioasic_device::SHUFFLE_CARNEVIL);
	// 433, 495, 490 Development PIC
	m_ioasic->set_upper(495/*433,  495 others? */);
	m_ioasic->set_yearoffs(80);
	m_ioasic->irq_handler().set(FUNC(vegas_state::ioasic_irq));
	//m_ioasic->set_auto_ack(1)
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

// there is a socket next to the main bios roms for updates, this is what the update region is.


ROM_START( gauntleg )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 )
	ROM_LOAD( "legend15.bin", 0x000000, 0x80000, CRC(a8372d70) SHA1(d8cd4fd4d7007ee38bb58b5a818d0f83043d5a48) ) // EPROM Boot code. Version: Nov 17 1998 19:18:28 / 1.5 Nov 17 1998 19:21:49

	ROM_REGION32_LE( 0x100000, PCI_ID_NILE":update", ROMREGION_ERASEFF )

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" ) // GUTS 1.5 1/14/1999 Game 1/14/1999
	DISK_IMAGE( "gauntleg", 0, SHA1(66eb70e2fba574a7abe54be8bd45310654b24b08) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 ) // Vegas SIO boot ROM
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // security PIC (provides game ID code and serial number)
	ROM_LOAD( "322_gauntlet.u37", 0x0000, 0x2000, CRC(0fe0bd0a) SHA1(bfd54572e2923d26392e89961d044357f551872a) )
ROM_END


ROM_START( gauntleg12 )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 )
	ROM_LOAD( "legend13.bin", 0x000000, 0x80000, CRC(34674c5f) SHA1(92ec1779f3ab32944cbd953b6e1889503a57794b) ) // EPROM Boot code. Version: Sep 25 1998 18:34:43 / 1.3 Sep 25 1998 18:33:45
	ROM_LOAD( "legend14.bin", 0x000000, 0x80000, CRC(66869402) SHA1(bf470e0b9198b80f8baf8b9432a7e1df8c7d18ca) ) // EPROM Boot code. Version: Oct 30 1998 17:48:21 / 1.4 Oct 30 1998 17:44:29

	ROM_REGION32_LE( 0x100000, PCI_ID_NILE":update", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "noupdate",       "No Update Rom" )

	ROM_SYSTEM_BIOS( 1, "up16_1",       "Disk Update 1.2 to 1.6 Step 1 of 3" )
	ROMX_LOAD("12to16.1.bin", 0x000000, 0x100000, CRC(253c6bf2) SHA1(5e129576afe2bc4c638242e010735655d269a747), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "up16_2",       "Disk Update 1.2 to 1.6 Step 2 of 3" )
	ROMX_LOAD("12to16.2.bin", 0x000000, 0x100000, CRC(15b1fe78) SHA1(532c4937b55befcc3a8cb25b0282d63e206fba47), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 3, "up16_3",       "Disk Update 1.2 to 1.6 Step 3 of 3" )
	ROMX_LOAD("12to16.3.bin", 0x000000, 0x100000, CRC(1027e54f) SHA1(a841f5cc5b022ddfaf70c97a64d1582f0a2ca70e), ROM_BIOS(3))


	DISK_REGION( PCI_ID_IDE":ide:0:hdd" ) // GUTS 1.4 10/22/1998 Main 10/23/1998
	DISK_IMAGE( "gauntl12", 0, SHA1(62917fbd692d004bc391287349041ebe669385cf) ) // compressed with -chs 4969,16,63 (which is apparently correct for a Quantum FIREBALL 2.5 GB and allows the update program to work)

	ROM_REGION16_LE( 0x10000, "dcs", 0 ) // Vegas SIO boot ROM
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // security PIC (provides game ID code and serial number)
	ROM_LOAD( "322_gauntlet.u37", 0x0000, 0x2000, CRC(0fe0bd0a) SHA1(bfd54572e2923d26392e89961d044357f551872a) )
ROM_END


ROM_START( gauntdl )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 ) // EPROM 1.7 12/14/1999
	ROM_LOAD( "gauntdl.bin", 0x000000, 0x80000, CRC(3d631518) SHA1(d7f5a3bc109a19c9c7a711d607ff87e11868b536) )

	ROM_REGION32_LE( 0x100000, PCI_ID_NILE":update", ROMREGION_ERASEFF )


	DISK_REGION( PCI_ID_IDE":ide:0:hdd" ) // GUTS: 1.9 3/17/2000 Game 5/9/2000
	DISK_IMAGE( "gauntdl", 0, SHA1(ba3af48171e727c2f7232c06dcf8411cbcf14de8) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 ) // Vegas SIO boot ROM
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // security PIC (provides game ID code and serial number)
	ROM_LOAD( "346_gauntlet-dl.u37", 0x0000, 0x2000, CRC(09420dd3) SHA1(9ffc62049b3e329b525469849944896163b1582b) )
ROM_END


ROM_START( gauntdl24 )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 ) // EPROM 1.7 12/14/1999
	ROM_LOAD( "gauntdl.bin", 0x000000, 0x80000, CRC(3d631518) SHA1(d7f5a3bc109a19c9c7a711d607ff87e11868b536) )

	ROM_REGION32_LE( 0x100000, PCI_ID_NILE":update", ROMREGION_ERASEFF )


	DISK_REGION( PCI_ID_IDE":ide:0:hdd" ) // GUTS: 1.9 3/17/2000 Game 3/19/2000
	DISK_IMAGE( "gauntd24", 0, SHA1(3e055794d23d62680732e906cfaf9154765de698) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 ) // Vegas SIO boot ROM
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // security PIC (provides game ID code and serial number)
	ROM_LOAD( "346_gauntlet-dl.u37", 0x0000, 0x2000, CRC(09420dd3) SHA1(9ffc62049b3e329b525469849944896163b1582b) )
ROM_END


ROM_START( warfa )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 ) // EPROM 1.9 3/25/1999
	ROM_LOAD( "warboot.v19", 0x000000, 0x80000, CRC(b0c095cd) SHA1(d3b8cccdca83f0ecb49aa7993864cfdaa4e5c6f0) )

	ROM_REGION32_LE( 0x100000, PCI_ID_NILE":update", ROMREGION_ERASEFF )


	DISK_REGION( PCI_ID_IDE":ide:0:hdd" ) // GUTS 1.3 4/20/1999 Game 4/20/1999
	DISK_IMAGE( "warfa", 0, SHA1(87f8a8878cd6be716dbd6c68fb1bc7f564ede484) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 ) // Vegas SIO boot ROM
	ROM_LOAD16_BYTE( "warsnd.106", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )
ROM_END

ROM_START( warfaa )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 ) // EPROM 1.6 Jan 14 1999
	ROM_LOAD( "warboot.v16", 0x000000, 0x80000, CRC(1c44b3a3) SHA1(e81c15d7c9bc19078787d39c7f5e48eab003c5f4) )

	ROM_REGION32_LE( 0x100000, PCI_ID_NILE":update", ROMREGION_ERASEFF )


	DISK_REGION( PCI_ID_IDE":ide:0:hdd" ) // GUTS 1.1 Mar 16 1999, GAME Mar 16 1999
	DISK_IMAGE( "warfaa", 0, SHA1(b443ba68003f8492e5c20156e0d3091fe51e9224) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 ) // Vegas SIO boot ROM
	ROM_LOAD16_BYTE( "warsnd.106", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )
ROM_END

ROM_START( warfab )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 ) //EPROM 1.3 Apr 7 1999
	// label: WAR 42CE / BOOT V1.9 / PROG V1.6
	ROM_LOAD( "war42ce.bin", 0x000000, 0x80000, CRC(1a6e7f59) SHA1(0d8b4ce1e4b1132689796c4374aa54447b9a3369) )

	ROM_REGION32_LE( 0x100000, PCI_ID_NILE":update", ROMREGION_ERASEFF )

	DISK_REGION( PCI_ID_IDE":ide:0:hdd" ) // GUTS 1.3 Apr 7 1999 GAME 1.3 Apr 7 1999
	// V1.5
	DISK_IMAGE( "warfa15", 0, SHA1(bd538bf2f6a245545dae4ea97c433bb3f7d4394e) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 ) // Vegas SIO boot ROM
	ROM_LOAD16_BYTE( "warsnd.106", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )
ROM_END

ROM_START( warfac )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 ) // EPROM 1.91 Apr 13 1999
	ROM_LOAD( "war__upgrade__ver_1.91.u27", 0x000000, 0x80000, CRC(4d8fe0f8) SHA1(b809d29760ff229200509ba6751d8255faca7082) )

	ROM_REGION32_LE( 0x100000, PCI_ID_NILE":update", ROMREGION_ERASEFF )

	// required HDD image version is guess
	DISK_REGION( PCI_ID_IDE":ide:0:hdd" ) // GUTS 1.3 4/20/1999 GAME 4/20/1999
	DISK_IMAGE( "warfa", 0, SHA1(87f8a8878cd6be716dbd6c68fb1bc7f564ede484) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 ) // Vegas SIO boot ROM
	ROM_LOAD16_BYTE( "warsnd.106", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )
ROM_END

ROM_START( tenthdeg )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 )
	ROM_LOAD( "tenthdeg.bio", 0x000000, 0x80000, CRC(1cd2191b) SHA1(a40c48f3d6a9e2760cec809a79a35abe762da9ce) )

	ROM_REGION32_LE( 0x100000, PCI_ID_NILE":update", ROMREGION_ERASEFF )


	DISK_REGION( PCI_ID_IDE":ide:0:hdd" ) // GUTS 5/26/1998 MAIN 8/25/1998
	DISK_IMAGE( "tenthdeg", 0, SHA1(41a1a045a2d118cf6235be2cc40bf16dbb8be5d1) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 ) // Vegas SIO boot ROM
	ROM_LOAD16_BYTE( "tenthdeg.snd", 0x000000, 0x8000, CRC(1c75c1c1) SHA1(02ac1419b0fd4acc3f39676e7dce879e926d998b) )
ROM_END


ROM_START( roadburn ) // version 1.04 - verified on hardware
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 ) // EPROM 2.6 4/22/1999
	ROM_LOAD( "rbmain.bin", 0x000000, 0x80000, CRC(060e1aa8) SHA1(2a1027d209f87249fe143500e721dfde7fb5f3bc) )

	ROM_REGION32_LE( 0x100000, PCI_ID_NILE":update", ROMREGION_ERASEFF )


	DISK_REGION( PCI_ID_IDE":ide:0:hdd" ) // GUTS 5/19/1999 GAME 5/19/1999
	DISK_IMAGE( "road burners v1.04", 0, SHA1(30567241c000ee572a9cfb1b080c02a51a2b12d2) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // security PIC (provides game ID code and serial number)
	ROM_LOAD( "325_road_burners.u37", 0x0000, 0x2000, CRC(146c1ea1) SHA1(4bfda77f2ea6a421f59bbbd251b193a4e7743691) )
ROM_END

ROM_START( roadburn1 ) // version 1.0 - verified on hardware
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 ) // EPROM 2.6 4/22/1999
	ROM_LOAD( "rbmain.bin", 0x000000, 0x80000, CRC(060e1aa8) SHA1(2a1027d209f87249fe143500e721dfde7fb5f3bc) )

	ROM_REGION32_LE( 0x100000, PCI_ID_NILE":update", ROMREGION_ERASEFF )


	DISK_REGION( PCI_ID_IDE":ide:0:hdd" ) // GUTS 4/22/1999 GAME 4/22/1999
	DISK_IMAGE( "roadburn", 0, SHA1(a62870cceafa6357d7d3505aca250c3f16087566) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // security PIC (provides game ID code and serial number)
	ROM_LOAD( "325_road_burners.u37", 0x0000, 0x2000, CRC(146c1ea1) SHA1(4bfda77f2ea6a421f59bbbd251b193a4e7743691) )
ROM_END


ROM_START( nbashowt )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 )
	ROM_LOAD( "showtime_mar15_1999.u27", 0x000000, 0x80000, CRC(ff5d620d) SHA1(8f07567929f40a2269a42495dfa9dd5edef688fe) ) // 16:09:14 Mar 15 1999 BIOS FOR SHOWTIME USING BANSHEE / 16:09:01 Mar 15 1999. POST FOR SHOWTIME USING BANSHEE

	ROM_REGION32_LE( 0x100000, PCI_ID_NILE":update", ROMREGION_ERASEFF )


	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	// various strings from this image
	// SHOWTIME REV 2.0
	// BUILD DATE: Apr 25 1999 (diag.exe?)
	// BUILD DATE: Apr 21 1999 (game?)
	DISK_IMAGE( "nbashowt", 0, SHA1(f7c56bc3dcbebc434de58034986179ae01127f87) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 ) // Vegas SIO boot ROM
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )
ROM_END


ROM_START( nbanfl )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 )
	ROM_LOAD( "2.5.u27", 0x000000, 0x80000, CRC(6a9bd382) SHA1(18b942df6af86ea944c24166dbe88148334eaff9) ) // 16:00:32 Sep 22 1999 BIOS FOR BLITZ00 USING BANSHEE / 16:00:26 Sep 22 1999 POST FOR BLITZ00 USING BANSHEE
	ROM_LOAD( "2.6.u27", 0x000000, 0x80000, CRC(ec2885e5) SHA1(a7cc77b00d509f87563b57be31df3f8190700905) ) // 10:45:23 Oct 5 1999 BIOS FOR BLITZ00 USING BANSHEE / 10:42:26 Oct 5 1999 POST FOR BLITZ00 USING BANSHEE
	// Bad dump: First 3 bytes of reset vector (0x0) are FF's.  Reset vector is fixed in driver init.
	ROM_LOAD( "2.7.u27", 0x000000, 0x80000, CRC(4242bf14) SHA1(c1fcec67d7463df5f41afc89f22c3b4484279534) BAD_DUMP) // 15:10:49 Nov 30 1999 BIOS FOR BLITZ00 USING BANSHEE / 15:10:43 Nov 30 1999 POST FOR BLITZ00 USING BANSHEE

	ROM_REGION32_LE( 0x100000, PCI_ID_NILE":update", ROMREGION_ERASEFF )


	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	// various strings from this image
	//NBA SHOWTIME 2.1
	//BUILD DATE: Sep 22 1999 (diag.exe?)
	//BUILD DATE: Sep 21 1999 (game?)
	DISK_IMAGE( "nbanfl", 0, SHA1(f60c627f85f1bf58f2ea674063736a1e516e7e9e) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 ) // Vegas SIO boot ROM
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )
ROM_END



ROM_START( nbagold ) //Also known as "Sportstation"
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 )
	ROM_LOAD( "nbagold_jan10_2000.u27", 0x000000, 0x80000, CRC(6768e802) SHA1(d994e3efe14f57e261841134ddd1489fa67d418b) ) // 11:29:11 Jan 10 2000. BIOS FOR NBAGOLD USING BANSHEE / 11:23:58 Jan 10 2000. POST FOR NBAGOLD USING BANSHEE

	ROM_REGION32_LE( 0x100000, PCI_ID_NILE":update", ROMREGION_ERASEFF )


	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	// various strings from this image
	//NBA SHOWTIME GOLD 3.00
	//BUILD DATE Feb 18 2000 (diag.exe)
	//BUILD DATE:Feb 17 2000 (game?)
	//BUILD DATE:Feb 10 2000 (something else?)
	DISK_IMAGE( "nbanfl3", 0,  SHA1(19a51346ce5ae4e06e8dff3eb4bed59ec1ee855f))

	ROM_REGION16_LE( 0x10000, "dcs", 0 ) // Vegas SIO boot ROM
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )

	// also a PIC?
ROM_END


ROM_START( cartfury )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 )
	ROM_LOAD( "cart_mar8_2000.u27", 0x000000, 0x80000, CRC(c44550a2) SHA1(ad30f1c3382ff2f5902a4cbacbb1f0c4e37f42f9) ) // 10:40:17 Mar 8 2000 BIOS FOR CART USING VOODOO3 / 10:39:55 Mar 8 2000 POST FOR CART USING VOODOO3

	ROM_REGION32_LE( 0x100000, PCI_ID_NILE":update", ROMREGION_ERASEFF )


	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "cartfury", 0, SHA1(4c5bc2803297ea9a191bbd8b002d0e46b4ae1563) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 ) // ADSP-2105 data
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )
ROM_END


ROM_START( sf2049 )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 ) // EPROM 1.02 7/9/1999
	ROM_LOAD( "sf2049.u27", 0x000000, 0x80000, CRC(174ba8fe) SHA1(baba83b811eca659f00514a008a86ef0ac9680ee) )

	ROM_REGION32_LE( 0x100000, PCI_ID_NILE":update", ROMREGION_ERASEFF )


	DISK_REGION( PCI_ID_IDE":ide:0:hdd" ) // Guts 1.03 9/3/1999 Game 9/8/1999
	DISK_IMAGE( "sf2049", 0, SHA1(9e0661b8566a6c78d18c59c11cd3a6628d025405) )

	ROM_REGION( 0x2000, "serial_security_pic", ROMREGION_ERASEFF ) // security PIC (provides game ID code and serial number)
	ROM_LOAD( "336_rush_2049.u18", 0x0000, 0x1000, CRC(e258c3ff) SHA1(c78f739638a0775e4075c6a460c70dafbcf08fd5) )
ROM_END


ROM_START( sf2049se )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 )
	// POST output reports bad checksum for boot ROM, this is correct as verified with several original U27 chips
	ROM_LOAD( "sf2049se.u27", 0x000000, 0x80000, CRC(da4ecd9c) SHA1(2574ff3d608ebcc59a63cf6dea13ee7650ae8921) )

	ROM_REGION32_LE( 0x100000, PCI_ID_NILE":update", ROMREGION_ERASEFF )


	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "sf2049se", 0, SHA1(7b27a8ce2a953050ce267548bb7160b41f3e8054) )

	ROM_REGION( 0x2000, "serial_security_pic", ROMREGION_ERASEFF ) // security PIC (provides game ID code and serial number)
	ROM_LOAD( "352_rush_2049_se.u18", 0x0000, 0x1007, CRC(6120c20d) SHA1(9bd76514de261aa7957f896c1ea0b3f91d4cb5d6) ) // is this original or bootleg ? PIC timestamp is 1 Jan 1980 and SN# very small number
ROM_END


ROM_START( sf2049te )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 )
	ROM_LOAD( "sf2049te.u27", 0x000000, 0x80000, CRC(cc7c8601) SHA1(3f37dbd1b32b3ac5caa300725468e8e426f0fb83) )

	ROM_REGION32_LE( 0x100000, PCI_ID_NILE":update", ROMREGION_ERASEFF )


	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "sf2049te", 0, SHA1(625aa36436587b7bec3e7db1d19793b760e2ea51) ) // GUTS 1.61 Game Apr 2, 2001 13:07:21

	ROM_REGION( 0x2000, "serial_security_pic", ROMREGION_ERASEFF ) // security PIC (provides game ID code and serial number)
	ROM_LOAD( "352_rush_2049_se.u18", 0x0000, 0x1007, CRC(6120c20d) SHA1(9bd76514de261aa7957f896c1ea0b3f91d4cb5d6) ) // SE PIC is fine for TE too
ROM_END

ROM_START( sf2049tea )
	ROM_REGION32_LE( 0x80000, PCI_ID_NILE":rom", 0 )
	ROM_LOAD( "sf2049te.u27", 0x000000, 0x80000, CRC(cc7c8601) SHA1(3f37dbd1b32b3ac5caa300725468e8e426f0fb83) )

	ROM_REGION32_LE( 0x100000, PCI_ID_NILE":update", ROMREGION_ERASEFF )

	// All 7 courses are unlocked
	DISK_REGION( PCI_ID_IDE":ide:0:hdd" )
	DISK_IMAGE( "sf2049tea", 0, SHA1(8d6badf1159903bf44d9a9c7570d4f2417398a93) )

	ROM_REGION( 0x2000, "serial_security_pic", ROMREGION_ERASEFF ) // security PIC (provides game ID code and serial number)
	ROM_LOAD( "352_rush_2049_se.u18", 0x0000, 0x1007, CRC(6120c20d) SHA1(9bd76514de261aa7957f896c1ea0b3f91d4cb5d6) ) // SE PIC is fine for TE too
ROM_END

/*************************************
 *
 *  Driver init
 *
 *************************************/

void vegas_state::init_gauntleg()
{
	// speedups
	m_maincpu->mips3drc_add_hotspot(0x80015430, 0x8CC38060, 250); // confirmed
	m_maincpu->mips3drc_add_hotspot(0x80015464, 0x3C09801E, 250); // confirmed
	m_maincpu->mips3drc_add_hotspot(0x800C8918, 0x8FA2004C, 250); // confirmed
	m_maincpu->mips3drc_add_hotspot(0x800C8890, 0x8FA20024, 250); // confirmed
}


void vegas_state::init_gauntdl()
{
	// speedups
	m_maincpu->mips3drc_add_hotspot(0x800158B8, 0x8CC3CC40, 250); // confirmed
	m_maincpu->mips3drc_add_hotspot(0x800158EC, 0x3C0C8022, 250); // confirmed
	m_maincpu->mips3drc_add_hotspot(0x800D40C0, 0x8FA2004C, 250); // confirmed
	m_maincpu->mips3drc_add_hotspot(0x800D4038, 0x8FA20024, 250); // confirmed
}


void vegas_state::init_warfa()
{
	// speedups
	m_maincpu->mips3drc_add_hotspot(0x8009436C, 0x0C031663, 250); // confirmed
	// TODO: For some reason game hangs if ethernet is on
	m_ethernet->set_link_connected(false);
}


void vegas_state::init_tenthdeg()
{
	// speedups
	m_maincpu->mips3drc_add_hotspot(0x80051CD8, 0x0C023C15, 250); // confirmed
	m_maincpu->mips3drc_add_hotspot(0x8005E674, 0x3C028017, 250); // confirmed
	m_maincpu->mips3drc_add_hotspot(0x8002DBCC, 0x8FA2002C, 250); // confirmed
	m_maincpu->mips3drc_add_hotspot(0x80015930, 0x8FC20244, 250); // confirmed
}


void vegas_state::init_roadburn()
{
}


void vegas_state::init_nbashowt()
{
	m_maincpu->mips3drc_set_options(MIPS3DRC_FASTEST_OPTIONS | MIPS3DRC_STRICT_VERIFY | MIPS3DRC_EXTRA_INSTR_CHECK);
}


void vegas_state::init_nbanfl()
{
	m_maincpu->mips3drc_set_options(MIPS3DRC_FASTEST_OPTIONS | MIPS3DRC_STRICT_VERIFY | MIPS3DRC_EXTRA_INSTR_CHECK);
	// The first three bytes of the blitz00_nov30_1999.u27 ROM are FF's which breaks the reset vector.
	// These bytes are from blitz00_sep22_1999.u27 which allows the other ROM to start.
	// The last byte which is part of the checksum is also FF. By changing it to 0x01 the 4 byte checksum matches with the other 3 changes.
	uint8_t *romPtr = memregion(PCI_ID_NILE":rom")->base();
	romPtr[0x0] = 0xe2;
	romPtr[0x1] = 0x00;
	romPtr[0x2] = 0xf0;
	romPtr[0x7ffff] = 0x01;
}


void vegas_state::init_nbagold()
{
	m_maincpu->mips3drc_set_options(MIPS3DRC_FASTEST_OPTIONS | MIPS3DRC_STRICT_VERIFY | MIPS3DRC_EXTRA_INSTR_CHECK);
}


void vegas_state::init_sf2049()
{
	m_a2d_shift = 4;
}


void vegas_state::init_sf2049se()
{
	m_a2d_shift = 4;
}


void vegas_state::init_sf2049te()
{
	m_a2d_shift = 4;
}


void vegas_state::init_cartfury()
{
}

} // Anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

// Vegas + Vegas SIO + Voodoo 2
GAME( 1998, gauntleg,   0,         gauntleg, gauntleg, vegas_state, init_gauntleg, ROT0, "Atari Games",  "Gauntlet Legends (version 1.6)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, gauntleg12, gauntleg,  gauntleg, gauntleg, vegas_state, init_gauntleg, ROT0, "Atari Games",  "Gauntlet Legends (version 1.2)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, tenthdeg,   0,         tenthdeg, tenthdeg, vegas_state, init_tenthdeg, ROT0, "Atari Games",  "Tenth Degree (prototype)", MACHINE_SUPPORTS_SAVE )

// Vegas/Durango + Vegas SIO + Voodoo 2
GAME( 1999, gauntdl,    0,         gauntdl,  gauntleg, vegas_state, init_gauntdl,  ROT0, "Midway Games", "Gauntlet Dark Legacy (version DL 2.52)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, gauntdl24,  gauntdl,   gauntdl,  gauntleg, vegas_state, init_gauntdl,  ROT0, "Midway Games", "Gauntlet Dark Legacy (version DL 2.4)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, warfa,      0,         warfa,    warfa,    vegas_state, init_warfa,    ROT0, "Atari Games",  "War: Final Assault (EPROM 1.9 Mar 25 1999, GUTS 1.3 Apr 20 1999, GAME Apr 20 1999)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, warfaa,     warfa,     warfa,    warfa,    vegas_state, init_warfa,    ROT0, "Atari Games",  "War: Final Assault (EPROM 1.6 Jan 14 1999, GUTS 1.1 Mar 16 1999, GAME Mar 16 1999)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, warfab,     warfa,     warfa,    warfa,    vegas_state, init_warfa,    ROT0, "Atari Games",  "War: Final Assault (EPROM 1.3 Apr 7 1999, GUTS 1.3 Apr 7 1999, GAME Apr 7 1999)", MACHINE_SUPPORTS_SAVE ) // version numbers comes from test mode, can be unreliable
GAME( 1999, warfac,     warfa,     warfa,    warfa,    vegas_state, init_warfa,    ROT0, "Atari Games",  "War: Final Assault (EPROM 1.91 Apr 13 1999, GUTS 1.3 Apr 7 1999, GAME Apr 7 1999)", MACHINE_SUPPORTS_SAVE )

// Durango + DSIO + Voodoo 2
GAME( 1999, roadburn,   0,         roadburn, roadburn, vegas_state, init_roadburn, ROT0, "Atari Games",  "Road Burners (ver 1.04)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, roadburn1,  roadburn,  roadburn, roadburn, vegas_state, init_roadburn, ROT0, "Atari Games",  "Road Burners (ver 1.0)", MACHINE_SUPPORTS_SAVE )

// Vegas/Durango + Vegas SIO + Voodoo banshee
// missing older versions of NBA Showtime?
GAME( 1999, nbashowt,   0,         nbashowt, nbashowt, vegas_state, init_nbashowt, ROT0, "Midway Games", "NBA Showtime NBA on NBC (ver 2.0, Apr 25 1999)", MACHINE_SUPPORTS_SAVE )
// the game select menu shows SportStation for both of these, was there ever a standalone NBA Showtime Gold?
GAME( 1999, nbanfl,     0,         nbanfl,   nbashowt, vegas_state, init_nbanfl,   ROT0, "Midway Games", "SportStation: NBA Showtime NBA on NBC (ver 2.1, Sep 22 1999) / NFL Blitz 2000 Gold Edition (ver 1.5, Sep 22 1999)", MACHINE_SUPPORTS_SAVE ) // NBA Showtime titlescreen still shows Version 2.0
GAME( 2000, nbagold,    0,         nbagold,  nbashowt, vegas_state, init_nbagold,  ROT0, "Midway Games", "SportStation: NBA Showtime NBA on NBC Gold Edition (ver 3.0, Feb 18 2000) / NFL Blitz 2000 Gold Edition", MACHINE_SUPPORTS_SAVE ) // boot game dipswitch has no effect, so NFL Blitz 2000 version number not shown

// Durango + Denver SIO + Voodoo 3
GAMEL(1999, sf2049,     0,         sf2049,   sf2049,   vegas_state, init_sf2049,   ROT0, "Atari Games",  "San Francisco Rush 2049", MACHINE_SUPPORTS_SAVE, layout_sf2049 )
GAMEL(2003, sf2049se,   0,         sf2049se, sf2049se, vegas_state, init_sf2049se, ROT0, "Atari Games",  "San Francisco Rush 2049: Special Edition", MACHINE_SUPPORTS_SAVE, layout_sf2049 )
GAMEL(2000, sf2049te,   0,         sf2049te, sf2049se, vegas_state, init_sf2049te, ROT0, "Atari Games",  "San Francisco Rush 2049: Tournament Edition", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_sf2049 )
GAMEL(2001, sf2049tea,  sf2049te,  sf2049te, sf2049se, vegas_state, init_sf2049te, ROT0, "Atari Games",  "San Francisco Rush 2049: Tournament Edition Unlocked", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_sf2049 )

// Durango + Vegas SIO + Voodoo 3
GAME( 2000, cartfury,   0,         cartfury, cartfury, vegas_state, init_cartfury, ROT0, "Midway Games", "CART Fury Championship Racing (ver 1.00)", MACHINE_SUPPORTS_SAVE )
