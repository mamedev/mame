// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

        Psion Series 7 (EPOC R5)

        TODO:
        - Digitiser
        - Audio
        - UART support
        - Compact Flash (Thera)
        - Bezel artwork

****************************************************************************/

#include "emu.h"
#include "codec.h"
#include "thera.h"

#include "bus/pccard/ataflash.h"
#include "bus/rs232/rs232.h"
#include "cpu/arm7/arm7.h"
#include "imagedev/snapquik.h"
#include "machine/eepromser.h"
#include "machine/intelfsh.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "machine/sa1110.h"
#include "sound/spkrdev.h"

#include "screen.h"
#include "speaker.h"


namespace {

class psion7_state : public driver_device
{
public:
	psion7_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_sa1100(*this, "sa1100")
		, m_ram(*this, "ram")
		, m_nvram(*this, "nvram")
		, m_eeprom(*this, "eeprom")
		, m_thera(*this, "thera")
		, m_pccard(*this, "pccard%u", 1U)
		, m_buzzer(*this, "buzzer")
		, m_codec(*this, "codec")
		, m_mic(*this, "mic")
		, m_touchx(*this, "TOUCHX")
		, m_touchy(*this, "TOUCHY")
		, m_touch(*this, "TOUCH")
		, m_keyboard(*this, "COL%u", 0U)
	{
	}

	void psion7(machine_config &config) ATTR_COLD;
	void netbook(machine_config &config) ATTR_COLD;

	void init_s7() ATTR_COLD;
	void init_nb() ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(touch_down);

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void s7_map(address_map &map) ATTR_COLD;
	void nb_map(address_map &map) ATTR_COLD;

	void init_eeprom(std::string type);

	uint8_t kbd_r();
	uint16_t ads7843_r(offs_t offset);
	//void update_amp();

	required_device<cpu_device> m_maincpu;
	required_device<sa1110_periphs_device> m_sa1100;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<thera_device> m_thera;
	required_device_array<pccard_slot_device, 2> m_pccard;
	required_device<speaker_sound_device> m_buzzer;
	required_device<psion_codec_device> m_codec;
	required_device<microphone_device> m_mic;
	required_ioport m_touchx;
	required_ioport m_touchy;
	required_ioport m_touch;
	required_ioport_array<8> m_keyboard;

	uint8_t m_key_scan = 0;
	//uint8_t m_volume = 0;
	//bool m_amp_enable = true;
};


void psion7_state::init_eeprom(std::string machine)
{
	uint8_t *eeprom = memregion("eeprom")->base();

	// defaults expected by the touchscreen code
	eeprom[0x0a] = 20;
	eeprom[0x0b] = 20;
	eeprom[0x0c] = 20;
	eeprom[0x0d] = 30;

	// machine serial
	eeprom[0x18] = 0xef;
	eeprom[0x19] = 0xbe;
	eeprom[0x1a] = 0xad;
	eeprom[0x1b] = 0xde;

	// machine type
	eeprom[0x28] = machine.length();
	for (int i = 0; i < 16; i++)
		eeprom[0x29 + i] = (i < machine.length()) ? machine[i] : 0x00;

	// calculate the checksum
	uint8_t chksum = 0;
	for (int i = 1; i < 0x80; i++)
		chksum ^= eeprom[i];

	// EPOC is expecting 0x42
	eeprom[0x00] = chksum ^ 0x42;
}

void psion7_state::init_s7()
{
	init_eeprom("Series 7");
}

void psion7_state::init_nb()
{
	init_eeprom("netBook");
}


void psion7_state::machine_start()
{
	switch (m_ram->size() >> 20)
	{
	case 16:
		m_maincpu->space(AS_PROGRAM).install_ram(0xc0000000, 0xc0ffffff, 0x07000000, m_ram->pointer());
		break;
	case 32:
		m_maincpu->space(AS_PROGRAM).install_ram(0xc0000000, 0xc0ffffff, 0x07000000, m_ram->pointer());
		m_maincpu->space(AS_PROGRAM).install_ram(0xc8000000, 0xc8ffffff, 0x07000000, m_ram->pointer() + 0x1000000);
		break;
	case 64:
		m_maincpu->space(AS_PROGRAM).install_ram(0xc0000000, 0xc0ffffff, 0x07000000, m_ram->pointer());
		m_maincpu->space(AS_PROGRAM).install_ram(0xc8000000, 0xc8ffffff, 0x07000000, m_ram->pointer() + 0x1000000);
		m_maincpu->space(AS_PROGRAM).install_ram(0xd0000000, 0xd0ffffff, 0x07000000, m_ram->pointer() + 0x2000000);
		m_maincpu->space(AS_PROGRAM).install_ram(0xd8000000, 0xd8ffffff, 0x07000000, m_ram->pointer() + 0x3000000);
		break;
	}

	m_nvram->set_base(m_ram->pointer(), m_ram->size());

	save_item(NAME(m_key_scan));
	//save_item(NAME(m_volume));
	//save_item(NAME(m_amp_enable));
}

void psion7_state::machine_reset()
{
	m_key_scan = 0;
}


uint8_t psion7_state::kbd_r()
{
	uint16_t data = 0x00;

	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_key_scan, i))
			data |= m_keyboard[i]->read();
	}

	return data;
}


