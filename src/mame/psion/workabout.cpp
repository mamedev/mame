// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion Workabout

    TODO:
    - expansion LIF ports

******************************************************************************/

#include "emu.h"
#include "machine/nvram.h"
#include "machine/psion_asic9.h"
#include "machine/psion_ssd.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"
//#include "bus/psion/exp/slot.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "utf8.h"


namespace {

class workabout_state : public driver_device
{
public:
	workabout_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_asic9(*this, "asic9")
		, m_ram(*this, "ram")
		, m_nvram(*this, "nvram")
		, m_palette(*this, "palette")
		, m_keyboard(*this, "COL%u", 0U)
		, m_speaker(*this, "speaker")
		, m_ssd(*this, "ssd%u", 1U)
		//, m_exp(*this, "exp")
	{ }

	void workabout(machine_config &config);
	void psionwa(machine_config &config);
	void psionwamx(machine_config &config);

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
	required_device_array<psion_ssd_device, 2> m_ssd;
	//required_device<psion_exp_slot_device> m_exp;

	void palette_init(palette_device &palette);

	uint16_t kbd_r();

	uint8_t m_key_col = 0;
};


void workabout_state::machine_start()
{
	m_nvram->set_base(m_ram->pointer(), m_ram->size());
}

void workabout_state::machine_reset()
{
}


static INPUT_PORTS_START( workabout )
	PORT_START("COL0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s')  PORT_CHAR('S')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u')  PORT_CHAR('U')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')  PORT_CHAR('Q')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a')  PORT_CHAR('A')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')  PORT_CHAR('?')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6')  PORT_CHAR('^')  PORT_CHAR('}')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))                  PORT_NAME(UTF8_DOWN)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))                   PORT_NAME("On/Esc")          PORT_CHANGED_MEMBER(DEVICE_SELF, workabout_state, wakeup, 0)

	PORT_START("COL1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t')  PORT_CHAR('T')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v')  PORT_CHAR('V')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r')  PORT_CHAR('R')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b')  PORT_CHAR('B')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)                                   PORT_NAME("Enter")
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))                 PORT_NAME(UTF8_RIGHT)
	PORT_BIT(0x180, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)                        PORT_NAME("Shift")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w')  PORT_CHAR('W')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g')  PORT_CHAR('G')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c')  PORT_CHAR('C')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1')  PORT_CHAR('!')                  PORT_NAME("1 ! @")
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')  PORT_CHAR('&')  PORT_CHAR('\'')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(0x08)                                 PORT_NAME("Del")
	PORT_BIT(0x180, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)       PORT_CHAR(UCHAR_SHIFT_2)                        PORT_NAME("Psion")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x')  PORT_CHAR('X')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h')  PORT_CHAR('H')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d')  PORT_CHAR('D')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2')  PORT_CHAR('"')  PORT_CHAR('#')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('(')  PORT_CHAR('[')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_CHAR(0x09)                                 PORT_NAME("Tab")
	PORT_BIT(0x180, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')  PORT_CHAR('Y')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m')  PORT_CHAR('M')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i')  PORT_CHAR('I')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e')  PORT_CHAR('E')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3')  PORT_CHAR(0xa3) PORT_CHAR('\\')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR(')')  PORT_CHAR(']')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))                    PORT_NAME(UTF8_UP)
	PORT_BIT(0x180, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n')  PORT_CHAR('N')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j')  PORT_CHAR('J')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f')  PORT_CHAR('F')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('+')  PORT_CHAR('=')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR('*')  PORT_CHAR(':')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)        PORT_CHAR(UCHAR_MAMEKEY(F11))                   PORT_NAME("Menu")
	PORT_BIT(0x180, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))              PORT_NAME("Control")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o')  PORT_CHAR('O')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k')  PORT_CHAR('K')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')  PORT_CHAR(';')  PORT_CHAR('<')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4')  PORT_CHAR('$')  PORT_CHAR('~')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)                                                        PORT_NAME("Off")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                                                               PORT_NAME("Contrast")
	PORT_BIT(0x180, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l')  PORT_CHAR('L')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')  PORT_CHAR(',')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5')  PORT_CHAR('%')  PORT_CHAR('{')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))                  PORT_NAME(UTF8_LEFT)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                                                               PORT_NAME("Backlight")
	PORT_BIT(0x180, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


INPUT_CHANGED_MEMBER(workabout_state::wakeup)
{
	m_asic9->eint0_w(newval);
}


uint16_t workabout_state::kbd_r()
{
	uint16_t data = 0x00;

	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_key_col, i))
			data |= m_keyboard[i]->read();
	}

	return data;
}


void workabout_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(190, 220, 190));
	palette.set_pen_color(1, rgb_t(130, 130, 110));
	palette.set_pen_color(2, rgb_t(190, 210, 180));
}


