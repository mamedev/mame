// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Fidelity Elite A/S series hardware (EAS, EAG, PC)
see eag68k.cpp for 68000-based EAG hardware

NOTE: To start a new game in EAS/PC, press Game Control (aka Reset), activate
the D6 square, and then press CL. See below for more info.

BTANB:
- feasglab locks up at boot if it was powered off in the middle of the game.
  To resolve this, hold the Game Control button while booting to clear nvram.
  The ROM dump was verified from 2 chesscomputers.

********************************************************************************

Elite A/S Challenger (EAS)
---------------------------------
This came out in 1983. 2 program updates were released in 1983 and 1984,
named Budapest and Glasgow, places where Fidelity competed in chess computer
matches (they won it in 1983). A/S stands for auto sensory, it's the 1st
Fidelity board with magnet sensors. The magnetic chessboard was licensed from
AVE Micro Systems, in fact, the PC model board is the same one as in AVE's ARB.

Hardware notes:
- PCB label: FIDELITY ELECTRONICS, 510-1071A01, copyright 1983
- R65C02P4 or R6502BP CPU, default frequency 3MHz*
- 4KB RAM (2*HM6116), 24KB ROM
- TSI S14001A + speech ROM
- I/O with 8255 PPI and bunch of TTL
- 8*8 magnet sensors, 11 buttons, 8*(8+1) LEDs + 4*7seg LEDs
- module slot and printer port

*In West Germany, some distributors released it with overclocked CPUs,
advertised as 3.2, 3.6, or 4MHz. Unmodified EAS PCB photos show only a 3MHz XTAL.
Though model EWC(improved Budapest) had a 3.57MHz XTAL and EAS-C(Glasgow) had
a 4MHz XTAL. Model E4.0 is also assumed to be 4MHz.

A condensator/battery keeps RAM contents alive for a while when powered off.

Note that EAS/PC doesn't have a "new game" button, it is done through game options:
Press GAME CONTROL, then place/lift a piece on D6 to restart with user settings
intact, or D8 to reset and clear memory, then press CL.

Anecdote from Ron Nelson regarding the new game issue (Sid Samole was the CEO):
"The next year I designed it into a wooden housing, and Kathy said she wanted to
design the User Interface. I said ok go for it. When it was finished & masked,
I was at an internal sales meeting. No one could figure out how to start a new game.
Sid turned to me with a questioning look. I showed him how to do it (multiple key
presses as I recall) and explained about Kathy. He said, don't let them do it again."

Prestige Challenger (model PC, 1982 510-1050A01 PCB) hardware is very similar. It
was released before EAS, it doesn't have the 8255 PPI, but has more RAM (7*TMM2016P).
Some were released at 3.6MHz instead of 4MHz, maybe due to hardware instability?
Opening module PC16 was included by default, this module is the same as CB16 but
at different form factor.

There's also an unreleased prototype on a newer 1985 PCB revision (Fidelity Computer
Products, 510-1071B01 PCB). It has 16KB RAM (2*HM6264LP-15), a 5MHz CPU, IRQ is via
a 556 timer instead of a 38.4kHz resonator. This board came from Peter Reckwitz's
inventory, he was a Fidelity representative in West Germany at the time. Considering
the "EXP" (experimental?) EPROM labels, it can be speculated that this version
competed in one of the chess computer tournaments.

Elite Avant Garde (models 6081,6088,6089, Fidelity International, 510-1071C01 PCB)
is nearly the same as the 510-1071B01 PCB above, with one extra 7seg panel. Level
B8 starts a self-test and displays ROM checksums, press CL to advance.

Fidelity Elite Private Line were EAS/EAG conversions released by Fidelity Deutschland.
The "Elite Privat" was probably for the local market and the "Private Line" for
export. They took out the motherboard and leds and placed them inside a little box
separate from a (ledless) magnetic chessboard. The ROMs were unmodified, that makes
them uninteresting to emulate as separate drivers.

*******************************************************************************/

#include "emu.h"
#include "clockdiv.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/m6502/m65c02.h"
#include "cpu/m6502/r65c02.h"
#include "machine/clock.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "sound/s14001a.h"
#include "video/pwm.h"

#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "fidel_eag.lh"
#include "fidel_eas.lh"
#include "fidel_pc.lh"


