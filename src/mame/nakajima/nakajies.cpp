// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Sandro Ronco
/******************************************************************************

  Driver for the ES-2xx series electronic typewriters made by Nakajima.

Nakajima was the OEM manufacturer for a series of typewriters which were
sold by different brands around the world. The PCB layouts for these
machines are the same. The models differed in the amount of (static) RAM:
128KB or 256KB; and in the system rom (mainly only different language
support).


Model   |  SRAM | Language | Branded model
--------+-------+----------+----------------------
ES-210N | 128KB | German   | Walther ES-210
ES-220  | 128KB | English  | NTS DreamWriter T100
ES-220  | 256KB | English  | NTS DreamWriter T400
ES-210N | 128KB | Spanish  | Dator 3000
ES-210N | 128KB | English  | NTS DreamWriter 325
ES-250  | xxxKB | English  | NTS DreamWriter T200


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

DreamWriter T400 v2.1 ROM decode notes at https://github.com/RealDeuce/dreamwriter

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


I/O Map:

0000 - LCD scanout base select
       MAME renders from main RAM + (data << 9). Boot writes 0x08, so the
       visible framebuffer starts at 0x1000.

0010-0017 - bank select registers for eight 128 KiB CPU windows
       0010 controls 00000-1ffff, 0011 controls 20000-3ffff, etc.
       Values 00-07 select ROM banks. Values with bit 4 set select RAM banks.
       Some DreamWriter configs also treat bit 3 as a RAM window select; T400
       needs 0x0e for its built-in 160 KiB storage formatter, while 1 MiB
       DreamWriter ROMs seed 0x0f for the same 20000-3ffff window.

       T400 startup mapping:
       10 = 17 -> RAM 00000-1ffff
       11 = 0e -> RAM 20000-3ffff
       12 = 1f -> RAM 00000-1ffff
       13 = 1e -> RAM 20000-3ffff
       14 = 1d -> RAM 00000-1ffff
       15 = 1c -> RAM 20000-3ffff
       16 = 01 -> ROM file 40000-5ffff
       17 = 00 -> ROM file 60000-7ffff

0020 - unknown
       Startup writes 0x00.

0030 - control latch mirrored at 6D94
       Bits 0-2 select the external RS-232 baud-clock divider, bit 4 is set
       during RS-232 setup, bit 5 is pulsed for Centronics -STB, and bit 7 is
       toggled by diagnostic commands.

0040 - Centronics parallel data output latch
       Startup/idle writes 0xff. Printer output writes bytes here.

0050 - buzzer/tone counter low byte
0051 - buzzer/tone counter high byte
0052 - buzzer/tone gate/control
       Firmware writes a 16-bit divisor to 50/51, writes 0x7f to 52 to enable,
       and writes 0xff to 52 to disable.

0060 - IRQ/source mask latch
       Firmware mirrors writes at 6D4F. Bits appear active-low and map in
       ascending vector order: bit 0 = F8, bit 1 = F9, ..., bit 7 = FF. This is
       reversed from the port 90 active/clear bit order used by this driver.
       The low-level idle path writes 6D4F immediately before sti/hlt;
       Centronics ACK output clears bit 6 while the byte feeder is active and
       sets it when the buffer ends.

0061 - keyboard scan/idle control candidate
       Keyboard scan helpers write 0xfe and 0xff here. Exact hardware role is
       still unconfirmed.

0070 - warm/reset/power transition control candidate
       Warm diagnostic and auto-off paths write 0x01 before halting in a loop.

0090 - interrupt source clear
       b7 clears irq vector f8
       b6 clears irq vector f9
       b5 clears irq vector fa, keyboard scan-cycle/reset
       b4 clears irq vector fb, keyboard row scan
       b3 clears irq vector fc, RS-232 receive
       b2 clears irq vector fd
       b1 clears irq vector fe, Centronics ACK
       b0 clears irq vector ff

00A0 - shared status input
       b7 - PCMCIA card absent/not-ready gate (when set)
       b6 - PCMCIA SRAM card write-protect candidate
       b4 - PCMCIA SRAM card battery status, low when clear and card present
       b3 - main battery low, active high
       b2 - CR2032 memory-retention battery low, active high
       b1 - Centronics BUSY, active high
       b5,b0 - unknown

00B0 - keyboard row input
       Returns the row selected by the keyboard scan state.

00C0 - RS-232 USART data register
00C1 - RS-232 USART status/control register
       Firmware programming sequence matches an 8251/8251A-style USART.

00D0-00DF - RTC register block
       MAME maps this to RP5C01. Firmware reads/writes D0-DC as 4-bit BCD
       time/date registers and uses DD-DF as RTC control/mode registers.


IRQ 0xF8:
The handler clears the irq active bit and copies a word from 6D79 to 6D85 and
from 6D7B to 6D87 then enters an endless loop (with interrupts disabled).
Perhaps power on/off related??


IRQ 0xF9: (T400: C049A)
Timer wake source. Firmware arms it by writing a count to port 0x53; the
handler clears bit 0 of 6DA9.


IRQ 0xFA: (T400: C04AE)
Keyboard scan-cycle/reset helper. Expects 6D4F to be set up properly, updates
the IRQ mask, clears the keyboard idle counter, then calls C106F to reset the
keyboard row scan state.


IRQ 0xFB: (T400: C04D1)
Keyboard row scan. Reads from input port 0xB0, stores the current row in
6D06..6D0F, and calls the higher-level keyboard processor after the tenth row.


IRQ 0xFC: (T400: C0550)
RS-232 receive. Reads USART status from port 0xC1, records error bits, reads
received data from port 0xC0, acknowledges with command 0x37 when needed, and
queues the byte for the firmware event/serial path.


IRQ 0xFD: (T400: C0724)
Purpose unknown. Clears bit 3 of 70A5.


IRQ 0xFE: (T400: C0738)
Centronics ACK-driven output. Expects 6D4F to be set up properly, reads the
next byte from the buffer pointer at 6D92, outputs it on port 0x40, and pulses
the strobe latch on port 0x30. No endless loop.


IRQ 0xFF:
This does some simple processing and then enters an endless loop (with interrupts
disabled). Perhaps power on/off related??

TODO:
- On boot, systems do not display version and copyright messages.
- Add PCMCIA slot

******************************************************************************/

