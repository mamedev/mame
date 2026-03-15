// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Sandro Ronco
/******************************************************************************

  Driver for the ES-2xx series electronic typewriters made by Nakajima.

Nakajima was the OEM manufacturer for a series of typewriters which were
sold by different brands around the world. The PCB layouts for these
machines are the same. The models differed in the amount of RAM, presence
of PCMCIA slot, or 1.44MB Floppy drive; and in the system rom (mainly
only different language support).


Model   |  SRAM | PCMCIA | FDD   | Language | Branded model
--------+-------+--------+-------+----------+----------------------
ES-210N | 128KB | Yes    | No    | German   | Walther ES-210
ES-220  | 128KB | No     | No    | English  | NTS DreamWriter T100
ES-220  | 256KB | Yes    | No    | English  | NTS DreamWriter T400
ES-210N | 128KB | Yes    | No    | Spanish  | Dator 3000
ES-210N | 128KB | Yes    | No    | English  | NTS DreamWriter 325
ES-250  | 256KB | Yes    | 1.44M | English  | NTS DreamWriter T200

The LCD is driven by 6x Sanyo LC7940 and 1x Sanyo LC7942.


The keyboard matrix:

   |         |       |        |        |    |    |    |         |
   |         |       |        |        |    |    |    |         |
-- LSHIFT --   ----- LEFT --- ENTER --   --   --   -- RSHIFT --   ---
   |         |       |        |        |    |    |    |         |
-- 3 ------- Q ----- W ------ E ------ S -- D --   -- 2 -------   ---
   |         |       |        |        |    |    |    |         |
-- 4 ------- Z ----- X ------ A ------ R -- F --   --   -------   ---
   |         |       |        |        |    |    |    |         |
--   ------- B ----- V ------ T ------ G -- C -- Y --   -------   ---
   |         |       |        |        |    |    |    |         |
-- CTRL ---- 1 ----- TAB ----   ------   --   --   -- CAPS ----   ---
   |         |       |        |        |    |    |    |         |
-- ALT ----- CAN --- SPACE --   ------ 5 --   --   -- ` -------   ---
   |         |       |        |        |    |    |    |         |
--   ------- INS --- RIGHT -- \ ------ H -- N -- / -- DOWN ---- 6 ---
   |         |       |        |        |    |    |    |         |
--   ------- ORGN -- UP ----- WP ----- M -- K -- U -- 7 ------- = ---
   |         |       |        |        |    |    |    |         |
--   ------- ] ----- [ ------ ' ------ J -- , -- I -- - ------- 8 ---
   |         |       |        |        |    |    |    |         |
--   ------- BACK -- P ------ ; ------ O -- . -- L -- 9 ------- 0 ---
   |         |       |        |        |    |    |    |         |
   |         |       |        |        |    |    |    |         |



NTS information from http://web.archive.org/web/19980205154137/nts.dreamwriter.com/dreamwt4.html:

File Management & Memory:

- Uniquely name up to 128 files
- Recall, rename or delete files
- Copy files to and from PCMCIA Memory card
- PCMCIA Memory expansion cards available for 60 or 250 pages of text
- Working memory allows up to 20 pages of text (50KB) to be displayed
- Storage memory allows up to 80 pages of text (128KB) in total
- DreamLink software exports and imports documents in RTF retaining all
  formatting to Macintosh or Windows PC to all commonly used Word Processing programs
- Transfer cable provided compatible to both Macintosh and Windows PC's.
- T400 is field upgradeable to IR with the optional Infrared module.

Hardware:

- LCD Screen displays 8 lines by 80 characters raised and tilted 30 degrees
- Contrast Dial and feet adjust to user preference
- Parallel and Serial ports (IR Upgrade Optional) for connectivity to printers, Macintosh and Windows PC's
- PCMCIA Slot
- Full size 64 key keyboard with color coded keys and quick reference menu bar
- NiCad rechargeable batteries for up to 8 hours of continuous use prior to recharging
- AC adapter for recharging batteries is lightweight and compact design
- NEC V20HL 9.83 MHz processor for fast response time
- Durable solid state construction weighing 2.2 lbs including battery pack
- Dimensions approximately 11" wide by 8" long by 1" deep
- FCC and CSA approved


Memory map:

0000 - unknown
       0x00 written during boot sequence

0010-0017 - control banking:
0010 - 00000 - 1ffff
0011 - 20000 - 3ffff
0012 - 40000 - 5ffff
0013 - 60000 - 7ffff
0014 - 80000 - 9ffff
0015 - a0000 - bffff
0016 - c0000 - dffff
0017 - e0000 - fffff

values 00-07 select a ROM bank
      00 - selects last 20000h region of ROM
      01 - 20000h region before last
      02 - etc
values 08-0f select additinal RAM (on models that have 256KB RAM)
values 10-17 select internal RAM
values 18-1f select a PCMCIA bank

on reset 0017 is set to 0, pointing to last 20000h bytes of ROM containing the boot setup code

I/O Map:

0020 - unknown
       0x00 written during boot sequence

0030 - parallel/printer related?

0040 - parallel/printer related?

0050 - 0051 - Beeper frequency
0052 - Beeper control

0053 - unknown

0060 - Irq enable/disable (?)
       0xff written at start of boot sequence
       0x7e written just before enabling interrupts

0061 - unknown
       0xFE written in irq 0xFB handler

0070 - 0x01 (probably any value) powers the unit down. The system will have already
       stored enough information to allow it to continue execution when started
       again.

0090 - Interrupt source clear
       b7 - 1 = clear interrupt source for irq vector 0xf8
       b6 - 1 = clear interrupt source for irq vector 0xf9
       b5 - 1 = clear interrupt source for irq vector 0xfa
       b4 - 1 = clear interrupt source for irq vector 0xfb
       b3 - 1 = clear interrupt source for irq vector 0xfc
       b2 - 1 = clear interrupt source for irq vector 0xfd
       b1 - 1 = clear interrupt source for irq vector 0xfe
       b0 - 1 = clear interrupt source for irq vector 0xff

00A0 - System status

00B0 - Keyboard

00C0 - 00C1 - serial/rs232c communication
  00C0 - data port
  00C1 - control/status

00D0 - 00DF - RTC



TODO:
- drwrt200,drwrt400,drwrt450 only go up to 512KB to initialize pcmcia card.
  Very likely BTANB. At least the documentation for the T400 mentions support
  for 512KB instead of 1MB SRAM card.
- Serial port
- Floppy support
- centronics ack signal is not checked anywhere?
- Frequncy of the keyboard timer is unknown.
- beeper sound is not verified against a real system.

******************************************************************************/

