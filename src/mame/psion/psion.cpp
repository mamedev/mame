// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Psion Organiser II series

        TODO:
        - dump CGROM of the HD44780
        - emulate other devices in slot C (Comms Link, Printer, etc.)

        Note:
        - 4 lines display has an custom LCD controller derived from an HD66780
        - NVRAM works only if the machine is turned off (with OFF menu) before closing MAME
        - psion1 goes into standby right after a cold boot, so press the ON button

        More info:
            http://archive.psion2.org/org2/techref/index.htm

****************************************************************************/


#include "emu.h"

#include "psion_pack.h"
#include "cpu/m6800/m6801.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "video/hd44780.h"
#include "sound/beep.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class psion_state : public driver_device
{
public:
	psion_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "hd44780")
		, m_screen(*this, "screen")
		, m_beep(*this, "beeper")
		, m_pack(*this, "pack%u", 1U)
		, m_topslot(*this, "topslot")
		, m_nvram(*this, "nvram")
		, m_ram(*this, "ram")
		, m_kb_lines(*this, "K%u", 1U)
		, m_kb_on(*this, "ON")
		, m_battery(*this, "BATTERY")
	{ }

	void psion_base(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(psion_on);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<hd6301x_cpu_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
	required_device<screen_device> m_screen;
	required_device<beep_device> m_beep;
	required_device_array<datapack_device, 2> m_pack;
	optional_device<datapack_device> m_topslot;
	required_device<nvram_device> m_nvram;
	required_device<ram_device> m_ram;
	required_ioport_array<7> m_kb_lines;
	required_ioport m_kb_on;
	required_ioport m_battery;

	uint8_t kb_read();
	void port2_w(offs_t offset, uint8_t data, uint8_t ddr);
	uint8_t port2_r();
	uint8_t port5_r();
	void port6_w(uint8_t data);
	uint8_t port6_r();

	void psion_palette(palette_device &palette) const;
	TIMER_DEVICE_CALLBACK_MEMBER(nmi_timer);

	uint16_t m_kb_counter = 0;
	bool m_nmi_enable = false;
	uint8_t m_pulse_enable = 0;
};


class psion1_state : public psion_state
{
public:
	psion1_state(const machine_config &mconfig, device_type type, const char *tag)
		: psion_state(mconfig, type, tag)
	{ }

	void psion1(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	HD44780_PIXEL_UPDATE(psion1_pixel_update);

	void psion1_mem(address_map &map) ATTR_COLD;

	uint8_t reset_kb_counter_r();
	uint8_t inc_kb_counter_r();
	uint8_t switchoff_r();
};


class psion2_state : public psion_state
{
public:
	psion2_state(const machine_config &mconfig, device_type type, const char *tag)
		: psion_state(mconfig, type, tag)
		, m_rambank1(*this, "rambank1")
		, m_rambank2(*this, "rambank2")
		, m_rombank1(*this, "rombank1")
		, m_rombank2(*this, "rombank2")
	{ }

