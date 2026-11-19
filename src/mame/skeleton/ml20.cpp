// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Micrologic-20 from Primion Digitek (now part of the Azkoyen Group)

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
 DP1 = Bank of8 DIP switches
 CN4 = Speaker
 CN9 = RS232
 CN8 = Power Out
 CN6 = Magnetic stripe reader
 CN2 = Display (dot-matrix, 2 lines x 16 characters, 5x7 each character)
 CN7 = Keypad

 Display = Hyundai HC16203-A (Hitachi HD44780A00 based).


 The Micrologic-20 100-NET-1 Bio is a much newer version of the
 Micrologic-20, with Ethernet, fingerprint reader, RFID interface and
 smartcard reader, with the following hardware:

 Main PCB:
 * NEC V25 (D70320GJ-8) CPU near a 16 MHz xtal
 * RT93LC66-A SEEPROM
 * R1LP0408CSP-5SI SRAM
 * SMSC LAN91C111-NS Ethernet near a 25 MHz xtal
 * Epson RTC72423 RTC
 * Maxim MAX693CWE
 * Battery
 * Bank of 8 DIP switches
 * Small subboard with a ST3232C RS-232 driver

 RFID PCB:
 * Atmel Atmega 128A near a 15 MHz xtal
 * NXP CLRC632 near a 13.560 MHz xtal
 * Bank of two DIP switches
 * AT25256B SEEPROM

 Fingerprint reader PCB (Suprema SFM3050-4M):
 * Analog Devices ADSP-BF531.
 * AT88SC153 SEEPROM
 * 39VF3201B-70-4I-EKE 32Mb Flash memory
 * IS42S16100H-7TLI 16M DRAM
 * 20 MHz xtal
 * LC245A
 * TCS1CM fingerprint scanning module

 Smartcard reader PCB:
 * Atmel GC144GH-UL near a 147 xtal
 * Texas Instruments MB3232I RS-232 driver


   Status:
   - ml20: the firmware moves its serial data and reads the magnetic stripe
     cards (INTP1/INTP2) with the V25 interrupt macro service, which isn't
     emulated, so the serial ports stay silent
   - ml20nb: stops at "UN MOMENTO, POR FAVOR" waiting for the fingerprint
     module on serial channel 1 (Suprema packets at 115200 baud); the
     smartcard reader it looks for on channel 0 (Gemplus block protocol),
     the LAN91C111 and the card readers on INTP1/INTP2 aren't emulated
     either

***************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/nec/v25.h"
#include "machine/eepromser.h"
#include "machine/intelfsh.h"
#include "machine/msm6242.h"
#include "sound/spkrdev.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "ml20.lh"

#define LOG_PORTS (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

#define LOGPORTS(...) LOGMASKED(LOG_PORTS, __VA_ARGS__)


namespace {

class ml20_base_state : public driver_device
{
protected:
	ml20_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_lcdc(*this, "lcdc"),
		m_rtc(*this, "rtc"),
		m_speaker(*this, "speaker"),
		m_keys(*this, "COL.%u", 0U)
	{
	}

	virtual void machine_start() override ATTR_COLD;

	void ml20_common(machine_config &config) ATTR_COLD;
	void common_io_map(address_map &map) ATTR_COLD;

	uint8_t keypad_rows(uint8_t columns);

	required_device<v25_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
	required_device<msm6242_device> m_rtc;
	required_device<speaker_sound_device> m_speaker;
	required_ioport_array<4> m_keys;

	uint8_t m_p0;
	uint8_t m_p1;
	uint8_t m_p2;

private:
	void lcd_palette(palette_device &palette) const ATTR_COLD;
	HD44780_PIXEL_UPDATE(lcd_pixel_update);

	uint8_t p1_r();
	void p1_w(uint8_t data);
	uint8_t p2_r();
	void p2_w(uint8_t data);
};

class ml20_state : public ml20_base_state
{
public:
	ml20_state(const machine_config &mconfig, device_type type, const char *tag) :
		ml20_base_state(mconfig, type, tag)
	{
	}

	void ml20(machine_config &config) ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	uint8_t p0_r();
	void p0_w(uint8_t data);
	uint8_t pt_r();
};

class ml20nb_state : public ml20_base_state
{
public:
	ml20nb_state(const machine_config &mconfig, device_type type, const char *tag) :
		ml20_base_state(mconfig, type, tag),
		m_flash(*this, "flash"),
		m_eeprom(*this, "eeprom"),
		m_dsw(*this, "DSW")
	{
	}

	void ml20nb(machine_config &config) ATTR_COLD;

private:
	required_device<intelfsh8_device> m_flash;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_ioport m_dsw;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint8_t p0_r();
	void p0_w(uint8_t data);
	uint8_t keypad_r(offs_t offset);
};

void ml20_state::mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).ram();
	map(0xf0000, 0xfffff).rom().region("bios", 0);
}

