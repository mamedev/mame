// license:BSD-3-Clause
// copyright-holders:Devin Acker

/*
    Casio WK-1600/1800 series keyboards

    Models on this hardware:
        - CTK-711EX (1998)
            61 keys, 5MB wave ROM
        - CTK-811EX (1998), CTK-731 (1999)
            61 keys, 5MB wave ROM, floppy drive
        - WK-1600, WK-1630 (2000)
            73 keys, 8MB wave ROM
        - WK-1800 (2000)
            73 keys, 8MB wave ROM, floppy drive
        - AP-60R (1999), AP-65R (2001)
            88 keys, 8MB wave ROM, floppy drive

    TODO:
        - fix floppy controller hookup for wk1800. current issues:
            - pressing the Disk button with the drive empty starts the drive motor,
              then the firmware waits forever on some status bit that is never set
            - pressing the Disk button with a disk inserted results in several 'forced abort'
              errors from the H8 DMA controller
            - wk1800 firmware seems to rely on different TS bit behavior from the HD63266
              compared to a standard uPD765
        - add software list for style/program disks
 */

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/h8/h83048.h"
#include "imagedev/floppy.h"
#include "machine/nvram.h"
#include "machine/gt913_kbd.h"
#include "machine/upd765.h"
#include "sound/gt155.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include <algorithm>

namespace {

class wk1600_state : public driver_device
{
public:
	wk1600_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gt155(*this, "gt155")
		, m_lcdc(*this, "lcdc")
		, m_sound_rom(*this, "gt155")
		, m_inputs(*this, "KC%u", 0U)
		, m_outputs(*this, "%02x.%d.%d", 0U, 0U, 0U)
		, m_led(*this, "led%d", 0U)
		, m_led_power(*this, "led_power")
	{
	}

	void wk1600(machine_config &config) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(nmi_clear) { m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE); }

	ioport_value lcd_r()   { return m_lcdc->db_r() >> 4; }
	void lcd_w(int state)  { m_lcdc->db_w(state << 4); }

	void shift_data_w(int state) { m_shift_data = state; }
	void led_clk_w(int state);
	void input_clk_w(int state);

	DECLARE_INPUT_CHANGED_MEMBER(power_w);

	template<int StartBit> ioport_value inputs_r();

	void apo_w(int state);

protected:
	virtual void driver_start() override ATTR_COLD;

	void common_map(address_map &map) ATTR_COLD;

	required_device<h83048_device> m_maincpu;
	required_device<gt155_device> m_gt155;
	required_device<hd44780_device> m_lcdc;

private:
	void wk1600_map(address_map &map) ATTR_COLD;

	void render_w(int state);

	required_memory_region m_sound_rom;

	emu_timer* m_nmi_timer = nullptr;

	optional_ioport_array<8> m_inputs;

	output_finder<64, 8, 5> m_outputs;
	output_finder<8> m_led;
	output_finder<> m_led_power;

	u8 m_sound_regs[16];
	u32 m_sound_rom_addr;
	u32 m_dsp_data[128];

	u8 m_led_sel, m_input_sel;
	u8 m_led_clk, m_input_clk, m_shift_data;
};

class wk1800_state : public wk1600_state
{
public:
	static constexpr feature_type unemulated_features() { return feature::DISK; }

	wk1800_state(machine_config const &mconfig, device_type type, char const *tag)
		: wk1600_state(mconfig, type, tag)
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:0")
	{
	}

	void wk1800(machine_config &config) ATTR_COLD;

	void fdc_rate_w(int state) { if (m_fdc) m_fdc->rate_w(state); }

private:
	void wk1800_map(address_map &map) ATTR_COLD;

	optional_device<hd63266f_device> m_fdc;
	optional_device<floppy_connector> m_floppy;
};

/**************************************************************************/
void wk1600_state::led_clk_w(int state)
{
	if (state && !m_led_clk)
	{
		m_led_sel <<= 1;
		m_led_sel |= (m_shift_data & 1);

		for (int i = 0; i < 8; i++)
			m_led[i] = BIT(~m_led_sel, i);
	}

	m_led_clk = state;
}

/**************************************************************************/
void wk1600_state::input_clk_w(int state)
{
	if (state && !m_input_clk)
	{
		m_input_sel <<= 1;
		m_input_sel |= (m_shift_data & 1);
	}

	m_input_clk = state;
}

