// license:BSD-3-Clause
// copyright-holders:AJR, Mark Garlanger
/***************************************************************************

    Zenith Z-29 (alias Heathkit H-29) video terminal.

    An 8031 runs the terminal, with two 8276 CRT controllers: U422 supplies
    the character codes and U446 the matching attributes. There is no DMA
    controller. The row buffers are filled by having the 8031 execute from
    the "DMA memory" window at 24K: each program fetch there reads one cell
    of display memory and writes it to both CRTCs, while the CPU sees a
    harmless jammed opcode. With P3.4 low (DMATYPE, "0 = clear memory,
    1 = DMA CRTC" in the ROM listing) the fetch instead clears the cell,
    which the firmware uses to erase rows, stopping the run with a timer
    interrupt.

    The keyboard is a separate 8021 connected through a bit-banged serial
    link. Setup is kept in an X2210 NVRAM.

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "bus/z29_kbd/keyboard.h"
#include "cpu/mcs51/i8051.h"
#include "machine/x2212.h"
#include "video/i8275.h"
#include "screen.h"

#include <iterator>


namespace {

class z29_state : public driver_device
{
public:
	z29_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_keyboard(*this, "keyboard")
		, m_crtc(*this, "crtc%u", 1U)
		, m_nvram(*this, "nvram")
		, m_serial(*this, { "dte", "dce" })
		, m_charmem(*this, "charmem")
		, m_attrmem(*this, "attrmem")
		, m_chargen(*this, "chargen")
		, m_config(*this, "CONFIG")
		, m_p1(0xff)
		, m_dmatype(true)
		, m_brdy(false)
		, m_row_chars(ROW_CHARS)
		, m_keyin(true)
		, m_blink(false)
		, m_aux_port(false)
		, m_txd(true)
		, m_rxd{ true, true }
		, m_cts{ false, false }
	{
	}

	void z29(machine_config &config);

	void init_z29() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// the firmware programs the CRTCs for 80 characters per row
	static constexpr u8 ROW_CHARS = 80;

	void keyin_w(int state);
	template <unsigned N> void rxd_w(int state) { m_rxd[N] = state; }
	template <unsigned N> void cts_w(int state) { m_cts[N] = state; }
	void update_txd();
	u8 p1_r();
	void p1_w(u8 data);
	u8 p3_r();
	void p3_w(u8 data);
	void brdy_w(int state);
	void crtc_irq_w(int state);
	void update_int0();
	u8 bs_24k_r(offs_t offset);
	void crtc_w(offs_t offset, u8 data);
	void latch_12k_w(u8 data);

	I8275_DRAW_CHARACTER_MEMBER(draw_character);

	void prg_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<z29_keyboard_port_device> m_keyboard;
	required_device_array<i8276_device, 2> m_crtc;
	required_device<x2210_device> m_nvram;
	required_device_array<rs232_port_device, 2> m_serial; // computer port (DTE), auxiliary port (DCE)

	required_shared_ptr<u8> m_charmem;
	required_shared_ptr<u8> m_attrmem;
	required_region_ptr<u8> m_chargen;
	required_ioport m_config;

	u8 m_p1;
	bool m_dmatype;
	bool m_brdy;
	u8 m_row_chars;
	bool m_keyin;
	bool m_blink;
	bool m_aux_port;
	bool m_txd;
	bool m_rxd[2];
	bool m_cts[2];
};

void z29_state::machine_start()
{
	save_item(NAME(m_p1));
	save_item(NAME(m_dmatype));
	save_item(NAME(m_brdy));
	save_item(NAME(m_row_chars));
	save_item(NAME(m_keyin));
	save_item(NAME(m_blink));
	save_item(NAME(m_aux_port));
	save_item(NAME(m_txd));
	save_item(NAME(m_rxd));
	save_item(NAME(m_cts));
}

void z29_state::machine_reset()
{
	// with nothing connected, pull-ups R404 and R401 hold the handshake inputs asserted
	for (int i = 0; i < 2; i++)
	{
		if (!m_serial[i]->get_card_device())
			m_cts[i] = false;
	}

	// p1_w only passes on changes, so bring the outputs in line with the latch
	m_keyboard->keyout_w(!BIT(m_p1, 0));
	m_serial[0]->write_dtr(BIT(m_p1, 4));
	m_serial[0]->write_rts(BIT(m_p1, 5));
	update_txd();
}

void z29_state::keyin_w(int state)
{
	m_keyin = state;
}

/**
 * Port 1 (based on ROM listing and service manual)
 *
 * bit 0 - KB output line
 * bit 1 - KB input line
 * bit 2 - 0 = normal video       1 = suppressed
 * bit 3 - 0 = setup mode locked  1 = normal
 * bit 4 - Data Terminal Ready
 * bit 5 - Ready to Send
 * bit 6 - Expansion socket
 * bit 7 - Clock run for CRT controller
 *
 * The service manual says a jumper to +5V locks setup, but the firmware only
 * enters setup when bit 3 reads 1, so the jumper must pull it low.
 */
