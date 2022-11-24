// license:BSD-3-Clause
// copyright-holders:ajxs
/*******************************************************************************

    Skeleton driver for the Yamaha DX9 FM synthesizer.
    There is currently no MAME emulation of the OPS/EGS chips, so emulating
    the synth's tone generation functionality is not possible.
    The cassette interface is currently not emulated.
    While there are rumours that an updated firmware ROM exists, this driver
    is set up to work with the only one that is widely available.

*******************************************************************************/

#include "emu.h"

#include "bus/midi/midi.h"
#include "cpu/m6800/m6801.h"
#include "machine/adc0808.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"

#include "dx9.lh"


namespace {

class yamaha_dx9_state : public driver_device
{
public:
	yamaha_dx9_state(const machine_config &mconfig, device_type type, const char *tag)
			: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_adc(*this, "adc"),
			m_leds(*this, "led_%u", 0U),
			m_key_switch_input_driver_line_0(*this, "KEY_SWITCH_INPUT_DRIVER_LINE_0"),
			m_key_switch_input_driver_line_1(*this, "KEY_SWITCH_INPUT_DRIVER_LINE_1"),
			m_key_switch_input_driver_line_2(*this, "KEY_SWITCH_INPUT_DRIVER_LINE_2"),
			m_key_switch_input_driver_line_3(*this, "KEY_SWITCH_INPUT_DRIVER_LINE_3")
	{
	}

	void dx9(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	/**
	 * @brief The 63B03RP microcontroller instance.
	 * Also refer to: https://docs.mamedev.org/techspecs/object_finders.html
	 */
	required_device<hd6303r_cpu_device> m_maincpu;
	required_device<adc0808_device> m_adc;
	output_finder<2> m_leds;
	// These ioport instances are used to communicate with the interface in the layout.
	// They emulate the circuits wired to the 'Key/Switch Scan Driver'.
	required_ioport m_key_switch_input_driver_line_0;
	required_ioport m_key_switch_input_driver_line_1;
	required_ioport m_key_switch_input_driver_line_2;
	required_ioport m_key_switch_input_driver_line_3;

	/**
	 * @brief Which input line on the keyboard/switch scan driver is currently selected.
	 * This driver is used to read the analog switch values from the synth's keyboard, and 
	 * front-panel switches.
	 * TODO: I'm currently not sure of the actual implementation of this circuit. This
	 * implementation is based off the *very* limited description in the service manual, and
	 * the behaviour that's shown in the firmware.
	 */
	uint8_t m_key_switch_scan_driver_select = 0;

	int m_rx_data;

	/** The polarity of the cassette interface's output line. */
	bool m_cassette_interface_output_polarity = 0;

	/** The polarity of the cassette interface's remote line. */
	bool m_cassette_interface_remote_polarity = 0;

	/**
	* @brief LCD pixel update function.
	* The `HD44780_PIXEL_UPDATE` macro expands the definition to include the correct
	* parameters for the LCD update function.
	* Refer to: `src/devices/video/hd44780.h` for the full definition format.
	*/
	HD44780_PIXEL_UPDATE(lcd_pixel_update);

	/**
	* @brief Handles a write to the synth's OPS chip registers.
	* This chip is currently not emulated, however this function is useful for debugging.
	* @param offset The offset into the memory mapped region being written.
	* @param data The data being written.
	*/
	void ops_w(offs_t offset, uint8_t data);

	/**
	* @brief Handles a write to the synth's EGS chip registers.
	* This chip is currently not emulated, however this function is useful for debugging.
	* @param offset The offset into the memory mapped region being written.
	* @param data The data being written.
	*/
	void egs_w(offs_t offset, uint8_t data);

	/**
	 * @brief Handles a read from the keyboard/switch scan driver.
	 * This multiplexing driver circuit is used to read the states of the synth's front-panel
	 * switches, and keyboard. The driver's input is wired to the CPU's IO port 1, and the 
	 * output is wired into the address map.
	 * Refer to the comments inside the function for the driver's internal mapping.
	 * @param offset The offset into the memory mapped region being read.
	 * @return uint8_t The value read from the bus.
	 */
	uint8_t key_switch_scan_driver_r(offs_t offset);

	/**
	* @brief Handles a write to the 7-segment LED memory mapped region.
	* This function is responsible for setting the two 7-segment LEDs set in the
	* device's layout file.
	* @param offset The offset into the memory mapped region being written.
	* @param data The data being written.
	*/
	void led_w(offs_t offset, uint8_t data);

	void palette_init(palette_device &palette);

	void mem_map(address_map &map);

	WRITE_LINE_MEMBER(midi_r) { m_rx_data = state; }

	WRITE_LINE_MEMBER(midiclock_w) { if (state) m_maincpu->m6801_clock_serial(); }

	/**
	 * @brief Handle a write to the synth's IO Port 1.
	 * IO Port 1 is mapped as follows:
	 *  Bit 0: Keyboard/Switch Scan Driver Input.
	 *  Bit 1: "".
	 *  Bit 2: "".
	 *  Bit 3: "".
	 *  Bit 4: ADC EOC Input Line.
	 *  Bit 5: Cassette Interface Remote Port.
	 *  Bit 6: Cassette Interface Tape Output.
	 *  Bit 7: Cassette Interface Tape Input.
	 * @param offset The offset into the memory mapped region being written.
	 * @param data The data being written.
	 */
	void p1_w(offs_t offset, uint8_t data);

	/**
	 * @brief Handle a read from the synth's IO Port 1.
	 * @param offset The offset into the memory mapped region being read.
	 * @return uint8_t The value read from the port.
	 */
	uint8_t p1_r(offs_t offset);
};


/**
* yamaha_dx9_state::machine_start
*/
void yamaha_dx9_state::machine_start()
{
	m_leds.resolve();
	m_rx_data = ASSERT_LINE;
}


/**
 * yamaha_dx9_state::lcd_pixel_update
 */
HD44780_PIXEL_UPDATE(yamaha_dx9_state::lcd_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 16)
			bitmap.pix(line * 10 + y + 1 + ((y == 7) ? 1 : 0), pos * 6 + x + 1) = state ? 1 : 2;
}


/**
 * yamaha_dx9_state::palette_init
 */
void yamaha_dx9_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(0x87, 0xAD, 0x34)); // background
	palette.set_pen_color(1, rgb_t(0x0, 0x0, 0x0)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(0x7D, 0x9F, 0x32)); // lcd pixel off
}