/**************************************************************************/
INPUT_CHANGED_MEMBER(wk1600_state::power_w)
{
	if (newval)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		m_nmi_timer->adjust(attotime::never);
	}
	else
	{
		// give the CPU enough time to switch NMI to active-high so it fires again
		// otherwise, releasing the power button too quickly may be ignored
		m_nmi_timer->adjust(attotime::from_msec(100));
	}
}

/**************************************************************************/
template<int StartBit>
ioport_value wk1600_state::inputs_r()
{
	ioport_value result = 0;
	for (unsigned i = 0U; i < m_inputs.size(); i++)
	{
		if (BIT(m_input_sel, i))
			result |= m_inputs[i].read_safe(0);
	}

	return result >> StartBit;
}

/**************************************************************************/
void wk1600_state::apo_w(int state)
{
	logerror("apo_w: %x\n", state);
	if (!state)
		m_lcdc->reset();

	m_led_power = state;
	m_gt155->set_output_gain(ALL_OUTPUTS, state ? 1.0 : 0.0);
}

/**************************************************************************/
void wk1600_state::render_w(int state)
{
	if (!state)
		return;

	const u8 *render = m_lcdc->render();
	for (int x = 0; x < 64; x++)
	{
		for (int y = 0; y < 8; y++)
		{
			u8 v = *render++;
			for (int z = 0; z < 5; z++)
				m_outputs[x][y][z] = (v >> z) & 1;
		}
		render += 8;
	}
}


/**************************************************************************/
void wk1600_state::common_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x2ffff).rw(m_gt155, FUNC(gt155_device::read), FUNC(gt155_device::write));
	map(0x30000, 0x30001).mirror(0x0fff0).r("kbd", FUNC(gt913_kbd_hle_device::read));
	map(0x30002, 0x30003).mirror(0x0fff0).r("kbd", FUNC(gt913_kbd_hle_device::status_r));
}

/**************************************************************************/
void wk1600_state::wk1600_map(address_map &map)
{
	common_map(map);
	map(0x80000, 0x9ffff).mirror(0x60000).ram().share("nvram");
}

/**************************************************************************/
void wk1800_state::wk1800_map(address_map &map)
{
	common_map(map);
//  map(0x40000, 0x40003).mirror(0x1fffc).m(m_fdc, FUNC(hd63266f_device::map));
//  map(0x60000, 0x7ffff).rw(m_fdc, FUNC(hd63266f_device::dma_r), FUNC(hd63266f_device::dma_w));
	map(0x80000, 0xbffff).mirror(0x40000).ram().share("nvram");
}


/**************************************************************************/
void wk1600_state::driver_start()
{
	m_led.resolve();
	m_led_power.resolve();
	m_outputs.resolve();

	m_nmi_timer = timer_alloc(FUNC(wk1600_state::nmi_clear), this);

	std::fill(std::begin(m_sound_regs), std::end(m_sound_regs), 0);
	std::fill(std::begin(m_dsp_data), std::end(m_dsp_data), 0);
	m_sound_rom_addr = 0;

	m_led_sel = 0xff;
	m_input_sel = 0;

	m_led_clk = m_input_clk = m_shift_data = 0;

	save_item(NAME(m_sound_regs));
	save_item(NAME(m_dsp_data));
	save_item(NAME(m_sound_rom_addr));

	save_item(NAME(m_led_sel));
	save_item(NAME(m_input_sel));

	save_item(NAME(m_led_clk));
	save_item(NAME(m_input_clk));
	save_item(NAME(m_shift_data));
}


/**************************************************************************/
void wk1600_state::wk1600(machine_config &config)
{
	H83048(config, m_maincpu, 16'000'000).set_mode_a20();
	m_maincpu->set_addrmap(AS_PROGRAM, &wk1600_state::wk1600_map);
	m_maincpu->read_adc<0>().set_constant(0);
	m_maincpu->read_adc<1>().set_ioport("AN1");
	m_maincpu->read_adc<2>().set_constant(0);
	m_maincpu->read_adc<3>().set_ioport("AN3");
	m_maincpu->read_port6().set_ioport("P6");
	m_maincpu->read_port7().set_ioport("P7");
	m_maincpu->read_port8().set_ioport("P8");
	m_maincpu->write_port8().set_ioport("P8");
	m_maincpu->read_port9().set_ioport("P9");
	m_maincpu->read_porta().set_ioport("PA");
	m_maincpu->write_porta().set_ioport("PA");
	m_maincpu->read_portb().set(FUNC(wk1600_state::lcd_r));
	m_maincpu->write_portb().set_ioport("PB");

	NVRAM(config, "nvram");

	GT913_KBD_HLE(config, "kbd"); // actually TC190C020AF-001 gate array

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set(m_maincpu, FUNC(h83048_device::sci_rx_w<0>));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_maincpu->write_sci_tx<0>().set(mdout, FUNC(midi_port_device::write_txd));

	HD44780(config, m_lcdc, 270'000); // TODO: Wrong device type, should be SED1278F2A; clock not measured, datasheet typical clock used
	m_lcdc->set_lcd_size(2, 8);

	auto &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1755, 450);
	screen.set_visarea_full();
	screen.screen_vblank().set(FUNC(wk1600_state::render_w));

	SPEAKER(config, "speaker", 2).front();

	GT155(config, m_gt155, 24.576_MHz_XTAL);
	m_gt155->add_route(0, "speaker", 1.0, 0);
	m_gt155->add_route(1, "speaker", 1.0, 1);
}

