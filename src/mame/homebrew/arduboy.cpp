/****************************************************************************

    Arduboy
    Arduboy FX

    This is a homebrew ATMega handheld system, based around the ATMega32u4,
    which provides us with an excellent AVR emulation test case.

    The Arduboy FX has a 16 mbyte flash chip on board that can store multiple games.
    However, since the ATMega can only execute from its own internal 32kbyte flash,
    the games must be copied there every time.

    Basic hardware:
    - MCU: ATMega32U4
        - Fuses: lfuse 0xFF, hfuse 0xD2, efuse 0xCB
        - External 16 MHz clock crystal
    - Display: SSD1306 OLED display, 128x64 resolution
    - SPI flash (on Arduboy FX and compatibles): Winbond W25Q128, 16 mbytes
    - D-pad and two buttons
    - RGB LED at top left
    - Two LEDs for serial activity
    - Super thin battery that will probably inflate and explode
      (Note: system will NOT power on if the battery is dead)

    Port mappings:
    - Port B
        - B.0 rx LED
        - B.1 SPI SCK
        - B.2 SPI MOSI
        - B.3 SPI MISO
        - B.4 button B
        - B.5 blue LED
        - B.6 red LED
        - B.7 green LED

    - Port C
        - C.6 speaker positive
        - C.7 speaker negative

    - Port D
        - D.3 flash /CS
        - D.4 OLED D/C#
        - D.5 tx LED
        - D.6 OLED /CS
        - D.7 OLED RST

    - Port E
        - E.6 button A

    - Port F
        - F.4 down
        - F.5 left
        - F.6 right
        - F.7 up
    

****************************************************************************/

#include "emu.h"

#include "avr8.h"
#include "generic_spi_flash.h"
#include "ssd1306.h"
#include "speaker.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "sound/spkrdev.h"

namespace {

class arduboy_state : public driver_device
{
public:
	arduboy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_screen(*this, "screen"),
		  m_speaker(*this, "speaker"),
          m_spi_flash(*this, "spi_flash"),
          m_ssd1306(*this, "ssd1306")
	{ }

    void arduboy_base(machine_config &config);

    void arduboy(machine_config &config);
	void ardbyfx(machine_config &config);


    void prg_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<atmega32u4_device> m_maincpu;
	required_device<screen_device> m_screen;
    required_device<speaker_sound_device> m_speaker;
    required_device<generic_spi_flash_device> m_spi_flash;
    required_device<ssd1306_device> m_ssd1306;


	optional_device<generic_slot_device> m_cart; // required for arduboy, not for ardbyfx

	uint8_t port_b_r();
	void port_b_w(uint8_t data);
	uint8_t port_c_r();
	void port_c_w(uint8_t data);
	uint8_t port_d_r();
	void port_d_w(uint8_t data);
	uint8_t port_e_r();
	void port_e_w(uint8_t data);
	uint8_t port_f_r();
	void port_f_w(uint8_t data);

    DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

    int m_spi_last_sck;

    bool m_oled_cs_inactive;
    bool m_flash_cs_inactive;
};



uint8_t arduboy_state::port_b_r()
{
    int spi_miso = m_spi_flash->so_r() ? (1 << 3) : 0;
    int button_a = ioport("PORTB")->read() & (1 << 4);

    return spi_miso | button_a;
}

void arduboy_state::port_b_w(uint8_t data)
{
    int rx_led   = data & (1 << 0);
    int spi_sck  = data & (1 << 1);
    int spi_mosi = data & (1 << 2);


    m_spi_flash->si_w(spi_mosi);
    m_ssd1306->spi_si_w(spi_mosi);

    m_spi_flash->sck_w(spi_sck);
    m_ssd1306->spi_sck_w(spi_sck);
}


uint8_t arduboy_state::port_c_r()
{
    return 0;
}

void arduboy_state::port_c_w(uint8_t data)
{
    int speaker_positive  = (data & (1<<6));
    int speaker_negative  = (data & (1<<7));
    m_speaker->level_w((speaker_positive && !speaker_negative) ? 2 :
                       (!speaker_positive && speaker_negative) ? 0 : 1);
}

uint8_t arduboy_state::port_d_r()
{
    // all outputs on this port
    return 0;
}

void arduboy_state::port_d_w(uint8_t data)
{

    m_spi_flash->cs_w(data & (1 << 3));
    m_ssd1306->set_dc_line(data & (1 << 4));
    // TX LED on D.5
    m_ssd1306->spi_cs_w(data & (1 << 6));
    m_ssd1306->set_rst(data & (1 << 7));
}


uint8_t arduboy_state::port_e_r()
{
    return ioport("PORTE")->read() & 0x40;
}

void arduboy_state::port_e_w(uint8_t data)
{
    // inputs only on this port
}


uint8_t arduboy_state::port_f_r()
{
    return ioport("PORTF")->read() & 0xF0;
}

void arduboy_state::port_f_w(uint8_t data)
{
    // inputs only on this port
}


void arduboy_state::prg_map(address_map &map)
{
    map(0x7800, 0x7fff).rom().region("loader");
}

void arduboy_state::data_map(address_map &map)
{
    // TODO: 32u4 flash registers. the FX needs it

    map(0x0100, 0x0AFF).ram(); // on-chip 2.5kbytes RAM
}


static INPUT_PORTS_START( arduboy )
    PORT_START("PORTB")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )

    PORT_START("PORTE")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )

    PORT_START("PORTF")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
