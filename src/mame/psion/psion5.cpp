// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Psion Series 5 (EPOC R3)

    TODO:
    - fix CODEC audio playback/record causes reboot
    - UART support
    - Compact Flash (Etna)
    - Bezel artwork

****************************************************************************/

#include "emu.h"
#include "codec.h"
#include "etna.h"

#include "bus/pccard/ataflash.h"
#include "cpu/arm7/arm7.h"
#include "machine/adc1213x.h"
#include "machine/clps7110.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class psion5_state : public driver_device
{
public:
	psion5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_soc(*this, "soc")
		, m_etna(*this, "etna")
		, m_pccard(*this, "pccard")
		, m_ram(*this, "ram")
		, m_nvram(*this, "nvram")
		, m_eeprom(*this, "eeprom")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_buzzer(*this, "buzzer")
		, m_codec(*this, "codec")
		, m_mic(*this, "mic")
		, m_touchx(*this, "TOUCHX")
		, m_touchy(*this, "TOUCHY")
		, m_touch(*this, "TOUCH")
		, m_kbd_cols(*this, "COL%u", 0U)
	{
	}

	void psion5(machine_config &config) ATTR_COLD;

	void init_s5() ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(touch_down);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void palette_init(palette_device &palette) const ATTR_COLD;

	void s5_map(address_map &map) ATTR_COLD;

	uint8_t keyboard_r();
	void portb_w(uint8_t data);
	void portd_w(uint8_t data);
	uint16_t adc_r(offs_t offset);
	void update_amp();

	required_device<arm710a_cpu_device> m_maincpu;
	required_device<clps7110_device> m_soc;
	required_device<etna_device> m_etna;
	required_device<pccard_slot_device> m_pccard;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<speaker_sound_device> m_buzzer;
	optional_device<psion_codec_device> m_codec;
	optional_device<microphone_device> m_mic;
	required_ioport m_touchx;
	required_ioport m_touchy;
	required_ioport m_touch;
	required_ioport_array<8> m_kbd_cols;

	uint8_t m_kbd_scan = 0;
	uint8_t m_volume = 0;
	bool m_amp_enable = true;
};


void psion5_state::init_s5()
{
	uint8_t *eeprom = memregion("eeprom")->base();

	// defaults expected by the touchscreen code
	eeprom[0x0a] = 20;
	eeprom[0x0b] = 20;
	eeprom[0x0c] = 20;
	eeprom[0x0d] = 30;

	// machine unique ID
	eeprom[0x1b] = 0xde;
	eeprom[0x1a] = 0xad;
	eeprom[0x19] = 0xbe;
	eeprom[0x18] = 0xef;

	// calculate the checksum
	uint8_t chksum = 0;
	for (int i = 1; i < 0x20; i++)
		chksum ^= eeprom[i];

	// EPOC is expecting 0x42
	eeprom[0x00] = chksum ^ 0x42;
}


void psion5_state::machine_start()
{
	// install RAM
	switch (m_ram->size())
	{
	case 0x1000000:
		m_maincpu->space(AS_PROGRAM).install_ram(0xc0000000, 0xc07fffff, 0x0f800000, m_ram->pointer());
		m_maincpu->space(AS_PROGRAM).install_ram(0xd0000000, 0xd07fffff, 0x0f800000, m_ram->pointer() + 0x0800000);
		break;

	case 0x0800000:
		m_maincpu->space(AS_PROGRAM).install_ram(0xc0000000, 0xc03fffff, 0x0fc00000, m_ram->pointer());
		m_maincpu->space(AS_PROGRAM).install_ram(0xd0000000, 0xd03fffff, 0x0fc00000, m_ram->pointer() + 0x0400000);
		break;

	case 0x0400000:
		m_maincpu->space(AS_PROGRAM).install_ram(0xc0000000, 0xc03fffff, 0x0fc00000, m_ram->pointer());
		break;
	}

	m_nvram->set_base(m_ram->pointer(), m_ram->size());

	save_item(NAME(m_kbd_scan));
	save_item(NAME(m_volume));
	save_item(NAME(m_amp_enable));
}

void psion5_state::machine_reset()
{
	m_kbd_scan = 0;
}