#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "bus/pccard/sram.h"
#include "cpu/nec/nec.h"
#include "imagedev/floppy.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/nvram.h"
#include "machine/rp5c01.h"
#include "machine/timer.h"
#include "machine/upd765.h"
#include "sound/beep.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

#define LOG     0
#define X301    19660000


class nakajies_state : public driver_device
{
public:
	nakajies_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "v20hl")
		, m_rtc(*this, "rtc")
		, m_view{
			{*this, "view_0"}, {*this, "view_1"}, {*this, "view_2"}, {*this, "view_3"},
		    {*this, "view_4"}, {*this, "view_5"}, {*this, "view_6"}, {*this, "view_7"}
		}
		, m_port_row(*this, "ROW%u", 0U)
		, m_port_status(*this, "status")
		, m_rombank(*this, "rombank%u", 0U)
		, m_rambank(*this, "rambank%u", 0U)
		, m_rom_region(*this, "bios")
		, m_nvram(*this, "nvram")
		, m_pcmcia(*this, "pcmcia")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_speaker(*this, "mono")
		, m_beep(*this, "beep")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:0")
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
		, m_uart(*this, "uart")
		, m_uart_clock(*this, "uart_clock")
		, m_serial(*this, "serial")
	{
	}

	void drwrt100(machine_config &config);
	void nakajies210(machine_config &config);
	void nakajies220(machine_config &config);
	void nakajies250(machine_config &config);
	void dator3k(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(power_button_nmi);
	DECLARE_INPUT_CHANGED_MEMBER(power_button_irq);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static constexpr int VIEW_ROM = 0;
	static constexpr int VIEW_RAM = 1;
	static constexpr int VIEW_EXT = 2;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void nakajies_update_irqs();
	u8 irq_clear_r();
	void irq_clear_w(u8 data);
	u8 irq_enable_r();
	void irq_enable_w(u8 data);
	u8 sys_stat_r();
	void lcd_memory_start_w(u8 data);
	u8 keyboard_r();
	void keyboard_row_reset(u8 data);
	void banking_w(offs_t offset, u8 data);
	void uart_control_w(u8 data);
	void centronics_busy_w(int state);
	void centronics_ack_w(int state);
	void uart_txrdy_w(int state);
	void uart_rxrdy_w(int state);

	void nakajies_palette(palette_device &palette) const;
	TIMER_DEVICE_CALLBACK_MEMBER(kb_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(hz10_timer);
	void nakajies_io_map(address_map &map) ATTR_COLD;
	void nakajies_io_map_fdc(address_map &map) ATTR_COLD;
	void nakajies_map(address_map &map) ATTR_COLD;
	void pcmcia_card_detect_w(int state) { m_pcmcia_card_detect = state; }
	void pcmcia_write_protect_w(int state) { m_pcmcia_write_protect = state; }
	void pcmcia_battery_failed_w(int state) { m_pcmcia_battery_failed = state; }
	int pcmcia_card_detect_r() { return m_pcmcia_card_detect; }
	int pcmcia_write_protect_r() { return m_pcmcia_write_protect; }
	template<int Bank> u8 pcmcia_r(offs_t offset);
	template<int Bank> void pcmcia_w(offs_t offset, u8 data);

	required_device<cpu_device> m_maincpu;
	required_device<rp5c01_device> m_rtc;
	memory_view m_view[8];

	required_ioport_array<10> m_port_row;
	required_ioport m_port_status;
	memory_bank_array_creator<8> m_rombank;
	memory_bank_array_creator<8> m_rambank;
	required_memory_region m_rom_region;
	required_device<nvram_device> m_nvram;
	optional_device<pccard_slot_device> m_pcmcia;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<speaker_device> m_speaker;
	required_device<beep_device> m_beep;
	optional_device<n82077aa_device> m_fdc;
	optional_device<floppy_connector> m_floppy;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<i8251_device> m_uart;
	required_device<clock_device> m_uart_clock;
	required_device<rs232_port_device> m_serial;

	u8 m_irq_enabled = 0;
	u8 m_irq_active = 0;
	u8 m_lcd_memory_start = 0;
	u8 m_keyboard_row = 0;
	u8 m_keyboard_row_reset = 0;
	std::unique_ptr<u8[]> m_ram_base;
	u32 m_ram_size = 0;
	u32 m_pcmcia_card_detect = 1;
	u32 m_pcmcia_write_protect = 1;
	u32 m_pcmcia_battery_failed = 1;
	u8 m_uart_control = 0;
	u32 m_centronics_busy = 0;
	u32 m_centronics_ack = 0;
	u32 m_uart_rxrdy = 0;
	u32 m_uart_txrdy = 0;
	bool m_lcd_on = false;
	u16 m_beep_freq = 0;
};


void nakajies_state::nakajies_map(address_map &map)
{
	for (int i = 0; i < 8; i++)
	{
		const offs_t start = i * 0x20000;
		const offs_t end = start + 0x1ffff;
		map(start, end).view(m_view[i]);
		m_view[i][VIEW_ROM](start, end).bankr(m_rombank[i]);
		m_view[i][VIEW_RAM](start, end).bankrw(m_rambank[i]);
		// Banking to external PCMCIA card space
		m_view[i][VIEW_EXT + 0](start, end).rw(FUNC(nakajies_state::pcmcia_r<0>), FUNC(nakajies_state::pcmcia_w<0>));
		m_view[i][VIEW_EXT + 1](start, end).rw(FUNC(nakajies_state::pcmcia_r<1>), FUNC(nakajies_state::pcmcia_w<1>));
		m_view[i][VIEW_EXT + 2](start, end).rw(FUNC(nakajies_state::pcmcia_r<2>), FUNC(nakajies_state::pcmcia_w<2>));
		m_view[i][VIEW_EXT + 3](start, end).rw(FUNC(nakajies_state::pcmcia_r<3>), FUNC(nakajies_state::pcmcia_w<3>));
		m_view[i][VIEW_EXT + 4](start, end).rw(FUNC(nakajies_state::pcmcia_r<4>), FUNC(nakajies_state::pcmcia_w<4>));
		m_view[i][VIEW_EXT + 5](start, end).rw(FUNC(nakajies_state::pcmcia_r<5>), FUNC(nakajies_state::pcmcia_w<5>));
		m_view[i][VIEW_EXT + 6](start, end).rw(FUNC(nakajies_state::pcmcia_r<6>), FUNC(nakajies_state::pcmcia_w<6>));
		m_view[i][VIEW_EXT + 7](start, end).rw(FUNC(nakajies_state::pcmcia_r<7>), FUNC(nakajies_state::pcmcia_w<7>));
	}
}


template<int Bank>
u8 nakajies_state::pcmcia_r(offs_t offset)
{
	return m_pcmcia->read_memory_byte((Bank * 0x20000) + offset);
}


template<int Bank>
void nakajies_state::pcmcia_w(offs_t offset, u8 data)
{
	m_pcmcia->write_memory_byte((Bank * 0x20000) + offset, data);
}


void nakajies_state::nakajies_update_irqs()
{
	// Hack: IRQ mask is temporarily disabled because it doesn't allow the IRQ vectors 0xFA
	// and 0xFB that are used for scanning the kb, this need further investigation.
	// drwrt200 stops responding to keyboard input when checking against m_irq_enabled
	uint8_t irq = m_irq_active; // & m_irq_enabled;
	uint8_t vector = 0xff;

	if (LOG)
		logerror("nakajies_update_irqs: irq_enabled = %02x, irq_active = %02x\n", m_irq_enabled, m_irq_active);

	// Assuming irq 0xFF has the highest priority and 0xF8 the lowest
	while (vector >= 0xf8 && !(irq & 0x01))
	{
		irq >>= 1;
		vector -= 1;
	}

	if (vector >= 0xf8)
	{
		m_maincpu->set_input_line_and_vector(0, ASSERT_LINE, vector); // V20
	}
	else
	{
		m_maincpu->set_input_line(0, CLEAR_LINE);
	}
}


u8 nakajies_state::irq_clear_r()
{
	return 0x00;
}


void nakajies_state::irq_clear_w(u8 data)
{
	m_irq_active &= ~data;
	nakajies_update_irqs();
}


u8 nakajies_state::irq_enable_r()
{
	return m_irq_enabled;
}


void nakajies_state::irq_enable_w(u8 data)
{
	// Bit 0 - set after scanning of keyboard?
	m_irq_enabled = data;
	nakajies_update_irqs();
}


/*
  System status:
  7------- PCMCIA card present. 0 = present, 1 = no card present.
  -6------ PCMCIA card write protected. 0 = not protected, 1 = write protected.
  --5----- unknown
  ---4---- PCMCIA battery status. 0 = ok, 1 = battery low/failed.
  ----3--- Battery pack ok. 0 = ok, 1 = low.
  -----2-- Lithium coin battery ok. 0 = ok, 1 = low.
  ------1- centronics busy? when set to 1 no parallel communication is performed.
  -------0 unknown
*/
u8 nakajies_state::sys_stat_r()
{
	return
		(m_pcmcia_card_detect ? 0x80 : 0x00) |
		(m_pcmcia_write_protect ? 0x40 : 0x00) |
		(m_pcmcia_battery_failed ? 0x10 : 0x00) |
		m_port_status->read() |
		(m_centronics_busy ? 0x02 : 0x00) |
		0x21;
}

void nakajies_state::lcd_memory_start_w(u8 data)
{
	// TODO: Not yet identified where or what enables the lcd display.
	m_lcd_on = true;
	m_lcd_memory_start = data;
}


void nakajies_state::banking_w(offs_t offset, u8 data)
{
	if (!BIT(data, 4))
	{
		// ROM or extra RAM
		if (BIT(data, 3))
		{
			if (m_ram_size >= 256 * 1024)
			{
				m_view[offset].select(VIEW_RAM);
				m_rambank[offset]->set_entry(1);
			}
			else
			{
				m_view[offset].disable();
			}
		}
		else
		{
			m_rombank[offset]->set_entry((data & 0x07) ^ 0x07);
			m_view[offset].select(VIEW_ROM);
		}
	}
	else
	{
		if (BIT(data, 3))
		{
			// External (S)RAM
			// Banking and actual storage not verified
			m_view[offset].select(VIEW_EXT + ((data & 0x07) & 0x07));
		}
		else
		{
			// Internal (S)RAM
			// Banking and actual storage not verified
			m_rambank[offset]->set_entry(0);
			m_view[offset].select(VIEW_RAM);
		}
	}
}


u8 nakajies_state::keyboard_r()
{
	const u8 row = m_keyboard_row;
	m_keyboard_row++;
	return (row < 10) ? m_port_row[row]->read() : 0;
}


void nakajies_state::keyboard_row_reset(u8 data)
{
	if (!BIT(m_keyboard_row_reset, 0) && BIT(data, 0))
	{
		m_keyboard_row = 0;
	}
	m_keyboard_row_reset = data;
}


void nakajies_state::nakajies_io_map(address_map &map)
{
	// Temp for debugging
	map(0x0000, 0x00ff).lrw8(NAME([](offs_t offset) { printf("read %02x\n", offset); return 0xff; }), NAME([](offs_t offset, u8 data) { printf("write %02x %02x\n", offset, data); }));

	map(0x0000, 0x0000).w(FUNC(nakajies_state::lcd_memory_start_w));
	map(0x0010, 0x0017).w(FUNC(nakajies_state::banking_w));
	// Parallel coomunication
	map(0x30, 0x30).w(FUNC(nakajies_state::uart_control_w));
	map(0x40, 0x40).w("cent_data_out", FUNC(output_latch_device::write));
	// Unknown, 60 is written frequently
	map(0x0050, 0x0050).lw8(NAME([this](u8 data) { m_beep_freq = (m_beep_freq & 0xff00) | data; m_beep->set_clock(250000/m_beep_freq); }));
	map(0x0051, 0x0051).lw8(NAME([this](u8 data) { m_beep_freq = (m_beep_freq & 0x00ff) | (data << 8); m_beep->set_clock(250000/m_beep_freq); }));
	map(0x0052, 0x0052).lw8(NAME([this](u8 data) { m_beep->set_state(!BIT(data, 7)); }));
	map(0x0053, 0x0053).noprw();
	map(0x0060, 0x0060).rw(FUNC(nakajies_state::irq_enable_r), FUNC(nakajies_state::irq_enable_w));
	map(0x0061, 0x0061).w(FUNC(nakajies_state::keyboard_row_reset));
	map(0x0070, 0x0070).lw8(NAME([this](u8 data) {
		m_lcd_on = false;
		m_maincpu->suspend(SUSPEND_REASON_HALT, true);
	}));
	map(0x0090, 0x0090).rw(FUNC(nakajies_state::irq_clear_r), FUNC(nakajies_state::irq_clear_w));
	map(0x00a0, 0x00a0).r(FUNC(nakajies_state::sys_stat_r));
	map(0x00b0, 0x00b0).r(FUNC(nakajies_state::keyboard_r));
	// Serial communication
	map(0x00c0, 0x00c1).rw(m_uart, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x00d0, 0x00df).rw(m_rtc, FUNC(rp5c01_device::read), FUNC(rp5c01_device::write));
}


void nakajies_state::nakajies_io_map_fdc(address_map &map)
{
	nakajies_io_map(map);
	map(0x00e0, 0x00ef).m(m_fdc, FUNC(n82077aa_device::map));
}


void nakajies_state::uart_control_w(u8 data)
{
	// 76------ unknown
	// --5----- centronics strobe
	// ---4---- unknown
	// ----3--- uart clock/reset?
	// -----210 baud rate
	//          000 - 19200
	//          001 - 9600
	//          010 - 4800
	//          011 - 2400
	//          100 - 1200

	m_centronics->write_strobe(BIT(data, 5));
	m_uart_control = data;

	if (BIT(m_uart_control, 3) == 1 && BIT(data, 3) == 0)
		m_uart->reset();

	m_uart_clock->set_clock_scale(1 << (data & 0x07));
}


void nakajies_state::centronics_busy_w(int state)
{
	m_centronics_busy = state;
}


void nakajies_state::centronics_ack_w(int state)
{
	m_centronics_ack = state;
}


void nakajies_state::uart_txrdy_w(int state)
{
	if (!m_uart_rxrdy && state)
	{
		m_irq_active |= 0x08;
		nakajies_update_irqs();
	}

	m_uart_rxrdy = state;
}


void nakajies_state::uart_rxrdy_w(int state)
{
	if (!m_uart_rxrdy && state)
	{
		m_irq_active |= 0x08;
		nakajies_update_irqs();
	}

	m_uart_rxrdy = state;
}


static INPUT_PORTS_START(nakajies)
	PORT_START("status")
	PORT_CONFNAME(0x08, 0x00, "Battery pack failed")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x08, DEF_STR(Yes))
	PORT_CONFNAME(0x04, 0x00, "Coin battery Failed")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x04, DEF_STR(Yes))

	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")  PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LEFT")        PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER")       PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ALT")         PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("`")           PORT_CODE(KEYCODE_BACKSLASH2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CAN")         PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SPACE")       PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5")           PORT_CODE(KEYCODE_5)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CONTROL")     PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CAPSLOCK")    PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1")           PORT_CODE(KEYCODE_1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TAB")         PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3")           PORT_CODE(KEYCODE_3)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2")           PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q")           PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W")           PORT_CODE(KEYCODE_W)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E")           PORT_CODE(KEYCODE_E)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S")           PORT_CODE(KEYCODE_S)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D")           PORT_CODE(KEYCODE_D)

	PORT_START("ROW4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4")           PORT_CODE(KEYCODE_4)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z")           PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X")           PORT_CODE(KEYCODE_X)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A")           PORT_CODE(KEYCODE_A)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R")           PORT_CODE(KEYCODE_R)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F")           PORT_CODE(KEYCODE_F)

	PORT_START("ROW5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B")           PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V")           PORT_CODE(KEYCODE_V)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T")           PORT_CODE(KEYCODE_T)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y")           PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G")           PORT_CODE(KEYCODE_G)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C")           PORT_CODE(KEYCODE_C)

	PORT_START("ROW6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6")           PORT_CODE(KEYCODE_6)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DOWN")        PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INSERT")      PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RIGHT")       PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\")          PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/")           PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H")           PORT_CODE(KEYCODE_H)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N")           PORT_CODE(KEYCODE_N)

	PORT_START("ROW7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("=")           PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7")           PORT_CODE(KEYCODE_7)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ORGN")        PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UP")          PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("WP")          PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U")           PORT_CODE(KEYCODE_U)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M")           PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K")           PORT_CODE(KEYCODE_K)

	PORT_START("ROW8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8")           PORT_CODE(KEYCODE_8)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-")           PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("]")           PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[")           PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\'")          PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I")           PORT_CODE(KEYCODE_I)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J")           PORT_CODE(KEYCODE_J)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(",")           PORT_CODE(KEYCODE_COMMA)

	PORT_START("ROW9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0")           PORT_CODE(KEYCODE_0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9")           PORT_CODE(KEYCODE_9)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BACK")        PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P")           PORT_CODE(KEYCODE_P)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(";")           PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L")           PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O")           PORT_CODE(KEYCODE_O)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".")           PORT_CODE(KEYCODE_STOP)