INPUT_PORTS_END

void arduboy_state::arduboy_base(machine_config &config)
{
    ATMEGA32U4(config, m_maincpu, 16'000'000);
    m_maincpu->set_addrmap(AS_PROGRAM, &arduboy_state::prg_map);
	m_maincpu->set_addrmap(AS_DATA, &arduboy_state::data_map);

	m_maincpu->set_eeprom_tag("eeprom");
    m_maincpu->set_low_fuses(0xFF);
    m_maincpu->set_high_fuses(0xD2);
    m_maincpu->set_extended_fuses(0xC2);

    m_maincpu->gpio_in<atmega328_device::GPIOB>().set(arduboy_state::port_b_r);
    m_maincpu->gpio_in<atmega328_device::GPIOC>().set(arduboy_state::port_c_r);
    m_maincpu->gpio_in<atmega328_device::GPIOD>().set(arduboy_state::port_d_r);
    m_maincpu->gpio_in<atmega328_device::GPIOE>().set(arduboy_state::port_e_r);
    m_maincpu->gpio_in<atmega328_device::GPIOF>().set(arduboy_state::port_f_r);

    m_maincpu->gpio_out<atmega328_device::GPIOB>().set(arduboy_state::port_b_w);
    m_maincpu->gpio_out<atmega328_device::GPIOC>().set(arduboy_state::port_c_w);
    m_maincpu->gpio_out<atmega328_device::GPIOD>().set(arduboy_state::port_d_w);
    m_maincpu->gpio_out<atmega328_device::GPIOE>().set(arduboy_state::port_e_w);
    m_maincpu->gpio_out<atmega328_device::GPIOF>().set(arduboy_state::port_f_w);


    SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(0, "mono", 1.00);
}


void arduboy_state::arduboy(machine_config &config)
{
    arduboy_base(config);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "arduboy", "bin");
	m_cart->set_must_be_loaded(true);
	m_cart->set_device_load(FUNC(arduboy_state::cart_load));
}

void arduboy_state::ardbyfx(machine_config &config)
{
    arduboy_base(config);

    GENERIC_SPI_FLASH(config, m_spi_flash);
}

DEVICE_IMAGE_LOAD_MEMBER(arduboy_state::cart_load)
{
	uint32_t size = std::min(m_cart->common_get_size("rom"), 0x7800);

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);

    memcpy(m_cart->get_rom_base(), image.get_software_region("intflash"), size);

	return std::make_pair(std::error_condition(), std::string());
}

ROM_START( arduboy )
    // arduboy treats this as a cart, ardbyfx loads games from spiflash into this space
    ROM_REGION( 0x7800, "intflash", ROMREGION_ERASEFF)

    // bootloader dumped from an Arduboy FX
	ROM_REGION( 0x800, "loader", ROMREGION_ERASEFF)
    ROM_LOAD("arduboy_boot.bin", 0x000, 0x800, CRC(4c49b0f5) SHA1(66a7411c46c04a8089a7ddfb5ffd9809dd08a21f))
ROM_END

} // anonymous namespace


//   YEAR  NAME     PARENT  COMPAT  MACHINE   INPUT    CLASS          INIT        COMPANY    FULLNAME
CONS(2015, arduboy, 0,      0,      arduboy,  arduboy, arduboy_state, empty_init, "Arduboy", "Arduboy",    MACHINE_NOT_WORKING)
CONS(2021, ardbyfx, 0,      0,      arduboy,  arduboy, arduboy_state, empty_init, "Arduboy", "Arduboy FX", MACHINE_NOT_WORKING)
