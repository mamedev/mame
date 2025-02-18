// license:BSD-3-Clause
// copyright-holders:R. Belmont, Nigel Barnes
/***************************************************************************

    Acorn Communicator

    Driver by R. Belmont

    Main CPU:
      65C816

    Other chips:
      6850 UART
      6522 VIA
      SAA5240 (Teletext)
      MC68B54 (Econet)
      AM7910  (Modem)
      PCD3312 (Tone Generator)
      PCF0335 (Pulse Dialler?)
      PCF8573 (RTC)
      SCN2641 (RS423)

****************************************************************************/

#include "emu.h"
#include "electron_ula.h"

#include "bus/centronics/ctronics.h"
#include "bus/econet/econet.h"
#include "bus/rs232/rs232.h"
#include "cpu/g65816/g65816.h"
#include "machine/6522via.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/input_merger.h"
#include "machine/mc6854.h"
#include "machine/nvram.h"
#include "machine/pcf8573.h"
#include "machine/ram.h"
#include "machine/scn_pci.h"
#include "sound/pcd3311.h"
#include "video/saa5240.h"

#include "screen.h"
#include "speaker.h"

#include "accomm.lh"


namespace {

class accomm_state : public driver_device
{
public:
	accomm_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_maincpu_region(*this, "maincpu"),
		m_irqs(*this, "irqs"),
		m_screen(*this, "screen"),
		m_cct(*this, "saa5240"),
		m_dtmf(*this, "dtmf"),
		m_ram(*this, RAM_TAG),
		m_ula(*this, "ula"),
		m_rtc(*this, "rtc"),
		m_via(*this, "via6522"),
		m_acia(*this, "acia"),
		m_acia_clock(*this, "acia_clock"),
		m_scn2641(*this, "aci"),
		m_adlc(*this, "mc6854"),
		m_vram(*this, "vram", 0x8000, ENDIANNESS_LITTLE),
		m_keybd{ { *this, "LINE1.%u", 0 }, { *this, "LINE2.%u", 0 } },
		m_shiftlock_led(*this, "shiftlock_led"),
		m_capslock_led(*this, "capslock_led"),
		m_ch00rom_enabled(true),
		m_ttx_enabled(false)
	{ }

	void accomm(machine_config &config);
	void accommi(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(trigger_reset);

protected:
	// driver_device overrides
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<g65816_device> m_maincpu;
	required_memory_region m_maincpu_region;
	required_device<input_merger_device> m_irqs;
	required_device<screen_device> m_screen;
	required_device<saa5240_device> m_cct;
	required_device<pcd3311_device> m_dtmf;
	required_device<ram_device> m_ram;
	required_device<electron_ula_device> m_ula;
	required_device<pcf8573_device> m_rtc;
	required_device<via6522_device> m_via;
	required_device<acia6850_device> m_acia;
	required_device<clock_device> m_acia_clock;
	required_device<scn2641_device> m_scn2641;
	required_device<mc6854_device> m_adlc;
	memory_share_creator<uint8_t> m_vram;
	required_ioport_array<13> m_keybd[2];
	output_finder<> m_shiftlock_led;
	output_finder<> m_capslock_led;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void set_cpu_clock(offs_t offset);
	void ch00switch();
	uint8_t kbd_r(offs_t offset);
	uint8_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint8_t data);
	uint8_t via_pb_r();
	void via_pb_w(uint8_t data);

	void mem_map(address_map &map) ATTR_COLD;

