// license:BSD-3-Clause
// copyright-holders:smf
/*
Namco I/O PCBs
--------------

ASCA-1A PCB
8662968202 (8662978202)
|--------------------------------------------------------|
|J105  J104  J103  J102                 J101             |
|       NJM2904             |-------|                    |
|                 ADM485    |ALTERA |                    |
|       NJM2904             |EPM7064|                    |
|                           |       |                    |
|                |-------|  |-------|                    |
|         |---|  |       |                               |
|         IC13|  | C78   | SW2            LB1235  LB1233 |
|         |---|  |       |  14.7460MHz       LB1235      |
|             JP1|-------|                               |
|         62256            PST592D                       |
|   MB87078       J106                                   |
|--------------------------------------------------------|
Notes:
     IC13 - Atmel AT29C020 2MBit EEPROM labelled 'ASC1IO-A' (PLCC32)
      C78 - Namco Custom MCU, positively identified as a Hitachi H8/3334 (PLCC84)
            Clock input 14.7460MHz
  EPM7064 - Altera EPM7064LC68-15 CPLD, labelled 'ASCA DR0' (PLCC68)
  PST592D - System Reset IC (SOIC4)
    62256 - Hitachi HM62256 32kB x8-bit SRAM
   ADM485 - Analog Devices +ADM485 5V Low Power EIA RS-485 Transceiver (SOIC8)
  MB87078 - Fujitsu MB87078 Electronic Volume Control IC (SOIC24)
  NJM2904 - New Japan Radio Co. NJM2904 Dual Operational Amplifier
   LB1235 - Sanyo LB1235 65V 1.5A 4-channel Darlington Driver
   LB1233 - Sanyo LB1233 Darlington Transistor Array with Built-in Current Limiting Resistors
     J101 - 64 pin connector joining to ?
     J102 - USB-A connector \  both tied together with common connections
     J103 - USB-B connector /
     J104 - Dual Red/White RCA Jacks (Twin Stereo Audio)
     J105 - 9 pin connector
     J106 - 12 pin connector
      JP1 - 3 pin jumper labelled 'WE' and 'NC'. Default position is NC. This is used to write enable
            the EEPROM for factory programming.
      SW2 - DIP switch with 4 switches (default all off)

This board is used with Final Furlong. This board connects to GORgON AV PCB


V187 ASCA-2A PCB
2477960102 (2477970102)
|--------------------------------------------------------|
|                   J105                                 |
|                           |-------|        14.7456MHz  |
|   J104                    |ALTERA |    ADM485   PST592D|
|                           |EPM7064|     |-------|      |
|                           |       |     |       |      |
|                           |-------|     | C78   |      |
|     LC78815                             |       |      |
|                                         |-------|      |
|     MB87078                              |---|         |
| LA4705                       LB1233      |IC1| 62256   |
|                        LB1235            |---|         |
|         J101                J102                       |
|--------------------------------------------------------|
Notes:
     IC1 - Atmel AT29C020 2MBit EEPROM labelled 'ASCA1 I/O-A' (PLCC32)
     C78 - Namco Custom MCU, positively identified as a Hitachi H8/3334 (PLCC84)
           Clock input 14.7456MHz
 EPM7064 - Altera EPM7064LC68-15 CPLD, labelled 'ASCA DR0' (PLCC68)
 PST592D - System Reset IC (SOIC4)
  ADM485 - Analog Devices +ADM485 5V Low Power EIA RS-485 Transceiver (SOIC8)
   62256 - Hitachi HM62256 32kB x8-bit SRAM
  LB1235 - Sanyo LB1235 65V 1.5A 4-channel Darlington Driver
  LB1233 - Sanyo LB1233 Darlington Transistor Array with Built-in Current Limiting Resistors
 MB87078 - Fujitsu MB87078 Electronic Volume Control IC (SOIC24)
 LC78815 - Sanyo LC78815 2-Channel 16-Bit D/A Converter (SOIC20)
  LA4705 - Sanyo LA4705 15W 2-channel Power Amplifier
    J101 - 34 pin flat cable connector for filter board
    J102 - 50 pin flat cable connector for filter board
    J104 - 8 pin JVS power connector (+5V, +12V, +3.3V)
    J105 - 64 pin connector for connection of Main PCB

This board is used with Rapid River but also works with Final Furlong.


ASCA-3A PCB 8662968301 (8662978301)
This is used with Motocross Go!
It's identical to the one shown below (ASCA-4A) but the ROM is a surface mounted
PLCC32 instead of a socketed DIP32 chip. Everything else is identical.

ASCA-4A PCB  8662968401 (8662978401)
(Used with most games and [for testing purposes] is able to boot all S23/SS23/Evolution2 games)
Sticker for Gunmen Wars: ASCA5 PCB 86629615
|---------------------------------------------------|
|                           PST592           62256  |
|                     14.7460MHz  |-------|         |
|                                 |  C78  |         |
|         LB1235       DSW(4)     |       | 27C1001 |
| LB1233  LB1235                  |-------|         |
|                                                   |
|                |------|            ADM485         |
|                |ALTERA|                           |
|                |EPM7096                           |
|      J101      |------|          J102  J103       |
|---------------------------------------------------|
Notes:
      J101     - 64 pin connector for power + inputs. This joins to another PCB at 90 degrees containing
                 a bunch of connectors named 'ASCA 4B PCB 8662968702 (8662978702)'. This is where ALL of the
                 inputs/outputs for game control and inter-PCB communication are connected
      PST592   - System Reset IC (SOIC4)
      C78      - Hitachi HD643334 H8/3334 Microcontroller rebadged as 'C78'. Clock input 14.746MHz (PLCC84)
      27C1001  - 128k x8 EPROM (DIP32)
                  - For Downhill Bikers labelled 'ASC3 IO-C'
                  - For Panic Park labelled 'ASC3 IO-C'
                  - For Race On! labelled 'ASC5 IO-A'
                  - For Gunmen Wars labelled 'ASC5 IO-A'
      62256    - 32k x8 SRAM (SOP28)
      EPM7096  - Altera EPM7096 CPLD with sticker 'ASCA,DR1' (PLCC68)
      ADM485   - Analog Devices ADM485 +5V Low Power EIA RS-485 Transceiver (SOIC8)
      J102/J103- Standard USB A and B connectors. These are not populated on most games, but are populated for
                 use with Motocross Go! on the ASCA-3A PCB.


FCA PCB  8662969102 (8662979102)
(Used with 500GP and Angler King. Another identical board is used with Ridge Racer V on System 246)
|---------------------------------------------------|
| J101                J106                          |
|            4.9152MHz                              |
|    DSW(6)                                         |
| LED2              |-----|                         |
|                   | MCU |                         |
|     LEDS3-10      |     |                         |
|  PIC16F84         |-----|                         |
|   JP1 LED1                           ADM485       |
|                                                   |
|                     J102              J104        |
|---------------------------------------------------|
Notes:
      J101     - 6 pin connector for power input
      J102     - 60 pin flat cable connector
      J104     - 5 pin connector
      J106     - 30 pin flat cable connector
      JP1      - 3 pin jumper, set to 'NORM'. Alt setting 'WR'
      3771     - Fujitsu MB3771 System Reset IC (SOIC8)
      PIC16F84 - Microchip PIC16F84 PIC (SOIC20)
                  - For 500GP and Angler King stamped 'CAP10'
                  - For Ridge Racer V (on System 246) stamped 'CAP11'
      MCU      - Fujitsu MB90F574 F2MC-16LX Family Microcontroller (QFP120)
                  - For 500 GP and Angler King stamped 'FCAF10'
                  - For Ridge Racer V (on System 246) stamped 'FCAF11'
      ADM485   - Analog Devices ADM485 +5V Low Power EIA RS-485 Transceiver (SOIC8)


Drive/Feedback PCB
------------------

V183 AMC PCB  2473966102 (2473970102)
|---------------------------------------------------|
|                                                   |
|                                                   |
|                          4.9152MHz              J2|
|                                     |-----|       |
|                                     | MCU |       |
|                                     |     |       |
|     SS22 FFB                        |-----|       |
|                      |------|                     |
|                      |ALTERA|              DSW(4) |
|                      |EPM7096                     |
|                      |------|                     |
|                                                 J4|
|                                       ADM485      |
|                 27C1024   62256x2               J5|
|                                                   |
|---------------------------------------------------|
Notes:
      This board is used only with Motocross Go! to control the steering feedback motor. It communicates as a slave JVS
      I/O board to both the game board and the ASCA I/O board. Another signal labelled as 'FREEZE/RELAY' connects between
      this board to its ASCA I/O board.

      MCU      - Fujitsu MB90611A F2MC-16F Family Microcontroller. Clock input 4.9152MHz (QFP100)
      62256    - 32k x2 SRAM (SOP28)
      EPM7096  - Altera EPM7064 CPLD labelled 'MG1,P LD0A' (PLCC44)
      27C1001  - 128k x8 EPROM labelled 'MG1-PRG0' (DIP40)
      ADM485   - Analog Devices ADM485 +5V Low Power EIA RS-485 Transceiver (SOIC8)
      J4/J5    - Standard USB A and B connectors.
      J2       - Ribbon cable connector.
      SS22 FFB - The recycled System 22 v147 motor drive board portion.

      From testing on an actual Motocross Go! cabinet, the game doesn't like it if this board is disconnected in any way.
      It needs both an ASCA and AMC I/O board chained in order to fully boot with properly working I/O. The following happens
      if any portion of an AMC pcb is disconnected (only applies to Motocross Go!).

      AMC board powered off: Board doesn't properly initialize its subcpu giving a 'subcpu timeout'.
      AMC board powered on, JVS comms disconnected, freeze/relay connected: Board initializes the subcpu properly but
      intentionally disables inputs.
      AMC board powered on, JVS comms connected, freeze/relay disconnected: Board initializes the subcpu properly.
      Main inputs work. Motor doesn't respond (if freeze/relay is reconnected, the motor will respond again).

      In RS232 mode, the data should be sent at 9600 8n1 in the following format:
          byte 0: 0xff
          byte 1: 0x00-0x7f Analog output 0
          byte 2: 0x00-0x0f/0x80-0x8f digital output
          byte 3: 0x00-0xff Analog output 1


I/O Boards for gun games
------------------------

Type 1:

V185 I/O PCB
2479961102 (2479971102)
|-----------------------------------------|
|   J601      LED1 LED2         |-------| |
|   DSW(4)JP1 |-------|         |TSSIO  | |
|     |-----| | C78   |14.746MHz|PLD    | |
|     |TSSIO| |       |PST592   |-------| |
|     |PROG | |-------|                   |
|     |-----|                             |
|                                         |
|     62256                               |
|ADM485                                   |
|J1               SLA4060               J5|
|                          J3       J4    |
|-----------------------------------------|
Notes:
      TSSIOPROG - Atmel AT29C020 EEPROM stamped 'TSSIOP8' (PLCC32)
      C78       - Namco Custom C78, actually a rebadged Hitachi HD643334 MCU, clock input 14.746MHz (PLCC84)
      TSSIOPLD  - Altera MAX EPM7128ELC84 CPLD with label 'TSSIOPLD' (PLCC84)
      SLA4060   - Sanken Electric SLA4060 NPN general purpose darlington transistor (used to drive the kick-back solenoid in the gun)
      PST592    - System Reset IC (SOIC4)
      J1        - 12 position connector for power and I/O communication
      J3        - 12 position connector for gun connection (trigger/buttons/optical signal/power)
      J4        - not used?
      J5        - 6 position connector for network
      J601      - not used?
      JP1       - jumper set to 1-2 (lower position), labelled 'WR'
      DSW       - 4 position dipswitch block, all off

This board is used only on Time Crisis II.
Note the gun is a standard light gun.


Type 2:

V221 MIU PCB
2512960101 (2512970101)
additional sticker for Time Crisis 3 says '2591961001 V291 XMIU PCB'
|---------------------------------------------|
|J10      J9    29C020     LC35256  DSW(4)    |
|    M0105          PRG.8F                LED |
|2267     6393                            LED |
|    T082  T082                 |------|      |
|           |--------|          | C78  |   J8 |
|           |ALTERA  |          |      |      |
|J11        |MAX     |          |------|    J7|
|   LM1881  |EPM7128 |                  3771  |
|R305526    |--------|                        |
|      ZUW1R51212            14.746MHz        |
|                                 ADM485    J6|
|                                             |
|  J1       J2   J3          J4    J5         |
|---------------------------------------------|
Notes:
      2267    - JRC2267 Current limiting diode array? (SOIC8)
      R305526 - Some kind of mini transformer or regulator?
      LC35256 - Sanyo LC35256 32k x8 SRAM (SOP28)
      LM1881  - National Semiconductor LM1881 Video Sync Separator (SOIC8)
      M0105   - Matsushita Panasonic 0105 = ? (SOIC16)
      T082    - Texas Instruments T082 (=TL082) JFET-Input operational amplifier (SOIC8)
      6393    - Sanyo 6393 (LA6393) High Performance Dual Comparator (SOIC8)
      ADM485  - Analog Devices ADM485 5V Low Power EIA RS-485 Transceiver (SOIC8)
      3771    - Fujitsu MB3771 Power Supply Monitor and Master Reset IC (SOIC8)
      EPM7128 - Altera MAX EPM7128SLC84-15 PLD labelled 'TMIU1/PLD0' (Time Crisis 3)
      29C020  - location for 29C020 PLCC32 Flash/EP ROM (not populated)
      ZUW1R51212 - Cosel ZUW1R51212 DC to DC Power Supply Module (input 9-18VDC, output +-12VDC or +24VDC)
      DSW     - 4 position dipswitch block, all off
      J1      - 6-pin power input connector
      J2      - 12-pin connector (cabinet buttons UP/DOWN/ENTER plus TEST/SERVICE/COIN etc)
      J3      - 4 pin connector (not used)
      J4      - 9 pin Namco female plug connector for gun (solenoid +24V/trigger/pedal/sensor)
      J5      - 5 pin connector used for I/O --> S246 communications (connects to USB link connection on main unit)
      J6      - 7-pin connector (not used)
      J9      - 6-pin connector (not used)
      J10     - 2-pin Namco female plug connector (not used)
      J11     - 6-pin Namco female plug connector (video input from CCD camera)
      PRG.8F  - 27C1001 EPROM with label
                                           'XMIU1 PRG0' (I/O program for Time Crisis 3)
                                           'CSZ1 PRG0A' (I/O program for Crisis Zone)

This board is used on Crisis Zone (System 23 Evolution2) and Time Crisis 3 (System 246)
Note both games use a CCD camera for the gun sensor.

*/