	void psion2(machine_config &config);
	void psioncm(machine_config &config);
	void psionxp(machine_config &config);
	void psionla(machine_config &config);
	void psionlz(machine_config &config);
	void psionlz64(machine_config &config);
	void psionp200(machine_config &config);
	void psionp350(machine_config &config);
	void psionp432(machine_config &config);
	void psionp464(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_memory_bank m_rambank1;
	optional_memory_bank m_rambank2;
	required_memory_bank m_rombank1;
	optional_memory_bank m_rombank2;

	HD44780_PIXEL_UPDATE(lz_pixel_update);

	void io_rw(uint16_t offset);
	void io_w(offs_t offset, uint8_t data);
	uint8_t io_r(offs_t offset);

	void psioncm_mem(address_map &map) ATTR_COLD;
	void psionxp_mem(address_map &map) ATTR_COLD;
	void psionla_mem(address_map &map) ATTR_COLD;

	// RAM/ROM banks
	uint8_t m_rambank_count = 0;
	uint8_t m_rombank_count = 0;
};


TIMER_DEVICE_CALLBACK_MEMBER(psion_state::nmi_timer)
{
	if (m_nmi_enable)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

uint8_t psion_state::kb_read()
{
	uint8_t data = 0x7c;

	if (m_kb_counter)
	{
		for (int line = 0; line < 7; line++)
			if (m_kb_counter == (0x7f & ~(1 << line)))
				data = m_kb_lines[line]->read();
	}
	else
	{
		// Read all the input lines
		for (int line = 0; line < 7; line++)
			data &= m_kb_lines[line]->read();
	}

	return data & 0x7c;
}


void psion_state::port2_w(offs_t offset, uint8_t data, uint8_t ddr)
{
	/* datapack i/o data bus */
	m_pack[0]->data_w(data & ddr);
	m_pack[1]->data_w(data & ddr);

	if (m_topslot)
		m_topslot->data_w(data & ddr);
}

uint8_t psion_state::port2_r()
{
	/* datapack i/o data bus */
	uint8_t data = m_pack[0]->data_r() | m_pack[1]->data_r();

	if (m_topslot)
		data |= m_topslot->data_r();

	return data;
}

uint8_t psion_state::port5_r()
{
	/*
	x--- ---- ON key active high
	-xxx xx-- keys matrix active low
	---- --x- pulse
	---- ---x battery status
	*/
	return kb_read() | m_battery->read() | m_kb_on->read() | (m_kb_counter == 0x7ff)<<1 | m_pulse_enable<<1;
}

void psion_state::port6_w(uint8_t data)
{
	/*
	datapack control lines
	x--- ---- slot on/off
	-x-- ---- slot 3
	--x- ---- slot 2
	---x ---- slot 1
	---- x--- output enable
	---- -x-- program line
	---- --x- reset line
	---- ---x clock line
	*/
	m_pack[0]->control_w((data & 0x8f) | (data & 0x10));
	m_pack[1]->control_w((data & 0x8f) | ((data & 0x20) >> 1));

	if (m_topslot)
		m_topslot->control_w((data & 0x8f) | ((data & 0x40) >> 2));
}

uint8_t psion_state::port6_r()
{
	/* datapack control lines */
	uint8_t data = (m_pack[0]->control_r() | (m_pack[1]->control_r() & 0x8f)) | ((m_pack[1]->control_r() & 0x10) << 1);

	if (m_topslot)
		data |= (m_topslot->control_r() & 0x8f) | ((m_topslot->control_r() & 0x10) << 2);

	return data;
}

/* Read/Write common */
void psion2_state::io_rw(uint16_t offset)
{
	if (machine().side_effects_disabled())
		return;

	switch (offset & 0xffc0)
	{
	case 0xc0:
		/* switch off, CPU goes into standby mode */
		m_nmi_enable = false;
		m_maincpu->set_input_line(M6801_STBY_LINE, ASSERT_LINE);
		break;
	case 0x100:
		m_pulse_enable = 1;
		break;
	case 0x140:
		m_pulse_enable = 0;
		break;
	case 0x180:
		m_beep->set_state(1);
		break;
	case 0x1c0:
		m_beep->set_state(0);
		break;
	case 0x200:
		m_kb_counter = 0;
		break;
	case 0x240:
		if (offset == 0x260 && (m_rombank_count || m_rambank_count))
		{
			if (m_rambank_count)
				m_rambank2->set_entry(0);

			if (m_rombank_count)
				m_rombank2->set_entry(0);
		}
		else
		{
			m_kb_counter++;
		}
		break;
	case 0x280:
		if (offset == 0x2a0 && m_rambank_count)
		{
			int const rambank = m_rambank2->entry() + 1;
			if (rambank < m_rambank_count)
				m_rambank2->set_entry(rambank);
		}
		else
		{
			m_nmi_enable = true;
		}
		break;
	case 0x2c0:
		if (offset == 0x2e0 && m_rombank_count)
		{
			int const rombank = m_rombank2->entry() + 1;
			if (rombank < m_rombank_count)
				m_rombank2->set_entry(rombank);
		}
		else
		{
			m_nmi_enable = false;
		}
		break;
	}
}

void psion2_state::io_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xffc0)
	{
	case 0x80:
		m_lcdc->write(offset & 0x01, data);
		break;
	default:
		io_rw(offset);
		break;
	}
}

uint8_t psion2_state::io_r(offs_t offset)
{
	switch (offset & 0xffc0)
	{
	case 0x80:
		return m_lcdc->read(offset & 0x01);
	default:
		io_rw(offset);
		break;
	}

	return 0;
}

INPUT_CHANGED_MEMBER(psion_state::psion_on)
{
	/* reset the CPU for resume from standby */
	if (newval && m_maincpu->standby())
	{
		m_maincpu->set_input_line(M6801_STBY_LINE, CLEAR_LINE);
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	}
}

uint8_t psion1_state::reset_kb_counter_r()
{
	if (!machine().side_effects_disabled())
		m_kb_counter = 0;

	return 0;
}

uint8_t psion1_state::inc_kb_counter_r()
{
	if (!machine().side_effects_disabled())
		m_kb_counter++;

	return 0;
}

uint8_t psion1_state::switchoff_r()
{
	if (!machine().side_effects_disabled())
		m_maincpu->set_input_line(M6801_STBY_LINE, ASSERT_LINE);

	return 0;
}

void psion1_state::psion1_mem(address_map &map)
{
	map(0x2000, 0x2001).mirror(0x07fe).rw(m_lcdc, FUNC(hd44780_device::read), FUNC(hd44780_device::write));
	map(0x2800, 0x2800).r(FUNC(psion1_state::reset_kb_counter_r));
	map(0x2e00, 0x2e00).r(FUNC(psion1_state::switchoff_r));
	map(0x3000, 0x3000).r(FUNC(psion1_state::inc_kb_counter_r));
	map(0x4000, 0x47ff).ram().share("ram");
}

void psion2_state::psioncm_mem(address_map &map)
{
	map.unmap_value_low();
	map(0x0100, 0x03ff).rw(FUNC(psion2_state::io_r), FUNC(psion2_state::io_w));
	map(0x2000, 0x3fff).bankrw("rambank1");
	map(0x8000, 0xbfff).bankr("rombank2");
	map(0xc000, 0xffff).bankr("rombank1");
}

void psion2_state::psionxp_mem(address_map &map)
{
	map.unmap_value_low();
	map(0x0100, 0x03ff).rw(FUNC(psion2_state::io_r), FUNC(psion2_state::io_w));
	map(0x2000, 0x5fff).bankrw("rambank1");
	map(0x8000, 0xbfff).bankr("rombank2");
	map(0xc000, 0xffff).bankr("rombank1");
}

void psion2_state::psionla_mem(address_map &map)
{
	map.unmap_value_low();
	map(0x0100, 0x03ff).rw(FUNC(psion2_state::io_r), FUNC(psion2_state::io_w));
	map(0x0400, 0x3fff).bankrw("rambank1");
	map(0x4000, 0x7fff).bankrw("rambank2");
	map(0x8000, 0xbfff).bankr("rombank2");
	map(0xc000, 0xffff).bankr("rombank1");
}


/* Input ports */
INPUT_PORTS_START( psion )
	PORT_START("BATTERY")
	PORT_CONFNAME( 0x01, 0x00, "Battery Status" )
	PORT_CONFSETTING( 0x00, DEF_STR( Normal ) )
	PORT_CONFSETTING( 0x01, "Low Battery" )

