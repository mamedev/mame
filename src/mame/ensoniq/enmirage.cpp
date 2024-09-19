// license:BSD-3-Clause
// copyright-holders:R. Belmont, tim lindner
/***************************************************************************

    drivers/enmirage.c

    Ensoniq Mirage Sampler
    Preliminary driver by R. Belmont
    Fleshed out by tim lindner

    Models:
        DSK-8: Pratt-Reed keyboard (early 1984)
        DSK-8: Fatar keyboard (late 1984)
        DMS-8: Rack mount (1985)
        DSK-1: Unweighted keyboard, stereo output (1986)

    M6809 Map for Mirage:
        0000-7fff: 32k window on 128k of sample RAM
        8000-bfff: main RAM
        c000-dfff: optional expansion RAM
        e100-e101: 6850 UART (for MIDI)
        e200-e2ff: 6522 VIA
        e400-e407: write to both filters
        e408-e40f: filter cut-off frequency
        e410-e417: filter resonance
        e418-e41f: DAC pre-set
        e800-e803: WD1770 FDC
        ec00-ecef: ES5503 "DOC" sound chip
        f000-ffff: boot ROM

    M6809 Interrupts:
        NMI: IRQ from WD1772
        IRQ: wired-ORed: DRQ from WD1772, IRQ from ES5503, IRQ from VIA6522, IRQ from cartridge
        FIRQ: IRQ from 6850 UART

    LED / switch matrix:

            A           B           C             D         E         F         G        DP
    ROW 0:  LOAD UPPER  LOAD LOWER  SAMPLE UPPER  PLAY SEQ  LOAD SEQ  SAVE SEQ  REC SEQ  SAMPLE LOWER
    ROW 1:  3           6           9             5         8         0         2        Enter
    ROW 2:  1           4           7             up arrow  PARAM     dn arrow  VALUE    CANCEL
    L. AN:  SEG A       SEG B       SEG C         SEG D     SEG E     SEG F     SEG G    SEG DP (decimal point)
    R. AN:  SEG A       SEG B       SEG C         SEG D     SEG E     SEG F     SEG G    SEG DP

    Column number in VIA port A bits 0-2 is converted to discrete lines by a 74LS145.
    Port A bit 3 is right anode, bit 4 is left anode
    ROW 0 is read on VIA port A bit 5, ROW 1 in port A bit 6, and ROW 2 in port A bit 7.

    Keyboard models talk to the R6500/11 through the VIA shifter: CA2 is handshake, CB1 is shift clock,
    CB2 is shift data.
    This is unconnected on the rackmount version.

    Unimplemented:
        * Four Pole Low-Pass Voltage Controlled Filter section
        * External sync signal
        * Foot pedal
        * ADC feedback
        * Piano keyboard controller
        * Expansion connector
        * Stereo output

***************************************************************************/


#include "emu.h"
#include "bus/midi/midi.h"
#include "cpu/m6809/m6809.h"
#include "formats/esq8_dsk.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/6522via.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/input_merger.h"
#include "machine/wd_fdc.h"
#include "sound/es5503.h"
#include "speaker.h"
#include "video/pwm.h"

#include "enmirage.lh"

#define LOG_ADC_READ        (1U << 1)
#define LOG_FILTER_WRITE    (1U << 2)
#define VERBOSE (0)
//#define VERBOSE (LOG_ADC_READ)
//#define VERBOSE (LOG_ADC_READ|LOG_FILTER_WRITE)

#include "logmacro.h"

#define LOGADCREAD(...)     LOGMASKED(LOG_ADC_READ, __VA_ARGS__)
#define LOGFILTERWRITE(...) LOGMASKED(LOG_FILTER_WRITE, __VA_ARGS__)


namespace {

#define PITCH_TAG "pitch"
#define MOD_TAG "mod"

class enmirage_state : public driver_device
{
public:
	enmirage_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_sample_ram(*this, "sampleram", 1024 * 128, ENDIANNESS_BIG)
		, m_sample_bank(*this, "samplebank")
		, m_display(*this, "display")
		, m_fdc(*this, "wd1772")
		, m_floppy_connector(*this, "wd1772:0")
		, m_via(*this, "via6522")
		, m_irq_merge(*this, "irqmerge")
		, m_cassette(*this, "cassette")
		, m_acia(*this, "acia6850")
		, m_wheel(*this, {PITCH_TAG, MOD_TAG})
		, m_key(*this, {"pb5", "pb6", "pb7"})
	{
	}