#include "emu.h"
#include "namcoio.h"
#include "jvshle.h"
#include "cpu/f2mc16/mb90570.h"

#define LOG_OUTPUT (1U << 1)

//#define VERBOSE (LOG_OUTPUT)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGOUTPUT(...)       LOGMASKED(LOG_OUTPUT, __VA_ARGS__)

namco_amc_device::namco_amc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NAMCO_AMC, tag, owner, clock),
	device_jvs_interface(mconfig, *this),
	device_rs232_port_interface(mconfig, *this),
	m_iocpu(*this, "iocpu"),
	m_analog_output{},
	m_clear_output(1),
	m_output(0),
	m_ppg_duty(0),
	m_ppg_flags(0),
	m_sense_latch(0),
	m_unknown_output(0)
{
}

void namco_amc_device::device_add_mconfig(machine_config &config)
{
	MB90611A(config, m_iocpu, 4.9152_MHz_XTAL);
	m_iocpu->set_addrmap(AS_PROGRAM, &namco_amc_device::iocpu_program_map);
	m_iocpu->ppg().output<1>().set([this](offs_t offset, uint32_t data, uint32_t mem_mask)
	{
		m_ppg_duty = mem_mask >> 26;
		update_analog_output0();
	});
	m_iocpu->adc().channel<0>().set(FUNC(namco_amc_device::adc_r<0>));
	m_iocpu->adc().channel<1>().set(FUNC(namco_amc_device::adc_r<1>));
	m_iocpu->adc().channel<2>().set(FUNC(namco_amc_device::adc_r<2>));
	m_iocpu->adc().channel<3>().set(FUNC(namco_amc_device::adc_r<3>));
	m_iocpu->port<0x2>().read().set([this]()
	{
		return (BIT(m_clear_output, 0) << 7) |
			((m_jvs_sense != jvs_port_device::sense::Initialized) << 6) |
			((m_jvs_sense != jvs_port_device::sense::None) << 5) |
			(1 << 4) | // sense
			(15 << 0); // output
	});
	m_iocpu->port<0x2>().write().set([this](uint8_t data)
	{
		m_sense_latch = BIT(data, 4); // HACK: improve compatibility by delaying sense::Initialized
		if (!m_sense_latch)
			sense(jvs_port_device::sense::Uninitialized);

		uint8_t output = bitswap<4>(data, 0, 1, 2, 3) << 4;
		if (m_output != output)
		{
			LOGOUTPUT("output_w %02x\n", output);
			m_output_cb(0, output, output ^ m_output);
			m_output = output;
		}
	});
	m_iocpu->port<0x3>().read().set([this](uint8_t data)
	{
		return ~bitswap<8>(player_r(0, 0xfc03), 10, 11, 12, 13, 14, 15, 0, 1);
	});
	m_iocpu->port<0x7>().read().set_ioport("DSW");
	m_iocpu->port<0x8>().write().set([this](uint8_t data)
	{
		m_ppg_flags = BIT(data, 0, 2);
		update_analog_output0();

		m_unknown_output = BIT(data, 2); // RS232 bit 7 of digital data, JVS unknown?
		// BIT(data, 5); // set at startup
	});
	m_iocpu->port<0x9>().write().set([this](uint8_t data)
	{
		if (m_sense_latch)
			sense(jvs_port_device::sense::Initialized); // HACK: improve compatibility by delaying sense::Initialized

		// BIT(data, 1); // set at startup
		rts(BIT(data, 2));
		// BIT(data, 3); // toggles to indicate activity/error?
		// BIT(data, 4); // ""
	});

	m_iocpu->uart<1>().sot().set(FUNC(namco_amc_device::txd));

	config.set_maximum_quantum(attotime::from_hz(2 * 115200));

	add_jvs_port(config);
}

