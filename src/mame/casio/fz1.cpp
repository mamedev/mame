// license:BSD-3-Clause
// copyright-holders:Devin Acker

/*
    Driver for Casio FZ-1 and FZ-10M/20M samplers

    Custom sound + video hardware consists of:

    - GAA (uPD65081G-012): address generator for sample RAM
    - GAB (uPD65042G-052): timing generator for PCM interrupt and sample/hold signals
    - GAS (uPD65012G-074): bus arbiter & DRAM refresh signal generator
    - GAX (MB653121): demultiplexes sample RAM output to two DACs
    - 4x MB87186 DCF/DCA (two inputs/outputs each)
    - GAL (uPD65012G-046): generates data & strobe signals for LCD controller
    - HD44350 LCD controller + 2x HD44251 segment drivers

    Floppy drive: Panasonic JU-386 @ 360 rpm
    Disk format: 2HD, 80 tracks * 8 sectors * 1024 bytes

    A good deal of hardware and programming info is available courtesy of Rainer Buchty:
    http://www.buchty.net/casio/
*/

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "bus/nscsi/hd.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/nec/v5x.h"
#include "imagedev/floppy.h"
#include "machine/bankdev.h"
#include "machine/mb87030.h"
#include "machine/i8255.h"
#include "machine/msm6200.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "video/hd44352.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "formats/fz1_dsk.h"
#include "formats/hxchfe_dsk.h"


namespace {

class fz1_state : public driver_device
{
public:
	fz1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_ram(*this, "ram")
		, m_ram_bank(*this, "ram_bank")
		, m_io(*this, "io%u", 0u)
		, m_kbd(*this, "kbd")
		, m_lcdc(*this, "lcdc")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:0")
		, m_keys(*this, "SC%u", 0u)
		, m_analog(*this, "AN%u", 0u)
		, m_led(*this, "led%u", 0u)
	{ }

	void fz1(machine_config &config);
	void fz10m(machine_config &config);
	void fz20m(machine_config &config);

	u8 fdc_irq_r() { return m_fdc->get_irq() ? 1 : 0; }
	void fdc_control_w(u8 data);

	void keys_w(u8 val) { m_key_sel = val; }
	u8 keys_r();

	void adc_sel_w(u8 val) { m_adc_sel = val; }
	u8 adc_latch_r();
	u8 adc_r() { return m_adc_value; }

	u8 lcd_ready_r() { return m_lcd_ready; }

	int cont49_r();
	int sync49_r() { return m_sync49; }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void maincpu_map(address_map &map) ATTR_COLD;
	void fz10m_io_map(address_map &map) ATTR_COLD;
	void fz20m_io_map(address_map &map) ATTR_COLD;
	void fz1_io_map(address_map &map) ATTR_COLD;
	void subcpu_map(address_map &map) ATTR_COLD;

	void gal_w(u8 data);
	void led_w(u8 data);

	// main CPU / key MCU comm methods
	u8 subcpu_r();
	void sub_p2_w(u8 data);

	required_device<v50_device> m_maincpu;
	optional_device<i8049_device> m_subcpu;

	required_device<ram_device> m_ram;
	required_device<address_map_bank_device> m_ram_bank;

	required_device_array<i8255_device, 2> m_io;
	optional_device<msm6200_device> m_kbd;
	required_device<hd44352_device> m_lcdc;

	required_device<upd72065_device> m_fdc;
	required_device<floppy_connector> m_floppy;

	required_ioport_array<4> m_keys;
	optional_ioport_array<8> m_analog;

	output_finder<5> m_led;

	u8 m_key_sel;
	u8 m_adc_sel;
	u8 m_adc_value;

	u8 m_sub_p2, m_sync49;