uint8_t psion5_state::keyboard_r()
{
	uint8_t data = 0x00;

	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_kbd_scan, i))
			data |= m_kbd_cols[i]->read();
	}

	return data;
}


void psion5_state::portb_w(uint8_t data)
{
	m_eeprom->cs_write(BIT(data, 0));
	m_eeprom->clk_write(BIT(data, 1));
}

void psion5_state::portd_w(uint8_t data)
{
	m_codec->pdn_w(BIT(data, 0)); // CODEN

	m_amp_enable = BIT(data, 1); // AMPEN
	update_amp();
}


uint16_t psion5_state::adc_r(offs_t offset)
{
	uint16_t data = 0xffff;

	// TODO: Proper SSP support with ADC12138 device
	switch (offset & 0xff)
	{
	case 0x0c: // unknown
		data = 0;
		break;
	case 0xc1: // Digitiser X
		data = 4079 - (uint16_t)(m_touchx->read() * 5.7);
		break;
	case 0x81: // Digitiser Y
		data = 3834 - (uint16_t)(m_touchy->read() * 13.225);
		break;
	case 0x91: // Main battery
		data = 3000;
		break;
	case 0xd1: // Backup Battery
		data = 3100;
		break;
	case 0xa1: // Reference
		data = 1000;
		break;
	}

	return data;
}


void psion5_state::update_amp()
{
	// TODO: MSC1192 speaker amplifier, can be put into standby to mute audio.
	static const float codec_volume[4] = { 1.0f, 0.75f, 0.5f, 0.25f };

	if (m_amp_enable)
		m_codec->set_output_gain(ALL_OUTPUTS, codec_volume[m_volume]); // VOL
	else
		m_codec->set_output_gain(ALL_OUTPUTS, 0.0);
}


void psion5_state::s5_map(address_map &map)
{
	map(0x00000000, 0x003fffff).mirror(0x0fc00000).rom().region("maincpu", 0);
	map(0x10000000, 0x103fffff).mirror(0x0fc00000).rom().region("maincpu", 0x400000);
	map(0x20000000, 0x20000fff).rw(m_etna, FUNC(etna_device::regs_r), FUNC(etna_device::regs_w));
	map(0x80000000, 0x80001fff).rw(m_soc, FUNC(clps7110_device::periphs_r), FUNC(clps7110_device::periphs_w));
}


void psion5_state::palette_init(palette_device &palette) const
{
	for (int i = 0; i < 16; i++)
	{
		const int r = (0x99 * i) / 15;
		const int g = (0xaa * i) / 15;
		const int b = (0x88 * i) / 15;
		m_palette->set_pen_color(15 - i, rgb_t(r, g, b));
	}
}


INPUT_CHANGED_MEMBER(psion5_state::touch_down)
{
	m_soc->eint3_w(newval ? ASSERT_LINE : CLEAR_LINE);
}

