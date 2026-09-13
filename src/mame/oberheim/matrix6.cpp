// license:BSD-3-Clause
// copyright-holders:m1macrophage

/*
The Matrix-6 is a 61-key, polyphonic, digitally-controlled analog synthesizer.

It features 6 voices built around CEM3396 chips. Each of those consist of 2 DCOs,
a mixer, a 4-pole lowpass VCF and 2 VCAs. The parameters of each voice can be
controlled independently, for a multitimbrality of 6.

Editing is done via a membrane button interface, with no knobs. The synth
supports MIDI and a cassette interface.

The Matrix-6 has two additional variants:
* Matrix-6R: a 3U rackmount version lacking a keyboard and pitch/mod wheels, but
  exposing the same editing interface. It also has a "REMOTE" connector, though
  a remote was apparently never made available.
* Matrix-1000: a 1U rackmount version with 1000 patches and no editing UI.
  Patches can still be edited via MIDI.

Note that the Matrix-12 does not belong to the Matrix-6 family, despite its name.
It is based on the Xpander architecture.

The Matrix-6 and -6R have 3 PCBs:
* Processor board.
* Display board.
* Voice board.

This driver is based on the Matrix-6 schematics, and is intended as an
educational tool.

Comments contain signal names in all caps (e.g. DAC*, ROM0*, DDATA, etc.). Those
are the signal names used in the schematic. The * suffix (in the schematic and
in the comments) means the signal is active-low.
*/

#include "emu.h"

#include "bus/midi/midi.h"
#include "cpu/m6809/m6809.h"
#include "machine/6522via.h"
#include "machine/6850acia.h"
#include "machine/nvram.h"
#include "machine/output_latch.h"
#include "machine/pit8253.h"
#include "video/roc10937.h"

#define LOG_BANKING (1U << 1)

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

namespace {

// Matrix-6R
class matrix6r_state :  public driver_device
{
public:
	static constexpr feature_type unemulated_features() { return feature::TAPE; }

	matrix6r_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD;

	void matrix6r(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(memory_protect_changed) { update_banking(); }

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	u8 kbd0_r();
	u8 kbd1_r();
	int footsw_r() const;

	void dac_w(offs_t offset, u8 data);
	void sound_control_latch_w(offs_t offset, u8 data);

	void update_banking();

	void memory_map(address_map &map) ATTR_COLD;

	required_device<mc6809_device> m_maincpu;
	required_device<via6522_device> m_via;
	memory_view m_ram1_view;
	memory_view m_mem2_view;
	required_ioport m_memory_protect;
	required_ioport m_pedal2;

	bool m_footsw_en;
};

// Matrix-6
// The Matrix-6R contains a subset of the Matrix6 hardware, so using the -6R as
// the base class.
// TODO: The extra Matrix-6 hardware is not emulated, so matrix6_state is just a
// "thin" subclass.
class matrix6_state :  public matrix6r_state
{
public:
	matrix6_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD
		: matrix6r_state(mconfig, type, tag) { }

