// license:BSD-3-Clause
// copyright-holders:Carl

#include "emu.h"
#include "cpu/i86/i86.h"
#include "imagedev/floppy.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "machine/i8251.h"
#include "machine/i8257.h"
#include "machine/upd765.h"
#include "video/mc6845.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/keyboard.h"
#include "emupal.h"
#include "screen.h"

class peoplepc_state : public driver_device
{
public:
	peoplepc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_pic_1(*this, "pic8259_1"),
		m_8251key(*this, "i8251_0"),
		m_8251ser(*this, "i8251_1"),
		m_fdc(*this, "upd765"),
		m_flop0(*this, "upd765:0"),
		m_flop1(*this, "upd765:1"),
		m_dmac(*this, "i8257"),
		m_gfxdecode(*this, "gfxdecode"),
		m_gvram(*this, "gvram"),
		m_cvram(*this, "cvram"),
		m_charram(4*1024)
	{ }

	void olypeopl(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<pic8259_device> m_pic_1;
	required_device<i8251_device> m_8251key;
	required_device<i8251_device> m_8251ser;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_flop0;
	required_device<floppy_connector> m_flop1;
	required_device<i8257_device> m_dmac;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint16_t> m_gvram;
	required_shared_ptr<uint16_t> m_cvram;
	std::vector<uint8_t> m_charram;

	MC6845_UPDATE_ROW(update_row);
	DECLARE_READ8_MEMBER(get_slave_ack);
	DECLARE_WRITE16_MEMBER(charram_w);
	DECLARE_WRITE_LINE_MEMBER(tty_clock_tick_w);
	DECLARE_WRITE_LINE_MEMBER(kbd_clock_tick_w);
	DECLARE_WRITE8_MEMBER(dmapg_w);
	DECLARE_WRITE_LINE_MEMBER(tc_w);
	DECLARE_WRITE_LINE_MEMBER(hrq_w);
	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_FLOPPY_FORMATS( floppy_formats );
	image_init_result floppy_load(floppy_image_device *dev);
	void floppy_unload(floppy_image_device *dev);

	uint8_t m_dma0pg;
	void peoplepc_io(address_map &map);
	void peoplepc_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;
};

static const gfx_layout peoplepc_charlayout =
{
	8, 19,                   /* 8 x 19 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0},
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8, 16*8, 17*8, 18*8 },
	8*32
};

MC6845_UPDATE_ROW(peoplepc_state::update_row)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	int i, j;

	for(i = 0; i < x_count; i++)
	{
		if(0)
		{
			uint16_t offset = ((ma | (ra << 1)) << 4) + i;
			uint8_t data = m_gvram[offset] >> (offset & 1 ? 8 : 0);

			for(j = 8; j >= 0; j--)
				bitmap.pix32(y, (i * 8) + j) = palette[( data & 1 << j ) ? 1 : 0];
		}
		else
		{
			uint8_t data = m_charram[(m_cvram[(ma + i) & 0x3fff] & 0x7f) * 32 + ra];
			for(j = 0; j < 8; j++)
				bitmap.pix32(y, (i * 8) + j) = palette[(data & (1 << j)) ? 1 : 0];
		}
	}
}

READ8_MEMBER(peoplepc_state::get_slave_ack)
{
	if (offset == 7)
		return m_pic_1->acknowledge();

	return 0x00;
}

WRITE16_MEMBER(peoplepc_state::charram_w)
{
	m_charram[offset] = data;
	m_gfxdecode->gfx(0)->mark_dirty(offset/16);
}

WRITE_LINE_MEMBER(peoplepc_state::tty_clock_tick_w)
{
	m_8251ser->write_txc(state);
	m_8251ser->write_rxc(state);
}

WRITE_LINE_MEMBER(peoplepc_state::kbd_clock_tick_w)
{
	m_8251key->write_txc(state);
	m_8251key->write_rxc(state);
}

WRITE8_MEMBER(peoplepc_state::dmapg_w)
{
	m_dma0pg = data;
}

WRITE_LINE_MEMBER(peoplepc_state::tc_w)
{
	m_fdc->tc_w(state);
}

WRITE_LINE_MEMBER(peoplepc_state::hrq_w)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);
	m_dmac->hlda_w(state);
}

READ8_MEMBER(peoplepc_state::memory_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset | (m_dma0pg << 16));
}

WRITE8_MEMBER(peoplepc_state::memory_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	prog_space.write_byte(offset | (m_dma0pg << 16), data);
}

image_init_result peoplepc_state::floppy_load(floppy_image_device *dev)
{
	dev->mon_w(0);
	return image_init_result::PASS;
}

void peoplepc_state::floppy_unload(floppy_image_device *dev)
{
	dev->mon_w(1);
}

void peoplepc_state::machine_reset()
{
	m_flop0->get_device()->mon_w(!m_flop0->get_device()->exists());
	m_flop1->get_device()->mon_w(!m_flop1->get_device()->exists());
}

void peoplepc_state::machine_start()
{
	m_gfxdecode->set_gfx(0, std::make_unique<gfx_element>(m_palette, peoplepc_charlayout, &m_charram[0], 0, 1, 0));
	m_dma0pg = 0;

	// FIXME: cheat as there no docs about how or obvious ports that set to control the motor
	m_flop0->get_device()->setup_load_cb(floppy_image_device::load_cb(&peoplepc_state::floppy_load, this));
	m_flop0->get_device()->setup_unload_cb(floppy_image_device::unload_cb(&peoplepc_state::floppy_unload, this));
	m_flop1->get_device()->setup_load_cb(floppy_image_device::load_cb(&peoplepc_state::floppy_load, this));
	m_flop1->get_device()->setup_unload_cb(floppy_image_device::unload_cb(&peoplepc_state::floppy_unload, this));
}

void peoplepc_state::peoplepc_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x7ffff).ram();
	map(0xc0000, 0xdffff).ram().share("gvram");
	map(0xe0000, 0xe3fff).ram().share("cvram");
	map(0xe4000, 0xe5fff).w(FUNC(peoplepc_state::charram_w));
	map(0xfe000, 0xfffff).rom().region("maincpu", 0);
}

void peoplepc_state::peoplepc_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0014, 0x0017).rw(m_pic_1, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x0018, 0x001b).rw("pic8259_0", FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x0020, 0x0031).rw(m_dmac, FUNC(i8257_device::read), FUNC(i8257_device::write)).umask16(0x00ff);
	map(0x0040, 0x0047).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x0048, 0x004f).rw("pit8253", FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0x0054, 0x0057).rw(m_8251key, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0x005c, 0x005f).rw(m_8251ser, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0x0064, 0x0067).m(m_fdc, FUNC(upd765a_device::map)).umask16(0x00ff);
	map(0x006c, 0x006c).w("h46505", FUNC(mc6845_device::address_w));
	map(0x006e, 0x006e).rw("h46505", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x0070, 0x0070).w(FUNC(peoplepc_state::dmapg_w));
}

static void peoplepc_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

FLOPPY_FORMATS_MEMBER( peoplepc_state::floppy_formats )
	FLOPPY_IMD_FORMAT
FLOPPY_FORMATS_END

void peoplepc_keyboard_devices(device_slot_interface &device)
{
	device.option_add("keyboard", SERIAL_KEYBOARD);
}

static DEVICE_INPUT_DEFAULTS_START(keyboard)
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void peoplepc_state::olypeopl(machine_config &config)
{
	/* basic machine hardware */
	I8086(config, m_maincpu, XTAL(14'745'600)/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &peoplepc_state::peoplepc_map);
	m_maincpu->set_addrmap(AS_IO, &peoplepc_state::peoplepc_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_0", FUNC(pic8259_device::inta_cb));

	pit8253_device &pit8253(PIT8253(config, "pit8253", 0));
	pit8253.set_clk<0>(XTAL(14'745'600)/6);
	pit8253.out_handler<0>().set(FUNC(peoplepc_state::kbd_clock_tick_w));
	pit8253.set_clk<1>(XTAL(14'745'600)/6);
	pit8253.out_handler<1>().set(FUNC(peoplepc_state::tty_clock_tick_w));
	pit8253.set_clk<2>(XTAL(14'745'600)/6);
	pit8253.out_handler<2>().set("pic8259_0", FUNC(pic8259_device::ir0_w));

	pic8259_device &pic8259_0(PIC8259(config, "pic8259_0", 0));
	pic8259_0.out_int_callback().set_inputline(m_maincpu, 0);
	pic8259_0.in_sp_callback().set_constant(1);
	pic8259_0.read_slave_ack_callback().set(FUNC(peoplepc_state::get_slave_ack));

	PIC8259(config, m_pic_1, 0);
	m_pic_1->out_int_callback().set("pic8259_0", FUNC(pic8259_device::ir7_w));
	m_pic_1->in_sp_callback().set_constant(0);

	I8255(config, "ppi8255");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_raw(XTAL(22'000'000), 640, 0, 640, 475, 0, 475);
	screen.set_screen_update("h46505", FUNC(mc6845_device::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfxdecode_device::empty);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	mc6845_device &crtc(HD6845S(config, "h46505", XTAL(22'000'000)/8)); // HD46505SP according to User's Guide
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(peoplepc_state::update_row));

	I8257(config, m_dmac, XTAL(14'745'600)/3);
	m_dmac->out_hrq_cb().set(FUNC(peoplepc_state::hrq_w));
	m_dmac->out_tc_cb().set(FUNC(peoplepc_state::tc_w));
	m_dmac->in_memr_cb().set(FUNC(peoplepc_state::memory_read_byte));
	m_dmac->out_memw_cb().set(FUNC(peoplepc_state::memory_write_byte));
	m_dmac->in_ior_cb<0>().set("upd765", FUNC(upd765a_device::dma_r));
	m_dmac->out_iow_cb<0>().set("upd765", FUNC(upd765a_device::dma_w));

	UPD765A(config, m_fdc, 8'000'000, true, true);
	m_fdc->intrq_wr_callback().set("pic8259_0", FUNC(pic8259_device::ir2_w));
	m_fdc->drq_wr_callback().set(m_dmac, FUNC(i8257_device::dreq0_w));
	FLOPPY_CONNECTOR(config, "upd765:0", peoplepc_floppies, "525qd", peoplepc_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "upd765:1", peoplepc_floppies, "525qd", peoplepc_state::floppy_formats);

	I8251(config, m_8251key, 0);
	m_8251key->rxrdy_handler().set("pic8259_1", FUNC(pic8259_device::ir1_w));
	m_8251key->txd_handler().set("kbd", FUNC(rs232_port_device::write_txd));

	rs232_port_device &kbd(RS232_PORT(config, "kbd", peoplepc_keyboard_devices, "keyboard"));
	kbd.rxd_handler().set(m_8251key, FUNC(i8251_device::write_rxd));
	kbd.set_option_device_input_defaults("keyboard", DEVICE_INPUT_DEFAULTS_NAME(keyboard));

	I8251(config, m_8251ser, 0);
}

ROM_START( olypeopl )
	ROM_REGION(0x2000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "hd",  "HD ROM")
	ROMX_LOAD( "u01271c0.bin", 0x00000, 0x1000, CRC(8e0ef114) SHA1(774bab0a3e29853e9f6b951cf73082063ea61e6d), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "u01271d0.bin", 0x00001, 0x1000, CRC(e2419bf9) SHA1(d88381f8709c91e2adba08f378e29bd0d19ee5ae), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "2fd",  "2 FD ROM")
	ROMX_LOAD( "u01277f3.bin", 0x00000, 0x1000, CRC(428ff135) SHA1(ec11f0e43455570c40f5dc4b84f8420da5939368), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "u01277g3.bin", 0x00001, 0x1000, CRC(3295691c) SHA1(7d7ade62117d11656b8dd86cf0703127616d55bc), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

COMP( 198?, olypeopl, 0, 0, olypeopl, 0, peoplepc_state, empty_init, "Olympia", "People PC", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
