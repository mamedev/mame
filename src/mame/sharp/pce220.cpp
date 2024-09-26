// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Sandro Ronco
/***************************************************************************

    Sharp PC-E220

    preliminary driver by Angelo Salese
    improvements by Sandro Ronco

    Notes:
    - NVRAM works only if the machine is turned off (with OFF key) before closing MAME
    - Holding SHIFT + COMMA on boot loads the Test Menu

    TODO:
    - 11 pin interface for extensions (printer, cassette).
    - LCD contrast.
    - Add other models that have a similar hardware.

    More info:
      http://wwwhomes.uni-bielefeld.de/achim/pc-e220.html
      http://www.akiyan.com/pc-g850_technical_data (in Japanese)

****************************************************************************/

#include "emu.h"

#include "pce220_ser.h"

#include "cpu/z80/z80.h"
#include "machine/input_merger.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "video/hd61202.h"
#include "video/sed1520.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "pce220.lh"
#include "pcg850v.lh"

namespace {

class pce220_state : public driver_device
{
public:
	pce220_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_ram(*this, RAM_TAG)
		, m_banks(*this, "bank%u", 1)
		, m_beep(*this, "beeper")
		, m_lcdc(*this, "hd61202")
		, m_input_merger(*this, "input_merger")
		, m_serial(*this, "serial")
		, m_keyboard(*this, "LINE%u", 0)
		, m_io_on(*this, "ON")
		, m_battery(*this, "BATTERY")
		, m_lcd_symbols(*this, "sym.%u", 0U)
	{ }

	void pce220(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(kb_irq);
	DECLARE_INPUT_CHANGED_MEMBER(on_irq);

protected:

	// Interrupt flags
	enum : uint8_t
	{
		IRQ_FLAG_KEY   = 0x01,
		IRQ_FLAG_ON    = 0x02,
		IRQ_FLAG_TIMER = 0x04,
	};

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<ram_device> m_ram;
	required_memory_bank_array<4> m_banks;
	required_device<beep_device> m_beep;
	optional_device<hd61202_device> m_lcdc;
	required_device<input_merger_device> m_input_merger;
	required_device<pce220_serial_device> m_serial;

	required_ioport_array<10> m_keyboard;
	required_ioport m_io_on;
	required_ioport m_battery;

	output_finder<18> m_lcd_symbols;

	//basic machine
	uint8_t m_bank_num;
	uint8_t m_irq_mask;
	uint8_t m_irq_flag;
	uint8_t m_timer_status;
	uint16_t m_kb_matrix;
	uint8_t m_power_state;

	uint8_t m_port15;
	uint8_t m_port18;
	uint8_t m_battery_sel;

	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(pce220_timer_callback);

	void pce220_palette(palette_device &palette) const;

	HD61202_UPDATE_CB(hd61202_update);
	uint8_t rom_bank_r();
	void rom_bank_w(uint8_t data);
	void ram_bank_w(uint8_t data);
	uint8_t timer_r();
	void timer_w(uint8_t data);
	void boot_bank_w(uint8_t data);
	uint8_t port15_r();
	void port15_w(uint8_t data);
	uint8_t port18_r();
	void port18_w(uint8_t data);
	void battery_w(uint8_t data);
	uint8_t battery_r();
	uint8_t port1f_r();
	void kb_matrix_w(offs_t offset, uint8_t data);
	uint8_t kb_r();
	uint8_t irq_status_r();
	void irq_ack_w(uint8_t data);
	void irq_mask_w(uint8_t data);
	void install_bootrom();

	void pce220_io_common(address_map &map) ATTR_COLD;
	void pce220_io(address_map &map) ATTR_COLD;
	void pce220_mem(address_map &map) ATTR_COLD;
};


class pcg815_state : public pce220_state
{
public:
	pcg815_state(const machine_config &mconfig, device_type type, const char *tag)
		: pce220_state(mconfig, type, tag)
		, m_lcdc2(*this, "hd61202_2")
		{ }

	void pcg815(machine_config &config);

private:
	required_device<hd61202_device> m_lcdc2;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void lcdc_control_w(uint8_t data);
	void lcdc_data_w(uint8_t data);
	HD61202_UPDATE_CB(hd61202_1_update);
	HD61202_UPDATE_CB(hd61202_2_update);
	void pcg815_io(address_map &map) ATTR_COLD;
};


class pcg850v_state : public pce220_state
{
public:
	pcg850v_state(const machine_config &mconfig, device_type type, const char *tag)
		: pce220_state(mconfig, type, tag)
		{ }