namespace {

struct DSW { enum : uint8_t
{
	Interface = 0x08,
}; };

static INPUT_PORTS_START(namco_amc)
	PORT_START("DSW")
	PORT_DIPNAME(0x08, 0x08, "Interface") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(0x08, "JVS")
	PORT_DIPSETTING(0x00, "RS232")
	PORT_DIPNAME(0x04, 0x04, "Output Test") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On)) // interface must be set to JVS
	PORT_DIPNAME(0x02, 0x00, "JVS Timeout") PORT_DIPLOCATION("SW1:2") // HACK: improve compatibility by setting default to Continue
	PORT_DIPSETTING(0x02, "Stop") // outputs are cleared and board requires power cycling
	PORT_DIPSETTING(0x00, "Continue") // outputs are cleared, but board still responds
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
INPUT_PORTS_END

static INPUT_PORTS_START(namco_amc_default)
	PORT_INCLUDE(namco_amc)

	PORT_START("PLAYER1")
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON1)
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON2)
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_BUTTON3)
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_BUTTON4)
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_BUTTON5)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_BUTTON6)
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_BUTTON7)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_BUTTON8)

	PORT_START("ANALOG_INPUT1")
	PORT_BIT(0xffff, 0x8000, IPT_PADDLE) PORT_MINMAX(0x0000, 0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Analog Input 1")

	PORT_START("ANALOG_INPUT2")
	PORT_BIT(0xffff, 0x8000, IPT_PADDLE) PORT_MINMAX(0x0000, 0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Analog Input 2")

	PORT_START("ANALOG_INPUT3")
	PORT_BIT(0xffff, 0x8000, IPT_PADDLE) PORT_MINMAX(0x0000, 0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Analog Input 3")

	PORT_START("ANALOG_INPUT4")
	PORT_BIT(0xffff, 0x8000, IPT_PADDLE) PORT_MINMAX(0x0000, 0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Analog Input 4")

	PORT_START("CLEAR")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_BUTTON9) PORT_NAME("Clear Output") PORT_WRITE_LINE_MEMBER(FUNC(namco_amc_device::clear_output))
INPUT_PORTS_END

ROM_START( namco_amc )
	ROM_REGION( 0x020000, "iocpu", 0 )
	ROM_LOAD( "mg1prog0a.3a",    0x000000, 0x020000, CRC(b2b5be8f) SHA1(803652b7b8fde2196b7fb742ba8b9843e4fcd2de) )
ROM_END

} // anonymous namespace

ioport_constructor namco_amc_device::device_input_ports() const
{
	return m_default_inputs ? INPUT_PORTS_NAME(namco_amc_default) : INPUT_PORTS_NAME(namco_amc);
}

const tiny_rom_entry *namco_amc_device::device_rom_region() const
{
	return ROM_NAME(namco_amc);
}

void namco_amc_device::device_start()
{
	save_item(NAME(m_analog_output));
	save_item(NAME(m_clear_output));
	save_item(NAME(m_output));
	save_item(NAME(m_ppg_duty));
	save_item(NAME(m_ppg_flags));
	save_item(NAME(m_sense_latch));
	save_item(NAME(m_unknown_output));
}

void namco_amc_device::rxd(int state)
{
	m_iocpu->uart<1>().sin(state);
}

void namco_amc_device::input_txd(int state)
{
	m_iocpu->uart<0>().sin(state);
}

void namco_amc_device::iocpu_program_map(address_map &map)
{
	map(0x0000c8, 0x0000c8).w(FUNC(namco_amc_device::analog_output_w<1>));
	map(0xfc0000, 0xfcffff).ram();
	map(0xfe0000, 0xffffff).rom().region("iocpu", 0);
}

void namco_amc_device::update_analog_output0()
{
	analog_output_w<0>((BIT(m_ppg_flags, 0) ? 0 : m_ppg_duty) | (!BIT(m_ppg_flags, 1) << 6));
}

template<unsigned N>
void namco_amc_device::analog_output_w(uint8_t data)
{
	if (m_analog_output[N] != data)
	{
		m_analog_output[N] = data;
		LOGOUTPUT("analog_output_w %04x %04x\n", m_analog_output[0] * 0x101, m_analog_output[1] * 0x101);
		m_analog_output_cb[N](data * 0x101);
	}
}

template<int N>
uint16_t namco_amc_device::adc_r()
{
	return analog_input_r(N, 0xffc0) >> 6;
}

void namco_amc_device::clear_output(int state)
{
	m_clear_output = state;
}

int namco_amc_device::unknown_output()
{
	return m_unknown_output;
}


namespace {

static INPUT_PORTS_START(namco_c78_io)
	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DIP SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DIP SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DIP SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DIP SW2:4")
INPUT_PORTS_END

class namco_c78_jvs_io_device :
	public device_t,
	public device_jvs_interface
{
protected:
	namco_c78_jvs_io_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, type, tag, owner, clock),
		device_jvs_interface(mconfig, *this),
		m_iocpu(*this, "iocpu"),
		m_dsw(*this, "DSW")
	{
	}

	// device_t
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD
	{
		H83334(config, m_iocpu, 14.7456_MHz_XTAL);
		m_iocpu->set_addrmap(AS_PROGRAM, &namco_c78_jvs_io_device::iocpu_program_map);
		m_iocpu->write_port1().set_nop(); // unused
		m_iocpu->write_port2().set_nop(); // unused
		m_iocpu->write_port3().set_nop(); // unused
		m_iocpu->read_port4().set(FUNC(namco_c78_jvs_io_device::iocpu_port_4_r));
		m_iocpu->write_port4().set(FUNC(namco_c78_jvs_io_device::iocpu_port_4_w));
		m_iocpu->read_port5().set(FUNC(namco_c78_jvs_io_device::iocpu_port_5_r));
		m_iocpu->write_port5().set(FUNC(namco_c78_jvs_io_device::iocpu_port_5_w));
		m_iocpu->read_port6().set(FUNC(namco_c78_jvs_io_device::iocpu_port_6_r));
		m_iocpu->write_port6().set(FUNC(namco_c78_jvs_io_device::iocpu_port_6_w));
		m_iocpu->read_port9().set(FUNC(namco_c78_jvs_io_device::iocpu_port_9_r));

		m_iocpu->write_sci_tx<0>().set(FUNC(namco_c78_jvs_io_device::txd));

		config.set_maximum_quantum(attotime::from_hz(2 * 115200));

		add_jvs_port(config);
	}

	virtual ioport_constructor device_input_ports() const override ATTR_COLD
	{
		return INPUT_PORTS_NAME(namco_c78_io);
	}

	virtual void device_start() override ATTR_COLD
	{
	}

	// device_jvs_interface
	virtual void rxd(int state) override
	{
		m_iocpu->sci_rx_w<0>(state);
	}

	virtual void iocpu_program_map(address_map &map) ATTR_COLD
	{
		map(0x0000, 0x3fff).rom().region("iocpu", 0);
		map(0xc000, 0xfb7f).ram();
	}

	uint8_t iocpu_port_4_r()
	{
		return (1 << 7) | // J6:4 (unknown input)
			(BIT(m_dsw->read(), 0, 2) << 5) | // SW2:3/SW2:4
			((m_jvs_sense != jvs_port_device::sense::None) << 4) |
			((m_jvs_sense != jvs_port_device::sense::Initialized) << 3) |
			(1 << 2) | // TR1 collector, emitter J1:1 & base J1:2 (sense)
			(1 << 1) | // J6:10 (output high at startup)
			(1 << 0); // NC (output)
	}

	void iocpu_port_4_w(uint8_t data)
	{
		sense(BIT(data, 2) ? jvs_port_device::sense::Initialized : jvs_port_device::sense::Uninitialized);
		// BIT(data, 1)
	}

	uint8_t iocpu_port_5_r()
	{
		return (31 << 3) | // pins not bonded
			(1 << 2) | // ADM485:2 (rts)
			(1 << 1) | // ADM485:1 (configured as RxD)
			(1 << 0); // ADM485:4 (configured as TxD)
	}

	void iocpu_port_5_w(uint8_t data)
	{
		rts(BIT(data, 2));
	}

	virtual uint8_t iocpu_port_6_r()
	{
		return (1 << 7) | // unknown input
			(3 << 5) | // coin counter output
			(BIT(~player_r(0, 0x40), 6) << 4) |
			(3 << 2) | // unknown output
			(BIT(m_dsw->read(), 2) << 1) | // SW2:2
			(1 << 0); // unused input
	}

	void iocpu_port_6_w(uint8_t data)
	{
		machine().bookkeeping().coin_counter_w(0, BIT(data, 6));
		machine().bookkeeping().coin_counter_w(1, BIT(data, 5)); // except tssio/csz1/xmiu1
		// BIT(data, 3); // toggles (except asca1)
		// BIT(data, 2); // toggles (except asca1)
	}

	virtual uint8_t iocpu_port_9_r() = 0;

	required_device<h83334_device> m_iocpu;
	required_ioport m_dsw;
};


static INPUT_PORTS_START(namco_asca1)
	PORT_INCLUDE(namco_c78_io)

	PORT_MODIFY("DSW")
	PORT_DIPNAME(0x04, 0x04, "Output Test") PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
INPUT_PORTS_END

static INPUT_PORTS_START(namco_asca1_default)
	PORT_INCLUDE(namco_asca1)

	PORT_START("SYSTEM")
	PORT_SERVICE(0x80, IP_ACTIVE_HIGH)

	PORT_START("PLAYER1")
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_SERVICE1)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON1)
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON2)
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_BUTTON3)
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_BUTTON4)
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_BUTTON5)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_BUTTON6)
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_BUTTON7)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_BUTTON8)
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_BUTTON9)
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_BUTTON10)
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_BUTTON11)
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_BUTTON12)
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_BUTTON13)

	PORT_START("COIN1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1)

	PORT_START("COIN2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN2)

	PORT_START("ANALOG_INPUT1")
	PORT_BIT(0xffff, 0x8000, IPT_PADDLE) PORT_MINMAX(0x0000, 0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Analog Input 1")

	PORT_START("ANALOG_INPUT2")
	PORT_BIT(0xffff, 0x8000, IPT_PADDLE) PORT_MINMAX(0x0000, 0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Analog Input 2")

	PORT_START("ANALOG_INPUT3")
	PORT_BIT(0xffff, 0x8000, IPT_PADDLE) PORT_MINMAX(0x0000, 0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Analog Input 3")

	PORT_START("ANALOG_INPUT4")
	PORT_BIT(0xffff, 0x8000, IPT_PADDLE) PORT_MINMAX(0x0000, 0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Analog Input 4")

	PORT_START("ANALOG_INPUT5")
	PORT_BIT(0xffff, 0x8000, IPT_PADDLE) PORT_MINMAX(0x0000, 0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Analog Input 5")

	PORT_START("ANALOG_INPUT6")
	PORT_BIT(0xffff, 0x8000, IPT_PADDLE) PORT_MINMAX(0x0000, 0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Analog Input 6")

	PORT_START("ANALOG_INPUT7")
	PORT_BIT(0xffff, 0x8000, IPT_PADDLE) PORT_MINMAX(0x0000, 0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Analog Input 7")

	PORT_START("ANALOG_INPUT8")
	PORT_BIT(0xffff, 0x8000, IPT_PADDLE) PORT_MINMAX(0x0000, 0xffff) PORT_SENSITIVITY(100) PORT_KEYDELTA(2560) PORT_NAME("Analog Input 8")
INPUT_PORTS_END

ROM_START( namco_asca1 )
	ROM_REGION( 0x040000, "iocpu", 0 )
	ROM_LOAD( "asc1_io-a.ic13",  0x000000, 0x040000, CRC(77cdf69a) SHA1(497af1059f85c07bea2dd0d303481623f6019dcf) )
ROM_END

class namco_asca_1_device :
	public namco_c78_jvs_io_device
{
public:
	namco_asca_1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) :
		namco_asca_1_device(mconfig, NAMCO_ASCA1, tag, owner, clock)
	{
	}

protected:
	namco_asca_1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		namco_c78_jvs_io_device(mconfig, type, tag, owner, clock),
		m_output(0)
	{
	}

	// device_t
	virtual ioport_constructor device_input_ports() const override ATTR_COLD
	{
		return m_default_inputs ? INPUT_PORTS_NAME(namco_asca1_default) : INPUT_PORTS_NAME(namco_asca1);
	}

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD
	{
		namco_c78_jvs_io_device::device_add_mconfig(config);

		m_iocpu->read_port8().set(FUNC(namco_asca_1_device::iocpu_port_8_r));
		m_iocpu->write_port8().set(FUNC(namco_asca_1_device::iocpu_port_8_w));
		m_iocpu->write_port9().set(FUNC(namco_asca_1_device::iocpu_port_9_w));

		m_iocpu->read_adc<0>().set(FUNC(namco_asca_1_device::adc_r<0>));
		m_iocpu->read_adc<1>().set(FUNC(namco_asca_1_device::adc_r<1>));
		m_iocpu->read_adc<2>().set(FUNC(namco_asca_1_device::adc_r<2>));
		m_iocpu->read_adc<3>().set(FUNC(namco_asca_1_device::adc_r<3>));
		m_iocpu->read_adc<4>().set(FUNC(namco_asca_1_device::adc_r<4>));
		m_iocpu->read_adc<5>().set(FUNC(namco_asca_1_device::adc_r<5>));
		m_iocpu->read_adc<6>().set(FUNC(namco_asca_1_device::adc_r<6>));
		m_iocpu->read_adc<7>().set(FUNC(namco_asca_1_device::adc_r<7>));
	}

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD
	{
		return ROM_NAME(namco_asca1);
	}

	virtual void device_start() override ATTR_COLD
	{
		namco_c78_jvs_io_device::device_start();

		save_item(NAME(m_output));
	}

	// namco_c78_jvs_io_device
	virtual void iocpu_program_map(address_map &map) override
	{
		namco_c78_jvs_io_device::iocpu_program_map(map);

		map(0x6000, 0x6003).r(FUNC(namco_asca_1_device::in_r));
		map(0x6004, 0x6005).w(FUNC(namco_asca_1_device::output_w));
	}

	uint8_t iocpu_port_8_r()
	{
		return (1 << 7) | // pin not bonded
			(3 << 5) | // unknown input
			(31 << 0); // unknown output
	}

	void iocpu_port_8_w(uint8_t data)
	{
		// BIT(data, 6) // set at startup, is set to input?
	}

	virtual uint8_t iocpu_port_9_r() override
	{
		return (3 << 6) | // unknown input
			(31 << 1) | // unknown output
			(BIT(m_dsw->read(), 3) << 0); // SW2:1 TODO: confirm
	}

	void iocpu_port_9_w(uint8_t data)
	{
	}

	uint8_t in_r(offs_t offset)
	{
		if (offset == 0)
		{
			const auto system = system_r(0x80);
			const auto p1 = player_r(0, 0xe033);

			return ~((BIT(p1, 13) << 7) |
				(BIT(p1, 14) << 6) |
				(BIT(p1, 15) << 5) |
				(BIT(p1, 0) << 4) |
				(BIT(p1, 1) << 3) |
				(BIT(p1, 4) << 2) |
				(BIT(p1, 5) << 1) |
				(BIT(system, 7) << 0));
		}
		else if (offset == 1)
		{
			const auto p1 = player_r(0, 0xe00300);

			return ~((BIT(p1, 21) << 4) |
				(BIT(p1, 22) << 3) |
				(BIT(p1, 23) << 2) |
				(BIT(p1, 8) << 1) |
				(BIT(p1, 9) << 0));
		}
		else if (offset == 2)
		{
			const auto p1 = player_r(0, 0x1c00);
			const auto c1 = coin_r(0, 0x01);

			return ~((BIT(c1, 0) << 3) |
				(BIT(p1, 10) << 2) |
				(BIT(p1, 11) << 1) |
				(BIT(p1, 12) << 0));
		}
		else if (offset == 3)
		{
			const auto c1 = coin_r(0, 0x80);
			const auto c2 = coin_r(1, 0x81);

			return ~((BIT(c2, 0) << 3) |
				(BIT(~c2, 7) << 1) | // not asca1
				(BIT(~c1, 7) << 0)); // not asca1
		}
		return 0xff;
	}

	template<int N> uint16_t adc_r()
	{
		return analog_input_r(N, 0xffc0) >> 6;
	}

	virtual void output_w(offs_t offset, uint8_t data)
	{
		uint16_t output = ((m_output & ~(0xff << (offset * 8))) | bitswap<8>(data, 0, 1, 2, 3, 4, 5, 6, 7) << (offset * 8)) & 0xf8ff;

		if (m_output != output)
		{
			LOGOUTPUT("output_w %04x\n", output);
			m_output_cb(0, output, output ^ m_output);
			m_output = output;
		}
	}

	uint64_t m_output;
};


static INPUT_PORTS_START(namco_asca3_default)
	PORT_INCLUDE(namco_asca1_default)

	PORT_MODIFY("DSW")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DIP SW2:2")

	PORT_MODIFY("COIN1")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_TILT2) PORT_NAME("Counter 1 disconnected")

	PORT_MODIFY("COIN2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_TILT3) PORT_NAME("Counter 2 disconnected")

	PORT_START("ROTARY_INPUT1")
	PORT_BIT(0xff, 0, IPT_MOUSE_X) PORT_SENSITIVITY(15) PORT_KEYDELTA(8) PORT_NAME("Rotary Input 1")

	PORT_START("ROTARY_INPUT2")
	PORT_BIT(0xff, 0, IPT_MOUSE_Y) PORT_SENSITIVITY(15) PORT_KEYDELTA(8) PORT_REVERSE PORT_NAME("Rotary Input 2")
INPUT_PORTS_END

ROM_START( namco_asca3 )
	ROM_REGION( 0x040000, "iocpu", 0 )
	ROM_LOAD( "asc3_io-c.ic14", 0x000000, 0x020000, CRC(2f272a7b) SHA1(9d7ebe274c0d26f5f38747224d42d0375e2ed14c) )
ROM_END

class namco_asca_3_device :
	public namco_asca_1_device
{
public:
	namco_asca_3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) :
		namco_asca_3_device(mconfig, NAMCO_ASCA3, tag, owner, clock)
	{
	}

protected:
	namco_asca_3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		namco_asca_1_device(mconfig, type, tag, owner, clock),
		m_rotary_prev{}
	{
	}

	// device_t
	virtual ioport_constructor device_input_ports() const override ATTR_COLD
	{
		return m_default_inputs ? INPUT_PORTS_NAME(namco_asca3_default) : namco_c78_jvs_io_device::device_input_ports();
	}

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD
	{
		return ROM_NAME(namco_asca3);
	}

	virtual void device_start() override ATTR_COLD
	{
		m_analog_output_timer = timer_alloc(FUNC(namco_asca_3_device::analog_output_timer_callback), this);

		namco_asca_1_device::device_start();

		save_item(NAME(m_analog_output));
		save_item(NAME(m_analog_output_cycles));
		save_item(NAME(m_analog_output_state));
		save_item(NAME(m_rotary_prev));
	}

	// namco_c78_jvs_io_device
	virtual void iocpu_program_map(address_map &map) override
	{
		namco_asca_1_device::iocpu_program_map(map);

		map(0x6006, 0x6007).r(FUNC(namco_asca_3_device::rotary_diff_r));
	}

	virtual void output_w(offs_t offset, uint8_t data) override
	{
		if (offset == 1)
		{
			namco_asca_1_device::output_w(offset, data & ~(1 << 4));

			int analog_output_state = BIT(data, 4);
			if (analog_output_state != m_analog_output_state)
			{
				if (analog_output_state)
				{
					uint64_t high = m_analog_output_cycles[0] - m_analog_output_cycles[1];
					uint64_t low = m_iocpu->total_cycles() - m_analog_output_cycles[0];
					update_analog_output((high ? (0xff * high) / (high + low) : 0x0000) * 0x101);
				}

				m_analog_output_cycles[analog_output_state] = m_iocpu->total_cycles();
				m_analog_output_timer->adjust(attotime::from_hz(18));
				m_analog_output_state = analog_output_state;
			}
		}
		else
			namco_asca_1_device::output_w(offset, data);
	}

	TIMER_CALLBACK_MEMBER(analog_output_timer_callback)
	{
		update_analog_output(m_analog_output_state ? 0xffff : 0x0000);
	}

	void update_analog_output(uint16_t data)
	{
		if (m_analog_output != data)
		{
			m_analog_output = data;
			LOGOUTPUT("analog_output_w %04x\n", data);
			m_analog_output_cb[0](data);
		}
	}

	virtual uint8_t rotary_diff_r(offs_t offset)
	{
		uint8_t data = rotary_input_r(offset) - m_rotary_prev[offset];
		m_rotary_prev[offset] += data;
		return data;
	}

	emu_timer *m_analog_output_timer;
	uint16_t m_analog_output;
	uint64_t m_analog_output_cycles[2];
	uint8_t m_analog_output_state;
	uint8_t m_rotary_prev[2];
};


ROM_START( namco_asca3a )
	ROM_REGION( 0x040000, "iocpu", 0 )
	ROM_LOAD( "asca-3a.ic14", 0x000000, 0x040000, CRC(8e9266e5) SHA1(ffa8782ca641d71d57df23ed1c5911db05d3df97) )
ROM_END

class namco_asca_3a_device :
	public namco_asca_3_device
{
public:
	namco_asca_3a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) :
		namco_asca_3_device(mconfig, NAMCO_ASCA3A, tag, owner, clock)
	{
	}

protected:
	// device_t
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD
	{
		return ROM_NAME(namco_asca3a);
	}
};


ROM_START( namco_asca5 )
	ROM_REGION( 0x040000, "iocpu", 0 )
	ROM_LOAD( "asc5_io-a.ic14",  0x000000, 0x020000, CRC(5964767f) SHA1(320db5e78ae23c5f94e368432d51573b409995db) )
ROM_END

class namco_asca_5_device :
	public namco_asca_3_device
{
public:
	namco_asca_5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) :
		namco_asca_3_device(mconfig, NAMCO_ASCA5, tag, owner, clock)
	{
	}

protected:
	// device_t
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD
	{
		return ROM_NAME(namco_asca5);
	}
};


ROM_START( namco_emio102 )
	ROM_REGION( 0x040000, "iocpu", 0 )
	ROM_LOAD( "th1io-a.4f",      0x000000, 0x040000, CRC(1cbbce27) SHA1(71d61d9218543e1b0b2a6c550a8ff2b7c6267257) )
ROM_END

class namco_em_io1_02_device :
	public namco_asca_1_device
{
public:
	namco_em_io1_02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) :
		namco_asca_1_device(mconfig, NAMCO_EMIO102, tag, owner, clock)
	{
	}

protected:
	// device_t
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD
	{
		return ROM_NAME(namco_emio102);
	}

	virtual void device_start() override ATTR_COLD
	{
		namco_c78_jvs_io_device::device_start();

		save_item(NAME(m_multiplexed_output));
	}

	virtual void output_w(offs_t offset, uint8_t data) override
	{
		if (m_multiplexed_output[offset] != data)
		{
			m_multiplexed_output[offset] = data;

			if (m_multiplexed_output[1] < 3)
			{
				int shift = m_multiplexed_output[1] == 2 ? 16 :
					m_multiplexed_output[1] == 0 ? 8 : 0;

				uint64_t output = (m_output & ~(0xff << shift)) | (m_multiplexed_output[0] << shift);

				if (m_output != output)
				{
					LOGOUTPUT("output_w %06x\n", output);
					m_output_cb(0, output, output ^ m_output);
					m_output = output;
				}
			}
		}
	}

	uint8_t m_multiplexed_output[2];
};


static INPUT_PORTS_START(namco_tssio)
	PORT_INCLUDE(namco_c78_io)

	PORT_START("SW")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON7) PORT_NAME("Unknown (SW2:unpopulated)")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON8) PORT_NAME("Service (SW3:unpopulated)")
INPUT_PORTS_END

static INPUT_PORTS_START(namco_tssio_default)
	PORT_INCLUDE(namco_tssio)

	PORT_START("SYSTEM")
	PORT_SERVICE(0x80, IP_ACTIVE_HIGH)

	PORT_START("PLAYER1")
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_SERVICE1)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON1)
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON2)
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_BUTTON3)
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_BUTTON4)
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_BUTTON5)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_BUTTON6)

	PORT_START("COIN1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_TILT2) PORT_NAME("Counter disconnected")

	PORT_START("SCREEN_POSITION_INPUT_X1") // tuned for CRT
	PORT_BIT(0xfff, 91 + 733 / 2, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(91, 91 + 733) PORT_SENSITIVITY(100) PORT_KEYDELTA(12)

	PORT_START("SCREEN_POSITION_INPUT_Y1") // tuned for CRT - can't shoot below the statusbar?
	PORT_BIT(0xfff, 38 + 247 / 2, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(38, 38 + 247) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)
INPUT_PORTS_END

ROM_START( namco_tssio )
	ROM_REGION( 0x040000, "iocpu", 0 )
	ROM_LOAD( "tssioprog.ic3",   0x000000, 0x040000, CRC(edad4538) SHA1(1330189184a636328d956c0e435f8d9ad2e96a80) )
ROM_END

class namco_tss_io_device :
	public namco_c78_jvs_io_device
{
public:
	namco_tss_io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) :
		namco_tss_io_device(mconfig, NAMCO_TSSIO, tag, owner, clock)
	{
	}

protected:
	namco_tss_io_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0) :
		namco_c78_jvs_io_device(mconfig, type, tag, owner, clock),
		m_sw(*this, "SW"),
		m_output(0)
	{
	}

	// device_t
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD
	{
		namco_c78_jvs_io_device::device_add_mconfig(config);

		m_iocpu->read_port8().set(FUNC(namco_tss_io_device::iocpu_port_8_r));
		m_iocpu->write_port8().set(FUNC(namco_tss_io_device::iocpu_port_8_w));
		m_iocpu->write_port9().set(FUNC(namco_tss_io_device::iocpu_port_9_w));
	}

	virtual ioport_constructor device_input_ports() const override ATTR_COLD
	{
		return m_default_inputs ? INPUT_PORTS_NAME(namco_tssio_default) : INPUT_PORTS_NAME(namco_tssio);
	}

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD
	{
		return ROM_NAME(namco_tssio);
	}

	virtual void device_start() override ATTR_COLD
	{
		namco_c78_jvs_io_device::device_start();

		save_item(NAME(m_output));
	}

	// namco_c78_jvs_io_device
	virtual void iocpu_program_map(address_map &map) override
	{
		namco_c78_jvs_io_device::iocpu_program_map(map);

		map(0x6000, 0x6001).r(FUNC(namco_tss_io_device::in_r));
		map(0x6002, 0x6002).w(FUNC(namco_tss_io_device::output_w));
		map(0x7000, 0x7005).r(FUNC(namco_tss_io_device::gun_r));
		map(0x7007, 0x7007).r(FUNC(namco_tss_io_device::gun_ack_r)); // guessed, result ignored
	}

	virtual uint8_t iocpu_port_6_r() override
	{
		return (1 << 7) | // Altera:75 (unknown input)
			(3 << 5) | // coin counter output
			((BIT(~player_r(0, 0x40), 6) & BIT(m_sw->read(), 1)) << 4) | // SW3
			(3 << 2) | // LED (unemulated)
			(BIT(m_dsw->read(), 2) << 1) | // SW1:2
			(BIT(m_sw->read(), 0) << 0); // SW2
	}

	uint8_t iocpu_port_8_r()
	{
		return (1 << 7) | // pin not bonded
			(1 << 6) | // pullup VCC (maybe in case IRQ5 becomes enabled?)
			(1 << 5) | // J6:2 (configured for RxD1)
			(1 << 4) | // J6:3 (configured for TxD1)
			(15 << 0); // A13/A12/A11/A10 (unemulated input)
	}

	void iocpu_port_8_w(uint8_t data)
	{
	}

	virtual uint8_t iocpu_port_9_r() override
	{
		return (1 << 7) | // Altera:74 (unknown input)
			(1 << 6) | // NC (system clock output)
			(7 << 3) | // Altera (unknown output)
			(3 << 1) | // Altera (unknown input)
			(BIT(m_dsw->read(), 3) << 0); // SW1:1/J6:11/Altera
	}

	void iocpu_port_9_w(uint8_t data)
	{
	}

	uint8_t gun_r(offs_t offset)
	{
		uint16_t data = (offset % 3) == 0 ? screen_position_x_r(0) :
			(offset % 3) == 1 ? screen_position_y_r(0) : // ystart
			screen_position_y_r(0) + 1;// yend
		return (offset < 3) ? data & 0xff : data >> 8;
	}

	uint8_t gun_ack_r()
	{
		return 0x00;
	}

	uint8_t in_r(offs_t offset)
	{
		if (offset == 0)
		{
			const auto system = system_r(0x80);
			const auto p1 = player_r(0, 0x32);
			const auto c1 = coin_r(0, 0x81);

			return ~((BIT(p1, 1) << 7) | // BUTTON1
				(BIT(p1, 4) << 6) | // DOWN
				(BIT(p1, 5) << 5) | // UP
				(BIT(system, 7) << 4) | // TEST
				(0 << 3) | // unknown input
				(BIT(~c1, 7) << 2) | // counter disconnected
				(0 << 1) | // unknown input
				(BIT(c1, 0) << 0)); // COIN
		}
		else if (offset == 1)
		{
			const auto p1 = player_r(0, 0xf001);

			return ~((BIT(p1, 12) << 4) | // BUTTON6
				(BIT(p1, 13) << 3) | // BUTTON5
				(BIT(p1, 14) << 2) | // BUTTON4
				(BIT(p1, 15) << 1) | // BUTTON3
				(BIT(p1, 0) << 0)); // BUTTON2
		}

		return 0xff;
	}

	void output_w(uint8_t data)
	{
		data = bitswap<3>(data, 0, 1, 2) << 5;
		if (m_output != data)
		{
			LOGOUTPUT("output_w %02x\n", data);
			m_output_cb(0, data, data ^ m_output);
			m_output = data;
		}
	}

	required_ioport m_sw;
	uint8_t m_output;
};


