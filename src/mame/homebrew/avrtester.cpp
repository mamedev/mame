// license:BSD-3-Clause
// copyright-holders:wurthless-elektroniks
/**********************************************************************

    "AVR-Transistortester" and its descendants

    The "AVR-Transistortester" is a series of devices that originated from
    a design by Markus Frejek. The original was later improved upon by
	Karl-Heinz Kübbeler, and the improved design can test far more kinds
	of components and supports other display devices besides the original's HD44780.

    Kübbeler's improved design became the basis for a wide variety
    of commercial Chinese-made clones. These in turn have their own tweaks
    in software and hardware, usually to support different displays.

    The component under test is switched between different potential
    dividers by the GPIO ports, and the resulting voltages are sampled
    on the ADC pins. To emulate this accurately would mean hooking up
    netlists that simulate each kind of component under test.
    For now, we can look at a pretty error screen.

    Port mappings (for the ATMega8 version, with ATMegaX8 series devices being pin compatible):
    - Port B
        - PB0: connects/disconnects R1 and TP1 (pull up or pull down by 680 ohms)
        - PB1: connects/disconnects R2 and TP1 (pull up or pull down by 470k ohms)
        - PB2: connects/disconnects R3 and TP2 (pull up or pull down by 680 ohms)
        - PB3: connects/disconnects R4 and TP2 (pull up or pull down by 470k ohms)
        - PB4: connects/disconnects R5 and TP3 (pull up or pull down by 680 ohms)
        - PB5: connects/disconnects R6 and TP3 (pull up or pull down by 470k ohms)

    - Port C
        - PC0 (ADC0): between R1/R2 and TP1
        - PC1 (ADC1): between R3/R4 and TP2
        - PC2 (ADC2): between R5/R6 and TP3
        - PC3 (ADC3): left floating
        - PC4 (ADC4): left floating
        - PC5 (ADC5): battery voltage sense
                      9v---[ 10k ]----o----[ 3k3 ]----0v
                      so approx 2.23 volts when B+ is exactly 9v

    - Port D
        - PD0: LCD D4
        - PD1: LCD D5
        - PD2: LCD D6
        - PD3: LCD D7
        - PD4: LCD RS
        - PD5: LCD CLK (E pin)
        - PD6: biases on Q1 to keep power on
        - PD7: pulled low when TEST button is pressed

    TODOs for AVR8 CPU core:
    - ATMega8 support: Not possible currently because the AVR8 core is too heavily
      oriented around the ATMegaX8 series. Frejek's ATMega48 build cuts several features
      to fit the program in flash.

    - Callback to drivers so that we can tell when PBx lines go tristate.
      Remember that for AVRs, simply going to input mode isn't enough to determine this,
      as there are pullup enables internally.

    TODOs for this driver:
    - Some sort of netlist "cartridge" mechanism.

    Further reading:
    - Markus Frejek's original (in German):
      https://www.mikrocontroller.net/articles/AVR-Transistortester

**********************************************************************/

#include "emu.h"

#include "cpu/avr8/avr8.h"
#include "video/hd44780.h"

#include "screen.h"
#include "emupal.h"

namespace {

class avrtt_state : public driver_device
{
public:
	avrtt_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_hd44780(*this, "hd44780")
	{ }

	void avrtt(machine_config &config) ATTR_COLD;

private:
	required_device<atmega88_device> m_maincpu; // actually a ATMega48; will change avr8 core later
	required_device<hd44780_device> m_hd44780;

	void prg_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void lcd_palette(palette_device &palette) const ATTR_COLD;
	HD44780_PIXEL_UPDATE(lcd_pixel_update);

	void port_b_w(uint8_t data);
	void port_d_w(uint8_t data);
	uint8_t port_d_r();

	uint8_t m_gpio_b;
	uint8_t m_gpio_d;

	template<int TestPoint> uint16_t tp_r();