	u8 m_lcd_ready;
	u8 m_lcd_data;
	u8 m_lcd_data_phase;
	u8 m_lcd_nibble;
};

/**************************************************************************/
static INPUT_PORTS_START(fz10m)
	PORT_START("SC0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD)

	PORT_START("SC1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad 8")       PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad 9")       PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad + / Yes") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Keypad - / No")  PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SC2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Cursor Up")    PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Cursor Down")  PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Cursor Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Cursor Left")  PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Enter")        PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Escape")       PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Display")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SC3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Play")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Modify")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Call/Set Menu")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Transpose")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tune")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO1_PB")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_CUSTOM) // GAA ready
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_CUSTOM) // GAB ready
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(fz1_state::lcd_ready_r))
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_CUSTOM) // ADC ready
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(fz1_state::cont49_r))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(fz1_state::sync49_r))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_CUSTOM) // parallel port IRQ
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(fz1_state::fdc_irq_r))

	PORT_START("IO2_PA")
	PORT_BIT(0x03, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_MEMBER(FUNC(fz1_state::keys_w));
	PORT_BIT(0x1c, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_MEMBER(FUNC(fz1_state::adc_sel_w));
	PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_MEMBER(FUNC(fz1_state::fdc_control_w));

	PORT_START("IO2_PB")
	PORT_BIT(0x3f, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_DEVICE_MEMBER("ram_bank", FUNC(address_map_bank_device::set_bank));
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OUTPUT) // sampling gain
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OUTPUT) // input sample rate

	PORT_START("AN0")
	PORT_BIT(0xff, 0x00, IPT_CUSTOM) // audio input peak level

	PORT_START("AN5")
	PORT_BIT(0xff, 0xff, IPT_POSITIONAL_V) PORT_NAME("Master Volume") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(2) PORT_CODE_DEC(JOYCODE_Y_DOWN_SWITCH) PORT_CODE_INC(JOYCODE_Y_UP_SWITCH)

	PORT_START("AN6")
	PORT_BIT(0xff, 0x80, IPT_POSITIONAL_V) PORT_NAME("Value") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_CODE_DEC(JOYCODE_Y_DOWN_SWITCH) PORT_CODE_INC(JOYCODE_Y_UP_SWITCH)

INPUT_PORTS_END

static INPUT_PORTS_START(fz1)
	PORT_INCLUDE(fz10m)

	PORT_START("kbd:KI8")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C6")

	PORT_START("kbd:KI9")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("B5")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A#5")

	PORT_START("kbd:KI10")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A5")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G#5")

	PORT_START("kbd:KI11")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G5")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F#5")

	PORT_START("kbd:KI12")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F5")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("E5")

	PORT_START("kbd:KI13")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D#5")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D5")

	PORT_START("kbd:KI14")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C#5")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C5")

	PORT_START("kbd:KI15")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("B4")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A#4")

	PORT_START("kbd:KI16")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A4")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G#4")

	PORT_START("kbd:KI17")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G4")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F#4")

	PORT_START("kbd:KI18")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F4")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("E4")

	PORT_START("kbd:KI19")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D#4")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D4")

	PORT_START("kbd:KI20")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C#4")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C4")

	PORT_START("kbd:KI21")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("B3")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A#3")

	PORT_START("kbd:KI22")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A3")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G#3")

	PORT_START("kbd:KI23")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G3")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F#3")

	PORT_START("kbd:KI24")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F3")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("E3")

	PORT_START("kbd:KI25")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D#3")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D3")

	PORT_START("kbd:KI26")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C#3")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C3")

	PORT_START("kbd:KI27")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("B2")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A#2")

	PORT_START("kbd:KI28")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A2")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G#2")

	PORT_START("kbd:KI29")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G2")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F#2")

	PORT_START("kbd:KI30")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F2")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("E2")

	PORT_START("kbd:KI31")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D#2")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D2")

	PORT_START("kbd:KI32")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C#2")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C2")

	PORT_START("kbd:KI33")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("B1")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A#1")

	PORT_START("kbd:KI34")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A1")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G#1")

	PORT_START("kbd:KI35")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G1")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F#1")

	PORT_START("kbd:KI36")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F1")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("E1")

	PORT_START("kbd:KI37")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D#1")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D1")

	PORT_START("kbd:KI38")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C#1")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C1")

	PORT_START("kbd:VELOCITY")
	PORT_BIT(0x3f, 0x3f, IPT_POSITIONAL_V) PORT_NAME("Key Velocity") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(7) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

	PORT_START("AN1")
	PORT_BIT(0xff, 0xff, IPT_POSITIONAL_V) PORT_NAME("Volume Pedal") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(6) PORT_CODE_DEC(JOYCODE_Y_DOWN_SWITCH) PORT_CODE_INC(JOYCODE_Y_UP_SWITCH)

	PORT_START("AN2")
	PORT_BIT(0xff, 0xff, IPT_POSITIONAL_V) PORT_NAME("Aftertouch") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(5) PORT_CODE_DEC(JOYCODE_Y_DOWN_SWITCH) PORT_CODE_INC(JOYCODE_Y_UP_SWITCH)

	PORT_START("AN3")
	PORT_BIT(0xff, 0x00, IPT_POSITIONAL_V) PORT_NAME("Modulation Wheel") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(4) PORT_CODE_DEC(JOYCODE_Y_DOWN_SWITCH) PORT_CODE_INC(JOYCODE_Y_UP_SWITCH)

	PORT_START("AN4")
	PORT_BIT(0xff, 0x7f, IPT_PADDLE)       PORT_NAME("Pitch Wheel") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_MINMAX(0x00, 0xff) PORT_PLAYER(3) PORT_CODE_DEC(JOYCODE_Y_DOWN_SWITCH) PORT_CODE_INC(JOYCODE_Y_UP_SWITCH)