/**
 * yamaha_dx9_state::mem_map
 */
void yamaha_dx9_state::mem_map(address_map &map)
{
	// Internal CPU registers.
	map(0x0000, 0x001f).m(m_maincpu, FUNC(hd6303r_cpu_device::m6801_io));

	map(0x0020, 0x0020).r(FUNC(yamaha_dx9_state::key_switch_scan_driver_r));

	map(0x0022, 0x0022).r("adc", FUNC(m58990_device::data_r));
	map(0x0024, 0x0024).w("adc", FUNC(m58990_device::address_data_start_w));

	// YM21280 OPS.
	map(0x0026, 0x0027).w(FUNC(yamaha_dx9_state::ops_w));
	// HD44780 LCD Controller.
	map(0x0028, 0x0029).rw("lcdc", FUNC(hd44780_device::read), FUNC(hd44780_device::write));
	// LED.
	map(0x002b, 0x002c).w(FUNC(yamaha_dx9_state::led_w));

	// Internal RAM.
	map(0x0040, 0x00ff).ram();

	// External RAM.
	// 2 * 2kb RAM1 IC19 M5M118P.
	map(0x0800, 0x0fff).ram().share("ram1");
	map(0x1000, 0x1800).ram().share("ram2");

	// YM21290 EGS
	map(0x1800, 0x18f3).w(FUNC(yamaha_dx9_state::egs_w));

	// ROM.
	map(0xc000, 0xffff).rom().region("program", 0);
}


/**
 * yamaha_dx9_state::dx9
 */
void yamaha_dx9_state::dx9(machine_config &config)
{
	// Initialise the HD63B03RP CPU.
	// This oscillator frequency comes from the service manual.
	HD6303R(config, m_maincpu, 9.4265_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &yamaha_dx9_state::mem_map);

	// Unlike the DX7 only IO port 1 is used.
	// The direction flags of other ports are set, however they are never read, or written.
	m_maincpu->in_p1_cb().set(FUNC(yamaha_dx9_state::p1_r));
	m_maincpu->out_p1_cb().set(FUNC(yamaha_dx9_state::p1_w));

	NVRAM(config, "ram1", nvram_device::DEFAULT_ALL_0);
	NVRAM(config, "ram2", nvram_device::DEFAULT_ALL_0);

	// Configure the ADC. The clock speed here is a guess.
	M58990(config, m_adc, 8_MHz_XTAL / 16);

	// ADC source 4 is the battery voltage. Set this input to always read 0x80.
	// If the read value is below 0x6F, the firmware considers this a low battery voltage.
	m_adc->in_callback<4>().set_constant(0x80);

	// Configure MIDI.
	auto &midiclock(CLOCK(config, "midiclock", 500_kHz_XTAL));
	midiclock.signal_handler().set(FUNC(yamaha_dx9_state::midiclock_w));

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(FUNC(yamaha_dx9_state::midi_r));

	auto &mdout(MIDI_PORT(config, "mdout", midiout_slot, "midiout"));
	m_maincpu->out_ser_tx_cb().set(mdout, FUNC(midi_port_device::write_txd));

	// Configure the LCD screen.
	screen_device &lcd_screen(SCREEN(config, "lcd_screen", SCREEN_TYPE_LCD));
	lcd_screen.set_refresh_hz(60);
	lcd_screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	lcd_screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	lcd_screen.set_size(6*16+1, 10*2+1);
	lcd_screen.set_visarea_full();
	lcd_screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(yamaha_dx9_state::palette_init), 3);

	hd44780_device &lcdc(HD44780(config, "lcdc", 0));
	lcdc.set_lcd_size(2, 16);
	lcdc.set_pixel_update_cb(FUNC(yamaha_dx9_state::lcd_pixel_update));

	config.set_default_layout(layout_dx9);
}


