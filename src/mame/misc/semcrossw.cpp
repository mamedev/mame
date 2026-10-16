// license:BSD-3-Clause
// copyright-holders:Tomás García-Merás Capote (ClawGrip)

/***************************************************************************

 ETRA (https://www.grupoetra.com/) semaphore controller for a crosswalk
 (unknown model, mid-1980s according to the date codes on the main PCB).

Main PCB
  __________________________________________________
 |     _______    _______    _______                |
 |    |      |   |      |   |      |                |
 |    |______|   |______|   |______|                |
 |                           _________    _________ |
 |                          |_74LS03_|   DM74LS122N |
 |              _________                           |
 |             |_UM6114_|    _________    _________ |
 |                          |T74LS04B1   |_74LS90_| |__
 |                                                   __|
 |              _________    _________               __|
 |             |_UM6114_|   DM74LS155N    _________  __|
 |       ________________                DM74LS155N  __|
  \     | X2816CP-12    |  Xtal                      __|
  _\    |_______________|  4.000 MHz                 __|
 |__     ________________    ________________        __|
 |__    | AT2716 EPROM  |   | MC6802P       |        __|
 |__    |_______________|   |_______________|        __|
 |__                                                 __|
 |__     _________           ________________        __|
 |__    |_74LS156|          | MC6821P       |       |
 |__                        |_______________|       |
 |__                                                |
 |__     _________    _________    _________        |
 |__    |________|   |_74LS132|   |_74LS90_|        |
   |                                                |
   |________________________________________________|

Relays PCB
           _________________________________________
          |                                         |
          |                                         |
          |                        _________        |
          |                       |74LS122N|        |
          |                 ________________        |
         /                 | MC6821P       |        |
        /                  |_______________|        |
       /                                            |__
  ____/                   _______ _______            __|
 |       _______          MOC3020 MOC3020            __|
 |      TXAL2215B         _______ _______   _______  __|
 |       _______          MOC3020 MOC3020  |_7404N|  __|
  \     TXAL2215B         _______ _______   _______  __|
   \     _______          MOC3020 MOC3020  |_7404N|  __|
   |    TXAL2215B         _______ _______   _______  __|
   |     _______          MOC3020 MOC3020  |_7404N|  __|
   |    TXAL2215B         _______ _______            __|
   |     _______          MOC3020 MOC3020            __|
   |    TXAL2215B         _______ _______            __|
   |     _______          MOC3020 MOC3020           |
   |    TXAL2215B                                   |
   |                                                |
   |                                                |
   |                                                |
   |                                                |
   |________________________________________________|

Programmer PCB (keyboard)
    _______________________________________________________
   |  ______ ______ ______ ______ ______ ______           |
   | | ___ || ___ || ___ || ___ || ___ || ___ |           |
   | ||__| |||__| |||__| |||__| |||__| |||__| |  ___      |
   | ||__| |||__| |||__| |||__| |||__| |||__| | |  |      |
   | |_____||_____||_____||_____||_____||_____| |  |<-7407N
   |                                            |__|      |
   |       __________________________________             |
   |      | ____   ____   ____   ____   _   |             |
   |      || R |  | M |  | N |  | K |  (_)  |             |
   |      ||___|  |___|  |___|  |___|       |             |
   | ___  | ____   ____   ____   ____       |      SWITCH |
   ||  |  || 0 |  | 1 |  | 2 |  | 3 |   __  |             |
   ||  |  ||___|  |___|  |___|  |___|  (||) |             |
   ||__|  | ____   ____   ____   ____       |             |
 74LS155N || 4 |  | 5 |  | 6 |  | 7 |   __  |             |
   |      ||___|  |___|  |___|  |___|  (||) |             |
   | ___  | ____   ____   ____   ____       |    ___      |
   ||  |  || 8 |  | 9 |  | A |  | B |   __  |   |  |      |
   ||  |  ||___|  |___|  |___|  |___|  (||) |   |  |<-SN74LS03N
   ||__|  | ____   ____   ____   ____       |   |__|      |
SCL4052BE || C |  | D |  | E |  | F |       |    ___  ___ |
   |      ||___|  |___|  |___|  |___|       |   |  |<-TC4093BP
   | ___  |_________________________________|   |  | |  | |
   ||  |                                        |__| |__|<-7407N
   ||  |<-CD4093BE                                        |
   ||__|   ____  ____  ____  ____  ____  ____  ____  ____ |
   |       4N32  B250  4N32  B250  4N32  B250  4N32  B250 |
   |            C1000       C1000       C1000       C1000 |
   |                               __________             |
   |                              |  CONN   |             |
   |______________________________________________________|

What the firmware does (facts from the disassembly):
 - It only clears the MC6802 internal RAM (0000-007f) at reset.
 - Programs and parameters are at 2000-23ff: decimal addresses 100-999 are 2064-23e7.
 - The keyboard, display, switches and interrupts use a PIA at 8800. A PIA at 8400 is
   initialized but never used (the main PCB has only one MC6821).
 - The relay boards are at a000-afff, one per address line from A2 to A11 (the PIAs at
   a004, a008, ... a800 are initialized). Port A drives the lamps, active low: 0xff at
   start-up, and 0x77 for a board with no lamps lit in the tables. Port B is compared
   with port A for the lamps in the mask at 160 (lamp current sensors).
 - The lamp tables are reached through the pointer at c625, and the pointers stored there
   (c65d-c695) only make sense with the EEPROM at c000. They have entries for three relay
   boards. Lamp bits, going by these tables: 0/4 red, 1/5 amber, 2/6 green; 2/6 can also
   use a second flashing rate (the pedestrian green flashing); 3/7 aren't lamps (never
   checked with the sensors). Groups 1 A and 1 B always match (first vehicle phase), 2 A
   is a second vehicle phase with green along with the pedestrians, 2 B red and green act
   as a walk / don't walk signal while its amber only flashes, alongside them (a separate
   lamp), 3 A only flashes amber in step 0 and 3 B is never lit.
 - The step durations are counted in units of 20 NMIs, the amber flashing toggles every 10
   NMIs and the second flashing rate every 7.
 - Each NMI enables either the CA1 interrupt (rising edge) or the CB1 one (falling edge)
   of the 8800 PIA. The IRQ handler writes the lamps after CA1, or checks the sensors after
   CB1, and disables both.
 - PA0-PA2 select a display digit (0-5, 0 on the left), a key row (0-4) or a switch row
   (5-7), PA3-PA4 the column, keys are read on PA5 and switches on PA6 (active low), PB
   drives the segments (active low). PA7 goes high for about one step unit plus the value
   at 163 on every synchronisation input, or once per cycle without them.
 - The main loop pulses CA2 low. After checking the sensors of a relay board, the IRQ
   handler pulses its CB2 low, except for a mismatch during the start-up flashing.
 - Three sensor mismatches (the count is cleared every 128 step units) hold it in the
   start-up flashing until the count is cleared.
 - The rest of the EEPROM has programs laid out for 2000 (17 and 3 steps) and tables at
   400-611 with pointers to 2xxx, never read by this firmware. With them, the controller
   wouldn't work: 103 s of steady amber and 103 s of all red at start-up, no lamp tables
   for 11 of the 17 steps, no vehicle amber in the 3 step program.

Assumptions, not verified on real hardware:
 - 2000-23FF are the two UM6114, battery backed (there's no battery in the PCB drawing,
   but the firmware never initializes them). The clear range command stops at 2800, and
   the unused EEPROM data suggests that another firmware version had the EEPROM at 2000.
 - The NMI is 20 Hz: step units of one second and the amber flashing at 1 Hz. It would be
   the 100 Hz mains zero crossings divided by five (74LS90), and CA1 and CB1 would get a
   50 Hz square wave from the mains (so the lamps switch at the zero crossings).
 - PA7 is a synchronisation output, and the assignment of the switches and optocoupler
   inputs (see the input ports).
 - The 74LS122 of each PCB is a watchdog retriggered by the CA2 or CB2 pulses.
 - The amber of 2 B and the one of 3 A are flashing amber arrows beside the heads of the
   first and second vehicle phases, to turn with caution for the pedestrians while the
   phase is red.

Programs 1-4 at 100, 200, 300, 400 (the firmware has no more):
 +0..+23   step durations in seconds, run from step N-1 down to step 0 (main green)
 +24       number of steps N
 +25..+27  start-up all red, steady amber and flashing durations (always the ones of the
           first program, the flashing one at 127: 0 = 256 s)
 +29       synchronisation offset
 +30..+53  non zero if the step also times out in manual mode
Other parameters: 160 lamp monitor mask (lamp bits, 0 = no check), 161/162 synchronisation
limits (maximum wait, shortening window), 163 synchronisation output pulse length, 164 step
after which the lamps rest in step 0 until there is a pedestrian demand (a step that never
comes, such as 255, gives fixed time cycles).
The lamps lit in each step come from the lamp tables in the EEPROM, so the meaning of each
duration depends on the installation.

How to program it, step by step (a crosswalk cycle for the lamp tables in the EEPROM):
The display shows a 3 digit address, a dot and the 3 digit value stored there: [100.020]
means that address 100 holds 020. Keys: 0-9, A-F, R, M, N and K on the PC keyboard, or
click them on the panel. The dot after the last digit is an indicator (see steps 10 and
11) and is left out below.

 1. Leave the three toggle switches down (FLASHING, MANUAL and EXT. PROG. off) and start
    the machine. With an empty memory the lamps flash amber and the display shows
    [000.000]. After about four minutes of flashing, if no step has a duration yet, the
    firmware loops forever looking for one and the controller stops (the display no
    longer reacts; the watchdog, not emulated, may restart it): reset it (F3) and go on,
    stored values are kept.
 2. Press R. The display goes blank: [   .   ].
 3. Type 1 0 0. The display shows [100.000]: address 100, value 000.
 4. Type 0 2 0 (20 seconds of green for the vehicles). The display shows [100.020].
 5. Press K to store it. The display still shows [100.020].
 6. Press K again to go to the next address. The display shows [101.000].
 7. For each of these addresses type the value, press K to store it and K again to go to
    the next one (all the times are in seconds):
      101  003  pedestrian clearance (all red)
      102  005  flashing pedestrian green (amber for the second vehicle phase)
      103  010  pedestrian green (and green for the second vehicle phase)
      104  002  vehicle clearance (all red)
      105  003  vehicle amber (no need to press K twice after this one)
 8. Press R, type 1 2 4 ([124.000]) and enter these values the same way:
      124  006  number of steps
      125  003  red at start-up
      126  003  steady amber at start-up
      127  005  flashing amber at start-up
 9. Press R, type 1 6 1 and then 0 6 0 ([161.060]) and press K. This is the maximum
    synchronisation wait; without it a green time can last about four minutes.
10. Reset the machine (F3). The lamps flash amber for 5 seconds, show steady amber for 3,
    red for 3 and then green, where they stay. The dot after the last digit goes off: the
    controller is waiting for a pedestrian.
11. Press the pedestrian button (Enter, or PUSH on the panel). The dot after the last
    digit lights up to confirm it; if it doesn't, try again a bit later. When the current
    cycle ends (it can take a minute, a little more just after a reset) the vehicle lamps
    turn amber (3 s) and red, 2 s later the pedestrians get green (10 s), then flashing
    green (5 s) and red, and 3 s later the vehicles get green again. The first pole has the
    first vehicle phase and the pedestrian signal, the second one the second vehicle phase,
    which has green along with the pedestrians; each vehicle head has a flashing amber
    turn arrow beside it.

Other keys and tips:
 - To check a value press R and type its address; to change it type the new value and K.
 - After a mistake press R and start again from the address: a value is only stored when
   exactly three digits are followed by K. Values go from 000 to 255 (higher ones are
   stored modulo 256).
 - M switches to hexadecimal: 4 address digits (any address, e.g. C625 for the lamp tables
   in the EEPROM) and 2 value digits. Press M again to go back.
 - N clears a range: R, the first address, the last address typed as the value, and N.
   For example R 2 0 0 2 9 9 N clears the second program.
 - Addresses 000-099 show the internal RAM, and only 086-095 can be changed. 091 selects
   the program (0 to 3 for the ones at 100 to 400) when EXT. PROG. is off; it goes back to
   0 after every reset. With EXT. PROG. on, PROG 0 and PROG 1 select it, and 3 selects the
   flashing instead. A new program starts when the current cycle ends.

TODO:
 - verify the memory map, the NMI and IRQ sources and the switch / input assignments
 - watchdogs: what the 74LS122 on the main and relays PCBs do on timeout is unknown
 - the lamp current sensors always report working lamps
 - the manual step input: the firmware advances when it goes inactive, so with a normally
   open button (as emulated) the step changes on release, and switching to manual mode
   skips the current step at once (unless it also times out in manual mode); maybe it's
   a normally closed button

***************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/clock.h"
#include "machine/eeprompar.h"
#include "machine/input_merger.h"
#include "machine/nvram.h"
#include "video/pwm.h"

#include <algorithm>
#include <iterator>

#include "semcrossw.lh"


namespace {

class semcrossw_state : public driver_device
{
public:
	semcrossw_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pia(*this, "pia")
		, m_relay_pia(*this, "relay_pia%u", 1U)
		, m_display(*this, "display")
		, m_keys(*this, "KEY%u", 0U)
		, m_switches(*this, "SW%u", 0U)
		, m_lamps(*this, "lamp%u_%u", 1U, 0U)
		, m_sync_out(*this, "sync_out")
	{
	}

	void semcrossw(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// the lamp tables in the EEPROM drive three relay boards, the firmware supports ten
	static constexpr unsigned RELAY_BOARDS = 3;

	required_device<m6802_cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia;
	required_device_array<pia6821_device, RELAY_BOARDS> m_relay_pia;
	required_device<pwm_display_device> m_display;
	required_ioport_array<5> m_keys;
	required_ioport_array<3> m_switches;
	output_finder<RELAY_BOARDS, 8> m_lamps;
	output_finder<> m_sync_out;

	u8 m_pia_pa = 0xff;
	u8 m_pia_pb = 0xff;
	u8 m_relay_pa[RELAY_BOARDS];
	u8 m_nmi_div = 0;

	void mem_map(address_map &map) ATTR_COLD;

	u8 pia_pa_r();
	void pia_pa_w(u8 data);
	void pia_pb_w(u8 data);
	void update_display();

	u8 relay_r(offs_t offset);
	void relay_w(offs_t offset, u8 data);
	template <unsigned N> void relay_pa_w(u8 data);
	template <unsigned N> u8 relay_pb_r();

	void mains_w(int state);
};


void semcrossw_state::machine_start()
{
	std::fill(std::begin(m_relay_pa), std::end(m_relay_pa), 0xff);

	save_item(NAME(m_pia_pa));
	save_item(NAME(m_pia_pb));
	save_item(NAME(m_relay_pa));
	save_item(NAME(m_nmi_div));
}


u8 semcrossw_state::pia_pa_r()
{
	// the firmware selects the rows with PA0-PA2 and the columns with PA3-PA4; the
	// programmer PCB has a 74LS155 and a 4052 that would do it (not verified)
	u8 const row = m_pia_pa & 0x07;
	u8 const col = BIT(m_pia_pa, 3, 2);
	u8 data = 0xff;

	if (row < 5)
	{
		if (!BIT(m_keys[row]->read(), col))
			data &= ~0x20;
	}
	else if (col < 3)
	{
		if (!BIT(m_switches[col]->read(), row - 5))
			data &= ~0x40;
	}

	return data;
}

void semcrossw_state::pia_pa_w(u8 data)
{
	m_pia_pa = data;
	m_sync_out = BIT(data, 7);
	update_display();
}

void semcrossw_state::pia_pb_w(u8 data)
{
	m_pia_pb = data;
	update_display();
}

void semcrossw_state::update_display()
{
	u8 const sel = m_pia_pa & 0x07;
	m_display->matrix((sel < 6) ? (1 << sel) : 0, ~m_pia_pb & 0xff);
}


u8 semcrossw_state::relay_r(offs_t offset)
{
	u8 data = 0xff;
	for (unsigned i = 0; i < RELAY_BOARDS; i++)
	{
		if (BIT(offset, i + 2))
			data &= m_relay_pia[i]->read(offset & 0x03);
	}

	return data;
}

void semcrossw_state::relay_w(offs_t offset, u8 data)
{
	for (unsigned i = 0; i < RELAY_BOARDS; i++)
	{
		if (BIT(offset, i + 2))
			m_relay_pia[i]->write(offset & 0x03, data);
	}
}

template <unsigned N>
void semcrossw_state::relay_pa_w(u8 data)
{
	m_relay_pa[N] = data;
	for (unsigned i = 0; i < 8; i++)
	{
		if (i != 3 && i != 7)
			m_lamps[N][i] = BIT(~data, i);
	}
}

template <unsigned N>
u8 semcrossw_state::relay_pb_r()
{
	// lamp current sensors: the firmware expects them to read like the outputs when the
	// lamps work
	return m_relay_pa[N];
}


void semcrossw_state::mains_w(int state)
{
	// assumed: CA1 and CB1 get the mains square wave (the firmware writes the lamps on a
	// rising edge and checks the sensors on a falling one), and the NMI (20 Hz according
	// to the firmware timings) is the zero crossings divided by five
	m_pia->ca1_w(state);
	m_pia->cb1_w(state);

	if (++m_nmi_div == 5)
	{
		m_nmi_div = 0;
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
}


void semcrossw_state::mem_map(address_map &map)
{
	map(0x2000, 0x23ff).ram().share("nvram");
	// 8400-8403: the firmware initializes a PIA here, but never uses it
	map(0x8800, 0x8803).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xa000, 0xafff).rw(FUNC(semcrossw_state::relay_r), FUNC(semcrossw_state::relay_w));
	map(0xc000, 0xc7ff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write));
	map(0xf800, 0xffff).rom().region("maincpu", 0);
}


INPUT_PORTS_START(semcrossw)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("R (Clear)") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("M (Decimal/Hex Mode)") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("N (Clear Range)") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("K (Store/Next)") PORT_CODE(KEYCODE_K)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	// the names and the switch / optocoupler assignment come from what the firmware does
	// with each input (not verified); the firmware sees a low input as active
	PORT_START("SW0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_TOGGLE PORT_NAME("External Program Select Bit 0") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_TOGGLE PORT_NAME("External Program Select Bit 1") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Pedestrian Request") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SW1") // flashing and manual modes are selected with the input inactive (high)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_TOGGLE PORT_NAME("Flashing Mode") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_TOGGLE PORT_NAME("Manual Mode") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_TOGGLE PORT_NAME("External Program Selection") PORT_CODE(KEYCODE_X)
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SW2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Synchronisation Input") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Manual Step Advance") PORT_CODE(KEYCODE_SPACE) // the firmware advances when it goes inactive (see TODO)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


void semcrossw_state::semcrossw(machine_config &config)
{
	M6802(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &semcrossw_state::mem_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // 2 x UM6114, battery backed

	EEPROM_2816(config, "eeprom");

	PIA6821(config, m_pia);
	m_pia->readpa_handler().set(FUNC(semcrossw_state::pia_pa_r));
	m_pia->writepa_handler().set(FUNC(semcrossw_state::pia_pa_w));
	m_pia->writepb_handler().set(FUNC(semcrossw_state::pia_pb_w));
	m_pia->ca2_handler().set_nop(); // pulsed low by the main loop, watchdog?
	m_pia->cb2_handler().set_nop(); // left floating while CB1 is enabled, unknown use
	m_pia->irqa_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));
	m_pia->irqb_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, M6802_IRQ_LINE);

	CLOCK(config, "mains", 50).signal_handler().set(FUNC(semcrossw_state::mains_w));

	for (auto &pia : m_relay_pia)
	{
		PIA6821(config, pia);
		pia->cb2_handler().set_nop(); // pulsed low after checking the lamp sensors, watchdog?
	}
	m_relay_pia[0]->writepa_handler().set(FUNC(semcrossw_state::relay_pa_w<0>));
	m_relay_pia[0]->readpb_handler().set(FUNC(semcrossw_state::relay_pb_r<0>));
	m_relay_pia[1]->writepa_handler().set(FUNC(semcrossw_state::relay_pa_w<1>));
	m_relay_pia[1]->readpb_handler().set(FUNC(semcrossw_state::relay_pb_r<1>));
	m_relay_pia[2]->writepa_handler().set(FUNC(semcrossw_state::relay_pa_w<2>));
	m_relay_pia[2]->readpb_handler().set(FUNC(semcrossw_state::relay_pb_r<2>));

	PWM_DISPLAY(config, m_display).set_size(6, 8);
	m_display->set_segmask(0x3f, 0xff);

	config.set_default_layout(layout_semcrossw);
}


ROM_START(semcrossw)
	ROM_REGION(0x800, "maincpu", 0)
	ROM_LOAD("at27c16.bin",    0x000, 0x800, CRC(2e7b10b1) SHA1(fba6465db1baa38ab79ed24a85de460f8be488b9))

	ROM_REGION(0x800, "eeprom", 0)
	// EEPROM configuration: lamp tables for three relay boards at 0x625, and
	// data for another firmware version that this one never reads
	ROM_LOAD("x2816cp-12.bin", 0x000, 0x800, CRC(c2ef2e80) SHA1(6c3c4215169c2941a37053888174fe0499301bac))
ROM_END

} // anonymous namespace


//   YEAR  NAME       PARENT COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY  FULLNAME                                              FLAGS
SYST(198?, semcrossw, 0,     0,      semcrossw, semcrossw, semcrossw_state, empty_init, "Etra",  "Crosswalk traffic light controller (unknown model)", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE)