#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/pccard/sram.h"
#include "bus/rs232/rs232.h"
#include "cpu/nec/nec.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/nvram.h"
#include "machine/output_latch.h"
#include "machine/rp5c01.h"
#include "machine/timer.h"
#include "sound/beep.h"

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
		, m_nvram(*this, "nvram")
		, m_upd71051(*this, "upd71051")
		, m_usart_clock(*this, "usart_clock")
		, m_centronics(*this, "centronics")
		, m_pcmcia(*this, "pcmcia")
		, m_beeper(*this, "beeper")
		, m_view{
			{*this, "view_0"}, {*this, "view_1"}, {*this, "view_2"}, {*this, "view_3"},
			{*this, "view_4"}, {*this, "view_5"}, {*this, "view_6"}, {*this, "view_7"} }
		, m_port_row(*this, "ROW%u", 0U)
		, m_port_debug(*this, "debug")
		, m_port_status(*this, "STATUS")
		, m_rombank(*this, "rombank%u", 0U)
		, m_rambank(*this, "rambank%u", 0U)
		, m_rom_region(*this, "bios")
	{
	}

	void nakajies210(machine_config &config) ATTR_COLD;
	void nakajies220(machine_config &config) ATTR_COLD;
	void nakajies220_t100(machine_config &config) ATTR_COLD;
	void nakajies220_t450(machine_config &config) ATTR_COLD;
	void nakajies220_256(machine_config &config) ATTR_COLD;
	void nakajies250(machine_config &config) ATTR_COLD;
	void dator3k(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(trigger_irq);
	DECLARE_INPUT_CHANGED_MEMBER(retained_reset);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void set_irq(u8 data);
	void nakajies_update_irqs();
	u8 irq_clear_r();
	void irq_clear_w(u8 data);
	u8 irq_enable_r();
	void irq_enable_w(u8 data);
	u8 unk_a0_r();
	void lcd_memory_start_w(u8 data);
	u8 keyboard_r();
	void banking_w(offs_t offset, u8 data);
	u8 pcmcia_window0_r(offs_t offset);
	u8 pcmcia_window1_r(offs_t offset);
	u8 pcmcia_window2_r(offs_t offset);
	u8 pcmcia_window3_r(offs_t offset);
	u8 pcmcia_window4_r(offs_t offset);
	u8 pcmcia_window5_r(offs_t offset);
	u8 pcmcia_window6_r(offs_t offset);
	u8 pcmcia_window7_r(offs_t offset);
	void pcmcia_window0_w(offs_t offset, u8 data);
	void pcmcia_window1_w(offs_t offset, u8 data);
	void pcmcia_window2_w(offs_t offset, u8 data);
	void pcmcia_window3_w(offs_t offset, u8 data);
	void pcmcia_window4_w(offs_t offset, u8 data);
	void pcmcia_window5_w(offs_t offset, u8 data);
	void pcmcia_window6_w(offs_t offset, u8 data);
	void pcmcia_window7_w(offs_t offset, u8 data);
	u8 pcmcia_memory_r(unsigned window, offs_t offset);
	void pcmcia_memory_w(unsigned window, offs_t offset, u8 data);
	void control_w(u8 data);
	void centronics_ack_w(int state);
	void centronics_busy_w(int state);
	void pcmcia_card_detect_w(int state);
	void pcmcia_battery_voltage_2_w(int state);
	void pcmcia_write_protect_w(int state);
	void buzzer_low_w(u8 data);
	void buzzer_high_w(u8 data);
	void buzzer_gate_w(u8 data);
	void buzzer_update_clock();
	void timer_count_w(u8 data);

	void nakajies_palette(palette_device &palette) const;
	TIMER_DEVICE_CALLBACK_MEMBER(kb_timer);
	TIMER_CALLBACK_MEMBER(f9_timer);
	void nakajies_io_map(address_map &map) ATTR_COLD;
	void nakajies_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<rp5c01_device> m_rtc;
	required_device<nvram_device> m_nvram;
	required_device<i8251_device> m_upd71051;
	required_device<clock_device> m_usart_clock;
	required_device<centronics_device> m_centronics;
	required_device<pccard_slot_device> m_pcmcia;
	required_device<beep_device> m_beeper;
	memory_view m_view[8];
	required_ioport_array<10> m_port_row;
	required_ioport m_port_debug;
	required_ioport m_port_status;
	memory_bank_array_creator<8> m_rombank;
	memory_bank_array_creator<8> m_rambank;
	required_memory_region m_rom_region;

	emu_timer *m_f9_timer = nullptr;
	u8 m_irq_enabled = 0;
	u8 m_irq_active = 0;
	u8 m_lcd_memory_start = 0;
	u8 m_matrix = 0;
	u8 m_control = 0;
	u8 m_buzzer_low = 0;
	u8 m_buzzer_high = 0;
	u8 m_bank_select[8]{};
	int m_centronics_busy = 0;
	int m_pcmcia_card_detect = 1;
	int m_pcmcia_battery_voltage_2 = 1;
	int m_pcmcia_write_protect = 1;
	std::unique_ptr<u8[]> m_ram_base;
	u32 m_ram_size = 0;
	bool m_bank_bit3_selects_ram = false;
};


