// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Psion Organiser II series

        Driver by Sandro Ronco

        TODO:
        - dump CGROM of the HD44780
        - emulate other devices in slot C(Comms Link, printer)

        Note:
        - 4 lines display has an custom LCD controller derived from an HD66780
        - NVRAM works only if the machine is turned off (with OFF menu) before closing MAME
        - psion1 goes into standby right after a cold boot, so press the ON button

        More info:
            http://archive.psion2.org/org2/techref/index.htm

****************************************************************************/


#include "emu.h"
#include "psion.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


TIMER_DEVICE_CALLBACK_MEMBER(psion_state::nmi_timer)
{
	if (m_enable_nmi)
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

void psion_state::update_banks()
{
	if (m_ram_bank < m_ram_bank_count && m_ram_bank_count)
		membank("rambank")->set_entry(m_ram_bank);

	if (m_rom_bank < m_rom_bank_count && m_rom_bank_count)
		membank("rombank")->set_entry(m_rom_bank);
}

void psion_state::port2_w(offs_t offset, uint8_t data, uint8_t ddr)
{
	/* datapack i/o data bus */
	m_pack1->data_w(data & ddr);
	m_pack2->data_w(data & ddr);
}

uint8_t psion_state::port2_r()
{
	/* datapack i/o data bus */
	return m_pack1->data_r() | m_pack2->data_r();
}

uint8_t psion_state::port5_r()
{
	/*
	x--- ---- ON key active high
	-xxx xx-- keys matrix active low
	---- --x- pulse
	---- ---x battery status
	*/
	return kb_read() | ioport("BATTERY")->read() | ioport("ON")->read() | (m_kb_counter == 0x7ff)<<1 | m_pulse<<1;
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
	m_pack1->control_w((data & 0x8f) | (data & 0x10));
	m_pack2->control_w((data & 0x8f) | ((data & 0x20) >> 1));
}

uint8_t psion_state::port6_r()
{
	/* datapack control lines */
	return (m_pack1->control_r() | (m_pack2->control_r() & 0x8f)) | ((m_pack2->control_r() & 0x10)<<1);
}

/* Read/Write common */
void psion_state::io_rw(uint16_t offset)
{
	if (machine().side_effects_disabled())
		return;

	switch (offset & 0xffc0)
	{
	case 0xc0:
		/* switch off, CPU goes into standby mode */
		m_enable_nmi = false;
		m_maincpu->set_input_line(M6801_STBY_LINE, ASSERT_LINE);
		break;
	case 0x100:
		m_pulse = 1;
		break;
	case 0x140:
		m_pulse = 0;
		break;
	case 0x200:
		m_kb_counter = 0;
		break;
	case 0x180:
		m_beep->set_state(1);
		break;
	case 0x1c0:
		m_beep->set_state(0);
		break;
	case 0x240:
		if (offset == 0x260 && (m_rom_bank_count || m_ram_bank_count))
		{
			m_ram_bank=0;
			m_rom_bank=0;
			update_banks();
		}
		else
			m_kb_counter++;
		break;
	case 0x280:
		if (offset == 0x2a0 && m_ram_bank_count)
		{
			m_ram_bank++;
			update_banks();
		}
		else
			m_enable_nmi = true;
		break;
	case 0x2c0:
		if (offset == 0x2e0 && m_rom_bank_count)
		{
			m_rom_bank++;
			update_banks();
		}
		else
			m_enable_nmi = false;
		break;
	}
}

void psion_state::io_w(offs_t offset, uint8_t data)
{
	switch (offset & 0x0ffc0)
	{
	case 0x80:
		m_lcdc->write(offset & 0x01, data);
		break;
	default:
		io_rw(offset);
		break;
	}
}

uint8_t psion_state::io_r(offs_t offset)
{
	switch (offset & 0xffc0)
	{
	case 0x80:
		return m_lcdc->read(offset & 0x01);
	default:
		if (!machine().side_effects_disabled())
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

void psion_state::psioncm_mem(address_map &map)
{
	map.unmap_value_low();
	map(0x0100, 0x03ff).rw(FUNC(psion_state::io_r), FUNC(psion_state::io_w));
	map(0x2000, 0x3fff).ram().share("ram");
	map(0x8000, 0xffff).rom();
}

void psion_state::psionla_mem(address_map &map)
{
	map.unmap_value_low();
	map(0x0100, 0x03ff).rw(FUNC(psion_state::io_r), FUNC(psion_state::io_w));
	map(0x0400, 0x5fff).ram().share("ram");
	map(0x8000, 0xffff).rom();
}

void psion_state::psionp350_mem(address_map &map)
{
	map.unmap_value_low();
	map(0x0100, 0x03ff).rw(FUNC(psion_state::io_r), FUNC(psion_state::io_w));
	map(0x0400, 0x3fff).ram().share("ram");
	map(0x4000, 0x7fff).bankrw("rambank");
	map(0x8000, 0xffff).rom();
}

void psion_state::psionlam_mem(address_map &map)
{
	map.unmap_value_low();
	map(0x0100, 0x03ff).rw(FUNC(psion_state::io_r), FUNC(psion_state::io_w));
	map(0x0400, 0x7fff).ram().share("ram");
	map(0x8000, 0xbfff).bankr("rombank");
	map(0xc000, 0xffff).rom();
}

void psion_state::psionlz_mem(address_map &map)
{
	map.unmap_value_low();
	map(0x0100, 0x03ff).rw(FUNC(psion_state::io_r), FUNC(psion_state::io_w));
	map(0x0400, 0x3fff).ram().share("ram");
	map(0x4000, 0x7fff).bankrw("rambank");
	map(0x8000, 0xbfff).bankr("rombank");
	map(0xc000, 0xffff).rom();
}

/* Input ports */
INPUT_PORTS_START( psion )
	PORT_START("BATTERY")
		PORT_CONFNAME( 0x01, 0x00, "Battery Status" )
		PORT_CONFSETTING( 0x00, DEF_STR( Normal ) )
		PORT_CONFSETTING( 0x01, "Low Battery" )

	PORT_START("ON")
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ON/CLEAR") PORT_CODE(KEYCODE_MINUS)  PORT_CHANGED_MEMBER(DEVICE_SELF, psion_state, psion_on, 0)

	PORT_START("K1")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MODE") PORT_CODE(KEYCODE_EQUALS)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up [CAP]") PORT_CODE(KEYCODE_UP)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down [NUM]") PORT_CODE(KEYCODE_DOWN)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT)

	PORT_START("K2")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)
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
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
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

INPUT_PORTS_START( psion1 )
	PORT_START("BATTERY")
	PORT_CONFNAME( 0x01, 0x00, "Battery Status" )
	PORT_CONFSETTING( 0x00, DEF_STR( Normal ) )
	PORT_CONFSETTING( 0x01, "Low Battery" )

	PORT_START("ON")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ON/CLEAR") PORT_CODE(KEYCODE_MINUS)  PORT_CHANGED_MEMBER(DEVICE_SELF, psion_state, psion_on, 0)

	PORT_START("K1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down [NUM]") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up [CAP]") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MODE") PORT_CODE(KEYCODE_EQUALS)

	PORT_START("K2")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S [;]") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M [,]") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G [=]") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A [<]") PORT_CODE(KEYCODE_A)

	PORT_START("K3")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y [0]") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T [:]") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N [$]") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H [\"]") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B [>]") PORT_CODE(KEYCODE_B)

	PORT_START("K4")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z [.]") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U [1]") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O [4]") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I [7]") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C [(]") PORT_CODE(KEYCODE_C)

	PORT_START("K5")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W [3]") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q [6]") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K [9]") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E [%]") PORT_CODE(KEYCODE_E)

	PORT_START("K6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EXE") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X [+]") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R [-]") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L [*]") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F [/]") PORT_CODE(KEYCODE_F)

	PORT_START("K7")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V [2]") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P [5]") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J [8]") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D [)]") PORT_CODE(KEYCODE_D)
