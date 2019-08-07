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
    |                      ULN2003A               |N|| ||
    | :<-JP4       ________               ____    |3||_|<- LM2575T
    |    ________  SN74HC05N             TLP504A  |_|   |
    |__  MAX232CPE                                      |
    || |            MEISEI         ________             |
    || | ________  P12  P12        MAX232CPE            |
    ||_| 26LS32ACN                                   __ |
    |   :: <-JP5                                     |F||
    |  ____________   ________  ________             |U||
    |  |___CN10____|  |_CN11__| |_______|   ________ |S||
    |  |___________|  |_____  | |_CN9___|   |_CN1___| E |
    |___________________________________________________|

   XT1 = 16.000 MHz
   DP1 = 8 dipswitches
   CN4 = Speaker
   CN9 = RS232
   CN8 = Power Out
   CN6 = Magnetic stripe reader
   CN2 = Display (dot-matrix, 2 lines x 16 characters, 5x7 each character)
   CN7 = Keypad

   Status:
   - Shows current date and time, then presumably waits for keypad input
   - Needs currently unimplemented V25 features (serial etc.)

***************************************************************************/

#include "emu.h"
#include "cpu/nec/v25.h"
#include "machine/msm6242.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"

class ml20_state : public driver_device
{
public:
	ml20_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_lcdc(*this, "lcdc")
		{}

	void ml20(machine_config &config);

	void lcd_palette(palette_device &palette) const;
	HD44780_PIXEL_UPDATE(lcd_pixel_update);

protected:

private:
	required_device<cpu_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;

	void mem_map(address_map &map);
	void io_map(address_map &map);
};

void ml20_state::mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).ram();
	map(0xf0000, 0xfffff).rom().region("bios", 0);
}

void ml20_state::io_map(address_map &map)
{
	map(0x00, 0x0f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write));
	map(0x10, 0x10).rw(m_lcdc, FUNC(hd44780_device::data_read), FUNC(hd44780_device::data_write));
	map(0x14, 0x14).rw(m_lcdc, FUNC(hd44780_device::control_read), FUNC(hd44780_device::control_write));
}

static INPUT_PORTS_START( ml20 )
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
		bitmap.pix16(1 + y + line*8 + line, 1 + pos*6 + x) = state ? 1 : 2;
}

void ml20_state::ml20(machine_config &config)
{
	V25(config, m_maincpu, 16_MHz_XTAL / 2); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &ml20_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &ml20_state::io_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(6*16+1, 18);
	screen.set_visarea_full();
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(ml20_state::lcd_palette), 3);

	HD44780(config, m_lcdc, 0);
	m_lcdc->set_lcd_size(2, 16);
	m_lcdc->set_pixel_update_cb(FUNC(ml20_state::lcd_pixel_update), this);

	MSM6242(config, "rtc", 32.768_kHz_XTAL);
}

ROM_START( ml20 )
	ROM_REGION(0x10000, "bios", 0)
	ROM_LOAD("ml-20_v1.27.ic4", 0x0000, 0x10000, CRC(844d6d23) SHA1(08cd290bc342da328abc91b0699662c9ba335c0d) )
ROM_END

//    YEAR  NAME  PARENT  COMPAT  MACHINE INPUT  CLASS       INIT        COMPANY    FULLNAME         FLAGS
COMP( 1999, ml20, 0,      0,      ml20,   ml20,  ml20_state, empty_init, "Digitek", "Micrologic 20", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE)