void nakajies_state::nakajies_map(address_map &map)
{
	for (int i = 0; i < 8; i++)
	{
		const offs_t start = i * 0x20000;
		const offs_t end = start + 0x1ffff;
		map(start, end).view(m_view[i]);
		m_view[i][0](start, end).bankr(m_rombank[i]);
		m_view[i][1](start, end).bankrw(m_rambank[i]);
	}

	m_view[0][2](0x00000, 0x1ffff).rw(FUNC(nakajies_state::pcmcia_window0_r), FUNC(nakajies_state::pcmcia_window0_w));
	m_view[1][2](0x20000, 0x3ffff).rw(FUNC(nakajies_state::pcmcia_window1_r), FUNC(nakajies_state::pcmcia_window1_w));
	m_view[2][2](0x40000, 0x5ffff).rw(FUNC(nakajies_state::pcmcia_window2_r), FUNC(nakajies_state::pcmcia_window2_w));
	m_view[3][2](0x60000, 0x7ffff).rw(FUNC(nakajies_state::pcmcia_window3_r), FUNC(nakajies_state::pcmcia_window3_w));
	m_view[4][2](0x80000, 0x9ffff).rw(FUNC(nakajies_state::pcmcia_window4_r), FUNC(nakajies_state::pcmcia_window4_w));
	m_view[5][2](0xa0000, 0xbffff).rw(FUNC(nakajies_state::pcmcia_window5_r), FUNC(nakajies_state::pcmcia_window5_w));
	m_view[6][2](0xc0000, 0xdffff).rw(FUNC(nakajies_state::pcmcia_window6_r), FUNC(nakajies_state::pcmcia_window6_w));
	m_view[7][2](0xe0000, 0xfffff).rw(FUNC(nakajies_state::pcmcia_window7_r), FUNC(nakajies_state::pcmcia_window7_w));
}


