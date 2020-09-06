// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Informer 207/376

    IBM 3270 compatible terminal

    Hardware:
    - M6809
    - Z80SCC 8530
    - 6850 ACIA (up to 4)
    - X2212P NVRAM

    TODO:
	- Redump bad ROMs
	- Dump keyboard controller and emulate it (currently HLE'd)
	- Problably needs improvements to at least the Z80SCC to
	  properly support synchrous modes
    - Figure out unknown reads/writes to the memory
	- Verify NMI hookup
	- Verify clock speeds

    Notes:
    - Thanks to charcole and his zmachine3270 project at
      https://github.com/charcole/zmachine3270

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/input_merger.h"
#include "machine/nvram.h"
#include "machine/z80scc.h"
#include "video/mc6845.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/printer.h"
#include "machine/informer_207_376_kbd.h"
#include "emupal.h"
#include "screen.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class informer_207_376_state : public driver_device
{
public:
	informer_207_376_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_crtc(*this, "crtc"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_scc(*this, "scc"),
		m_acia(*this, "acia%u", 0U),
		m_ram(*this, "ram"),
		m_chargen(*this, "chargen"),
		m_nmi_enabled(false)
	{ }

	void informer_207_376(machine_config &config);

protected:
	void machine_start() override;
	void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<scc85c30_device> m_scc;
	required_device_array<acia6850_device, 2> m_acia;
	required_shared_ptr<uint8_t> m_ram;
	required_region_ptr<uint8_t> m_chargen;

	void mem_map(address_map &map);

	MC6845_UPDATE_ROW(crtc_update_row);

	void vsync_w(int state);
	void nmi_control_w(uint8_t data);

	bool m_nmi_enabled;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void informer_207_376_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).ram().share("ram");
	map(0x8000, 0x8000).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x8001, 0x8001).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x8400, 0x8400).lr8(NAME([] () { return 0xff; })).nopw(); // ?
	map(0x8802, 0x8803).rw(m_acia[0], FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x8804, 0x8805).rw(m_acia[1], FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x8c00, 0x8c00).w(FUNC(informer_207_376_state::nmi_control_w));
	map(0x9400, 0x9403).rw(m_scc, FUNC(scc85c30_device::ab_dc_r), FUNC(scc85c30_device::ab_dc_w));
	map(0x9c00, 0x9cff).ram().share("nvram");
	map(0xa000, 0xffff).rom().region("maincpu", 0);
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( informer_207_376 )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void informer_207_376_state::vsync_w(int state)
{
	if (m_nmi_enabled && state == 1)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
}

MC6845_UPDATE_ROW( informer_207_376_state::crtc_update_row )
{
	const pen_t *pen = m_palette->pens();

	for (int x = 0; x < x_count; x++)
	{
		uint8_t code = m_ram[ma + x];
		uint8_t data = m_chargen[(code << 4) + ra];

		if (x == cursor_x)
			data = 0xff;

		// draw 8 pixels of the character
		bitmap.pix32(y, x * 8 + 7) = pen[BIT(data, 0)];
		bitmap.pix32(y, x * 8 + 6) = pen[BIT(data, 1)];
		bitmap.pix32(y, x * 8 + 5) = pen[BIT(data, 2)];
		bitmap.pix32(y, x * 8 + 4) = pen[BIT(data, 3)];
		bitmap.pix32(y, x * 8 + 3) = pen[BIT(data, 4)];
		bitmap.pix32(y, x * 8 + 2) = pen[BIT(data, 5)];
		bitmap.pix32(y, x * 8 + 1) = pen[BIT(data, 6)];
		bitmap.pix32(y, x * 8 + 0) = pen[BIT(data, 7)];
	}
}

static const gfx_layout char_layout =
{
	8,11,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8 },
	8*16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void informer_207_376_state::nmi_control_w(uint8_t data)
{
	// 76543---  unused?
	// -----21-  nmi enable?
	// -------0  unused?

	m_nmi_enabled = bool(data & 0x06);
}

void informer_207_376_state::machine_start()
{
	// register for save states
	save_item(NAME(m_nmi_enabled));
}