/**
 * yamaha_dx9_state::key_switch_scan_driver_r
 */
uint8_t yamaha_dx9_state::key_switch_scan_driver_r(offs_t offset)
{
	/**
	 * @brief The value to be returned.
	 * This is implemented as a separate variable to allow for much easier debugging, as
	 * opposed to just instantly returning the value read from the input lines.
	 */
	uint8_t value = 0;

	// Note: Input line 4 covers the keyboard circuit, which is not implemented here.
	// When the keyboard state is read, the default value of 0 will be returned.
	if(m_key_switch_scan_driver_select == 0)
	{
		// Input line 0 covers the 'main' front-panel switches.
		value = m_key_switch_input_driver_line_0->read();
	} else if(m_key_switch_scan_driver_select == 1)
	{
		// Input line 1 covers the numeric front-panel switches 1 through 8.
		value = m_key_switch_input_driver_line_1->read();
	} else if(m_key_switch_scan_driver_select == 2)
	{
		// Input line 2 covers the numeric front-panel switches 9 though 16.
		value = m_key_switch_input_driver_line_2->read();
	} else if(m_key_switch_scan_driver_select == 3)
	{
		// Input line 3 covers the numeric front-panel switches 17 though 20.
		// The Portamento, and Sustain pedals are mapped to bits 6, and 7 respectively.
		value = m_key_switch_input_driver_line_3->read();
	}

	return value;
}


/**
 * yamaha_dx9_state::ops_w
 */
void yamaha_dx9_state::ops_w(offs_t offset, uint8_t data)
{
	// printf("OPS: %02X=%02X\n", offset, data);
}


/**
 * yamaha_dx9_state::egs_w
 */
void yamaha_dx9_state::egs_w(offs_t offset, uint8_t data)
{
	// printf("EGS: %02X=%02X\n", offset, data);
}


/**
 * yamaha_dx9_state::led_w
 */
void yamaha_dx9_state::led_w(offs_t offset, uint8_t data)
{
	// Since the memory mapped region that calls this function is only two byts in
	// size, the led number is the least-significant bit of the offset.
	// The DX9's LEDs are wired so that a high input line disables the segment, so
	// get the one's complement of the data.
	m_leds[offset & 1] = (~data) & 0xFF;
}


/**
 * yamaha_dx9_state::p1_r
 */
uint8_t yamaha_dx9_state::p1_r(offs_t offset)
{
	// The ADC EOC line is wired to bit 4, as well as the Cassette Interface input, which
	// is wired to bit 7. This is currently not fully implemented.
	return m_adc->eoc_r() << 4;
}


/**
 * yamaha_dx9_state::p1_w
 */
void yamaha_dx9_state::p1_w(offs_t offset, uint8_t data)
{
	// The low-nibble is written by the firmware to select the key/switch driver input line.
	m_key_switch_scan_driver_select = data & 0xF;

	// The cassette interface remote port polarity is set by bit 5.
	m_cassette_interface_remote_polarity = data & 0x20;

	// The cassette interface output polarity is set by bit 6.
	m_cassette_interface_output_polarity = data & 0x40;
}


static INPUT_PORTS_START(dx9)
	PORT_START("KEY_SWITCH_INPUT_DRIVER_LINE_0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1)    // Front-panel button Yes/Up.
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2)    // Front-panel button No/Down.
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3)    // Front-panel button Store.
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5)    // Front-panel button Function.
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON6)    // Front-panel button Edit.
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON7)    // Front-panel button Memory.
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON8)

	PORT_START("KEY_SWITCH_INPUT_DRIVER_LINE_1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1)    // Front-panel button 1.
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2)    // Front-panel button 2.
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3)    // Front-panel button 3.
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4)    // Front-panel button 4.
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5)    // Front-panel button 5.
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON6)    // Front-panel button 6.
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON7)    // Front-panel button 7.
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON8)    // Front-panel button 8.

	PORT_START("KEY_SWITCH_INPUT_DRIVER_LINE_2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1)    // Front-panel button 9.
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2)    // Front-panel button 10.
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3)    // Front-panel button 11.
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4)    // Front-panel button 12.
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5)    // Front-panel button 13.
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON6)    // Front-panel button 14.
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON7)    // Front-panel button 15.
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON8)    // Front-panel button 16.

	PORT_START("KEY_SWITCH_INPUT_DRIVER_LINE_3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2)    // Front-panel button 17.
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3)    // Front-panel button 18.
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4)    // Front-panel button 19.
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5)    // Front-panel button 20.
INPUT_PORTS_END


ROM_START(dx9)
	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("dx9.bin", 0x0000, 0x4000, CRC(c45e1832) SHA1(a92d7add3b89537ad06c719da0005c450d178d81))
ROM_END

} // anonymous namespace


SYST(1983, dx9, 0, 0, dx9, dx9, yamaha_dx9_state, empty_init, "Yamaha", "DX9 Digital Programmable Algorithm Synthesizer", MACHINE_NO_SOUND | MACHINE_CLICKABLE_ARTWORK)