void ml20_base_state::common_io_map(address_map &map)
{
	map(0x00, 0x0f).rw(m_rtc, FUNC(msm6242_device::read), FUNC(msm6242_device::write));
	map(0x10, 0x10).rw(m_lcdc, FUNC(hd44780_device::data_r), FUNC(hd44780_device::data_w));
	map(0x14, 0x14).rw(m_lcdc, FUNC(hd44780_device::control_r), FUNC(hd44780_device::control_w));
}

void ml20nb_state::mem_map(address_map &map)
{
	map(0x00000, 0x7ffff).ram();
	map(0x7e000, 0x7e00f).noprw(); // TODO: LAN91C111 Ethernet controller
	map(0x80000, 0xfffff).rw(m_flash, FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
}

void ml20nb_state::io_map(address_map &map)
{
	common_io_map(map);
	map(0x60, 0x6f).r(FUNC(ml20nb_state::keypad_r));
	map(0xa0, 0xa0).portr(m_dsw);
}

INPUT_PORTS_START( ml20 )
	PORT_START("COL.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("ENTER")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_0)     PORT_NAME("0")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL)   PORT_NAME("CLEAR")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_D)     PORT_NAME("ler. MARC")
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("COL.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("9")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("8")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("7")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("MENSAJ.")
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("COL.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("6")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("5")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("4")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("SALDOS")
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("COL.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("3")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("1")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("ANULAR")
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

void ml20_base_state::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t( 92,  83,  88)); // LCD pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // LCD pixel off
}

HD44780_PIXEL_UPDATE( ml20_base_state::lcd_pixel_update )
{
	// char size is 5x7
	if (x > 4 || y > 6)
		return;

	if (line < 2 && pos < 16)
		bitmap.pix(1 + y + line * 8 + line, 1 + pos * 6 + x) = state ? 1 : 2;
}

// the keypad columns are selected by active low lines, and the rows read back active low
uint8_t ml20_base_state::keypad_rows(uint8_t columns)
{
	uint8_t data = 0xff;

	for (int i = 0; i < 4; i++)
	{
		if (!BIT(columns, i))
			data &= m_keys[i]->read();
	}

	return data;
}

// 765-----  unknown (outputs on ml20)
// ---4321-  unknown (inputs)
// -------0  NMI pin, from the MAX693 power-fail output (high while power is good)

uint8_t ml20_base_state::p1_r()
{
	return m_p1 | 0x01;
}

void ml20_base_state::p1_w(uint8_t data)
{
	LOGPORTS("%s: p1_w %d %d %d %d  %d %d %d %d\n", machine().describe_context(), BIT(data, 7), BIT(data, 6), BIT(data, 5), BIT(data, 4), BIT(data, 3), BIT(data, 2), BIT(data, 1), BIT(data, 0));

	m_p1 = data;
}

// ml20:
// 7-------  some kind of heartbeat? led?
// -6------  unknown (set to output)
// --543---  unknown (set to input)
// -----2--  set when waiting for keypad or other data?
// ------1-  MAX693 watchdog input
// -------0  unknown (set to output)

uint8_t ml20_base_state::p2_r()
{
	return m_p2;
}

void ml20_base_state::p2_w(uint8_t data)
{
	LOGPORTS("%s: p2_w %d %d %d %d  %d %d %d %d\n", machine().describe_context(), BIT(data, 7), BIT(data, 6), BIT(data, 5), BIT(data, 4), BIT(data, 3), BIT(data, 2), BIT(data, 1), BIT(data, 0));

	m_p2 = data;
}

// 7654----  unknown (set to input)
// ----3210  keypad column

uint8_t ml20_state::p0_r()
{
	return m_p0;
}

void ml20_state::p0_w(uint8_t data)
{
	LOGPORTS("%s: p0_w %d %d %d %d  %d %d %d %d\n", machine().describe_context(), BIT(data, 7), BIT(data, 6), BIT(data, 5), BIT(data, 4), BIT(data, 3), BIT(data, 2), BIT(data, 1), BIT(data, 0));

	m_p0 = data;
}

uint8_t ml20_state::pt_r()
{
	return keypad_rows(m_p0);
}

// 76-4----  unknown
// --5-----  MAX693 watchdog input
// ----3---  93LC66 CS
// -----2--  93LC66 SK
// ------1-  93LC66 DI
// -------0  93LC66 DO

uint8_t ml20nb_state::p0_r()
{
	return (m_p0 & 0xfe) | m_eeprom->do_read();
}

void ml20nb_state::p0_w(uint8_t data)
{
	m_p0 = data;
	m_eeprom->di_write(BIT(data, 1));
	m_eeprom->clk_write(BIT(data, 2));
	m_eeprom->cs_write(BIT(data, 3));
}

// A3-A0 select the keypad columns
uint8_t ml20nb_state::keypad_r(offs_t offset)
{
	return keypad_rows(offset);
}

void ml20_base_state::machine_start()
{
	m_p0 = m_p1 = m_p2 = 0xff;

	// register for save states
	save_item(NAME(m_p0));
	save_item(NAME(m_p1));
	save_item(NAME(m_p2));
}

void ml20_base_state::ml20_common(machine_config &config)
{
	V25(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->p1_in_cb().set(FUNC(ml20_base_state::p1_r));
	m_maincpu->p1_out_cb().set(FUNC(ml20_base_state::p1_w));
	m_maincpu->p2_in_cb().set(FUNC(ml20_base_state::p2_r));
	m_maincpu->p2_out_cb().set(FUNC(ml20_base_state::p2_w));

	// channel 0 is the host link (4800 baud, 7 bits, odd parity by default)
	// TODO: which connector each channel reaches
	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_maincpu, FUNC(v25_device::rxd_w<0>));
	m_maincpu->txd_handler<0>().set(rs232a, FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_maincpu, FUNC(v25_device::rxd_w<1>));
	m_maincpu->txd_handler<1>().set(rs232b, FUNC(rs232_port_device::write_txd));

	// video hardware
	screen_device &screen(SCREEN(config, "screen").set_lcd());
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(6 * 16 + 1, 18);
	screen.set_visarea_full();
	screen.set_screen_update(m_lcdc, FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(ml20_base_state::lcd_palette), 3);

	HD44780(config, m_lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_lcdc->set_lcd_size(2, 16);
	m_lcdc->set_pixel_update_cb(FUNC(ml20_base_state::lcd_pixel_update));

	config.set_default_layout(layout_ml20);

	// sound
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

void ml20_state::ml20(machine_config &config)
{
	ml20_common(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &ml20_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &ml20_state::common_io_map);
	m_maincpu->p0_in_cb().set(FUNC(ml20_state::p0_r));
	m_maincpu->p0_out_cb().set(FUNC(ml20_state::p0_w));
	m_maincpu->pt_in_cb().set(FUNC(ml20_state::pt_r));

	// TODO: the day of the week shows one day ahead (the firmware counts from Monday,
	// while the RTC starts from the host time counting from Sunday)
	MSM6242(config, m_rtc, 32.768_kHz_XTAL);

	// HACK: the firmware polls the busy flag only 100 times after a clear display, too few
	// with the emulated timing (no V25 bus wait states, unmeasured LCD clock), and it then
	// keeps reinitialising and blanking the LCD
	m_lcdc->set_busy_factor(0.5f);
}

void ml20nb_state::ml20nb(machine_config &config)
{
	ml20_common(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &ml20nb_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &ml20nb_state::io_map);
	m_maincpu->p0_in_cb().set(FUNC(ml20nb_state::p0_r));
	m_maincpu->p0_out_cb().set(FUNC(ml20nb_state::p0_w));

	AMD_29F040(config, m_flash);
	EEPROM_93C66_16BIT(config, m_eeprom); // 16-bit organisation, as the firmware addresses it
	RTC72423(config, m_rtc, 32.768_kHz_XTAL);
}

ROM_START( ml20 )
	ROM_REGION(0x10000, "bios", 0)
	ROM_LOAD("ml-20_v1.27.ic4", 0x0000, 0x10000, CRC(844d6d23) SHA1(08cd290bc342da328abc91b0699662c9ba335c0d) )
ROM_END

ROM_START( ml20nb )
	ROM_REGION(0x80000, "flash", 0)
	ROM_LOAD("bios_net_100_0.4_am29f040.ic2", 0x0000, 0x80000, CRC(ab9af8ff) SHA1(308ea6336be6bace1c7e55c5d922308510173430) )

	ROM_REGION16_BE(0x200, "eeprom", ROMREGION_ERASEFF)
	ROM_LOAD("rt93lc66-a.ic4", 0x000, 0x200, NO_DUMP)

	ROM_REGION(0x29200, "rfid", 0)
	ROM_LOAD("atmega128a.ic1", 0x00000, 0x20000, NO_DUMP) // internal flash
	ROM_LOAD("atmega128a_eeprom.ic1", 0x20000, 0x01000, NO_DUMP) // internal EEPROM
	ROM_LOAD("clrc632.ic4", 0x21000, 0x00200, NO_DUMP) // EEPROM, blocks 8-31 hold the Crypto1 keys and can't be read back
	ROM_LOAD("at25256b.ic2", 0x21200, 0x08000, NO_DUMP)

	ROM_REGION(0x400500, "fingerprint", 0)
	ROM_LOAD("39vf3201b.u4", 0x000000, 0x400000, NO_DUMP)
	ROM_LOAD("at88sc153.u7", 0x400000, 0x000100, NO_DUMP)
	ROM_LOAD("sfm3050-tc_v21f_16022200_nov-17_adsp-bf531.u6", 0x400100, 0x000400, NO_DUMP) // internal boot ROM

	ROM_REGION(0x4000, "smartcard", 0)
	ROM_LOAD("gc144gh-ul.ic2", 0x0000, 0x4000, NO_DUMP) // AT83C21GC (GemCore Serial Lite PRO on an AT83C5121), firmware version 144
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY    FULLNAME                       FLAGS
SYST( 1999, ml20,   0,      0,      ml20,    ml20,  ml20_state,   empty_init, "Digitek", "Micrologic-20",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE)
SYST( 2021, ml20nb, 0,      0,      ml20nb,  ml20,  ml20nb_state, empty_init, "Digitek", "Micrologic-20 100-NET-1 Bio", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE) // v.5.51 (custom 2.36)
