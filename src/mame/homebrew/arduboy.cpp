// license:BSD-3-Clause
// copyright-holders:wurthless-elektroniks
/****************************************************************************

    Arduboy hardware

    This is a homebrew ATMega handheld system, based around the ATMega32u4,
    which provides us with an excellent AVR emulation test case.

    The vanilla Arduboy expects you to upload or flash software to it. Such software is
    virtually always in Intel HEX format, so we have to support that.

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
    
    About the Arduboy FX:
    -----------------------------
    The Arduboy FX has a 16 mbyte flash chip on board that can store multiple games.
    However, since the ATMega can only execute from its own internal 32kbyte flash,
    the games must be copied there every time.

    Some FX games support reading data from the 16 mbyte flash. Those games are
    distributed as .arduboy files, which are standard ZIP files containing a
    JSON manifest, the main game code as a .hex, and the game resources as
    .bin files.

    Driver status:
    -----------------------------
    Preliminary (MACHINE_NOT_WORKING).
    
    The AVR8 core is missing a lot of features that the Arduboy platform
    as a whole needs. In particular, the Arudino APIs like delay() do not
    work like they're supposed to, so software basically runs "1988 DOS game on a
    Pentium 4" levels of fast, or bootloops.

    Games based off the Arduboy2 library can work and are somewhat playable.
    This may be because they use the sleep opcode instead of relying on
    specific timer values.

    MACHINE_IMPERFECT_GRAPHICS should be set until SSD1306 features are
    fully implemented. There are many modes that Arduboy games don't use.

****************************************************************************/

#include "emu.h"

#include "speaker.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/avr8/avr8.h"
#include "machine/nvram.h"
#include "machine/generic_spi_flash.h"
#include "sound/spkrdev.h"
#include "video/ssd1306.h"

namespace {

class arduboy_state : public driver_device
{
public:
	arduboy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_screen(*this, "screen"),
		  m_speaker(*this, "speaker"),
          m_ssd1306(*this, "ssd1306"),
          m_cart(*this, "cart"),
          m_spi_flash(*this, "spi_flash")
	{ }

    void arduboy_base(machine_config &config);

    void arduboy(machine_config &config);


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

    // this is stubbed in for the Arduboy FX; do nullpointer checks before accessing it
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

    uint8_t intflash_r(offs_t offset);

    DECLARE_DEVICE_IMAGE_LOAD_MEMBER(gameprg_load);

    bool m_rx_led;
    bool m_tx_led;
    bool m_rgbled_r;
    bool m_rgbled_g;
    bool m_rgbled_b;
    