	void pcg850v(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint8_t m_g850v_bank_num = 0;

	SED1560_UPDATE_CB(sed1560_update);
	uint8_t g850v_bank_r();
	void g850v_rom_bank_w(uint8_t data);
	void g850v_bank_w(uint8_t data);
	void pcg850v_io(address_map &map) ATTR_COLD;
};


HD61202_UPDATE_CB(pce220_state::hd61202_update)
{
	if (lcd_on)
	{
		bitmap.fill(2, cliprect);
		for (int y = 0; y < 4 * 8; y++)
		{
			if ((y & 7) == 7)
				continue;

			int ys = start_line + y;
			for (int x = 0; x < 12 * 5; x++)
			{
				int xdest = x + (x / 5);
				int bitpos = (ys & ~7) * 64 + x * 8 + (ys & 7);

				//first 12 columns
				bitmap.pix(y, xdest) = BIT(ddr[(bitpos / 8) & 0x1ff], bitpos & 7);

				//last 12 columns
				bitmap.pix(y, 142 - xdest) = BIT(ddr[(0x100 + bitpos / 8) & 0x1ff], bitpos & 7);
			}
		}
	}
	else
		bitmap.fill(0, cliprect);


	static const uint16_t lcd_symbol_bits[] =
	{
		//BUSY  CAPS  KANA  SHO   2ndF   DE     G     RAD    CONST   M     E     BATT   RUN    PRO    CASL   TEXT   STAT   PRINT
		480,    481,  482,  483,  484,  2534,  2533,  2532,  2531, 2530,  2529,  2528,  2022,  2021,  2020,  2019,  4069,  4070
	};

	for (int i=0; i<18; i++)
	{
		int bitpos = ((start_line & ~7) * 64 + lcd_symbol_bits[i]) + (start_line & 7);
		m_lcd_symbols[i] = !lcd_on ? 0 : BIT(ddr[(bitpos / 8) & 0x1ff], bitpos & 7);
	}

	return 0;
}

HD61202_UPDATE_CB(pcg815_state::hd61202_1_update)
{
	if (lcd_on)
	{
		for (int y = 0; y < 4 * 8; y++)
		{
			int ys = start_line + y;
			for (int x = 0; x < 2 * 6; x++)
			{
				int bitpos = (ys & ~7) * 64 + x * 8 + (ys & 7);
				bitmap.pix(y, 60 + x) = BIT(ddr[(bitpos / 8) & 0x1ff], bitpos & 7);
				bitmap.pix(y, 83 - x) = BIT(ddr[(0x100 + bitpos / 8) & 0x1ff], bitpos & 7);
			}
		}
	}

	static const uint16_t lcd_symbol_bits[] =
	{
		//BUSY  CAPS   KANA   SHO    2ndF    DE     G     RAD    CONST   M      E     BATT   RUN   PRO   CASL  TEXT  STAT  PRINT
		504,    2555,  2553,  2552,  2556,  3068,  3067,  3066,  2554,  3065,  1529,  3576,  505,  506,  507,  510,  509,  3064
	};

	for (int i=0; i<18; i++)
	{
		int bitpos = ((start_line & ~7) * 64 + lcd_symbol_bits[i]) + (start_line & 7);
		m_lcd_symbols[i] = !lcd_on ? 0 : BIT(ddr[(bitpos / 8) & 0x1ff], bitpos & 7);
	}

	return 0;
}


HD61202_UPDATE_CB(pcg815_state::hd61202_2_update)
{
	if (lcd_on)
	{
		for (int y = 0; y < 4 * 8; y++)
		{
			int ys = start_line + y;
			for (int x = 0; x < 10 * 6; x++)
			{
				int bitpos = (ys & ~7) * 64 + x * 8 + (ys & 7);
				bitmap.pix(y, x) = BIT(ddr[(bitpos / 8) & 0x1ff], bitpos & 7);
				bitmap.pix(y, 143 - x) = BIT(ddr[(0x100 + bitpos / 8) & 0x1ff], bitpos & 7);
			}
		}
	}

	return 0;
}


uint32_t pcg815_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_lcdc->screen_update(screen, bitmap, cliprect);
	m_lcdc2->screen_update(screen, bitmap, cliprect);
	return 0;
}


SED1560_UPDATE_CB(pcg850v_state::sed1560_update)
{
	if (lcd_on)
	{
		uint16_t color0 = reverse ? 1 : 0;
		uint16_t color1 = reverse ? 0 : 1;

		for (int y = 0; y < 6 * 8; y++)
		{
			int ys = start_line + y;
			for (int x = 0; x < 24 * 6; x++)
			{
				int bitpos = ((ys & ~7) * 166 + x * 8 + (ys & 7)) % (64 * 166);
				bitmap.pix(y, x) = BIT(dram[bitpos / 8], bitpos & 7) ? color1 : color0;
			}
		}
	}
	else
		bitmap.fill(0, cliprect);


	static const uint16_t lcd_symbol_bits[] =
	{
		//BUSY   BATT   RUN    PRO    TEXT   CASL   STAT   2ndF    M     CAPS   KANA   SHO     DE     G     RAD    CONST  PRINT
		7798,    7799,  1153,  1155,  1158,  2483,  3808,  3813,  3815,  5138,  5143,  6465,  6468,  6470,  7792,  7795,  7796,
	};

	for (int i=0; i<17; i++)
	{
		int bitpos = (((start_line & ~7) * 166 + lcd_symbol_bits[i]) + (start_line & 7)) % (64 * 166);
		m_lcd_symbols[i] = !lcd_on ? 0 : BIT(dram[bitpos / 8], bitpos & 7);
	}

	return 0;
}

void pcg815_state::lcdc_control_w(uint8_t data)
{
	m_lcdc->control_w(data);
	m_lcdc2->control_w(data);
}

void pcg815_state::lcdc_data_w(uint8_t data)
{
	m_lcdc->data_w(data);
	m_lcdc2->data_w(data);
}

uint8_t pce220_state::rom_bank_r()
{
	return m_bank_num;
}

void pce220_state::rom_bank_w(uint8_t data)
{
	m_bank_num = data;

	m_banks[2]->set_entry((data & 0x70) >> 4);  // bits 4,5,6
	m_banks[3]->set_entry(data & 0x0f);         // bits 0,1,2,3
}

void pcg850v_state::g850v_rom_bank_w(uint8_t data)
{
	pce220_state::rom_bank_w(data);
	m_g850v_bank_num = m_bank_num & 0x0f;
}

void pce220_state::ram_bank_w(uint8_t data)
{
	uint8_t bank = BIT(data,2);

	m_banks[0]->set_entry(bank);
	m_banks[1]->set_entry(bank);
}

uint8_t pce220_state::timer_r()
{
	return m_timer_status;
}

void pce220_state::timer_w(uint8_t data)
{
	m_input_merger->in_w<2>(CLEAR_LINE);
	m_timer_status = 0;
}

void pce220_state::boot_bank_w(uint8_t data)
{
	// set to 1 after boot for restore the ram in the first bank
	if (data & 0x01)
	{
		address_space &space_prg = m_maincpu->space(AS_PROGRAM);
		space_prg.install_readwrite_bank(0x0000, 0x3fff, m_banks[0]);
		m_banks[0]->set_entry(0);
	}
}

uint8_t pce220_state::port15_r()
{
	/*
	x--- ---- XIN input enabled
	---- ---0
	*/
	return m_port15;
}

void pce220_state::port15_w(uint8_t data)
{
	m_serial->enable_interface(BIT(data, 7));

	m_port15 = data;
}

uint8_t pce220_state::port18_r()
{
	/*
	x--- ---- XOUT/TXD
	---- --x- DOUT
	---- ---x BUSY/CTS
	*/

	return m_port18;
}

void pce220_state::port18_w(uint8_t data)
{
	m_beep->set_state(BIT(data, 7));

	m_serial->out_busy(BIT(data, 0));
	m_serial->out_dout(BIT(data, 1));
	m_serial->out_xout(BIT(data, 7));

	m_port18 = data;
}

void pce220_state::battery_w(uint8_t data)
{
	m_battery_sel = data;
}

uint8_t pce220_state::battery_r()
{
	if (m_battery_sel & 2)
		return BIT(m_battery->read(), 1);
	if (m_battery_sel & 1)
		return BIT(m_battery->read(), 0);

	return 0;
}

uint8_t pce220_state::port1f_r()
{
	/*
	x--- ---- ON - resp. break key status (?)
	---- -x-- XIN/RXD
	---- --x- ACK/RTS
	---- ---x DIN
	*/

	uint8_t data = 0;

	data |= m_serial->in_din()<<0;
	data |= m_serial->in_ack()<<1;
	data |= m_serial->in_xin()<<2;

	data |= m_io_on->read()<<7;

	return data;
}

void pce220_state::kb_matrix_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0:
		m_kb_matrix = (m_kb_matrix & 0x300) | data;
		break;
	case 1:
		m_kb_matrix = (m_kb_matrix & 0xff) | ((data&0x03)<<8);
		break;
	}
}

