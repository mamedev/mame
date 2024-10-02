// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    TI-89, TI-92, TI-92 plus, Voyage 200 PLT and TI-89 Titanium

    Driver by Sandro Ronco

    Note:
     -NVRAM works only if the calculator is turned off (2nd + ON) before closing MESS

    TODO:
     -Link
     -HW 3 I/O port
     -RTC
     -LCD contrast

****************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/intelfsh.h"
#include "machine/nvram.h"
#include "machine/timer.h"

#include "emupal.h"
#include "screen.h"


namespace {

class ti68k_state : public driver_device
{
public:
	ti68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_flash(*this, "flash")
		, m_rom_base(*this, "flash")
		, m_ram_base(*this, "nvram")
		, m_inputs(*this, "IN.%u", 0)
	{ }

	void v200(machine_config &config);
	void ti92(machine_config &config);
	void ti89(machine_config &config);
	void ti92p(machine_config &config);
	void ti89t(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(ti68k_on_key);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// hardware versions
	enum { HW1=1, HW2, HW3, HW4 };

	required_device<cpu_device> m_maincpu;
	optional_device<intelfsh16_device> m_flash;
	required_region_ptr<uint16_t> m_rom_base;
	required_shared_ptr<uint16_t> m_ram_base;
	required_ioport_array<10> m_inputs;

	// HW specifications
	uint8_t m_hw_version = 0;
	bool m_ram_enabled = false;

	// keyboard
	uint16_t m_kb_mask = 0;
	uint8_t m_on_key = 0;

	// LCD
	uint8_t m_lcd_on = 0;
	uint32_t m_lcd_base = 0;
	uint16_t m_lcd_width = 0;
	uint16_t m_lcd_height = 0;
	uint16_t m_lcd_contrast = 0;

	// I/O
	uint16_t m_io_hw1[0x10]{};
	uint16_t m_io_hw2[0x80]{};

	// Timer
	uint8_t m_timer_on = 0;
	uint8_t m_timer_val = 0;
	uint16_t m_timer_mask = 0;
	uint64_t m_timer = 0;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t keypad_r();
	void ti68k_io_w(offs_t offset, uint16_t data);
	uint16_t ti68k_io_r(offs_t offset);
	void ti68k_io2_w(offs_t offset, uint16_t data);
	uint16_t ti68k_io2_r(offs_t offset);
	uint16_t rom_r(offs_t offset);
	uint16_t reset_overlay_r(offs_t offset);
	void ti68k_palette(palette_device &palette) const;

	TIMER_DEVICE_CALLBACK_MEMBER(ti68k_timer_callback);

	void ti89_mem(address_map &map) ATTR_COLD;
	void ti89t_mem(address_map &map) ATTR_COLD;
	void ti92_mem(address_map &map) ATTR_COLD;
	void ti92p_mem(address_map &map) ATTR_COLD;
	void v200_mem(address_map &map) ATTR_COLD;
};


uint8_t ti68k_state::keypad_r()
{
	uint8_t data = 0xff;

	for (int i = 0; i < 10; i++)
		if (!BIT(m_kb_mask, i))
			data &= m_inputs[i]->read();

	return data;
}


void ti68k_state::ti68k_io_w(offs_t offset, uint16_t data)
{
	switch(offset & 0x0f)
	{
		case 0x00:
			m_lcd_contrast = (m_lcd_contrast & 0xfe) | BIT(data, 5);
			break;
		case 0x02:
			if (!(data & 0x10) && data != m_io_hw1[offset])
				m_maincpu->suspend(SUSPEND_REASON_DISABLE, 1);
			break;
		case 0x08:
				m_lcd_base = data << 3;
			break;
		case 0x09:
				m_lcd_width = (0x40 - ((data >> 8) & 0xff)) * 0x10;
				m_lcd_height = 0x100 - (data & 0xff);
			break;
		case 0x0a:
			m_timer_on = BIT(data ,1);
			switch((data >> 4) & 0x03)
			{
				case 0: m_timer_mask = 0x0000; break;
				case 1: m_timer_mask = 0x000f; break;
				case 2: m_timer_mask = 0x007f; break;
				case 3: m_timer_mask = 0x1fff; break;
			}
			break;
		case 0x0b:
			m_timer_val = data & 0xff;
			break;
		case 0x0c:
			m_kb_mask = data & 0x03ff;
			break;
		case 0x0e:
			m_lcd_on = (((data >> 8) & 0x3c) == 0x3c) ? 0 : 1;
			m_lcd_contrast = (m_lcd_contrast & 0x01) | ((data & 0x0f) << 1);
			break;
		default: break;
	}
	m_io_hw1[offset & 0x0f] = data;
}


uint16_t ti68k_state::ti68k_io_r(offs_t offset)
{
	uint16_t data;

	switch (offset & 0x0f)
	{
		case 0x00:
			data = 0x0400 | ((m_lcd_contrast & 1) << 13);
			break;
		case 0x0b:
			data = m_timer_val;
			break;
		case 0x0d:
			data = ((!m_on_key) << 9) | keypad_r();
			break;
		default:
			data= m_io_hw1[offset & 0x0f];
	}
	return data;
}


void ti68k_state::ti68k_io2_w(offs_t offset, uint16_t data)
{
	switch(offset & 0x7f)
	{
		case 0x0b:
			m_lcd_base = 0x4c00 + 0x1000 * (data & 0x03);
			break;
		case 0x0e:
			m_lcd_on = (data & 0x02) ? 1 : 0;
			break;
		default: break;
	}
	m_io_hw2[offset & 0x7f] = data;
}


uint16_t ti68k_state::ti68k_io2_r(offs_t offset)
{
	uint16_t data;

	switch (offset & 0x7f)
	{
		default:
			data= m_io_hw2[offset & 0x7f];
	}
	return data;
}


uint16_t ti68k_state::rom_r(offs_t offset)
{
	return m_rom_base[offset];
}


uint16_t ti68k_state::reset_overlay_r(offs_t offset)
{
	if (m_ram_enabled)
		return m_ram_base[offset];
	else
	{
		// FIXME: probably triggered by something else
		if (offset == 3 && !machine().side_effects_disabled())
			m_ram_enabled = true;

		// FIXME: this reset vector happens to work for all Flash systems, but may not be loaded
		// first. It is algorithmically located by the preceding initialization code which looks
		// for the string of four CC bytes preceding it. This code has not been enabled since it
		// also contains a "Certificate Memory" self-test which fails.
		if (m_flash.found())
			return m_flash->read(offset + 0x12088/2);
		else
			return m_rom_base[offset];
	}
}


TIMER_DEVICE_CALLBACK_MEMBER(ti68k_state::ti68k_timer_callback)
{
	m_timer++;

	if (m_timer_on)
	{
		if (!(m_timer & m_timer_mask) && BIT(m_io_hw1[0x0a], 3))
		{
			if (m_timer_val)
				m_timer_val++;
			else
				m_timer_val = (m_io_hw1[0x0b]) & 0xff;
		}

		if (!BIT(m_io_hw1[0x0a], 7) && ((m_hw_version == HW1) || (!BIT(m_io_hw1[0x0f], 2) && !BIT(m_io_hw1[0x0f], 1))))
		{
			if (!(m_timer & 0x003f))
				m_maincpu->set_input_line(M68K_IRQ_1, HOLD_LINE);

			if (!(m_timer & 0x3fff) && !BIT(m_io_hw1[0x0a], 3))
				m_maincpu->set_input_line(M68K_IRQ_3, HOLD_LINE);

			if (!(m_timer & m_timer_mask) && BIT(m_io_hw1[0x0a], 3) && m_timer_val == 0)
				m_maincpu->set_input_line(M68K_IRQ_5, HOLD_LINE);
		}
	}

	if (keypad_r() != 0xff)
		m_maincpu->set_input_line(M68K_IRQ_2, HOLD_LINE);
}


void ti68k_state::ti92_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x0fffff).ram().share("nvram");
	map(0x000000, 0x000007).r(FUNC(ti68k_state::reset_overlay_r));
	map(0x200000, 0x5fffff).unmaprw();   // ROM
	map(0x600000, 0x6fffff).rw(FUNC(ti68k_state::ti68k_io_r), FUNC(ti68k_state::ti68k_io_w));
}