	void matrix6(machine_config &config) ATTR_COLD { matrix6r(config); }
};

matrix6r_state::matrix6r_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	, m_maincpu(*this, "maincpu")
	, m_via(*this, "via")
	, m_ram1_view(*this, "ram1_view")
	, m_mem2_view(*this, "mem2_view")
	, m_memory_protect(*this, "memory_protect")
	, m_pedal2(*this, "pedal2")
	, m_footsw_en(false)
{
}

u8 matrix6r_state::kbd0_r()
{
	// The Matrix-6R doesn't have a keyboard, but the port is sometimes read.
	return 0xff;
}

u8 matrix6r_state::kbd1_r()
{
	// The Matrix-6R doesn't have a keyboard, but the port is sometimes read.
	return 0xff;
}

int matrix6r_state::footsw_r() const
{
	const bool footsw_in = m_footsw_en ? BIT(m_pedal2->read(), 0) : false;
	return footsw_in ? 0 : 1;  // Inverted by Q6 (display board).
}

void matrix6r_state::dac_w(offs_t offset, u8 data)
{
	// TODO: implement.
}

void matrix6r_state::sound_control_latch_w(offs_t offset, u8 data)
{
	// TODO: implement.
}

void matrix6r_state::update_banking()
{
	const bool mem_protect = BIT(m_memory_protect->read(), 0);
	m_ram1_view.select(mem_protect ? 1 : 0);

	if (BIT(m_via->read_pa(), 3))  // BSEL
		m_mem2_view.select(2);  // ROM0
	else
		m_mem2_view.select(mem_protect ? 1 : 0);  // RAM2

	LOGMASKED(LOG_BANKING, "Ram1: %d, Mem2: %d\n", *m_ram1_view.entry(), *m_mem2_view.entry());
}

void matrix6r_state::machine_start()
{
	save_item(NAME(m_footsw_en));
}

void matrix6r_state::machine_reset()
{
	update_banking();
}

void matrix6r_state::memory_map(address_map &map)
{
	// The signal names below (e.g. TIMER1*, DAC*) refer to those in the schematics.
	// All component designations are for the processor board, unless otherwise noted.

	map.unmap_value_high();  // A floating 74LS245 (U15) input will likely resolve to 1.

	// MSB address decoding done by U23 (74LS138)

	// U23-O0: I/O* -> U10 (74LS138)
	// All timers are write-only. /RD is tied high.
	map(0x0000, 0x0003).mirror(0x03fc).w("timer1", FUNC(pit8254_device::write));  // U10-O0: TIMER1*
	map(0x0400, 0x0403).mirror(0x03fc).w("timer2", FUNC(pit8254_device::write));  // U10-O1: TIMER2*
	map(0x0800, 0x0803).mirror(0x03fc).w("timer3", FUNC(pit8254_device::write));  // U10-O2: TIMER3*
	map(0x0c00, 0x0c03).mirror(0x03fc).w("timer4", FUNC(pit8254_device::write));  // U10-O3: TIMER4*

	// U10-O4: CLKDIO* -> U717 (74HC42, voice board)
	map(0x1000, 0x102f).mirror(0x03c0).w(FUNC(matrix6r_state::sound_control_latch_w));  // U717-O[0-5]: L1*-L6*
	map(0x1030, 0x1037).mirror(0x03c0);  // U717-O6: VSPARE. TODO: find out what this is.

	// U717-O7: LED* -> U2 (74LS138, display board)
	map(0x1038, 0x1038).mirror(0x03c0).w("mode_led_latch", FUNC(output_latch_device::write));  // U2-O0
	map(0x1039, 0x1039).mirror(0x03c0).w("column_led_latch", FUNC(output_latch_device::write));  // U2-O1
	map(0x103a, 0x103f).mirror(0x03c0).unmaprw();  // U2-O[2-7] not connected.

	map(0x1406, 0x1407).mirror(0x03f8).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));  // U10-O5: UART*
	map(0x1800, 0x180f).mirror(0x03f0).m("via", FUNC(via6522_device::map));  // U10-O6: VIA*

	// U10-O7: MOREIO* -> U12A (74LS139)
	map(0x1c00, 0x1dff).w(FUNC(matrix6r_state::dac_w));  // U12A-O[0, 1]: DAC*
	map(0x1e00, 0x1e00).mirror(0x00ff).unmaprw();  // U12A-O2 not connected.

	// U12A-O3 -> U12B (74LS139)
	map(0x1f00, 0x1f00).mirror(0x003f).r(FUNC(matrix6r_state::kbd0_r));  // U12B-O0: KBD0*
	map(0x1f40, 0x1f40).mirror(0x003f).r(FUNC(matrix6r_state::kbd1_r));  // U12B-O1: KBD1*

	// U12B-O2: SWITCH* -> U1 (74LS138, display board)
	map(0x1f80, 0x1f80).mirror(0x0038).portr("switch0");  // U1-O0
	map(0x1f81, 0x1f81).mirror(0x0038).portr("switch1");  // U1-O1
	map(0x1f82, 0x1f82).mirror(0x0038).portr("switch2");  // U1-O2
	map(0x1f83, 0x1f87).mirror(0x0038).unmaprw();  // U1-O[3-7] not connected.

	map(0x1fc0, 0x1fc0).mirror(0x003f).unmaprw();  // U12B-O3 not connected.

	map(0x2000, 0x3fff).ram().share("ram0");  // U23-O1: RAM0*

	map(0x4000, 0x5fff).view(m_ram1_view);  // U23-O2: RAM1*
	m_ram1_view[0](0x4000, 0x5fff).ram().share("ram1");
	m_ram1_view[1](0x4000, 0x5fff).readonly().share("ram1");

	// Jumper wires can be used to configure the RAM2 (U6) slot for different
	// RAM types (6116 or 6264). The mapping here is what seems to be the shipped
	// configuration (6264), based on the schematic.
	map(0x6000, 0x7fff).view(m_mem2_view);  // U23-O3: MEM2*
	m_mem2_view[0](0x6000, 0x7fff).ram().share("ram2");  // RAM2*
	m_mem2_view[1](0x6000, 0x7fff).readonly().share("ram2");
	m_mem2_view[2](0x6000, 0x7fff).rom().region("maincpu", 0);  // ROM0*