	bool m_ch00rom_enabled;
	bool m_ttx_enabled;
};


uint8_t accomm_state::kbd_r(offs_t offset)
{
	uint8_t data = 0;

	for (int i = 0; i < 13; i++)
	{
		if (!BIT(offset, i))
			data |= m_keybd[BIT(offset, 13)][i]->read();
	}

	return data;
}

void accomm_state::machine_reset()
{
	m_ch00rom_enabled = true;
}

void accomm_state::machine_start()
{
	m_shiftlock_led.resolve();
	m_capslock_led.resolve();

	m_ula->set_ram(m_vram);

	m_maincpu->space(AS_PROGRAM).install_readwrite_tap(0x000000, 0xffffff, "cpu_clock_tap",
		[this](offs_t offset, uint8_t &data, uint8_t mem_mask) { if (!machine().side_effects_disabled()) set_cpu_clock(offset); },
		[this](offs_t offset, uint8_t &data, uint8_t mem_mask) { if (!machine().side_effects_disabled()) set_cpu_clock(offset); });

	save_item(NAME(m_ch00rom_enabled));
	save_item(NAME(m_ttx_enabled));
}

void accomm_state::set_cpu_clock(offs_t offset)
{
	switch (offset & 0xc80000)
	{
	case 0x400000:
		// TODO: this is not verified but the ULA RAM can only be accessed at 1MHz
		m_maincpu->set_clock_scale(0.5);
		break;

	default:
		m_maincpu->set_clock_scale(1.0);
		break;
	}
}

void accomm_state::ch00switch()
{
	if (!machine().side_effects_disabled())
		m_ch00rom_enabled = false;
}


uint32_t accomm_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_ttx_enabled)
		m_cct->screen_update(screen, bitmap, cliprect);
	else
		m_ula->screen_update(screen, bitmap, cliprect);

	return 0;
}


uint8_t accomm_state::ram_r(offs_t offset)
{
	if (m_ch00rom_enabled && (offset < 0x10000))
		return m_maincpu_region->base()[offset];
	else
		return m_ram->pointer()[offset & m_ram->mask()];
}

void accomm_state::ram_w(offs_t offset, uint8_t data)
{
	m_ram->pointer()[offset & m_ram->mask()] = data;
}


uint8_t accomm_state::via_pb_r()
{
	return 0xfe | (m_rtc->sda_r() & m_cct->read_sda());
}

void accomm_state::via_pb_w(uint8_t data)
{
	data ^= 0xff;

	m_rtc->sda_w(BIT(data, 1));
	m_rtc->scl_w(BIT(data, 2));

	m_cct->write_sda(BIT(data, 1));
	m_cct->write_scl(BIT(data, 2));

	if (BIT(data, 6) != m_ttx_enabled)
	{
		m_ttx_enabled = BIT(data, 6);
		if (m_ttx_enabled)
			m_screen->set_visible_area(0, 480-1, 0, 250-1);
		else
			m_screen->set_visible_area(0, 640-1, 0, 256-1);
	}
}


void accomm_state::mem_map(address_map &map)
{
	map(0x000000, 0x1fffff).rw(FUNC(accomm_state::ram_r), FUNC(accomm_state::ram_w));               /* System RAM */
	map(0x200000, 0x3fffff).noprw();                                                                /* External expansion RAM */
	map(0x400000, 0x400001).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));  /* MODEM */
	map(0x410000, 0x410000).mirror(0xffff).ram();                                                                  /* Econet ID */
	map(0x420000, 0x42000f).mirror(0xfff0).m(m_via, FUNC(via6522_device::map));                                    /* 6522 VIA (printer etc) */
	map(0x430000, 0x430003).mirror(0xfffc).rw(m_scn2641, FUNC(scn2641_device::read), FUNC(scn2641_device::write)); /* 2641 ACIA (RS423) */
	map(0x440000, 0x440000).mirror(0xffff).lr8(NAME([this]() { ch00switch(); return 0x44; }));                     /* CH00SWITCH */
	map(0x440000, 0x440000).mirror(0xffff).lw8(NAME([this](u8 data) { ch00switch(); }));                           /* CH00SWITCH */
	map(0x450000, 0x45ffff).rw(m_ula, FUNC(electron_ula_device::read), FUNC(electron_ula_device::write));          /* Video ULA */
	map(0x460000, 0x467fff).mirror(0x8000).ram().share("nvram");                                                   /* CMOS RAM */
	map(0x470000, 0x470003).mirror(0xfffc).rw(m_adlc, FUNC(mc6854_device::read), FUNC(mc6854_device::write));      /* 68B54 (Econet) */
	map(0x480000, 0x7fffff).noprw();                                                                /* Reserved */
	map(0x800000, 0xbfffff).noprw();                                                                /* External expansion IO   */
	map(0xc00000, 0xefffff).rom().region("ext", 0);                                                 /* External expansion ROM  */
	map(0xf00000, 0xf0ffff).rom().region("maincpu", 0x000000);                                      /* ROM bank 0 (ROM Slot 0) */
	map(0xf10000, 0xf1ffff).rom().region("maincpu", 0x010000);                                      /* ROM bank 1 (ROM Slot 0) */
	map(0xf80000, 0xf9ffff).rom().region("maincpu", 0x060000);                                      /* Empty      (ROM Slot 3) */
	map(0xfa0000, 0xfbffff).rom().region("maincpu", 0x040000);                                      /* ROM bank 4 (ROM Slot 2) */
	map(0xfc0000, 0xfcffff).rom().region("maincpu", 0x030000);                                      /* ROM bank 3 (ROM Slot 1) */
	map(0xfd0000, 0xfdffff).rom().region("maincpu", 0x020000);                                      /* ROM bank 2 (ROM Slot 1) */
	map(0xfe0000, 0xfeffff).rom().region("maincpu", 0x000000);                                      /* ROM bank 0 (ROM Slot 0) */
	map(0xff0000, 0xffffff).rom().region("maincpu", 0x010000);                                      /* ROM bank 1 (ROM Slot 0) */
}