void ti68k_state::ti89_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x0fffff).ram().share("nvram");
	map(0x000000, 0x000007).r(FUNC(ti68k_state::reset_overlay_r));
	map(0x200000, 0x3fffff).rw(m_flash, FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));
	map(0x400000, 0x5fffff).noprw();
	map(0x600000, 0x6fffff).rw(FUNC(ti68k_state::ti68k_io_r), FUNC(ti68k_state::ti68k_io_w));
	map(0x700000, 0x7fffff).rw(FUNC(ti68k_state::ti68k_io2_r), FUNC(ti68k_state::ti68k_io2_w));
}


void ti68k_state::ti92p_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x0fffff).ram().share("nvram");
	map(0x000000, 0x000007).r(FUNC(ti68k_state::reset_overlay_r));
	map(0x200000, 0x3fffff).noprw();
	map(0x400000, 0x5fffff).rw(m_flash, FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));
	map(0x600000, 0x6fffff).rw(FUNC(ti68k_state::ti68k_io_r), FUNC(ti68k_state::ti68k_io_w));
	map(0x700000, 0x7fffff).rw(FUNC(ti68k_state::ti68k_io2_r), FUNC(ti68k_state::ti68k_io2_w));
}


void ti68k_state::v200_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x0fffff).ram().share("nvram");
	map(0x000000, 0x000007).r(FUNC(ti68k_state::reset_overlay_r));
	map(0x200000, 0x5fffff).rw(m_flash, FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));
	map(0x600000, 0x6fffff).rw(FUNC(ti68k_state::ti68k_io_r), FUNC(ti68k_state::ti68k_io_w));
	map(0x700000, 0x70ffff).rw(FUNC(ti68k_state::ti68k_io2_r), FUNC(ti68k_state::ti68k_io2_w));
}