uint16_t psion7_state::ads7843_r(offs_t offset)
{
	uint16_t data = 0;

	// TODO: Proper SSP support with ADS7843 device
	switch (offset)
	{
	case 0xd0d3: // Touch X
		data = 50 + (uint16_t)(m_touchx->read() * 5.7);
		break;
	case 0x9093: // Touch Y
		data = 3834 - (uint16_t)(m_touchy->read() * 13.225);
		break;
	case 0xa4a4: // Main Battery
	case 0xe4e4: // Backup Battery
		data = 3100;
		break;
	}

	return data;
}


#if 0
void psion7_state::update_amp()
{
	// TODO: MSC1192 speaker amplifier, can be put into standby to mute audio.
	constexpr float codec_volume[4] = { 1.0f, 0.75f, 0.5f, 0.25f };

	if (m_amp_enable)
	{
		m_buzzer->set_output_gain(ALL_OUTPUTS, 1.0);
		m_codec->set_output_gain(ALL_OUTPUTS, codec_volume[m_volume]); // VOL
	}
	else
	{
		m_buzzer->set_output_gain(ALL_OUTPUTS, 0.0);
		m_codec->set_output_gain(ALL_OUTPUTS, 0.0);
	}
}
#endif


void psion7_state::s7_map(address_map &map)
{
	map(0x00000000, 0x00ffffff).mirror(0x0f000000).rom().region("maincpu", 0);
	map(0x10000000, 0x1000007f).rw(m_thera, FUNC(thera_device::read), FUNC(thera_device::write));
	map(0x80000000, 0xbfffffff).m(m_sa1100, FUNC(sa1110_periphs_device::map));
	map(0xe0000000, 0xe0001fff).nopr();
}

void psion7_state::nb_map(address_map &map)
{
	map(0x00000000, 0x001fffff).rw("flash", FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));
	map(0x10000000, 0x1000007f).rw(m_thera, FUNC(thera_device::read), FUNC(thera_device::write));
	map(0x80000000, 0xbfffffff).m(m_sa1100, FUNC(sa1110_periphs_device::map));
	map(0xe0000000, 0xe0001fff).nopr();
}


INPUT_CHANGED_MEMBER(psion7_state::touch_down)
{
	m_thera->write_pb7(newval ? 0 : 1);
}