uint8_t pce220_state::kb_r()
{
	uint8_t data = 0x00;

	for (int i = 0; i < 10; i++)
		if (BIT(m_kb_matrix, i))
			data |= m_keyboard[i]->read();

	return data;
}

uint8_t pce220_state::irq_status_r()
{
	/*
	---- -x-- timer
	---- --x- ON-Key
	---- ---x keyboard
	*/
	return m_irq_flag;
}

void pce220_state::irq_ack_w(uint8_t data)
{
	m_irq_flag &= ~data;
}

void pce220_state::irq_mask_w(uint8_t data)
{
	m_irq_mask = data;
}

uint8_t pcg850v_state::g850v_bank_r()
{
	return m_g850v_bank_num;
}

void pcg850v_state::g850v_bank_w(uint8_t data)
{
	address_space &space_prg = m_maincpu->space(AS_PROGRAM);

	if (data < 0x16)
	{
		space_prg.install_read_bank(0xc000, 0xffff, m_banks[3]);
		m_banks[3]->set_entry(data);
	}
	else
	{
		space_prg.unmap_read(0xc000, 0xffff);
	}

	m_g850v_bank_num = data;

	m_bank_num = (m_bank_num & 0xf0) | (m_g850v_bank_num & 0x0f);
}

void pce220_state::pce220_mem(address_map &map)
{
	map(0x0000, 0x3fff).bankrw("bank1");
	map(0x4000, 0x7fff).bankrw("bank2");
	map(0x8000, 0xbfff).bankr("bank3");
	map(0xc000, 0xffff).bankr("bank4");
}

void pce220_state::pce220_io_common(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x10, 0x10).r(FUNC(pce220_state::kb_r));
	map(0x11, 0x12).w(FUNC(pce220_state::kb_matrix_w));
	map(0x13, 0x13).portr("SHIFT");
	map(0x14, 0x14).rw(FUNC(pce220_state::timer_r), FUNC(pce220_state::timer_w));
	map(0x15, 0x15).rw(FUNC(pce220_state::port15_r), FUNC(pce220_state::port15_w));
	map(0x16, 0x16).rw(FUNC(pce220_state::irq_status_r), FUNC(pce220_state::irq_ack_w));
	map(0x17, 0x17).w(FUNC(pce220_state::irq_mask_w));
	map(0x18, 0x18).rw(FUNC(pce220_state::port18_r), FUNC(pce220_state::port18_w));
	map(0x19, 0x19).rw(FUNC(pce220_state::rom_bank_r), FUNC(pce220_state::rom_bank_w));
	map(0x1a, 0x1a).w(FUNC(pce220_state::boot_bank_w));
	map(0x1b, 0x1b).w(FUNC(pce220_state::ram_bank_w));
	map(0x1c, 0x1c).lw8(NAME([this](uint32_t data) { m_power_state = data; })); //peripheral reset
	map(0x1d, 0x1d).r(FUNC(pce220_state::battery_r));
	map(0x1e, 0x1e).w(FUNC(pce220_state::battery_w));
	map(0x1f, 0x1f).r(FUNC(pce220_state::port1f_r));
}