INPUT_CHANGED_MEMBER(accomm_state::trigger_reset)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}

static INPUT_PORTS_START( accomm )
	PORT_START("LINE1.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('_') PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')

	PORT_START("LINE1.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')

	PORT_START("LINE1.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')

	PORT_START("LINE1.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR(UCHAR_MAMEKEY(TILDE))     PORT_NAME("Function")

	PORT_START("LINE1.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("LINE1.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')

	PORT_START("LINE1.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)       PORT_CHAR(UCHAR_MAMEKEY(LALT))      PORT_NAME("Shift Lock")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_CHAR(9)                        PORT_NAME("Tab")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("LINE1.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("LINE1.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(EQUALS_PAD))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))

	PORT_START("LINE1.9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE)) PORT_NAME("Del CE")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)        PORT_CHAR(UCHAR_MAMEKEY(END))       PORT_NAME("Copy EE")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(10)                       PORT_NAME(u8"\u2193  +") // U+2193 = ↓
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))      PORT_NAME("Home  %")

	PORT_START("LINE1.10")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))   PORT_NAME("Keypad .")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))

	PORT_START("LINE1.11")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')  PORT_CHAR('}')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))        PORT_NAME("Phone")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(UCHAR_MAMEKEY(ESC))       PORT_NAME("Escape")

	PORT_START("LINE1.12")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("LINE2.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')

	PORT_START("LINE2.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR(0xa3)   // £
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')

	PORT_START("LINE2.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')

	PORT_START("LINE2.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_NAME("Help")

	PORT_START("LINE2.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_NAME("Ctrl")

	PORT_START("LINE2.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')

	PORT_START("LINE2.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("LINE2.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')

	PORT_START("LINE2.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE2.9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))    PORT_NAME(u8"\u2192  -") // U+2192 = →
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(UCHAR_MAMEKEY(INSERT))   PORT_NAME(u8"Insert  ÷")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))       PORT_NAME(u8"\u2191  ×") // U+2191 = ↑
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))     PORT_NAME(u8"\u2190  AC") // U+2190 = ←

	PORT_START("LINE2.10")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("LINE2.11")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))      PORT_NAME("Comp")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)        PORT_CHAR(UCHAR_MAMEKEY(F11))      PORT_NAME("Calc")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)

	PORT_START("LINE2.12")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("STOP")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12)        PORT_CHAR(UCHAR_MAMEKEY(F12))      PORT_NAME("Stop") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(accomm_state::trigger_reset), 0)
INPUT_PORTS_END