void nakajies_state::nakajies_update_irqs()
{
	// Hack: IRQ mask is temporarily disabled because it doesn't allow the IRQ vectors 0xFA
	// and 0xFB that are used for scanning the kb, this need further investigation.
	uint8_t irq = m_irq_active; // & m_irq_enabled;
	uint8_t vector = 0xff;

	if (LOG)
		logerror("nakajies_update_irqs: irq_enabled = %02x, irq_active = %02x\n", m_irq_enabled, m_irq_active );

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


void nakajies_state::set_irq(u8 data)
{
	m_irq_active |= data;
	nakajies_update_irqs();
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
	m_irq_enabled = data;
	nakajies_update_irqs();
}


/*
  I/O Port a0:
  bit 7   - PCMCIA card absent/not-ready gate (when set)
  bit 6   - PCMCIA SRAM card write-protect candidate
  bit 4   - PCMCIA SRAM card battery status (low when clear and card is present)
  bit 3   - main battery low (when set)
  bit 2   - CR2032 memory-retention battery low (when set)
  bit 1   - Centronics BUSY (when set)
  bit 5,0 - unknown
*/
u8 nakajies_state::unk_a0_r()
{
	u8 data = m_port_status->read() & ~(0xd2);

	if (m_centronics_busy)
		data |= 0x02;
	if (m_pcmcia_battery_voltage_2)
		data |= 0x10;
	if (!m_pcmcia_card_detect && m_pcmcia_write_protect)
		data |= 0x40;
	if (m_pcmcia_card_detect)
		data |= 0x80;

	return data;
}


void nakajies_state::lcd_memory_start_w(u8 data)
{
	m_lcd_memory_start = data;
}


void nakajies_state::banking_w(offs_t offset, u8 data)
{
	m_bank_select[offset] = data;

	const bool card_window = !m_pcmcia_card_detect && BIT(data, 4) && (data & 0x0f) >= 0x08;
	const bool bit3_ram = m_bank_bit3_selects_ram && BIT(data, 3) && !BIT(data, 4);
	const u8 internal_ram_pages = std::max<u8>(1, m_ram_size / 0x20000);
	const u8 ram_entry = bit3_ram ? (offset % internal_ram_pages) : ((data & 0x0f) ^ 0xf);

	m_rombank[offset]->set_entry((data & 0x0f) ^ 0xf);
	m_rambank[offset]->set_entry(ram_entry % internal_ram_pages);
	m_view[offset].select(card_window ? 2 : (BIT(data, 4) || bit3_ram));
}


u8 nakajies_state::pcmcia_memory_r(unsigned window, offs_t offset)
{
	const u32 page = 0x0f - (m_bank_select[window] & 0x0f);
	return m_pcmcia->read_memory_byte((page * 0x20000) + offset);
}


void nakajies_state::pcmcia_memory_w(unsigned window, offs_t offset, u8 data)
{
	const u32 page = 0x0f - (m_bank_select[window] & 0x0f);
	m_pcmcia->write_memory_byte((page * 0x20000) + offset, data);
}


u8 nakajies_state::pcmcia_window0_r(offs_t offset) { return pcmcia_memory_r(0, offset); }
u8 nakajies_state::pcmcia_window1_r(offs_t offset) { return pcmcia_memory_r(1, offset); }
u8 nakajies_state::pcmcia_window2_r(offs_t offset) { return pcmcia_memory_r(2, offset); }
u8 nakajies_state::pcmcia_window3_r(offs_t offset) { return pcmcia_memory_r(3, offset); }
u8 nakajies_state::pcmcia_window4_r(offs_t offset) { return pcmcia_memory_r(4, offset); }
u8 nakajies_state::pcmcia_window5_r(offs_t offset) { return pcmcia_memory_r(5, offset); }
u8 nakajies_state::pcmcia_window6_r(offs_t offset) { return pcmcia_memory_r(6, offset); }
u8 nakajies_state::pcmcia_window7_r(offs_t offset) { return pcmcia_memory_r(7, offset); }


void nakajies_state::pcmcia_window0_w(offs_t offset, u8 data) { pcmcia_memory_w(0, offset, data); }
void nakajies_state::pcmcia_window1_w(offs_t offset, u8 data) { pcmcia_memory_w(1, offset, data); }
void nakajies_state::pcmcia_window2_w(offs_t offset, u8 data) { pcmcia_memory_w(2, offset, data); }
void nakajies_state::pcmcia_window3_w(offs_t offset, u8 data) { pcmcia_memory_w(3, offset, data); }
void nakajies_state::pcmcia_window4_w(offs_t offset, u8 data) { pcmcia_memory_w(4, offset, data); }
void nakajies_state::pcmcia_window5_w(offs_t offset, u8 data) { pcmcia_memory_w(5, offset, data); }
void nakajies_state::pcmcia_window6_w(offs_t offset, u8 data) { pcmcia_memory_w(6, offset, data); }
void nakajies_state::pcmcia_window7_w(offs_t offset, u8 data) { pcmcia_memory_w(7, offset, data); }


void nakajies_state::control_w(u8 data)
{
	m_control = data;

	m_centronics->write_strobe(BIT(data, 5));
	m_usart_clock->set_clock_scale(1.0 / double(1 << (data & 0x07)));
}


void nakajies_state::centronics_ack_w(int state)
{
	if (state)
		set_irq(0x02); // IRQ vector 0xfe: ACK-driven printer byte feeder.
}


void nakajies_state::centronics_busy_w(int state)
{
	m_centronics_busy = state;
}


void nakajies_state::pcmcia_card_detect_w(int state)
{
	m_pcmcia_card_detect = state;
}


void nakajies_state::pcmcia_battery_voltage_2_w(int state)
{
	m_pcmcia_battery_voltage_2 = state;
}


void nakajies_state::pcmcia_write_protect_w(int state)
{
	m_pcmcia_write_protect = state;
}


void nakajies_state::buzzer_update_clock()
{
	const u16 divisor = m_buzzer_low | (m_buzzer_high << 8);

	m_beeper->set_clock(divisor ? (X301 / 64) / divisor : 0);
}


void nakajies_state::buzzer_low_w(u8 data)
{
	m_buzzer_low = data;
	buzzer_update_clock();
}


void nakajies_state::buzzer_high_w(u8 data)
{
	m_buzzer_high = data;
	buzzer_update_clock();
}


void nakajies_state::buzzer_gate_w(u8 data)
{
	m_beeper->set_state(data == 0x7f);
}


void nakajies_state::timer_count_w(u8 data)
{
	m_f9_timer->adjust(data ? attotime::from_ticks(data, X301 / 20480) : attotime::never);
}


u8 nakajies_state::keyboard_r()
{
	return (m_matrix > 0x00) ? m_port_row[m_matrix - 1]->read() : 0;
}


void nakajies_state::nakajies_io_map(address_map &map)
{
	map(0x0000, 0x0000).w(FUNC(nakajies_state::lcd_memory_start_w));
	map(0x0010, 0x0017).w(FUNC(nakajies_state::banking_w));
	map(0x0030, 0x0030).w(FUNC(nakajies_state::control_w));
	map(0x0040, 0x0040).w("cent_data_out", FUNC(output_latch_device::write));
	map(0x0050, 0x0050).w(FUNC(nakajies_state::buzzer_low_w));
	map(0x0051, 0x0051).w(FUNC(nakajies_state::buzzer_high_w));
	map(0x0052, 0x0052).w(FUNC(nakajies_state::buzzer_gate_w));
	map(0x0053, 0x0053).w(FUNC(nakajies_state::timer_count_w));
	map(0x0060, 0x0060).rw(FUNC(nakajies_state::irq_enable_r), FUNC(nakajies_state::irq_enable_w));
	map(0x0090, 0x0090).rw(FUNC(nakajies_state::irq_clear_r), FUNC(nakajies_state::irq_clear_w));
	map(0x00a0, 0x00a0).r(FUNC(nakajies_state::unk_a0_r));
	map(0x00b0, 0x00b0).r(FUNC(nakajies_state::keyboard_r));
	map(0x00c0, 0x00c1).rw(m_upd71051, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x00d0, 0x00df).rw(m_rtc, FUNC(rp5c01_device::read), FUNC(rp5c01_device::write));
}


INPUT_CHANGED_MEMBER(nakajies_state::trigger_irq)
{
	if (newval)
		set_irq(m_port_debug->read());
}


INPUT_CHANGED_MEMBER(nakajies_state::retained_reset)
{
	if (!newval)
		return;

	machine().schedule_soft_reset();
}


static INPUT_PORTS_START(nakajies)
	PORT_START("WAKE")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_NAME("Retained Reset/Wake") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nakajies_state::retained_reset), 0)

	PORT_START("STATUS")
	PORT_BIT(0x21, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_CONFNAME(0x02, 0x00, "Printer BUSY")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x02, DEF_STR(Yes))
	PORT_CONFNAME(0x04, 0x00, "CR2032 Memory Retention Battery")
	PORT_CONFSETTING(0x00, "Normal")
	PORT_CONFSETTING(0x04, "Low")
	PORT_CONFNAME(0x08, 0x00, "Main Battery")
	PORT_CONFSETTING(0x00, "Normal")
	PORT_CONFSETTING(0x08, "Low")
	PORT_CONFNAME(0x10, 0x10, "PCMCIA SRAM Card Battery")
	PORT_CONFSETTING(0x00, "Low")
	PORT_CONFSETTING(0x10, "Normal")
	PORT_CONFNAME(0x40, 0x00, "PCMCIA SRAM Card Write Protect")
	PORT_CONFSETTING(0x00, DEF_STR(Off))
	PORT_CONFSETTING(0x40, DEF_STR(On))
	PORT_CONFNAME(0x80, 0x80, "PCMCIA Card")
	PORT_CONFSETTING(0x00, "Present")
	PORT_CONFSETTING(0x80, DEF_STR(None))

	PORT_START("debug")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_NAME("irq 0xff") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nakajies_state::trigger_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_NAME("irq 0xfe") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nakajies_state::trigger_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_NAME("irq 0xfd") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nakajies_state::trigger_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_NAME("irq 0xfc") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nakajies_state::trigger_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_NAME("irq 0xfb") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nakajies_state::trigger_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_NAME("irq 0xfa") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nakajies_state::trigger_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_NAME("irq 0xf9") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nakajies_state::trigger_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_NAME("irq 0xf8") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nakajies_state::trigger_irq), 0)

	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")  PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LEFT")        PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER")       PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
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
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/")           PORT_CODE(KEYCODE_SLASH) PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H")           PORT_CODE(KEYCODE_H)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N")           PORT_CODE(KEYCODE_N)

	PORT_START("ROW7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("=")           PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7")           PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ORGN")        PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UP")          PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("WP")          PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U")           PORT_CODE(KEYCODE_U) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M")           PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K")           PORT_CODE(KEYCODE_K) PORT_CODE(KEYCODE_2_PAD)

	PORT_START("ROW8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8")           PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-")           PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("]")           PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[")           PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\'")          PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I")           PORT_CODE(KEYCODE_I) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J")           PORT_CODE(KEYCODE_J) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(",")           PORT_CODE(KEYCODE_COMMA) PORT_CODE(KEYCODE_DEL_PAD)

	PORT_START("ROW9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0")           PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9")           PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BACK")        PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P")           PORT_CODE(KEYCODE_P) PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(";")           PORT_CODE(KEYCODE_COLON) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L")           PORT_CODE(KEYCODE_L) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O")           PORT_CODE(KEYCODE_O) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".")           PORT_CODE(KEYCODE_STOP)
