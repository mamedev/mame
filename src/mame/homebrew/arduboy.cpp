/****************************************************************************

    Arduboy / Arduboy FX hardware

    This is a homebrew ATMega handheld system, based around the ATMega32u4,
    which provides us with an excellent AVR emulation test case.

    The vanilla Arduboy expects you to upload or flash software to it. Such software is
    virtually always in Intel HEX format, so we have to support that.

    The Arduboy FX has a 16 mbyte flash chip on board that can store multiple games.
    However, since the ATMega can only execute from its own internal 32kbyte flash,
    the games must be copied there every time.

    Some FX games support reading data from the 16 mbyte flash. Those games are
    distributed as .arduboy files, which are standard ZIP files containing a
    JSON manifest, the main game code as a .hex, and the game resources as
    .bin files. These are currently not supported as that would be a gigantic
    chore to support within the MAME framework. For those games, you should
    create your own flashcart with the game installed, then feed that
    into the ardbyfx driver.

    Basic hardware:
    - MCU: ATMega32U4
        - Fuses: lfuse 0xFF, hfuse 0xD2, efuse 0xCB
        - External 16 MHz clock crystal
    - Display: SSD1306 OLED display, 128x64 resolution, in 4-wire SPI mode
    - SPI flash (on Arduboy FX and compatibles): Winbond W25Q128, 16 mbytes
    - D-pad and two buttons
    - RGB LED at top left, software driven
    - Two yellow LEDs for serial activity, software driven
    - One red LED for charge indication
    - Super thin battery that will probably inflate and explode
        - NOTE: The system will NOT power on if the battery is dead or missing.
          If you want to remove the battery, then the easiest reversable hack
          is to jump a 10uF capacitor across BATT+ and ground. This will keep
          the cap charged at 4.2 volts, and the system will run without complaints.

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
#include "machine/nvram.h"
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
    required_device<ssd1306_device> m_ssd1306;


    optional_device<generic_slot_device> m_cart;            // required for arduboy, not for ardbyfx
    optional_device<generic_slot_device> m_spicart;         // required for ardbyfx, not present on arduboy

    optional_device<generic_spi_flash_device> m_spi_flash;


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

    DECLARE_DEVICE_IMAGE_LOAD_MEMBER(gameprg_load);
    DECLARE_DEVICE_IMAGE_LOAD_MEMBER(spiflash_load);

    int m_spi_last_sck;

    bool m_oled_cs_inactive;
    bool m_flash_cs_inactive;

    uint8_t m_internal_flash[0x7800];

    uint8_t* m_spi_flash_data;
};

void arduboy_state::machine_start()
{
    subdevice<nvram_device>("intflash")->set_base(&m_internal_flash[0], 0x7800);
}

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

    if (m_spi_flash) m_spi_flash->si_w(spi_mosi);
    m_ssd1306->spi_si_w(spi_mosi);

    if (m_spi_flash) m_spi_flash->sck_w(spi_sck);
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

    if (m_spi_flash) m_spi_flash->cs_w(data & (1 << 3));
    m_ssd1306->dc_w(data & (1 << 4));
    // TX LED on D.5
    m_ssd1306->spi_cs_w(data & (1 << 6));
    m_ssd1306->rst_w(data & (1 << 7));
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
    map(0x0000, 0x77ff).rom().region("intflash");
    map(0x7800, 0x7fff).rom().region("loader");
}

void arduboy_state::data_map(address_map &map)
{
    // TODO: 32u4 flash registers. the FX needs it

    map(0x0100, 0x0aff).ram(); // on-chip 2.5kbytes RAM
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

    NVRAM(config, "intflash", nvram_device::DEFAULT_ALL_1);

    SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(0, "mono", 1.00);
}

void arduboy_state::arduboy(machine_config &config)
{
    arduboy_base(config);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "mainprg", "bin,hex");
	m_cart->set_must_be_loaded(true);
	m_cart->set_device_load(FUNC(arduboy_state::gameprg_load));
}

void arduboy_state::ardbyfx(machine_config &config)
{
    arduboy_base(config);

    GENERIC_SPI_FLASH(config, m_spi_flash);
	m_spi_flash->set_rom_ptr(memregion("spi")->base());
	m_spi_flash->set_rom_size(memregion("spi")->bytes());

    GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "gameprg", "bin,hex");
	m_cart->set_must_be_loaded(false);
	m_cart->set_device_load(FUNC(arduboy_state::gameprg_load));

    GENERIC_CARTSLOT(config, m_spicart, generic_plain_slot, "spiflash", "bin");
	m_spicart->set_must_be_loaded(true);
	m_spicart->set_device_load(FUNC(arduboy_state::spiflash_load));
}

//////////////////////////////////////////////////////////////////////////////////////

DEVICE_IMAGE_LOAD_MEMBER(arduboy_state::spiflash_load)
{
    if (!image.is_filetype("bin"))
    {
        return std::make_pair(image_error::BADSOFTWARE, "spiflash dump must be a .bin");
    }

    memory_region* spimem = memregion("spi");
    if (image.length() > spimem->length())
    {
        return std::make_pair(image_error::BADSOFTWARE, "spiflash dump too large!");
    }

    image.fseek(0, SEEK_SET);
    image.fread(spimem->base(), image.length());

    return std::make_pair(std::error_condition(), std::string());
}

//////////////////////////////////////////////////////////////////////////////////////
// 
// Game (Intel binhex) loader code
//
//////////////////////////////////////////////////////////////////////////////////////

#define PARSE_HEX(xin, xout) { \
    if ('0' <= xin && xin <= '9') \
    {   \
        xout = xin - '0';   \
    }   \
    else if ('A' <= xin && xin <= 'F')  \
    {   \
        xout = (xin - 'A') + 0x0A;  \
    }   \
    else  \
    {   \
        return std::make_pair(image_error::BADSOFTWARE, "invalid hex byte");    \
    }   \
}

#define FREAD_BOUNDSCHECK(img, bufptr, count) \
    if (img.fread(bufptr,count) != count) \
        return std::make_pair(image_error::BADSOFTWARE, "file read error or premature EOF");

DEVICE_IMAGE_LOAD_MEMBER(arduboy_state::gameprg_load)
{
    // remember: loading a new game overwrites the previous one up until EOF,
    // so we let the old one persist at least in part.
    if (image.is_filetype("bin"))
    {
        image.fread(m_internal_flash, 0x7800);
        return std::make_pair(std::error_condition(), std::string());
    }

    if (!image.is_filetype("hex"))
    {
        return std::make_pair(image_error::BADSOFTWARE, "cart must be bin or hex");
    }

    // oh boy oh boy! someone gave us a .hex. and that's gonna be super painful.
    image.fseek(0, SEEK_SET);
    char buf[80];
    while(image.ftell() < image.length())
    {
        uint8_t  num_bytes;
        uint16_t address;
        uint8_t  record_type;

        uint8_t hex[4 + 16 + 4];
        uint8_t checksum = 0;

        FREAD_BOUNDSCHECK(image, buf, 1);
        if (buf[0] != ':')
        {
            return std::make_pair(image_error::BADSOFTWARE, "hexdump line did not start with ':'");
        }

        memset(buf, 0, sizeof(buf));
        FREAD_BOUNDSCHECK(image, buf, 8);
        
        if (sscanf(buf, "%02X%04X%02X", &num_bytes, &address, &record_type) == EOF)
        { 
            return std::make_pair(image_error::BADSOFTWARE, "record parse error");
        }

        // while the intel hex standard can support data lines greater than 16 bytes,
        // virtually all software for the Arduboy only uses 16,
        // so complain if we see anything else
        if (num_bytes > 16)
        {
            return std::make_pair(image_error::BADSOFTWARE, "record greater than 16 bytes");
        }

        if (num_bytes != 0)
        {
            FREAD_BOUNDSCHECK(image, buf + 8, num_bytes * 2);
        }

        for (int i = 0; i < 8 + num_bytes; i++)
        {
            uint8_t hibits_byte;
            uint8_t lobits_byte;

            char hibits = buf[i*2];
            char lobits = buf[(i*2)+1];

            PARSE_HEX(hibits, hibits_byte);
            PARSE_HEX(lobits, lobits_byte);

            uint8_t byte = (hibits << 4) | lobits; 
            hex[i] = byte;

            if (i == 0)
            {
                checksum = byte;
            }
            else
            {
                checksum += byte;
            }
        }

        memset(buf, 0, sizeof(buf));
        FREAD_BOUNDSCHECK(image, buf, 2);
        uint8_t expected_checksum;
        if (sscanf(buf, "%02X", &expected_checksum) == EOF)
        {
            return std::make_pair(image_error::BADSOFTWARE, "checksum parse error");
        }

        if ((~checksum + 1) != expected_checksum)
        {
            return std::make_pair(image_error::BADSOFTWARE, "checksum mismatch"); 
        }

        // we've finally parsed the entire line, that's cause for celebration.
        // but we're still not done, unfortunately!
        if (record_type == 1)
        {
            // ***** SUCCESS PATH: *****
            // if we hit the EOF record, then treat it as the success path.
            // we're counting on the hexdump being valid to begin with
            // or loaded from a softlist. no need to complain about
            // data past EOF
            return std::make_pair(std::error_condition(), std::string());
        }

        if (record_type != 0)
        {
            // lots of other record types in the hex format,
            // but arduboy games never use them
            return std::make_pair(image_error::BADSOFTWARE, "invalid/unimplemented hexdump record type");
        }

        if (!(
                (0x0000 <= address && address <= 0x77ff) ||
                (0x0000 <= (address + num_bytes) && (address + num_bytes) <= 0x77ff)
             ))
        {
            return std::make_pair(image_error::BADSOFTWARE, "hex record writes out of bounds");
        }
        
        // all that just to do this. whew
        memcpy(m_internal_flash + address, hex + 4, num_bytes); // n.b.: 8 chars = 4 hex bytes

        // skip garbage until next record begins
        while(1)
        {
            FREAD_BOUNDSCHECK(image, buf, 1);
            if (buf[0] == ':')
            {
                // remember that the top of loop expects to read ':'
                image.fseek(-1, SEEK_CUR);
                break;
            }
        }

        // ... continue loop from top ...
    }

    // we shouldn't end up here as EOF checks in the while loop should catch this for us
    return std::make_pair(image_error::BADSOFTWARE, "hexdump hit premature EOF");
}

//////////////////////////////////////////////////////////////////////////////////////
// 
// ROM and machine declarations
// 
//////////////////////////////////////////////////////////////////////////////////////

ROM_START( arduboy )
    // generic Cathy2k loader
    // from https://github.com/MrBlinky/Arduboy/blob/master/cathy/hexfiles/arduboy-bootloader.hex
    // keeping only the actual bootloader segment (0x7800-0x7FFF)
	ROM_REGION(0x800, "loader", ROMREGION_ERASEFF)
    ROM_LOAD("arduboy-bootloader.bin", 0x000, 0x800, CRC(12345678) SHA1(garbagegarbagegarbage))
ROM_END

ROM_START( ardbyfx )
    // bootloader dumped from an Arduboy FX
	ROM_REGION( 0x800, "loader", ROMREGION_ERASEFF)
    ROM_LOAD("ardbyfx_boot.bin", 0x000, 0x800, CRC(12345678) SHA1(garbagegarbagegarbage))

    // Arduboy FX has a 16mbyte chip on board, so honor that.
    // note though that various clones and mods can support larger flash sizes.
    ROM_REGION(0x01000000, "spi", ROMREGION_ERASEFF)
} // anonymous namespace


//   YEAR  NAME     PARENT  COMPAT  MACHINE   INPUT    CLASS          INIT        COMPANY    FULLNAME
CONS(2015, arduboy, 0,      0,      arduboy,  arduboy, arduboy_state, empty_init, "Arduboy", "Arduboy",    MACHINE_NOT_WORKING)
CONS(2021, ardbyfx, 0,      0,      arduboy,  arduboy, arduboy_state, empty_init, "Arduboy", "Arduboy FX", MACHINE_NOT_WORKING)