	void mirage(machine_config &config);
	void enmirage_es5503_map(address_map &map) ATTR_COLD;

	void init_mirage();
	DECLARE_INPUT_CHANGED_MEMBER(input_changed);
	static void floppy_formats(format_registration &fr);

protected:
	virtual void machine_start() override ATTR_COLD;
	void coefficients_w(offs_t offset, uint8_t data);

private:
	void update_keypad_matrix();

	uint8_t mirage_via_read_portb();
	void mirage_via_write_porta(uint8_t data);
	void mirage_via_write_portb(uint8_t data);
	uint8_t mirage_adc_read();

	void mirage_map(address_map &map) ATTR_COLD;

	virtual void machine_reset() override ATTR_COLD;

	required_device<mc6809e_device> m_maincpu;
	memory_share_creator<uint8_t> m_sample_ram;
	required_memory_bank m_sample_bank;
	required_device<pwm_display_device> m_display;
	required_device<wd1772_device> m_fdc;
	required_device<floppy_connector> m_floppy_connector;
	required_device<via6522_device> m_via;
	required_device<input_merger_device> m_irq_merge;
	required_device<cassette_image_device> m_cassette;
	required_device<acia6850_device> m_acia;

	required_ioport_array<2> m_wheel;
	required_ioport_array<3> m_key;

	int m_mux_value;
	int m_key_col_select;

	/* temporary audio data -- used to get past startup filter calibration -- remove when filters are implemented */
	const uint8_t m_wave[34] = {0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x69};
	int m_wave_index;
};

void enmirage_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_ESQ8IMG_FORMAT);
}

static void ensoniq_floppies(device_slot_interface &device)
{
	device.option_add("35dd", PANA_JU_363);
}

uint8_t enmirage_state::mirage_adc_read()
{
	uint8_t value = 0;
	switch(m_mux_value & 0x03)
	{
		case 0:
//          value = m_cassette->input(); /* compressed and mixed input: audio in and ES 5503 (TODO) */
			value = m_wave[m_wave_index]; /* fake data to get past filter calibration (remove when filter implemented) */
			LOGADCREAD("%s, 5503 sample: channel: compressed input, data: $%02x\n", machine().describe_context(), value);
			if(++m_wave_index == 34) m_wave_index = 0;
			break;
		case 1:
			value = m_cassette->input(); /* line level and mixed input: audio in and ES 5503 (TODO) */
			LOGADCREAD("%s, 5503 sample: channel: line input, data: $%02x\n", machine().describe_context(), value);
			break;
		case 2:
			value = m_wheel[0]->read(); /* pitch wheel */
			LOGADCREAD("%s, 5503 sample: channel: pitch wheel, data: $%02x\n", machine().describe_context(), value);
			break;
		case 3:
			value = m_wheel[1]->read(); /* mod wheel */
			LOGADCREAD("%s, 5503 sample: channel: mod wheel, data: $%02x\n", machine().describe_context(), value);
			break;
	}

	return value;
}

void enmirage_state::machine_start()
{
	save_item(NAME(m_mux_value));
	save_item(NAME(m_key_col_select));
	save_item(NAME(m_wave_index));
	m_sample_bank->configure_entries(0, 4, m_sample_ram, 0x8000);
}

void enmirage_state::machine_reset()
{
	m_sample_bank->set_entry(0);
	m_mux_value = 0;
	m_wave_index = 0;
}

void enmirage_state::mirage_map(address_map &map)
{
	map(0x0000, 0x7fff).bankrw("samplebank"); // 32k window on 128k of sample RAM
	map(0x8000, 0xbfff).ram(); // main RAM
	map(0xc000, 0xdfff).ram(); // expansion RAM
	map(0xe100, 0xe101).rw("acia6850", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xe200, 0xe2ff).m(m_via, FUNC(via6522_device::map));
	map(0xe400, 0xe41f).w(FUNC(enmirage_state::coefficients_w));
	map(0xe800, 0xe803).rw(m_fdc, FUNC(wd1772_device::read), FUNC(wd1772_device::write));
	map(0xec00, 0xecef).rw("es5503", FUNC(es5503_device::read), FUNC(es5503_device::write));
	map(0xf000, 0xffff).rom().region("osrom", 0);
}