INPUT_PORTS_START( psion7 )
	PORT_START("TOUCHX")
	PORT_BIT(0x3ff, 362, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(42,681) PORT_SENSITIVITY(25) PORT_KEYDELTA(13)

	PORT_START("TOUCHY")
	PORT_BIT(0x0ff, 125, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(5,244) PORT_SENSITIVITY(25) PORT_KEYDELTA(13)

	PORT_START("TOUCH")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Touch") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psion7_state::touch_down), 0)
	PORT_BIT(0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)     PORT_CHAR(13)  PORT_NAME("Enter")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)     PORT_CHAR(UCHAR_MAMEKEY(RIGHT))  PORT_NAME(u8"\u2192") // U+2192 = →
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)       PORT_CHAR(9) PORT_NAME("Tab  Caps")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)         PORT_CHAR('y')  PORT_CHAR('Y')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)      PORT_CHAR(UCHAR_MAMEKEY(LEFT))   PORT_NAME(u8"\u2190") // U+2190 = ←
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)      PORT_CHAR(UCHAR_MAMEKEY(DOWN))   PORT_NAME(u8"\u2193") // U+2193 = ↓
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)         PORT_CHAR('n')  PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)    PORT_CHAR(UCHAR_SHIFT_1)         PORT_NAME("Left Shift")

	PORT_START("COL1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_NAME("Del<-")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('-')  PORT_CHAR('_') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)    PORT_CHAR('=')  PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)         PORT_CHAR('0')  PORT_CHAR(')') PORT_CHAR(']')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)         PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(';')  PORT_CHAR(':')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)    PORT_NAME("Right Shift")

	PORT_START("COL2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("On/Off")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)         PORT_CHAR('k')  PORT_CHAR('K')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)         PORT_CHAR('i')  PORT_CHAR('I')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)         PORT_CHAR('8')  PORT_CHAR('*') PORT_CHAR('}')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)         PORT_CHAR('9')  PORT_CHAR('(') PORT_CHAR('[')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)         PORT_CHAR('o')  PORT_CHAR('O')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)         PORT_CHAR('l')  PORT_CHAR('L')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)  PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_NAME("Ctrl")

	PORT_START("COL3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',')  PORT_CHAR('<')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR('\'') PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)         PORT_CHAR('m')  PORT_CHAR('M')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)         PORT_CHAR('j')  PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)         PORT_CHAR('u')  PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)         PORT_CHAR('7')  PORT_CHAR('&') PORT_CHAR('{')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)      PORT_CHAR(UCHAR_SHIFT_2) PORT_NAME("Fn")

	PORT_START("COL4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)     PORT_CHAR(' ')                   PORT_NAME("Space")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)         PORT_CHAR('r')  PORT_CHAR('R')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)         PORT_CHAR('4')  PORT_CHAR('$')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)         PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)         PORT_CHAR('t')  PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)         PORT_CHAR('g')  PORT_CHAR('G')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b')  PORT_CHAR('B')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('~')

	PORT_START("COL5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)       PORT_NAME("Menu")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)         PORT_CHAR('f')  PORT_CHAR('F')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)         PORT_CHAR('v')  PORT_CHAR('V')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)         PORT_CHAR('c')  PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)         PORT_CHAR('d')  PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)         PORT_CHAR('e')  PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)         PORT_CHAR('3')  PORT_CHAR(163)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('/')  PORT_CHAR('?')

	PORT_START("COL6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Notify")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)         PORT_CHAR('q')  PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('a')  PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)         PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)         PORT_CHAR('s')  PORT_CHAR('S')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)         PORT_CHAR('w')  PORT_CHAR('W')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)         PORT_CHAR('x')  PORT_CHAR('X')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)       PORT_CHAR(27)   PORT_NAME("Esc")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)         PORT_CHAR('1')  PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)         PORT_CHAR('2')  PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)         PORT_CHAR('6')  PORT_CHAR('^')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.')  PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)        PORT_CHAR(UCHAR_MAMEKEY(UP))     PORT_NAME(u8"\u2191") // U+2191 = ↑
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)         PORT_CHAR('h')  PORT_CHAR('H')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


static DEVICE_INPUT_DEFAULTS_START(debug_port)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD",   0xff, RS232_BAUD_115200)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY",   0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END


QUICKLOAD_LOAD_MEMBER(psion7_state::quickload_cb)
{
	// load the OS image into RAM (this would normally be loaded from CF by the bootloader)
	uint8_t *ram = m_ram->pointer() + 0x1000000;
	image.fseek(0x100, SEEK_SET); // skip image header
	image.fread(ram, image.length() - 0x100);

	// patch the bootloader to jump to the OS in RAM
	uint32_t *rom = &memregion("flash")->as_u32();
	rom[0x0000/4] = 0xea00003c; // B    0x000000F8
	rom[0x00f8/4] = 0xe51ff004; // LDR  R15, 0x000000FC
	rom[0x00fc/4] = 0xc8000000; // C8000000
	m_maincpu->space(AS_PROGRAM).install_rom(0x00000000, 0x001fffff, rom);

	machine().schedule_soft_reset();

	return std::make_pair(std::error_condition(), std::string());
}


static void pcmcia_devices(device_slot_interface &device)
{
	device.option_add("cf", ATA_FLASH_PCCARD);
}


