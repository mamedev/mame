// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

    Zenith Z-39 video terminal.

    Skeleton driver, based on the Z-39 Technical Training manual (TT-27,
    585-169). No ROMs have been dumped.

    The terminal logic board (HE-181-5193) is a close relative of the Z-29
    board. An 8031 (U433) runs the terminal, with two 8276 CRT controllers:
    U418 supplies the character codes and U419 the matching attributes. As
    on the Z-29 there is no DMA controller; the 8031 fills the row buffers
    by executing from the "DMA memory" window, with P3.4 selecting between
    loading the CRTCs and clearing display memory. Setup is kept in an X2210
    NVRAM (U439).

    The manual's memory map figure uses 8K blocks decoded by U423, but parts
    of the text repeat Z-29 addresses (latch at 12K, DMA memory and NVRAM at
    28K). This driver follows the figure:

    0000-1fff  expansion socket EPROM (internal ROM on later 8051 boards)
    2000-3fff  external program firmware
    4000-5fff  CRT controller registers
    6000-7fff  control latch U448
    8000-9fff  attribute display memory (2114 x2, U437/U438)
    a000-bfff  character display memory (6116, U436)
    c000-dfff  DMA memory
    e000-ffff  NVRAM (X2210, U439)

    The keyboard is a separate unit with its own 8021 (3.579 MHz), an 8x11
    key matrix, the status LEDs and the beeper. It talks to the terminal
    over a serial link on P1.0/P1.1. SHIFT+RESET resets the terminal.

    TODO:
    - dump the program ROMs, character ROM and keyboard 8021
    - confirm the memory map against a real board or the schematics
    - keyboard (8021) emulation
    - blind print pass-through (control latch bit 4, U432)
    - 50 Hz mode
    - the Z-29 holds INT0 for a whole row, because the MCS-51 core latches
      the BRDY glitches between characters as extra interrupts; this may
      need the same workaround once the firmware can be tested

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/mcs51/i8051.h"
#include "machine/x2212.h"
#include "video/i8275.h"

#include "screen.h"


namespace {

class z39_state : public driver_device
{
public:
	z39_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc%u", 1U)
		, m_nvram(*this, "nvram")
		, m_serial(*this, { "dte", "dce" })
		, m_charmem(*this, "charmem")
		, m_attrmem(*this, "attrmem", 0x800, ENDIANNESS_LITTLE)
		, m_chargen(*this, "chargen")
		, m_config(*this, "CONFIG")
		, m_p1(0xff)
		, m_dmatype(true)
		, m_blink(false)
		, m_aux_port(false)
		, m_passthru(true)
		, m_txd(true)
		, m_rxd{ true, true }
		, m_cts{ false, false }
	{
	}

