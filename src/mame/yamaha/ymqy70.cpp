// license:GPL-2.0+
// copyright-holders: BALATON Zoltan

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/h8/h83002.h"
#include "machine/nvram.h"
//#include "sound/swp00.h"
#include "video/t6963c.h"

#include "speaker.h"


namespace {

static INPUT_PORTS_START(qy70)
	PORT_START("SWA")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Record")    PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Top")   PORT_CODE(KEYCODE_C)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Stop")  PORT_CODE(KEYCODE_X)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Backward")  PORT_CODE(KEYCODE_V)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Play")  PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Forward")   PORT_CODE(KEYCODE_B)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Song")  PORT_CODE(KEYCODE_J)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Pattern")   PORT_CODE(KEYCODE_K)

	PORT_START("SWB")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Menu")  PORT_CODE(KEYCODE_M)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F1")    PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F2")    PORT_CODE(KEYCODE_S)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F3")    PORT_CODE(KEYCODE_D)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F4")    PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Exit")  PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SWC")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Left")  PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Up")    PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Down")  PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SWD")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("OctDown")   PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("OctUp") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("E2")    PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F2")    PORT_CODE(KEYCODE_W)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Gb2")   PORT_CODE(KEYCODE_3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G2")    PORT_CODE(KEYCODE_E)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Ab2")   PORT_CODE(KEYCODE_4)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A2")    PORT_CODE(KEYCODE_R)

	PORT_START("SWE")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bb2")   PORT_CODE(KEYCODE_5)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("B2")    PORT_CODE(KEYCODE_T)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C3")    PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Db3")   PORT_CODE(KEYCODE_7)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D3")    PORT_CODE(KEYCODE_U)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Eb3")   PORT_CODE(KEYCODE_8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("E3")    PORT_CODE(KEYCODE_I)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F3")    PORT_CODE(KEYCODE_O)

	PORT_START("SWF")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Gb3")   PORT_CODE(KEYCODE_0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G3")    PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Ab3")   PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A3")    PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bb3")   PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("B3")    PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C4")    PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Db4")   PORT_CODE(KEYCODE_L)

	PORT_START("SWG")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D4")    PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Eb4")   PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("E4")    PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Dec")   PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Inc")   PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

class qy70_state : public driver_device
{
public:
	qy70_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "lcdc")
		, m_switches(*this, "SW%c", 'A')
	{ }

	void qy70(machine_config &config);

private:
	required_device<h83002_device> m_maincpu;
	required_device<t6963c_device> m_lcdc;

	required_ioport_array<7> m_switches;

	u8 sw_sel;

	void pa_w(u8 data) { if (data) sw_sel = data; }
	u8 sw_in();
	u16 adc_bkupbat_r() { return 0x2a0; }
	void mem_map(address_map &map) ATTR_COLD;
	void lcd_map(address_map &map) ATTR_COLD;
	void lcd_palette(palette_device &palette) const;
	virtual void machine_start() override ATTR_COLD;
};

void qy70_state::machine_start()
{
	sw_sel = 0xff;
}

u8 qy70_state::sw_in()
{
	if (sw_sel == 0xff)
		return 0x1f; // Start in test mode for now. TODO: This should be an option.
	int idx = 6;
	while (idx && sw_sel != (1 << idx))
		idx--;
	return m_switches[idx]->read();
}

void qy70_state::mem_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("mainprog", 0);
	map(0x200000, 0x21ffff).ram().share("userdata.nv");
	//map(0x400000, 0x43ffff).rw("tosubcpu", ...);
	map(0x440000, 0x440001).rw("lcdc", FUNC(t6963c_device::read), FUNC(t6963c_device::write));
	map(0x480000, 0x480000).r(FUNC(qy70_state::sw_in));
	map(0x600000, 0x607fff).ram().share("workdata.nv");
}

void qy70_state::lcd_map(address_map &map)
{
	map(0x0000, 0x7fff).ram();
}

void qy70_state::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(69, 62, 66));
}

void qy70_state::qy70(machine_config &config)
{
	H83002(config, m_maincpu, 10_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &qy70_state::mem_map);
	// ADC_0  Power battery voltage
	// ADC_2  Host type select: 5V Mac, 3.33V PC-1, 1.66V PC-2, 0V MIDI
	m_maincpu->read_adc<4>().set(FUNC(qy70_state::adc_bkupbat_r));
	m_maincpu->write_porta().set(FUNC(qy70_state::pa_w));
	// PORT_B bit 0 1MHz output for Mac serial, bit 1 Rec LED, bit 2 Play LED,
	//        bit 3 lcdc reset, bit 4 GND, bit 5 subcpu SBSY, bit 6-7 subcpu

	NVRAM(config, "userdata.nv", nvram_device::DEFAULT_NONE);
	NVRAM(config, "workdata.nv", nvram_device::DEFAULT_NONE);

	T6963C(config, m_lcdc, 0);
	m_lcdc->set_addrmap(0, &qy70_state::lcd_map);
	m_lcdc->set_fs(2);
	m_lcdc->set_md(8);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_size(128, 64);
	screen.set_visarea_full();
	screen.set_screen_update("lcdc", FUNC(t6963c_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(qy70_state::lcd_palette), 2);

	SPEAKER(config, "speaker", 2).front();

	auto &mdin_a(MIDI_PORT(config, "mdin_a"));
	midiin_slot(mdin_a);
	mdin_a.rxd_handler().set(m_maincpu, FUNC(h83002_device::sci_rx_w<1>));

	auto &mdin_b(MIDI_PORT(config, "mdin_b"));
	midiin_slot(mdin_b);
	mdin_b.rxd_handler().set(m_maincpu, FUNC(h83002_device::sci_rx_w<0>));

	auto &mdout(MIDI_PORT(config, "mdout_a"));
	midiout_slot(mdout);
	m_maincpu->write_sci_tx<0>().set(mdout, FUNC(midi_port_device::write_txd));
}

ROM_START(qy70)
	ROM_REGION16_BE(0x200000, "mainprog", 0)
	ROM_LOAD("qy70main.bin", 0x000000, 0x200000, CRC(c581164b) SHA1(6f804cb6956dff50c5b64fb2f695bb36e3f278ec))

	ROM_REGION(0x400, "lcdc:cgrom", 0)
	ROM_LOAD("t6963c_0101.bin", 0x000, 0x400, CRC(547d118b) SHA1(0dd3e3acd3d47e6ece644c98c390fc86587373e9))
	// This t6963c_0101 internal CG ROM is similar to lm24014w_0101.bin which may be used as a replacement
ROM_END

} // anonymous namespace


CONS(1997, qy70, 0, 0, qy70, qy70, qy70_state, empty_init, "Yamaha", "QY70 Music Sequencer", MACHINE_NOT_WORKING)