	PORT_START("ON")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ON/CLEAR") PORT_CODE(KEYCODE_ESC)  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psion_state::psion_on), 0)
INPUT_PORTS_END

INPUT_PORTS_START( psion1 )
	PORT_INCLUDE(psion)

	PORT_START("K1")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2192 [>>]") PORT_CODE(KEYCODE_RIGHT)  // U+2192 = →
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2190 [<<]") PORT_CODE(KEYCODE_LEFT)   // U+2190 = ←
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2193 [FIND]") PORT_CODE(KEYCODE_DOWN) // U+2193 = ↓
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2191 [SAVE]") PORT_CODE(KEYCODE_UP)   // U+2191 = ↑
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MODE/HOME") PORT_CODE(KEYCODE_EQUALS)

	PORT_START("K2")
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S [,]") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M [%]") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G [=]") PORT_CODE(KEYCODE_G)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A [<]") PORT_CODE(KEYCODE_A)

	PORT_START("K3")
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y [(]") PORT_CODE(KEYCODE_Y)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T [:]") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N [$]") PORT_CODE(KEYCODE_N)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H [\"]") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B [>]") PORT_CODE(KEYCODE_B)

	PORT_START("K4")
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z [)]") PORT_CODE(KEYCODE_Z)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U [0]") PORT_CODE(KEYCODE_U)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O [1]") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I [4]") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C [7]") PORT_CODE(KEYCODE_C)

	PORT_START("K5")
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DELETE") PORT_CODE(KEYCODE_DEL)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W [EE]") PORT_CODE(KEYCODE_W)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q [3]") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K [6]") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E [9]") PORT_CODE(KEYCODE_E)

	PORT_START("K6")
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EXECUTE") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X [+]") PORT_CODE(KEYCODE_X)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R [-]") PORT_CODE(KEYCODE_R)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L [*]") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F [/]") PORT_CODE(KEYCODE_F)

	PORT_START("K7")
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V [.]") PORT_CODE(KEYCODE_V)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P [2]") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J [5]") PORT_CODE(KEYCODE_J)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D [8]") PORT_CODE(KEYCODE_D)
INPUT_PORTS_END

INPUT_PORTS_START( psion2 )
	PORT_INCLUDE(psion)

	// Normal keyboard
	PORT_START("K1")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MODE") PORT_CODE(KEYCODE_EQUALS)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2191 [CAP]") PORT_CODE(KEYCODE_UP)   // U+2191 = ↑
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2193 [NUM]") PORT_CODE(KEYCODE_DOWN) // U+2193 = ↓
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2190") PORT_CODE(KEYCODE_LEFT)       // U+2190 = ←
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2192") PORT_CODE(KEYCODE_RIGHT)      // U+2192 = →

	PORT_START("K2")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S [;]") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M [,]") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G [=]") PORT_CODE(KEYCODE_G)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A [<]") PORT_CODE(KEYCODE_A)

	PORT_START("K3")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T [:]") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N [$]") PORT_CODE(KEYCODE_N)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H [\"]") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B [>]") PORT_CODE(KEYCODE_B)

	PORT_START("K4")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y [0]") PORT_CODE(KEYCODE_Y)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U [1]") PORT_CODE(KEYCODE_U)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O [4]") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I [7]") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C [(]") PORT_CODE(KEYCODE_C)

	PORT_START("K5")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W [3]") PORT_CODE(KEYCODE_W)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q [6]") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K [9]") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E [%]") PORT_CODE(KEYCODE_E)

	PORT_START("K6")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EXE") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X [+]") PORT_CODE(KEYCODE_X)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R [-]") PORT_CODE(KEYCODE_R)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L [*]") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F [/]") PORT_CODE(KEYCODE_F)

	PORT_START("K7")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z [.]") PORT_CODE(KEYCODE_Z)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V [2]") PORT_CODE(KEYCODE_V)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P [5]") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J [8]") PORT_CODE(KEYCODE_J)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D [)]") PORT_CODE(KEYCODE_D)