INPUT_PORTS_END

/**************************************************************************/
void fz1_state::maincpu_map(address_map &map)
{
	map(0x00000, 0x0ffff).ram();
	map(0x10000, 0x1ffff).rw(m_ram_bank, FUNC(address_map_bank_device::read16), FUNC(address_map_bank_device::write16));
	map(0x80000, 0x8ffff).mirror(0x70000).rom().region("maincpu", 0);
}

/**************************************************************************/
void fz1_state::fz10m_io_map(address_map &map)
{
	// 0x00-07: GAA
	// 0x08-0f: GAB
	map(0x10, 0x13).mirror(0x04).m(m_fdc, FUNC(upd72065_device::map)).umask16(0x00ff);
	map(0x18, 0x1f).rw(m_io[0], FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x20, 0x27).rw(m_io[1], FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x60, 0x67).r(FUNC(fz1_state::adc_latch_r));
	map(0x68, 0x6f).r(FUNC(fz1_state::adc_r)).umask16(0x00ff);
	map(0x70, 0x77).w(FUNC(fz1_state::gal_w)).umask16(0x00ff);
	map(0x78, 0x7f).w(FUNC(fz1_state::led_w)).umask16(0x00ff);
	// 0x80-bf: DCF/DCA
}

/**************************************************************************/
void fz1_state::fz1_io_map(address_map &map)
{
	fz10m_io_map(map);
	map(0x28, 0x2f).r(FUNC(fz1_state::subcpu_r)).umask16(0x00ff);
}

/**************************************************************************/
void fz1_state::fz20m_io_map(address_map &map)
{
	fz10m_io_map(map);
	map(0x30, 0x3f).m("scsi:7:spc", FUNC(mb89352_device::map));
}

/**************************************************************************/
void fz1_state::subcpu_map(address_map &map)
{
	map(0x00, 0xff).rw(m_kbd, FUNC(msm6200_device::read), FUNC(msm6200_device::write));
}

/**************************************************************************/
static void fz1_floppies(device_slot_interface &device)
{
	device.option_add("35hd", PANA_JU_386);
}

/**************************************************************************/
static void floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_FZ1_FORMAT);
	fr.add(FLOPPY_HFE_FORMAT);
}

/**************************************************************************/
void fz1_state::fz10m(machine_config &config)
{
	V50(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &fz1_state::maincpu_map);
	m_maincpu->set_addrmap(AS_IO, &fz1_state::fz10m_io_map);
	m_maincpu->set_tclk(2_MHz_XTAL);
	m_maincpu->tout2_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ7);

	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set(m_maincpu, FUNC(v50_device::rxd_w));

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");
	m_maincpu->txd_handler_cb().set("mdout", FUNC(midi_port_device::write_txd));

	MIDI_PORT(config, "mdthru", midiout_slot, "midiout");
	mdin.rxd_handler().append("mdthru", FUNC(midi_port_device::write_txd));

	// RAM is fixed on FZ-10M/20M but expandable on FZ-1
	RAM(config, m_ram).set_default_size("2M");
	ADDRESS_MAP_BANK(config, m_ram_bank).set_options(ENDIANNESS_LITTLE, 16, 21, 0x10000);

	I8255(config, m_io[0]);
	// port A: parallel port data bus
	m_io[0]->in_pb_callback().set_ioport("IO1_PB");
	// port C: parallel port control

	I8255(config, m_io[1]);
	m_io[1]->out_pa_callback().set_ioport("IO2_PA");
	m_io[1]->out_pb_callback().set_ioport("IO2_PB");
	m_io[1]->in_pc_callback().set(FUNC(fz1_state::keys_r));

	UPD72065(config, m_fdc, 16_MHz_XTAL / 4);
	m_fdc->set_select_lines_connected(false);
	// WP/TS pin is only used for write protect signal; firmware requires TS low even for DSHD disks
	m_fdc->set_ts_line_connected(false);

	FLOPPY_CONNECTOR(config, m_floppy, fz1_floppies, "35hd", floppy_formats); // leave sound off because drive motor is almost always running

	HD44352(config, m_lcdc, 2_MHz_XTAL); // actually HD44350 + 2x HD44251

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(m_lcdc, FUNC(hd44352_device::screen_update));
	screen.set_size(96, 64);
	screen.set_visarea(0, 96-1, 0, 64-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);
}