static INPUT_PORTS_START(namco_csz1_default)
	PORT_INCLUDE(namco_tssio_default)

	PORT_MODIFY("SCREEN_POSITION_INPUT_X1")
	PORT_BIT(0xfff, 0x1bf, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0x040, 0x33f) PORT_SENSITIVITY(100) PORT_KEYDELTA(12)

	PORT_MODIFY("SCREEN_POSITION_INPUT_Y1")
	PORT_BIT(0xfff, 0x08f, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0x020, 0x0ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)
INPUT_PORTS_END

ROM_START( namco_csz1 )
	ROM_REGION( 0x040000, "iocpu", 0 )
	ROM_LOAD( "csz1prg0a.8f",    0x000000, 0x020000, CRC(8edc36b3) SHA1(b5df211988d856572fcc313480e693c8561784e4) )
	ROM_FILL( 0x04ed, 1, 0x05 ) // HACK: Increase timeout from 4
ROM_END

class namco_csz1_device :
	public namco_tss_io_device
{
public:
	namco_csz1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) :
		namco_tss_io_device(mconfig, NAMCO_CSZ1, tag, owner, clock)
	{
	}

protected:
	// device_t
	virtual ioport_constructor device_input_ports() const override ATTR_COLD
	{
		return m_default_inputs ? INPUT_PORTS_NAME(namco_csz1_default) : namco_tss_io_device::device_input_ports();
	}

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD
	{
		return ROM_NAME(namco_csz1);
	}

	virtual void device_start() override ATTR_COLD
	{
		namco_tss_io_device::device_start();

		m_analog_output = 0;

		save_item(NAME(m_analog_output));
	}

	// namco_c78_jvs_io_device
	virtual void iocpu_program_map(address_map &map) override
	{
		namco_tss_io_device::iocpu_program_map(map);

		map(0x6003, 0x6003).w(FUNC(namco_csz1_device::analog_output_w));
	}

	void analog_output_w(uint8_t data)
	{
		if (m_analog_output != data)
		{
			m_analog_output = data;
			LOGOUTPUT("analog_output_w %04x\n", data * 0x101);
			m_analog_output_cb[0](data * 0x101);
		}
	}

	uint8_t m_analog_output;
};