INPUT_PORTS_END

INPUT_PORTS_START( psion2a )
	PORT_INCLUDE(psion)

	// Alpha keyboard
	PORT_START("K1")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MODE") PORT_CODE(KEYCODE_EQUALS)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2191") PORT_CODE(KEYCODE_UP)    // U+2191 = ↑
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2193") PORT_CODE(KEYCODE_DOWN)  // U+2193 = ↓
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2190") PORT_CODE(KEYCODE_LEFT)  // U+2190 = ←
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2192") PORT_CODE(KEYCODE_RIGHT) // U+2192 = →

	PORT_START("K2")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)

	PORT_START("K3")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("No [N]") PORT_CODE(KEYCODE_N)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)

	PORT_START("K4")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("YES [Y]") PORT_CODE(KEYCODE_Y)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)

	PORT_START("K5")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". [W]") PORT_CODE(KEYCODE_W)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 [Q]") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 [K]") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 [E]") PORT_CODE(KEYCODE_E)

	PORT_START("K6")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- [X]") PORT_CODE(KEYCODE_X)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 [R]") PORT_CODE(KEYCODE_R)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 [L]") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 [F]") PORT_CODE(KEYCODE_F)

	PORT_START("K7")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 [V]") PORT_CODE(KEYCODE_V)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 [P]") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 [J]") PORT_CODE(KEYCODE_J)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 [D]") PORT_CODE(KEYCODE_D)
INPUT_PORTS_END

INPUT_PORTS_START( psion2n )
	PORT_INCLUDE(psion)

	// Numerical keyboard
	PORT_START("K1")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)

	PORT_START("K2")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EXE") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MODE") PORT_CODE(KEYCODE_EQUALS)

	PORT_START("K3")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K4")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K5")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K6")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K7")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


void psion_state::machine_start()
{
	m_nvram->set_base(m_ram->pointer(), m_ram->size());

	save_item(NAME(m_kb_counter));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_pulse_enable));
}

void psion2_state::machine_start()
{
	psion_state::machine_start();

	uint8_t *rom_base = memregion("maincpu")->base();
	uint32_t rom_size = memregion("maincpu")->bytes();

	m_rombank_count = 0;
	switch (rom_size)
	{
	case 0x8000: // 32K
		m_rombank1->set_base(rom_base + 0x4000);
		m_rombank2->set_base(rom_base);
		break;

	default: // >= 64K
		m_rombank_count = (rom_size - 0x4000) / 0x4000;

		m_rombank1->set_base(rom_base + 0x4000);
		m_rombank2->configure_entry(0, rom_base);
		m_rombank2->configure_entries(1, m_rombank_count-1, rom_base + 0x8000, 0x4000);
		m_rombank2->set_entry(0);
		break;
	}

	m_rambank_count = 0;
	switch (m_ram->size())
	{
	case 0x2000: // 8K
	case 0x4000: // 16K
		m_rambank1->set_base(m_ram->pointer());
		break;

	case 0x8000: // 32K
		m_rambank1->set_base(m_ram->pointer() + 0x0400);
		m_rambank2->set_base(m_ram->pointer() + 0x4000);
		break;

	default: // >= 64K
		m_rambank_count = (m_ram->size() - 0x4000) / 0x4000;

		m_rambank1->set_base(m_ram->pointer() + 0x0400);
		m_rambank2->configure_entries(0, m_rambank_count, m_ram->pointer() + 0x4000, 0x4000);
		m_rambank2->set_entry(0);
		break;
	}
}

void psion_state::machine_reset()
{
	m_nmi_enable = false;
	m_kb_counter = 0;
	m_pulse_enable = 0;
}

void psion1_state::machine_reset()
{
	psion_state::machine_reset();

	m_nmi_enable = true;
}

void psion2_state::machine_reset()
{
	psion_state::machine_reset();

	if (m_rambank_count)
		m_rambank2->set_entry(0);

	if (m_rombank_count)
		m_rombank2->set_entry(0);
}


HD44780_PIXEL_UPDATE(psion2_state::lz_pixel_update)
{
	if (pos < 40)
	{
		static const uint8_t psion_display_layout[] =
		{
			0x00, 0x01, 0x02, 0x03, 0x28, 0x29, 0x2a, 0x2b, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x2c, 0x2d, 0x2e, 0x2f,
			0x30, 0x31, 0x32, 0x33, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
			0x14, 0x15, 0x16, 0x17, 0x3c, 0x3d, 0x3e, 0x3f, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x40, 0x41, 0x42, 0x43,
			0x44, 0x45, 0x46, 0x47, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f
		};

		uint8_t char_pos = psion_display_layout[line*40 + pos];
		bitmap.pix((char_pos / 20) * 9 + y, (char_pos % 20) * 6 + x) = m_maincpu->standby() ? 0 : state;
	}
}