void psion7_state::psion7(machine_config &config)
{
	SA1100(config, m_maincpu, 132710000);
	m_maincpu->set_addrmap(AS_PROGRAM, &psion7_state::s7_map);

	SA1110_PERIPHERALS(config, m_sa1100, 132710000, m_maincpu);
	m_sa1100->uart3_tx_out().set("uart3", FUNC(rs232_port_device::write_txd));
	m_sa1100->set_screen_origin(71, 0);
	m_sa1100->set_screen("screen");

	rs232_port_device &uart3(RS232_PORT(config, "uart3", default_rs232_devices, nullptr));
	uart3.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(debug_port));

	//ADS7843(config, "adc", 0);

	RAM(config, m_ram).set_default_size("16M").set_extra_options("32M");
	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	THERA(config, m_thera, 3.6864_MHz_XTAL);
	m_thera->int_cb().set(m_sa1100, FUNC(sa1110_periphs_device::gpio_in<10>));
	m_thera->col_cb().set([this](uint8_t data) { m_key_scan = data; });
	m_thera->spi_r().set(FUNC(psion7_state::ads7843_r));
	m_thera->porta_r().set(FUNC(psion7_state::kbd_r));
	//m_thera->portb_r().set(FUNC(psion7_state::kbd_r));
	m_thera->portc_w().set(m_buzzer, FUNC(speaker_sound_device::level_w)).bit(0);

	PCCARD_SLOT(config, m_pccard[0], pcmcia_devices, "cf").set_fixed(true);
	//m_pccard[0]->cd1().set(m_thera, FUNC(thera_device::write_pd));
	//m_pccard[0]->cd2().set(m_thera, FUNC(thera_device::write_pd));
	//m_pccard[0]->bvd1().set(m_thera, FUNC(thera_device::write_pd));
	//m_pccard[0]->bvd2().set(m_thera, FUNC(thera_device::write_pd));

	PCCARD_SLOT(config, m_pccard[1], pcmcia_devices, nullptr);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_screen_update(m_sa1100, FUNC(sa1110_periphs_device::screen_update));
	screen.set_size(783, 480); // PEN 783x480 LCD 640x480
	screen.set_visarea_full();

	PSION_CODEC(config, m_codec, 8000).add_route(ALL_OUTPUTS, "mono", 1.0); // TODO: MSM7716

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_buzzer).add_route(ALL_OUTPUTS, "mono", 1.0);

	MICROPHONE(config, m_mic, 1).front_center();
	m_mic->add_route(0, m_codec, 1.0);

	EEPROM_93C46_16BIT(config, m_eeprom);
}

void psion7_state::netbook(machine_config& config)
{
	psion7(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &psion7_state::nb_map);

	m_ram->set_default_size("32M").set_extra_options("64M");

	SHARP_LH28F160S3(config, "flash");

	quickload_image_device &quickload(QUICKLOAD(config, "quickload", "img"));
	quickload.set_load_callback(FUNC(psion7_state::quickload_cb));
	quickload.set_interface("psion_quik");

	SOFTWARE_LIST(config, "quik_ls").set_original("psion_quik").set_filter("NB");
}


ROM_START( psion7 )
	ROM_REGION32_LE(0x1000000, "maincpu", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "756", "V1.05(254) Build 756")
	ROMX_LOAD("s7_105_254_build_756.rom", 0x000000, 0x1000000, CRC(56162c36) SHA1(8e78d474185b443d10267d18446f2524b49fdfcc), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "754", "V1.05(254) Build 754")
	ROMX_LOAD("s7_105_254_build_754.rom", 0x000000, 0x1000000, CRC(2447e9bb) SHA1(288e9ecaf67f56794e65bf2cf90fbd54a5c8eb4a), ROM_BIOS(1))

	ROM_REGION16_LE(0x80, "eeprom", ROMREGION_ERASEFF)
ROM_END

ROM_START( netbook )
	ROM_REGION16_BE(0x200000, "flash", 0)
	// Other known versions: Version 1.0 04/04/01
	ROM_SYSTEM_BIOS(0, "011", "Version 011 08/08/00")
	ROMX_LOAD("nb_bl_v011.bin", 0x0000, 0x200000, CRC(d98d3302) SHA1(8879a68a76bcd74d2f256b079d7a80b46c26942f), ROM_BIOS(0))

	ROM_REGION16_LE(0x80, "eeprom", ROMREGION_ERASEFF)
ROM_END

} // anonymous namespace


//    YEAR  NAME         PARENT     COMPAT  MACHINE    INPUT      CLASS           INIT        COMPANY       FULLNAME               FLAGS
COMP( 1999, netbook,     0,         0,      netbook,   psion7,    psion7_state,   init_nb,    "Psion",      "netBook",             MACHINE_NOT_WORKING )
COMP( 2000, psion7,      0,         0,      psion7,    psion7,    psion7_state,   init_s7,    "Psion",      "Series 7",            MACHINE_NOT_WORKING )