void workabout_state::workabout(machine_config &config)
{
	PSION_ASIC9(config, m_asic9, 7.68_MHz_XTAL); // V30H
	m_asic9->set_screen("screen");
	m_asic9->set_ram_rom("ram", "rom");
	m_asic9->port_ab_r().set(FUNC(workabout_state::kbd_r));
	m_asic9->buz_cb().set(m_speaker, FUNC(speaker_sound_device::level_w));
	m_asic9->col_cb().set([this](uint8_t data) { m_key_col = data; });
	m_asic9->data_r<0>().set(m_ssd[1], FUNC(psion_ssd_device::data_r));      // SSD Pack 1
	m_asic9->data_w<0>().set(m_ssd[1], FUNC(psion_ssd_device::data_w));
	m_asic9->data_r<1>().set(m_ssd[0], FUNC(psion_ssd_device::data_r));      // SSD Pack 2
	m_asic9->data_w<1>().set(m_ssd[0], FUNC(psion_ssd_device::data_w));
	//m_asic9->data_r<2>().set(m_exp[0], FUNC(psion_exp_slot_device::data_r)); // Expansion port A
	//m_asic9->data_w<2>().set(m_exp[0], FUNC(psion_exp_slot_device::data_w));
	//m_asic9->data_r<3>().set(m_exp[1], FUNC(psion_exp_slot_device::data_r)); // Expansion port B
	//m_asic9->data_w<3>().set(m_exp[1], FUNC(psion_exp_slot_device::data_w));
	//m_asic9->data_r<4>().set(m_exp[2], FUNC(psion_exp_slot_device::data_r)); // Expansion port C
	//m_asic9->data_w<4>().set(m_exp[2], FUNC(psion_exp_slot_device::data_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_size(240, 100);
	screen.set_visarea_full();
	screen.set_refresh_hz(66);
	screen.set_screen_update(m_asic9, FUNC(psion_asic9_device::screen_update));
	screen.set_palette(m_palette);
	PALETTE(config, "palette", FUNC(workabout_state::palette_init), 3);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00); // Piezo buzzer

	RAM(config, m_ram);
	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	PSION_SSD(config, m_ssd[0]);
	m_ssd[0]->door_cb().set(m_asic9, FUNC(psion_asic9_device::medchng_w));
	PSION_SSD(config, m_ssd[1]);
	m_ssd[1]->door_cb().set(m_asic9, FUNC(psion_asic9_device::medchng_w));

	//PSION_EXP_SLOT(config, m_exp, psion_exp_devices, nullptr);

	SOFTWARE_LIST(config, "ssd_list").set_original("psion_ssd").set_filter("WA");
}


void workabout_state::psionwa(machine_config &config)
{
	workabout(config);

	m_ram->set_default_size("1M");
}

void workabout_state::psionwamx(machine_config &config)
{
	workabout(config);

	m_asic9->set_clock(3.6864_MHz_XTAL * 15 / 2); // V30MX

	m_ram->set_default_size("2M");
}


ROM_START(psionwa)
	ROM_REGION16_LE(0x200000, "rom", 0)
	ROM_SYSTEM_BIOS(0, "240f", "V2.40F 060897")
	ROMX_LOAD("w1_v2.40f.bin", 0x00000, 0x200000, CRC(4ef1d380) SHA1(d155edf7995c2a799525b53079fff9fb68789f0f), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "100f", "V1.00F 010195")
	ROMX_LOAD("w1_v1.00f.bin", 0x00000, 0x100000, CRC(8fd7c127) SHA1(316fdc6c54629470f1af3295c46a2e7d58ebdca9), ROM_BIOS(1))
	ROM_RELOAD(0x100000, 0x100000)
	ROM_SYSTEM_BIOS(2, "024b", "V0.24B 120296")
	ROMX_LOAD("w1_v0.24b.bin", 0x00000, 0x200000, CRC(6e7e3016) SHA1(5c35eea431f975cdefc770697a88fe62a1e7af7f), ROM_BIOS(2))
ROM_END

ROM_START(psionwamx)
	ROM_REGION16_LE(0x200000, "rom", 0)
	ROM_SYSTEM_BIOS(0, "720f", "V7.20F 230798")
	ROMX_LOAD("w2mx_v7.20f.bin", 0x00000, 0x200000, CRC(63734683) SHA1(9d8aa1e45f52e7fcb52d6e81ac47f60d1104c35d), ROM_BIOS(0))
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY   FULLNAME        FLAGS
COMP( 1995, psionwa,   0,      0,      psionwa,   workabout, workabout_state, empty_init, "Psion",  "Workabout",    MACHINE_SUPPORTS_SAVE )
COMP( 1998, psionwamx, 0,      0,      psionwamx, workabout, workabout_state, empty_init, "Psion",  "Workabout mx", MACHINE_SUPPORTS_SAVE )