void pce220_state::pce220_io(address_map &map)
{
	pce220_io_common(map);
	map(0x58, 0x58).w(m_lcdc, FUNC(hd61202_device::control_w));
	map(0x59, 0x59).r(m_lcdc, FUNC(hd61202_device::status_r));
	map(0x5a, 0x5a).w(m_lcdc, FUNC(hd61202_device::data_w));
	map(0x5b, 0x5b).r(m_lcdc, FUNC(hd61202_device::data_r));
}

void pcg815_state::pcg815_io(address_map &map)
{
	pce220_io_common(map);
	map(0x50, 0x50).w(FUNC(pcg815_state::lcdc_control_w));
	map(0x52, 0x52).w(FUNC(pcg815_state::lcdc_data_w));
	map(0x54, 0x54).w(m_lcdc2, FUNC(hd61202_device::control_w));
	map(0x55, 0x55).r(m_lcdc2, FUNC(hd61202_device::status_r));
	map(0x56, 0x56).w(m_lcdc2, FUNC(hd61202_device::data_w));
	map(0x57, 0x57).r(m_lcdc2, FUNC(hd61202_device::data_r));
	map(0x58, 0x58).w(m_lcdc, FUNC(hd61202_device::control_w));
	map(0x59, 0x59).r(m_lcdc, FUNC(hd61202_device::status_r));
	map(0x5a, 0x5a).w(m_lcdc, FUNC(hd61202_device::data_w));
	map(0x5b, 0x5b).r(m_lcdc, FUNC(hd61202_device::data_r));
}

void pcg850v_state::pcg850v_io(address_map &map)
{
	pce220_io_common(map);
	map(0x19, 0x19).rw(FUNC(pcg850v_state::rom_bank_r), FUNC(pcg850v_state::g850v_rom_bank_w));
	map(0x40, 0x41).mirror(0x1e).rw("sed1560", FUNC(sed1560_device::read), FUNC(sed1560_device::write));
	map(0x69, 0x69).rw(FUNC(pcg850v_state::g850v_bank_r), FUNC(pcg850v_state::g850v_bank_w));
	map(0x74, 0x74).nopr();
}

INPUT_CHANGED_MEMBER(pce220_state::kb_irq)
{
	if (m_irq_mask & IRQ_FLAG_KEY)
	{
		m_input_merger->in_w<0>(newval ? ASSERT_LINE : CLEAR_LINE);

		m_irq_flag = (m_irq_flag & 0xfe) | (newval & 0x01);
	}
}

INPUT_CHANGED_MEMBER(pce220_state::on_irq)
{
	if (m_irq_mask & IRQ_FLAG_ON)
	{
		if (!(m_power_state & 1))
		{
			install_bootrom();
			m_maincpu->reset();
		}

		m_input_merger->in_w<1>(newval ? ASSERT_LINE : CLEAR_LINE);

		m_irq_flag = (m_irq_flag & 0xfd) | ((newval & 0x01)<<1);
	}
}

/* Input ports */
static INPUT_PORTS_START( pce220 )
	PORT_START("BATTERY")
	PORT_CONFNAME( 0x03, 0x00, "Battery Status" )
	PORT_CONFSETTING( 0x00, DEF_STR( Normal ) )
	PORT_CONFSETTING( 0x01, "Low" )
	PORT_CONFSETTING( 0x02, "Empty" )

	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F10)          PORT_NAME("OFF")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_Q)            PORT_NAME("Q")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('Q')  PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_W)            PORT_NAME("W")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('W')  PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_E)            PORT_NAME("E")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('E')  PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_R)            PORT_NAME("R")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('R')  PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_T)            PORT_NAME("T")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('T')  PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_Y)            PORT_NAME("Y")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('Y')  PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_U)            PORT_NAME("U")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('U')  PORT_CHAR('\'')
	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_A)            PORT_NAME("A")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('A')  PORT_CHAR('[')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_S)            PORT_NAME("S")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('S')  PORT_CHAR(']')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_D)            PORT_NAME("D")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('D')  PORT_CHAR('{')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F)            PORT_NAME("F")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('F')  PORT_CHAR('}')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_G)            PORT_NAME("G")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('G')  PORT_CHAR('\\')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_H)            PORT_NAME("H")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('H')  PORT_CHAR('|')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_J)            PORT_NAME("J")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('J')  PORT_CHAR('~')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_K)            PORT_NAME("K")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('K')  PORT_CHAR('_')
	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_Z)            PORT_NAME("Z")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('Z')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_X)            PORT_NAME("X")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('X')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_C)            PORT_NAME("C")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('C')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_V)            PORT_NAME("V")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('V')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_B)            PORT_NAME("B")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('B')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_N)            PORT_NAME("N")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('N')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_M)            PORT_NAME("M")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('M')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_COMMA)        PORT_NAME(",")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(',')  PORT_CHAR('?')
	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F1)           PORT_NAME("CAL")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F2)           PORT_NAME("BAS")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_CAPSLOCK)     PORT_NAME("CAPS")       PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_HOME)         PORT_NAME("ANS")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_TAB)          PORT_NAME("TAB")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_SPACE)        PORT_NAME("SPACE")      PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(' ')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_DOWN)         PORT_NAME("DOWN")       PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_UP)           PORT_NAME("UP")         PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_LEFT)         PORT_NAME("LEFT")       PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_RIGHT)        PORT_NAME("RIGHT")      PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F4)           PORT_NAME("CONS")       PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_0)            PORT_NAME("0")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('0')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_STOP)         PORT_NAME(".")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('.')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_BACKSLASH)    PORT_NAME("+/-")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_PLUS_PAD)     PORT_NAME("+")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('+')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_ENTER)        PORT_NAME("RET")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(13)
	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_L)            PORT_NAME("L")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('L')  PORT_CHAR('=')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_COLON)        PORT_NAME(";")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(';')  PORT_CHAR(':')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_DEL)          PORT_NAME("DEL")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_1)            PORT_NAME("1")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('1')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_2)            PORT_NAME("2")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('2')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_3)            PORT_NAME("3")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('3')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_MINUS)        PORT_NAME("-")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('-')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F5)           PORT_NAME("M+")         PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_I)            PORT_NAME("I")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('I')  PORT_CHAR('<')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_O)            PORT_NAME("O")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('O')  PORT_CHAR('>')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_INSERT)       PORT_NAME("INS")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_4)            PORT_NAME("4")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('4')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_5)            PORT_NAME("5")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('5')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_6)            PORT_NAME("6")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('6')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_ASTERISK)     PORT_NAME("*")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('*')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F6)           PORT_NAME("RM")         PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_START("LINE7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_P)            PORT_NAME("P")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('P')  PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_BACKSPACE)    PORT_NAME("BS")         PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE),8)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F7)           PORT_NAME("n!")         PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_7)            PORT_NAME("7")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('7')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_8)            PORT_NAME("8")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('8')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_9)            PORT_NAME("9")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('9')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_SLASH)        PORT_NAME("/")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('/')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_NAME(")")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(')')
	PORT_START("LINE8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_1_PAD)        PORT_NAME("hyp")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_2_PAD)        PORT_NAME("DEG")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_3_PAD)        PORT_NAME("y^x")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('^')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_4_PAD)        PORT_NAME("sqrt")       PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_5_PAD)        PORT_NAME("x^2")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_OPENBRACE)    PORT_NAME("(")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('(')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_6_PAD)        PORT_NAME("1/x")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_7_PAD)        PORT_NAME("MDF")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_START("LINE9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_LCONTROL)     PORT_NAME("2nd")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_8_PAD)        PORT_NAME("sin")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_9_PAD)        PORT_NAME("cos")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_0_PAD)        PORT_NAME("ln")         PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F8)           PORT_NAME("log")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F9)           PORT_NAME("tan")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F11)          PORT_NAME("FSE")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_ESC)          PORT_NAME("CCE")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(27)
	PORT_START("SHIFT")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)     PORT_NAME("Shift")      PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(UCHAR_SHIFT_1)
	PORT_START("ON")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_PGUP)         PORT_NAME("ON")         PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  on_irq, 0 )