	uint16_t vbat_r();
};

void avrtt_state::avrtt(machine_config &config)
{
	ATMEGA88(config, m_maincpu, 8'000'000 / 8); // 8 MHz from internal R/C oscillator; fuses set div by 8
	m_maincpu->set_addrmap(AS_PROGRAM, &avrtt_state::prg_map);
	m_maincpu->set_addrmap(AS_DATA, &avrtt_state::data_map);
	m_maincpu->set_low_fuses(0x42);
	m_maincpu->set_high_fuses(0xdf);
	m_maincpu->set_extended_fuses(0xff);
	m_maincpu->set_eeprom_tag("eeprom");
	m_maincpu->gpio_out<atmega88_device::GPIOB>().set(FUNC(avrtt_state::port_b_w));
	m_maincpu->gpio_out<atmega88_device::GPIOD>().set(FUNC(avrtt_state::port_d_w));
	m_maincpu->gpio_in<atmega88_device::GPIOD>().set(FUNC(avrtt_state::port_d_r));

	m_maincpu->adc_in<0>().set(FUNC(avrtt_state::tp_r<0>));
	m_maincpu->adc_in<1>().set(FUNC(avrtt_state::tp_r<1>));
	m_maincpu->adc_in<2>().set(FUNC(avrtt_state::tp_r<2>));
	m_maincpu->adc_in<5>().set(FUNC(avrtt_state::vbat_r));

	HD44780(config, m_hd44780, 270'000); // typical HD44780 internal oscillator value
	m_hd44780->set_lcd_size(2, 16);
	m_hd44780->set_pixel_update_cb(FUNC(avrtt_state::lcd_pixel_update));

	// this SCREEN implementation shamelessly stolen from mdisplay2.cpp.
	// it would be a really good idea to unify HD44780 displays globally...
	screen_device &screen(SCREEN(config, "screen").set_lcd());
	screen.set_refresh_hz(60); // arbitrary
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(6*16+1, 9*2+1);
	screen.set_visarea_full();
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(avrtt_state::lcd_palette), 3);
}

void avrtt_state::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0xff, 0xff, 0xff)); // background
	palette.set_pen_color(1, rgb_t(0x00, 0x00, 0x00)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(0xe8, 0xe8, 0xe8)); // lcd pixel off
}

HD44780_PIXEL_UPDATE(avrtt_state::lcd_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 16)
		bitmap.pix(line*9 + 1 + y, 1 + pos*6 + x) = state ? 1 : 2;
}

INPUT_PORTS_START( empty_input )
	// technically, the TEST button powers the tester on/off,
	// but we're pretending it doesn't exist for the time being
INPUT_PORTS_END

void avrtt_state::machine_start()
{
	save_item(NAME(m_gpio_b));
	save_item(NAME(m_gpio_d));
}

void avrtt_state::machine_reset()
{
	m_gpio_b = 0;
	m_gpio_d = 0;
}

void avrtt_state::prg_map(address_map &map)
{
	// internal flash
	// this is the ATMega88's flash size (8k); actual on the ATMega44 is 4k.
	map(0x0000, 0x1fff).rom();
}

void avrtt_state::data_map(address_map &map)
{
	// internal SRAM
	// this is the ATMega88's SRAM size (1k); actual on the ATMega44 is 512b.
	map(0x0100, 0x04ff).ram();
}

void avrtt_state::port_b_w(uint8_t data)
{
	m_gpio_b = data;
}

void avrtt_state::port_d_w(uint8_t data)
{
	m_gpio_d = data;

	m_hd44780->db_w(BIT(data, 0, 4) << 4);
	m_hd44780->rs_w(BIT(data, 4));
	m_hd44780->e_w(BIT(data, 5));
}

uint8_t avrtt_state::port_d_r()
{
	return m_gpio_d | (1 << 7); // pretend TEST is never pushed
}

template<int TestPoint>
uint16_t avrtt_state::tp_r()
{
	// TODO: this is obviously wrong.
	// we need both an underlying netlist AND support for gpio tristating
	// in the avr8 core for this to return anything meaningful.
	// for now, just return hi/lo states of port b.
	return (BIT(m_gpio_b, TestPoint * 2, 2)) ? 0x3ff : 0;
}

uint16_t avrtt_state::vbat_r()
{
	// maximum battery level; too low causes software complaints
	return 0x3ff;
}

// original build files are in Intel HEX format
ROM_START( avrtt )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD( "avrtt48-ic1.rom", 0x0000, 0x1000, CRC(77f620bd) SHA1(91e8a4c1ce568d2acf3bcf57be0a45eb4f9060bf) ) // =TransistorTestNew.hex

	ROM_REGION( 0x200, "eeprom", ROMREGION_ERASEFF )
	ROM_LOAD( "avrtt48-ic1.eep", 0x0000, 0x0100, CRC(54f3d6c9) SHA1(ed89924f5367449c47ec9c7b43e4a0d0d4b1dc73) ) // =TransistorTestNew.eep
ROM_END

} // anonymous namespace

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT        CLASS        INIT        COMPANY          FULLNAME                FLAGS
SYST( 2009, avrtt, 0,      0,      avrtt,   empty_input, avrtt_state, empty_init, "Markus Frejek", "AVR-Transistortester", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