INPUT_PORTS_END


void nakajies_state::machine_start()
{
	u32 rom_size = m_rom_region->bytes();

	m_ram_base = make_unique_clear<uint8_t[]>(m_ram_size);
	m_nvram->set_base(m_ram_base.get(), m_ram_size);

	for (int i = 0; i < 8; i++)
	{
		// TODO: rom banks outside max bank size; assuming the banks are simply mirrored
		for (int j = 0; j < 16; j += rom_size / 0x20000)
			m_rombank[i]->configure_entries(j, rom_size / 0x20000, m_rom_region->base(), 0x20000);
		// TODO: ram banks outside max bank size; assuming the banks are simply mirrored
		// how would that work for a system with 160KB RAM?
		for (int j = 0; j < 16; j += m_ram_size / 0x20000)
			m_rambank[i]->configure_entries(j, m_ram_size / 0x20000, &m_ram_base[0], 0x20000);
	}

	m_f9_timer = timer_alloc(FUNC(nakajies_state::f9_timer), this);

	save_item(NAME(m_irq_enabled));
	save_item(NAME(m_irq_active));
	save_item(NAME(m_lcd_memory_start));
	save_item(NAME(m_matrix));
	save_item(NAME(m_control));
	save_item(NAME(m_buzzer_low));
	save_item(NAME(m_buzzer_high));
	save_item(NAME(m_bank_select));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_pcmcia_card_detect));
	save_item(NAME(m_pcmcia_battery_voltage_2));
	save_item(NAME(m_pcmcia_write_protect));
}