/**************************************************************************/
[[maybe_unused]] static void wk1800_floppies(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_HD);
}

/**************************************************************************/
void wk1800_state::wk1800(machine_config &config)
{
	wk1600(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &wk1800_state::wk1800_map);

#if 0
	HD63266F(config, m_fdc, 16'000'000);
	m_fdc->set_ready_line_connected(false);
	m_fdc->drq_wr_callback().set_inputline(m_maincpu, H8_INPUT_LINE_DREQ0);
	m_maincpu->tend0().set(m_fdc, FUNC(upd765a_device::tc_line_w));

	FLOPPY_CONNECTOR(config, m_floppy, wk1800_floppies, "35hd", floppy_image_device::default_pc_floppy_formats);
	m_floppy->enable_sound(true);
	SOFTWARE_LIST(config, "flop_list").set_compatible("midi_flop");
#endif
}


INPUT_PORTS_START(wk1600)
	PORT_START("kbd:FI0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("E1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("F1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("F1#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("G1")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("G1#")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("A1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("A1#")

	PORT_START("kbd:FI1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("B1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("C2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("C2#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("D2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("D2#")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("E2")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("F2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("F2#")

	PORT_START("kbd:FI2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("G2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("G2#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("A2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("A2#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("B2")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("C3")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("C3#")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("D3")

	PORT_START("kbd:FI3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("D3#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("E3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("F3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("F3#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("G3")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("G3#")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("A3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("A3#")

	PORT_START("kbd:FI4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("B3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("C4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("C4#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("D4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("D4#")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("E4")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("F4")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("F4#")

	PORT_START("kbd:FI5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("G4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("G4#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("A4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("A4#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("B4")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("C5")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("C5#")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("D5")

	PORT_START("kbd:FI6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("D5#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("E5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("F5")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("F5#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("G5")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("G5#")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("A5")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("A5#")

	PORT_START("kbd:FI7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("B5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("C6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("C6#")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("D6")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("D6#")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("E6")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("F6")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("F6#")

	PORT_START("kbd:FI8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("G6")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("G6#")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("A6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("A6#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("B6")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("C7")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("C7#")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("D7")

	PORT_START("kbd:FI9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("D7#")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("E7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("F7")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("F7#")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("G7")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("kbd:FI10")
	PORT_START("kbd:KI0")
	PORT_START("kbd:KI1")
	PORT_START("kbd:KI2")

	PORT_START("kbd:VELOCITY")
	PORT_BIT( 0xff, 0xff, IPT_POSITIONAL ) PORT_NAME("Key Velocity") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_CODE_DEC(KEYCODE_PGDN) PORT_CODE_INC(KEYCODE_PGUP)

	PORT_START("KC0")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Mode")
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Intro")
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Mixer Select")
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Channel 8 / Chord 3")
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Registration A")
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Channel 16 / Track 6")
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Split")
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad - / No") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Demo")

	PORT_START("KC1")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Record")
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Normal / Fill-In")
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Channel 1 / Upper 1")
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Channel 9 / Bass")
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Registration B")
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Auto Harmonize")
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Layer")
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Synth")

	PORT_START("KC2")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Song")
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Variation / Fill-In")
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Channel 2 / Upper 2")
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Channel 10 / Rhythm")
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Registration C")
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Rhythm")
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tune")

	PORT_START("KC3")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Pattern")
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Synchro / Ending")
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Channel 3 / Lower 1")
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Channel 11 / Track 1")
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Registration D")
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Cursor Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tone")
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("MIDI")

	PORT_START("KC4")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("DSP")
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Start / Stop")
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Channel 4 / Lower 2")
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Channel 12 / Track 2")
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Registration E")
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Cursor Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad + / Yes") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KC5")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Contrast")
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tempo Down")
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Channel 5 / Acc. Vol.")
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Channel 13 / Track 3")
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Registration Store")
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Cursor Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KC6")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Free Session")
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tempo Up")
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Channel 6 / Chord 1")
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Channel 14 / Track 4")
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Transpose Down")
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Cursor Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KC7")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("One Touch Preset")
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Registration Bank")
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Channel 7 / Chord 2")
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Channel 15 / Track 5")
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Transpose Up")
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Touch Response")
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SWITCH")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POWER_ON ) PORT_NAME("Power") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(wk1600_state::power_w), 0)

	PORT_START("AN1")
	PORT_BIT( 0x3ff, 0x200, IPT_PADDLE ) PORT_NAME("Pitch Wheel") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_MINMAX(0x00, 0x3ff) PORT_CODE_DEC(JOYCODE_Y_DOWN_SWITCH) PORT_CODE_INC(JOYCODE_Y_UP_SWITCH)

	PORT_START("AN3")
	PORT_BIT( 0x3ff, 0x000, IPT_POSITIONAL_V ) PORT_NAME("Modulation Wheel") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_MINMAX(0x00, 0x3ff) PORT_PLAYER(2) PORT_CODE_DEC(JOYCODE_Y_DOWN_SWITCH) PORT_CODE_INC(JOYCODE_Y_UP_SWITCH)

	PORT_START("P6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(wk1600_state::inputs_r<1>))
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(wk1600_state::inputs_r<0>))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x04, 0x00, "Power Source" )
	PORT_CONFSETTING(    0x00, "AC Adapter" )
	PORT_CONFSETTING(    0x04, "Battery" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER  ) PORT_NAME("Pedal")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(wk1600_state::inputs_r<4>))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) // reset sound, LCD, FDC
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) // high = WK-1800, low = WK-1600
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(wk1600_state::inputs_r<3>))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x38, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(wk1600_state::inputs_r<5>))

	PORT_START("PA")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(wk1600_state::inputs_r<8>))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(wk1600_state::input_clk_w))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(wk1600_state::shift_data_w))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(wk1600_state::led_clk_w))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(wk1600_state::apo_w))

	PORT_START("PB")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(wk1600_state::lcd_w))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("lcdc", FUNC(hd44780_device::e_w))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("lcdc", FUNC(hd44780_device::rw_w))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("lcdc", FUNC(hd44780_device::rs_w))
INPUT_PORTS_END

INPUT_PORTS_START( wk1800 )
	PORT_INCLUDE(wk1600)

	PORT_MODIFY("KC2")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tune / MIDI")

	PORT_MODIFY("KC3")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Touch Response")

	PORT_MODIFY("KC7")
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Disk")

	PORT_MODIFY("P7")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) // TODO: disk HD/DD detect

	PORT_MODIFY("P8")
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_CUSTOM ) // high = WK-1800, low = WK-1600

	PORT_MODIFY("PA")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(wk1800_state::fdc_rate_w))