namespace {

// note: sub-class of fidel_clockdiv_state (see clockdiv.*)

// EAS / shared

class elite_state : public fidel_clockdiv_state
{
public:
	elite_state(const machine_config &mconfig, device_type type, const char *tag) :
		fidel_clockdiv_state(mconfig, type, tag),
		m_ppi8255(*this, "ppi8255"),
		m_rombank(*this, "rombank"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_speech(*this, "speech"),
		m_speech_rom(*this, "speech"),
		m_language(*this, "language"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine configs
	void pc(machine_config &config);
	void eas(machine_config &config);
	void ewc(machine_config &config);
	void easc(machine_config &config);
	void easx(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(change_cpu_freq);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override { fidel_clockdiv_state::machine_reset(); }

	// devices/pointers
	optional_device<i8255_device> m_ppi8255;
	optional_memory_bank m_rombank;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_device<s14001a_device> m_speech;
	required_region_ptr<u8> m_speech_rom;
	required_region_ptr<u8> m_language;
	required_ioport_array<2> m_inputs;

	u8 m_led_data = 0;
	u8 m_7seg_data = 0;
	u8 m_inp_mux = 0;
	u8 m_speech_bank = 0;

	// address maps
	void eas_map(address_map &map);
	void eag_map(address_map &map);
	void pc_map(address_map &map);

	// I/O handlers
	void update_display();
	u8 speech_r(offs_t offset);
	void segment_w(offs_t offset, u8 data);
	void led_w(offs_t offset, u8 data);
	u8 input_r();
	virtual u8 board_r() { return m_board->read_file(m_inp_mux, true); }
	void ppi_porta_w(u8 data);
	u8 ppi_portb_r();
	void ppi_portc_w(u8 data);
};

void elite_state::machine_start()
{
	fidel_clockdiv_state::machine_start();

	// register for savestates
	save_item(NAME(m_led_data));
	save_item(NAME(m_7seg_data));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_speech_bank));
}

INPUT_CHANGED_MEMBER(elite_state::change_cpu_freq)
{
	// known official CPU speeds: 3MHz(EAS), 3.57MHz(PC/EWC/Privat), 4MHz(PC/EAS-C)
	static const XTAL xtal[3] = { 3_MHz_XTAL, 3.579545_MHz_XTAL, 4_MHz_XTAL };
	m_maincpu->set_unscaled_clock(xtal[newval % 3]);
}


// EAG

class eag_state : public elite_state
{
public:
	eag_state(const machine_config &mconfig, device_type type, const char *tag) :
		elite_state(mconfig, type, tag)
	{ }

	// machine configs
	void eag(machine_config &config);
	void eag2100(machine_config &config);

	void init_eag2100();

protected:
	virtual void machine_reset() override;

private:
	void eag2100_map(address_map &map);

	// board is rotated 90 degrees
	virtual u8 board_r() override { return m_board->read_rank(m_inp_mux); }
};

void eag_state::init_eag2100()
{
	m_rombank->configure_entries(0, 4, memregion("rombank")->base(), 0x2000);
}

void eag_state::machine_reset()
{
	elite_state::machine_reset();

	if (m_rombank != nullptr)
		m_rombank->set_entry(0);
}



/*******************************************************************************
    I/O
*******************************************************************************/

// TTL/generic

void elite_state::update_display()
{
	// 4/8 7seg leds+H, 8*8(+1) chessboard leds
	u8 seg_data = bitswap<8>(m_7seg_data,0,1,3,2,7,5,6,4);
	m_display->matrix(1 << m_inp_mux, m_led_data << 8 | seg_data);
}

u8 elite_state::speech_r(offs_t offset)
{
	return m_speech_rom[m_speech_bank << 12 | offset];
}

void elite_state::segment_w(offs_t offset, u8 data)
{
	// a0-a2,d7: digit segment
	u8 mask = 1 << offset;
	m_7seg_data = (m_7seg_data & ~mask) | ((data & 0x80) ? mask : 0);
	update_display();
}

void elite_state::led_w(offs_t offset, u8 data)
{
	// a0-a2,d0: led data
	m_led_data = (m_led_data & ~(1 << offset)) | ((data & 1) << offset);
	update_display();
}

u8 elite_state::input_r()
{
	u8 data = 0;

	// multiplexed inputs (active low)
	// read chessboard sensors
	if (m_inp_mux < 8)
		data = board_r();

	// read button panel
	else if (m_inp_mux == 8)
		data = m_inputs[0]->read();

	return ~data;
}


// 8255 PPI (PC: done with TTL instead)

void elite_state::ppi_porta_w(u8 data)
{
	// d0-d5: TSI C0-C5
	// d6: TSI START line
	m_speech->data_w(data & 0x3f);
	m_speech->start_w(data >> 6 & 1);

	// d7: printer? (black wire to LED pcb)
}

void elite_state::ppi_portc_w(u8 data)
{
	// d0-d3: 7442 a0-a3
	// 7442 0-8: led select, input mux
	m_inp_mux = data & 0xf;
	update_display();

	// 7442 9: speaker out
	m_dac->write(BIT(1 << m_inp_mux, 9));

	// d4: speech ROM A12
	m_speech->force_update(); // update stream to now
	m_speech_bank = data >> 4 & 1;

	// d5: lower TSI volume
	m_speech->set_output_gain(0, (data & 0x20) ? 0.25 : 1.0);

	// d6,d7: bookrom bankswitch (model EAG)
	if (m_rombank != nullptr)
		m_rombank->set_entry(data >> 6 & 3);
}

u8 elite_state::ppi_portb_r()
{
	// d0: printer? white wire from LED pcb
	u8 data = 1;

	// d1: TSI BUSY line
	data |= (m_speech->busy_r()) ? 2 : 0;

	// d2,d3: language switches(hardwired)
	data |= *m_language << 2 & 0x0c;

	// d5: 3 more buttons
	data |= (BIT(m_inputs[1]->read(), m_inp_mux)) ? 0 : 0x20;

	// other: ?
	return data | 0xd0;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void elite_state::pc_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x17ff).ram();
	map(0x2000, 0x5fff).r("cartslot", FUNC(generic_slot_device::read_rom));
	map(0x7000, 0x7000).w(FUNC(elite_state::ppi_porta_w));
	map(0x7010, 0x7010).r(FUNC(elite_state::ppi_portb_r));
	map(0x7020, 0x7027).w(FUNC(elite_state::segment_w)).nopr();
	map(0x7030, 0x7037).w(FUNC(elite_state::led_w)).nopr();
	map(0x7040, 0x7040).w(FUNC(elite_state::ppi_portc_w));
	map(0x7050, 0x7050).r(FUNC(elite_state::input_r));
	map(0x8000, 0x9fff).ram();
	map(0xb000, 0xffff).rom();
}

void elite_state::eas_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x0800, 0x0fff).ram();
	map(0x2000, 0x5fff).r("cartslot", FUNC(generic_slot_device::read_rom));
	map(0x7000, 0x7003).rw(m_ppi8255, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x7020, 0x7027).w(FUNC(elite_state::segment_w)).nopr();
	map(0x7030, 0x7037).w(FUNC(elite_state::led_w)).nopr();
	map(0x7050, 0x7050).r(FUNC(elite_state::input_r));
	map(0x8000, 0x9fff).rom();
	map(0xc000, 0xffff).rom();
}