void nakajies_state::machine_reset()
{
	m_irq_enabled = 0;
	m_irq_active = 0;
	m_lcd_memory_start = 0;
	m_matrix = 0;
	m_control = 0;
	m_buzzer_low = 0;
	m_buzzer_high = 0;
	std::fill(std::begin(m_bank_select), std::end(m_bank_select), 0);
	m_centronics_busy = 0;
	m_f9_timer->adjust(attotime::never);
	m_beeper->set_clock(0);
	m_beeper->set_state(0);

	for (auto &view : m_view)
		view.select(0);

	for (auto &bank : m_rombank)
		bank->set_entry(0x0f);

	for (auto &bank : m_rambank)
		bank->set_entry(0x0f);
}

u32 nakajies_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t* lcd_memory_start = &m_ram_base[m_lcd_memory_start << 9];
	int height = screen.height();

	for (int y = 0; y < height; y++)
		for (int x = 0; x < 60; x++)
		{
			u8 data = lcd_memory_start[y*64 + x];

			for (int px = 0; px < 8; px++)
			{
				bitmap.pix(y, (x * 8) + px) = BIT(data, 7);
				data <<= 1;
			}
		}

	return 0;
}


TIMER_DEVICE_CALLBACK_MEMBER(nakajies_state::kb_timer)
{
	if (m_matrix > 0x09)
	{
		// reset the keyboard scan
		m_matrix = 0;
		m_irq_active |= 0x20;
	}
	else
	{
		// next row
		m_matrix++;
		m_irq_active |= 0x10;
	}

	nakajies_update_irqs();
}