u8 z29_state::p1_r()
{
	return (m_keyin ? 0xfd : 0xff) & (m_config->read() | ~0x08);
}

void z29_state::p1_w(u8 data)
{
	u8 const changed = m_p1 ^ data;
	m_p1 = data;

	if (BIT(changed, 0))
		m_keyboard->keyout_w(!BIT(data, 0));

	// only the computer port has DTR and RTS
	if (BIT(changed, 4))
		m_serial[0]->write_dtr(BIT(data, 4));
	if (BIT(changed, 5))
		m_serial[0]->write_rts(BIT(data, 5));
}

/**
 * Port 3 (based on ROM listing)
 *
 * bit 0 - serial port receive pin
 * bit 1 - predefined serial port transmit pin
 * bit 4 - 0 = clear memory  1 = DMA CRTC
 * bit 5 - Clear to Send
 *
 * Multiplexer U404 connects RXD and CTS to the port selected by the 12K latch.
 * On the auxiliary port, CTS comes from the RTS input.
 */
u8 z29_state::p3_r()
{
	return 0xde | (m_cts[m_aux_port] ? 0x20 : 0) | (m_rxd[m_aux_port] ? 0x01 : 0);
}

void z29_state::p3_w(u8 data)
{
	m_dmatype = BIT(data, 4);

	if (m_txd != BIT(data, 1))
	{
		m_txd = BIT(data, 1);
		update_txd();
	}
}

// U402C and U402D hold the transmit line of the deselected port at mark
void z29_state::update_txd()
{
	m_serial[0]->write_txd(m_txd || m_aux_port);
	m_serial[1]->write_txd(m_txd || !m_aux_port);
}

/**
 * The CRTC is programmed for one character per buffer load with no delay, so
 * BRDY drops for a moment after each character written. The firmware fills a
 * row with two characters per machine cycle, and the 8031 only samples /INT0
 * once a cycle, so it sees a single request per row.
 *
 * HACK: the MCS-51 core latches every falling edge on INT0 instead of
 * sampling the pin once per machine cycle, so INT0 is held for the whole row
 * (ROW_CHARS fetches) to keep the glitches from latching extra interrupts. If
 * a row is cut short, crtc_irq_w releases the hold at the frame interrupt.
 * This can go once the CPU core models INT0 sampling.
 */
void z29_state::brdy_w(int state)
{
	if (state && (m_row_chars >= ROW_CHARS))
		m_row_chars = 0;

	m_brdy = state;
	update_int0();
}

// the frame interrupt comes between row transfers, so use it to recover if a row was cut short
void z29_state::crtc_irq_w(int state)
{
	if (state && !m_brdy)
	{
		m_row_chars = ROW_CHARS;
		update_int0();
	}

	m_maincpu->set_input_line(MCS51_INT1_LINE, state);
}

void z29_state::update_int0()
{
	m_maincpu->set_input_line(MCS51_INT0_LINE, (m_brdy || (m_row_chars < ROW_CHARS)) ? ASSERT_LINE : CLEAR_LINE);
}

u8 z29_state::bs_24k_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (m_dmatype)
		{
			u8 chardata = m_charmem[offset];
			u8 attrdata = m_attrmem[offset] & 0xf;

			if (m_row_chars < ROW_CHARS)
				m_row_chars++;

			m_crtc[0]->dack_w(chardata & 0x7f);
			m_crtc[1]->dack_w((chardata & 0x60) | (BIT(chardata, 7) ? 0x10 : 0) | attrdata);
		}
		else
		{
			m_charmem[offset] = 0x20;
			m_attrmem[offset] = 0;
		}
	}

	return m_dmatype ? 0x24 : 0x20;
}