HD44780_PIXEL_UPDATE(psion1_state::psion1_pixel_update)
{
	if (pos < 8 && line < 2)
		bitmap.pix(y, (line * 8 + pos) * 6 + x) = m_maincpu->standby() ? 0 : state;
}

void psion_state::psion_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

static const gfx_layout psion_charlayout =
{
	5, 8,                   /* 5 x 8 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	{ 3, 4, 5, 6, 7},
	{ 0, 8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8                     /* 8 bytes */
};

static GFXDECODE_START( gfx_psion )
	GFXDECODE_ENTRY( "hd44780:cgrom", 0x0000, psion_charlayout, 0, 1 )
GFXDECODE_END


void psion_state::psion_base(machine_config &config)
{
	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(psion_state::psion_palette), 2);
	GFXDECODE(config, "gfxdecode", "palette", gfx_psion);

	HD44780(config, m_lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 3250).add_route(ALL_OUTPUTS, "mono", 1.00);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // RAM

	/* Datapack */
	PSION_DATAPACK(config, m_pack[0]).set_interface("psion_pack");
	PSION_DATAPACK(config, m_pack[1]).set_interface("psion_pack");
}

void psion1_state::psion1(machine_config &config)
{
	psion_base(config);

	HD6301X0(config, m_maincpu, 3.6864_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &psion1_state::psion1_mem);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->in_p2_cb().set(FUNC(psion1_state::port2_r));
	m_maincpu->out_p2_cb().set(FUNC(psion1_state::port2_w));
	m_maincpu->in_p5_cb().set(FUNC(psion1_state::port5_r));
	m_maincpu->in_p6_cb().set(FUNC(psion1_state::port6_r));
	m_maincpu->out_p6_cb().set(FUNC(psion1_state::port6_w));

	RAM(config, m_ram).set_default_size("2K");

	timer_device &nmi_timer(TIMER(config, "nmi_timer"));
	nmi_timer.configure_periodic(FUNC(psion1_state::nmi_timer), attotime::from_msec(500));
	nmi_timer.set_start_delay(attotime::from_seconds(1));

	m_screen->set_size(6*16, 1*8);
	m_screen->set_visarea_full();

	m_lcdc->set_lcd_size(1, 16);
	m_lcdc->set_pixel_update_cb(FUNC(psion1_state::psion1_pixel_update));

	SOFTWARE_LIST(config, "pack_list").set_original("psion1");
}

void psion2_state::psion2(machine_config &config)
{
	psion_base(config);

	HD6303X(config, m_maincpu, 3.6864_MHz_XTAL); // internal operating frequency is 0.9216 MHz
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->in_p2_cb().set(FUNC(psion2_state::port2_r));
	m_maincpu->out_p2_cb().set(FUNC(psion2_state::port2_w));
	m_maincpu->in_p5_cb().set(FUNC(psion2_state::port5_r));
	m_maincpu->in_p6_cb().set(FUNC(psion2_state::port6_r));
	m_maincpu->out_p6_cb().set(FUNC(psion2_state::port6_w));

	RAM(config, m_ram).set_default_size("32K");

	m_screen->set_size(6*16, 9*2);
	m_screen->set_visarea_full();

	m_lcdc->set_lcd_size(2, 16);

	TIMER(config, "nmi_timer").configure_periodic(FUNC(psion2_state::nmi_timer), attotime::from_seconds(1));

	PSION_DATAPACK(config, m_topslot).set_interface("psion_topslot");

	SOFTWARE_LIST(config, "pack_list").set_original("psion2");
}

void psion2_state::psioncm(machine_config &config)
{
	psion2(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &psion2_state::psioncm_mem);

	m_ram->set_default_size("8K");

	subdevice<software_list_device>("pack_list")->set_filter("CM");
}

void psion2_state::psionxp(machine_config &config)
{
	psion2(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &psion2_state::psionxp_mem);

	m_ram->set_default_size("16K");

	subdevice<software_list_device>("pack_list")->set_filter("XP");
}

void psion2_state::psionla(machine_config &config)
{
	psionxp(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &psion2_state::psionla_mem);

	m_ram->set_default_size("32K");
}

void psion2_state::psionp200(machine_config &config)
{
	psionla(config);

	subdevice<software_list_device>("pack_list")->set_filter("POS");
}

void psion2_state::psionp350(machine_config &config)
{
	psionla(config);

	m_ram->set_default_size("96K");

	subdevice<software_list_device>("pack_list")->set_filter("POS");
}

void psion2_state::psionlz(machine_config &config)
{
	psionla(config);

	/* video hardware */
	m_screen->set_size(6*20, 9*4);
	m_screen->set_visarea_full();

	m_lcdc->set_lcd_size(4, 20);
	m_lcdc->set_pixel_update_cb(FUNC(psion2_state::lz_pixel_update));

	subdevice<software_list_device>("pack_list")->set_filter("LZ");
}