INPUT_PORTS_END


static INPUT_PORTS_START(nakajies_irq)
	PORT_INCLUDE(nakajies)
	PORT_START("power")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Power On/Off") PORT_CODE(KEYCODE_END) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nakajies_state::power_button_irq), 0)
INPUT_PORTS_END


static INPUT_PORTS_START(nakajies_nmi)
	PORT_INCLUDE(nakajies)
	PORT_START("power")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Power On/Off") PORT_CODE(KEYCODE_END) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nakajies_state::power_button_nmi), 0)
INPUT_PORTS_END


INPUT_CHANGED_MEMBER(nakajies_state::power_button_nmi)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? ASSERT_LINE : CLEAR_LINE);
}


INPUT_CHANGED_MEMBER(nakajies_state::power_button_irq)
{
	m_irq_active |= 0x01;
	nakajies_update_irqs();
}


void nakajies_state::machine_start()
{
	u32 rom_size = m_rom_region->bytes();

	m_ram_base = make_unique_clear<uint8_t[]>(m_ram_size);
	m_nvram->set_base(&m_ram_base[0], m_ram_size);

	for (int i = 0; i < 8; i++)
	{
		// TODO: rom banks outside max bank size; assuming the banks are simply mirrored
		for (int j = 0; j < 8; j += rom_size / 0x20000)
			m_rombank[i]->configure_entries(j, rom_size / 0x20000, m_rom_region->base(), 0x20000);

		for (int j = 0; j < 2; j += m_ram_size / 0x20000)
			m_rambank[i]->configure_entries(j, m_ram_size / 0x20000, &m_ram_base[0], 0x20000);
	}

	save_item(NAME(m_irq_enabled));
	save_item(NAME(m_irq_active));
	save_item(NAME(m_lcd_memory_start));
	save_item(NAME(m_keyboard_row));
	save_item(NAME(m_keyboard_row_reset));
	save_item(NAME(m_pcmcia_card_detect));
	save_item(NAME(m_pcmcia_write_protect));
	save_item(NAME(m_pcmcia_battery_failed));
	save_item(NAME(m_uart_control));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_centronics_ack));
	save_item(NAME(m_uart_rxrdy));
	save_item(NAME(m_uart_txrdy));
}


