// license: BSD-3-Clause
// copyright-holders: Devin Acker
/***************************************************************************

    Breakaway Vocalizer 1000

    Voice-powered synth and MIDI controller. ES5503 "DOC"-based.

    To enable MIDI input, press "Option" and then "Melody Guide".

    U11: 512kbit program ROM
    U12: Sony CXK5864PM-15L (64kbit SRAM)
    U13: Motorola MC68B09EP
    U17: NCR E106-71 609-0381069 (ES5503 compatible/clone)
    U18: 512kbit wave ROM
    U19: Breakaway GA1 NCR0880995
    Y1: 8.000 MHz

    TODO:
    - Microphone input. Data is read by function at $9BFD (after FIRQ_MIC)
      and then processed by function at $C705.
    - Instruments that use the sync/AM bit sound weird (need hardware recordings)
    - Link cable (uses UART 0)
    - Song cartridges (connected to CPU at $0000-3fff)
    - Instrument cartridges (connected to DOC at $10000+)

    Misc. notes:
    - Hold "Cursor Left" / "Delete" on boot to run a self test and re-init NVRAM.
    - Hold the "Jazz" style button on boot to run a key/button test.
      Press all buttons (in any order), including both handset buttons,
      then press any button to exit.
    - Press "Option" and then "Master Volume -" to display the ROM version.

***************************************************************************/

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/m6809/m6809.h"
#include "machine/nvram.h"
#include "sound/es5503.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

//#define VERBOSE 1
#include "logmacro.h"

class vocalizer_uart_device : public device_t, public device_serial_interface
{
public:
	vocalizer_uart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto tx_cb() { return m_tx_cb.bind(); }
	auto tx_irq_cb() { return m_tx_irq_cb.bind(); }
	auto rx_irq_cb() { return m_rx_irq_cb.bind(); }

	u8 read() { m_rx_irq_cb(0); return get_received_char(); }
	void write(u8 data) { m_tx_irq_cb(0); transmit_register_setup(data); }

protected:
	virtual void device_start() override {}
	virtual void device_reset() override ATTR_COLD;

	virtual void tra_callback() override { m_tx_cb(transmit_register_get_data_bit()); }
	virtual void tra_complete() override { m_tx_irq_cb(1); }
	virtual void rcv_complete() override { receive_register_extract(); m_rx_irq_cb(1); }

	devcb_write_line m_tx_cb;
	devcb_write_line m_tx_irq_cb;
	devcb_write_line m_rx_irq_cb;
};

DEFINE_DEVICE_TYPE(VOCALIZER_UART, vocalizer_uart_device, "vocalizer_uart", "Vocalizer 1000 UART")

vocalizer_uart_device::vocalizer_uart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):
	device_t(mconfig, VOCALIZER_UART, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_tx_cb(*this),
	m_tx_irq_cb(*this),
	m_rx_irq_cb(*this)
{
}

void vocalizer_uart_device::device_reset()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(31250);
	m_tx_irq_cb(0);
	m_rx_irq_cb(0);
}

namespace {

//**************************************************************************
class vocalizer_state : public driver_device
{
public:
	vocalizer_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_lcdc(*this, "lcdc"),
		m_es5503(*this, "es5503"),
		m_uart(*this, "uart%u", 0),
		m_bank(*this, "rombank"),
		m_keys(*this, "IN%u", 0)
	{ }

	void vocalizer(machine_config &config);

	void vocalizer_palette(palette_device &palette) const;
	HD44780_PIXEL_UPDATE(lcd_pixel_update);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	enum
	{
		FIRQ_TX0 = (1<<0), // UART 0 Tx (link cable)
		FIRQ_RX  = (1<<1), // UART Rx (either one)
		FIRQ_MIC = (1<<2), // audio input detection (triggered by input waveform edge, or something else?)
		FIRQ_TX1 = (1<<3)  // UART 1 Tx (MIDI)
	};

	void maincpu_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	template <int Shift> u8 time_r();

	u8 uart_rx_r();
	template <int Num> void uart_rx_irq(int state);