INPUT_PORTS_END

static INPUT_PORTS_START( pcg850v )
	PORT_START("BATTERY")
	PORT_CONFNAME( 0x03, 0x00, "Battery Status" )
	PORT_CONFSETTING( 0x00, DEF_STR( Normal ) )
	PORT_CONFSETTING( 0x01, "Low" )
	PORT_CONFSETTING( 0x02, "Empty" )

	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F10)          PORT_NAME("OFF")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_Q)            PORT_NAME("Q")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('Q')  PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_W)            PORT_NAME("W")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('W')  PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_E)            PORT_NAME("E")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('E')  PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_R)            PORT_NAME("R")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('R')  PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_T)            PORT_NAME("T")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('T')  PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_Y)            PORT_NAME("Y")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('Y')  PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_U)            PORT_NAME("U")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('U')  PORT_CHAR('\'')
	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_A)            PORT_NAME("A")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('A')  PORT_CHAR('[')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_S)            PORT_NAME("S")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('S')  PORT_CHAR(']')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_D)            PORT_NAME("D")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('D')  PORT_CHAR('{')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F)            PORT_NAME("F")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('F')  PORT_CHAR('}')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_G)            PORT_NAME("G")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_H)            PORT_NAME("H")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('H')  PORT_CHAR('|')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_J)            PORT_NAME("J")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('J')  PORT_CHAR('~')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_K)            PORT_NAME("K")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('K')  PORT_CHAR('_')
	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_Z)            PORT_NAME("Z")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('Z')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_X)            PORT_NAME("X")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('X')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_C)            PORT_NAME("C")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('C')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_V)            PORT_NAME("V")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('V')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_B)            PORT_NAME("B")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('B')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_N)            PORT_NAME("N")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('N')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_M)            PORT_NAME("M")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('M')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_COMMA)        PORT_NAME(",")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(',')  PORT_CHAR('?')
	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F1)           PORT_NAME("BAS")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F2)           PORT_NAME("TEXT")       PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_CAPSLOCK)     PORT_NAME("CAPS")       PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_HOME)         PORT_NAME("KANA")       PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_TAB)          PORT_NAME("TAB")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_SPACE)        PORT_NAME("SPACE")      PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(' ')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_DOWN)         PORT_NAME("DOWN")       PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_UP)           PORT_NAME("UP")         PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_LEFT)         PORT_NAME("LEFT")       PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_RIGHT)        PORT_NAME("RIGHT")      PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F4)           PORT_NAME("CONS")       PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_0)            PORT_NAME("0")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('0')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_STOP)         PORT_NAME(".")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('.')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_EQUALS)       PORT_NAME("=")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_PLUS_PAD)     PORT_NAME("+")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('+')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_ENTER)        PORT_NAME("RET")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(13)
	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_L)            PORT_NAME("L")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('L')  PORT_CHAR('=')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_COLON)        PORT_NAME(";")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(';')  PORT_CHAR(':')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_DEL)          PORT_NAME("DEL")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_1)            PORT_NAME("1")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('1')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_2)            PORT_NAME("2")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('2')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_3)            PORT_NAME("3")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('3')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_MINUS)        PORT_NAME("-")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('-')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F5)           PORT_NAME("M+")         PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_I)            PORT_NAME("I")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('I')  PORT_CHAR('<')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_O)            PORT_NAME("O")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('O')  PORT_CHAR('>')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_INSERT)       PORT_NAME("INS")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_4)            PORT_NAME("4")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('4')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_5)            PORT_NAME("5")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('5')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_6)            PORT_NAME("6")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('6')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_ASTERISK)     PORT_NAME("*")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('*')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F6)           PORT_NAME("RM")         PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_START("LINE7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_P)            PORT_NAME("P")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('P')  PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_BACKSPACE)    PORT_NAME("BS")         PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE),8)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F7)           PORT_NAME("pi")         PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_7)            PORT_NAME("7")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('7')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_8)            PORT_NAME("8")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('8')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_9)            PORT_NAME("9")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('9')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_SLASH)        PORT_NAME("/")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('/')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_NAME(")")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(')')
	PORT_START("LINE8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_1_PAD)        PORT_NAME("nPr")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_2_PAD)        PORT_NAME("DEG")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_3_PAD)        PORT_NAME("SQR")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_4_PAD)        PORT_NAME("SQU")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_5_PAD)        PORT_NAME("x^y")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('^')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_OPENBRACE)    PORT_NAME("(")          PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR('(')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_6_PAD)        PORT_NAME("1/x")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_7_PAD)        PORT_NAME("MDF")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_START("LINE9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_LCONTROL)     PORT_NAME("2nd")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_8_PAD)        PORT_NAME("sin")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_9_PAD)        PORT_NAME("cos")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_0_PAD)        PORT_NAME("ln")         PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F8)           PORT_NAME("log")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F9)           PORT_NAME("tan")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_F11)          PORT_NAME("FSE")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_ESC)          PORT_NAME("CCE")        PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(27)
	PORT_START("SHIFT")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)     PORT_NAME("Shift")      PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  kb_irq, 0 )  PORT_CHAR(UCHAR_SHIFT_1)
	PORT_START("ON")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_CODE(KEYCODE_PGUP)         PORT_NAME("ON")         PORT_CHANGED_MEMBER(DEVICE_SELF, pce220_state,  on_irq, 0 )