void nakajies_state::machine_reset()
{
	m_irq_enabled = 0;
	m_irq_active = 0;
	m_lcd_memory_start = 0;
	m_keyboard_row = 0;

	for (int i = 0; i < 8; i++)
	{
		m_view[i].select(VIEW_ROM);
	}

	for (int i = 0; i < 8; i++)
	{
		m_rombank[i]->set_entry(0x07);
		m_rambank[i]->set_entry(0);
	}
}

u32 nakajies_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t* lcd_memory_start = &m_ram_base[m_lcd_memory_start << 9];
	int height = screen.height();

	if (m_lcd_on)
	{
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < 60; x++)
			{
				u8 data = lcd_memory_start[y*64 + x];

				for (int px = 0; px < 8; px++)
				{
					bitmap.pix(y, (x * 8) + px) = BIT(data, 7);
					data <<= 1;
				}
			}
		}
	}
	else
	{
		bitmap.fill(0);
	}

	return 0;
}


TIMER_DEVICE_CALLBACK_MEMBER(nakajies_state::kb_timer)
{
	if (m_keyboard_row > 0x09)
	{
		// trigger reset of keyboard scan
		m_irq_active |= 0x20;
	}
	else
	{
		// trigger handling of keyboard row
		m_irq_active |= 0x10;
	}

	nakajies_update_irqs();
}