void psion2_state::psionlz64(machine_config &config)
{
	psionlz(config);

	m_ram->set_default_size("64K");

	subdevice<software_list_device>("pack_list")->set_filter("LZ,LZ64");
}

void psion2_state::psionp432(machine_config &config)
{
	psionlz(config);

	subdevice<software_list_device>("pack_list")->set_filter("POS");
}

void psion2_state::psionp464(machine_config &config)
{
	psionlz64(config);

	subdevice<software_list_device>("pack_list")->set_filter("POS");
}

/* ROM definition */

ROM_START( psion1 )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("psion1.rom", 0x0000, 0x1000, CRC(7e2609c1) SHA1(a3320ea8ac3ab9e0039ee16f7c571731adde5869))
ROM_END

ROM_START( psioncm )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS( "v33" )
	ROM_SYSTEM_BIOS(0, "v24", "CM v2.4 English")
	ROMX_LOAD("24-cm.rom",    0x0000, 0x8000,  CRC(f6798394) SHA1(736997f0db9a9ee50d6785636bdc3f8ff1c33c66), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v26", "CM v2.6 English")
	ROMX_LOAD("26-cm.rom",    0x0000, 0x8000,  CRC(21b7c94c) SHA1(e0a3168c96a3f0b37b8698e86574e40597fe3c62), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v33", "CM v3.3 English")
	ROMX_LOAD("33-cm.rom",    0x0000, 0x8000,  CRC(5c10b167) SHA1(6deea00fe648bddae1d61a22858023bc80277ea0), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v33f", "CM v3.3 French")
	ROMX_LOAD("33-cmf.rom",   0x0000, 0x8000,  CRC(4d626ce2) SHA1(82b96f11a0abfc1931b6022b84733d975ad7ab2b), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v36f", "CM v3.6 French")
	ROMX_LOAD("36-cmf.rom",   0x0000, 0x8000,  CRC(beabe0f5) SHA1(a5ef3bb92190a257cb0e94d58b2c23935436edeb), ROM_BIOS(4))
ROM_END

ROM_START( psioncmm )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v37", "CM v3.7 Multilingual")
	ROMX_LOAD("37-cmm.rom",   0x0000, 0x10000, CRC(5be1aa1d) SHA1(34a6d9e2bb4941740b8bcf6b9b46a34b0ae0be5a), ROM_BIOS(0))
ROM_END

ROM_START( psionxp )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS( "v31" )
	ROM_SYSTEM_BIOS(0, "v24", "XP v2.4 English")
	ROMX_LOAD("24-xp.rom",    0x0000, 0x8000,  CRC(3407062c) SHA1(c3818f3f0865cc235905bd7ef6cba325e44fa076), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v26", "XP v2.6 English")
	ROMX_LOAD("26-xp.rom",    0x0000, 0x8000,  CRC(a81db40f) SHA1(af72d94ccee1fa1dade8776bdbd39920665a68b7), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v31", "XP v3.1 English")
	ROMX_LOAD("31-xp.rom",    0x0000, 0x8000,  CRC(cb10b6bf) SHA1(af5e82f62a149ff0dfa6e60414c928649a2abaaa), ROM_BIOS(2))
ROM_END

ROM_START( psionla )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS( "v36" )
	ROM_SYSTEM_BIOS(0, "v30hp", "LA v3.0 Hand Held Products")
	ROMX_LOAD("30-lahp.rom",  0x0000, 0x8000,  CRC(50192528) SHA1(c556d53f70bf5ecae756b2ebfc6d954912316bbe), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v33", "LA v3.3 English")
	ROMX_LOAD("33-la.rom",    0x0000, 0x8000,  CRC(02668ed4) SHA1(e5d4ee6b1cde310a2970ffcc6f29a0ce09b08c46), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v34g", "LA v3.4 German")
	ROMX_LOAD("34-lag.rom",   0x0000, 0x8000,  CRC(13a92c4b) SHA1(dab8bd6a41a5fd509c5ad4b0b0ab80d14f2c421a), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v36", "LA v3.6 English")
	ROMX_LOAD("36-la.rom",    0x0000, 0x8000,  CRC(7442c7f6) SHA1(94f15bd06bd750be70fa4a4ab588237c5a703f65), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v36f", "LA v3.6 French")
	ROMX_LOAD("36-laf.rom",   0x0000, 0x8000,  CRC(036ef00e) SHA1(98f303273e570e94a1e25a58cf1ffcec0db32165), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "v36g", "LA v3.6 German")
	ROMX_LOAD("36-lag.rom",   0x0000, 0x8000,  CRC(33c6b4d4) SHA1(81d665ce0fa325996d52004655349162093d60a6), ROM_BIOS(5))
	ROM_END

ROM_START( psionlam )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v37", "LA v3.7 Multilingual")
	ROMX_LOAD("37-lam.dat",   0x0000, 0x10000, CRC(7ee3a1bc) SHA1(c7fbd6c8e47c9b7d5f636e9f56e911b363d6796b), ROM_BIOS(0))
ROM_END

ROM_START( psionp200 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v36", "POS 200 v3.6")
	ROMX_LOAD("36-p200.rom",  0x0000, 0x8000,  CRC(4569ef5b) SHA1(8c275474cc6e3f50156f0b6e32121cadd14ea8be), ROM_BIOS(0))
ROM_END

ROM_START( psionp200a )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS( "v36" )
	ROM_SYSTEM_BIOS(0, "v33", "Alpha POS 200 v3.3")
	ROMX_LOAD("33-p200a.rom", 0x0000, 0x8000,  CRC(91e94998) SHA1(e9e8106eb9283d20452697859894aa407cc07bd1), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v36", "Alpha POS 200 v3.6")
	ROMX_LOAD("36-p200a.rom", 0x0000, 0x8000,  CRC(36cceeb7) SHA1(57069812c5a16babfff91dc7d7e0842e5dc68652), ROM_BIOS(1))
ROM_END

ROM_START( psionp250 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v36b", "POS 250 v3.6")
	ROMX_LOAD("36-p250.rom",  0x0000, 0x8000,  CRC(235cc76a) SHA1(3229cdff4b049a1fbf9a758ce3abf3fdc9b547c9), ROM_BIOS(0))
ROM_END

ROM_START( psionp350 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS( "v36" )
	ROM_SYSTEM_BIOS(0, "v36", "POS 350 v3.6")
	ROMX_LOAD("36-p350.rom",  0x0000, 0x8000,  CRC(3a371a74) SHA1(9167210b2c0c3bd196afc08ca44ab23e4e62635e), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v38", "POS 350 v3.8")
	ROMX_LOAD("38-p350.rom",  0x0000, 0x8000,  CRC(1b8b082f) SHA1(a3e875a59860e344f304a831148a7980f28eaa4a), ROM_BIOS(1))
ROM_END

ROM_START( psionlz )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS( "v46" )
	ROM_SYSTEM_BIOS(0, "v42", "LZ v4.2 English, French, German")
	ROMX_LOAD("42-lz.rom",    0x0000, 0x10000, CRC(f2d6ad47) SHA1(ee8315ae872463068d805c6e0b71f62ae8eb65be), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v44", "LZ v4.4 English, French, German")
	ROMX_LOAD("44-lz.rom",    0x0000, 0x10000, CRC(4a0a990b) SHA1(dde0ba69a4a7f02b610ad6bd69a8b8552b060223), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v45", "LZ v4.5 English, French, German")
	ROMX_LOAD("45-lz.rom",    0x0000, 0x10000, CRC(f95d8f39) SHA1(cb64152c2418bf730c89999d1b13c1d1ada1f082), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v45i", "LZ v4.5 English, Spanish, Italian")
	ROMX_LOAD("45-lzi.rom",   0x0000, 0x10000, CRC(202b556d) SHA1(39f061c4b94a1371bfe62484828a8ce424d7315c), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v45s", "LZ v4.5 English, Swedish, Danish")
	ROMX_LOAD("45-lzs.rom",   0x0000, 0x10000, CRC(2d082d7f) SHA1(fcd00864a0cc617e61997240945ea70a8e9fa211), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "v46", "LZ v4.6 English, French, German")
	ROMX_LOAD("46-lz.rom",    0x0000, 0x10000, CRC(22715f48) SHA1(cf460c81cadb53eddb7afd8dadecbe8c38ea3fc2), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "v46i", "LZ v4.6 English, Spanish, Italian")
	ROMX_LOAD("46-lzi.rom",   0x0000, 0x10000, CRC(ee50b6e9) SHA1(f1821ad179b018d6b22129ef4e8e8b23f452b2f6), ROM_BIOS(6))

	ROM_REGION( 0x1000, "hd44780", 0 )
	ROM_LOAD("psion_lz_charset.bin", 0x0000, 0x1000, BAD_DUMP CRC(44bff6f6) SHA1(aef544548b783d608a7d55456f6c46f421a11ed7))
ROM_END

ROM_START( psionlz64 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS( "v46a" )
	ROM_SYSTEM_BIOS(0, "v42", "LZ64 v4.2 English, French, German")
	ROMX_LOAD("42-lz64.rom",  0x0000, 0x10000, CRC(2df76f5a) SHA1(4255bc2abfc6f6cb666c0464a86d4ac98050268e), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v43", "LZ64 v4.3 English, French, German")
	ROMX_LOAD("43-lz64.rom",  0x0000, 0x10000, CRC(57e7a372) SHA1(46c2da1cfe991c0c1f2486e4aa28388767937ddd), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v44", "LZ64 v4.4 English, French, German")
	ROMX_LOAD("44-lz64.rom",  0x0000, 0x10000, CRC(aa487913) SHA1(5a44390f63fc8c1bc94299ab2eb291bc3a5b989a), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v45", "LZ64 v4.5 English, French, German")
	ROMX_LOAD("45-lz64.rom",  0x0000, 0x10000, CRC(4fbd5d88) SHA1(43f97549d2060840aa6313d526000530f384a08f), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v46a", "LZ64 v4.6a English, French, German")
	ROMX_LOAD("46a-lz64.rom", 0x0000, 0x10000, CRC(9b0d5a7a) SHA1(f1cdd6ef43cd65ef18e148deca0500f0c1ad2f80), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "v46b", "LZ64 v4.6b English, French, German")
	ROMX_LOAD("46b-lz64.rom", 0x0000, 0x10000, CRC(8d1101e2) SHA1(eddd0c3a2881667a1485b0d66f82f8c7792995c2), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "v46i", "LZ64 v4.6 English, Spanish, Italian")
	ROMX_LOAD("46-lz64i.rom", 0x0000, 0x10000, CRC(c96c7e65) SHA1(1b4af43657bbd3ecd92f370762bde166047b85e2), ROM_BIOS(6))
	ROM_SYSTEM_BIOS(7, "v46s", "LZ64 v4.6 English, Swedish, Danish")
	ROMX_LOAD("46-lz64s.rom", 0x0000, 0x10000, CRC(328d9772) SHA1(7f9e2d591d59ecfb0822d7067c2fe59542ea16dd), ROM_BIOS(7))

	ROM_REGION( 0x1000, "hd44780", 0 )
	ROM_LOAD("psion_lz_charset.bin", 0x0000, 0x1000, BAD_DUMP CRC(44bff6f6) SHA1(aef544548b783d608a7d55456f6c46f421a11ed7))
ROM_END

ROM_START( psionp432 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v46", "POS432 v4.6")
	ROMX_LOAD("46-p432n.rom",  0x0000, 0x10000, CRC(6683737a) SHA1(2c6bd9938f1b1d762adc8d45802c24f007e3445a), ROM_BIOS(0))

	ROM_REGION( 0x1000, "hd44780", 0 )
	ROM_LOAD("psion_lz_charset.bin", 0x0000, 0x1000, BAD_DUMP CRC(44bff6f6) SHA1(aef544548b783d608a7d55456f6c46f421a11ed7))
ROM_END

ROM_START( psionp464 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v46", "POS464 v4.6")
	ROMX_LOAD("46-p464.rom",  0x0000, 0x10000, CRC(672a0945) SHA1(d2a6e3fe1019d1bd7ae4725e33a0b9973f8cd7d8), ROM_BIOS(0))

	ROM_REGION( 0x1000, "hd44780", 0 )
	ROM_LOAD("psion_lz_charset.bin", 0x0000, 0x1000, BAD_DUMP CRC(44bff6f6) SHA1(aef544548b783d608a7d55456f6c46f421a11ed7))
ROM_END

} // anonymous namespace


//    YEAR  NAME        PARENT   COMPAT  MACHINE    INPUT    CLASS         INIT        COMPANY  FULLNAME                         FLAGS
COMP( 1984, psion1,     0,       0,      psion1,    psion1,  psion1_state, empty_init, "Psion", "Organiser I",                   MACHINE_NOT_WORKING )
COMP( 1986, psioncm,    0,       0,      psioncm,   psion2,  psion2_state, empty_init, "Psion", "Organiser II CM",               MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1988, psioncmm,   psioncm, 0,      psioncm,   psion2,  psion2_state, empty_init, "Psion", "Organiser II CM Multilingual",  MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1986, psionxp,    psioncm, 0,      psionxp,   psion2,  psion2_state, empty_init, "Psion", "Organiser II XP",               MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1986, psionla,    psioncm, 0,      psionla,   psion2,  psion2_state, empty_init, "Psion", "Organiser II LA",               MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1988, psionlam,   psioncm, 0,      psionla,   psion2,  psion2_state, empty_init, "Psion", "Organiser II LA Multilingual",  MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1987, psionp200,  psioncm, 0,      psionp200, psion2n, psion2_state, empty_init, "Psion", "Organiser II POS 200",          MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1987, psionp200a, psioncm, 0,      psionp200, psion2a, psion2_state, empty_init, "Psion", "Organiser II Alpha POS 200",    MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1988, psionp250,  psioncm, 0,      psionp200, psion2,  psion2_state, empty_init, "Psion", "Organiser II P 250",            MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1988, psionp350,  psioncm, 0,      psionp350, psion2,  psion2_state, empty_init, "Psion", "Organiser II P 350",            MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1989, psionlz,    0,       0,      psionlz,   psion2,  psion2_state, empty_init, "Psion", "Organiser II LZ",               MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1989, psionlz64,  psionlz, 0,      psionlz64, psion2,  psion2_state, empty_init, "Psion", "Organiser II LZ64",             MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1989, psionp432,  psionlz, 0,      psionp432, psion2n, psion2_state, empty_init, "Psion", "Organiser II P 432",            MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1989, psionp464,  psionlz, 0,      psionp464, psion2,  psion2_state, empty_init, "Psion", "Organiser II P 464",            MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