INPUT_PORTS_END

void pce220_state::device_resolve_objects()
{
	m_lcd_symbols.resolve();
}

void pce220_state::machine_start()
{
	uint8_t *rom = memregion("user1")->base();
	uint8_t *ram = m_ram->pointer();

	m_banks[0]->configure_entries(0, 2, ram + 0x0000, 0x8000);
	m_banks[1]->configure_entries(0, 2, ram + 0x4000, 0x8000);
	m_banks[2]->configure_entries(0, 8, rom, 0x4000);
	m_banks[3]->configure_entries(0, 16, rom, 0x4000);

	subdevice<nvram_device>("nvram")->set_base(ram, m_ram->size());

	save_item(NAME(m_bank_num));
	save_item(NAME(m_irq_mask));
	save_item(NAME(m_irq_flag));
	save_item(NAME(m_timer_status));
	save_item(NAME(m_kb_matrix));
	save_item(NAME(m_power_state));
	save_item(NAME(m_port15));
	save_item(NAME(m_port18));
	save_item(NAME(m_battery_sel));
}

void pcg850v_state::machine_start()
{
	pce220_state::machine_start();

	uint8_t *rom = memregion("user1")->base();
	m_banks[3]->configure_entries(0, 22, rom, 0x4000);

	save_item(NAME(m_g850v_bank_num));
}

void pce220_state::install_bootrom()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.unmap_write(0x0000, 0x3fff);

	// install the boot code into the first bank
	space.install_rom(0x0000, 0x3fff, memregion("user1")->base() + 0x0000);
}

void pce220_state::machine_reset()
{
	install_bootrom();
	m_bank_num = 0;
	m_irq_mask = 0;
	m_irq_flag = 0;
	m_timer_status = 0;
	m_kb_matrix = 0;
	m_power_state = 0;
	m_port15 = 0;
	m_port18 = 0;
	m_battery_sel = 0;
}

void pcg850v_state::machine_reset()
{
	pce220_state::machine_reset();

	m_g850v_bank_num = 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(pce220_state::pce220_timer_callback)
{
	m_timer_status = 1;
	if (m_irq_mask & IRQ_FLAG_TIMER)
	{
		m_input_merger->in_w<2>(ASSERT_LINE);

		m_irq_flag = (m_irq_flag & 0xfb) | (m_timer_status<<2);
	}
}

void pce220_state::pce220_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(131, 136, 139)); // lcd pixel off
	palette.set_pen_color(1, rgb_t( 51,  42,  43)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(138, 146, 148)); // background
}


void pce220_state::pce220(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 3.072_MHz_XTAL); // CMOS-SC7852
	m_maincpu->set_addrmap(AS_PROGRAM, &pce220_state::pce220_mem);
	m_maincpu->set_addrmap(AS_IO, &pce220_state::pce220_io);

	INPUT_MERGER_ANY_HIGH(config, m_input_merger);
	m_input_merger->output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	/* video hardware */
	// 4 lines x 24 characters, resp. 144 x 32 pixel
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_screen_update(m_lcdc, FUNC(hd61202_device::screen_update));
	m_screen->set_size(24*6, 4*8);
	m_screen->set_visarea_full();
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(pce220_state::pce220_palette), 3);

	HD61202(config, m_lcdc);
	m_lcdc->set_screen_update_cb(FUNC(pce220_state::hd61202_update));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 3250).add_route(ALL_OUTPUTS, "mono", 0.50);

	TIMER(config, "pce220_timer").configure_periodic(FUNC(pce220_state::pce220_timer_callback), attotime::from_msec(468));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K"); // 32K internal + 32K external card

	PCE220SERIAL(config, m_serial, 0);
	config.set_default_layout(layout_pce220);
}