void elite_state::eag_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).ram().share("nvram.ic8");
	map(0x2000, 0x5fff).r("cartslot", FUNC(generic_slot_device::read_rom));
	map(0x7000, 0x7003).rw(m_ppi8255, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x7020, 0x7027).w(FUNC(elite_state::segment_w)).nopr();
	map(0x7030, 0x7037).w(FUNC(elite_state::led_w)).nopr();
	map(0x7050, 0x7050).r(FUNC(elite_state::input_r));
	map(0x8000, 0x9fff).ram().share("nvram.ic6");
	map(0xa000, 0xffff).rom();
}

void eag_state::eag2100_map(address_map &map)
{
	eag_map(map);
	map(0xa000, 0xbfff).bankr(m_rombank);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( eas )
	PORT_INCLUDE( fidel_clockdiv_4 )

	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Game Control")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Speaker")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("PB / King")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("PV / Queen")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("TM / Rook")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("ST / Bishop")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("TB / Knight")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("LV / Pawn")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("DM")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("CL")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("RV")

	PORT_START("CPU")
	PORT_CONFNAME( 0x03, 0x00, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, elite_state, change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "3MHz (EAS)" )
	PORT_CONFSETTING(    0x01, "3.57MHz (EWC)" )
	PORT_CONFSETTING(    0x02, "4MHz (EAS-C)" )
INPUT_PORTS_END

static INPUT_PORTS_START( ewc )
	PORT_INCLUDE( eas )

	PORT_MODIFY("CPU") // default to 3.57MHz
	PORT_CONFNAME( 0x03, 0x01, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, elite_state, change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "3MHz (EAS)" )
	PORT_CONFSETTING(    0x01, "3.57MHz (EWC)" )
	PORT_CONFSETTING(    0x02, "4MHz (EAS-C)" )
INPUT_PORTS_END

static INPUT_PORTS_START( easc )
	PORT_INCLUDE( eas )

	PORT_MODIFY("CPU") // default to 4MHz
	PORT_CONFNAME( 0x03, 0x02, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, elite_state, change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "3MHz (EAS)" )
	PORT_CONFSETTING(    0x01, "3.57MHz (EWC)" )
	PORT_CONFSETTING(    0x02, "4MHz (EAS-C)" )
INPUT_PORTS_END

static INPUT_PORTS_START( pc )
	PORT_INCLUDE( easc )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Reset") // led display still says - G C -
INPUT_PORTS_END

