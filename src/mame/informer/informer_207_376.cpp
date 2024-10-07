// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Informer 207/376

    IBM 3270 compatible terminal

    Hardware:
    - M6809
    - 8253 PIT
    - Z80SCC 8530
    - 2x 6850 ACIA (up to 4)
    - 2x X2212P NOVRAM

    TODO:
    - Dump keyboard controller and emulate it (currently HLE'd)
    - Problably needs improvements to at least the Z80SCC to
      properly support synchrous modes
    - Figure out the unknown bits at 0x8400
    - Verify clock speeds

    Notes:
    - Thanks to charcole and his zmachine3270 project at
      https://github.com/charcole/zmachine3270

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6850acia.h"
#include "machine/pit8253.h"
#include "machine/input_merger.h"
#include "machine/ripple_counter.h"
#include "machine/x2212.h"
#include "machine/z80scc.h"
#include "video/mc6845.h"
#include "sound/beep.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/printer.h"
#include "informer_207_376_kbd.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class informer_207_376_state : public driver_device
{
public:
	informer_207_376_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_novram(*this, "novram%u", 0U),
		m_crtc(*this, "crtc"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_pit(*this, "pit"),
		m_scc(*this, "scc"),
		m_acia(*this, "acia%u", 0U),
		m_beep(*this, "beep"),
		m_ram(*this, "ram"),
		m_chargen(*this, "chargen")
	{ }

	void informer_207_376(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<x2212_device, 2> m_novram;
	required_device<mc6845_device> m_crtc;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<pit8253_device> m_pit;
	required_device<scc85c30_device> m_scc;
	required_device_array<acia6850_device, 2> m_acia;
	required_device<beep_device> m_beep;
	required_shared_ptr<uint8_t> m_ram;
	required_region_ptr<uint8_t> m_chargen;

	void mem_map(address_map &map) ATTR_COLD;

	void unk_8400_w(uint8_t data);
	void crt_brightness_w(uint8_t data);
	uint8_t novram_r(address_space &space, offs_t offset);
	void novram_w(offs_t offset, uint8_t data);
	uint8_t novram_recall_r();

	MC6845_UPDATE_ROW(crtc_update_row);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void informer_207_376_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).ram().share("ram");
	map(0x8000, 0x8000).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x8001, 0x8001).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x8400, 0x8400).lr8(NAME([] () { return 0xff; })).w(FUNC(informer_207_376_state::unk_8400_w)); // ?
	map(0x8802, 0x8803).rw(m_acia[0], FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x8804, 0x8805).rw(m_acia[1], FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x8c00, 0x8c00).w(FUNC(informer_207_376_state::crt_brightness_w));
	map(0x9000, 0x9003).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x9400, 0x9403).rw(m_scc, FUNC(scc85c30_device::ab_dc_r), FUNC(scc85c30_device::ab_dc_w));
	map(0x9800, 0x9800).r(FUNC(informer_207_376_state::novram_recall_r));
	map(0x9c00, 0x9cff).rw(FUNC(informer_207_376_state::novram_r), FUNC(informer_207_376_state::novram_w));
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

void informer_207_376_state::crt_brightness_w(uint8_t data)
{
	// unknown algorithm for the brightness
	// default value is 6, range is 0 (off) to 15 (brightest)
	m_screen->set_brightness(256 - (256 / ((data & 0x0f) + 1)));
}

