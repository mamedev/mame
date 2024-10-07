// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Preliminary driver for Adacom IBM 3287 ASCII printer adapters.

    Currently this does not much more than pass the extensive self-test.

    TODO: make this a bus device in the eventuality of synchronous
    communications and a 3270-compatible host both being emulated.

***************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/bcp/dp8344.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"


namespace {

class adacp150_state : public driver_device
{
public:
	adacp150_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bcp(*this, "bcp")
		, m_lcdc(*this, "lcdc")
		, m_leds(*this, "led%u", 0U)
		, m_bcp_cmd(false)
	{
	}

	void adacp150(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	HD44780_PIXEL_UPDATE(pixel_update);
	void palette_init(palette_device &palette);

	u8 bcp_ram_r(offs_t offset);
	void bcp_ram_w(offs_t offset, u8 data);
	void output_control_w(u8 data);

	void z80_mem_map(address_map &map) ATTR_COLD;
	void z80_io_map(address_map &map) ATTR_COLD;
	void bcp_prog_map(address_map &map) ATTR_COLD;
	void bcp_data_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<dp8344_device> m_bcp;
	required_device<hd44780_device> m_lcdc;
	output_finder<4> m_leds;

	bool m_bcp_cmd;
};

void adacp150_state::machine_start()
{
	m_leds.resolve();

	m_lcdc->rw_w(0);

	save_item(NAME(m_bcp_cmd));
}

HD44780_PIXEL_UPDATE(adacp150_state::pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 16)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}


u8 adacp150_state::bcp_ram_r(offs_t offset)
{
	if (m_bcp_cmd)
		return m_bcp->cmd_r();
	else
		return m_bcp->remote_read(offset | 0x4000);
}

void adacp150_state::bcp_ram_w(offs_t offset, u8 data)
{
	if (m_bcp_cmd)
		m_bcp->cmd_w(data);
	else
		m_bcp->remote_write(offset | 0x4000, data);
}

void adacp150_state::output_control_w(u8 data)
{
	for (int n = 0; n < 4; n++)
		m_leds[n] = BIT(data, n);

	m_bcp_cmd = BIT(data, 4);
	m_lcdc->rs_w(BIT(data, 6));
	m_lcdc->e_w(BIT(data, 5));
}


void adacp150_state::z80_mem_map(address_map &map)
{
	map(0x0000, 0xbfff).rom().region("program", 0);
	map(0xc000, 0xffff).rw(FUNC(adacp150_state::bcp_ram_r), FUNC(adacp150_state::bcp_ram_w));
}

void adacp150_state::z80_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("pio", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x04, 0x07).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x08, 0x0b).rw("sio", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x80, 0x80).portr("IN0");
	map(0xa0, 0xa0).portr("IN1");
	map(0xc0, 0xc0).w(FUNC(adacp150_state::output_control_w));
	map(0xe0, 0xe0).w(m_lcdc, FUNC(hd44780_device::db_w));
}

void adacp150_state::bcp_prog_map(address_map &map)
{
	map(0x0000, 0x07ff).ram(); // 2x CXK5814P-35L
}

void adacp150_state::bcp_data_map(address_map &map)
{
	map(0x0000, 0x7fff).ram().share("nvram");
}


static INPUT_PORTS_START(adacp150)
	PORT_START("IN0") // not verified
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Save")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Roll")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Set-Up")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Reset")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Cancel")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Reprint")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PA 1")

	PORT_START("IN1") // not verified
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PA 2")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LF")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("FF")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hold")
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

void adacp150_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(131, 136, 139));
	palette.set_pen_color(1, rgb_t( 92,  83,  88));
}

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ "sio" },
	{ "pio" },
	{ nullptr }
};

void adacp150_state::adacp150(machine_config &config)
{
	auto MAIN_CLOCK = 18.8696_MHz_XTAL / 6;

	Z80(config, m_maincpu, MAIN_CLOCK); // Zilog Z84C0006PEC
	m_maincpu->set_addrmap(AS_PROGRAM, &adacp150_state::z80_mem_map);
	m_maincpu->set_addrmap(AS_IO, &adacp150_state::z80_io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	DP8344A(config, m_bcp, 18.8696_MHz_XTAL); // DP8344AV
	m_bcp->set_addrmap(AS_PROGRAM, &adacp150_state::bcp_prog_map);
	m_bcp->set_addrmap(AS_DATA, &adacp150_state::bcp_data_map);
	m_bcp->set_auto_start(false);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // CXK58257P-10L + battery

	z80ctc_device &ctc(Z80CTC(config, "ctc", MAIN_CLOCK)); // TODO: part of Zilog Z84C9008VSC Z80 KIO
	ctc.set_clk<0>(2.4576_MHz_XTAL / 2);
	ctc.set_clk<1>(2.4576_MHz_XTAL / 2);
	ctc.zc_callback<0>().set("sio", FUNC(z80sio_device::rxca_w));
	ctc.zc_callback<0>().append("sio", FUNC(z80sio_device::txca_w));
	ctc.zc_callback<1>().set("sio", FUNC(z80sio_device::rxcb_w));
	ctc.zc_callback<1>().append("sio", FUNC(z80sio_device::txcb_w));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80pio_device &pio(Z80PIO(config, "pio", MAIN_CLOCK)); // TODO: part of Zilog Z84C9008VSC Z80 KIO
	pio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80sio_device &sio(Z80SIO(config, "sio", MAIN_CLOCK)); // TODO: part of Zilog Z84C9008VSC Z80 KIO
	sio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	sio.out_txda_callback().set("printer", FUNC(rs232_port_device::write_txd));
	sio.out_txdb_callback().set("host", FUNC(rs232_port_device::write_txd));

	rs232_port_device &printer(RS232_PORT(config, "printer", default_rs232_devices, nullptr));
	printer.rxd_handler().set("sio", FUNC(z80sio_device::rxa_w));

	rs232_port_device &host(RS232_PORT(config, "host", default_rs232_devices, nullptr));
	host.rxd_handler().set("sio", FUNC(z80sio_device::rxb_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_screen_update(m_lcdc, FUNC(hd44780_device::screen_update));
	screen.set_size(16*6, 16);
	screen.set_visarea(0, 16*6-1, 0, 16-1);
	screen.set_palette("palette");

	HD44780(config, m_lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_lcdc->set_lcd_size(2, 20);
	m_lcdc->set_pixel_update_cb(FUNC(adacp150_state::pixel_update));

	PALETTE(config, "palette", FUNC(adacp150_state::palette_init), 2);
}

ROM_START(adacp150p)
	ROM_REGION(0xc000, "program", 0)
	ROM_LOAD("pa_2436__rev-4.52.u11", 0x0000, 0x8000, CRC(a381674c) SHA1(1d3cb4ca3ead40da67a353efe7553ea953fa929d)) // Intel D27C256
	ROM_LOAD("pa_2437__rev-4.52.u22", 0x8000, 0x4000, CRC(eb468ad0) SHA1(881a90a6aa89d7e289d7adbec46d007a8cfa5351)) // Intel D27C128
ROM_END

} // anonymous namespace


SYST(1989, adacp150p, 0, 0, adacp150, adacp150, adacp150_state, empty_init, "Adacom", "CP-150 Plus", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