TIMER_DEVICE_CALLBACK_MEMBER(nakajies_state::hz10_timer)
{
	m_irq_active |= 0x40;
	nakajies_update_irqs();
}


void nakajies_state::nakajies_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}


/* F4 Character Displayer */
static const gfx_layout nakajies_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	2331,                   /* 2331 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START(gfx_wales210)
	GFXDECODE_ENTRY("bios", 0x55043, nakajies_charlayout, 0, 1)
GFXDECODE_END

static GFXDECODE_START(gfx_dator3k)
	GFXDECODE_ENTRY("bios", 0x54fb1, nakajies_charlayout, 0, 1)
GFXDECODE_END

static GFXDECODE_START(gfx_drwrt200)
	GFXDECODE_ENTRY("bios", 0xdbbeb, nakajies_charlayout, 0, 1)
GFXDECODE_END

static GFXDECODE_START(gfx_drwrt400)
	GFXDECODE_ENTRY("bios", 0x580b6, nakajies_charlayout, 0, 1)
GFXDECODE_END

static void pcmcia_devices(device_slot_interface &device)
{
	device.option_add("sram_1m", PCCARD_SRAM_CENTENNIAL_1M);
}

void nakajies_state::drwrt100(machine_config &config)
{
	V20(config, m_maincpu, X301 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &nakajies_state::nakajies_map);
	m_maincpu->set_addrmap(AS_IO, &nakajies_state::nakajies_io_map);

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(50);  // Wild guess
	m_screen->set_screen_update(FUNC(nakajies_state::screen_update));
	m_screen->set_size(80 * 6, 8 * 8);
	m_screen->set_visarea(0, 6 * 80 - 1, 0, 8 * 8 - 1);
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_wales210);
	PALETTE(config, m_palette, FUNC(nakajies_state::nakajies_palette), 2);

	/* sound */
	SPEAKER(config, m_speaker).front_center();
	BEEP(config, m_beep, 0).add_route(ALL_OUTPUTS, m_speaker, 0.25);

	/* rtc */
	RP5C01(config, m_rtc, XTAL(32'768));

	TIMER(config, "kb_timer").configure_periodic(FUNC(nakajies_state::kb_timer), attotime::from_hz(500));
	TIMER(config, "10hz_timer").configure_periodic(FUNC(nakajies_state::hz10_timer), attotime::from_hz(10));

	NVRAM(config, m_nvram);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(nakajies_state::centronics_busy_w));
	m_centronics->ack_handler().set(FUNC(nakajies_state::centronics_ack_w));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	I8251(config, m_uart, 0);
	m_uart->txd_handler().set(m_serial, FUNC(rs232_port_device::write_txd));
	m_uart->rts_handler().set(m_serial, FUNC(rs232_port_device::write_rts));
	m_uart->dtr_handler().set(m_serial, FUNC(rs232_port_device::write_dtr));
	m_uart->rxrdy_handler().set(FUNC(nakajies_state::uart_rxrdy_w));
	m_uart->txrdy_handler().set(FUNC(nakajies_state::uart_txrdy_w));

	CLOCK(config, m_uart_clock, 150 * 16);
	m_uart_clock->signal_handler().set(m_uart, FUNC(i8251_device::write_rxc));
	m_uart_clock->signal_handler().append(m_uart, FUNC(i8251_device::write_txc));

	RS232_PORT(config, m_serial, default_rs232_devices, nullptr);
	m_serial->rxd_handler().set(m_uart, FUNC(i8251_device::write_rxd));
	m_serial->cts_handler().set(m_uart, FUNC(i8251_device::write_cts));
	m_serial->dsr_handler().set(m_uart, FUNC(i8251_device::write_dsr));

	m_ram_size = 128 * 1024;
}