ROM_START( namco_xmiu1 )
	ROM_REGION( 0x040000, "iocpu", 0 )
	ROM_LOAD( "csz1prg0a.8f",    0x000000, 0x040000, CRC(bb112fe0) SHA1(6fa5dc81d258137c1b1ad427d49d136d0bbf53fa) )
ROM_END

class namco_xmiu1_device :
	public namco_tss_io_device
{
public:
	namco_xmiu1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) :
		namco_tss_io_device(mconfig, NAMCO_XMIU1, tag, owner, clock)
	{
	}

protected:
	// device_t
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD
	{
		return ROM_NAME(namco_xmiu1);
	}
};


ROM_START( namco_fca11 )
	ROM_REGION( 0x040000, "iocpu2", 0 ) // 256KB internal flash ROM
	ROM_LOAD( "fcaf11.ic4",      0x030000, 0x010000, CRC(13d936df) SHA1(fbb2191263b2b326f1f49729767ee6fae2db21f7) ) // almost good dump, all JVS related code and data is in place
	ROM_FILL(                    0x000000, 0x034000, 0x67 ) // dump was made from $0000 to $ffff, not $fc0000. The ROM only appears in memory from $4000-$ffff

	ROM_REGION( 0x040000, "iocpu", 0 )
	ROM_LOAD( "asc3_io-c.ic14",  0x000000, 0x020000, CRC(2f272a7b) SHA1(9d7ebe274c0d26f5f38747224d42d0375e2ed14c) )
	ROM_COPY( "iocpu2",          0x039040, 0x001082, 0x00003b ) // patch ASCA3 ROM to report FCA11 description

	ROM_REGION( 0x10000, "pic", 0 ) // I/O board PIC16F84 code
	ROM_LOAD( "fcap11.ic2",      0x000000, 0x004010, CRC(1b2592ce) SHA1(a1a487361053af564f6ec67e545413e370a3b38c) )
