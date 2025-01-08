// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

  Yamaha FB-01

***************************************************************************/

#include "emu.h"

#include "bus/midi/midi.h"
#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/nvram.h"
#include "sound/ymopm.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "fb01.lh"


namespace {

class fb01_state : public driver_device
{
public:
	fb01_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_upd71051(*this, "upd71051")
		, m_ym2164_irq(CLEAR_LINE)
		, m_upd71051_txrdy(CLEAR_LINE)
		, m_upd71051_rxrdy(CLEAR_LINE)
	{
	}

	void fb01(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void ym2164_irq_w(int state);
	void upd71051_txrdy_w(int state);
	void upd71051_rxrdy_w(int state);

	void fb01_palette(palette_device &palette) const;
	HD44780_PIXEL_UPDATE(fb01_pixel_update);

	void fb01_io(address_map &map) ATTR_COLD;
	void fb01_mem(address_map &map) ATTR_COLD;

	void update_int();

	required_device<z80_device> m_maincpu;
	required_device<i8251_device> m_upd71051;
	int m_ym2164_irq;
	int m_upd71051_txrdy;
	int m_upd71051_rxrdy;
};


void fb01_state::fb01_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).mirror(0x4000).ram().share("nvram"); // 2 * 8KB S-RAM
}


void fb01_state::fb01_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	// 00-01  YM2164
	map(0x00, 0x00).w("ym2164", FUNC(ym2164_device::address_w));
	map(0x01, 0x01).rw("ym2164", FUNC(ym2164_device::status_r), FUNC(ym2164_device::data_w));

	// 10-11  USART uPD71051C  4MHz & 4MHz / 8
	map(0x10, 0x11).rw(m_upd71051, FUNC(i8251_device::read), FUNC(i8251_device::write));

	// 20     PANEL SWITCH
	map(0x20, 0x20).portr("PANEL");

	// 30-31  HD44780A
	map(0x30, 0x31).rw("hd44780", FUNC(hd44780_device::read), FUNC(hd44780_device::write));
}


static INPUT_PORTS_START( fb01 )
	PORT_START("PANEL")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("System Set Up")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Inst Select")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Inst Assign")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Inst Function")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice Function")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Voice Select")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("-1/No")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("+1/Yes")
INPUT_PORTS_END


void fb01_state::machine_start()
{
	save_item(NAME(m_ym2164_irq));
	save_item(NAME(m_upd71051_txrdy));
	save_item(NAME(m_upd71051_rxrdy));
}


void fb01_state::machine_reset()
{
	m_upd71051->write_cts(0);
	m_upd71051->write_rxd(ASSERT_LINE);
}


void fb01_state::ym2164_irq_w(int state)
{
	m_ym2164_irq = state;
	update_int();
}


void fb01_state::upd71051_txrdy_w(int state)
{
	m_upd71051_txrdy = state;
	update_int();
}


void fb01_state::upd71051_rxrdy_w(int state)
{
	m_upd71051_rxrdy = state;
	update_int();
}


void fb01_state::update_int()
{
	m_maincpu->set_input_line(0, (m_ym2164_irq || m_upd71051_txrdy || m_upd71051_rxrdy) ? ASSERT_LINE : CLEAR_LINE);
}


HD44780_PIXEL_UPDATE(fb01_state::fb01_pixel_update)
{
	if ( pos < 8 && line < 2 )
	{
		bitmap.pix(y, line*6*8 + pos*6 + x) = state;
	}
}


void fb01_state::fb01_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(30, 0, 0));
	palette.set_pen_color(1, rgb_t(150, 0, 0));
}


void fb01_state::fb01(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(12'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &fb01_state::fb01_mem);
	m_maincpu->set_addrmap(AS_IO, &fb01_state::fb01_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(6*16, 9);
	screen.set_visarea_full();
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	config.set_default_layout(layout_fb01);

	PALETTE(config, "palette", FUNC(fb01_state::fb01_palette), 2);

	hd44780_device &hd44780(HD44780(config, "hd44780", 270'000)); // TODO: clock not measured, datasheet typical clock used
	hd44780.set_lcd_size(2, 8); // 2x8 displayed as 1x16
	hd44780.set_pixel_update_cb(FUNC(fb01_state::fb01_pixel_update));

	I8251(config, m_upd71051, XTAL(4'000'000));
	m_upd71051->rxrdy_handler().set(FUNC(fb01_state::upd71051_rxrdy_w));
	m_upd71051->txrdy_handler().set(FUNC(fb01_state::upd71051_txrdy_w));
	m_upd71051->txd_handler().set("mdout", FUNC(midi_port_device::write_txd));

	clock_device &usart_clock(CLOCK(config, "usart_clock", XTAL(4'000'000) / 8)); // 500KHz
	usart_clock.signal_handler().set(m_upd71051, FUNC(i8251_device::write_txc));
	usart_clock.signal_handler().append(m_upd71051, FUNC(i8251_device::write_rxc));

	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set("mdthru", FUNC(midi_port_device::write_txd));
	mdin.rxd_handler().append(m_upd71051, FUNC(i8251_device::write_rxd));

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	MIDI_PORT(config, "mdthru", midiout_slot, "midiout");

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	ym2164_device &ym2164(YM2164(config, "ym2164", XTAL(4'000'000)));
	ym2164.irq_handler().set(FUNC(fb01_state::ym2164_irq_w));
	ym2164.add_route(0, "lspeaker", 1.00);
	ym2164.add_route(1, "rspeaker", 1.00);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}


/* ROM definition */
ROM_START( fb01 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD("nec__-011_xb712c0__8709ex700__d27c256c-15.ic11", 0, 0x8000, CRC(7357e9a4) SHA1(049c482d6c91b7e2846757dd0f5138e0d8b687f0)) // OTP 27c256 windowless eprom?
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  STATE       INIT        COMPANY   FULLNAME                     FLAGS
CONS( 1986, fb01, 0,      0,      fb01,    fb01,  fb01_state, empty_init, "Yamaha", "FB-01 FM Sound Generator",  MACHINE_SUPPORTS_SAVE )
