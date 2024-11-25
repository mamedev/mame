// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Digitek Micrologic 20

    Access control device with dot-matrix display

  ML20/232
  ____________________________________________________
  |   ________  ________  ________  ______    __      |
  |   |74HC04N|M74HC139B1 |__DP1__| |_XT1_| : CN7     |
  |__ _______________    ____________      JP2        |
  ||C||IC4 M27C512  ||   |D70320L-8  |       (CN4)__  |
  ||N||_____________||   |NEC V25    |   ______   |C| |
  ||2|_______________    |           |   |BATT |  |N| |
  ||_||KM681000CLP-7L|   |           |   |_____|  |6| |
  |__ |______________|   |___________|   o <- LED     |
  ||C| ________          ________       .. <- JP1     |
  ||N||_M6242B_|         MAX693CPE              __    |
  ||8|                   ________               |C|__ |
  |         XT2          ULN2003A               |N|| ||
  | :<-JP4       ________               ____    |3||_|<- LM2575T
  |    ________  SN74HC05N             TLP504A  |_|   |
  |__  MAX232CPE                :<-JP8                |
  || |            MEISEI         ________             |
  || | ________  P12  P12        MAX232CPE            |
  ||_| 26LS32ACN                                   __ |
  |   :: <-JP5                                     |F||
  |  ____________   ________  ________             |U||
  |  |___CN10____|  |_CN11__| |_______|   ________ |S||
  |  |___________|  |_____  | |_CN9___|   |_CN1___| E |
  |___________________________________________________|

 XT1 = 16.000 MHz
 XT2 = S873
 DP1 = 8 dipswitches
 CN4 = Speaker
 CN9 = RS232
 CN8 = Power Out
 CN6 = Magnetic stripe reader
 CN2 = Display (dot-matrix, 2 lines x 16 characters, 5x7 each character)
 CN7 = Keypad

 Display = Hyundai HC16203-A (Hitachi HD44780A00 based).

   Status:
   - Needs currently unimplemented V25 features (serial etc.)

***************************************************************************/

#include "emu.h"
#include "cpu/nec/v25.h"
#include "machine/msm6242.h"
#include "video/hd44780.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "ml20.lh"


namespace {

class ml20_state : public driver_device
{
public:
	ml20_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_lcdc(*this, "lcdc"),
		m_speaker(*this, "speaker"),
		m_keys(*this, "COL.%d", 0U),
		m_dsw(*this, "DSW")
		{}

	void ml20(machine_config &config);

	void lcd_palette(palette_device &palette) const;
	HD44780_PIXEL_UPDATE(lcd_pixel_update);

protected:
	void machine_start() override ATTR_COLD;

private:
	required_device<v25_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
	required_device<speaker_sound_device> m_speaker;
	required_ioport_array<4> m_keys;
	required_ioport m_dsw;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint8_t p0_r();
	uint8_t p1_r();
	uint8_t p2_r();
	void p0_w(uint8_t data);
	void p1_w(uint8_t data);
	void p2_w(uint8_t data);
	uint8_t pt_r();

	uint8_t m_p0;
	uint8_t m_p1;
	uint8_t m_p2;
};

void ml20_state::mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).ram();
	map(0xf0000, 0xfffff).rom().region("bios", 0);
}

void ml20_state::io_map(address_map &map)
{
	map(0x00, 0x0f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write));
	map(0x10, 0x10).rw(m_lcdc, FUNC(hd44780_device::data_r), FUNC(hd44780_device::data_w));
	map(0x14, 0x14).rw(m_lcdc, FUNC(hd44780_device::control_r), FUNC(hd44780_device::control_w));
}