void nakajies_state::nakajies210(machine_config &config)
{
	drwrt100(config);

	PCCARD_SLOT(config, m_pcmcia, pcmcia_devices, nullptr);
	m_pcmcia->cd1().set(FUNC(nakajies_state::pcmcia_card_detect_w));
	m_pcmcia->wp().set(FUNC(nakajies_state::pcmcia_write_protect_w));
	m_pcmcia->bvd1().set(FUNC(nakajies_state::pcmcia_battery_failed_w));

}

void nakajies_state::dator3k(machine_config &config)
{
	nakajies210(config);
	m_gfxdecode->set_info(gfx_dator3k);
}

void nakajies_state::nakajies220(machine_config &config)
{
	nakajies210(config);
	m_gfxdecode->set_info(gfx_drwrt400);
	m_ram_size = 256 * 1024;
}

void nakajies_state::nakajies250(machine_config &config)
{
	nakajies220(config);
	m_maincpu->set_addrmap(AS_IO, &nakajies_state::nakajies_io_map_fdc);

	m_screen->set_size(80 * 6, 16 * 8);
	m_screen->set_visarea(0, 6 * 80 - 1, 0, 16 * 8 - 1);
	m_gfxdecode->set_info(gfx_drwrt200);

	N82077AA(config, m_fdc, 24_MHz_XTAL);  // Actually Intel N82877SL
	FLOPPY_CONNECTOR(config, m_floppy, "35hd", FLOPPY_35_HD, true, floppy_image_device::default_pc_floppy_formats).enable_sound(false);
}


