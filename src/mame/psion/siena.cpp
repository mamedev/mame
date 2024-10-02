// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion Siena

******************************************************************************/

#include "emu.h"
#include "machine/nvram.h"
#include "machine/psion_asic9.h"
#include "machine/psion_condor.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"
#include "bus/psion/honda/slot.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "utf8.h"


namespace {

class siena_state : public driver_device
{
public:
	siena_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_asic9(*this, "asic9")
		, m_ram(*this, "ram")
		, m_nvram(*this, "nvram")
		, m_palette(*this, "palette")
		, m_keyboard(*this, "COL%u", 0U)
		, m_speaker(*this, "speaker")
		, m_condor(*this, "condor")
		, m_honda(*this, "honda")
	{ }

	void siena(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(wakeup);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<psion_asic9_device> m_asic9;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram;
	required_device<palette_device> m_palette;
	required_ioport_array<8> m_keyboard;
	required_device<speaker_sound_device> m_speaker;
	required_device<psion_condor_device> m_condor;
	required_device<psion_honda_slot_device> m_honda;

	void palette_init(palette_device &palette);

	uint16_t kbd_r();

	uint8_t m_key_col = 0;
};


void siena_state::machine_start()
{
	m_nvram->set_base(m_ram->pointer(), m_ram->size());
}

void siena_state::machine_reset()
{
	m_asic9->io_space().install_readwrite_handler(0x0100, 0x011f, read8sm_delegate(*m_condor, FUNC(psion_condor_device::read)), write8sm_delegate(*m_condor, FUNC(psion_condor_device::write)), 0x00ff);
}


static INPUT_PORTS_START( siena )
	PORT_START("COL0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3')                  PORT_NAME("3 MC")
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2')                  PORT_NAME("2 MR")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1')                  PORT_NAME("1 Min")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)                                                       PORT_NAME("On/CE")           PORT_CHANGED_MEMBER(DEVICE_SELF, siena_state, wakeup, 0)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_UNKNOWN)

	PORT_START("COL1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w')  PORT_CHAR('W') PORT_CHAR('"')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d')  PORT_CHAR('D') PORT_CHAR('/')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c')  PORT_CHAR('C')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')  PORT_CHAR(':')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o')  PORT_CHAR('O') PORT_CHAR('(')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL)   PORT_CHAR(UCHAR_SHIFT_2)                        PORT_NAME("Fn")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)                                                       PORT_NAME("IR Send")         PORT_CHANGED_MEMBER(DEVICE_SELF, siena_state, wakeup, 0)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))                    PORT_NAME("Word")            PORT_CHANGED_MEMBER(DEVICE_SELF, siena_state, wakeup, 0)

	PORT_START("COL2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e')  PORT_CHAR('E') PORT_CHAR(0xa3)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f')  PORT_CHAR('F') PORT_CHAR('~')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v')  PORT_CHAR('V')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))                   PORT_NAME("Help")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR('<')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p')  PORT_CHAR('P') PORT_CHAR(')')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)       PORT_CHAR(UCHAR_MAMEKEY(LALT))                  PORT_NAME("Psion")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)                                                       PORT_NAME("IR Receive")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))                    PORT_NAME("Agenda")

	PORT_START("COL3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r')  PORT_CHAR('R') PORT_CHAR('$')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g')  PORT_CHAR('G') PORT_CHAR('@')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b')  PORT_CHAR('B')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))                  PORT_NAME(UTF8_LEFT" Home")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')  PORT_CHAR('>')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=')                                  PORT_NAME("= %")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)                        PORT_NAME("Shift (L)")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))                    PORT_NAME("Time")            PORT_CHANGED_MEMBER(DEVICE_SELF, siena_state, wakeup, 0)

	PORT_START("COL4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t')  PORT_CHAR('T') PORT_CHAR('%')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h')  PORT_CHAR('H') PORT_CHAR('{')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n')  PORT_CHAR('N')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))                  PORT_NAME(UTF8_DOWN" Pg Dn")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))                    PORT_NAME(UTF8_UP" Pg Up")
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)                                   PORT_NAME("Enter")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR('+')                                  PORT_NAME("+ M+")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)                        PORT_NAME("Shift (R)")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))                    PORT_NAME("World")           PORT_CHANGED_MEMBER(DEVICE_SELF, siena_state, wakeup, 0)

	PORT_START("COL5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')  PORT_CHAR('Y') PORT_CHAR('^')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j')  PORT_CHAR('J') PORT_CHAR('}')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m')  PORT_CHAR('M')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))                 PORT_NAME(UTF8_RIGHT" End")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')  PORT_CHAR('?')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(0x08)                                 PORT_NAME("Del")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR('*')                                  PORT_NAME(u8"×")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)                                                        PORT_NAME("Off")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))                    PORT_NAME("Sheet")           PORT_CHANGED_MEMBER(DEVICE_SELF, siena_state, wakeup, 0)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))                    PORT_NAME("Calc")            PORT_CHANGED_MEMBER(DEVICE_SELF, siena_state, wakeup, 0)

	PORT_START("COL6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')  PORT_CHAR('Q') PORT_CHAR('!')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s')  PORT_CHAR('S') PORT_CHAR('\\')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x')  PORT_CHAR('X')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))              PORT_NAME("\xe2\x97\x86 Caps")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l')  PORT_CHAR('L') PORT_CHAR(']')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i')  PORT_CHAR('I') PORT_CHAR('*')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)  PORT_CHAR('/')                                  PORT_NAME(u8"÷")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR('-')                                  PORT_NAME("- M-")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))                   PORT_NAME("Esc")             PORT_CHANGED_MEMBER(DEVICE_SELF, siena_state, wakeup, 0)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))                    PORT_NAME("Data")            PORT_CHANGED_MEMBER(DEVICE_SELF, siena_state, wakeup, 0)

	PORT_START("COL7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_CHAR(0x09)                                 PORT_NAME("Tab")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a')  PORT_CHAR('A') PORT_CHAR('#')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)        PORT_CHAR(UCHAR_MAMEKEY(F11))                   PORT_NAME("Menu Info")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k')  PORT_CHAR('K') PORT_CHAR('[')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u')  PORT_CHAR('U') PORT_CHAR('&')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR('.')                                  PORT_NAME(". +/-")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))              PORT_NAME("Control")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))                    PORT_NAME("System")          PORT_CHANGED_MEMBER(DEVICE_SELF, siena_state, wakeup, 0)
INPUT_PORTS_END


INPUT_CHANGED_MEMBER(siena_state::wakeup)
{
	m_asic9->eint0_w(newval);
}


uint16_t siena_state::kbd_r()
{
	uint16_t data = 0x00;

	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_key_col, i))
			data |= m_keyboard[i]->read();
	}

	return data;
}