/**************************************************************************/
void fz1_state::fz1(machine_config &config)
{
	fz10m(config);
	m_maincpu->set_addrmap(AS_IO, &fz1_state::fz1_io_map);

	// expansion slot on a stock unit only allows 2MB, but hardware and firmware support up to 4MB
	m_ram->set_extra_options("1M,3M,4M");
	m_ram_bank->set_addr_width(22);

	I8049(config, m_subcpu, 8.96_MHz_XTAL);
	m_subcpu->set_addrmap(AS_IO, &fz1_state::subcpu_map);
	m_subcpu->p2_out_cb().set(FUNC(fz1_state::sub_p2_w));
	m_subcpu->t0_in_cb().set(FUNC(fz1_state::sync49_r));

	MSM6200(config, m_kbd, 2.47_MHz_XTAL);
	m_kbd->irq_cb().set_inputline(m_subcpu, MCS48_INPUT_IRQ);
}

/**************************************************************************/
static void scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add_internal("mb89352", MB89352);
}

/**************************************************************************/
void fz1_state::fz20m(machine_config &config)
{
	fz10m(config);
	m_maincpu->set_addrmap(AS_IO, &fz1_state::fz20m_io_map);
	m_maincpu->in_ior_cb<1>().set("scsi:7:spc", FUNC(mb89352_device::dma_r));
	m_maincpu->out_iow_cb<1>().set("scsi:7:spc", FUNC(mb89352_device::dma_w));

	// note: loading from HD requires running "HDD Operater" [sic] from FL-D1 program disk
	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("spc", MB89352).machine_config(
		[this](device_t *device)
		{
			mb89352_device &spc = downcast<mb89352_device &>(*device);

			spc.set_clock(8_MHz_XTAL);
			spc.out_irq_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ3);
			spc.out_dreq_callback().set(m_maincpu, FUNC(v50_device::dreq_w<1>));
		});
}

/**************************************************************************/
void fz1_state::machine_start()
{
	m_led.resolve();

	m_key_sel = 0;
	m_adc_sel = 0;
	m_adc_value = 0;
	m_sub_p2 = 0;

	m_ram_bank->space().install_ram(0, m_ram->mask(), m_ram->pointer());

	m_fdc->set_rate(500000);
	m_fdc->set_floppy(m_floppy->get_device());

	save_item(NAME(m_key_sel));
	save_item(NAME(m_adc_sel));
	save_item(NAME(m_adc_value));
	save_item(NAME(m_sub_p2));

	save_item(NAME(m_lcd_ready));
	save_item(NAME(m_lcd_nibble));
	save_item(NAME(m_lcd_data));
	save_item(NAME(m_lcd_data_phase));
}

/**************************************************************************/
void fz1_state::machine_reset()
{
	m_lcd_ready = 1;
	m_lcd_nibble = 0;
	m_lcd_data = 0;
	m_lcd_data_phase = 0;
}

/**************************************************************************/
void fz1_state::fdc_control_w(u8 data)
{
	m_fdc->tc_w(BIT(data, 0));

	auto *dev = m_floppy->get_device();
	if (dev)
	{
		dev->mon_w(BIT(~data, 1));
		dev->inuse_w(BIT(data, 2));
	}
}

/**************************************************************************/
u8 fz1_state::keys_r()
{
	return m_keys[m_key_sel & 3]->read();
}

/**************************************************************************/
u8 fz1_state::adc_latch_r()
{
	if (!machine().side_effects_disabled())
		m_adc_value = m_analog[m_adc_sel & 7].read_safe(0);
	return 0;
}

/**************************************************************************/
u8 fz1_state::subcpu_r()
{
	if (!machine().side_effects_disabled())
	{
		m_sync49 = 0;
		m_maincpu->set_input_line(INPUT_LINE_IRQ6, CLEAR_LINE);
	}

	return m_subcpu->p1_r();
}

/**************************************************************************/
void fz1_state::sub_p2_w(u8 data)
{
	if (BIT(~data & m_sub_p2, 6))
		m_maincpu->set_input_line(INPUT_LINE_IRQ6, ASSERT_LINE);

	if (BIT(~data & m_sub_p2, 7))
		m_sync49 = 1;

	m_sub_p2 = data;
}

/**************************************************************************/
int fz1_state::cont49_r()
{
	return BIT(m_sub_p2, 5);
}