ROM_START(wales210)
	ROM_REGION(0x80000, "bios", 0)

	ROM_SYSTEM_BIOS(0, "wales210", "Walther ES-210")
	ROMX_LOAD("wales210.ic303", 0x00000, 0x80000, CRC(a8e8d991) SHA1(9a133b37b2fbf689ae1c7ab5c7f4e97cd33fd596), ROM_BIOS(0))        /* 27c4001 */

	ROM_SYSTEM_BIOS(1, "drwrtr325_102", "NTS DreamWriter 325 (v1.02)")
	ROMX_LOAD("dr3_1_02uk.ic303", 0x00000, 0x80000, CRC(027db9fe) SHA1(eb52a30510f2e2924c6dae9bc4348cd3572f4997), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "drwrtr325_103", "NTS DreamWriter 325 (v1.03)")
	ROMX_LOAD("dr3_1_03.ic303", 0x00000, 0x80000, CRC(21fd074e) SHA1(52cd07527f3ba96b0ce37578996af502db144e80), ROM_BIOS(2)) // label actually DR3 (1.03)

	ROM_SYSTEM_BIOS(3, "drwrtr325_20", "NTS DreamWriter 325 (v2.0)")
	ROMX_LOAD("nts_325_basic.ic303", 0x00000, 0x80000, CRC(feb40854) SHA1(8838dcbcd7f6be282bb8ed1d7f97187b45f00a58), ROM_BIOS(3))