void accomm_state::accomm(machine_config &config)
{
	G65816(config, m_maincpu, 16_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &accomm_state::mem_map);

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, G65816_LINE_IRQ);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(16_MHz_XTAL, 1024, 0, 640, 312, 0, 256);
	m_screen->set_screen_update(FUNC(accomm_state::screen_update));
	m_screen->set_video_attributes(VIDEO_UPDATE_SCANLINE);

	config.set_default_layout(layout_accomm);

	RAM(config, RAM_TAG).set_default_size("512K").set_extra_options("1M");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SPEAKER(config, "mono").front_center();

	ELECTRON_ULA(config, m_ula, 16_MHz_XTAL);
	m_ula->set_cpu_tag("maincpu");
	m_ula->kbd_cb().set(FUNC(accomm_state::kbd_r));
	m_ula->caps_lock_cb().set([this](int state) { m_capslock_led = state; });
	m_ula->cas_mo_cb().set([this](int state) { m_shiftlock_led = !state; });
	m_ula->add_route(ALL_OUTPUTS, "mono", 0.5);
	m_ula->irq_cb().set_inputline(m_maincpu, 0);

	PCF8573(config, m_rtc, 32.768_kHz_XTAL);
	//m_rtc->comp_cb().set(m_via, FUNC(via6522_device::write_cb1));

	SAA5240A(config, m_cct, 6_MHz_XTAL);
	m_cct->set_ram_size(0x800);

	MOS6522(config, m_via, 16_MHz_XTAL / 16);
	m_via->writepa_handler().set("cent_data_out", FUNC(output_latch_device::write));
	m_via->ca2_handler().set("centronics", FUNC(centronics_device::write_strobe));
	m_via->readpb_handler().set(FUNC(accomm_state::via_pb_r));
	m_via->writepb_handler().set(FUNC(accomm_state::via_pb_w));
	m_via->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	SCN2641(config, m_scn2641, 3.6864_MHz_XTAL);
	m_scn2641->txd_handler().set("rs423", FUNC(rs232_port_device::write_txd));
	m_scn2641->rts_handler().set("rs423", FUNC(rs232_port_device::write_rts));
	m_scn2641->intr_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));

	rs232_port_device &rs423(RS232_PORT(config, "rs423", default_rs232_devices, nullptr));
	rs423.rxd_handler().set(m_scn2641, FUNC(scn2641_device::rxd_w));
	rs423.dcd_handler().set(m_scn2641, FUNC(scn2641_device::dcd_w));
	rs423.cts_handler().set(m_scn2641, FUNC(scn2641_device::cts_w));

	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("modem", FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set("modem", FUNC(rs232_port_device::write_rts));
	m_acia->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<2>));

	rs232_port_device &modem(RS232_PORT(config, "modem", default_rs232_devices, "null_modem")); // AM7910 (internal modem)
	modem.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	modem.dcd_handler().set(m_acia, FUNC(acia6850_device::write_dcd));
	modem.cts_handler().set(m_acia, FUNC(acia6850_device::write_cts));

	CLOCK(config, m_acia_clock, 16_MHz_XTAL / 13);
	m_acia_clock->signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
	m_acia_clock->signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));

	PCD3311(config, m_dtmf, 3.57864_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.25); // PCD3312

	MC6854(config, m_adlc);
	m_adlc->out_txd_cb().set("econet", FUNC(econet_device::host_data_w));
	m_adlc->out_irq_cb().set_inputline(m_maincpu, G65816_LINE_NMI);

	econet_device &econet(ECONET(config, "econet", 0));
	econet.clk_wr_callback().set(m_adlc, FUNC(mc6854_device::txc_w));
	econet.clk_wr_callback().append(m_adlc, FUNC(mc6854_device::rxc_w));
	econet.data_wr_callback().set(m_adlc, FUNC(mc6854_device::set_rx));

	ECONET_SLOT(config, "econet254", "econet", econet_devices);

	centronics_device &centronics(CENTRONICS(config, "centronics", centronics_devices, "printer"));
	centronics.ack_handler().set(m_via, FUNC(via6522_device::write_ca1));
	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	centronics.set_output_latch(cent_data_out);
}


void accomm_state::accommi(machine_config &config)
{
	accomm(config);

	SAA5240B(config.replace(), m_cct, 6_MHz_XTAL);
	m_cct->set_ram_size(0x800);
}