/**************************************************************************/
void fz1_state::gal_w(u8 data)
{
	m_lcd_ready = BIT(data, 7);

	switch (data & 0x7f)
	{
	case 0x1f: // command start
		m_lcdc->control_write(0x87);
		m_lcd_data_phase = 0;
		break;

	case 0x10: // data start
		m_lcdc->control_write(0x86);
		m_lcd_data_phase = 1;
		break;

	default:
		if (!m_lcd_ready)
		{
			if (!m_lcd_nibble)
			{
				if (!m_lcd_data_phase)
					m_lcd_data = (data & 0xf);
				else
					m_lcd_data = (data << 4);
			}
			else
			{
				if (!m_lcd_data_phase)
					m_lcd_data |= (data << 4);
				else
					m_lcd_data |= (data & 0xf);

				m_lcdc->data_write(m_lcd_data);
			}

			m_lcd_nibble ^= 1;
		}
	}
}

/**************************************************************************/
void fz1_state::led_w(u8 data)
{
	for (int i = 0; i < m_led.size(); i++)
		m_led[i] = BIT(~data, i);
}

/**************************************************************************/
ROM_START( fz1 )
	ROM_REGION(0x10000, "maincpu", 0) // "ROM ver.[B]"
	ROM_LOAD16_BYTE( "fz1s.bin", 0x00000, 0x08000, CRC(b0ba313d) SHA1(45a3660d708f0a584f9f61d04e205a96688f0950) )
	ROM_LOAD16_BYTE( "fz2s.bin", 0x00001, 0x08000, CRC(57098176) SHA1(ceeb4de4a3df35430497bee37ec73f1b12042729) )

	ROM_REGION(0x800, "subcpu", 0) // this dump is actually uPD80C49HC-187 from the HT-6000, though it appears functionally identical
	ROM_LOAD("upd8049hc-672.bin", 0x000, 0x800, BAD_DUMP CRC(47b47af7) SHA1(8f0515f95dcc6e224a8a59e0c2cd7ddb4796e34e))

	ROM_REGION( 0x800, "lcdc", 0 ) // taken from pb1000, may not be completely identical
	ROM_LOAD( "charset.bin", 0x000, 0x800, BAD_DUMP CRC(7f144716) SHA1(a02f1ecc6dc0ac55b94f00931d8f5cb6b9ffb7b4))
ROM_END

ROM_START( fz10m )
	ROM_REGION(0x10000, "maincpu", 0) // "ROM ver.[B]"
	ROM_LOAD16_BYTE( "mz1b.bin", 0x00000, 0x08000, CRC(3e16943a) SHA1(d754b60901f848b43ee256c2d3f1dfeee031d943) )
	ROM_LOAD16_BYTE( "mz2b.bin", 0x00001, 0x08000, CRC(1db8400d) SHA1(01edf16839afdd78820d7a0f8a410882c54889a9) )

	ROM_REGION( 0x800, "lcdc", 0 ) // taken from pb1000, may not be completely identical
	ROM_LOAD( "charset.bin", 0x000, 0x800, BAD_DUMP CRC(7f144716) SHA1(a02f1ecc6dc0ac55b94f00931d8f5cb6b9ffb7b4))
ROM_END

ROM_START( fz20m )
	ROM_REGION(0x10000, "maincpu", 0) // "ROM hdd.[C]"
	ROM_LOAD16_BYTE( "fz20m_l.bin", 0x00000, 0x08000, CRC(e16c9ccf) SHA1(aa2c5fc0465cb6c9b0c7a7ab31606ad1703327ee) )
	ROM_LOAD16_BYTE( "fz20m_h.bin", 0x00001, 0x08000, CRC(be272cad) SHA1(09d5c901e2f8905415789595a69e7d4166c0e2d2) )

	ROM_REGION( 0x800, "lcdc", 0 ) // taken from pb1000, may not be completely identical
	ROM_LOAD( "charset.bin", 0x000, 0x800, BAD_DUMP CRC(7f144716) SHA1(a02f1ecc6dc0ac55b94f00931d8f5cb6b9ffb7b4))
ROM_END

} // anonymous namespace

SYST( 1987, fz1,   0,      0, fz1,   fz1,   fz1_state, empty_init, "Casio", "FZ-1 Digital Sampling Synthesizer",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
SYST( 1987, fz10m, fz1,    0, fz10m, fz10m, fz1_state, empty_init, "Casio", "FZ-10M Digital Sampling Synthesizer Module", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
SYST( 1989, fz20m, fz1,    0, fz20m, fz10m, fz1_state, empty_init, "Casio", "FZ-20M Digital Sampling Synthesizer Module", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