void pcg815_state::pcg815(machine_config &config)
{
	pce220(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &pcg815_state::pcg815_io);

	/* video hardware */
	// 4 lines x 24 characters, resp. 144 x 32 pixel
	m_screen->set_screen_update(FUNC(pcg815_state::screen_update));

	m_lcdc->set_screen_update_cb(FUNC(pcg815_state::hd61202_1_update));

	HD61202(config, m_lcdc2);
	m_lcdc2->set_screen_update_cb(FUNC(pcg815_state::hd61202_2_update));
}

void pcg850v_state::pcg850v(machine_config &config)
{
	pce220(config);

	/* basic machine hardware */
	m_maincpu->set_clock(8_MHz_XTAL); // CMOS-SC7852
	m_maincpu->set_addrmap(AS_IO, &pcg850v_state::pcg850v_io);

	/* video hardware */
	// 6 lines x 24 characters, resp. 144 x 48 pixel
	m_screen->set_screen_update("sed1560", FUNC(sed1560_device::screen_update));
	m_screen->set_size(144, 48);
	m_screen->set_visarea_full();

	config.device_remove("hd61202");

	sed1560_device &lcdc(SED1560(config, "sed1560"));
	lcdc.set_screen_update_cb(FUNC(pcg850v_state::sed1560_update));

	config.set_default_layout(layout_pcg850v);
}

