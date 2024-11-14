// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*******************************************************************************

    Kyber Minus

*******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/keyboard.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "machine/wd_fdc.h"
#include "video/mc6845.h"
#include "bus/rs232/rs232.h"
#include "imagedev/floppy.h"
#include "screen.h"
#include "softlist.h"


namespace {

class kminus_state : public driver_device
{
public:
	kminus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_chargen(*this, "chargen")
		, m_videoram(*this, "videoram")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_view(*this, "bootview")
	{ }

	void kminus(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	MC6845_UPDATE_ROW(update_row);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_region_ptr<uint8_t> m_chargen;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<fd1793_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	floppy_image_device *m_selected_floppy = nullptr;

	memory_view m_view;

	uint8_t pio1_pa_r();
	void pio1_pa_w(uint8_t data);
	uint8_t pio1_pb_r();
	void pio1_pb_w(uint8_t data);

	void kbd_put(uint8_t data);
	uint8_t kbd_r();
	uint8_t m_kbd_data = 0;
};


MC6845_UPDATE_ROW(kminus_state::update_row)
{
	uint32_t *p = &bitmap.pix(y);

	for (int column = 0; column < x_count; column++)
	{
		uint8_t chr = m_videoram[(ma + column) & 0x7ff];
		uint16_t addr = (chr << 4) | (ra & 0x0f);
		uint16_t data = m_chargen[(0x800 | addr) & 0xfff];
		//uint16_t data = m_chargen[addr & 0xfff];

		if (column == cursor_x)
		{
			data = 0xff;
		}

		for (int bit = 0; bit < 8; bit++)
		{
			*p++ = BIT(data, 0) ? rgb_t::white() : rgb_t::black();
			data >>= 1;
		}
	}
}


void kminus_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).view(m_view);
	m_view[0](0x0000, 0x0fff).rom().region("maincpu", 0).mirror(0xf000);
	m_view[1](0x0000, 0xffff).ram();
	m_view[1](0xc000, 0xc7ff).ram().share("videoram");
	m_view[1](0xf000, 0xffff).rom().region("maincpu", 0);
}

void kminus_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x43).rw("dart", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0x48, 0x4b).rw("pio0", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x50, 0x53).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x58, 0x5b).rw("pio1", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x60, 0x63).rw("fdc", FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0x68, 0x6f).noprw(); // sel1
	map(0x70, 0x77).noprw(); // sel2
	map(0x78, 0x78).w("crtc", FUNC(mc6845_device::address_w));
	map(0x79, 0x79).w("crtc", FUNC(mc6845_device::register_w));
	map(0x7a, 0x7a).r("crtc", FUNC(mc6845_device::status_r));
	map(0x7b, 0x7b).r("crtc", FUNC(mc6845_device::register_r));
	map(0x7c, 0x7c).lr8([]() { return 0x00; }, "ready"); //22 s3 r: b7 15 s4, b1 printer ready? w: printer data?
	//map(0x7d, 0x7d).w(); // 9 s4 video out chargen related? writes 0x10
	map(0x7e, 0x7e).lw8([this](uint8_t data) { logerror("enable_ram: %02x\n", data); m_view.select(1); }, "enable_ram_w"); // 13 s4 write $03 enable video ram? bits 3,1,0 vram [b1 vram
	map(0x7f, 0x7f).r(FUNC(kminus_state::kbd_r)); // 17 s5 kbd
}


uint8_t kminus_state::pio1_pa_r()
{
	/*

	bit     signal      description

	2       WPRT        write protect
	3       READY       ready
	4       FD2S        disk is two sided
	5       IP          index pulse
	6       INTRQ
	7       DRQ

	*/

	uint8_t data = 0x00;

	if (m_selected_floppy)
	{
		data |= m_selected_floppy->wpt_r() << 2;
		data |= m_selected_floppy->ready_r() << 3;
		data |= m_selected_floppy->twosid_r() << 4;
		data |= m_selected_floppy->idx_r() << 5;
	}

	data |= m_fdc->intrq_r() ? 0x00 : 0x40;
	data |= m_fdc->drq_r() ? 0x00 : 0x80;

	logerror("pio_pa_r: %02x\n", data);

	return data;
}

void kminus_state::pio1_pa_w(uint8_t data)
{
	/*

	bit     signal      description

	0       ?           wd1000 hard controller
	1       ?           wd1000 hard controller

	*/

	logerror("pio_pa_w: %02x\n", data);
}

uint8_t kminus_state::pio1_pb_r()
{
	/*

	bit     signal      description

	4       TG43        track > 43
	6       F5/F8       5.25"/8" select

	*/

	uint8_t data = 0x40;

	logerror("pio_pb_r: %02x\n", data);

	return data;
}