INPUT_PORTS_END


ROM_START(wk1800)
	ROM_REGION(0x20000, "maincpu", 0) // "Ver.1.61"
	ROM_LOAD("hd6433048sa89f.lsi9", 0x00000, 0x20000, CRC(bd5bfab3) SHA1(2731b5ab1cb288553bfee9b856264a5d1eb0ef1a))

	ROM_REGION16_LE(0x800000, "gt155", 0) // "Ver.1.60"
	ROM_LOAD("lhmn5kpn.lsi2", 0x000000, 0x400000, CRC(f75d21f0) SHA1(e08937ce2fa152db85fa96cef53f81351e690666))
	ROM_LOAD("lhmn5kpp.lsi1", 0x400000, 0x400000, CRC(f6cc5048) SHA1(9f48730a5bd3582f6fe08cb937848907d11aa804))

	ROM_REGION(585110, "screen", 0)
	ROM_LOAD("wk1800.svg", 0, 585110, CRC(5fab0b26) SHA1(6181b9eb950cd30474efb37b8cd660cba4b0b914))
ROM_END

#define rom_wk1600 rom_wk1800

} // anonymous namespace

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY  FULLNAME   FLAGS
SYST( 2000, wk1800,  0,      0,      wk1800,  wk1800, wk1800_state, empty_init,  "Casio", "WK-1800", MACHINE_SUPPORTS_SAVE )
SYST( 2000, wk1600,  wk1800, 0,      wk1600,  wk1600, wk1600_state, empty_init,  "Casio", "WK-1600", MACHINE_SUPPORTS_SAVE )