static INPUT_PORTS_START( ml20 )
	PORT_START("COL.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("ENTER")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)     PORT_NAME("0")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)   PORT_NAME("CLEAR")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)     PORT_NAME("ler. MARC")
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("COL.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_NAME("9")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_NAME("8")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_NAME("7")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_NAME("MENSAJ.")
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("COL.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_NAME("6")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_NAME("5")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_NAME("4")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_NAME("SALDOS")
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("COL.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_NAME("3")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_NAME("2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_NAME("1")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_NAME("ANULAR")
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("DSW")
	PORT_DIPUNKNOWN(0x01, 0x01)
	PORT_DIPUNKNOWN(0x02, 0x02)
	PORT_DIPUNKNOWN(0x04, 0x04)
	PORT_DIPUNKNOWN(0x08, 0x08)
	PORT_DIPUNKNOWN(0x10, 0x10)
	PORT_DIPUNKNOWN(0x20, 0x20)
	PORT_DIPUNKNOWN(0x40, 0x40)
	PORT_DIPUNKNOWN(0x80, 0x80)
INPUT_PORTS_END

void ml20_state::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t( 92,  83,  88)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // lcd pixel off
}

HD44780_PIXEL_UPDATE( ml20_state::lcd_pixel_update )
{
	// char size is 5x7
	if (x > 4 || y > 6)
		return;

	if (line < 2 && pos < 16)
		bitmap.pix(1 + y + line*8 + line, 1 + pos*6 + x) = state ? 1 : 2;
}

// 7654----  unknown (set to input)
// ----3210  keypad column

uint8_t ml20_state::p0_r()
{
	return m_p0;
}

void ml20_state::p0_w(uint8_t data)
{
	if (0)
		logerror("p0_w: %d %d %d %d  %d %d %d %d\n", BIT(data, 7), BIT(data, 6), BIT(data, 5), BIT(data, 4), BIT(data, 3), BIT(data, 2), BIT(data, 1), BIT(data, 0));

	m_p0 = data;
}

// 765-----  unknown (set to output)
// ---43210  unknown (set to input)

uint8_t ml20_state::p1_r()
{
	return m_p1;
}

void ml20_state::p1_w(uint8_t data)
{
	if (0)
		logerror("p1_w: %d %d %d %d  %d %d %d %d\n", BIT(data, 7), BIT(data, 6), BIT(data, 5), BIT(data, 4), BIT(data, 3), BIT(data, 2), BIT(data, 1), BIT(data, 0));

	m_p1 = data;
}

// 7-------  some kind of heartbeat? led?
// -6------  unknown (set to output)
// --543---  unknown (set to input)
// -----2--  set when waiting for keypad or other data?
// ------1-  toggles continously
// -------0  unknown (set to output)

uint8_t ml20_state::p2_r()
{
	return m_p2;
}

void ml20_state::p2_w(uint8_t data)
{
	if (0)
		logerror("p2_w: %d %d %d %d  %d %d %d %d\n", BIT(data, 7), BIT(data, 6), BIT(data, 5), BIT(data, 4), BIT(data, 3), BIT(data, 2), BIT(data, 1), BIT(data, 0));

	m_p2 = data;
}

uint8_t ml20_state::pt_r()
{
	uint8_t data = 0xff;

	if (BIT(m_p0, 0) == 0) data &= m_keys[0]->read();
	if (BIT(m_p0, 1) == 0) data &= m_keys[1]->read();
	if (BIT(m_p0, 2) == 0) data &= m_keys[2]->read();
	if (BIT(m_p0, 3) == 0) data &= m_keys[3]->read();

	return data;
}

void ml20_state::machine_start()
{
	// register for save states
	save_item(NAME(m_p0));
	save_item(NAME(m_p1));
	save_item(NAME(m_p2));
}

void ml20_state::ml20(machine_config &config)
{
	V25(config, m_maincpu, 16_MHz_XTAL / 2); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &ml20_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &ml20_state::io_map);
	m_maincpu->p0_in_cb().set(FUNC(ml20_state::p0_r));
	m_maincpu->p0_out_cb().set(FUNC(ml20_state::p0_w));
	m_maincpu->p1_in_cb().set(FUNC(ml20_state::p1_r));
	m_maincpu->p1_out_cb().set(FUNC(ml20_state::p1_w));
	m_maincpu->p2_in_cb().set(FUNC(ml20_state::p2_r));
	m_maincpu->p2_out_cb().set(FUNC(ml20_state::p2_w));
	m_maincpu->pt_in_cb().set(FUNC(ml20_state::pt_r));

	MSM6242(config, "rtc", 32.768_kHz_XTAL);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(6*16+1, 18);
	screen.set_visarea_full();
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(ml20_state::lcd_palette), 3);

	HD44780(config, m_lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_lcdc->set_lcd_size(2, 16);
	m_lcdc->set_pixel_update_cb(FUNC(ml20_state::lcd_pixel_update));

	config.set_default_layout(layout_ml20);

	// sound
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

ROM_START( ml20 )
	ROM_REGION(0x10000, "bios", 0)
	ROM_LOAD("ml-20_v1.27.ic4", 0x0000, 0x10000, CRC(844d6d23) SHA1(08cd290bc342da328abc91b0699662c9ba335c0d) )
ROM_END

} // anonymous namespace


//    YEAR  NAME  PARENT  COMPAT  MACHINE INPUT  CLASS       INIT        COMPANY    FULLNAME         FLAGS
COMP( 1999, ml20, 0,      0,      ml20,   ml20,  ml20_state, empty_init, "Digitek", "Micrologic 20", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE)