ROM_END

class namco_fca_11_device :
	public namco_asca_3_device
{
public:
	namco_fca_11_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) :
		namco_fca_11_device(mconfig, NAMCO_FCA11, tag, owner, clock)
	{
	}

protected:
	namco_fca_11_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		namco_asca_3_device(mconfig, type, tag, owner, clock),
		m_iocpu2(*this, "iocpu2")
	{
	}

	// device_t
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD
	{
		namco_asca_3_device::device_add_mconfig(config);

		MB90F574(config, m_iocpu2, 4.9152_MHz_XTAL);
		m_iocpu2->set_addrmap(AS_PROGRAM, &namco_fca_11_device::iocpu2_program_map);
	}

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD
	{
		return ROM_NAME(namco_fca11);
	}

	virtual void device_reset() override ATTR_COLD
	{
		namco_asca_3_device::device_reset();

		m_iocpu2->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}

	// device_jvs_interface
	virtual void rxd(int state) override
	{
		namco_asca_3_device::rxd(state);

		m_iocpu2->uart<1>().sin(state);
	}

	void iocpu2_program_map(address_map &map)
	{
		map(0x004000, 0x00ffff).rom().region("iocpu2", 0x034000);
		map(0xfc0000, 0xffffff).rom().region("iocpu2", 0);
	}

	required_device<mb90f574_device> m_iocpu2;
};


