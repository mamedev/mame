// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion Series 3

    TODO:
    - DTMF tone generator

******************************************************************************/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "machine/nvram.h"
#include "machine/psion_asic1.h"
#include "machine/psion_asic2.h"
#include "machine/psion_ssd.h"
#include "machine/ram.h"
#include "sound/pcd3311.h"
#include "sound/spkrdev.h"
#include "bus/psion/sibo/slot.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "utf8.h"


namespace {

class psion3_state : public driver_device
{
public:
	psion3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_asic1(*this, "asic1")
		, m_asic2(*this, "asic2")
		, m_ram(*this, "ram")
		, m_nvram(*this, "nvram")
		, m_palette(*this, "palette")
		, m_keyboard(*this, "COL%u", 0U)
		, m_dtmf(*this, "dtmf")
		, m_speaker(*this, "speaker")
		, m_ssd(*this, "ssd%u", 1U)
		, m_sibo(*this, "sibo")
	{ }

	void psion3(machine_config &config);
	void psion3s(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(key_on);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<psion_asic1_device> m_asic1;
	required_device<psion_asic2_device> m_asic2;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram;
	required_device<palette_device> m_palette;
	required_ioport_array<10> m_keyboard;
	required_device<pcd3311_device> m_dtmf;
	required_device<speaker_sound_device> m_speaker;
	required_device_array<psion_ssd_device, 2> m_ssd;
	required_device<psion_sibo_slot_device> m_sibo;

	void palette_init(palette_device &palette);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void asic1_map(address_map &map) ATTR_COLD;

	uint8_t port_data_r();
	void port_data_w(uint8_t data);
};


void psion3_state::machine_start()
{
	m_asic1->space(0).install_ram(0, m_ram->mask(), m_ram->pointer());
	m_nvram->set_base(m_ram->pointer(), m_ram->size());
}

void psion3_state::machine_reset()
{
}


void psion3_state::mem_map(address_map &map)
{
	map(0x00000, 0xfffff).rw(m_asic1, FUNC(psion_asic1_device::mem_r), FUNC(psion_asic1_device::mem_w));
}

void psion3_state::io_map(address_map &map)
{
	map(0x0000, 0x001f).rw(m_asic1, FUNC(psion_asic1_device::io_r), FUNC(psion_asic1_device::io_w));
	map(0x0080, 0x008f).rw(m_asic2, FUNC(psion_asic2_device::io_r), FUNC(psion_asic2_device::io_w)).umask16(0x00ff);
	//map(0x0100, 0x01ff).lr8(NAME([]() { return 0xff; })); // w: enable Vcc lines?
	map(0x0200, 0x02ff).w(m_dtmf, FUNC(pcd3311_device::write));
}

void psion3_state::asic1_map(address_map &map)
{
	map(0x00000, 0x7ffff).noprw();
	map(0x80000, 0xfffff).rom().region("flash", 0);
}


static INPUT_PORTS_START( psion3 )
	PORT_START("COL0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1')  PORT_CHAR('!')                  PORT_NAME("1 ! Off")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')  PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f')  PORT_CHAR('F')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r')  PORT_CHAR('R')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k')  PORT_CHAR('K')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')  PORT_CHAR('?')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))                 PORT_NAME(UTF8_RIGHT" End")

	PORT_START("COL1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2')  PORT_CHAR('"')  PORT_CHAR('#')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a')  PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v')  PORT_CHAR('V')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4')  PORT_CHAR('$')  PORT_CHAR('~')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))                   PORT_NAME("Help Dial")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i')  PORT_CHAR('I')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)                        PORT_NAME("Shift (R)")

	PORT_START("COL2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6')  PORT_CHAR('^')  PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c')  PORT_CHAR('C')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5')  PORT_CHAR('%')  PORT_CHAR('\'')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m')  PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('?')  PORT_CHAR('}')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('+')  PORT_CHAR('=')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')  PORT_CHAR('Y')

	PORT_START("COL3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')  PORT_CHAR('>')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)        PORT_CHAR(UCHAR_MAMEKEY(F11))                   PORT_NAME("Menu")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d')  PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t')  PORT_CHAR('T')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j')  PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR('(')  PORT_CHAR('[')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')  PORT_CHAR(')')  PORT_CHAR(']')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))                  PORT_NAME(UTF8_LEFT" Home")

	PORT_START("COL4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))                    PORT_NAME(UTF8_UP" Pg Up")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))              PORT_NAME("Caps Lock")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e')  PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g')  PORT_CHAR('G')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u')  PORT_CHAR('U')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o')  PORT_CHAR('O')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))                  PORT_NAME(UTF8_DOWN" Pg Dn")

	PORT_START("COL5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h')  PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x')  PORT_CHAR('X')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3')  PORT_CHAR(0xa3) PORT_CHAR('\\')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b')  PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')  PORT_CHAR('&')  PORT_CHAR('{')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l')  PORT_CHAR('L')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR('*')  PORT_CHAR(':')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n')  PORT_CHAR('N')

	PORT_START("COL6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w')  PORT_CHAR('W')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s')  PORT_CHAR('S')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))              PORT_NAME("Control")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)                        PORT_NAME("Shift (L)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)       PORT_CHAR(UCHAR_SHIFT_2)                        PORT_NAME("Psion")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(0x08)                                 PORT_NAME("Delete")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)                                   PORT_NAME("Enter")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')

	PORT_START("COL7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))                    PORT_NAME("Data")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))                    PORT_NAME("Time")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))                    PORT_NAME("Program")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_CHAR(0x09)                                 PORT_NAME("Tab")

	PORT_START("COL8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))                    PORT_NAME("Word")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))                    PORT_NAME("World")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0xf8, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))                    PORT_NAME("System")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))                    PORT_NAME("Agenda")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))                    PORT_NAME("Calc")
	PORT_BIT(0xf8, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ESC")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC)) PORT_NAME("Esc On") PORT_CHANGED_MEMBER(DEVICE_SELF, psion3_state, key_on, 0)
INPUT_PORTS_END


static INPUT_PORTS_START( psion3s )
	PORT_INCLUDE(psion3)

	PORT_MODIFY("COL7")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))                    PORT_NAME("Sheet")
INPUT_PORTS_END


static INPUT_PORTS_START( pocketbk )
	PORT_INCLUDE(psion3)

	PORT_MODIFY("COL1")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))                   PORT_NAME("Help")

	PORT_MODIFY("COL6")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)       PORT_CHAR(UCHAR_SHIFT_2)                        PORT_NAME("Acorn")

	PORT_MODIFY("COL7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))                    PORT_NAME("Cards")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))                    PORT_NAME("Time")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))                    PORT_NAME("Key2")

	PORT_MODIFY("COL8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))                    PORT_NAME("Write")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))                    PORT_NAME("Calc")

	PORT_MODIFY("COL9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))                    PORT_NAME("Desktop")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))                    PORT_NAME("Abacus")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))                    PORT_NAME("Key1")
INPUT_PORTS_END


INPUT_CHANGED_MEMBER(psion3_state::key_on)
{
	if (newval)
	{
		m_asic2->on_clr_w(newval);
	}
}


uint8_t psion3_state::port_data_r()
{
	// b0 MainBattery   - 0 Low, 1 Good
	// b1 BackupBattery - 0 Low, 1 Good
	// b2 ?
	// b3 ExternalPower - 0 Yes, 1 No
	// b7 NC
	return 0x03;
}

void psion3_state::port_data_w(uint8_t data)
{
	// b4 ?
	// b5 VOL0
	// b6 VOL1
	logerror("port_data_w: %02x\n", data);
}


void psion3_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(170, 180, 160));
	palette.set_pen_color(1, rgb_t(80, 75, 50));
}


void psion3_state::psion3(machine_config &config)
{
	V30(config, m_maincpu, 7.68_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &psion3_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &psion3_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(m_asic1, FUNC(psion_asic1_device::inta_cb));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_size(240, 80);
	screen.set_visarea_full();
	screen.set_refresh_hz(66);
	screen.set_screen_update(m_asic1, FUNC(psion_asic1_device::screen_update_single));
	screen.set_palette(m_palette);
	PALETTE(config, "palette", FUNC(psion3_state::palette_init), 2);

	PSION_ASIC1(config, m_asic1, 7.68_MHz_XTAL);
	m_asic1->set_screen("screen");
	m_asic1->set_laptop_mode(false);
	m_asic1->set_addrmap(0, &psion3_state::asic1_map);
	m_asic1->int_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_asic1->nmi_cb().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_asic1->frcovl_cb().set(m_asic2, FUNC(psion_asic2_device::frcovl_w));

	PSION_ASIC2(config, m_asic2, 7.68_MHz_XTAL);
	m_asic2->int_cb().set(m_asic1, FUNC(psion_asic1_device::eint3_w));
	m_asic2->nmi_cb().set(m_asic1, FUNC(psion_asic1_device::enmi_w));
	m_asic2->cbusy_cb().set_inputline(m_maincpu, NEC_INPUT_LINE_POLL);
	m_asic2->buz_cb().set(m_speaker, FUNC(speaker_sound_device::level_w));
	m_asic2->buzvol_cb().set([this](int state) { m_speaker->set_output_gain(ALL_OUTPUTS, state ? 1.0 : 0.25); });
	m_asic2->col_cb().set([this](uint8_t data) { return m_keyboard[data]->read(); });
	m_asic2->read_pd_cb().set(FUNC(psion3_state::port_data_r));
	m_asic2->write_pd_cb().set(FUNC(psion3_state::port_data_w));
	m_asic2->data_r<1>().set(m_ssd[0], FUNC(psion_ssd_device::data_r));      // SSD Pack 1
	m_asic2->data_w<1>().set(m_ssd[0], FUNC(psion_ssd_device::data_w));
	m_asic2->data_r<2>().set(m_ssd[1], FUNC(psion_ssd_device::data_r));      // SSD Pack 2
	m_asic2->data_w<2>().set(m_ssd[1], FUNC(psion_ssd_device::data_w));
	m_asic2->data_r<7>().set(m_sibo, FUNC(psion_sibo_slot_device::data_r));  // SIBO expansion port
	m_asic2->data_w<7>().set(m_sibo, FUNC(psion_sibo_slot_device::data_w));

	RAM(config, m_ram).set_default_size("256K").set_extra_options("128K");
	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00); // Piezo buzzer

	PCD3311(config, m_dtmf, 3'580'000).add_route(ALL_OUTPUTS, "mono", 0.25); // PCD3311CT

	PSION_SSD(config, m_ssd[0]);
	m_ssd[0]->door_cb().set(m_asic2, FUNC(psion_asic2_device::dnmi_w));
	PSION_SSD(config, m_ssd[1]);
	m_ssd[1]->door_cb().set(m_asic2, FUNC(psion_asic2_device::dnmi_w));

	PSION_SIBO_SLOT(config, m_sibo, psion_sibo_devices, nullptr);
	m_sibo->int_cb().set(m_asic2, FUNC(psion_asic2_device::sds_int_w));

	SOFTWARE_LIST(config, "ssd_list").set_original("psion_ssd").set_filter("S3");
}

void psion3_state::psion3s(machine_config &config)
{
	psion3(config);

	m_ram->set_default_size("256K").set_extra_options("");
}


ROM_START(psion3)
	ROM_REGION16_LE(0x80000, "flash", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "177f", "V1.77F/ENG 221091")
	ROMX_LOAD("3504-3002-01_19-11-91v1.77f_eng.bin", 0x00000, 0x20000, CRC(73ba99fa) SHA1(1b3f7b2da9cc2f189e88a9aa01fdb6fad7598925), ROM_BIOS(0))
	ROMX_LOAD("3504-3001-01_19-11-91v1.77f_eng.bin", 0x40000, 0x40000, CRC(e868c250) SHA1(48cce7dd219fb776bffe247c48ba070a89bff121), ROM_BIOS(0))
ROM_END

ROM_START(psion3s)
	ROM_REGION16_LE(0x80000, "flash", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "191f", "V1.91F/ENG 140694")
	ROMX_LOAD("s3_v1.91f_eng.bin", 0x00000, 0x80000, CRC(8b4cfa7a) SHA1(582ffba8ec81960b2db283ef0c280a6d8444414f), ROM_BIOS(0))
ROM_END

ROM_START(pocketbk)
	ROM_REGION16_LE(0x80000, "flash", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "191f", "V1.91F/ACN 270892")
	ROMX_LOAD("pb_v1.91f_acn.bin", 0x00000, 0x80000, CRC(875a804b) SHA1(9db07b3de9bcb9cc0c56c9a6fb35b9653eba68b3), ROM_BIOS(0))

	ROM_REGION(0x80000, "ssd1", 0) // Acorn Spell was only available pre-installed in a Pocket Book
	ROM_LOAD("acspell.bin", 0x00000, 0x80000, CRC(2e55032a) SHA1(560a425a19b3f3d12da9a0e2127f2c67aa829082))
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT   COMPAT  MACHINE   INPUT      CLASS          INIT         COMPANY             FULLNAME           FLAGS
COMP( 1991, psion3,    0,       0,      psion3,   psion3,    psion3_state,  empty_init,  "Psion",            "Series 3",        MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
COMP( 1992, pocketbk,  psion3,  0,      psion3s,  pocketbk,  psion3_state,  empty_init,  "Acorn Computers",  "Pocket Book",     MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
COMP( 1994, psion3s,   psion3,  0,      psion3s,  psion3s,   psion3_state,  empty_init,  "Psion",            "Series 3s",       MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