void enmirage_state::coefficients_w(offs_t offset, uint8_t data)
{
	uint8_t channel = offset & 0x07;
	uint8_t filter_input = (offset >> 3) & 0x03;

	LOGFILTERWRITE("%s, filter update: channel: %d, data: $%02x (%s%s%s%s)\n",
				machine().describe_context(),
				channel,
				data,
				(filter_input & 0x01) == 0 ? "VF" : "", /* cut-off frequency */
				(filter_input & 0x03) == 0 ? " and " : "",
				(filter_input & 0x02) == 0 ? "VQ" : "", /* filter resonance */
				(filter_input & 0x03) == 0x03 ? "preload dac" : "");
}

// port A:
//  bits 5/6/7 keypad rows 0/1/2 return
INPUT_CHANGED_MEMBER(enmirage_state::input_changed)
{
	update_keypad_matrix();
}

void enmirage_state::update_keypad_matrix()
{
	uint8_t value;

	value  = ((m_key[0]->read() >> m_key_col_select) & 0x01) << 5;
	value |= ((m_key[1]->read() >> m_key_col_select) & 0x01) << 6;
	value |= ((m_key[2]->read() >> m_key_col_select) & 0x01) << 7;

	m_via->write_pa(value);
}

// port B:
//  bit 6: IN disk load
//  bit 5: IN Q Chip sync

uint8_t enmirage_state::mirage_via_read_portb()
{
	uint8_t value = m_via->read_pb();

	floppy_image_device *floppy = m_floppy_connector ? m_floppy_connector->get_device() : nullptr;
	if (floppy)
	{
		if (floppy->dskchg_r())
			value |= 0x40;
		else
			value &= ~0x40;
	}

	return value;
}

// port A: front panel
// bits 0/1/2: dual purpose (0 to 7 lines, though a 74LS145 decoder):
//      keyboard matrix column select
//      7 segment display driver
//  bits 3/4 = right and left 7 segment display enable
//  bits 5/6/7 = Keyboard matrix row sense from 0 to 2
void enmirage_state::mirage_via_write_porta(uint8_t data)
{
	u8 segdata = data & 7;
	m_display->matrix(((data >> 3) & 3) ^ 3, (1<<segdata));

	uint8_t new_select = (data & 0x07);
	if (m_key_col_select != new_select)
	{
		m_key_col_select = new_select;
		update_keypad_matrix();
	}
}

// port B:
//  bit 7: OUT UART clock
//  bit 4: OUT disk select, motor on, and 6500/11 reset
//  bit 3: OUT sample/play
//  bit 2: OUT mic line/in
//  bit 1: OUT upper/lower bank (64k halves)
//  bit 0: OUT bank 0/bank 1 (32k quarters)

void enmirage_state::mirage_via_write_portb(uint8_t data)
{
	int bank = 0;

	// handle sound RAM bank switching
	bank = data & 0x03;
	m_sample_bank->set_entry(bank);

	// handle floppy motor on
	floppy_image_device *floppy = m_floppy_connector->get_device();
	if (floppy)
		floppy->mon_w(data & 0x10 ? 1 : 0);

	// handle 6500/11 reset (TODO)

	// record audio input mixer position
	m_mux_value = (data >> 2) & 0x03;

	// handle acia clock
	// this bit is set by the internal via timer
	int clock = (data >> 7) & 0x01;
	m_acia->write_txc(clock);
	m_acia->write_rxc(clock);
}

void enmirage_state::enmirage_es5503_map(address_map &map)
{
	map(0x00000, 0x1ffff).ram().share("sampleram");
}

void enmirage_state::mirage(machine_config &config)
{
	MC6809E(config, m_maincpu, 2000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &enmirage_state::mirage_map);

	INPUT_MERGER_ANY_HIGH(config, m_irq_merge).output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);
	// <0> via6522
	// <1> wd1772
	// <2> es5502
	// <3> cartridge connector (TODO)

	SPEAKER(config, "speaker").front_center();

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "speaker", 1.0);

	es5503_device &es5503(ES5503(config, "es5503", 8000000));
	es5503.set_channels(8);
	es5503.set_addrmap(0, &enmirage_state::enmirage_es5503_map);
	es5503.irq_func().set(m_irq_merge, FUNC(input_merger_device::in_w<2>));
	es5503.adc_func().set(FUNC(enmirage_state::mirage_adc_read));
	es5503.add_route(ALL_OUTPUTS, "speaker", 1.0);

	MOS6522(config, m_via, 3000000);
	m_via->writepa_handler().set(FUNC(enmirage_state::mirage_via_write_porta));
	m_via->readpb_handler().set(FUNC(enmirage_state::mirage_via_read_portb));
	m_via->writepb_handler().set(FUNC(enmirage_state::mirage_via_write_portb));
	m_via->irq_handler().set(m_irq_merge, FUNC(input_merger_device::in_w<0>));

	PWM_DISPLAY(config, m_display).set_size(2, 8);
	config.set_default_layout(layout_enmirage);

	ACIA6850(config, m_acia).txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	m_acia->irq_handler().set_inputline(m_maincpu, M6809_FIRQ_LINE);
	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	WD1772(config, m_fdc, 8000000);
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_fdc->drq_wr_callback().set(m_irq_merge, FUNC(input_merger_device::in_w<1>));

	FLOPPY_CONNECTOR(config, "wd1772:0", ensoniq_floppies, "35dd", enmirage_state::floppy_formats).enable_sound(true);

	// This clock allows the CPU to keep in sync with the sound chip - may not be used in the firmware
	clock_device &es5503_ca3_clock(CLOCK(config, "ca3_clock", XTAL(8'000'000) / 16));
	es5503_ca3_clock.signal_handler().set(m_via, FUNC(via6522_device::write_pb5));
}