	void z39(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	template <u32 N> void rxd_w(int state) { m_rxd[N] = state; }
	template <u32 N> void cts_w(int state) { m_cts[N] = state; }
	void update_txd();
	u8 p1_r();
	void p1_w(u8 data);
	u8 p3_r();
	void p3_w(u8 data);
	u8 dma_r(offs_t offset);
	u8 attr_r(offs_t offset);
	void attr_w(offs_t offset, u8 data);
	void crtc_w(offs_t offset, u8 data);
	void latch_w(u8 data);

	I8275_DRAW_CHARACTER_MEMBER(draw_character);

	void prg_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_maincpu;
	required_device_array<i8276_device, 2> m_crtc;
	required_device<x2210_device> m_nvram;
	required_device_array<rs232_port_device, 2> m_serial; // computer port (DTE), auxiliary port (DCE)

	required_shared_ptr<u8> m_charmem;
	memory_share_creator<u8> m_attrmem;
	required_region_ptr<u8> m_chargen;
	required_ioport m_config;

	u8 m_p1;
	bool m_dmatype;
	bool m_blink;
	bool m_aux_port;
	bool m_passthru;
	bool m_txd;
	bool m_rxd[2];
	bool m_cts[2];
};

void z39_state::machine_start()
{
	save_item(NAME(m_p1));
	save_item(NAME(m_dmatype));
	save_item(NAME(m_blink));
	save_item(NAME(m_aux_port));
	save_item(NAME(m_passthru));
	save_item(NAME(m_txd));
	save_item(NAME(m_rxd));
	save_item(NAME(m_cts));

	// p1_w only passes on changes, so set the initial line states here
	m_serial[0]->write_dtr(BIT(m_p1, 4));
	m_serial[0]->write_rts(BIT(m_p1, 5));
	update_txd();
}

/**
 * Port 1 (from the service manual)
 *
 * bit 0 - keyboard handshake (output)
 * bit 1 - keyboard data (input)
 * bit 2 - 0 = normal video       1 = suppressed
 * bit 3 - setup mode lock jumper
 * bit 4 - Data Terminal Ready
 * bit 5 - Request to Send ("CTS" in the manual)
 * bit 6 - RTS from the auxiliary port in blind print mode
 * bit 7 - initial synchronization of the video counter (U403)
 *
 * The manual says a jumper to +5V locks setup; on the Z-29 the firmware
 * shows the jumper actually pulls the bit low, so assume the same here.
 */
u8 z39_state::p1_r()
{
	// TODO: keyboard data on bit 1; idle (mark) until the keyboard is emulated
	return m_config->read() | ~0x08;
}

void z39_state::p1_w(u8 data)
{
	u8 const changed = m_p1 ^ data;
	m_p1 = data;

	// only the computer port has DTR and RTS
	if (BIT(changed, 4))
		m_serial[0]->write_dtr(BIT(data, 4));
	if (BIT(changed, 5))
		m_serial[0]->write_rts(BIT(data, 5));
}

/**
 * Port 3 (from the service manual)
 *
 * bit 0 - serial port receive data
 * bit 1 - serial port transmit data
 * bit 2 - /INT0, buffer ready from the character CRTC (inverted by U401F)
 * bit 3 - /INT1, interrupt from the character CRTC
 * bit 4 - 0 = clear memory  1 = DMA CRTC (jams 20h instead of the DMA opcode)
 * bit 5 - RTS input from the selected port
 * bit 6 - /WR
 * bit 7 - /RD
 *
 * Multiplexer U431 connects RXD and RTS to the port selected by the control latch.
 */
u8 z39_state::p3_r()
{
	return 0xde | (m_cts[m_aux_port] ? 0x20 : 0) | (m_rxd[m_aux_port] ? 0x01 : 0);
}

void z39_state::p3_w(u8 data)
{
	m_dmatype = BIT(data, 4);

	if (m_txd != BIT(data, 1))
	{
		m_txd = BIT(data, 1);
		update_txd();
	}
}

// U443C and U443D hold the transmit line of the deselected port at mark
void z39_state::update_txd()
{
	m_serial[0]->write_txd(m_txd || m_aux_port);
	m_serial[1]->write_txd(m_txd || !m_aux_port);
}

// program fetches from the DMA memory window load both CRTCs (or clear the cell) and return a jammed opcode
u8 z39_state::dma_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (m_dmatype)
		{
			u8 const chardata = m_charmem[offset];
			u8 const attrdata = m_attrmem[offset];

			m_crtc[0]->dack_w(chardata & 0x7f);
			m_crtc[1]->dack_w((chardata & 0x60) | (BIT(chardata, 7) ? 0x10 : 0) | attrdata);
		}
		else
		{
			m_charmem[offset] = 0x20;
			m_attrmem[offset] = 0;
		}
	}

	// TODO: the manual only says the clear opcode is 20h; 24h for DMA is assumed from the Z-29
	return m_dmatype ? 0x24 : 0x20;
}

// the attribute RAM is two 2114s, so only four bits are stored and the upper four float
u8 z39_state::attr_r(offs_t offset)
{
	return m_attrmem[offset] | 0xf0;
}

void z39_state::attr_w(offs_t offset, u8 data)
{
	m_attrmem[offset] = data & 0x0f;
}

void z39_state::crtc_w(offs_t offset, u8 data)
{
	m_crtc[0]->write(offset, data);
	m_crtc[1]->write(offset, data);
}

/**
 * Control latch U448 (74LS174)
 *
 * bit 0 - EAROMSTORE: store setup to the NVRAM
 * bit 1 - B1: blank characters with the blink attribute
 * bit 2 - PRT CNTL: 0 = main port  1 = auxiliary port (U448 pin 7)
 * bit 3 - EAROMRECALL: recall setup from the NVRAM
 * bit 4 - PASSTHRU: 0 = blind print pass-through enabled (U448 pin 12)
 *
 * Bit order follows the manual's list, which agrees with the two pins it
 * names. The store and recall polarity is assumed from the Z-29 (active low);
 * the manual says "when this bit is set".
 */
void z39_state::latch_w(u8 data)
{
	m_nvram->store(!BIT(data, 0));
	m_blink = BIT(data, 1);
	m_nvram->recall(!BIT(data, 3));
	m_passthru = BIT(data, 4);

	if (m_aux_port != BIT(data, 2))
	{
		m_aux_port = BIT(data, 2);
		update_txd();
	}
}

/**
 * The character CRTC (U418) supplies the character code, and the attribute
 * CRTC (U419) supplies, for the same position (assumed to match the Z-29):
 *
 * bit 0 - reverse video
 * bit 1 - underline
 * bit 2 - blink
 * bit 3 - half intensity
 * bit 4 - character bit 7, selects the upper 2K of the character ROM
 *
 * VSP isn't used, so graphics characters can join from one row to the next.
 */