TIMER_CALLBACK_MEMBER(nakajies_state::f9_timer)
{
	set_irq(0x40); // IRQ vector 0xf9: short timer wake/acknowledge handler.
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
	device.option_add("melcard_1m", PCCARD_SRAM_MITSUBISHI_1M);
	device.option_add("sram_1m", PCCARD_SRAM_CENTENNIAL_1M);
	device.option_add("sram_2m", PCCARD_SRAM_CENTENNIAL_2M);
	device.option_add("sram_4m", PCCARD_SRAM_CENTENNIAL_4M);
}


static DEVICE_INPUT_DEFAULTS_START(pty_defaults)
	DEVICE_INPUT_DEFAULTS("FLOW_CONTROL", 0x07, 0x01)
DEVICE_INPUT_DEFAULTS_END


void nakajies_state::nakajies210(machine_config &config)
{
	V20(config, m_maincpu, X301 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &nakajies_state::nakajies_map);
	m_maincpu->set_addrmap(AS_IO, &nakajies_state::nakajies_io_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);  // Wild guess
	screen.set_screen_update(FUNC(nakajies_state::screen_update));
	screen.set_size(80 * 6, 8 * 8);
	screen.set_visarea(0, 6 * 80 - 1, 0, 8 * 8 - 1);
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_wales210);
	PALETTE(config, "palette", FUNC(nakajies_state::nakajies_palette), 2);

	/* sound */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 0).add_route(ALL_OUTPUTS, "mono", 0.05);

	/* rtc */
	RP5C01(config, m_rtc, XTAL(32'768));
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// NEC uPD71051-compatible USART.
	I8251(config, m_upd71051, 0);
	m_upd71051->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_upd71051->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_upd71051->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_upd71051->rxrdy_handler().set([this](int state) { if (state) set_irq(0x08); });
	m_upd71051->txrdy_handler().set([this](int state) { if (state) set_irq(0x04); });

	clock_device &usart_clock(CLOCK(config, "usart_clock", 19200 * 16));
	usart_clock.signal_handler().set(m_upd71051, FUNC(i8251_device::write_rxc));
	usart_clock.signal_handler().append(m_upd71051, FUNC(i8251_device::write_txc));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.set_option_device_input_defaults("pty", DEVICE_INPUT_DEFAULTS_NAME(pty_defaults));
	rs232.rxd_handler().set(m_upd71051, FUNC(i8251_device::write_rxd));
	rs232.cts_handler().set(m_upd71051, FUNC(i8251_device::write_cts));
	rs232.dsr_handler().set(m_upd71051, FUNC(i8251_device::write_dsr));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(FUNC(nakajies_state::centronics_ack_w));
	m_centronics->busy_handler().set(FUNC(nakajies_state::centronics_busy_w));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	PCCARD_SLOT(config, m_pcmcia, pcmcia_devices, nullptr);
	m_pcmcia->cd1().set(FUNC(nakajies_state::pcmcia_card_detect_w));
	m_pcmcia->bvd2().set(FUNC(nakajies_state::pcmcia_battery_voltage_2_w));
	m_pcmcia->wp().set(FUNC(nakajies_state::pcmcia_write_protect_w));

	TIMER(config, "kb_timer").configure_periodic(FUNC(nakajies_state::kb_timer), attotime::from_hz(X301 / 20480));

	m_ram_size = 128 * 1024;
}

void nakajies_state::dator3k(machine_config &config)
{
	nakajies210(config);
	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_dator3k);
}