void kminus_state::pio1_pb_w(uint8_t data)
{
	/*

	bit     signal      description

	0       DS0         drive select 0
	1       DS1         drive select 1
	2       DS2         drive select 2
	3       DS3         drive select 3
	5       DDEN        density select
	7       SIDE1       side 1 select

	*/

	// drive select
	if (BIT(data, 0)) m_selected_floppy = m_floppy[0]->get_device();
	if (BIT(data, 1)) m_selected_floppy = m_floppy[1]->get_device();
	if (BIT(data, 2)) m_selected_floppy = nullptr; // floppy 2
	if (BIT(data, 3)) m_selected_floppy = nullptr; // floppy 3

	m_fdc->set_floppy(m_selected_floppy);

	// density select
	m_fdc->dden_w(BIT(data, 5));

	if (m_selected_floppy)
	{
		// side select
		m_selected_floppy->ss_w(!BIT(data, 7));
		m_selected_floppy->mon_w(0);
	}

	logerror("pio_pb_w: %02x\n", data);
}


void kminus_state::kbd_put(uint8_t data)
{
	//data &= 0x7f;
	/* allow backspace to work */
	//if (data == 8) data = 0x7f;
	// assert strobe
	m_kbd_data = data;
}

uint8_t kminus_state::kbd_r()
{
	uint8_t data = m_kbd_data;

	m_kbd_data = 0x00;

	return data;
}


void kminus_state::machine_start()
{
	save_item(NAME(m_kbd_data));
}

void kminus_state::machine_reset()
{
	m_view.select(0);
}


static const z80_daisy_config daisy_chain[] =
{
	{ "pio1" },
	{ "dart" },
	{ "pio0" },
	{ "ctc" },
	{ nullptr }
};

void kminus_state::kminus(machine_config &config)
{
	Z80(config, m_maincpu, 3.6864_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &kminus_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &kminus_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	z80pio_device &pio0(Z80PIO(config, "pio0", 3.6864_MHz_XTAL));
	pio0.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80pio_device &pio1(Z80PIO(config, "pio1", 3.6864_MHz_XTAL));
	pio1.in_pa_callback().set(FUNC(kminus_state::pio1_pa_r));
	pio1.out_pa_callback().set(FUNC(kminus_state::pio1_pa_w));
	pio1.in_pb_callback().set(FUNC(kminus_state::pio1_pb_r));
	pio1.out_pb_callback().set(FUNC(kminus_state::pio1_pb_w));
	pio1.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80ctc_device &ctc(Z80CTC(config, "ctc", 3.6864_MHz_XTAL));
	ctc.zc_callback<0>().set("dart", FUNC(z80dart_device::txca_w));
	ctc.zc_callback<0>().append("dart", FUNC(z80dart_device::rxca_w));
	ctc.zc_callback<1>().set("dart", FUNC(z80dart_device::txca_w));
	ctc.zc_callback<2>().set("dart", FUNC(z80dart_device::rxtxcb_w));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80dart_device &dart(Z80DART(config, "dart", 3.6864_MHz_XTAL));
	dart.out_txda_callback().set("terminal", FUNC(rs232_port_device::write_txd));
	dart.out_dtra_callback().set("terminal", FUNC(rs232_port_device::write_dtr));
	dart.out_rtsa_callback().set("terminal", FUNC(rs232_port_device::write_rts));
	dart.out_txdb_callback().set("serial", FUNC(rs232_port_device::write_txd));
	dart.out_dtrb_callback().set("serial", FUNC(rs232_port_device::write_dtr));
	dart.out_rtsb_callback().set("serial", FUNC(rs232_port_device::write_rts));
	dart.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	rs232_port_device &rs232a(RS232_PORT(config, "terminal", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set("dart", FUNC(z80dart_device::rxa_w));

	rs232_port_device &rs232b(RS232_PORT(config, "serial", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set("dart", FUNC(z80dart_device::rxb_w));

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(kminus_state::kbd_put));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(26_MHz_XTAL / 2, 840, 0, 640, 309, 0, 250);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	mc6845_device &crtc(MC6845(config, "crtc", 26_MHz_XTAL / 16));
	crtc.set_screen("screen");
	crtc.set_char_width(8);
	crtc.set_show_border_area(false);
	crtc.set_update_row_callback(FUNC(kminus_state::update_row));

	FD1793(config, m_fdc, 8_MHz_XTAL / 8);
	m_fdc->ready_wr_callback().set("pio1", FUNC(z80pio_device::pa3_w));
	m_fdc->intrq_wr_callback().set("pio1", FUNC(z80pio_device::pa6_w)).invert();
	m_fdc->drq_wr_callback().set("pio1", FUNC(z80pio_device::pa7_w)).invert();
	//m_fdc->set_force_ready(true);
	FLOPPY_CONNECTOR(config, m_floppy[0], "525qd", FLOPPY_525_QD, true, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], "525qd", FLOPPY_525_QD, true, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	//SOFTWARE_LIST(config, "flop_list").set_original("kminus_flop");
}


ROM_START(kminus)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("kyberware_firmware.u83", 0x0000, 0x1000, CRC(fb7fa0b6) SHA1(7bc54743452d587e3b3ac3fcc088c2c979fa7c8e))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("kyberware_chargen.u24", 0x0000, 0x1000, CRC(18d9e79e) SHA1(e0a80dab109c0ac8b7f214d597955c97618208b3))
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY              FULLNAME       FLAGS
COMP( 1982, kminus, 0,      0,      kminus,  0,      kminus_state, empty_init, "Kyber Calcolatori", "Kyber Minus", MACHINE_NOT_WORKING )