void ti68k_state::ti89t_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x0fffff).ram().mirror(0x200000).share("nvram");
	map(0x000000, 0x000007).r(FUNC(ti68k_state::reset_overlay_r));
	map(0x600000, 0x6fffff).rw(FUNC(ti68k_state::ti68k_io_r), FUNC(ti68k_state::ti68k_io_w));
	map(0x700000, 0x70ffff).rw(FUNC(ti68k_state::ti68k_io2_r), FUNC(ti68k_state::ti68k_io2_w));
	map(0x800000, 0xbfffff).rw(m_flash, FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));
	map(0xc00000, 0xffffff).noprw();
}


INPUT_CHANGED_MEMBER(ti68k_state::ti68k_on_key)
{
	m_on_key = newval;

	if (m_on_key)
	{
		if (m_maincpu->suspended(SUSPEND_REASON_DISABLE))
			m_maincpu->resume(SUSPEND_REASON_DISABLE);

		m_maincpu->set_input_line(M68K_IRQ_6, HOLD_LINE);
	}
}


/* Input ports for TI-89 and TI-89 Titanium*/
static INPUT_PORTS_START (ti8x)
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2nd") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3nd") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ALPHA") PORT_CODE(KEYCODE_CAPSLOCK)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER  (ENTRY)") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CLEAR") PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("(-)") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("<--") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(")") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CATALOG") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("(") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Mode") PORT_CODE(KEYCODE_F12)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("APPS") PORT_CODE(KEYCODE_F11)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STORE") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EE") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("|") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("=") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ON") PORT_CODE(KEYCODE_F10) PORT_CHANGED_MEMBER(DEVICE_SELF, ti68k_state, ti68k_on_key, 0)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN.7")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN.8")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN.9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

