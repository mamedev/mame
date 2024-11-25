// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

Skeleton driver for Reuters Model SK 101 BL trading terminal.

This terminal is a small portable-sized unit with a 115-key keyboard, which is driven largely through LSTTL shift registers. The
keyboard layout is similar to that depicted in patent EP0434224A2.

Attached to the keyboard is a backlit LCD module displaying one row of 16 characters.

The main board incorporates two DSWs and a "BLMHYB01" hybrid module containing unknown interface logic.

TODO: figure out keycodes (are they translated externally?)

***********************************************************************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/spkrdev.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class sk101bl_state : public driver_device
{
public:
	sk101bl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "lcdc")
		, m_key_row(*this, "ROW%u", 0U)
	{
	}

	void sk101bl(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	HD44780_PIXEL_UPDATE(pixel_update);

	u8 p1_r();
	void p1_w(u8 data);
	void scan_load_w(u8 data);
	u8 scan_shift_r();
	void lcd_control_w(u8 data);

	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	required_device<i80c31_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
	optional_ioport_array<16> m_key_row;

	u8 m_p1_data = 0;
	u8 m_lcd_control = 0;
	u16 m_key_scan = 0;
	u16 m_shift_output = 0;
	u8 m_row_counter = 0;
};

void sk101bl_state::machine_start()
{
	m_p1_data = 0xff;
	m_shift_output = 0;
	m_key_scan = 0xffff;
	m_row_counter = 0;

	save_item(NAME(m_p1_data));
	save_item(NAME(m_lcd_control));
	save_item(NAME(m_key_scan));
	save_item(NAME(m_shift_output));
	save_item(NAME(m_row_counter));
}

void sk101bl_state::machine_reset()
{
	m_lcd_control = 0;
}

HD44780_PIXEL_UPDATE(sk101bl_state::pixel_update)
{
	if (pos < 16 && line == 0)
		bitmap.pix(line * 10 + y, pos * 6 + x) = state;
}

u8 sk101bl_state::p1_r()
{
	return m_lcdc->db_r();
}

void sk101bl_state::p1_w(u8 data)
{
	if (BIT(m_p1_data, 7) && BIT(m_p1_data, 6) && !BIT(data, 6) && !BIT(m_lcd_control, 2))
		m_row_counter = (m_row_counter - 1) & 0x0f;

	m_p1_data = data;
	m_lcdc->db_w(data);
}

void sk101bl_state::scan_load_w(u8 data)
{
	m_key_scan = m_key_row[m_row_counter & 0x0f].read_safe(0xffff);
	m_maincpu->set_input_line(MCS51_INT0_LINE, BIT(m_key_scan, 15) ? CLEAR_LINE : ASSERT_LINE);
}

u8 sk101bl_state::scan_shift_r()
{
	if (!machine().side_effects_disabled())
	{
		m_key_scan = (m_key_scan << 1) | 1;
		m_maincpu->set_input_line(MCS51_INT0_LINE, BIT(m_key_scan, 15) ? CLEAR_LINE : ASSERT_LINE);
	}
	return 0xff;
}

void sk101bl_state::lcd_control_w(u8 data)
{
	if (BIT(data, 7) && !BIT(m_lcd_control, 7))
		m_shift_output = (m_shift_output << 1) | (BIT(m_p1_data, 7) ? 0 : 1);

	m_lcdc->rw_w(BIT(data, 3));
	m_lcdc->rs_w(BIT(data, 2));
	m_lcdc->e_w(BIT(data, 1));

	if (BIT(data, 0))
		m_row_counter = 0;

	m_lcd_control = data;
}

void sk101bl_state::prog_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("program", 0);
}

void sk101bl_state::ext_map(address_map &map)
{
	map(0x0000, 0x0000).mirror(0x1fff).r(FUNC(sk101bl_state::scan_shift_r));
	map(0x1000, 0x1000).mirror(0x0fff).w(FUNC(sk101bl_state::scan_load_w));
	map(0x2000, 0x7fff).rom().region("program", 0x2000);
	map(0x4000, 0x4000).mirror(0x0fff).w(FUNC(sk101bl_state::lcd_control_w));
	map(0xe000, 0xffff).ram(); // TC5565APL-15L
}

static INPUT_PORTS_START(sk101bl)
INPUT_PORTS_END

void sk101bl_state::sk101bl(machine_config &config)
{
	I80C31(config, m_maincpu, 11.0592_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sk101bl_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &sk101bl_state::ext_map);
	m_maincpu->port_in_cb<1>().set(FUNC(sk101bl_state::p1_r));
	m_maincpu->port_out_cb<1>().set(FUNC(sk101bl_state::p1_w));
	m_maincpu->port_out_cb<3>().set("alarm", FUNC(speaker_sound_device::level_w)).bit(3);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(16*6, 10);
	screen.set_visarea(0, 16*6-1, 0, 10-1);
	screen.set_palette("palette");

	HD44780(config, m_lcdc, 270'000).set_lcd_size(1, 16); // TODO: clock not measured, datasheet typical clock used
	m_lcdc->set_pixel_update_cb(FUNC(sk101bl_state::pixel_update));

	PALETTE(config, "palette").set_entries(2);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "alarm").add_route(ALL_OUTPUTS, "mono", 1.00);
}

ROM_START(sk101bl)
	ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("sk_101_5.0_1ca2.ic7", 0x0000, 0x8000, CRC(f7903ca5) SHA1(66648cc1622c1241cdbd443af706750acbb93502)) // 27256-25
ROM_END

} // anonymous namespace


COMP(1988, sk101bl, 0, 0, sk101bl, sk101bl, sk101bl_state, empty_init, "Reuters", "Model SK 101 BL", MACHINE_NOT_WORKING)