	u8 status_r();
	void config_w(u8 data) { m_config = data; }
	void input_sel_w(u8 data) { m_input_sel = BIT(data, 3, 4); }

	template<int Num> void firq_w(int state);
	void firq_ack_w(u8 data);
	void firq_mask_w(u8 data);
	void firq_update();

	void volume_w(u8 data);

	void apo_w(u8 data);

	template <int Num> void bank_w(u8 data) { m_bank->set_entry(Num); }

	required_device<mc6809e_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
	required_device<es5503_device> m_es5503;
	required_device_array<vocalizer_uart_device, 2> m_uart;
	required_memory_bank m_bank;
	required_ioport_array<12> m_keys;

	u8 m_power;
	u8 m_config;
	u8 m_input_sel;
	u8 m_firq_status, m_firq_mask;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void vocalizer_state::maincpu_map(address_map &map)
{
	map(0x0000, 0x3fff).noprw(); // TODO: song cartridge
	map(0x4000, 0x40ff).mirror(0x0f00).rw(m_es5503, FUNC(es5503_device::read), FUNC(es5503_device::write));
	map(0x5000, 0x5000).mirror(0x0ff0).r(FUNC(vocalizer_state::uart_rx_r));
	map(0x5000, 0x5000).mirror(0x0ff0).w(m_uart[0], FUNC(vocalizer_uart_device::write));
	map(0x5001, 0x5001).mirror(0x0ff0).rw(FUNC(vocalizer_state::status_r), FUNC(vocalizer_state::config_w));
	map(0x5002, 0x5002).mirror(0x0ff0).rw(FUNC(vocalizer_state::time_r<8>), FUNC(vocalizer_state::input_sel_w));
	map(0x5003, 0x5003).mirror(0x0ff0).r(FUNC(vocalizer_state::time_r<0>));
	map(0x5003, 0x5003).mirror(0x0ff0).w(m_uart[1], FUNC(vocalizer_uart_device::write));
	map(0x5004, 0x5005).mirror(0x0ff0).nopr(); // TODO: audio in amplitude
	map(0x5004, 0x5004).mirror(0x0ff0).w(FUNC(vocalizer_state::firq_ack_w));
	map(0x5005, 0x5005).mirror(0x0ff0).w(FUNC(vocalizer_state::firq_mask_w));
	map(0x5006, 0x5007).mirror(0x0ff0).nopr(); // TODO: audio in freq/edge counter
	map(0x5006, 0x5006).mirror(0x0ff0).nopw(); // TODO: mic input gain
	map(0x5007, 0x5007).mirror(0x0ff0).w(FUNC(vocalizer_state::volume_w));
	map(0x5008, 0x5008).mirror(0x0ff0).portr("PORT8");
	map(0x5009, 0x5009).mirror(0x0f70).rw(m_lcdc, FUNC(hd44780_device::control_r), FUNC(hd44780_device::control_w));
	map(0x500a, 0x500a).mirror(0x0ff0).nopw(); // ?
	map(0x500b, 0x500b).mirror(0x0f30).w(FUNC(vocalizer_state::apo_w));
	map(0x5089, 0x5089).mirror(0x0f70).rw(m_lcdc, FUNC(hd44780_device::data_r), FUNC(hd44780_device::data_w));
	map(0x508b, 0x508b).mirror(0x0f30).w(FUNC(vocalizer_state::bank_w<0>));
	map(0x50cb, 0x50cb).mirror(0x0f30).w(FUNC(vocalizer_state::bank_w<1>));
	map(0x6000, 0x7fff).ram().share("nvram");
	map(0x8000, 0xffff).bankr("rombank");
}

void vocalizer_state::sound_map(address_map &map)
{
	map(0x00000, 0x0ffff).rom().region("es5503", 0);
	map(0x10000, 0x1ffff).nopr(); // TODO: instrument cartridge
}

//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( vocalizer )
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Drums Only")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Song (Country)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Song (Latin 2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Song (Rock 4)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Full SmartSong")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Bass & Chord")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Drums & Bass")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Song 6 / Edit Rhythm")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Multi-Track")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Song 1 / Record")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Song 2 / Stop")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Song 3 / Play")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Song 4 / Track")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Song 5 / Edit Pitch")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Cart")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Demo")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Cursor Right / Tap")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT)  PORT_NAME("Cursor Left / Delete")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN)  PORT_NAME("Cursor Down / No")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP)    PORT_NAME("Cursor Up / Yes")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tuning")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Song (Jazz)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Song (Soul 1)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Song (Rock 1)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Option")
	PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Stop/Start")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Song (Blues)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Song (Soul 2)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Song (Rock 2)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Song Variations")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Key +")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tempo +")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Ending")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Song (Reggae)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Song (Latin 1)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Song (Rock 3)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Rhythm Variations")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Key -")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tempo -")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Melody Guide")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Master Volume -")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Instrument Volume -")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Voice Volume -")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Octave -")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Harmony")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Slide")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Piano)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Spirit)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Bells)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Violin)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Sax)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Flute)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Electric Guitar)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Electric Piano)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Warp)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Marimba)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Cello)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Trumpet)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Whistle)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Fuzz Guitar)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Piano Strings)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Fusion)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Steel Drum)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Bass)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Trombone)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Clarinet)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Electric Bass)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN10")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Organ)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Sara)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Synth Drums)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Synth Strings)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Synth Brass)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Harmonica)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tone (Slap Bass)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN11")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Voice Guide")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Master Volume +")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Instrument Volume +")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Voice Volume +")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Octave +")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Echo")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Chorus")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("PORT8") // handset and power button
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_BUTTON1) PORT_NAME("Octave + (Hold)")
	PORT_BIT(0x06, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) // power
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_BUTTON2) PORT_NAME("Slide (Hold)")
	PORT_BIT(0xe0, IP_ACTIVE_LOW,  IPT_UNUSED )
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void vocalizer_state::vocalizer_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t( 63,  59,  62)); // LCD pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // LCD pixel off
}

