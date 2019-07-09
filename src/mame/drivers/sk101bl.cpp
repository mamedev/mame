// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

Skeleton driver for Reuters Model SK 101 BL trading terminal.

This terminal is a small portable-sized unit with a 115-key keyboard, which is driven largely through LSTTL shift registers. The
keyboard layout is similar to that depicted in patent EP0434224A2.

Attached to the keyboard is a backlit LCD module displaying one row of 16 characters.

The main board incorporates two DSWs and a "BLMHYB01" hybrid module containing unknown interface logic.

***********************************************************************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/spkrdev.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class sk101bl_state : public driver_device
{
public:
	sk101bl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "lcdc")
	{
	}

	void sk101bl(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	HD44780_PIXEL_UPDATE(pixel_update);

	u8 p1_r();
	void p1_w(u8 data);
	void p24_latch_w(u8 data);
	void lcd_control_w(u8 data);

	void prog_map(address_map &map);
	void ext_map(address_map &map);

	required_device<i80c31_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;

	u8 m_lcd_data;
	u8 m_lcd_control;
};

void sk101bl_state::machine_start()
{
	save_item(NAME(m_lcd_data));
	save_item(NAME(m_lcd_control));
}

void sk101bl_state::machine_reset()
{
	m_lcd_control = 0;
}

HD44780_PIXEL_UPDATE(sk101bl_state::pixel_update)
{
	if (pos < 16)
		bitmap.pix16(line * 10 + y, pos * 6 + x) = state;
}

u8 sk101bl_state::p1_r()
{
	if (BIT(m_lcd_control, 1) && BIT(m_lcd_control, 3))
		return m_lcdc->read(BIT(m_lcd_control, 2));

	return m_lcd_data;
}

void sk101bl_state::p1_w(u8 data)
{
	m_lcd_data = data;
}

void sk101bl_state::p24_latch_w(u8 data)
{
	logerror("%s: Writing %02X to P2.4 latch\n", machine().describe_context(), data);
}

void sk101bl_state::lcd_control_w(u8 data)
{
	if (BIT(data, 1) && !BIT(data, 3))
		m_lcdc->write(BIT(data, 2), m_lcd_data);

	m_lcd_control = data;
}

void sk101bl_state::prog_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("program", 0);
}

void sk101bl_state::ext_map(address_map &map)
{
	map(0x1000, 0x1000).mirror(0x0fff).w(FUNC(sk101bl_state::p24_latch_w));
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
	screen.set_size(16*6, 20);
	screen.set_visarea(0, 16*6-1, 0, 20-1);
	screen.set_palette("palette");

	HD44780(config, m_lcdc).set_lcd_size(1, 16);
	m_lcdc->set_pixel_update_cb(FUNC(sk101bl_state::pixel_update), this);

	PALETTE(config, "palette").set_entries(2);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "alarm").add_route(ALL_OUTPUTS, "mono", 1.00);
}

ROM_START(sk101bl)
	ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("sk_101_5.0_1ca2.ic7", 0x0000, 0x8000, CRC(f7903ca5) SHA1(66648cc1622c1241cdbd443af706750acbb93502)) // 27256-25
ROM_END

COMP(1988, sk101bl, 0, 0, sk101bl, sk101bl, sk101bl_state, empty_init, "Reuters", "Model SK 101 BL", MACHINE_NOT_WORKING)