	// U23-O[4-7]: jumper wires can be used to support different types of ROMs
	// for ROM1 (U4) and ROM2 (U5). Based on the schematic, the shipped jumper
	// configuration leaves these outputs unconnected, and ROM1 (U4) is marked
	// with an "X". Based on PCB photos, U4 is unpopulated and missing a socket.
	// While the Matrix-6 includes a ROM0 (U47), the Matrix-6R does not, and it
	// just has an empty socket (based on PCB photos).

	// ROM2 gets enabled when A15=1 in the shipped jumper configuration.
	map(0x8000, 0xffff).rom().region("maincpu", 0x2000);
}

void matrix6r_state::matrix6r(machine_config &config)
{
	// All component designations are for the processor board, unless otherwise noted.

	MC6809(config, m_maincpu, 8_MHz_XTAL);  // U16
	m_maincpu->set_addrmap(AS_PROGRAM, &matrix6r_state::memory_map);

	NVRAM(config, "ram0", nvram_device::DEFAULT_ALL_0);  // U8 (6264)
	NVRAM(config, "ram1", nvram_device::DEFAULT_ALL_0);  // U7 (6264)
	NVRAM(config, "ram2", nvram_device::DEFAULT_ALL_0);  // U6 (6264 or 6116, see memory_map())

	auto &acia = ACIA6850(config, "acia");  // U3
	acia.txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	acia.irq_handler().set_inputline(m_maincpu, M6809_FIRQ_LINE);

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");  // J1
	MIDI_PORT(config, "mdthru", midiout_slot, "midiout");  // J3
	auto &midi_in = MIDI_PORT(config, "mdin", midiin_slot, "midiin");  // J2
	midi_in.rxd_handler().append("acia", FUNC(acia6850_device::write_rxd));
	midi_in.rxd_handler().append("mdthru", FUNC(midi_port_device::write_txd));

	// Clock input connected to the M6809's E signal.
	MOS6522(config, m_via, 8_MHz_XTAL / 4);  // U9

	// PA, ordered from LSBit to MSBit.
	// TODO: PA0 - DOR - input.
	m_via->readpa_handler().append_ioport("memory_protect").mask(1).lshift(1);  // PROTECT
	m_via->readpa_handler().append(FUNC(matrix6r_state::footsw_r)).mask(1).lshift(2);  // FOOTSW
	m_via->writepa_handler().append([this] (int state) { update_banking(); }).bit(3);  // BSEL
	m_via->writepa_handler().append_output("cassout").bit(4);  // CASSOUT
	m_via->writepa_handler().append("vfd", FUNC(roc10937_device::por)).bit(5).invert();  // DCLR*, inverted by Q5 (display board).
	m_via->readpa_handler().append_ioport("pedal1").rshift(1).mask(1).lshift(6);  // RING1
	m_via->readpa_handler().append_ioport("pedal2").rshift(1).mask(1).lshift(7);  // RING2

	// PB, ordered from LSBit to MSBit. TODO: emulate all these.
	// PB0 - KBDCLR* - output.
	// PB1 - Not connected.
	// PB2 - FSL - output.
	// PB3-PB5 - MUX select - output.
	// PB6 - MUX - input.
	// PB7 - LSET -> invert -> LSET* - output.

	m_via->cb1_handler().set("vfd", FUNC(roc10937_device::sclk)).invert();  // Inverted by U11. DCLK*
	m_via->cb2_handler().set("vfd", FUNC(roc10937_device::data));  // DDATA
	m_via->irq_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);  // IRQ*

	ROC10937(config, "vfd");  // U6 (10937) and DISP1 (F6156A2 ?), both on the display board.

	auto &mode_leds = OUTPUT_LATCH(config, "mode_led_latch");  // U4 (74LS174, display board)
	mode_leds.bit_handler<0>().set_output("led5").invert();  // patch select
	mode_leds.bit_handler<1>().set_output("led6").invert();  // patch edit
	mode_leds.bit_handler<2>().set_output("led7").invert();  // matrix mod
	mode_leds.bit_handler<3>().set_output("led8").invert();  // split select
	mode_leds.bit_handler<4>().set_output("led9").invert();  // split edit
	mode_leds.bit_handler<5>().set_output("led10").invert(); // master edit

	auto &col_leds = OUTPUT_LATCH(config, "column_led_latch");  // U5 (74LS174, display board)
	col_leds.bit_handler<0>().set_output("led1").invert();  // column 1
	col_leds.bit_handler<1>().set_output("led2").invert();  // column 2
	col_leds.bit_handler<2>().set_output("led3").invert();  // column 3
	col_leds.bit_handler<3>().set_output("led4").invert();  // column 4
	// Bit 4 not connected.
	col_leds.bit_handler<5>().set([this] (int state) { m_footsw_en = state; });

	// Timers are located on the voice board.
	PIT8254(config, "timer1");  // U730
	PIT8254(config, "timer2");  // U729
	PIT8254(config, "timer3");  // U728
	PIT8254(config, "timer4");  // U727
}