/* ROM definition */
ROM_START( pce220 )
	ROM_REGION( 0x40000, "user1", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v1", "v 0.1")
	ROM_SYSTEM_BIOS( 1, "v2", "v 0.2")
	ROM_LOAD(  "bank0.bin",      0x0000, 0x4000, CRC(1fa94d11) SHA1(24c54347dbb1423388360a359aa09db47d2057b7))
	ROM_LOAD(  "bank1.bin",      0x4000, 0x4000, CRC(0f9864b0) SHA1(6b7301c96f1a865e1931d82872a1ed5d1f80644e))
	ROM_LOAD(  "bank2.bin",      0x8000, 0x4000, CRC(1625e958) SHA1(090440600d461aa7efe4adbf6e975aa802aabeec))
	ROM_LOAD(  "bank3.bin",      0xc000, 0x4000, CRC(ed9a57f8) SHA1(58087dc64103786a40325c0a1e04bd88bfd6da57))
	ROM_LOAD(  "bank4.bin",     0x10000, 0x4000, CRC(e37665ae) SHA1(85f5c84f69f79e7ac83b30397b2a1d9629f9eafa))
	ROMX_LOAD( "bank5.bin",     0x14000, 0x4000, CRC(6b116e7a) SHA1(b29f5a070e846541bddc88b5ee9862cc36b88eee), ROM_BIOS(1))
	ROMX_LOAD( "bank5_0.1.bin", 0x14000, 0x4000, CRC(13c26eb4) SHA1(b9cd0efd6b195653b9610e20ad8aab541824a689), ROM_BIOS(0))
	ROMX_LOAD( "bank6.bin",     0x18000, 0x4000, CRC(4fbfbd18) SHA1(e5aab1df172dcb94aa90e7d898eacfc61157ff15), ROM_BIOS(1))
	ROMX_LOAD( "bank6_0.1.bin", 0x18000, 0x4000, CRC(e2cda7a6) SHA1(01b1796d9485fde6994cb5afbe97514b54cfbb3a), ROM_BIOS(0))
	ROMX_LOAD( "bank7.bin",     0x1c000, 0x4000, CRC(5e98b5b6) SHA1(f22d74d6a24f5929efaf2983caabd33859232a94), ROM_BIOS(1))
	ROMX_LOAD( "bank7_0.1.bin", 0x1c000, 0x4000, CRC(d8e821b2) SHA1(18245a75529d2f496cdbdc28cdf40def157b20c0), ROM_BIOS(0))
ROM_END

ROM_START( pcg815 )
	ROM_REGION( 0x40000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "rom00.bin",    0x000000, 0x004000, CRC(43767d51) SHA1(0d8b8b88a5d084f750daffc0abbb6b868fd3f144) )
	ROM_LOAD( "rom01.bin",    0x004000, 0x004000, CRC(b0e32ace) SHA1(e84c84c18c09dd2920fa38e77d8049b980692d29) )
	ROM_LOAD( "rom02.bin",    0x008000, 0x004000, CRC(f5cb78ba) SHA1(dfb2e415a5603a820b960d810995dd1109d84547) )
	ROM_LOAD( "rom03.bin",    0x00c000, 0x004000, CRC(5eff142b) SHA1(71d8d0a1bd0bcdef43f951036d9e1da0377d6a33) )
	ROM_LOAD( "rom04.bin",    0x010000, 0x004000, CRC(cb91bab5) SHA1(5b354ba6fc48043f03b573ad470518e9bb48cf99) )
	ROM_LOAD( "rom05.bin",    0x014000, 0x004000, CRC(ccaa1876) SHA1(404bf36c3832ee1a4602fa6a917d1ef2db5411c1) )
	ROM_LOAD( "rom06.bin",    0x018000, 0x004000, CRC(9d80802f) SHA1(aec989b8763d1346313a13c761ed0306b4f84257) )
	ROM_LOAD( "rom07.bin",    0x01c000, 0x004000, CRC(618a7be8) SHA1(5f16e47eeeb9c153bb95c6d6e3af276576b8274e) )
	ROM_LOAD( "rom08.bin",    0x020000, 0x004000, CRC(d8ba21a9) SHA1(a5580f22864afee8d4a8f1ecf6231acbde16b612) )
	ROM_LOAD( "rom09.bin",    0x024000, 0x004000, CRC(f6f49677) SHA1(e0d70a11b7dd21662cb444ac93d4b43f584909b5) )
	ROM_LOAD( "rom0a.bin",    0x028000, 0x004000, CRC(ecbabb64) SHA1(14beef60dfd1f18375459e9acb9527589e09e3ce) )
	ROM_LOAD( "rom0b.bin",    0x02c000, 0x004000, CRC(b4000f62) SHA1(eb12db023ff4b3946ab4aa9481a7e96695af0e4c) )
	ROM_LOAD( "rom0c.bin",    0x030000, 0x004000, CRC(e2d40305) SHA1(262aa749ed39fdfa1204297dd6e6c3d4abb9fd65) )
	ROM_LOAD( "rom0d.bin",    0x034000, 0x004000, CRC(3bb1dc8e) SHA1(ee9831e07f028b37b9acdb807becafccfb15d583) )
ROM_END

ROM_START( pcg850v )
	ROM_REGION( 0x58000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "rom00.bin", 0x00000, 0x4000, CRC(c41a7a3e) SHA1(8b85d07e6f2fa048f8a958c261ab3344df750bd2))
	ROM_LOAD( "rom01.bin", 0x04000, 0x4000, CRC(45cafcaf) SHA1(e6142ac2ddb50c90bf1afd03ee2d2741abde8b76))
	ROM_LOAD( "rom02.bin", 0x08000, 0x4000, CRC(c81a804e) SHA1(322b9c6ca5cb7bc41a55c7874838e601ef50644e))
	ROM_LOAD( "rom03.bin", 0x0c000, 0x4000, CRC(9301e937) SHA1(b152186577abf86a7fb535cd63985f2a67e154b9))
	ROM_LOAD( "rom04.bin", 0x10000, 0x4000, CRC(6bf11755) SHA1(fe7458758d26fabfcca34e791fe2425104830b7b))
	ROM_LOAD( "rom05.bin", 0x14000, 0x4000, CRC(8a808f5e) SHA1(448caf0cc30de483d876c31785c8bb27305860b8))
	ROM_LOAD( "rom06.bin", 0x18000, 0x4000, CRC(3902f135) SHA1(40fbd51718a830e3f374843990522e29508376fd))
	ROM_LOAD( "rom07.bin", 0x1c000, 0x4000, CRC(618a7be8) SHA1(5f16e47eeeb9c153bb95c6d6e3af276576b8274e))
	ROM_LOAD( "rom08.bin", 0x20000, 0x4000, CRC(bf05b34d) SHA1(8556d64ddd89191c9dc048280e384f9146d81a16))
	ROM_LOAD( "rom09.bin", 0x24000, 0x4000, CRC(b75dbd0a) SHA1(ca149134a819fa52591ae1733270f8b3c811a18a))
	ROM_LOAD( "rom0a.bin", 0x28000, 0x4000, CRC(e7c5ec8f) SHA1(70cac20d4f01b5b2fdd76837659bc4947a1ebd21))
	ROM_LOAD( "rom0b.bin", 0x2c000, 0x4000, CRC(bdc4f889) SHA1(154d6223e8e750fcbffdb069215fee9492685747))
	ROM_LOAD( "rom0c.bin", 0x30000, 0x4000, CRC(6ef5560a) SHA1(5b2ad8e36354f448fe01acba1d2f69630de345a7))
	ROM_LOAD( "rom0d.bin", 0x34000, 0x4000, CRC(4295a44e) SHA1(0f8dfc759ed17f2b0acc1dc4912e881b1f0f65aa))
	ROM_LOAD( "rom0e.bin", 0x38000, 0x4000, CRC(0c8212e1) SHA1(8145728d87ea5a5e29c0e98c3d2d4278b1070359))
	ROM_LOAD( "rom0f.bin", 0x3c000, 0x4000, CRC(33c30dba) SHA1(296fb6fa3921822c3458acde0962c3a25cb19a1e))
	ROM_LOAD( "rom10.bin", 0x40000, 0x4000, CRC(553a0926) SHA1(2149827f9a2eb879756b3da609e17fc5b1c9bd78))
	ROM_LOAD( "rom11.bin", 0x44000, 0x4000, CRC(385a128d) SHA1(eda1192a75e4cfb50c0ac058f230a84cbfe67d51))
	ROM_LOAD( "rom12.bin", 0x48000, 0x4000, CRC(cd12dfca) SHA1(50a5defdeb4da9b2eac196816559056fc55e1dca))
	ROM_LOAD( "rom13.bin", 0x4c000, 0x4000, CRC(9bca873b) SHA1(4ce43553fb15c0ca866e9582a3ef8dc22a2795b4))
	ROM_LOAD( "rom14.bin", 0x50000, 0x4000, CRC(a2938c40) SHA1(d2c24401eea5e56268ef6eadcc612c8bbaa3342a))
	ROM_LOAD( "rom15.bin", 0x54000, 0x4000, CRC(53a0bf0a) SHA1(bf27baeaf208628fbe3c959d623cc08de21cf9f8))
ROM_END

} // anonymous namespace

/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY  FULLNAME    FLAGS
COMP( 1991, pce220,  0,      0,      pce220,  pce220,  pce220_state,  empty_init, "Sharp", "PC-E220",  MACHINE_SUPPORTS_SAVE )
COMP( 1992, pcg815,  0,      0,      pcg815,  pcg850v, pcg815_state,  empty_init, "Sharp", "PC-G815",  MACHINE_SUPPORTS_SAVE )
COMP( 2001, pcg850v, 0,      0,      pcg850v, pcg850v, pcg850v_state, empty_init, "Sharp", "PC-G850V", MACHINE_NOT_WORKING )