I8275_DRAW_CHARACTER_MEMBER(z39_state::draw_character)
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

	// the blink also turns off the underline (tenth scan line)
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

void z39_state::prg_map(address_map &map)
{
	map(0x0000, 0x0fff).mirror(0x1000).rom().region("maincpu", 0x0000);
	map(0x2000, 0x2fff).mirror(0x1000).rom().region("maincpu", 0x1000);
	map(0xc000, 0xc7ff).mirror(0x1800).r(FUNC(z39_state::dma_r));
}

void z39_state::ext_map(address_map &map)
{
	map(0x4000, 0x4001).mirror(0x1ffe).r(m_crtc[0], FUNC(i8276_device::read)).w(FUNC(z39_state::crtc_w));
	map(0x6000, 0x6000).mirror(0x1fff).w(FUNC(z39_state::latch_w));
	map(0x8000, 0x87ff).mirror(0x1800).rw(FUNC(z39_state::attr_r), FUNC(z39_state::attr_w));
	map(0xa000, 0xa7ff).mirror(0x1800).ram().share(m_charmem);
	map(0xe000, 0xe03f).mirror(0x1fc0).rw(m_nvram, FUNC(x2210_device::read), FUNC(x2210_device::write));
}


static INPUT_PORTS_START(z39)
	PORT_START("CONFIG")
	PORT_CONFNAME(0x08, 0x08, "Setup Mode")
	PORT_CONFSETTING(0x00, "Locked")
	PORT_CONFSETTING(0x08, "Unlocked")
INPUT_PORTS_END


void z39_state::z39(machine_config &config)
{
	I8031(config, m_maincpu, 14.784_MHz_XTAL / 2); // U433
	m_maincpu->set_addrmap(AS_PROGRAM, &z39_state::prg_map);
	m_maincpu->set_addrmap(AS_DATA, &z39_state::ext_map);
	m_maincpu->port_in_cb<1>().set(FUNC(z39_state::p1_r));
	m_maincpu->port_out_cb<1>().set(FUNC(z39_state::p1_w));
	m_maincpu->port_in_cb<3>().set(FUNC(z39_state::p3_r));
	m_maincpu->port_out_cb<3>().set(FUNC(z39_state::p3_w));

	X2210(config, m_nvram); // U439

	// P402 computer port (DTE)
	RS232_PORT(config, m_serial[0], default_rs232_devices, "loopback");
	m_serial[0]->rxd_handler().set(FUNC(z39_state::rxd_w<0>));
	m_serial[0]->cts_handler().set(FUNC(z39_state::cts_w<0>));

	// P401 auxiliary port (DCE)
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);
	m_serial[1]->rxd_handler().set(FUNC(z39_state::rxd_w<1>));
	m_serial[1]->cts_handler().set(FUNC(z39_state::cts_w<1>));

	// 14.784 MHz gives exactly 60 Hz with 110 characters by 28 rows of 10 lines
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(14.784_MHz_XTAL, 880, 0, 640, 280, 0, 250);
	screen.set_screen_update(m_crtc[0], FUNC(i8276_device::screen_update));

	I8276(config, m_crtc[0], 14.784_MHz_XTAL / 8).set_character_width(8); // U418
	m_crtc[0]->set_display_callback(FUNC(z39_state::draw_character));
	m_crtc[0]->set_next_crtc(m_crtc[1]);
	m_crtc[0]->drq_wr_callback().set_inputline(m_maincpu, MCS51_INT0_LINE); // inverted by U401F onto /INT0
	m_crtc[0]->irq_wr_callback().set_inputline(m_maincpu, MCS51_INT1_LINE);

	I8276(config, m_crtc[1], 14.784_MHz_XTAL / 8).set_character_width(8); // U419
}


// ROM sizes are a guess; the block diagram shows two 4K program ROMs
ROM_START(z39)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("444-296.u435", 0x0000, 0x1000, NO_DUMP) // Terminal ROM 1, expansion socket
	ROM_LOAD("444-297.u434", 0x1000, 0x1000, NO_DUMP) // Terminal ROM 2

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("444-248.u416", 0x0000, 0x1000, NO_DUMP) // domestic and foreign sets, selected by A11

	ROM_REGION(0x400, "keyboard", 0)
	ROM_LOAD("8021.u1", 0x000, 0x400, NO_DUMP) // keyboard 8021, not hooked up
ROM_END

} // anonymous namespace

//    year  name  parent compat  machine  input class      init        company                fullname         flags
COMP( 198?, z39,  0,     0,      z39,     z39,  z39_state, empty_init, "Zenith Data Systems", "Z-39 Terminal", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE)