INPUT_PORTS_START( psion5 )
	PORT_START("TOUCHX")
	PORT_BIT(0x3ff, 362, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0,694) PORT_SENSITIVITY(25) PORT_KEYDELTA(13)

	PORT_START("TOUCHY")
	PORT_BIT(0x1ff, 125, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0,279) PORT_SENSITIVITY(25) PORT_KEYDELTA(13)

	PORT_START("TOUCH")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Touch") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psion5_state::touch_down), 0)
	PORT_BIT(0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)         PORT_CHAR('6') PORT_CHAR('^') PORT_CHAR('>')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)         PORT_CHAR('5') PORT_CHAR('%') PORT_CHAR('<')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)         PORT_CHAR('4') PORT_CHAR('$') PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)         PORT_CHAR('3') PORT_CHAR(163) PORT_CHAR('\\')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)         PORT_CHAR('2') PORT_CHAR('"') PORT_CHAR('#')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)         PORT_CHAR('1') PORT_CHAR('!') PORT_CHAR('_')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)    PORT_NAME("Dictaphone Record")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)     PORT_CHAR('\'') PORT_CHAR('~') PORT_CHAR(':')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)   PORT_NAME("Del<-")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)         PORT_CHAR('0') PORT_CHAR(')') PORT_CHAR('}')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)         PORT_CHAR('9') PORT_CHAR('(') PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('*') PORT_CHAR(']')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)         PORT_CHAR('7') PORT_CHAR('&') PORT_CHAR('[')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)      PORT_NAME("Dictaphone Play")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)         PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)         PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)         PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)         PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(128)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)         PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)         PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)       PORT_CHAR(27)  PORT_NAME("Esc")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)     PORT_CHAR(13)  PORT_NAME("Enter")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)         PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(';')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)         PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR('=')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)         PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR('-')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)         PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR('+')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)         PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)       PORT_NAME("Menu")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)         PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)         PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)         PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)         PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)       PORT_CHAR(9)   PORT_NAME("Tab  Caps")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)  PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_NAME("Ctrl")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)      PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_NAME(u8"\u2193") // U+2193 = ↓
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)         PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)         PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)         PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)         PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)      PORT_CHAR(UCHAR_SHIFT_2) PORT_NAME("Fn")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)         PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)         PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)         PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)         PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)         PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)    PORT_NAME("Right Shift")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)     PORT_CHAR(UCHAR_MAMEKEY(RIGHT))  PORT_NAME(u8"\u2192") // U+2192 = →
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)      PORT_CHAR(UCHAR_MAMEKEY(LEFT))   PORT_NAME(u8"\u2190") // U+2190 = ←
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR('/')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)        PORT_CHAR(UCHAR_MAMEKEY(UP))     PORT_NAME(u8"\u2191") // U+2191 = ↑
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)     PORT_CHAR(' ')                   PORT_NAME("Space")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)                                       PORT_NAME("Dictaphone Stop")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)    PORT_CHAR(UCHAR_SHIFT_1)         PORT_NAME("Left Shift")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

INPUT_PORTS_START( psion5_de )
	PORT_INCLUDE(psion5)

	PORT_MODIFY("COL2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('z') PORT_CHAR('Z')

	PORT_MODIFY("COL6")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('y') PORT_CHAR('Y')
INPUT_PORTS_END


static void pcmcia_devices(device_slot_interface &device)
{
	device.option_add("cf", ATA_FLASH_PCCARD);
}


void psion5_state::psion5(machine_config &config)
{
	ARM710A(config, m_maincpu, 3.6864_MHz_XTAL * 5);
	m_maincpu->set_addrmap(AS_PROGRAM, &psion5_state::s5_map);

	CLPS7110(config, m_soc, 3.6864_MHz_XTAL, m_maincpu);
	m_soc->lcd_dma_cb().set(m_ram, FUNC(ram_device::read));
	m_soc->porta_r().set(FUNC(psion5_state::keyboard_r));
	m_soc->portb_w().set(FUNC(psion5_state::portb_w));
	m_soc->portd_w().set(FUNC(psion5_state::portd_w));
	m_soc->pcm_in().set(m_codec, FUNC(psion_codec_device::pcm_out));
	m_soc->pcm_out().set(m_codec, FUNC(psion_codec_device::pcm_in));
	m_soc->buz_cb().set(m_buzzer, FUNC(speaker_sound_device::level_w));
	m_soc->col_cb().set([this](uint8_t data) { m_kbd_scan = data; });
	m_soc->adc_r().set(FUNC(psion5_state::adc_r));
	m_soc->set_screen_origin(55, 0);
	m_soc->set_screen("screen");

	ADC12138(config, "adc");

	RAM(config, m_ram).set_default_size("8M").set_extra_options("4MB, 16M");
	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	ETNA(config, m_etna);
	m_etna->porta_r().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::do_read)).lshift(3);
	m_etna->porta_w().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::di_write)).bit(2);

	PCCARD_SLOT(config, m_pccard, pcmcia_devices, "cf").set_fixed(true);
	//m_pccard->cd1().set(m_etna, FUNC(etna_device::write_pc1_cd1));
	//m_pccard->cd2().set(m_etna, FUNC(etna_device::write_pc1_cd2));
	//m_pccard->bvd1().set(m_etna, FUNC(etna_device::write_pc1_bvd1));
	//m_pccard->bvd2().set(m_etna, FUNC(etna_device::write_pc1_bvd2));
	//m_pccard->wp().set(m_etna, FUNC(etna_device::write_pc1_wp));

	SCREEN(config, m_screen).set_lcd();
	m_screen->set_refresh_hz(58);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_screen_update(m_soc, FUNC(clps7110_device::screen_update));
	m_screen->set_size(695, 280); // PEN 695x280 LCD 640x240
	m_screen->set_visarea_full();
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(psion5_state::palette_init), 16);

	EEPROM_93C06_16BIT(config, m_eeprom); // 93S06
	m_eeprom->enable_streaming(true);

	PSION_CODEC(config, m_codec, 8000).add_route(ALL_OUTPUTS, "mono", 1.0); // TODO: MSM7702

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_buzzer).add_route(ALL_OUTPUTS, "mono", 1.0);

	MICROPHONE(config, m_mic, 1).front_center();
	m_mic->add_route(0, m_codec, 1.0);
}