INPUT_PORTS_END


void psion_state::machine_start()
{
	if (!strcmp(machine().system().name, "psionlam"))
	{
		m_rom_bank_count = 3;
		m_ram_bank_count = 0;
	}
	else if (!strncmp(machine().system().name, "psionlz", 7))
	{
		m_rom_bank_count = 3;
		m_ram_bank_count = 3;
	}
	else if (!strcmp(machine().system().name, "psionp464"))
	{
		m_rom_bank_count = 3;
		m_ram_bank_count = 9;
	}
	else if (!strncmp(machine().system().name, "psionp", 6))
	{
		m_rom_bank_count = 0;
		m_ram_bank_count = 5;
	}
	else
	{
		m_rom_bank_count = 0;
		m_ram_bank_count = 0;
	}

	if (m_rom_bank_count)
	{
		uint8_t* rom_base = (uint8_t *)memregion("maincpu")->base();

		membank("rombank")->configure_entry(0, rom_base + 0x8000);
		membank("rombank")->configure_entries(1, m_rom_bank_count-1, rom_base + 0x10000, 0x4000);
		membank("rombank")->set_entry(0);
	}

	if (m_ram_bank_count)
	{
		m_paged_ram = std::make_unique<uint8_t[]>(m_ram_bank_count * 0x4000);
		memset(m_paged_ram.get(), 0, sizeof(uint8_t) * (m_ram_bank_count * 0x4000));
		membank("rambank")->configure_entries(0, m_ram_bank_count, m_paged_ram.get(), 0x4000);
		membank("rambank")->set_entry(0);
	}

	m_nvram1->set_base(m_ram, m_ram.bytes());
	if (m_nvram2)
		m_nvram2->set_base(m_paged_ram.get(), m_ram_bank_count * 0x4000);

	save_item(NAME(m_kb_counter));
	save_item(NAME(m_enable_nmi));
	save_item(NAME(m_pulse));
	save_item(NAME(m_rom_bank));
	save_item(NAME(m_ram_bank));
	save_pointer(NAME(m_paged_ram), m_ram_bank_count * 0x4000);
}