HD44780_PIXEL_UPDATE( vocalizer_state::lcd_pixel_update )
{
	// char size is 5x8
	if (!m_power || x > 4 || y > 7)
		return;

	if (line < 2 && pos < 8)
		bitmap.pix(1 + y, 1 + line*48 + pos*6 + x) = state ? 1 : 2;
}

//**************************************************************************
void vocalizer_state::machine_start()
{
	m_bank->configure_entries(0, 2, memregion("maincpu")->base(), 0x8000);

	save_item(NAME(m_power));
	save_item(NAME(m_config));
	save_item(NAME(m_input_sel));
	save_item(NAME(m_firq_status));
	save_item(NAME(m_firq_mask));
}

//**************************************************************************
void vocalizer_state::machine_reset()
{
	m_power = 1;
	m_config = 0;
	m_input_sel = 0;
	m_firq_status = m_firq_mask = 0;
}

//**************************************************************************
template <int Shift>
u8 vocalizer_state::time_r()
{
	// used for tempo, auto power off timer, etc
	return machine().time().as_ticks(2'000'000) >> Shift;
}

//**************************************************************************
u8 vocalizer_state::uart_rx_r()
{
	return m_uart[BIT(m_config, 0)]->read();
}

//**************************************************************************
template <int Num>
void vocalizer_state::uart_rx_irq(int state)
{
	if (Num == BIT(m_config, 0))
		firq_w<FIRQ_RX>(state);
}

//**************************************************************************
u8 vocalizer_state::status_r()
{
	u8 status = 0x80;
	u8 keys = 0;

	if (m_input_sel == 0xf)
	{
		// selecting row 15 scans all rows together...
		for (int i = 0; i < m_keys.size(); i++)
			keys |= m_keys[i]->read();
		// and also inverts the ready bit, apparently
		status = 0x00;
	}
	else if (m_input_sel < m_keys.size())
	{
		keys = m_keys[m_input_sel]->read();
	}

	for (int i = 7; i >= 0; i--)
	{
		if (BIT(keys, i))
		{
			status |= i;
			status ^= 0x80;
			break;
		}
	}

	status |= (m_firq_status & 0xf) << 3;

	return status;
}