static INPUT_PORTS_START( easx )
	PORT_INCLUDE( eas )

	PORT_MODIFY("CPU") // 5MHz
	PORT_BIT(0x03, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( eag )
	PORT_INCLUDE( fidel_clockdiv_4 )

	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("RV")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Option")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("LV / Pawn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("TB / Knight")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("ST / Bishop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("TM / Rook")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("PV / Queen")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("PB / King")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("CL")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("DM")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void elite_state::pc(machine_config &config)
{
	// basic machine hardware
	R65C02(config, m_maincpu, 4_MHz_XTAL); // R65C02P3/4
	m_maincpu->set_addrmap(AS_PROGRAM, &elite_state::pc_map);

	auto &irq_clock(CLOCK(config, "irq_clock", 38.4_kHz_XTAL/64)); // through 4060 IC, 600Hz
	irq_clock.set_pulse_width(attotime::from_nsec(13700)); // active for 13.7us
	irq_clock.signal_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(100));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 16);
	m_display->set_segmask(0xf, 0x7f);
	config.set_default_layout(layout_fidel_pc);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	S14001A(config, m_speech, 25000); // R/C circuit, around 25khz
	m_speech->ext_read().set(FUNC(elite_state::speech_r));
	m_speech->add_route(ALL_OUTPUTS, "speaker", 0.75);

	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);

	// cartridge
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "fidel_scc");
	SOFTWARE_LIST(config, "cart_list").set_original("fidel_scc");
}

void elite_state::eas(machine_config &config)
{
	pc(config);

	// basic machine hardware
	m_maincpu->set_clock(3_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &elite_state::eas_map);

	I8255(config, m_ppi8255); // port B: input, port A & C: output
	m_ppi8255->out_pa_callback().set(FUNC(elite_state::ppi_porta_w));
	m_ppi8255->tri_pa_callback().set_constant(0);
	m_ppi8255->in_pb_callback().set(FUNC(elite_state::ppi_portb_r));
	m_ppi8255->out_pc_callback().set(FUNC(elite_state::ppi_portc_w));
	m_ppi8255->tri_pc_callback().set_constant(0);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	m_board->set_nvram_enable(true);

	config.set_default_layout(layout_fidel_eas);
}

void elite_state::ewc(machine_config &config)
{
	eas(config);
	m_maincpu->set_clock(3.579545_MHz_XTAL);
}

void elite_state::easc(machine_config &config)
{
	eas(config);
	m_maincpu->set_clock(4_MHz_XTAL);
}

void elite_state::easx(machine_config &config)
{
	eas(config);

	// basic machine hardware
	m_maincpu->set_clock(5_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &elite_state::eag_map);

	auto &irq_clock(CLOCK(config.replace(), "irq_clock", 600)); // from 556 timer (22nF, 82K+pot, 1K), ideal frequency is 600Hz
	irq_clock.set_pulse_width(attotime::from_nsec(15250)); // active for 15.25us
	irq_clock.signal_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	config.device_remove("nvram");
	NVRAM(config, "nvram.ic8", nvram_device::DEFAULT_ALL_0);
	NVRAM(config, "nvram.ic6", nvram_device::DEFAULT_ALL_0);
}

void eag_state::eag(machine_config &config)
{
	easx(config);

	// video hardware
	m_display->set_segmask(0x1ef, 0x7f);
	config.set_default_layout(layout_fidel_eag);
}

void eag_state::eag2100(machine_config &config)
{
	eag(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &eag_state::eag2100_map);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( feas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("elite", 0x8000, 0x0800, CRC(cc904af9) SHA1(2b1d54c597a445ccc1ec73838e4bd2ac5154d7ad) ) // Intel D2764-2
	ROM_CONTINUE( 0x9000, 0x0800 )
	ROM_CONTINUE( 0x8800, 0x0800 )
	ROM_CONTINUE( 0x9800, 0x0800 )
	ROM_LOAD("101-1052a02", 0xc000, 0x2000, CRC(fee42863) SHA1(1854bb9cddb883d86e0d98594ac0186fb3908a01) ) // Mostek MK36C63N-5
	ROM_LOAD("101-1052a01", 0xe000, 0x2000, CRC(41261e1b) SHA1(43664e30348a708fbe449b47e6eed39d4e9acb4c) ) // "

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) ) // NEC D2332C
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) ) // NEC D2364C
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) ) // "
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) ) // "
ROM_END