void psion_state::machine_reset()
{
	m_enable_nmi = false;
	m_kb_counter = 0;
	m_ram_bank = 0;
	m_rom_bank = 0;
	m_pulse = 0;

	if (m_rom_bank_count || m_ram_bank_count)
		update_banks();
}

void psion1_state::machine_reset()
{
	psion_state::machine_reset();
	m_enable_nmi = true;
}

HD44780_PIXEL_UPDATE(psion_state::lz_pixel_update)
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

/* basic configuration for 2 lines display */
void psion_state::psion_2lines(machine_config &config)
{
	/* basic machine hardware */
	HD6303X(config, m_maincpu, 3.6864_MHz_XTAL); // internal operating frequency is 0.9216 MHz
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->in_p2_cb().set(FUNC(psion_state::port2_r));
	m_maincpu->out_p2_cb().set(FUNC(psion_state::port2_w));
	m_maincpu->in_p5_cb().set(FUNC(psion_state::port5_r));
	m_maincpu->in_p6_cb().set(FUNC(psion_state::port6_r));
	m_maincpu->out_p6_cb().set(FUNC(psion_state::port6_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_size(6*16, 9*2);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(psion_state::psion_palette), 2);
	GFXDECODE(config, "gfxdecode", "palette", gfx_psion);

	HD44780(config, m_lcdc, 270'000); // TODO: Wrong device? Custom? clock not measured, datasheet typical clock used
	m_lcdc->set_lcd_size(2, 16);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 3250).add_route(ALL_OUTPUTS, "mono", 1.00);

	NVRAM(config, "nvram1", nvram_device::DEFAULT_ALL_0); // RAM

	TIMER(config, "nmi_timer").configure_periodic(FUNC(psion_state::nmi_timer), attotime::from_seconds(1));

	/* Datapack */
	PSION_DATAPACK(config, m_pack1, 0);
	PSION_DATAPACK(config, m_pack2, 0);

	/* Software lists */
	SOFTWARE_LIST(config, "pack_list").set_original("psion2");
}

/* basic configuration for 4 lines display */
void psion_state::psion_4lines(machine_config &config)
{
	psion_2lines(config);

	/* video hardware */
	subdevice<screen_device>("screen")->set_size(6*20, 9*4);
	subdevice<screen_device>("screen")->set_visarea_full();

	m_lcdc->set_lcd_size(4, 20);
	m_lcdc->set_pixel_update_cb(FUNC(psion_state::lz_pixel_update));
}

void psion1_state::psion1(machine_config &config)
{
	psion_2lines(config);

	HD6301X0(config.replace(), m_maincpu, 3.6864_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &psion1_state::psion1_mem);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->in_p2_cb().set(FUNC(psion1_state::port2_r));
	m_maincpu->out_p2_cb().set(FUNC(psion1_state::port2_w));
	m_maincpu->in_p5_cb().set(FUNC(psion1_state::port5_r));
	m_maincpu->in_p6_cb().set(FUNC(psion1_state::port6_r));
	m_maincpu->out_p6_cb().set(FUNC(psion1_state::port6_w));

	timer_device &nmi_timer(TIMER(config.replace(), "nmi_timer"));
	nmi_timer.configure_periodic(FUNC(psion1_state::nmi_timer), attotime::from_msec(500));
	nmi_timer.set_start_delay(attotime::from_seconds(1));

	subdevice<screen_device>("screen")->set_size(6*16, 1*8);
	subdevice<screen_device>("screen")->set_visarea_full();

	m_lcdc->set_lcd_size(1, 16);
	m_lcdc->set_pixel_update_cb(FUNC(psion1_state::psion1_pixel_update));

	/* Software lists */
	SOFTWARE_LIST(config.replace(), "pack_list").set_original("psion1");
}

void psion_state::psioncm(machine_config &config)
{
	psion_2lines(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &psion_state::psioncm_mem);
}

void psion_state::psionla(machine_config &config)
{
	psion_2lines(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &psion_state::psionla_mem);
}

void psion_state::psionlam(machine_config &config)
{
	psion_2lines(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &psion_state::psionlam_mem);
}

void psion_state::psionp350(machine_config &config)
{
	psion_2lines(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &psion_state::psionp350_mem);

	NVRAM(config, "nvram2", nvram_device::DEFAULT_ALL_0); // paged RAM
}

void psion_state::psionlz(machine_config &config)
{
	psion_4lines(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &psion_state::psionlz_mem);

	NVRAM(config, "nvram2", nvram_device::DEFAULT_ALL_0); // paged RAM
}

/* ROM definition */

ROM_START( psion1 )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v1", "Organiser I")
	ROMX_LOAD( "psion1.rom",  0x0000, 0x1000, CRC(7e2609c1) SHA1(a3320ea8ac3ab9e0039ee16f7c571731adde5869), ROM_BIOS(0))
ROM_END

ROM_START( psioncm )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v24", "CM v2.4")
	ROMX_LOAD("24-cm.dat",    0x8000, 0x8000,  CRC(f6798394) SHA1(736997f0db9a9ee50d6785636bdc3f8ff1c33c66), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v26", "CM v2.6")
	ROMX_LOAD("26-cm.rom",    0x8000, 0x8000,  CRC(21b7c94c) SHA1(e0a3168c96a3f0b37b8698e86574e40597fe3c62), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v33", "CM v3.3")
	ROMX_LOAD("33-cm.rom",    0x8000, 0x8000,  CRC(5c10b167) SHA1(6deea00fe648bddae1d61a22858023bc80277ea0), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v33f","CM v3.3 French")
	ROMX_LOAD("33-cmf.rom",   0x8000, 0x8000,  CRC(4d626ce2) SHA1(82b96f11a0abfc1931b6022b84733d975ad7ab2b), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v36f","CM v3.6 French")
	ROMX_LOAD("36-cmf.rom",   0x8000, 0x8000,  CRC(beabe0f5) SHA1(a5ef3bb92190a257cb0e94d58b2c23935436edeb), ROM_BIOS(4))
ROM_END

ROM_START( psionxp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v26", "XP v2.6")
	ROMX_LOAD( "26-xp.rom",    0x8000, 0x8000,  CRC(a81db40f) SHA1(af72d94ccee1fa1dade8776bdbd39920665a68b7), ROM_BIOS(0) )
ROM_END

ROM_START( psionla )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v33", "LA v3.3")
	ROMX_LOAD("33-la.dat",    0x8000, 0x8000,  CRC(02668ed4) SHA1(e5d4ee6b1cde310a2970ffcc6f29a0ce09b08c46), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v34g", "LA v3.4 German")
	ROMX_LOAD("34-lag.rom",   0x8000, 0x8000,  CRC(13a92c4b) SHA1(dab8bd6a41a5fd509c5ad4b0b0ab80d14f2c421a), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v36", "LA v3.6")
	ROMX_LOAD("36-la.rom",    0x8000, 0x8000,  CRC(7442c7f6) SHA1(94f15bd06bd750be70fa4a4ab588237c5a703f65), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v30", "LA v3.0")
	ROMX_LOAD("30-lahp.rom",    0x8000, 0x8000,  CRC(50192528) SHA1(c556d53f70bf5ecae756b2ebfc6d954912316bbe), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v36f", "LA v3.6 French")
	ROMX_LOAD("36-laf.rom",    0x8000, 0x8000, CRC(036ef00e) SHA1(98f303273e570e94a1e25a58cf1ffcec0db32165), ROM_BIOS(4))
ROM_END

ROM_START( psionp200 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v33", "POS200a v3.3")
	ROMX_LOAD("33-p200a.rom", 0x8000, 0x8000, CRC(91e94998) SHA1(e9e8106eb9283d20452697859894aa407cc07bd1), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v36", "POS200 v3.6")
	ROMX_LOAD("36-p200.rom",  0x8000, 0x8000, CRC(4569ef5b) SHA1(8c275474cc6e3f50156f0b6e32121cadd14ea8be), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v36a", "POS200a v3.6")
	ROMX_LOAD("36-p200a.rom", 0x8000, 0x8000, CRC(36cceeb7) SHA1(57069812c5a16babfff91dc7d7e0842e5dc68652), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v36b", "POS250 v3.6")
	ROMX_LOAD("36-p250.rom",  0x8000, 0x8000, CRC(235cc76a) SHA1(3229cdff4b049a1fbf9a758ce3abf3fdc9b547c9), ROM_BIOS(3))
ROM_END

ROM_START( psionp350 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v36", "POS350 v3.6")
	ROMX_LOAD("36-p350.dat",  0x8000, 0x8000,  CRC(3a371a74) SHA1(9167210b2c0c3bd196afc08ca44ab23e4e62635e), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v38", "POS350 v3.8")
	ROMX_LOAD("38-p350.dat",  0x8000, 0x8000,  CRC(1b8b082f) SHA1(a3e875a59860e344f304a831148a7980f28eaa4a), ROM_BIOS(1))
ROM_END

ROM_START( psionlam )
	ROM_REGION( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v37", "LA v3.7")
	ROMX_LOAD("37-lam.dat",   0x8000, 0x10000, CRC(7ee3a1bc) SHA1(c7fbd6c8e47c9b7d5f636e9f56e911b363d6796b), ROM_BIOS(0))
ROM_END

ROM_START( psionlz64 )
	ROM_REGION( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v44", "LZ64 v4.4")
	ROMX_LOAD("44-lz64.dat",  0x8000, 0x10000, CRC(aa487913) SHA1(5a44390f63fc8c1bc94299ab2eb291bc3a5b989a), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v46si", "LZ64 v4.6 Spanish / Italian")
	ROMX_LOAD("46-lz64i.rom", 0x8000, 0x10000, CRC(c96c7e65) SHA1(1b4af43657bbd3ecd92f370762bde166047b85e2), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v43", "LZ64 v4.3")
	ROMX_LOAD("43-lz64.rom",  0x8000, 0x10000, CRC(57e7a372) SHA1(46c2da1cfe991c0c1f2486e4aa28388767937ddd), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v46a", "LZ64 v4.6a")
	ROMX_LOAD("46a-lz64.rom", 0x8000, 0x10000, CRC(9b0d5a7a) SHA1(f1cdd6ef43cd65ef18e148deca0500f0c1ad2f80), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v46b", "LZ64 v4.6b")
	ROMX_LOAD("46b-lz64.rom", 0x8000, 0x10000, CRC(8d1101e2) SHA1(eddd0c3a2881667a1485b0d66f82f8c7792995c2), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "v45", "LZ64 v4.5")
	ROMX_LOAD("45-lz64.rom",  0x8000, 0x10000, CRC(4fbd5d88) SHA1(43f97549d2060840aa6313d526000530f384a08f), ROM_BIOS(5))

	ROM_REGION( 0x1000, "hd44780", 0 )
	ROM_LOAD("psion_lz_charset.bin",    0x0000, 0x1000,  BAD_DUMP CRC(44bff6f6) SHA1(aef544548b783d608a7d55456f6c46f421a11ed7))
ROM_END

ROM_START( psionlz64s )
	ROM_REGION( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v46", "LZ64 v4.6")
	ROMX_LOAD("46-lz64s.dat", 0x8000, 0x10000, CRC(328d9772) SHA1(7f9e2d591d59ecfb0822d7067c2fe59542ea16dd), ROM_BIOS(0))

	ROM_REGION( 0x1000, "hd44780", 0 )
	ROM_LOAD("psion_lz_charset.bin",    0x0000, 0x1000,  BAD_DUMP CRC(44bff6f6) SHA1(aef544548b783d608a7d55456f6c46f421a11ed7))
ROM_END

ROM_START( psionlz )
	ROM_REGION( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v46", "LZ v4.6")
	ROMX_LOAD("46-lz.dat",    0x8000, 0x10000, CRC(22715f48) SHA1(cf460c81cadb53eddb7afd8dadecbe8c38ea3fc2), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v42", "LZ v4.2")
	ROMX_LOAD("42-lz.rom",    0x8000, 0x10000, CRC(f2d6ad47) SHA1(ee8315ae872463068d805c6e0b71f62ae8eb65be), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v44", "LZ v4.4")
	ROMX_LOAD("44-lz.rom",    0x8000, 0x10000, CRC(4a0a990b) SHA1(dde0ba69a4a7f02b610ad6bd69a8b8552b060223), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v45", "LZ v4.5")
	ROMX_LOAD("45-lz.rom",    0x8000, 0x10000, CRC(f95d8f39) SHA1(cb64152c2418bf730c89999d1b13c1d1ada1f082), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v45s", "LZ v4.5S")
	ROMX_LOAD("45-lzs.rom",   0x8000, 0x10000, CRC(2d082d7f) SHA1(fcd00864a0cc617e61997240945ea70a8e9fa211), ROM_BIOS(4))

	ROM_REGION( 0x1000, "hd44780", 0 )
	ROM_LOAD("psion_lz_charset.bin",    0x0000, 0x1000,  BAD_DUMP CRC(44bff6f6) SHA1(aef544548b783d608a7d55456f6c46f421a11ed7))
ROM_END

ROM_START( psionp464 )
	ROM_REGION( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v46", "POS464 v4.6")
	ROMX_LOAD( "46-p464.dat",  0x8000, 0x10000, CRC(672a0945) SHA1(d2a6e3fe1019d1bd7ae4725e33a0b9973f8cd7d8), ROM_BIOS(0))

	ROM_REGION( 0x1000, "hd44780", 0 )
	ROM_LOAD( "psion_lz_charset.bin",    0x0000, 0x1000,  BAD_DUMP CRC(44bff6f6) SHA1(aef544548b783d608a7d55456f6c46f421a11ed7))
ROM_END

/* Driver */

//    YEAR  NAME        PARENT   COMPAT  MACHINE    INPUT   CLASS         INIT        COMPANY  FULLNAME              FLAGS
COMP( 1984, psion1,     0,       0,      psion1,    psion1, psion1_state, empty_init, "Psion", "Organiser I",        MACHINE_NOT_WORKING )
COMP( 1986, psioncm,    0,       0,      psioncm,   psion,  psion_state,  empty_init, "Psion", "Organiser II CM",    MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1986, psionla,    psioncm, 0,      psionla,   psion,  psion_state,  empty_init, "Psion", "Organiser II LA",    MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1986, psionxp,    psioncm, 0,      psionla,   psion,  psion_state,  empty_init, "Psion", "Organiser II XP",    MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1986, psionp200,  psioncm, 0,      psionp350, psion,  psion_state,  empty_init, "Psion", "Organiser II P200",  MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1986, psionp350,  psioncm, 0,      psionp350, psion,  psion_state,  empty_init, "Psion", "Organiser II P350",  MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1986, psionlam,   psioncm, 0,      psionlam,  psion,  psion_state,  empty_init, "Psion", "Organiser II LAM",   MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1989, psionlz,    0,       0,      psionlz,   psion,  psion_state,  empty_init, "Psion", "Organiser II LZ",    MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1989, psionlz64,  psionlz, 0,      psionlz,   psion,  psion_state,  empty_init, "Psion", "Organiser II LZ64",  MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1989, psionlz64s, psionlz, 0,      psionlz,   psion,  psion_state,  empty_init, "Psion", "Organiser II LZ64S", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1989, psionp464,  psionlz, 0,      psionlz,   psion,  psion_state,  empty_init, "Psion", "Organiser II P464",  MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
