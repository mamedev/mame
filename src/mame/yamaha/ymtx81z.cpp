// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Driver for Yamaha TX81Z.

****************************************************************************/

#include "emu.h"

#include "bus/midi/midi.h"
#include "cpu/m6800/m6801.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "sound/ymopz.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "tx81z.lh"


namespace {

class ymtx81z_state : public driver_device
{
public:
	ymtx81z_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_port2(*this, "P2")
	{
	}

	void tx81z(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	HD44780_PIXEL_UPDATE(lcd_pixel_update);
	void palette_init(palette_device &palette);

	void mem_map(address_map &map) ATTR_COLD;

	u8 p2_r();
	void midi_rx_r(int state) { m_rx_data = state; }
	void midiclock_w(int state) { if (state) m_maincpu->clock_serial(); }

	required_device<hd6303x_cpu_device> m_maincpu;
	required_ioport m_port2;

	int m_rx_data;
};

HD44780_PIXEL_UPDATE(ymtx81z_state::lcd_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 16)
		bitmap.pix(line * 10 + y + 1 + ((y == 7) ? 1 : 0), pos * 6 + x + 1) = state ? 1 : 2;
}

void ymtx81z_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(0x00, 0x00, 0x00)); // background
	palette.set_pen_color(1, rgb_t(0xff, 0xff, 0xff)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(0x18, 0x18, 0x18)); // lcd pixel off
}

void ymtx81z_state::machine_start()
{
	membank("rombank")->configure_entries(0, 2, memregion("program")->base(), 0x8000);
	m_rx_data = ASSERT_LINE;
}

void ymtx81z_state::mem_map(address_map &map)
{
	map(0x2000, 0x2001).mirror(0x1ffe).rw("ymsnd", FUNC(ym2414_device::read), FUNC(ym2414_device::write));
	map(0x4000, 0x4001).mirror(0x1ffe).rw("lcdc", FUNC(hd44780_device::read), FUNC(hd44780_device::write));
	map(0x6000, 0x7fff).ram().share("nvram");
	map(0x8000, 0xffff).bankr("rombank");
}

u8 ymtx81z_state::p2_r()
{
	u8 result = m_port2->read() & 0xf7;
	result |= (m_rx_data << 3);
	return result;
}

static INPUT_PORTS_START(tx81z)
	PORT_START("P2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED) // actually cassette data
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED) // actually 500 kHz clock
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED) // actually MIDI In data
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Cursor")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_VOLUME_UP) PORT_NAME(u8"Master Volume \u2192") // →
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_VOLUME_DOWN) PORT_NAME(u8"Master Volume \u2190") // ←

	PORT_START("P5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED) // actually INT1 from YM2414
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON9) PORT_NAME("Store/Eq Copy")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED) // actually MR from analog wait circuit tied to LCDE
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Data Entry Inc")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Data Entry Dec")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME(u8"Voice Parameter \u2192") // →
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME(u8"Voice Parameter \u2190") // ←

	PORT_START("P6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_NAME("Play/Perform")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON7) PORT_NAME("Edit/Compare")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON8) PORT_NAME("Utility")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

void ymtx81z_state::tx81z(machine_config &config)
{
	HD6303X(config, m_maincpu, 7.15909_MHz_XTAL); // HD63B03XP
	m_maincpu->set_addrmap(AS_PROGRAM, &ymtx81z_state::mem_map);
	m_maincpu->in_p2_cb().set(FUNC(ymtx81z_state::p2_r));
	m_maincpu->in_p5_cb().set_ioport("P5");
	m_maincpu->in_p6_cb().set_ioport("P6");
	m_maincpu->out_p6_cb().set_membank("rombank").bit(3);
	m_maincpu->out_p6_cb().append_output("led1").bit(4);
	m_maincpu->out_p6_cb().append_output("led2").bit(5);
	m_maincpu->out_p6_cb().append_output("led3").bit(6);
	m_maincpu->out_p6_cb().append_output("led4").bit(7);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // TC5564PL-15/-20 + CR2032 battery

	auto &midiclock(CLOCK(config, "midiclock", 500_kHz_XTAL));
	midiclock.signal_handler().set(FUNC(ymtx81z_state::midiclock_w));

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(FUNC(ymtx81z_state::midi_rx_r));

	auto &mdout(MIDI_PORT(config, "mdout", midiout_slot, "midiout"));
	m_maincpu->out_ser_tx_cb().set(mdout, FUNC(midi_port_device::write_txd));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6*16+1, 10*2+1);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(ymtx81z_state::palette_init), 3);

	config.set_default_layout(layout_tx81z);

	hd44780_device &lcdc(HD44780(config, "lcdc", 270'000)); // TODO: clock not measured, datasheet typical clock used
	lcdc.set_lcd_size(2, 16);
	lcdc.set_pixel_update_cb(FUNC(ymtx81z_state::lcd_pixel_update));

	SPEAKER(config, "speaker", 2).front();

	ym2414_device &ymsnd(YM2414(config, "ymsnd", 7.15909_MHz_XTAL / 2));
	ymsnd.irq_handler().set_inputline(m_maincpu, HD6301_IRQ1_LINE);
	ymsnd.add_route(0, "speaker", 0.60, 0);
	ymsnd.add_route(1, "speaker", 0.60, 1);
}

ROM_START(tx81z)
	ROM_REGION(0x10000, "program", 0)
	ROM_SYSTEM_BIOS(0, "v16", "Version 1.6")
	ROMX_LOAD("tx81z-v1.6.ic15", 0x00000, 0x10000, CRC(ab9b7347) SHA1(208a72c0dc615825c442240e520a6a3c5fe860ea), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v15", "Version 1.5")
	ROMX_LOAD("tx81z-27512-image-version-1_5.ic15", 0x00000, 0x10000, CRC(64ab615b) SHA1(82cdd8637caf3828aee5ccf25f1ed92ae5d65d3b), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v14", "Version 1.4")
	ROMX_LOAD("tx81z-v1.4.ic15", 0x00000, 0x10000, CRC(694a13e2) SHA1(0b656a8040748f1e4ee73df2a9436fee1c724be8), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v13", "Version 1.3")
	ROMX_LOAD("tx81z-v1.3.ic15", 0x00000, 0x10000, CRC(7abd5a61) SHA1(93ae5498ce650fe09952ca81c9ac3821f44c20dc), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v12", "Version 1.2")
	ROMX_LOAD("tx81z-v1.2.ic15", 0x00000, 0x10000, CRC(8378f744) SHA1(d76b573d4deb67f0e1553c9c17804e970b392803), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "v11", "Version 1.1")
	ROMX_LOAD("tx81z-v1.1.ic15", 0x00000, 0x10000, CRC(3e78db9f) SHA1(52eafb9a1cb3ffb68e8b8dd7a2b85d9e607f9e1c), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "v10", "Version 1.0")
	ROMX_LOAD("tx81z-27512-image-first-version-1_0.ic15", 0x00000, 0x10000, CRC(2f9628fa) SHA1(ce62dfb9a86da092c469fd25328b5447375f5bb2), ROM_BIOS(6))
ROM_END

} // anonymous namespace


SYST(1987, tx81z, 0, 0, tx81z, tx81z, ymtx81z_state, empty_init, "Yamaha", "TX81Z FM Tone Generator", MACHINE_IMPERFECT_SOUND)