void z29_state::crtc_w(offs_t offset, u8 data)
{
	m_crtc[0]->write(offset, data);
	m_crtc[1]->write(offset, data);
}

/**
 * Control latch at 12K (*XLATCH* in the ROM listing)
 *
 * bit 0 - 0 = store setup to EPROM
 * bit 1 - 1 = blank characters with the blink attribute
 * bit 2 - 0 = main port      1 = auxiliary port
 * bit 3 - 0 = recall setup from EPROM
 */
void z29_state::latch_12k_w(u8 data)
{
	m_nvram->store(!BIT(data, 0));
	m_blink = BIT(data, 1);
	m_nvram->recall(!BIT(data, 3));

	if (m_aux_port != BIT(data, 2))
	{
		m_aux_port = BIT(data, 2);
		update_txd();
	}
}

/**
 * The character CRTC (U422) supplies the character code, and the attribute
 * CRTC (U446) supplies, for the same position:
 *
 * bit 0 - reverse video
 * bit 1 - underline
 * bit 2 - blink
 * bit 3 - half intensity
 * bit 4 - character bit 7, selects the upper half of the character ROM
 *
 * VSP isn't used, so graphics characters can join from one row to the next.
 */
I8275_DRAW_CHARACTER_MEMBER(z29_state::draw_character)
{
	using namespace i8275_attributes;

	// P1.2 blanks everything, for the screen saver and the unused rows at 50 Hz
	u32 *const pix = &bitmap.pix(y, x);

	if (BIT(m_p1, 2))
	{
		for (int i = 0; i < 8; i++)
			pix[i] = rgb_t::black();
		return;
	}

	u8 const attr = BIT(charcode, 8, 8);
	bool const blanked = BIT(attr, 2) && m_blink;

	u8 gfx = m_chargen[(BIT(attr, 4) << 11) | ((charcode & 0x7f) << 4) | linecount];

	// the blink also turns off the underline
	if (blanked)
		gfx = 0;
	else if (BIT(attr, 1) && (linecount == 9))
		gfx = 0xff;

	// cursor, from LTEN and RVV of the character CRTC
	if (BIT(attrcode, LTEN) || BIT(attrcode, RVV))
		gfx ^= 0xff;

	if (BIT(attr, 0))
		gfx ^= 0xff;

	rgb_t const on = BIT(attr, 3) ? rgb_t(0x00, 0x80, 0x00) : rgb_t(0x00, 0xff, 0x00);

	for (int i = 0; i < 8; i++)
		pix[i] = BIT(gfx, 7 - i) ? on : rgb_t::black();
}

void z29_state::prg_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
	map(0x6000, 0x67ff).mirror(0x800).r(FUNC(z29_state::bs_24k_r));
}

void z29_state::ext_map(address_map &map)
{
	map(0x2000, 0x2001).mirror(0xffe).r("crtc1", FUNC(i8276_device::read)).w(FUNC(z29_state::crtc_w));
	map(0x3000, 0x3000).mirror(0xfff).w(FUNC(z29_state::latch_12k_w));
	map(0x4000, 0x47ff).mirror(0x800).ram().share("attrmem");
	map(0x5000, 0x57ff).mirror(0x800).ram().share("charmem");
	map(0x7000, 0x703f).mirror(0xfc0).rw("nvram", FUNC(x2210_device::read), FUNC(x2210_device::write));
}


static INPUT_PORTS_START(z29)
	PORT_START("CONFIG")
	PORT_CONFNAME(0x08, 0x08, "Setup Mode")
	PORT_CONFSETTING(0x00, "Locked")
	PORT_CONFSETTING(0x08, "Unlocked")
INPUT_PORTS_END