/* Input ports for TI-92 and TI92 Plus*/
static INPUT_PORTS_START (ti9x)
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2nd") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3nd") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Clock") PORT_CODE(KEYCODE_F11)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F8") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STORE") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F7") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("(") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(")") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SIN") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COS") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TAN") PORT_CODE(KEYCODE_INSERT)

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LN") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("=") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CLEAR") PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("APPS") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_ASTERISK)

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("<--") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("o") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MODE") PORT_CODE(KEYCODE_F12)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ON") PORT_CODE(KEYCODE_F10) PORT_CHANGED_MEMBER(DEVICE_SELF, ti68k_state, ti68k_on_key, 0)

	PORT_START("IN.9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("(-)") PORT_CODE(KEYCODE_MINUS_PAD)
INPUT_PORTS_END


void ti68k_state::machine_start()
{
	if (m_flash.found())
	{
		uint32_t base = ((((m_rom_base[0x82]) << 16) | m_rom_base[0x83]) & 0xffff)>>1;

		if (m_rom_base[base] >= 8)
			m_hw_version = ((m_rom_base[base + 0x0b]) << 16) | m_rom_base[base + 0x0c];

		if (!m_hw_version)
			m_hw_version = HW1;
	}
	else
	{
		m_hw_version = HW1;
		uint32_t initial_pc = ((m_rom_base[2]) << 16) | m_rom_base[3];

		m_maincpu->space(AS_PROGRAM).unmap_read(0x200000, 0x5fffff);

		if (initial_pc > 0x400000)
		{
			m_maincpu->space(AS_PROGRAM).install_read_handler(0x400000, 0x5fffff, read16sm_delegate(*this, FUNC(ti68k_state::rom_r)));
		}
		else
		{
			m_maincpu->space(AS_PROGRAM).install_read_handler(0x200000, 0x3fffff, read16sm_delegate(*this, FUNC(ti68k_state::rom_r)));
		}
	}

	logerror("HW=v%x, Type=%s\n", m_hw_version, m_flash.found() ? "Flash" : "ROM");
}


void ti68k_state::machine_reset()
{
	m_ram_enabled = false;

	m_kb_mask = 0xff;
	m_on_key = 0;
	m_lcd_base = 0;
	m_lcd_width = 0;
	m_lcd_height = 0;
	m_lcd_on = 0;
	m_lcd_contrast = 0;
	std::fill(std::begin(m_io_hw1), std::end(m_io_hw1), 0);
	std::fill(std::begin(m_io_hw2), std::end(m_io_hw2), 0);
	m_timer_on = 0;
	m_timer_mask = 0xf;
	m_timer_val = 0;
}

/* video */
uint32_t ti68k_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* preliminary implementation, doesn't use the contrast value */
	uint8_t const width = screen.width();
	uint8_t const height = screen.height();

	if (!m_lcd_on || !m_lcd_base)
		bitmap.fill(0);
	else
		for (uint8_t y = 0; y < height; y++)
			for (uint8_t x = 0; x < width / 8; x++)
			{
				uint8_t s_byte= m_maincpu->space(AS_PROGRAM).read_byte(m_lcd_base + y * (width/8) + x);
				for (uint8_t b = 0; b<8; b++)
					bitmap.pix(y, x * 8 + (7 - b)) = BIT(s_byte, b);
			}

	return 0;
}

void ti68k_state::ti68k_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

