// license:BSD-3-Clause
// copyright-holders:R. Belmont, superctr
/*
	Yamaha TG100 AWM Tone Generator
	Driver by R. Belmont, based on a skeleton written by superctr

	Roms dumped by vampirefrog
	Service manual scanned by bmos

	CPU: Hitachi HD6435208A00P (H8/520)
	- 20MHz clock
	- Mode bits 0,1 high, 2 low
	- 32kb RAM (1x HM65256 PSRAM)
	Sound generator: Yamaha YMW258-F (GEW8, identical to MultiPCM?)
	- 9.4MHz clock
	- 28 voices polyphony
	- 2MB sample ROM containing 140 12-bit PCM samples
	Effect DSP: Yamaha YM3413
	- clocked by sound generator
	- Effect memory: 64kb (2x HM65256 PSRAM)
	LCD:
	- 1x16 characters

	Other ICs:
	HG62E11R54FS (XK462A00) Gate array (LCD control, glue logic)

	TODO:
	- Investigate YM3413 effect DSP
	- LEDs
	- Layout
*/

#include "emu.h"

#include "bus/midi/midi.h"
#include "cpu/h8500/h8520.h"
#include "machine/eepromser.h"
#include "sound/multipcm.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class tg100_state : public driver_device
{
public:
	tg100_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_lcd(*this, "lcd"),
		m_ymw258(*this, "ymw258"),
		m_eeprom(*this, "eeprom"),
		m_mdout(*this, "mdout"),
		m_host_select(*this, "HOST_SELECT"),
		m_loopback(*this, "LOOPBACK")
	{ }

	void tg100(machine_config &config);

	HD44780_PIXEL_UPDATE(lcd_pixel_update);

protected:
	virtual void machine_reset() override;

private:
	required_device<h8520_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<hd44780_device> m_lcd;
	required_device<multipcm_device> m_ymw258;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<midi_port_device> m_mdout;
	required_ioport m_host_select;
	required_ioport m_loopback;

	void tg100_map(address_map &map) ATTR_COLD;
	void ymw258_map(address_map &map) ATTR_COLD;
	void tg_palette(palette_device &palette) const;

	u8 port5_r();
	void port5_w(u8 data);
	u16 host_select_r();
	void midi_tx_w(int state);
};

u16 tg100_state::host_select_r()
{
	// the rear-panel HOST SELECT switch is a resistor ladder on AN1:
	// 0V = MIDI, 1/3 VCC = PC-2, 2/3 VCC = PC-1, VCC = Macintosh
	return m_host_select->read() * 0x3ff / 3;
}

void tg100_state::midi_tx_w(int state)
{
	m_mdout->write_txd(state);

	// Test mode (hold CURSOR, -/NO and +/YES at power-up) requires a MIDI
	// loopback cable or test mode will not enter and the module will boot normally.
	if (m_loopback->read() & 1)
	{
		m_maincpu->sci_rx_w<1>(state);
	}
}

void tg100_state::machine_reset()
{
	// IRQ0 is level triggered and tied high.  The firmware uses it as a
	// sort of software interrupt by toggling it on in the interrupt controller
	// whenever it needs that particular task to run.
	m_maincpu->set_input_line(0, ASSERT_LINE);
}

u8 tg100_state::port5_r()
{
	return m_eeprom->do_read() ? 0x01 : 0x00;
}

void tg100_state::port5_w(u8 data)
{
	m_eeprom->cs_write(BIT(data, 3));
	m_eeprom->clk_write(BIT(data, 2));
	m_eeprom->di_write(BIT(data, 1));
}

void tg100_state::tg_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(88, 247, 0));
	palette.set_pen_color(1, rgb_t(3, 3, 60));
}

HD44780_PIXEL_UPDATE(tg100_state::lcd_pixel_update)
{
	// 1x16 character module is wired to the HD44780 as 2x8
	if (x < 5 && y < 8 && line < 2 && pos < 8)
	{
		bitmap.pix(y, (line * 8 + pos) * 6 + x) = state;
	}
}