ROM_START( psion5 )
	// Known versions: English, American, French, German, Italian, Spanish, Swedish, Norwegian, Danish, Finnish
	ROM_REGION32_LE(0x800000, "maincpu", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "145", "V1.01(145)")
	ROMX_LOAD("s5_uk_v145.rom", 0x000000, 0x600000, CRC(1d386469) SHA1(41e7d011c7f42bd6c0c72b22ce09597fa4d161c3), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "209", "V1.02(209)")
	ROMX_LOAD("s5_uk_v209.rom", 0x000000, 0x600000, CRC(8ceb01a1) SHA1(701f9af79134956e1d5381a0d2330134a2b1b1de), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "113", "V1.00(113)") // TODO: this fails to boot, maybe needs a wakeup signal.
	ROMX_LOAD("s5_uk_v113.rom", 0x000000, 0x600000, CRC(b4dd658a) SHA1(1d37127195ce4074b2c8b3d1f771ad8cd0e8d0c6), ROM_BIOS(2)) // 3 x 2MB

	ROM_REGION16_LE(0x20, "eeprom", ROMREGION_ERASE00)
ROM_END

ROM_START( psion5_de )
	ROM_REGION32_LE(0x800000, "maincpu", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "145", "V1.01(145)")
	ROMX_LOAD("s5_grm_v145.rom", 0x000000, 0x600000, CRC(8f45aff3) SHA1(756baa9d7461025e299f003a0e052137bb6075b9), ROM_BIOS(0))

	ROM_REGION16_LE(0x20, "eeprom", ROMREGION_ERASE00)
ROM_END

ROM_START( psion5_nl )
	ROM_REGION32_LE(0x800000, "maincpu", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "145", "V1.01(145)")
	ROMX_LOAD("s5_nl_v145.rom", 0x000000, 0x600000, CRC(2eccb3b0) SHA1(0c9e96e940a415664047eaf566141b2c0dcf4057), ROM_BIOS(0))

	ROM_REGION16_LE(0x20, "eeprom", ROMREGION_ERASE00)
ROM_END

ROM_START( psion5_it )
	ROM_REGION32_LE(0x800000, "maincpu", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "145", "V1.01(145)")
	ROMX_LOAD("s5_it_v145.rom", 0x000000, 0x600000, CRC(798fcc74) SHA1(bc8cd98f8f7a8e314954ed707274e3b0f2eff27a), ROM_BIOS(0))

	ROM_REGION16_LE(0x20, "eeprom", ROMREGION_ERASE00)
ROM_END

} // anonymous namespace


//    YEAR  NAME           PARENT     COMPAT  MACHINE    INPUT       CLASS          INIT      COMPANY       FULLNAME                FLAGS
COMP( 1997, psion5,        0,         0,      psion5,    psion5,     psion5_state,  init_s5,  "Psion",      "Series 5",             MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1997, psion5_de,     psion5,    0,      psion5,    psion5_de,  psion5_state,  init_s5,  "Psion",      "Series 5 (German)",    MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1997, psion5_nl,     psion5,    0,      psion5,    psion5,     psion5_state,  init_s5,  "Psion",      "Series 5 (Dutch)",     MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1997, psion5_it,     psion5,    0,      psion5,    psion5,     psion5_state,  init_s5,  "Psion",      "Series 5 (Italian)",   MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