static INPUT_PORTS_START(mirage)
	PORT_START("pb5") /* KEY ROW 0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Load Upper")         PORT_CODE(KEYCODE_A) PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Load Lower")         PORT_CODE(KEYCODE_B) PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Sample Upper")       PORT_CODE(KEYCODE_C) PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Play Sequence")      PORT_CODE(KEYCODE_D) PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Load Sequence")      PORT_CODE(KEYCODE_E) PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Save Sequence")      PORT_CODE(KEYCODE_F) PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Record Sequence")    PORT_CODE(KEYCODE_G) PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Sample Lower")       PORT_CODE(KEYCODE_H) PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_START("pb6") /* KEY ROW 1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3")      PORT_CODE(KEYCODE_3)        PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6")      PORT_CODE(KEYCODE_6)        PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9")      PORT_CODE(KEYCODE_9)        PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5")      PORT_CODE(KEYCODE_5)        PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8")      PORT_CODE(KEYCODE_8)        PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0/Prog") PORT_CODE(KEYCODE_0)        PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2")      PORT_CODE(KEYCODE_2)        PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter")  PORT_CODE(KEYCODE_ENTER)    PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_START("pb7") /* KEY ROW 2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1")          PORT_CODE(KEYCODE_1)    PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")          PORT_CODE(KEYCODE_4)    PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7")          PORT_CODE(KEYCODE_7)    PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("On/Up")      PORT_CODE(KEYCODE_UP)   PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Param")      PORT_CODE(KEYCODE_I)    PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Off/Down")   PORT_CODE(KEYCODE_DOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Value")      PORT_CODE(KEYCODE_J)    PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cancel")     PORT_CODE(KEYCODE_K)    PORT_CHANGED_MEMBER(DEVICE_SELF, enmirage_state, input_changed, 0)

	PORT_START(PITCH_TAG)
	PORT_BIT(0xff, 0x7f, IPT_PADDLE) PORT_NAME("Pitch Wheel") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_MINMAX(0x00,0xff) PORT_CODE_INC(KEYCODE_4_PAD) PORT_CODE_DEC(KEYCODE_1_PAD) PORT_PLAYER(1)
	PORT_START(MOD_TAG)
	PORT_BIT(0xff, 0x7f, IPT_PADDLE) PORT_NAME("Mod Wheel") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_MINMAX(0x00,0xff) PORT_CODE_INC(KEYCODE_6_PAD) PORT_CODE_DEC(KEYCODE_3_PAD) PORT_PLAYER(1)
INPUT_PORTS_END

ROM_START(enmirage)
	ROM_REGION(0x1000, "osrom", 0)
	ROM_LOAD("mirage.bin", 0x0000, 0x1000, CRC(9fc7553c) SHA1(ec6ea5613eeafd21d8f3a7431a35a6ff16eed56d))

	ROM_REGION(0x20000, "es5503", ROMREGION_ERASE)
ROM_END

void enmirage_state::init_mirage()
{
	floppy_image_device *floppy = m_floppy_connector ? m_floppy_connector->get_device() : nullptr;
	if (floppy)
	{
		m_fdc->set_floppy(floppy);
	}
}

} // anonymous namespace


CONS(1984, enmirage, 0, 0, mirage, mirage, enmirage_state, init_mirage, "Ensoniq", "Mirage DMS-8", MACHINE_NOT_WORKING)