void ti68k_state::ti89(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(10'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &ti68k_state::ti89_mem);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(ti68k_state::screen_update));
	screen.set_size(240, 128);
	screen.set_visarea(0, 160-1, 0, 100-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(ti68k_state::ti68k_palette), 2);

	SHARP_LH28F160S3(config, m_flash);

	TIMER(config, "ti68k_timer").configure_periodic(FUNC(ti68k_state::ti68k_timer_callback), attotime::from_hz(1<<14));
}


void ti68k_state::ti92(machine_config &config)
{
	ti89(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &ti68k_state::ti92_mem);

	config.device_remove("flash");

	/* video hardware */
	subdevice<screen_device>("screen")->set_visarea(0, 240-1, 0, 128-1);
}


void ti68k_state::ti92p(machine_config &config)
{
	ti89(config);
	m_maincpu->set_clock(XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &ti68k_state::ti92p_mem);

	/* video hardware */
	subdevice<screen_device>("screen")->set_visarea(0, 240-1, 0, 128-1);
}


void ti68k_state::v200(machine_config &config)
{
	ti89(config);
	m_maincpu->set_clock(XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &ti68k_state::v200_mem);

	SHARP_LH28F320BF(config.replace(), m_flash);

	/* video hardware */
	subdevice<screen_device>("screen")->set_visarea(0, 240-1, 0, 128-1);
}


void ti68k_state::ti89t(machine_config &config)
{
	ti89(config);
	m_maincpu->set_clock(XTAL(16'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &ti68k_state::ti89t_mem);

	SHARP_LH28F320BF(config.replace(), m_flash);
}


/* ROM definition */
ROM_START( ti89 )
	ROM_REGION16_BE( 0x200000, "flash", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v100", "V 1.00 - HW1" )
	ROMX_LOAD( "ti89v100.rom",   0x000000, 0x200000, CRC(264b34ad) SHA1(c87586a7e9b6d49fbe908fbb6f3c0038f3498573), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v100a", "V 1.00 [a] - HW1" )
	ROMX_LOAD( "ti89v100a.rom",  0x000000, 0x200000, CRC(95199934) SHA1(b8e3cdeb4705b0c7e0a15ab6c6f62bcde14a3a55), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "v100m", "V 1.00 [m] - HW1" )
	ROMX_LOAD( "ti89v100m.rom",  0x000000, 0x200000, CRC(b9059e06) SHA1(b33a7c2935eb9f73b210bcf6e7c7f32d1548a9d5), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 3, "v100m2", "V 1.00 [m2] - HW1" )
	ROMX_LOAD( "ti89v100m2.rom", 0x000000, 0x200000, CRC(cdd69d34) SHA1(1686362b0997bb9597f39b443490d4d8d85b56cc), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 4, "v105", "V 1.05 - HW1" )
	ROMX_LOAD( "ti89v105.rom",   0x000000, 0x200000, CRC(3bc0b474) SHA1(46fe0cd511eb81d53dc12cc4bdacec8a5bba5171), ROM_BIOS(4))
	ROM_SYSTEM_BIOS( 5, "v201", "V 2.01 - HW1" )
	ROMX_LOAD( "ti89v201.rom",   0x000000, 0x200000, CRC(fa6745e9) SHA1(e4ee6067df9b975356cef6c5a81d0ec664371c1d), ROM_BIOS(5))
	ROM_SYSTEM_BIOS( 6, "v203", "V 2.03 - HW1" )
	ROMX_LOAD( "ti89v203.rom",   0x000000, 0x200000, CRC(a3a74eca) SHA1(55aae3561722a96973b430e3d4cb4f513298ea4e), ROM_BIOS(6))
	ROM_SYSTEM_BIOS( 7, "v203m", "V 2.03 [m] - HW 1" )
	ROMX_LOAD( "ti89v203m.rom",  0x000000, 0x200000, CRC(d79068f7) SHA1(5b6f571417889b11ae19eef99a5fda4f027d5ec2), ROM_BIOS(7))
	ROM_SYSTEM_BIOS( 8, "v209",  "V 2.09 - HW 1" )
	ROMX_LOAD( "ti89v209.rom",   0x000000, 0x200000, CRC(f76f9c15) SHA1(66409ef4b20190a3b7c0d48cbd30257580b47dcd), ROM_BIOS(8))
	ROM_SYSTEM_BIOS( 9, "v105-2","V 1.05 - HW2" )
	ROMX_LOAD( "ti89v105-2.rom", 0x000000, 0x200000, CRC(83817402) SHA1(b2ddf785e973cc3f9a437d058a68abdf7ca52ea2), ROM_BIOS(9))
	ROM_SYSTEM_BIOS( 10, "v203-2",  "V 2.03 - HW2" )
	ROMX_LOAD( "ti89v203-2.rom", 0x000000, 0x200000, CRC(5e0400a9) SHA1(43c608ee72f15aed56cb5762948ec6a3c93dd9d8), ROM_BIOS(10))
	ROM_SYSTEM_BIOS( 11, "v203m-2", "V 2.03 [m] - HW2" )
	ROMX_LOAD( "ti89v203m-2.rom", 0x000000, 0x200000, CRC(04d5d76d) SHA1(14ca44b64c29aa1bf274508ca40fe69224f5a7cc), ROM_BIOS(11))
	ROM_SYSTEM_BIOS( 12, "v205-2", "V 2.05 - HW2" )
	ROMX_LOAD( "ti89v205-2.rom", 0x000000, 0x200000, CRC(37c4653c) SHA1(f48d00a57430230e489e243383513485009b1b98), ROM_BIOS(12))
	ROM_SYSTEM_BIOS( 13, "v205-2m", "V 2.05 [m] - HW2" )
	ROMX_LOAD( "ti89v205m-2.rom", 0x000000, 0x200000, CRC(e58a23f9) SHA1(d4cb23fb4b414a43802c37dc3c572a8ede670e0f), ROM_BIOS(13))
	ROM_SYSTEM_BIOS( 14, "v205-2m2","V 2.05 [m2] - HW2" )
	ROMX_LOAD( "ti89v205m2-2.rom", 0x000000, 0x200000, CRC(a8ba976c) SHA1(38bd25ada5e2066c64761d1008a9327a37d68654), ROM_BIOS(14))
	ROM_SYSTEM_BIOS( 15,"v209-2", "V 2.09 - HW2" )
	ROMX_LOAD( "ti89v209-2.rom", 0x000000, 0x200000, CRC(242a238f) SHA1(9668df314a0180ef210796e9cb651c5e9f17eb07), ROM_BIOS(15))
ROM_END

ROM_START( ti92 )
	ROM_REGION16_BE( 0x200000, "flash", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v111", "V 1.11" )
	ROMX_LOAD( "ti92v111.rom",  0x000000, 0x100000, CRC(67878d52) SHA1(c0fdf162961922a76f286c93fd9b861ce20f23a3), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v13e", "V 1.3 [e]" )
	ROMX_LOAD( "ti92v13e.rom",  0x000000, 0x100000, CRC(316c8196) SHA1(82c8cd484c6aebe36f814a2023d2afad6d87f840), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "v14e", "V 1.4 [e]" )
	ROMX_LOAD( "ti92v14e.rom",  0x000000, 0x100000, CRC(239e9405) SHA1(df2f1ab17d490fda43a02f5851b5a15052361b28), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 3, "v17e", "V 1.7 [e]" )
	ROMX_LOAD( "ti92v17e.rom",  0x000000, 0x100000, CRC(83e27cc5) SHA1(aec5a6a6157ff94a4e665fa3fe747bacb6688cd4), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 4, "v111e", "V 1.11 [e]" )
	ROMX_LOAD( "ti92v111e.rom", 0x000000, 0x100000, CRC(4a343833) SHA1(ab4eaacc8c83a861c8d37df5c10e532d0d580460), ROM_BIOS(4))
	ROM_SYSTEM_BIOS( 5, "v112e", "V 1.12 [e]" )
	ROMX_LOAD( "ti92v112e.rom", 0x000000, 0x100000, CRC(9a6947a0) SHA1(8bb0538ca98711e9ad46c56e4dfd609d4699be30), ROM_BIOS(5))
	ROM_SYSTEM_BIOS( 6, "v21e", "V 2.1 [e]" )
	ROMX_LOAD( "ti92v21e.rom",  0x000000, 0x200000, CRC(5afb5863) SHA1(bf7b260d37d1502cc4b08dea5e1d55b523f27925), ROM_BIOS(6))
ROM_END

ROM_START( ti92p )
	ROM_REGION16_BE( 0x200000, "flash", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v100", "V 1.00 - HW1" )
	ROMX_LOAD( "ti92pv100.rom", 0x0000, 0x200000, CRC(c651a586) SHA1(fbbf7e053e70eefe517f9aae40c072036bc614ea), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v101", "V 1.01 - HW1" )
	ROMX_LOAD( "ti92pv101.rom", 0x0000, 0x200000, CRC(826b1539) SHA1(dd8969511fc6233bf2047f83c3306ac8d2be5644), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "v101a","V 1.01 [a] - HW1" )
	ROMX_LOAD( "ti92pv101a.rom", 0x0000, 0x200000, CRC(18f9002f) SHA1(2bf13ba7da0212a8706c5a43853dc2ccb8c2257d), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 3, "v101m", "V 1.01 [m] - HW1" )
	ROMX_LOAD( "ti92pv101m.rom", 0x0000, 0x200000, CRC(fe2b6e77) SHA1(0e1bb8c677a726ee086c1a4280ab59de95b4abe2), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 4, "v105", "V 1.05 - HW1" )
	ROMX_LOAD( "ti92pv105.rom", 0x0000, 0x200000, CRC(cd945824) SHA1(6941aca243c6fd5c8a377253bffc2ffb5a84c41b), ROM_BIOS(4))
	ROM_SYSTEM_BIOS( 5, "v105-2", "V 1.05 - HW2" )
	ROMX_LOAD( "ti92pv105-2.rom", 0x0000, 0x200000, CRC(289aa84f) SHA1(c9395750e20d5a201401699d156b62f00530fcdd), ROM_BIOS(5))
	ROM_SYSTEM_BIOS( 6, "v203", "V 2.03 - HW2" )
	ROMX_LOAD( "ti92pv203.rom", 0x0000, 0x200000, CRC(1612213e) SHA1(1715dd5913bed12baedc4912e9abe0cb4e48cd45), ROM_BIOS(6))
	ROM_SYSTEM_BIOS( 7, "v204", "V 2.04 - HW2" )
	ROMX_LOAD( "ti92pv204.rom", 0x0000, 0x200000, CRC(86819be3) SHA1(78032a0f5f11d1e9a45ffbea91e7f9657fd1a8ae), ROM_BIOS(7))
	ROM_SYSTEM_BIOS( 8, "v205", "V 2.05 - HW2" )
	ROMX_LOAD( "ti92pv205.rom",  0x0000, 0x200000, CRC(9509c575) SHA1(703410d8bb98b8ec14277efcd8b7dda45a7cf358), ROM_BIOS(8))
	ROM_SYSTEM_BIOS( 9, "v205m", "V 2.05 [m] - HW2" )
	ROMX_LOAD( "ti92pv205m.rom",  0x000000, 0x200000, CRC(13ef4d57) SHA1(6ef290bb0dda72f645cd3eca9cc1185f6a2d32dc), ROM_BIOS(9))
	ROM_SYSTEM_BIOS( 10, "v209", "V 2.09 - HW2" )
	ROMX_LOAD( "ti92pv209.rom",  0x000000, 0x200000, CRC(4851ad52) SHA1(10e6c2cdc60623bf0be7ea72a9ec840259fb37c3), ROM_BIOS(10))
ROM_END

ROM_START( v200 )
	ROM_REGION16_BE( 0x400000, "flash", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v209", "V 2.09" )
	ROMX_LOAD( "voyage200v209.rom", 0x0000, 0x400000, CRC(f805c7a6) SHA1(818b919058ba3bd7d15604f11fff6740010d07fc), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v310", "V 3.10" )
	ROMX_LOAD( "voyage200v310.rom", 0x0000, 0x400000, CRC(ed4cbfd2) SHA1(39cdb9932f314ff792b1cc5e3fe041d98b9fd101), ROM_BIOS(1))
ROM_END

ROM_START( ti89t )
	ROM_REGION16_BE( 0x400000, "flash", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v300", "V 3.00" )
	ROMX_LOAD( "ti89tv300.rom", 0x0000, 0x400000, CRC(55eb4f5a) SHA1(4f919d7752caf2559a79883ec8711a9701d19513), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v310", "V 3.10" )
	ROMX_LOAD( "ti89tv310.rom", 0x0000, 0x400000, CRC(b6967cca) SHA1(fb4f09e5c4500dee651b8de537e502ab97cb8328), ROM_BIOS(1))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY              FULLNAME          FLAGS
COMP( 1998, ti89,  0,       0,     ti89,    ti8x,  ti68k_state, empty_init, "Texas Instruments", "TI-89",          MACHINE_NO_SOUND )
COMP( 1995, ti92,  0,       0,     ti92,    ti9x,  ti68k_state, empty_init, "Texas Instruments", "TI-92",          MACHINE_NO_SOUND )
COMP( 1999, ti92p, 0,       0,     ti92p,   ti9x,  ti68k_state, empty_init, "Texas Instruments", "TI-92 Plus",     MACHINE_NO_SOUND )
COMP( 2002, v200,  0,       0,     v200,    ti9x,  ti68k_state, empty_init, "Texas Instruments", "Voyage 200 PLT", MACHINE_NO_SOUND )
COMP( 2004, ti89t, 0,       0,     ti89t,   ti8x,  ti68k_state, empty_init, "Texas Instruments", "TI-89 Titanium", MACHINE_NO_SOUND )