ROM_START( namco_fca10 )
	ROM_REGION( 0x040000, "iocpu2", ROMREGION_ERASE00 ) // 256KB internal flash ROM
	ROM_LOAD( "fcaf10.bin",      0x000000, 0x040000, NO_DUMP )

	ROM_REGION( 0x040000, "iocpu", 0 )
	ROM_LOAD( "asc3_io-c.ic14",  0x000000, 0x020000, CRC(2f272a7b) SHA1(9d7ebe274c0d26f5f38747224d42d0375e2ed14c) )

	ROM_REGION( 0x10000, "pic", 0 ) // I/O board PIC16F84 code
	ROM_LOAD( "fcap10.ic2",      0x000000, 0x004010, NO_DUMP )
ROM_END

class namco_fca_10_device :
	public namco_fca_11_device
{
public:
	namco_fca_10_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) :
		namco_fca_11_device(mconfig, NAMCO_FCA10, tag, owner, clock)
	{
	}

protected:
	// device_t
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD
	{
		return ROM_NAME(namco_fca10);
	}

	virtual void device_start() override ATTR_COLD
	{
		namco_fca_11_device::device_start();

		// patch ASCA3 ROM to report FCA10 description
		auto iocpu = memregion("iocpu")->base();
		for (int i = 0; i < 0x30; i++)
			iocpu[BYTE_XOR_BE(i + 0x108d)] = "FCA-1;Ver1.00;JPN,Multipurpose + Rotary Encoder"[i];
	}
};


