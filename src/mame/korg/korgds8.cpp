// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Korg DS-8 & 707 synthesizers.

****************************************************************************/

#include "emu.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
//#include "bus/midi/midi.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/nvram.h"
#include "sound/ymopm.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class korg_ds8_state : public driver_device
{
public:
	korg_ds8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_card(*this, "card")
		, m_kbd(*this, "KBD%u", 0U)
		, m_sw(*this, "SW%u", 0U)
		, m_scan(0)
		, m_card_bank(0)
	{
	}

	void ds8(machine_config &config);
	void korg707(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	HD44780_PIXEL_UPDATE(lcd_pixel_update);
	HD44780_PIXEL_UPDATE(korg707_pixel_update);

	u8 kbd_sw_r();
	void scan_w(u8 data);
	u8 card_r(offs_t offset);
	void card_w(offs_t offset, u8 data);
	void ddl_w(u8 data);
	void line_mute_w(u8 data);
	void led_data_w(offs_t offset, u8 data);

	void mem_map(address_map &map) ATTR_COLD;

	void palette_init_ds8(palette_device &palette);

	required_device<upd7810_device> m_maincpu;
	required_device<generic_slot_device> m_card;
	required_ioport_array<16> m_kbd;
	optional_ioport_array<8> m_sw;

	u8 m_scan;
	u16 m_card_bank;
};

void korg_ds8_state::machine_start()
{
	save_item(NAME(m_scan));
	save_item(NAME(m_card_bank));
}

HD44780_PIXEL_UPDATE(korg_ds8_state::lcd_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 40)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}

HD44780_PIXEL_UPDATE(korg_ds8_state::korg707_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 20)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}

u8 korg_ds8_state::kbd_sw_r()
{
	if (BIT(m_scan, 4))
		return m_sw[m_scan & 0x07].read_safe(0xff);
	else
		return m_kbd[m_scan & 0x0f]->read();
}

void korg_ds8_state::scan_w(u8 data)
{
	m_scan = data;
}

u8 korg_ds8_state::card_r(offs_t offset)
{
	return m_card->read_ram(offset | m_card_bank);
}

void korg_ds8_state::card_w(offs_t offset, u8 data)
{
	m_card->write_ram(offset | m_card_bank, data);
}

void korg_ds8_state::ddl_w(u8 data)
{
}

void korg_ds8_state::line_mute_w(u8 data)
{
	m_card_bank = (BIT(data, 7) ? 0x2000 : 0) | (BIT(data, 6) ? 0x4000 : 0);
}

void korg_ds8_state::led_data_w(offs_t offset, u8 data)
{
}

void korg_ds8_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("program", 0);
	map(0x8000, 0x9fff).rw(FUNC(korg_ds8_state::card_r), FUNC(korg_ds8_state::card_w));
	map(0xa000, 0xbfff).ram().share("nvram");
	map(0xc000, 0xc7ff).ram();
	map(0xe000, 0xe001).mirror(0x1e).rw("fm", FUNC(ym2164_device::read), FUNC(ym2164_device::write));
	map(0xe020, 0xe020).mirror(0x1f).w(FUNC(korg_ds8_state::ddl_w));
	map(0xe040, 0xe040).mirror(0x1f).w(FUNC(korg_ds8_state::line_mute_w));
	map(0xe060, 0xe060).mirror(0x1f).rw("lcdc", FUNC(hd44780_device::db_r), FUNC(hd44780_device::db_w));
	map(0xe080, 0xe081).mirror(0x1e).w(FUNC(korg_ds8_state::led_data_w));
}


static INPUT_PORTS_START(ds8)
	PORT_START("KBD0") // CN3A/B-1
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KBD1") // CN3A/B-2
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KBD2") // CN3A/B-3
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KBD3") // CN3A/B-4
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KBD4") // CN3A/B-5
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KBD5") // CN3A/B-6
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KBD6") // CN3A/B-7
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KBD7") // CN3A/B-8
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KBD8") // CN3A/B-9
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KBD9") // CN4A/B-1
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KBD10") // CN4A/B-2
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KBD11") // CN4A/B-3
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KBD12") // CN4A/B-4
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KBD13") // CN4A/B-5
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KBD14") // CN4A/B-6
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KBD15") // CN4A/B-7
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SW0") // CN5A/B-5
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SW1") // CN5A/B-5
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SW2") // CN5A/B-5
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SW3") // CN5A/B-5
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SW4") // CN5A/B-1
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END

void korg_ds8_state::palette_init_ds8(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(131, 136, 139));
	palette.set_pen_color(1, rgb_t( 92,  83,  88));
}

void korg_ds8_state::ds8(machine_config &config)
{
	UPD78C10(config, m_maincpu, 12_MHz_XTAL); // µPD78C10CW
	m_maincpu->set_addrmap(AS_PROGRAM, &korg_ds8_state::mem_map);
	m_maincpu->pa_in_cb().set(FUNC(korg_ds8_state::kbd_sw_r));
	m_maincpu->pb_out_cb().set(FUNC(korg_ds8_state::scan_w));
	m_maincpu->pc_out_cb().set("lcdc", FUNC(hd44780_device::rs_w)).bit(5);
	m_maincpu->pc_out_cb().append("lcdc", FUNC(hd44780_device::rw_w)).bit(6);
	m_maincpu->pc_out_cb().append("lcdc", FUNC(hd44780_device::e_w)).bit(7);
	m_maincpu->set_pc_pullups(0x0e);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // µPD4364 + battery

	GENERIC_CARTSLOT(config, m_card, generic_plain_slot, nullptr, "ds8_card");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6*40, 8*2);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(korg_ds8_state::palette_init_ds8), 2);

	hd44780_device &lcdc(HD44780(config, "lcdc", 270'000)); // TODO: clock not measured, datasheet typical clock used
	lcdc.set_lcd_size(2, 40);
	lcdc.set_pixel_update_cb(FUNC(korg_ds8_state::lcd_pixel_update));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2164_device &fm(YM2164(config, "fm", 3.579545_MHz_XTAL)); // YM2164 + YM3012
	fm.add_route(0, "lspeaker", 1.00);
	fm.add_route(1, "rspeaker", 1.00);
}

void korg_ds8_state::korg707(machine_config &config)
{
	ds8(config);

	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_size(6*20, 8*2);
	screen.set_visarea_full();

	hd44780_device &lcdc(*subdevice<hd44780_device>("lcdc"));
	lcdc.set_lcd_size(2, 20);
	lcdc.set_pixel_update_cb(FUNC(korg_ds8_state::korg707_pixel_update));
}

ROM_START(ds8)
	ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("870214.ic9", 0x0000, 0x8000, CRC(6b418e7c) SHA1(f16f9d87f1d424335a04ab36fef9386ed0b7b159))
ROM_END

ROM_START(korg707)
	ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("870904.ic7", 0x0000, 0x8000, CRC(3eb80aae) SHA1(8574f48c9a1724c483ac8ee7c82ea46d1f583d6d)) // 27C256
ROM_END

} // anonymous namespace


SYST(1986, ds8,     0, 0, ds8,     ds8, korg_ds8_state, empty_init, "Korg", "DS-8 Digital Synthesizer",    MACHINE_IS_SKELETON)
SYST(1987, korg707, 0, 0, korg707, ds8, korg_ds8_state, empty_init, "Korg", "707 Performing Synthesizer", MACHINE_IS_SKELETON)