//**************************************************************************
template <int Num>
void vocalizer_state::firq_w(int state)
{
	if (state)
	{
		m_firq_status |= Num;
		firq_update();
	}
}

//**************************************************************************
void vocalizer_state::firq_ack_w(u8 data)
{
	m_firq_status &= BIT(~data, 3, 4);
	firq_update();
}

//**************************************************************************
void vocalizer_state::firq_mask_w(u8 data)
{
	m_firq_mask = BIT(data, 3, 4);
	firq_update();
}

//**************************************************************************
void vocalizer_state::firq_update()
{
	if (m_firq_status & ~m_firq_mask & 0xf)
		m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
	else
		m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
}

//**************************************************************************
void vocalizer_state::volume_w(u8 data)
{
	// TODO: increase overall gain? per-voice volume almost never goes above 0x40
	m_es5503->set_output_gain(ALL_OUTPUTS, (float)data / 255);
}

//**************************************************************************
void vocalizer_state::apo_w(u8 data)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_lcdc->reset();
	m_es5503->reset();
	m_power = 0;
}

//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void vocalizer_state::vocalizer(machine_config &config)
{
	MC6809E(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &vocalizer_state::maincpu_map);

	NVRAM(config, "nvram");

	VOCALIZER_UART(config, m_uart[0]);
	m_uart[0]->tx_irq_cb().set(FUNC(vocalizer_state::firq_w<FIRQ_TX0>));
	m_uart[0]->rx_irq_cb().set(FUNC(vocalizer_state::uart_rx_irq<0>));
	// TODO: link cable on uart 0

	VOCALIZER_UART(config, m_uart[1]);
	m_uart[1]->tx_irq_cb().set(FUNC(vocalizer_state::firq_w<FIRQ_TX1>));
	m_uart[1]->rx_irq_cb().set(FUNC(vocalizer_state::uart_rx_irq<1>));

	midi_port_device& mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set(m_uart[1], FUNC(vocalizer_uart_device::rx_w));

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");
	m_uart[1]->tx_cb().set("mdout", FUNC(midi_port_device::write_txd));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(6*16 + 1, 10);
	screen.set_visarea_full();
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(vocalizer_state::vocalizer_palette), 3);

	HD44780(config, m_lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_lcdc->set_lcd_size(2, 8);
	m_lcdc->set_pixel_update_cb(FUNC(vocalizer_state::lcd_pixel_update));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ES5503(config, m_es5503, 8_MHz_XTAL).set_channels(16);
	m_es5503->set_addrmap(0, &vocalizer_state::sound_map);
	m_es5503->irq_func().set_inputline(m_maincpu, M6809_IRQ_LINE);
	for (int i = 0; i < 16; i++)
	{
		if (i <= 8)
			m_es5503->add_route(i, "lspeaker", 1.0);
		else if (i < 15)
			m_es5503->add_route(i, "lspeaker", (15 - i) / 7.0);

		if (i >= 8)
			m_es5503->add_route(i, "rspeaker", 1.0);
		else if (i > 0)
			m_es5503->add_route(i, "rspeaker", i / 8.0);
	}
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( vocalizer )
	ROM_REGION(0x10000, "maincpu", 0) // "Version 1.5g"
	ROM_LOAD("system v-e cs=e700.u11", 0x00000, 0x10000, CRC(ead225ba) SHA1(89ebda078a98babd6513953ee29db4e31e06db83))

	ROM_REGION(0x10000, "es5503", 0)
	ROM_LOAD("waverom v-a cs=7018.u18", 0x00000, 0x10000, CRC(55567c6d) SHA1(5c67997301f4d3bdc2e5dd893b0234425eb374f0))

	ROM_REGION(0x2000, "nvram", 0)
	ROM_LOAD("init_ram.bin", 0x0000, 0x2000, CRC(f1e85f3d) SHA1(7897b488d128e044af737e7e6cbb7857d92e4891))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY                    FULLNAME          FLAGS
SYST( 1988, vocalizer, 0,      0,      vocalizer, vocalizer, vocalizer_state, empty_init, "Breakaway Music Systems", "Vocalizer 1000", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING | MACHINE_NODEVICE_MICROPHONE )