INPUT_PORTS_START(matrix6r)
	PORT_START("switch0")  // TODO: figure out mappings to physical buttons.
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_8)

	PORT_START("switch1")  // TODO: figure out mappings to physical buttons.
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_R)  // "unblocks" the device.
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_I)

	PORT_START("switch2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("COL 1") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("COL 2") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("COL 3") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("COL 4") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("PATCHES") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("SPLITS") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("MASTER") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	// A DPDT switch. When enabled, the PROTECT signal (readable by the firmware)
	// will go high, and the /WR input of RAM1 and RAM2 will be logically
	// disconnected from the CPU /WR signal.
	PORT_START("memory_protect")  // SW1
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Memory Protect") PORT_TOGGLE
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(matrix6r_state::memory_protect_changed), 0)

	// There seems to be a labeling mistake in the schematic. It shows PEDAL1/PJ1
	// connected to the FOOTSW and RING2 signals, and  PEDAL2/PJ2 connected to
	// FOOTP and RING1. FOOTP is treated as an analog input, and FOOTSW as an
	// on/off input. But according to the owners manual, PEDAL1 is the analog
	// one, and PEDAL2 is the on/off one. So going with that here. Doing so also
	// fixes the numbering discrepancy between "PEDAL{num}" and "RING{num}".

	PORT_START("pedal1")  // PJ1
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("FOOTPEDAL")  // tip. TODO: this is an analog input.
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("RING1") PORT_TOGGLE  // ring

	PORT_START("pedal2")  // PJ2
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("FOOTSWITCH")  // tip
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("RING2") PORT_TOGGLE  // ring
INPUT_PORTS_END

INPUT_PORTS_START(matrix6)
	PORT_INCLUDE(matrix6r)
	// TODO: Add keyboard and pitch & mod wheels.
INPUT_PORTS_END

ROM_START(matrix6)
	ROM_REGION(0xa000, "maincpu", 0)
	ROM_FILL(0x000000, 0x002000, 0xff)  // ROM0, U47 (2764). Addressable but not populated.
	// ROM1, U4 (2764). Not addressable nor populated in shipped configuration. See memory_map().
	ROM_LOAD("m6_2.13a.u5", 0x002000, 0x008000, CRC(7134123c) SHA1(f0df11ad7ce52869a1a2d73341b137f110b6e46f))  // ROM2, U5 (27256)
ROM_END

ROM_START(matrix6r)
	ROM_DEFAULT_BIOS("2.13")
	ROM_SYSTEM_BIOS(0, "2.13", "Matrix-6R 2.13")
	ROM_SYSTEM_BIOS(1, "2.11", "Matrix-6R 2.11")

	ROM_REGION(0xa000, "maincpu", 0)
	ROM_FILL(0x000000, 0x002000, 0xff)  // ROM0, U47 (2764). Addressable but not populated.
	// ROM1, U4 (2764). Not addressable nor populated in shipped configuration. See memory_map().
	ROMX_LOAD("m6r_2.13.u5", 0x002000, 0x008000, CRC(77d76aec) SHA1(8cf86af681c6b01a337e8202b57beb1626ba5372), ROM_BIOS(0))  // ROM2, U5 (27256)
	ROMX_LOAD("m6r_2.11.u5", 0x002000, 0x008000, CRC(728d045d) SHA1(34a470fc14586922c545f7b5df03d4a9d3d25b45), ROM_BIOS(1))
ROM_END

}  // anonymous namespace

SYST(1985, matrix6, 0, 0, matrix6, matrix6, matrix6_state, empty_init, "Oberheim", "Matrix-6", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)  // 1985-1988
SYST(1986, matrix6r, 0, 0, matrix6r, matrix6r, matrix6r_state, empty_init, "Oberheim", "Matrix-6R", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)  // 1986-1988