ROM_START(accomm)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_DEFAULT_BIOS("100")
	ROM_SYSTEM_BIOS(0, "100", "MOS v1.00 13/Nov/86") /* MOS: Version 1.00 13/Nov/86 (C)1986 */
	ROMX_LOAD("romv100-0.rom", 0x000000, 0x010000, CRC(6d22950d) SHA1(d4cbdccf8d2bc836fb81182b2ed344d7134fe5c9), ROM_BIOS(0))
	ROM_RELOAD(                0x010000, 0x010000)
	ROMX_LOAD("romv100-1.rom", 0x020000, 0x010000, CRC(adc6a073) SHA1(3e87f21fafc1d69f33c5b541a20a98e82aacbfab), ROM_BIOS(0))
	ROM_RELOAD(                0x030000, 0x010000)
	ROMX_LOAD("romv100-2.rom", 0x040000, 0x010000, CRC(3438adee) SHA1(cd9d5522d9430cb2e1936210b77d2edd280f9419), ROM_BIOS(0))
	ROM_RELOAD(                0x050000, 0x010000)
	ROMX_LOAD("romv100-3.rom", 0x060000, 0x010000, CRC(bd87a157) SHA1(b9b9ed1aab9ffef2de988b2cfeac293afa11448a), ROM_BIOS(0))
	ROM_RELOAD(                0x070000, 0x010000)

	ROM_REGION(0x300000, "ext", ROMREGION_ERASEFF)
ROM_END

ROM_START(accommp)
	ROM_REGION(0x80000, "maincpu", 0)
	/* ROM labels on both evaluation prototypes were hand written A, B, C, D */
	ROM_DEFAULT_BIOS("011-1985")
	/* Serial B01-PPC01-0000004 (owned by Acorn co-founder Chris Curry) */
	ROM_SYSTEM_BIOS(0, "011-1985", "CMOS v0.11 1985") /* CMOS version 0.11 October (C)1985 */
	ROMX_LOAD("004-a.rom", 0x000000, 0x008000, CRC(d0d4d5e3) SHA1(67710e349235ed5c71380b5a7d4b570ce355b10e), ROM_BIOS(0))
	ROM_RELOAD(            0x008000, 0x008000)
	ROM_RELOAD(            0x010000, 0x008000)
	ROM_RELOAD(            0x018000, 0x008000)
	ROMX_LOAD("004-b.rom", 0x020000, 0x010000, CRC(e2fcef94) SHA1(fd065bcdb6c48bee39db9f71b8d193ee228557f7), ROM_BIOS(0))
	ROM_RELOAD(            0x030000, 0x010000)
	ROMX_LOAD("004-c.rom", 0x040000, 0x008000, CRC(348c0018) SHA1(9681b6b9eefa9ba294fac6a41dec12ba203e5142), ROM_BIOS(0))
	ROM_RELOAD(            0x048000, 0x008000)
	ROM_RELOAD(            0x050000, 0x008000)
	ROM_RELOAD(            0x058000, 0x008000)
	ROMX_LOAD("004-d.rom", 0x060000, 0x010000, CRC(1379eb9f) SHA1(8d57bc7e279c5f17c6f0e4d1d5fa7f784aadd549), ROM_BIOS(0))
	ROM_RELOAD(            0x070000, 0x010000)
	/* Serial ending 094 */
	ROM_SYSTEM_BIOS(1, "011-1986", "CMOS v0.11 1986") /* CMOS version 0.11 October (C)1985 */
	ROMX_LOAD("094-a.rom", 0x008000, 0x008000, CRC(d0d4d5e3) SHA1(67710e349235ed5c71380b5a7d4b570ce355b10e), ROM_BIOS(1))
	ROM_RELOAD(            0x008000, 0x008000)
	ROM_RELOAD(            0x010000, 0x008000)
	ROM_RELOAD(            0x018000, 0x008000)
	ROMX_LOAD("094-b.rom", 0x020000, 0x008000, CRC(8d793909) SHA1(392028386f831dfae3353e0b7b51a608798e89c6), ROM_BIOS(1))
	ROM_RELOAD(            0x028000, 0x008000)
	ROM_RELOAD(            0x030000, 0x008000)
	ROM_RELOAD(            0x038000, 0x008000)
	ROMX_LOAD("094-c.rom", 0x040000, 0x008000, CRC(e544e849) SHA1(31cd2dcd2a50880a97b12d61ef144f7d7f112345), ROM_BIOS(1))
	ROM_RELOAD(            0x048000, 0x008000)
	ROM_RELOAD(            0x050000, 0x008000)
	ROM_RELOAD(            0x058000, 0x008000)
	ROMX_LOAD("094-d-view+castoff.rom", 0x060000, 0x010000, CRC(8027df77) SHA1(51751bfdcf68683c092b6442fb22f11cb565898c), ROM_BIOS(1))
	ROM_RELOAD(            0x070000, 0x010000)

	ROM_REGION(0x300000, "ext", ROMREGION_ERASEFF)