ROM_START( namco_fcb )
	ROM_REGION( 0x040000, "iocpu2", 0 ) // 256KB internal flash ROM
	ROM_LOAD( "fcb1_io-0b.ic4",  0x034000, 0x00c000, BAD_DUMP CRC(5e25b73f) SHA1(fa805a422ff8793989b0ce901cc868ec1a87c7ac) ) // most JVS handling code is in undumped area
	ROM_FILL(                    0x000000, 0x034000, 0x67 ) // dump was made from $0000 to $ffff, not $fc0000. The ROM only appears in memory from $4000-$ffff

	ROM_REGION( 0x040000, "iocpu", 0 )
	ROM_LOAD( "asc3_io-c.ic14",  0x000000, 0x020000, CRC(2f272a7b) SHA1(9d7ebe274c0d26f5f38747224d42d0375e2ed14c) )
	ROM_COPY( "iocpu2",          0x03436a, 0x001082, 0x00003b ) // patch ASCA3 ROM to report FCB description

	ROM_REGION( 0x10000, "pic", 0 ) // I/O board PIC16F84 code
	ROM_LOAD( "fcb_pic",         0x000000, 0x004010, NO_DUMP )
ROM_END

class namco_fcb_device :
	public namco_fca_11_device
{
public:
	namco_fcb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) :
		namco_fca_11_device(mconfig, NAMCO_FCB, tag, owner, clock)
	{
	}

protected:
	// device_t
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD
	{
		return ROM_NAME(namco_fcb);
	}
};


class namco_em_pri1_01_device :
	public jvs_hle_device
{
public:
	namco_em_pri1_01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) :
		jvs_hle_device(mconfig, NAMCO_EMPRI101, tag, owner, clock)
	{
	}

	// device_t
	static constexpr feature_type imperfect_features() { return feature::PRINTER; }

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD
	{
		add_jvs_port(config);
	}

	// jvs_hle_device
	virtual const char *device_id() override
	{
		return "namco ltd.;EM Pri1-01;Ver2.00;JPN&EXP,Techno-Drive PRN";
	}

	virtual void execute(uint8_t command) override
	{
		const uint8_t *parameters;
		uint8_t *response;

		switch (command)
		{
		case 0x72:
			if (consume(1, parameters))
			{
				switch (parameters[0])
				{
				case 0x01: // initialise?
					if (produce(1, response))
						response[0] = ReportCode::Normal;
					break;

				case 0x02: // end transmit?
					if (produce(1, response))
						response[0] = ReportCode::Normal;
					break;

				case 0x03: // get status
					if (produce(2, response))
					{
						response[0] = ReportCode::Normal;
						response[1] = 0x01; // 1=ok/2=busy/4=paper empty
					}
					break;

				case 0x04: // begin transmit?
					if (produce(1, response))
						response[0] = ReportCode::Normal;
					break;

				case 0x05: // end of print job?
					if (consume(1, parameters) && produce(1, response))
						response[0] = (parameters[0] == 1) ? ReportCode::Normal : ReportCode::InvalidParameter;
					break;

				case 0x07: // transmit data
					if (consume(1, parameters))
					{
						uint8_t length = parameters[0];

						if (consume(length, parameters) && produce(1, response))
							response[0] = ReportCode::Normal;
					}
					break;

				default:
					status_code(StatusCode::UnknownCommand);
					break;
				}
			}
			break;

		default:
			jvs_hle_device::execute(command);
			break;
		}
	}
};

} // anonymous namespace


DEFINE_DEVICE_TYPE(NAMCO_AMC, namco_amc_device, "namco_amc", "Namco AMC PCB (Extra I/O,JPN,Ver1.10)")
DEFINE_DEVICE_TYPE_PRIVATE(NAMCO_ASCA1, device_jvs_interface, namco_asca_1_device, "namco_asca1", "Namco ASCA-1 (Multipurpose I/O,JPN,Ver2.00)")
DEFINE_DEVICE_TYPE_PRIVATE(NAMCO_ASCA3, device_jvs_interface, namco_asca_3_device, "namco_asca3", "Namco ASCA-3 (Multipurpose + Rotary Encoder,JPN,Ver2.04)")
DEFINE_DEVICE_TYPE_PRIVATE(NAMCO_ASCA3A, device_jvs_interface, namco_asca_3a_device, "namco_asca3a", "Namco ASCA-3 (Multipurpose + Rotary Encoder,JPN,Ver2.02)")
DEFINE_DEVICE_TYPE_PRIVATE(NAMCO_ASCA5, device_jvs_interface, namco_asca_5_device, "namco_asca5", "Namco ASCA-5 (Multipurpose,JPN,Ver2.09)")
DEFINE_DEVICE_TYPE_PRIVATE(NAMCO_CSZ1, device_jvs_interface, namco_csz1_device, "namco_csz1", "Namco CSZ1 MIU-I/O (GUN-EXTENTION,JPN,Ver2.05)")
DEFINE_DEVICE_TYPE_PRIVATE(NAMCO_EMIO102, device_jvs_interface, namco_em_io1_02_device, "namco_emio102", "Namco EM I/O1-02 (Techno-Drive I/O,JPN&EXP,Ver2.00)")
DEFINE_DEVICE_TYPE_PRIVATE(NAMCO_EMPRI101, device_jvs_interface, namco_em_pri1_01_device, "namco_empri101", "Namco EM Pri1-01")
DEFINE_DEVICE_TYPE_PRIVATE(NAMCO_FCA10, device_jvs_interface, namco_fca_10_device, "namco_fca10", "Namco FCA-1 (Multipurpose + Rotary Encoder,JPN,Ver1.00)")
DEFINE_DEVICE_TYPE_PRIVATE(NAMCO_FCA11, device_jvs_interface, namco_fca_11_device, "namco_fca11", "Namco FCA-1 (Multipurpose + Rotary Encoder,JPN,Ver1.01)")
DEFINE_DEVICE_TYPE_PRIVATE(NAMCO_FCB, device_jvs_interface, namco_fcb_device, "namco_fcb", "Namco FCB (TouchPanel&Multipurpose,JPN,Ver1.02)")
DEFINE_DEVICE_TYPE_PRIVATE(NAMCO_TSSIO, device_jvs_interface, namco_tss_io_device, "namco_tssio", "Namco TSS-I/O (GUN-EXTENTION,JPN,Ver2.02)")
DEFINE_DEVICE_TYPE_PRIVATE(NAMCO_XMIU1, device_jvs_interface, namco_xmiu1_device, "namco_xmiu1", "Namco XMIU1 TSS-I/O (GUN-EXTENTION,JPN,Ver2.11,Ver2.12)")