    uint8_t m_internal_flash[0x7800];
};

void arduboy_state::machine_start()
{
    // internal ATMega flash memory persists past a reboot on real hardware,
    // so replicate that here.
    // it also gives us the bonus of saving .hex contents to the nvram folder,
    // which can be fed into a disassembler later.
    subdevice<nvram_device>("intflash")->set_base(&m_internal_flash[0], 0x7800);
}

void arduboy_state::machine_reset()
{
    if (m_cart)
    {
        // actual behavior on real hardware:
        // - running a .hex flash will only affect the bytes written by the .hex
        // - the Arduboy FX flashcart menu/loader overwrites 0...n bytes
        //   containing the game; all others up until 0x7800 persist.
        memcpy(m_internal_flash, m_cart->get_rom_base(), m_cart->get_rom_size());
    }
}

uint8_t arduboy_state::port_b_r()
{
    int spi_miso = m_spi_flash ? (m_spi_flash->so_r() ? (1 << 3) : 0) : 0;
    int button_a = ioport("PORTB")->read() & (1 << 4);

    return spi_miso | button_a;
}

void arduboy_state::port_b_w(uint8_t data)
{
    m_rx_led     = data & (1 << 0);
    int spi_sck  = data & (1 << 1);
    int spi_mosi = data & (1 << 2);
    // B.3 = MISO
    // B.4 = button B 
    m_rgbled_r   = data & (1 << 5);
    m_rgbled_g   = data & (1 << 6);
    m_rgbled_b   = data & (1 << 7);

    if (m_spi_flash) m_spi_flash->si_w(spi_mosi);
    m_ssd1306->spi_si_w(spi_mosi);

    if (m_spi_flash) m_spi_flash->sck_w(spi_sck);
    m_ssd1306->spi_sck_w(spi_sck);
}


uint8_t arduboy_state::port_c_r()
{
    logerror("%s: read from write-only port c\n", tag());
    return 0;
}

void arduboy_state::port_c_w(uint8_t data)
{
    int speaker_positive  = (data & (1<<6));
    int speaker_negative  = (data & (1<<7));
    m_speaker->level_w( (speaker_positive ^ speaker_negative) ? 1 : 0 );
}

uint8_t arduboy_state::port_d_r()
{
    logerror("%s: read from write-only port d\n", tag());
    return 0;
}

void arduboy_state::port_d_w(uint8_t data)
{
    if (m_spi_flash) m_spi_flash->cs_w(data & (1 << 3));
    m_ssd1306->dc_w(data & (1 << 4));
    m_tx_led = data & (1 << 5);
    m_ssd1306->spi_cs_w(data & (1 << 6));
    m_ssd1306->rst_w(data & (1 << 7));
}

uint8_t arduboy_state::port_e_r()
{
    return ioport("PORTE")->read() & 0x40;
}

void arduboy_state::port_e_w(uint8_t data)
{
    logerror("%s: write to read-only port e\n", tag());
}

uint8_t arduboy_state::port_f_r()
{
    return ioport("PORTF")->read() & 0xF0;
}

void arduboy_state::port_f_w(uint8_t data)
{
    logerror("%s: write to read-only port f\n", tag());
}

uint8_t arduboy_state::intflash_r(offs_t offset)
{
    return m_internal_flash[offset];
}

void arduboy_state::prg_map(address_map &map)
{
    map(0x0000, 0x77ff).r(FUNC(arduboy_state::intflash_r));
    
    // bootloader sits at 0x7800-0x7fff.
    // note though that very old bootloaders are 3k
}

void arduboy_state::data_map(address_map &map)
{
    // TODO: 32u4 flash registers. the Arduboy FX needs it

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
    m_maincpu->set_high_fuses(0xD3); // actually 0xD2, but games will run without the bootloader
    m_maincpu->set_extended_fuses(0xC2);

    m_maincpu->gpio_in<atmega328_device::GPIOB>().set(FUNC(arduboy_state::port_b_r));
    m_maincpu->gpio_in<atmega328_device::GPIOC>().set(FUNC(arduboy_state::port_c_r));
    m_maincpu->gpio_in<atmega328_device::GPIOD>().set(FUNC(arduboy_state::port_d_r));
    m_maincpu->gpio_in<atmega328_device::GPIOE>().set(FUNC(arduboy_state::port_e_r));
    m_maincpu->gpio_in<atmega328_device::GPIOF>().set(FUNC(arduboy_state::port_f_r));

    m_maincpu->gpio_out<atmega328_device::GPIOB>().set(FUNC(arduboy_state::port_b_w));
    m_maincpu->gpio_out<atmega328_device::GPIOC>().set(FUNC(arduboy_state::port_c_w));
    m_maincpu->gpio_out<atmega328_device::GPIOD>().set(FUNC(arduboy_state::port_d_w));
    m_maincpu->gpio_out<atmega328_device::GPIOE>().set(FUNC(arduboy_state::port_e_w));
    m_maincpu->gpio_out<atmega328_device::GPIOF>().set(FUNC(arduboy_state::port_f_w));

    NVRAM(config, "intflash", nvram_device::DEFAULT_ALL_1);

    SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(0, "mono", 1.00);

    SSD1306(config, m_ssd1306, 0);
    m_ssd1306->set_screen("screen");
    m_ssd1306->set_intf_mode(SPI_4WIRE);
    m_ssd1306->set_base_rowscan_invert(true);

	screen_device &screen(SCREEN(config, m_screen));
    screen.set_size(128, 64);
    screen.set_visarea(0, 127, 0, 63);
    screen.set_lcd();
    screen.set_screen_update(m_ssd1306, FUNC(ssd1306_device::screen_update));
    screen.set_palette(m_ssd1306);
}

void arduboy_state::arduboy(machine_config &config)
{
    arduboy_base(config);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "gameprg", "bin,hex");
	m_cart->set_must_be_loaded(true);
	m_cart->set_device_load(FUNC(arduboy_state::gameprg_load));
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
        printf("%s: invalid hexbyte on load: %02x (@ %08x)\n", tag(), xin, (uint32_t)image.ftell()); \
        return std::make_pair(image_error::BADSOFTWARE, "invalid hex byte");    \
    }   \
}

#define FREAD_BOUNDSCHECK(img, bufptr, count) \
    if (img.fread(bufptr,count) != count) \
        return std::make_pair(image_error::BADSOFTWARE, "file read error or premature EOF");

DEVICE_IMAGE_LOAD_MEMBER(arduboy_state::gameprg_load)
{
    m_cart->rom_alloc(0x7800, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);

    uint8_t* rom = m_cart->get_rom_base();
    memset(rom, 0xff, 0x7800);

    if (image.is_filetype("bin"))
    {
        image.fread(rom, 0x7800);
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
        // unsigned int, because of sscanf() below
        unsigned int num_bytes, address, record_type;

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

        for (int i = 0; i < 4 + num_bytes; i++)
        {
            uint8_t hibits_byte;
            uint8_t lobits_byte;

            char hibits = buf[i*2];
            char lobits = buf[(i*2)+1];

            PARSE_HEX(hibits, hibits_byte);
            PARSE_HEX(lobits, lobits_byte);

            uint8_t byte = (hibits_byte << 4) | lobits_byte; 
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
        unsigned int expected_checksum;
        if (sscanf(buf, "%02X", &expected_checksum) == EOF)
        {
            return std::make_pair(image_error::BADSOFTWARE, "checksum parse error");
        }

        uint8_t actual_checksum = (~checksum + 1) & 0xff;

        if (actual_checksum != expected_checksum)
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
        memcpy(rom + address, hex + 4, num_bytes); // n.b.: 8 chars = 4 hex bytes

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

            if (!(buf[0] == 0x0D || buf[0] == 0x0A))
            {
                return std::make_pair(image_error::BADSOFTWARE, "hit bad newline character");
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
    // games will work without the bootloader, but it would be ideal
    // in the long term to include a loader here and run it.
    // that won't work as long as there are AVR8 limitations however.

    ROM_REGION( 0x800, "eeprom", ROMREGION_ERASE00 )
ROM_END

// for documentation purposes: the Arduboy FX bootloader.
// CRC(4c49b0f5) SHA1(66a7411c46c04a8089a7ddfb5ffd9809dd08a21f)

} // anonymous namespace

//   YEAR  NAME     PARENT  COMPAT  MACHINE   INPUT    CLASS          INIT        COMPANY    FULLNAME
CONS(2015, arduboy, 0,      0,      arduboy,  arduboy, arduboy_state, empty_init, "Arduboy", "Arduboy", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING)