ROM_END

ROM_START(accommb)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_DEFAULT_BIOS("170")
	ROM_SYSTEM_BIOS(0, "170", "MOS v1.70 04/Jun/87") /* MOS: Version 1.70 04/Jun/87 (C)1987 */
	ROMX_LOAD("0252.200-1-rom0-v1.00.rom", 0x000000, 0x010000, CRC(6d22950d) SHA1(d4cbdccf8d2bc836fb81182b2ed344d7134fe5c9), ROM_BIOS(0))
	ROM_RELOAD(                            0x010000, 0x010000)
	ROMX_LOAD("0252.201-1-rom1-v1.00.rom", 0x020000, 0x010000, CRC(adc6a073) SHA1(3e87f21fafc1d69f33c5b541a20a98e82aacbfab), ROM_BIOS(0))
	ROM_RELOAD(                            0x030000, 0x010000)
	ROMX_LOAD("0252.202-1-rom2-v1.00.rom", 0x040000, 0x010000, CRC(3438adee) SHA1(cd9d5522d9430cb2e1936210b77d2edd280f9419), ROM_BIOS(0))
	ROM_RELOAD(                            0x050000, 0x010000)
	ROMX_LOAD("0252.203-1-rom3-v1.00.rom", 0x060000, 0x010000, CRC(bd87a157) SHA1(b9b9ed1aab9ffef2de988b2cfeac293afa11448a), ROM_BIOS(0))
	ROM_RELOAD(                            0x070000, 0x010000)

	/* Expansion board: Acorn Computer 0167,000 Issue 1 Spectar II */
	/* Contains 8 slots for ASTRON Data Cards */
	ROM_REGION(0x300000, "ext", ROMREGION_ERASEFF)
	ROM_LOAD("spectar-v1.0-0267-200-03.ic1", 0x000000, 0x010000, CRC(71ad0491) SHA1(c3ace8cdd2383e97eb58d64d011444da678d537c))
ROM_END

ROM_START(accommi)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_DEFAULT_BIOS("300")
	ROM_SYSTEM_BIOS(0, "300", "MOS v3.00 13/gen/88") /* MOS: Versione 3.00 13/gen/88 (C)1988 */
	ROMX_LOAD("rom0.rom",        0x000000, 0x020000, CRC(841bd984) SHA1(2c3bc77178e5bf0342e0410f6c398bb3ac40d0c4), ROM_BIOS(0))
	ROMX_LOAD("252216-iss1.rom", 0x020000, 0x020000, CRC(40767d31) SHA1(258f4ed92d74523aaaa4aa250db5a99428aaf960), ROM_BIOS(0))
	ROMX_LOAD("rom2.rom",        0x040000, 0x010000, CRC(e3511af8) SHA1(88a5654a5e84a31078a0a64139fe84db08196c2a), ROM_BIOS(0))
	ROM_RELOAD(                  0x050000, 0x010000)

	ROM_REGION(0x300000, "ext", ROMREGION_ERASEFF)
ROM_END

} // anonymous namespace


/*    YEAR  NAME     PARENT  COMPAT MACHINE  INPUT   CLASS         INIT        COMPANY            FULLNAME                          FLAGS */
COMP( 1986, accomm,  0,      0,     accomm,  accomm, accomm_state, empty_init, "Acorn Computers", "Acorn Communicator",             MACHINE_NOT_WORKING )
COMP( 1985, accommp, accomm, 0,     accomm,  accomm, accomm_state, empty_init, "Acorn Computers", "Acorn Communicator (prototype)", MACHINE_NOT_WORKING )
COMP( 1987, accommb, accomm, 0,     accomm,  accomm, accomm_state, empty_init, "Acorn Computers", "Acorn Briefcase Communicator",   MACHINE_NOT_WORKING )
COMP( 1988, accommi, accomm, 0,     accommi, accomm, accomm_state, empty_init, "Acorn Computers", "Acorn Communicator (Italian)",   MACHINE_NOT_WORKING )