MC6845_UPDATE_ROW( informer_207_376_state::crtc_update_row )
{
	pen_t const *const pen = m_palette->pens();

	for (int x = 0; x < x_count; x++)
	{
		uint8_t const code = m_ram[ma + x];
		uint8_t data = m_chargen[(code << 4) + ra];

		if (x == cursor_x)
			data = 0xff;

		// the line above the status bar seems to be hardcoded
		if (y == 274)
			data = 0xff;

		// draw 8 pixels of the character
		bitmap.pix(y, x * 8 + 7) = pen[BIT(data, 0)];
		bitmap.pix(y, x * 8 + 6) = pen[BIT(data, 1)];
		bitmap.pix(y, x * 8 + 5) = pen[BIT(data, 2)];
		bitmap.pix(y, x * 8 + 4) = pen[BIT(data, 3)];
		bitmap.pix(y, x * 8 + 3) = pen[BIT(data, 4)];
		bitmap.pix(y, x * 8 + 2) = pen[BIT(data, 5)];
		bitmap.pix(y, x * 8 + 1) = pen[BIT(data, 6)];
		bitmap.pix(y, x * 8 + 0) = pen[BIT(data, 7)];
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

void informer_207_376_state::unk_8400_w(uint8_t data)
{
	// 7-------  beeper
	// -6------  unknown
	// --5-----  1=internal modem, 0=host rs232
	// ---43---  unknown
	// -----2--  unknown
	// ------10  unknown

	m_beep->set_state(BIT(data, 7));
}

uint8_t informer_207_376_state::novram_r(address_space &space, offs_t offset)
{
	return (m_novram[0]->read(space, offset) << 4) | (m_novram[1]->read(space, offset) & 0x0f);
}

void informer_207_376_state::novram_w(offs_t offset, uint8_t data)
{
	m_novram[0]->write(offset, data >> 4);
	m_novram[1]->write(offset, data & 0x0f);
}

uint8_t informer_207_376_state::novram_recall_r()
{
	m_novram[0]->recall(1);
	m_novram[1]->recall(1);
	m_novram[0]->recall(0);
	m_novram[1]->recall(0);

	return 0xff;
}

void informer_207_376_state::machine_start()
{
}

void informer_207_376_state::machine_reset()
{
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
	MC6809(config, m_maincpu, 36_MHz_XTAL / 4); // unknown clock divisor
	m_maincpu->set_addrmap(AS_PROGRAM, &informer_207_376_state::mem_map);

	input_merger_device &cpu_irq(INPUT_MERGER_ANY_HIGH(config, "cpu_irq"));
	cpu_irq.output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	X2212(config, m_novram[0]);
	X2212(config, m_novram[1]);

	PIT8253(config, m_pit);
	m_pit->set_clk<0>(2.457600_MHz_XTAL);
	m_pit->out_handler<0>().set(m_acia[1], FUNC(acia6850_device::write_txc));
	m_pit->out_handler<0>().append(m_acia[1], FUNC(acia6850_device::write_rxc));
	m_pit->set_clk<1>(2.457600_MHz_XTAL);
	m_pit->out_handler<1>().set(m_acia[0], FUNC(acia6850_device::write_txc));
	m_pit->out_handler<1>().append(m_acia[0], FUNC(acia6850_device::write_rxc));
	m_pit->out_handler<1>().append("nmi_clk", FUNC(ripple_counter_device::clock_w));

	ripple_counter_device &nmi_clk(RIPPLE_COUNTER(config, "nmi_clk")); // CD4020BE
	nmi_clk.set_stages(14);
	nmi_clk.count_out_cb().set_inputline(m_maincpu, INPUT_LINE_NMI).bit(13); // Q14

	SCC85C30(config, m_scc, 0); // externally clocked?
	m_scc->out_txda_callback().set("com1", FUNC(rs232_port_device::write_txd));
	m_scc->out_dtra_callback().set("com1", FUNC(rs232_port_device::write_dtr));
	m_scc->out_rtsa_callback().set("com1", FUNC(rs232_port_device::write_rts));
	m_scc->out_txdb_callback().set("com2", FUNC(rs232_port_device::write_txd));
	m_scc->out_dtrb_callback().set("com2", FUNC(rs232_port_device::write_dtr));
	m_scc->out_rtsb_callback().set("com2", FUNC(rs232_port_device::write_rts));
	m_scc->out_int_callback().set("cpu_irq", FUNC(input_merger_device::in_w<0>));

	ACIA6850(config, m_acia[0]);
	m_acia[0]->txd_handler().set("kbd", FUNC(informer_207_376_kbd_hle_device::rx_w));
	m_acia[0]->irq_handler().set("cpu_irq", FUNC(input_merger_device::in_w<1>));
	m_acia[0]->rts_handler().set(m_novram[0], FUNC(x2212_device::store)).invert();
	m_acia[0]->rts_handler().append(m_novram[1], FUNC(x2212_device::store)).invert();

	ACIA6850(config, m_acia[1]);
	m_acia[1]->txd_handler().set("printer", FUNC(rs232_port_device::write_txd));

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
	m_screen->set_raw(36_MHz_XTAL / 2.5, 800, 0, 640, 300, 0, 286);
	m_screen->set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	MC6845(config, m_crtc, 36_MHz_XTAL / 20);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(informer_207_376_state::crtc_update_row));

	// sound
	SPEAKER(config, "mono").front_center();
	BEEP(config, "beep", 500).add_route(ALL_OUTPUTS, "mono", 0.50); // frequency unknown
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( in207376 )
	ROM_REGION(0x6000, "maincpu", 0)
	// M2764A F1
	// 79590-023  376SNA V1.00  201C 7224  (checksum matches)
	ROM_LOAD("79590-023.z37", 0x0000, 0x2000, CRC(61d49637) SHA1(aabcd55af88ff0b6a198eef04a88f58cc8f65dcc))
	// M27128AF1
	// 79589-023  376 SNAV1.00  201C E86F  (checksum matches)
	ROM_LOAD("79589-023.z36", 0x2000, 0x4000, CRC(cdfaf629) SHA1(1f21ef6848020726ef3d7ab05166ac8590d58476))

	ROM_REGION(0x1000, "chargen", 0)
	// TMS2732AGL-25
	// 79573-001  V1.00 376/8  CHAR. GEN.
	ROM_LOAD("79573-001.z6", 0x0000, 0x1000, CRC(f704b827) SHA1(bcc56eeb8681c2bebe3a9f4b6b78c0373c06d875))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT   COMPAT  MACHINE           INPUT             CLASS                   INIT        COMPANY     FULLNAME            FLAGS
COMP( 1986, in207376, 0,       0,      informer_207_376, informer_207_376, informer_207_376_state, empty_init, "Informer", "Informer 207/376", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