void nakajies_state::nakajies220(machine_config &config)
{
	nakajies210(config);
	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_drwrt400);
}

void nakajies_state::nakajies220_t100(machine_config &config)
{
	nakajies220(config);
	m_bank_bit3_selects_ram = true;
}

void nakajies_state::nakajies220_t450(machine_config &config)
{
	nakajies220(config);
	m_ram_size = 256 * 1024;
	m_bank_bit3_selects_ram = true;
}

void nakajies_state::nakajies220_256(machine_config &config)
{
	nakajies220(config);
	m_ram_size = 256 * 1024;
	m_bank_bit3_selects_ram = true;
}

void nakajies_state::nakajies250(machine_config &config)
{
	nakajies210(config);
	subdevice<screen_device>("screen")->set_size(80 * 6, 16 * 8);
	subdevice<screen_device>("screen")->set_visarea(0, 6 * 80 - 1, 0, 16 * 8 - 1);
	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_drwrt200);
	m_ram_size = 256 * 1024;
	m_bank_bit3_selects_ram = true;
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


//    YEAR  NAME      PARENT    COMPAT  MACHINE          INPUT     CLASS           INIT        COMPANY     FULLNAME            FLAGS
COMP( 199?, wales210, 0,        0,      nakajies210,     nakajies, nakajies_state, empty_init, "Walther",  "ES-210",           MACHINE_NOT_WORKING ) // German, 128KB RAM
COMP( 199?, dator3k,  wales210, 0,      dator3k,         nakajies, nakajies_state, empty_init, "Dator",    "Dator 3000",       MACHINE_NOT_WORKING ) // Spanish, 128KB RAM
COMP( 199?, es210_es, wales210, 0,      nakajies210,     nakajies, nakajies_state, empty_init, "Nakajima", "ES-210 (Spain)",   MACHINE_NOT_WORKING ) // Spanish, 128KB RAM
COMP( 199?, drwrt100, wales210, 0,      nakajies220_t100, nakajies, nakajies_state, empty_init, "NTS",      "DreamWriter T100", MACHINE_NOT_WORKING ) // English, 128KB RAM
COMP( 1996, drwrt400, wales210, 0,      nakajies220_256, nakajies, nakajies_state, empty_init, "NTS",      "DreamWriter T400", 0 ) // English, 256KB RAM; built-in store formats as 160KB
COMP( 199?, drwrt450, wales210, 0,      nakajies220_t450, nakajies, nakajies_state, empty_init, "NTS",      "DreamWriter 450",  MACHINE_NOT_WORKING ) // English, 256KB? RAM
COMP( 199?, drwrt200, wales210, 0,      nakajies250,     nakajies, nakajies_state, empty_init, "NTS",      "DreamWriter T200", MACHINE_NOT_WORKING ) // English, 256KB? RAM