void z29_state::z29(machine_config &config)
{
	I8031(config, m_maincpu, 14.784_MHz_XTAL / 2); // Intel P8031AH
	m_maincpu->set_addrmap(AS_PROGRAM, &z29_state::prg_map);
	m_maincpu->set_addrmap(AS_DATA, &z29_state::ext_map);
	m_maincpu->port_in_cb<1>().set(FUNC(z29_state::p1_r));
	m_maincpu->port_out_cb<1>().set(FUNC(z29_state::p1_w));
	m_maincpu->port_in_cb<3>().set(FUNC(z29_state::p3_r));
	m_maincpu->port_out_cb<3>().set(FUNC(z29_state::p3_w));

	// the keyboard link is bit-banged, and each bit is sampled about 200 us after it is clocked
	config.set_maximum_quantum(attotime::from_usec(20));

	X2210(config, m_nvram);

	// P402 computer port (DTE)
	RS232_PORT(config, m_serial[0], default_rs232_devices, "loopback");
	m_serial[0]->rxd_handler().set(FUNC(z29_state::rxd_w<0>));
	m_serial[0]->cts_handler().set(FUNC(z29_state::cts_w<0>));

	// P401 auxiliary port (DCE)
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);
	m_serial[1]->rxd_handler().set(FUNC(z29_state::rxd_w<1>));
	m_serial[1]->cts_handler().set(FUNC(z29_state::cts_w<1>));

	Z29_KEYBOARD(config, m_keyboard, z29_keyboards, "heath");
	m_keyboard->keyin_callback().set(FUNC(z29_state::keyin_w));
	m_keyboard->reset_callback().set_inputline(m_maincpu, INPUT_LINE_RESET).invert();

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(14.784_MHz_XTAL, 880, 0, 640, 280, 0, 250);
	screen.set_screen_update("crtc1", FUNC(i8276_device::screen_update));

	I8276(config, m_crtc[0], 14.784_MHz_XTAL / 8).set_character_width(8);
	m_crtc[0]->set_display_callback(FUNC(z29_state::draw_character));
	m_crtc[0]->set_next_crtc(m_crtc[1]);
	m_crtc[0]->drq_wr_callback().set(FUNC(z29_state::brdy_w));
	m_crtc[0]->irq_wr_callback().set(FUNC(z29_state::crtc_irq_w));

	I8276(config, m_crtc[1], 14.784_MHz_XTAL / 8).set_character_width(8);
}


ROM_START(z29) // All EPROMs on 85-2835-1 terminal board are HN482732AG-30
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("u440.bin", 0x0000, 0x1000, CRC(169b9517) SHA1(c18b6a193655a64808e9ae8765d3e54d13e6669e)) // occupies pins 9-32 of 40-pin expansion socket
	ROM_LOAD("u407.bin", 0x1000, 0x1000, CRC(b5aae8e6) SHA1(692e521a85d7e07647c66a660faa2041d1bfd785))

	ROM_REGION(0x40, "nvram", ROMREGION_ERASE00) // default setup, filled in by init_z29

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("u429.bin", 0x0000, 0x1000, CRC(5e3bc5bf) SHA1(18d73e3d74a9768bee8b063ea45891f955558ae7))
ROM_END

/**
 * Default setup for the X2210 NVRAM
 *
 * The firmware keeps the setup in internal RAM at 25h-34h and stores it in
 * the first 32 nibbles of the NVRAM, high nibble first. The last byte is the
 * sum of the others. The settings are the sample menus shown in the User's
 * and Technical Guide, except that the terminal is on line.
 */
void z29_state::init_z29()
{
	static constexpr u8 SETUP[15] = {
		0xc8,   // 25h: full duplex, on line, auto wrap, space parity
		0x9c,   // 26h: status line, auto repeat, key click, underline cursor
		0x0a,   // 27h: software handshake, main port, screen saver, 60 Hz, blinking cursor
		0x02,   // 28h: Zenith mode
		0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, // 29h-32h: tab every 8 columns
		0xfe }; // 33h: 9600 baud (timer 1 reload)

	u8 *const nvram = memregion("nvram")->base();
	u8 sum = 0;
	for (unsigned i = 0; i < std::size(SETUP); i++)
	{
		nvram[i * 2] = SETUP[i] >> 4;
		nvram[i * 2 + 1] = SETUP[i] & 0x0f;
		sum += SETUP[i];
	}
	nvram[30] = sum >> 4;
	nvram[31] = sum & 0x0f;
}

} // anonymous namespace

//    year  name   parent compat  machine  input class      init      company                fullname         flags
COMP( 1983, z29,   0,     0,      z29,     z29,  z29_state, init_z29, "Zenith Data Systems", "Z-29 Terminal", MACHINE_SUPPORTS_SAVE)