void informer_207_376_state::machine_reset()
{
	m_nmi_enabled = false;
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void printer_devices(device_slot_interface &device)
{
	device.option_add("printer", SERIAL_PRINTER);
}

void informer_207_376_state::informer_207_376(machine_config &config)
{
	MC6809(config, m_maincpu, 36_MHz_XTAL / 4); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &informer_207_376_state::mem_map);

	input_merger_device &cpu_irq(INPUT_MERGER_ANY_HIGH(config, "cpu_irq"));
	cpu_irq.output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // 2x X2212P

	SCC85C30(config, m_scc, 0); // unknown clock
	m_scc->out_txda_callback().set("com1", FUNC(rs232_port_device::write_txd));
	m_scc->out_dtra_callback().set("com1", FUNC(rs232_port_device::write_dtr));
	m_scc->out_rtsa_callback().set("com1", FUNC(rs232_port_device::write_rts));
	m_scc->out_txdb_callback().set("com2", FUNC(rs232_port_device::write_txd));
	m_scc->out_dtrb_callback().set("com2", FUNC(rs232_port_device::write_dtr));
	m_scc->out_rtsb_callback().set("com2", FUNC(rs232_port_device::write_rts));
	m_scc->out_int_callback().set("cpu_irq", FUNC(input_merger_device::in_w<0>));

	ACIA6850(config, m_acia[0], 0); // unknown clock
	m_acia[0]->txd_handler().set("kbd", FUNC(informer_207_376_kbd_hle_device::rx_w));
	m_acia[0]->irq_handler().set("cpu_irq", FUNC(input_merger_device::in_w<1>));

	ACIA6850(config, m_acia[1], 0); // unknown clock
	m_acia[1]->txd_handler().set("printer", FUNC(rs232_port_device::write_txd));

	clock_device &acia_clock(CLOCK(config, "acia_clock", 153600)); // source?
	acia_clock.signal_handler().set(m_acia[0], FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia[0], FUNC(acia6850_device::write_rxc));
	acia_clock.signal_handler().append(m_acia[1], FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia[1], FUNC(acia6850_device::write_rxc));

	rs232_port_device &com1(RS232_PORT(config, "com1", default_rs232_devices, nullptr));
	com1.rxd_handler().set(m_scc, FUNC(scc85c30_device::rxa_w));
	com1.dcd_handler().set(m_scc, FUNC(scc85c30_device::dcda_w));
	com1.cts_handler().set(m_scc, FUNC(scc85c30_device::ctsa_w));

	rs232_port_device &com2(RS232_PORT(config, "com2", default_rs232_devices, nullptr));
	com2.rxd_handler().set(m_scc, FUNC(scc85c30_device::rxb_w));
	com2.dcd_handler().set(m_scc, FUNC(scc85c30_device::dcdb_w));
	com2.cts_handler().set(m_scc, FUNC(scc85c30_device::ctsb_w));

	RS232_PORT(config, "printer", printer_devices, nullptr);

	informer_207_376_kbd_hle_device &kbd(INFORMER_207_376_KBD_HLE(config, "kbd"));
	kbd.tx_handler().set(m_acia[0], FUNC(acia6850_device::write_rxd));

	// video
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::green());
	m_screen->set_raw(14400000, 800, 0, 640, 300, 0, 286); // unknown clock
	m_screen->set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	MC6845(config, m_crtc, 1800000); // unknown clock
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(informer_207_376_state::crtc_update_row));
	m_crtc->out_vsync_callback().set(FUNC(informer_207_376_state::vsync_w));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( in207376 )
	ROM_REGION(0x6000, "maincpu", 0)
	// 79590-023  376SNA V1.00  201C 7224  (our checksum: 7264)
	ROM_LOAD("79590-023.bin", 0x0000, 0x2000, BAD_DUMP CRC(ed4ff488) SHA1(fdd95c520d0288ea483b5d2e8e6c8eecb04063c1))
	// 79589-023  376 SNAV1.00  201C E86F  (checksum matches)
	ROM_LOAD("79589-023.bin", 0x2000, 0x4000, CRC(cdfaf629) SHA1(1f21ef6848020726ef3d7ab05166ac8590d58476))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("chargen.bin", 0x0000, 0x1000, BAD_DUMP CRC(819e4b4e) SHA1(4937e49b2d91af993e8f65f5df1d6f729e775fc4))
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT   COMPAT  MACHINE           INPUT             CLASS                   INIT        COMPANY     FULLNAME            FLAGS
COMP( 1986, in207376, 0,       0,      informer_207_376, informer_207_376, informer_207_376_state, empty_init, "Informer", "Informer 207/376", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
