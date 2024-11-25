// license:BSD-3-Clause
// copyright-holders:Kevin Horton, Jonathan Gevaryahu, Sandro Ronco, hap
/*******************************************************************************

Fidelity Voice Sensory Chess Challenger (VSC)
---------------------------------------------
RE notes by Kevin Horton

The display/button/LED/speech technology is identical to Fidelity CSC.
Only the CPU board was changed. As such, it works the same but is interfaced
to different port chips this time.

Hardware:
---------
On the board are 13 chips.

The CPU is a Z80A running at 3.9MHz, with 20K of ROM and 1K of RAM mapped.
I/O is composed of an 8255 triple port adaptor, and a Z80A PIO parallel I/O
interface.

There's the usual TSI S14001A speech synth with its requisite 4K ROM which is the
same as on the other talking chess boards. The TSI chip is running at 26.37KHz.
It uses a 470K resistor and a 100pf capacitor.

The "perfect" clock would be 1/RC most likely (actually this will be skewed a tad by
duty cycle of the oscillator) which with those parts values gives 21.27KHz. The
formula is probably more likely to be 1/1.2RC or so.

Rounding out the hardware are three driver chips for the LEDs, a 7404 inverter to
run the crystal osc, a 555 timer to generate a clock, and a 74138 selector.

NMI runs to a 555 oscillator that generates a 600Hz clock (measured: 598.9Hz.
It has a multiturn pot to adjust).
INT is pulled to 5V
RST connects to a power-on reset circuit

Memory map:
-----------
0000-1FFF: 8K ROM 101-64018 or 101-64015
2000-3FFF: 8K ROM 101-64019 or 101-64016 (101-64019 is also used on the CSC)
4000-5FFF: 4K ROM 101-32024 or 101-32019
6000-7FFF: 1K of RAM (2114 * 2)
8000-FFFF: not used, maps to open bus

Port map:
---------
There's only two chips in the portmap, an 8255 triple port chip, and a Z80A PIO
parallel input/output device.

Decoding isn't performed using a selector, but instead address lines are used.

A2 connects to /CE on the 8255
A3 connects to /CE on the Z80A PIO

A0 connects to port A/B select on PIO & A0 of 8255
A1 connects to control/data select on PIO & A1 of 8255

So to enable only the 8255, you'd write/read to 08-0Bh for example
To enable only the PIO, you'd write/read to 04-07h for example.

writing to 00-03h will enable and write to BOTH chips, and reading 00-03h
will return data from BOTH chips (and cause a bus conflict). The code probably
never does either of these things.

Likewise, writing/reading to 0Ch-0Fh will result in open bus, because neither chip's
enable line will be low.

This sequence repeats every 16 addresses. So to recap:

00-03: both chips enabled (probably not used)
04-07: PIO enabled
08-0B: 8255 enabled
0C-0F: neither enabled

10-FF: mirrors of 00-0F.

Refer to the Sensory Champ. Chess Chall. for explanations of the below
I/O names and labels. It's the same.

8255:
-----
PA.0 - segment D, TSI A0
PA.1 - segment E, TSI A1
PA.2 - segment F, TSI A2
PA.3 - segment A, TSI A3
PA.4 - segment B, TSI A4
PA.5 - segment C, TSI A5
PA.6 - segment G
PA.7 - segment H

PB.0 - LED row 1
PB.1 - LED row 2
PB.2 - LED row 3
PB.3 - LED row 4
PB.4 - LED row 5
PB.5 - LED row 6
PB.6 - LED row 7
PB.7 - LED row 8

PC.0 - LED column A, button column A, 7seg digit 1
PC.1 - LED column B, button column B, 7seg digit 2
PC.2 - LED column C, button column C, 7seg digit 3
PC.3 - LED column D, button column D, 7seg digit 4
PC.4 - LED column E, button column E
PC.5 - LED column F, button column F
PC.6 - LED column G, button column G
PC.7 - LED column H, button column H

Z80A PIO:
---------
PA.0 - button row 1
PA.1 - button row 2
PA.2 - button row 3
PA.3 - button row 4
PA.4 - button row 5
PA.5 - button row 6
PA.6 - button row 7
PA.7 - button row 8

PB.0 - button column I
PB.1 - button column J
PB.2 - hi/lo TSI speaker volume
PB.3 - violet wire to printer port?
PB.4 - white wire to printer port? (and TSI BUSY line)
PB.5 - selection jumper input (see below)
PB.6 - TSI START line
PB.7 - TSI ROM A12 line

Selection jumpers:
------------------
These act like another row of buttons. It is composed of two diode locations,
so there's up to 4 possible configurations. My board does not have either diode
stuffed, so this most likely is "English". I suspect it selects which language
to use for the speech synth. Of course you need the other speech ROMs for this
to function properly.

Anyways, the two jumpers are connected to button columns A and B and the common
connects to Z80A PIO PB.5, which basically makes a 10th button row. I would
expect that the software reads these once on startup only.

Printer:
--------
This is the 1st Fidelity chess computer with a printer port. Many later Fidelity
chess computers also have support for it. Two models were released:
FP: Challenger Printer - thermal printer, MCU=D8048C243
IFP: Impact Printer - also compatible with C64 apparently.

*******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/clock.h"
#include "machine/sensorboard.h"
#include "machine/z80pio.h"
#include "sound/s14001a.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "fidel_vsc.lh"


namespace {

class vsc_state : public driver_device
{
public:
	vsc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_z80pio(*this, "z80pio"),
		m_ppi8255(*this, "ppi8255"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_speech(*this, "speech"),
		m_language(*this, "language"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine configs
	void vsc(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_z80pio;
	required_device<i8255_device> m_ppi8255;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<s14001a_device> m_speech;
	required_region_ptr<u8> m_language;
	required_ioport_array<2> m_inputs;

	u8 m_led_data = 0;
	u8 m_7seg_data = 0;
	u8 m_cb_mux = 0;
	u8 m_kp_mux = 0;

	// address maps
	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;
	u8 main_io_trampoline_r(offs_t offset);
	void main_io_trampoline_w(offs_t offset, u8 data);

	// I/O handlers
	void update_display();
	void ppi_porta_w(u8 data);
	void ppi_portb_w(u8 data);
	void ppi_portc_w(u8 data);
	u8 pio_porta_r();
	u8 pio_portb_r();
	void pio_portb_w(u8 data);
};

void vsc_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_led_data));
	save_item(NAME(m_7seg_data));
	save_item(NAME(m_cb_mux));
	save_item(NAME(m_kp_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// I8255 PPI

void vsc_state::update_display()
{
	// 4 7seg leds+H, 8*8 chessboard leds
	m_display->matrix(m_cb_mux, m_led_data << 8 | m_7seg_data);
}

void vsc_state::ppi_porta_w(u8 data)
{
	// d0-d5: S14001A C0-C5
	m_speech->data_w(data & 0x3f);

	// d0-d7: data for the 4 7seg leds, bits are HGCBAFED (H is extra led)
	m_7seg_data = bitswap<8>(data,7,6,2,1,0,5,4,3);
	update_display();
}

void vsc_state::ppi_portb_w(u8 data)
{
	// d0-d7: led row data
	m_led_data = data;
	update_display();
}

void vsc_state::ppi_portc_w(u8 data)
{
	// d0-d3: select digits
	// d0-d7: select leds, chessboard input mux
	m_cb_mux = data;
	update_display();
}


// Z80 PIO

u8 vsc_state::pio_porta_r()
{
	u8 data = 0;

	// d0-d7: multiplexed inputs
	// read chessboard sensors
	for (int i = 0; i < 8; i++)
		if (BIT(m_cb_mux, i))
			data |= m_board->read_file(i);

	// read other buttons
	for (int i = 0; i < 2; i++)
		if (BIT(m_kp_mux, i))
			data |= m_inputs[i]->read();

	// also language jumpers (hardwired with 2 diodes)
	if (m_kp_mux & 0x20)
		data |= *m_language;

	return data;
}

u8 vsc_state::pio_portb_r()
{
	// d4: S14001A busy pin
	return (m_speech->busy_r()) ? 0 : 0x10;
}

void vsc_state::pio_portb_w(u8 data)
{
	// d0,d1: keypad input mux
	// d5: enable language jumpers
	m_kp_mux = data;

	// d7: speech ROM A12
	m_speech->set_rom_bank(BIT(data, 7));

	// d6: S14001A start pin
	m_speech->start_w(BIT(data, 6));

	// d2: lower S14001A volume
	m_speech->set_output_gain(0, (data & 4) ? 0.25 : 1.0);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void vsc_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x4fff).mirror(0x1000).rom();
	map(0x6000, 0x63ff).mirror(0x1c00).ram();
}

// VSC io: A2 is 8255 _CE, A3 is Z80 PIO _CE - in theory, both chips can be accessed simultaneously
u8 vsc_state::main_io_trampoline_r(offs_t offset)
{
	u8 data = 0xff; // open bus
	if (~offset & 4)
		data &= m_ppi8255->read(offset & 3);
	if (~offset & 8)
		data &= m_z80pio->read(offset & 3);

	return data;
}

void vsc_state::main_io_trampoline_w(offs_t offset, u8 data)
{
	if (~offset & 4)
		m_ppi8255->write(offset & 3, data);
	if (~offset & 8)
		m_z80pio->write(offset & 3, data);
}

void vsc_state::main_io(address_map &map)
{
	map.global_mask(0x0f);
	map(0x00, 0x0f).rw(FUNC(vsc_state::main_io_trampoline_r), FUNC(vsc_state::main_io_trampoline_w));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( vsc )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Rook")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Knight")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("King")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CL")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_N) PORT_NAME("RE")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("TM")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("RV")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Speaker")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("LV")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("DM")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("ST")
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void vsc_state::vsc(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 3.9_MHz_XTAL); // 3.9MHz resonator
	m_maincpu->set_addrmap(AS_PROGRAM, &vsc_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &vsc_state::main_io);

	auto &nmi_clock(CLOCK(config, "nmi_clock", 600)); // 555 timer, ideal frequency is 600Hz (measurement was 587Hz)
	nmi_clock.set_pulse_width(attotime::from_usec(845)); // active for 0.845ms (approx half)
	nmi_clock.signal_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	I8255(config, m_ppi8255);
	m_ppi8255->out_pa_callback().set(FUNC(vsc_state::ppi_porta_w));
	m_ppi8255->out_pb_callback().set(FUNC(vsc_state::ppi_portb_w));
	m_ppi8255->out_pc_callback().set(FUNC(vsc_state::ppi_portc_w));

	Z80PIO(config, m_z80pio, 3.9_MHz_XTAL);
	m_z80pio->in_pa_callback().set(FUNC(vsc_state::pio_porta_r));
	m_z80pio->in_pb_callback().set(FUNC(vsc_state::pio_portb_r));
	m_z80pio->out_pb_callback().set(FUNC(vsc_state::pio_portb_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(250));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 16);
	m_display->set_segmask(0xf, 0x7f);
	config.set_default_layout(layout_fidel_vsc);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	S14001A(config, m_speech, 25000); // R/C circuit, around 25khz
	m_speech->add_route(ALL_OUTPUTS, "speaker", 0.75);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( vsc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-64018", 0x0000, 0x2000, CRC(c9c98490) SHA1(e6db883df088d60463e75db51433a4b01a3e7626) )
	ROM_LOAD("101-64019", 0x2000, 0x2000, CRC(08a3577c) SHA1(69fe379d21a9d4b57c84c3832d7b3e7431eec341) )
	ROM_LOAD("101-32024", 0x4000, 0x1000, CRC(2a078676) SHA1(db2f0aba7e8ac0f84a17bae7155210cdf0813afb) )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 0, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 3, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY, FULLNAME, FLAGS
SYST( 1980, vsc,  0,      0,      vsc,     vsc,   vsc_state, empty_init, "Fidelity Electronics", "Voice Sensory Chess Challenger", MACHINE_SUPPORTS_SAVE )