void siena_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(190, 220, 190));
	palette.set_pen_color(1, rgb_t(130, 130, 110));
	palette.set_pen_color(2, rgb_t(190, 210, 180));
}


void siena_state::siena(machine_config &config)
{
	PSION_ASIC9(config, m_asic9, 7.68_MHz_XTAL);
	m_asic9->set_screen("screen");
	m_asic9->set_ram_rom("ram", "rom");
	m_asic9->port_ab_r().set(FUNC(siena_state::kbd_r));
	m_asic9->buz_cb().set(m_speaker, FUNC(speaker_sound_device::level_w));
	m_asic9->col_cb().set([this](uint8_t data) { m_key_col = data; });

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_size(240, 160);
	screen.set_visarea_full();
	screen.set_refresh_hz(66);
	screen.set_screen_update(m_asic9, FUNC(psion_asic9_device::screen_update));
	screen.set_palette(m_palette);
	PALETTE(config, "palette", FUNC(siena_state::palette_init), 3);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00); // Piezo buzzer

	RAM(config, m_ram).set_default_size("512K").set_extra_options("1M");
	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	PSION_CONDOR(config, m_condor);
	m_condor->txd_handler().set(m_honda, FUNC(psion_honda_slot_device::write_txd));
	m_condor->rts_handler().set(m_honda, FUNC(psion_honda_slot_device::write_rts));
	m_condor->dtr_handler().set(m_honda, FUNC(psion_honda_slot_device::write_dtr));
	m_condor->int_handler().set(m_asic9, FUNC(psion_asic9_device::eint1_w));

	// Honda expansion port
	PSION_HONDA_SLOT(config, m_honda, psion_honda_devices, "ssd");
	m_honda->rxd_handler().set(m_condor, FUNC(psion_condor_device::write_rxd));
	m_honda->dcd_handler().set(m_condor, FUNC(psion_condor_device::write_dcd));
	m_honda->dsr_handler().set(m_condor, FUNC(psion_condor_device::write_dsr));
	m_honda->cts_handler().set(m_condor, FUNC(psion_condor_device::write_cts));
	m_honda->sdoe_handler().set(m_asic9, FUNC(psion_asic9_device::medchng_w));
	m_asic9->data_r<4>().set(m_honda, FUNC(psion_honda_slot_device::data_r));
	m_asic9->data_w<4>().set(m_honda, FUNC(psion_honda_slot_device::data_w));

	SOFTWARE_LIST(config, "ssd_list").set_original("psion_ssd").set_filter("SIENA");
}


ROM_START(siena)
	ROM_REGION16_LE(0x100000, "rom", 0)
	ROM_SYSTEM_BIOS(0, "420f", "V4.20F")
	ROMX_LOAD("vine_v4.20f.bin", 0x00000, 0x100000, CRC(641f8e7c) SHA1(fe0e46540e0aac5aabb2dd1b96689da41e8f55fb), ROM_BIOS(0))
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT    CLASS          INIT         COMPANY   FULLNAME        FLAGS
COMP( 1996, siena,    0,      0,      siena,    siena,   siena_state,   empty_init,  "Psion",  "Siena",        MACHINE_SUPPORTS_SAVE )