void tg100_state::tg100_map(address_map &map)
{
	map(0x0000'0000, 0x0000'7fff).rom().region("prgrom", 0x00000);
	map(0x0000'9000, 0x0000'9003).rw(m_ymw258, FUNC(multipcm_device::read), FUNC(multipcm_device::write)).umask16(0xffff);
	map(0x0000'a000, 0x0000'a000).portr("BUTTONS");
	map(0x0000'e000, 0x0000'e001).w(m_lcd, FUNC(hd44780_device::write));
	map(0x0000'f000, 0x0000'f001).r(m_lcd, FUNC(hd44780_device::read));
	map(0x0008'0000, 0x0009'ffff).rom().region("prgrom", 0x00000);
	map(0x000c'0000, 0x000c'7fff).ram();
}

static INPUT_PORTS_START( tg100 )
	PORT_START("BUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Cursor")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("-/No")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("+/Yes")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("Edit")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME("Part")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_NAME("Play")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("HOST_SELECT")
	PORT_CONFNAME(0x03, 0x00, "Host Select")
	PORT_CONFSETTING(0x00, "MIDI")
	PORT_CONFSETTING(0x01, "PC-2 (NEC PC-98)")
	PORT_CONFSETTING(0x02, "PC-1 (IBM PC AT)")
	PORT_CONFSETTING(0x03, "Macintosh")

	PORT_START("LOOPBACK")
	PORT_CONFNAME(0x01, 0x00, "MIDI Loopback")
	PORT_CONFSETTING(0x00, DEF_STR(Off))
	PORT_CONFSETTING(0x01, DEF_STR(On))
INPUT_PORTS_END

void tg100_state::ymw258_map(address_map &map)
{
	map(0x00'0000, 0x1f'ffff).rom();
}

void tg100_state::tg100(machine_config &config)
{
	HD6435208(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tg100_state::tg100_map);
	m_maincpu->read_port5().set(FUNC(tg100_state::port5_r));
	m_maincpu->write_port5().set(FUNC(tg100_state::port5_w));
	m_maincpu->read_adc<0>().set_constant(0); // ADC 0 isn't connected, but its scan group is enabled
	m_maincpu->read_adc<1>().set(FUNC(tg100_state::host_select_r));
	m_maincpu->write_sci_tx<1>().set(FUNC(tg100_state::midi_tx_w));

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(m_maincpu, FUNC(h8520_device::sci_rx_w<1>));

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	EEPROM_93C46_16BIT(config, m_eeprom);

	SPEAKER(config, "speaker", 2).front();

	MULTIPCM(config, m_ymw258, 9.4_MHz_XTAL);
	m_ymw258->set_addrmap(0, &tg100_state::ymw258_map);
	m_ymw258->add_route(0, "speaker", 1.0, 0);
	m_ymw258->add_route(1, "speaker", 1.0, 1);

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(80);
	m_screen->set_size(240, 64); // 1x16, 8x8
	m_screen->set_visarea(0, 240 - 1, 0, 64 - 1);
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(tg100_state::tg_palette), 2);

	HD44780(config, m_lcd, 4.9152_MHz_XTAL / 4);
	m_lcd->set_lcd_size(2, 8);
	m_lcd->set_pixel_update_cb(FUNC(tg100_state::lcd_pixel_update));

	m_screen->set_screen_update(m_lcd, FUNC(hd44780_device::screen_update));
	m_screen->set_size(6 * 16, 8 * 1);
	m_screen->set_visarea_full();
}

ROM_START( tg100 )
	ROM_REGION(0x20000, "prgrom", 0)
	ROM_LOAD( "xk731c0.ic4", 0x00000, 0x20000, CRC(8fb6139c) SHA1(483103a2ffc63a90a2086c597baa2b2745c3a1c2) )

	ROM_REGION(0x4000, "maincpu", 0)
	ROM_COPY( "prgrom", 0x0000, 0x0000, 0x4000 )

	ROM_REGION(0x200000, "ymw258", 0)
	ROM_LOAD( "xk992a0.ic6", 0x000000, 0x200000, CRC(01dc6954) SHA1(32ec77a46f4d005538c735f56ad48fa7243c63be) )
ROM_END

} // anonymous namespace


//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY   FULLNAME      FLAGS
CONS( 1991, tg100, 0,      0,      tg100,   tg100, tg100_state, empty_init, "Yamaha", "TG100",      MACHINE_IMPERFECT_SOUND )