ROM_END


ROM_START(dator3k)
	ROM_REGION(0x80000, "bios", 0)
	ROM_LOAD("dator3000.ic303", 0x00000, 0x80000, CRC(b67fffeb) SHA1(e48270d15bef9453bcb6ba8aa949fd2ab3feceed))
ROM_END


ROM_START(drwrt100)
	ROM_REGION(0x80000, "bios", 0)
	ROM_LOAD("t100_2.3.ic303", 0x00000, 0x80000, CRC(8a16f12f) SHA1(0a907186db3d1756566d767ee847a7ecf694e74b))      /* Checksum 01F5 on label */
ROM_END


ROM_START(drwrt200)
	ROM_REGION(0x100000, "bios", 0)
	ROM_LOAD("drwrt200.bin", 0x000000, 0x100000, CRC(3c39483c) SHA1(48293e6bdb7e7322d76da7174b716243c0ab7c2c))
ROM_END


ROM_START(drwrt400)
	ROM_REGION(0x100000, "bios", 0)

	ROM_SYSTEM_BIOS(0, "v3_1", "v3.1")
	ROMX_LOAD("t4_ir_3.1_e588.ic303", 0x00000, 0x100000, CRC(1724ceb2) SHA1(101effc8cfa2f084c09b25c4074f684aeb97d044), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "v2_1", "v2.1")
	ROMX_LOAD("t4_ir_2.1.ic303", 0x80000, 0x80000, CRC(f0f45fd2) SHA1(3b4d5722b3e32e202551a1be8ae36f34ad705ddd), ROM_BIOS(1))
ROM_END


ROM_START(drwrt450)
	ROM_REGION(0x100000, "bios", 0)
	ROM_LOAD("t4_ir_35ba308.ic303", 0x00000, 0x100000, CRC(3b5a580d) SHA1(72df34ece1e6d70adf953025d1c458e22ce819e1))
ROM_END


ROM_START(es210_es)
	ROM_REGION(0x80000, "bios", 0)
	ROM_LOAD("nakajima_es.ic303", 0x00000, 0x80000, CRC(214d73ce) SHA1(ce9967c5b2d122ebebe9401278d8ea374e8fb289))
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT    COMPAT  MACHINE      INPUT     CLASS           INIT        COMPANY     FULLNAME            FLAGS
COMP( 199?, wales210, 0,        0,      nakajies210, nakajies_irq, nakajies_state, empty_init, "Walther",  "ES-210",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // German, 128KB RAM
COMP( 199?, dator3k,  wales210, 0,      dator3k,     nakajies_irq, nakajies_state, empty_init, "Dator",    "Dator 3000",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // Spanish, 128KB RAM
COMP( 199?, es210_es, wales210, 0,      nakajies210, nakajies_irq, nakajies_state, empty_init, "Nakajima", "ES-210 (Spain)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // Spanish, 128KB RAM
COMP( 1996, drwrt100, wales210, 0,      drwrt100,    nakajies_irq, nakajies_state, empty_init, "NTS",      "DreamWriter T100", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // English, 128KB RAM
COMP( 1996, drwrt400, wales210, 0,      nakajies220, nakajies_nmi, nakajies_state, empty_init, "NTS",      "DreamWriter T400", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // English, 256KB RAM
COMP( 199?, drwrt450, wales210, 0,      nakajies220, nakajies_nmi, nakajies_state, empty_init, "NTS",      "DreamWriter 450",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // English, 256KB RAM
COMP( 1996, drwrt200, wales210, 0,      nakajies250, nakajies_nmi, nakajies_state, empty_init, "NTS",      "DreamWriter T200", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // English, 256KB RAM