ROM_START( feasbu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("eli_bu3", 0x8000, 0x0800, CRC(93dcc23b) SHA1(2eb8c5a85e566948bc256d6b1804694e6b0ffa6f) )
	ROM_CONTINUE( 0x9000, 0x0800 )
	ROM_CONTINUE( 0x8800, 0x0800 )
	ROM_CONTINUE( 0x9800, 0x0800 )
	ROM_LOAD("eli_bu1", 0xc000, 0x2000, CRC(859d69f1) SHA1(a8b057683369e2387f22fc7e916b6f3c75d44b21) )
	ROM_LOAD("eli_bu2", 0xe000, 0x2000, CRC(571a33a7) SHA1(43b110cf0918caf16643178f401e58b2dc73894f) )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

ROM_START( feasbua ) // model EWC
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("white_a", 0x8000, 0x0800, CRC(93dcc23b) SHA1(2eb8c5a85e566948bc256d6b1804694e6b0ffa6f) ) // HN482764G-2
	ROM_CONTINUE( 0x9000, 0x0800 )
	ROM_CONTINUE( 0x8800, 0x0800 )
	ROM_CONTINUE( 0x9800, 0x0800 )
	ROM_LOAD("black", 0xc000, 0x0800, CRC(132b7f68) SHA1(a7aaac221387275f3a11f4441dcef73c2bfd0ee6) ) // M5L2764K
	ROM_CONTINUE( 0xd000, 0x0800 )
	ROM_CONTINUE( 0xc800, 0x0800 )
	ROM_CONTINUE( 0xd800, 0x0800 )
	ROM_LOAD("green", 0xe000, 0x0800, CRC(c7bbfbbe) SHA1(63fe13d0e64d1e5c1ea1b4de13ac3e753797a992) ) // M5L2764K - only 4 bytes different to feasbu (after descramble)
	ROM_CONTINUE( 0xf000, 0x0800 )
	ROM_CONTINUE( 0xe800, 0x0800 )
	ROM_CONTINUE( 0xf800, 0x0800 )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

ROM_START( feasgla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("4.0_8_6", 0x8000, 0x0800, CRC(32784e2d) SHA1(dae060a5c49cc1993a78db293cd80464adfd892d) )
	ROM_CONTINUE( 0x9000, 0x0800 )
	ROM_CONTINUE( 0x8800, 0x0800 )
	ROM_CONTINUE( 0x9800, 0x0800 )
	ROM_LOAD("4.0_c_5", 0xc000, 0x0800, CRC(ddb80412) SHA1(b1d9435d9a71b8eb241a2169bfbaa0499f510769) )
	ROM_CONTINUE( 0xd000, 0x0800 )
	ROM_CONTINUE( 0xc800, 0x0800 )
	ROM_CONTINUE( 0xd800, 0x0800 )
	ROM_LOAD("4.0_e_4", 0xe000, 0x0800, CRC(62a5305a) SHA1(a361bd9a54b903d7b0fbacabe55ea5ccbbc1dc51) )
	ROM_CONTINUE( 0xf000, 0x0800 )
	ROM_CONTINUE( 0xe800, 0x0800 )
	ROM_CONTINUE( 0xf800, 0x0800 )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

ROM_START( feasglaa ) // model EAS-C
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("orange", 0x8000, 0x0800, CRC(32784e2d) SHA1(dae060a5c49cc1993a78db293cd80464adfd892d) )
	ROM_CONTINUE( 0x9000, 0x0800 )
	ROM_CONTINUE( 0x8800, 0x0800 )
	ROM_CONTINUE( 0x9800, 0x0800 )
	ROM_LOAD("black", 0xc000, 0x0800, CRC(3f0b01b6) SHA1(fe8d214f1678e000ba945e2f6dc3438af97c6f33) ) // only 2 bytes different to feasgla
	ROM_CONTINUE( 0xd000, 0x0800 )
	ROM_CONTINUE( 0xc800, 0x0800 )
	ROM_CONTINUE( 0xd800, 0x0800 )
	ROM_LOAD("green", 0xe000, 0x0800, CRC(62a5305a) SHA1(a361bd9a54b903d7b0fbacabe55ea5ccbbc1dc51) )
	ROM_CONTINUE( 0xf000, 0x0800 )
	ROM_CONTINUE( 0xe800, 0x0800 )
	ROM_CONTINUE( 0xf800, 0x0800 )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

ROM_START( feasglab )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("6a", 0x8000, 0x0800, CRC(2fdddb4f) SHA1(6da0a328a45462f285ae6a0756f97c5a43148f97) )
	ROM_CONTINUE( 0x9000, 0x0800 )
	ROM_CONTINUE( 0x8800, 0x0800 )
	ROM_CONTINUE( 0x9800, 0x0800 )
	ROM_LOAD("5a", 0xc000, 0x0800, CRC(f094e625) SHA1(fef84c6a3da504aac15988ec9af94417e5fedfbd) )
	ROM_CONTINUE( 0xd000, 0x0800 )
	ROM_CONTINUE( 0xc800, 0x0800 )
	ROM_CONTINUE( 0xd800, 0x0800 )
	ROM_LOAD("4a", 0xe000, 0x0800, CRC(5f6845d1) SHA1(684eb16faf36a49560e5a73b55fd0022dc090e35) )
	ROM_CONTINUE( 0xf000, 0x0800 )
	ROM_CONTINUE( 0xe800, 0x0800 )
	ROM_CONTINUE( 0xf800, 0x0800 )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

ROM_START( feasx ) // 510-1071B01 PCB
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("exp_8000_6", 0xa000, 0x2000, CRC(b555c5ab) SHA1(d85ae44f0c13e2bfafbc3cadf829e74c7f4ba2e3) ) // Seeq DQ5133-25
	ROM_LOAD("exp_c000_5", 0xc000, 0x2000, CRC(fd8471e3) SHA1(e684ded8ed4934bc5ef0cc4ae37dc5d12496d39e) ) // "
	ROM_LOAD("exp_e000_4", 0xe000, 0x2000, CRC(19c36d83) SHA1(33438c316284182ebe195a383bb2d96d3524c88d) ) // "

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END


ROM_START( fpres ) // serial 0000082x
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("u09_yellow", 0xb000, 0x1000, CRC(03fac294) SHA1(5a9d72978318c61185efd4bc9e4a868c226465b8) ) // TMS2532JL-45
	ROM_LOAD("u10_green",  0xc000, 0x1000, CRC(97727bd2) SHA1(68cb931db0fc705959d3f59271923602918c3892) ) // "
	ROM_LOAD("u11_black",  0xd000, 0x1000, CRC(98bd01b7) SHA1(48cc560c4ca736f54e30d757990ff403c05c39ae) ) // "
	ROM_LOAD("u12_blue",   0xe000, 0x1000, CRC(f6f1e21c) SHA1(b60a02fee3a9c8f8322da3194170c167a8cca3b7) ) // "
	ROM_LOAD("u13_red",    0xf000, 0x1000, CRC(d5076ebd) SHA1(d2a93256b5ed74301ab4e5150b5b5816eb288be5) ) // "

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

ROM_START( fpresa ) // serial 0000076x
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("u09_yellow", 0xb000, 0x1000, CRC(03fac294) SHA1(5a9d72978318c61185efd4bc9e4a868c226465b8) ) // TMS2532JL-45
	ROM_LOAD("u10_green",  0xc000, 0x1000, CRC(97727bd2) SHA1(68cb931db0fc705959d3f59271923602918c3892) ) // "
	ROM_LOAD("u11_black",  0xd000, 0x1000, CRC(98bd01b7) SHA1(48cc560c4ca736f54e30d757990ff403c05c39ae) ) // "
	ROM_LOAD("u12_blue",   0xe000, 0x1000, CRC(6f18115f) SHA1(a08b3a66bfdc23f3400e03fe253a8b9a4967d14f) ) // "
	ROM_LOAD("u13_red",    0xf000, 0x1000, CRC(1e044d0c) SHA1(2bb7a22ef5c4af7d02d88b250adfc8c313a435fc) ) // "

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

ROM_START( fpresb ) // serial ?
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("u09_yellow", 0xb000, 0x1000, CRC(03fac294) SHA1(5a9d72978318c61185efd4bc9e4a868c226465b8) )
	ROM_LOAD("u10_green",  0xc000, 0x1000, CRC(5d049d5e) SHA1(c7359bead92729e8a92d6cf1789d87ae43d23cbf) )
	ROM_LOAD("u11_black",  0xd000, 0x1000, CRC(98bd01b7) SHA1(48cc560c4ca736f54e30d757990ff403c05c39ae) )
	ROM_LOAD("u12_blue",   0xe000, 0x1000, CRC(6f18115f) SHA1(a08b3a66bfdc23f3400e03fe253a8b9a4967d14f) )
	ROM_LOAD("u13_red",    0xf000, 0x1000, CRC(dea8091d) SHA1(1d94a90ae076215c2c009e78ec4919dbd8467ef8) )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

ROM_START( fpresc ) // serial 0000011x, but updated EPROMs
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("u09", 0xb000, 0x1000, CRC(0782e41e) SHA1(7d3fd37b00e34aa214b56e591d175ea1b6b334f9) ) // TMS2532JL-45, yellow sticker on all
	ROM_LOAD("u10", 0xc000, 0x1000, CRC(6fe6871e) SHA1(882cd8aee8e2a56684cb64a1faa343432bc190aa) ) // "
	ROM_LOAD("u11", 0xd000, 0x1000, CRC(92eb49b5) SHA1(e4fa506f7991e90afd2184b62c28774af023925c) ) // "
	ROM_LOAD("u12", 0xe000, 0x1000, CRC(4916bdfd) SHA1(5fbc06a93b0edc64b04ad192887e81b03c7d40aa) ) // "
	ROM_LOAD("u13", 0xf000, 0x1000, CRC(87ef02f4) SHA1(ac223927d1c6adb4c2d0a008f34be93e52e522f9) ) // "

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

ROM_START( fpresd ) // serial 0000015x
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("u09_yellow", 0xb000, 0x1000, CRC(4d6084fb) SHA1(2c846b3003f347973197b0837c8791cbdf4e1aa3) ) // TMS2532JL-45
	ROM_LOAD("u10_green",  0xc000, 0x1000, CRC(6fe6871e) SHA1(882cd8aee8e2a56684cb64a1faa343432bc190aa) ) // "
	ROM_LOAD("u11_black",  0xd000, 0x1000, CRC(92eb49b5) SHA1(e4fa506f7991e90afd2184b62c28774af023925c) ) // "
	ROM_LOAD("u12_blue",   0xe000, 0x1000, CRC(84cde5ca) SHA1(d60f49e1e92b44bc2ddb9ddada46155cbad27590) ) // "
	ROM_LOAD("u13_red",    0xf000, 0x1000, CRC(46668eb1) SHA1(434a5b6909571f9fc2080fb0962d94d7e8833a79) ) // "

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

ROM_START( fprese ) // serial 0000003x
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("u09_yellow", 0xb000, 0x1000, CRC(4d6084fb) SHA1(2c846b3003f347973197b0837c8791cbdf4e1aa3) ) // TMS2532JDL-45
	ROM_LOAD("u10_green",  0xc000, 0x1000, CRC(6fe6871e) SHA1(882cd8aee8e2a56684cb64a1faa343432bc190aa) ) // "
	ROM_LOAD("u11_black",  0xd000, 0x1000, CRC(6316adc5) SHA1(a98a0c6cc0a9f823a70bae9a4b3bce538d0f58af) ) // "
	ROM_LOAD("u12_blue",   0xe000, 0x1000, CRC(7c6ed6cc) SHA1(8a39678fc28340c208b3da60a83dbf0b509c6604) ) // "
	ROM_LOAD("u13_red",    0xf000, 0x1000, CRC(46668eb1) SHA1(434a5b6909571f9fc2080fb0962d94d7e8833a79) ) // "

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

ROM_START( fpresbu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("u09_yellow", 0xb000, 0x1000, CRC(bb1cb486) SHA1(b83f50a3ef361d254b88eefaa5aac657aaa72375) )
	ROM_LOAD("u10_green",  0xc000, 0x1000, CRC(af0aec0e) SHA1(8293d00a12efa1c142b9e37bc7786012250536d9) )
	ROM_LOAD("u11_black",  0xd000, 0x1000, CRC(214a91cc) SHA1(aab07ecdd66ac208874f4053fc4b0b0659b017aa) )
	ROM_LOAD("u12_blue",   0xe000, 0x1000, CRC(dae4d8e4) SHA1(f06dbb643f0324c0bddaaae9537d5829768bda22) )
	ROM_LOAD("u13_red",    0xf000, 0x1000, CRC(5fb67708) SHA1(1e9ee724c2be38daf39d5cf37b0ae587e408777c) )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

ROM_START( fpresgla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("u09_yellow", 0xb000, 0x1000, CRC(b0a7e8f1) SHA1(f703d56917ea0e2d1dc553e0a9fd5aaf99a8328b) )
	ROM_LOAD("u10_green",  0xc000, 0x1000, CRC(32606946) SHA1(26333f19ec103111f2df83fda9a5660bcfc8d1ec) )
	ROM_LOAD("u11_black",  0xd000, 0x1000, CRC(cecc2094) SHA1(e7860fbcf5a7ef2aa11f7c1e509b1a5de2acfd33) )
	ROM_LOAD("u12_blue",   0xe000, 0x1000, CRC(fe1f25f1) SHA1(3e6b234cb9f05919fead386c9501b3f320db9f10) )
	ROM_LOAD("u13_red",    0xf000, 0x1000, CRC(64d337d2) SHA1(c7573bc15356ddd6e538730d7f059ffa7195d5bf) )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END


ROM_START( feag ) // model 6081, aka "Mobile Master" - checksum BE41 9B27 E959 42C1
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("eg_orange.ic9", 0xa000, 0x2000, CRC(df9e7e74) SHA1(db76750eba5515213ecce07402c4d974c14e1a23) ) // M5L2764K, orange sticker
	ROM_LOAD("eg_black.ic5",  0xc000, 0x2000, CRC(a5f6f295) SHA1(319f00d4b7a1704a3ca722c40f4096004b4b89d2) ) // M5L2764K, black sticker
	ROM_LOAD("eg_green.ic4",  0xe000, 0x2000, CRC(1dc6508a) SHA1(6f2e730b216bfb900074d1d786124fc3cb038a8d) ) // M5L2764K, green sticker

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107.ic16", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(                 0x1000, 0x1000)
	ROMX_LOAD("101-64101.ic16", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105.ic16", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106.ic16", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

ROM_START( feag2100 ) // checksum F234 9D4A 2373 B2F1
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("el2100_2.ic5", 0xc000, 0x2000, CRC(76fec42f) SHA1(34660edb8458919fd179e93fdab3fe428a6625d0) )
	ROM_LOAD("el2100_3.ic4", 0xe000, 0x2000, CRC(2079a506) SHA1(a7bb83138c7b6eff6ea96702d453a214697f4890) )

	ROM_REGION( 0x8000, "rombank", 0 )
	ROM_LOAD("el2100_1.ic9", 0x0000, 0x8000, CRC(9b62b7d5) SHA1(cfcaea2e36c2d52fe4a85c77dbc7fa135893860c) )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107.ic16", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(                 0x1000, 0x1000)
	ROMX_LOAD("101-64101.ic16", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105.ic16", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106.ic16", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

ROM_START( feag2100a ) // model 6088 - checksum F361 9D5E 1D31 ADF0
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("2100_c_black.ic5",  0xc000, 0x2000, CRC(454eb839) SHA1(83d206464c194b022d43913b5f4092a8201f36b9) )
	ROM_LOAD("2100_c_green.ic4",  0xe000, 0x2000, CRC(f1f76a63) SHA1(337b4572b743d383c6a12c360875d37682de3647) )

	ROM_REGION( 0x8000, "rombank", 0 )
	ROM_LOAD("2100_c_orange.ic9", 0x0000, 0x8000, CRC(feeff71c) SHA1(87614ca850848581d946193efa317181ef9c7a09) )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107.ic16", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(                 0x1000, 0x1000)
	ROMX_LOAD("101-64101.ic16", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105.ic16", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106.ic16", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT  COMPAT  MACHINE   INPUT  CLASS        INIT          COMPANY, FULLNAME, FLAGS
SYST( 1983, feas,      0,      0,      eas,      eas,   elite_state, empty_init,   "Fidelity Electronics", "Elite A/S Challenger (original program)", MACHINE_SUPPORTS_SAVE )
SYST( 1983, feasbu,    feas,   0,      eas,      eas,   elite_state, empty_init,   "Fidelity Electronics", "Elite A/S Challenger (Budapest program, model EAS)", MACHINE_SUPPORTS_SAVE )
SYST( 1983, feasbua,   feas,   0,      ewc,      ewc,   elite_state, empty_init,   "Fidelity Electronics", "Elite A/S Challenger (Budapest program, model EWC)", MACHINE_SUPPORTS_SAVE )
SYST( 1984, feasgla,   feas,   0,      easc,     easc,  elite_state, empty_init,   "Fidelity Electronics", "Elite A/S Challenger (Glasgow program, set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1984, feasglaa,  feas,   0,      easc,     easc,  elite_state, empty_init,   "Fidelity Electronics", "Elite A/S Challenger (Glasgow program, set 2)", MACHINE_SUPPORTS_SAVE )
SYST( 1984, feasglab,  feas,   0,      easc,     easc,  elite_state, empty_init,   "Fidelity Electronics", "Elite A/S Challenger (Glasgow program, set 3)", MACHINE_SUPPORTS_SAVE )
SYST( 1985, feasx,     feas,   0,      easx,     easx,  elite_state, empty_init,   "Fidelity Computer Products", "Elite A/S Challenger (experimental)", MACHINE_SUPPORTS_SAVE )

SYST( 1982, fpres,     0,      0,      pc,       pc,    elite_state, empty_init,   "Fidelity Electronics", "Prestige Challenger (original program, set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1982, fpresa,    fpres,  0,      pc,       pc,    elite_state, empty_init,   "Fidelity Electronics", "Prestige Challenger (original program, set 2)", MACHINE_SUPPORTS_SAVE )
SYST( 1982, fpresb,    fpres,  0,      pc,       pc,    elite_state, empty_init,   "Fidelity Electronics", "Prestige Challenger (original program, set 3)", MACHINE_SUPPORTS_SAVE )
SYST( 1982, fpresc,    fpres,  0,      pc,       pc,    elite_state, empty_init,   "Fidelity Electronics", "Prestige Challenger (original program, set 4)", MACHINE_SUPPORTS_SAVE )
SYST( 1982, fpresd,    fpres,  0,      pc,       pc,    elite_state, empty_init,   "Fidelity Electronics", "Prestige Challenger (original program, set 5)", MACHINE_SUPPORTS_SAVE )
SYST( 1982, fprese,    fpres,  0,      pc,       pc,    elite_state, empty_init,   "Fidelity Electronics", "Prestige Challenger (original program, set 6)", MACHINE_SUPPORTS_SAVE )
SYST( 1983, fpresbu,   fpres,  0,      pc,       pc,    elite_state, empty_init,   "Fidelity Electronics", "Prestige Challenger (Budapest program)", MACHINE_SUPPORTS_SAVE )
SYST( 1984, fpresgla,  fpres,  0,      pc,       pc,    elite_state, empty_init,   "Fidelity Electronics", "Prestige Challenger (Glasgow program)", MACHINE_SUPPORTS_SAVE )

SYST( 1986, feag,      0,      0,      eag,      eag,   eag_state,   empty_init,   "Fidelity International", "Elite Avant Garde (model 6081)", MACHINE_SUPPORTS_SAVE )
SYST( 1986, feag2100,  feag,   0,      eag2100,  eag,   eag_state,   init_eag2100, "Fidelity International", "Elite Avant Garde 2100 (set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1986, feag2100a, feag,   0,      eag2100,  eag,   eag_state,   init_eag2100, "Fidelity International", "Elite Avant Garde 2100 (set 2)", MACHINE_SUPPORTS_SAVE )
