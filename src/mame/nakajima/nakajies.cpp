// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Sandro Ronco
/******************************************************************************

  Driver for the ES-2xx series electronic typewriters made by Nakajima.

Nakajima was the OEM manufacturer for a series of typewriters which were
sold by different brands around the world. The PCB layouts for these
machines are the same. The models differed in the amount of (static) RAM:
128KB or 256KB, presence of PC Card slot or 1.44MB floppy drive, and in the
system rom (mainly only different language support).


Model   |  SRAM | PC Card | FDD   | Language | Branded model
--------+-------+---------+-------+----------+----------------------
ES-210N | 128KB | Yes     | No    | German   | Walther ES-210
ES-210N | 128KB | Yes     | No    | Spanish  | Dator 3000
ES-210N | 128KB | Yes     | No    | English  | NTS DreamWriter 325
ES-220  | 128KB | No      | No    | English  | NTS DreamWriter T100
ES-220  | 256KB | Yes     | No    | English  | NTS DreamWriter T400
ES-250  | 256KB | Yes     | 1.44M | English  | NTS DreamWriter T200


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
- Copy files to and from PC Card Memory card
- PC Card Memory expansion cards available for 60 or 250 pages of text
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
- PC Card Slot
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

0060 - IRQ/source control latch
       Firmware mirrors writes at 6D4F. It appears to use the opposite bit
       order from the 0x90 active/clear latch, and clear bits enable sources.
       The exact source-control hardware is not fully understood.
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
       b7 - PC Card absent/not-ready gate (when set)
       b6 - PC Card SRAM write-protect candidate
       b4 - PC Card SRAM battery status, low when clear and card present
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

Notes:
- drwrt200,drwrt400, and drwrt450 only go up to 512KB to initialize pccard card.
  At least the documentation for the T400 mentions support for 512KB instead of 1MB SRAM card.

TODO:
- Serial port causes source selection to hang on dreamlink
- Floppy support in drwtr200
- centronics ack signal is not checked anywhere?
- Frequency of the keyboard timer is unknown.
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
#include "machine/output_latch.h"
#include "machine/rp5c01.h"
#include "machine/timer.h"
#include "machine/upd765.h"
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
		, m_view{
			{*this, "view_0"}, {*this, "view_1"}, {*this, "view_2"}, {*this, "view_3"},
			{*this, "view_4"}, {*this, "view_5"}, {*this, "view_6"}, {*this, "view_7"} }
		, m_port_row(*this, "ROW%u", 0U)
		, m_port_status(*this, "status")
		, m_rombank(*this, "rombank%u", 0U)
		, m_rambank(*this, "rambank%u", 0U)
		, m_rom_region(*this, "bios")
		, m_nvram(*this, "nvram")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
		, m_uart(*this, "uart")
		, m_uart_clock(*this, "uart_clock")
		, m_serial(*this, "serial")
		, m_pccard(*this, "pccard")
		, m_beeper(*this, "beeper")
	{
	}

	void drwrt100(machine_config &config) ATTR_COLD;
	void nakajies210(machine_config &config) ATTR_COLD;
	void nakajies220(machine_config &config) ATTR_COLD;
	void dator3k(machine_config &config) ATTR_COLD;
	void nakajies220_t100(machine_config &config) ATTR_COLD;
	void nakajies220_t450(machine_config &config) ATTR_COLD;
	void nakajies220_256(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(power_button_nmi);
	DECLARE_INPUT_CHANGED_MEMBER(power_button_irq);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

protected:
	static constexpr int NUMBER_OF_AREAS = 8;
	static constexpr int VIEW_ROM = 0;
	static constexpr int VIEW_RAM = 1;
	static constexpr int VIEW_EXT = 2;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void set_irq(u8 data);
	void nakajies_update_irqs();
	u8 sys_stat_r();
	u8 keyboard_r();
	void keyboard_row_reset(u8 data);
	void banking_w(offs_t offset, u8 data);
	void uart_control_w(u8 data);
	void centronics_busy_w(int state);
	void centronics_ack_w(int state);
	void uart_txrdy_w(int state);
	void uart_rxrdy_w(int state);
	void pccard_card_detect_w(int state) { m_pccard_card_detect = state; }
	void pccard_write_protect_w(int state) { m_pccard_write_protect = state; }
	void pccard_battery_failed_w(int state) { m_pccard_battery_failed = state; }
	int pccard_card_detect_r() { return m_pccard_card_detect; }
	int pccard_write_protect_r() { return m_pccard_write_protect; }
	template<int Bank> u8 pccard_r(offs_t offset);
	template<int Bank> void pccard_w(offs_t offset, u8 data);

	void sample_keyboard_rows();
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
	memory_view m_view[NUMBER_OF_AREAS];
	required_ioport_array<10> m_port_row;
	required_ioport m_port_status;
	memory_bank_array_creator<NUMBER_OF_AREAS> m_rombank;
	memory_bank_array_creator<NUMBER_OF_AREAS> m_rambank;
	required_memory_region m_rom_region;
	required_device<nvram_device> m_nvram;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<i8251_device> m_uart;
	required_device<clock_device> m_uart_clock;
	required_device<rs232_port_device> m_serial;
	optional_device<pccard_slot_device> m_pccard;
	required_device<beep_device> m_beeper;

	emu_timer *m_f9_timer = nullptr;
	u8 m_irq_enabled = 0;
	u8 m_irq_active = 0;
	u8 m_lcd_memory_start = 0;
	u8 m_keyboard_row = 0;
	u8 m_keyboard_row_reset = 0xff;
	std::unique_ptr<u8[]> m_ram_base;
	u32 m_ram_size = 0;
	u8 m_uart_control = 0;
	u8 m_buzzer_low = 0;
	u8 m_buzzer_high = 0;
	u32 m_centronics_busy = 0;
	u32 m_centronics_ack = 0;
	u32 m_uart_rxrdy = 0;
	u32 m_uart_txrdy = 0;
	bool m_lcd_on = true;
	u32 m_pccard_card_detect = 1;
	u32 m_pccard_write_protect = 1;
	u32 m_pccard_battery_failed = 1;
	u8 m_bank_select[NUMBER_OF_AREAS]{};
	bool m_bank_bit3_selects_ram = false;
};


class nakajies_fdc_state : public nakajies_state
{
public:
	nakajies_fdc_state(const machine_config &mconfig, device_type type, const char *tag)
		: nakajies_state(mconfig, type, tag)
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:0")
	{ }

	void nakajies250(machine_config &config) ATTR_COLD;

protected:
	required_device<n82077aa_device> m_fdc;
	required_device<floppy_connector> m_floppy;

	u8 m_fdc_dor = 0;
	u8 m_fdc_cmd[9] = {};
	u8 m_fdc_cmd_len = 0;
	u8 m_fdc_cmd_pos = 0;
	u8 m_fdc_reset_sense_data[2] = {};
	u8 m_fdc_reset_sense_count = 0;
	u8 m_fdc_reset_sense_pos = 0;
	u8 m_fdc_sense_interrupt_pos = 0;
	u16 m_fdc_data_fifo_reads = 0;
	u16 m_fdc_data_fifo_writes = 0;
	bool m_fdc_reset_sense_active = false;
	bool m_fdc_sense_interrupt_active = false;

	void nakajies_io_map_fdc(address_map &map) ATTR_COLD;

	void fdc_dor_w(u8 data);
	u8 fdc_msr_r();
	u8 fdc_fifo_r();
	void fdc_fifo_w(u8 data);
};


void nakajies_state::nakajies_map(address_map &map)
{
	for (int i = 0; i < NUMBER_OF_AREAS; i++)
	{
		const offs_t start = i * 0x20000;
		const offs_t end = start + 0x1ffff;
		map(start, end).view(m_view[i]);
		m_view[i][VIEW_ROM](start, end).bankr(m_rombank[i]);
		m_view[i][VIEW_RAM](start, end).bankrw(m_rambank[i]);
		// Banking to external PC Card space
		m_view[i][VIEW_EXT + 0](start, end).rw(FUNC(nakajies_state::pccard_r<0>), FUNC(nakajies_state::pccard_w<0>));
		m_view[i][VIEW_EXT + 1](start, end).rw(FUNC(nakajies_state::pccard_r<1>), FUNC(nakajies_state::pccard_w<1>));
		m_view[i][VIEW_EXT + 2](start, end).rw(FUNC(nakajies_state::pccard_r<2>), FUNC(nakajies_state::pccard_w<2>));
		m_view[i][VIEW_EXT + 3](start, end).rw(FUNC(nakajies_state::pccard_r<3>), FUNC(nakajies_state::pccard_w<3>));
		m_view[i][VIEW_EXT + 4](start, end).rw(FUNC(nakajies_state::pccard_r<4>), FUNC(nakajies_state::pccard_w<4>));
		m_view[i][VIEW_EXT + 5](start, end).rw(FUNC(nakajies_state::pccard_r<5>), FUNC(nakajies_state::pccard_w<5>));
		m_view[i][VIEW_EXT + 6](start, end).rw(FUNC(nakajies_state::pccard_r<6>), FUNC(nakajies_state::pccard_w<6>));
		m_view[i][VIEW_EXT + 7](start, end).rw(FUNC(nakajies_state::pccard_r<7>), FUNC(nakajies_state::pccard_w<7>));
	}
}


void nakajies_state::nakajies_io_map(address_map &map)
{
	map(0x0000, 0x0000).lw8(
		NAME([this](u8 data) {
			m_lcd_on = true;
			m_lcd_memory_start = data;
		})
	);
	map(0x0010, 0x0017).w(FUNC(nakajies_state::banking_w));
	map(0x30, 0x30).w(FUNC(nakajies_state::uart_control_w));
	map(0x40, 0x40).w(m_cent_data_out, FUNC(output_latch_device::write));
	map(0x0050, 0x0050).w(FUNC(nakajies_state::buzzer_low_w));
	map(0x0051, 0x0051).w(FUNC(nakajies_state::buzzer_high_w));
	map(0x0052, 0x0052).w(FUNC(nakajies_state::buzzer_gate_w));
	map(0x0053, 0x0053).w(FUNC(nakajies_state::timer_count_w));
	map(0x0060, 0x0060).lrw8(
		NAME([this] { return m_irq_enabled; }),
		NAME([this](u8 data) {
			m_irq_enabled = data;
			nakajies_update_irqs();
		})
	);
	map(0x0061, 0x0061).w(FUNC(nakajies_state::keyboard_row_reset));
	map(0x0070, 0x0070).lw8(
		NAME([this](u8 data) {
			// Firmware writes 0x01 here immediately before entering the retained
			// power-off loop.
			if (BIT(data, 0))
			{
				m_lcd_on = false;
				m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			}
		})
	);
	map(0x0090, 0x0090).lw8(
		NAME([this](u8 data) {
			m_irq_active &= ~data;
			nakajies_update_irqs();
		})
	);
	map(0x00a0, 0x00a0).r(FUNC(nakajies_state::sys_stat_r));
	map(0x00b0, 0x00b0).r(FUNC(nakajies_state::keyboard_r));
	map(0x00c0, 0x00c1).rw(m_uart, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x00d0, 0x00df).rw(m_rtc, FUNC(rp5c01_device::read), FUNC(rp5c01_device::write));
}


void nakajies_fdc_state::nakajies_io_map_fdc(address_map &map)
{
	nakajies_io_map(map);
	map(0x00e0, 0x00e0).r(m_fdc, FUNC(n82077aa_device::sra_r));
	map(0x00e1, 0x00e1).r(m_fdc, FUNC(n82077aa_device::srb_r));
	map(0x00e2, 0x00e2).r(m_fdc, FUNC(n82077aa_device::dor_r));
	map(0x00e2, 0x00e2).w(FUNC(nakajies_fdc_state::fdc_dor_w));
	map(0x00e3, 0x00e3).rw(m_fdc, FUNC(n82077aa_device::tdr_r), FUNC(n82077aa_device::tdr_w));
	map(0x00e4, 0x00e4).r(FUNC(nakajies_fdc_state::fdc_msr_r));
	map(0x00e4, 0x00e4).w(m_fdc, FUNC(n82077aa_device::dsr_w));
	map(0x00e5, 0x00e5).rw(FUNC(nakajies_fdc_state::fdc_fifo_r), FUNC(nakajies_fdc_state::fdc_fifo_w));
	map(0x00e7, 0x00e7).rw(m_fdc, FUNC(n82077aa_device::dir_r), FUNC(n82077aa_device::ccr_w));
}


static u8 t200_fdc_command_length(u8 command)
{
	switch (command & 0x1f)
	{
	case 0x03: // specify
	case 0x0f: // seek
		return 3;
	case 0x04: // sense drive status
	case 0x07: // recalibrate
	case 0x0a: // read ID
	case 0x12: // perpendicular mode
		return 2;
	case 0x05: // write data
	case 0x06: // read data
	case 0x09: // write deleted data
	case 0x0c: // read deleted data
		return 9;
	case 0x0d: // format track
		return 6;
	case 0x13: // configure
		return 4;
	default:
		return 1;
	}
}


static u16 t200_fdc_sector_size(u8 n, u8 dtl)
{
	return n ? (128U << n) : dtl;
}


void nakajies_fdc_state::fdc_dor_w(u8 data)
{
	const bool reset_released = !BIT(m_fdc_dor, 2) && BIT(data, 2);

	m_fdc_dor = data;
	m_fdc->dor_w(data);

	if (reset_released)
	{
		// The T200 ROM immediately polls Sense Interrupt Status after
		// releasing reset and expects reset-complete status bytes.
		m_fdc_cmd_len = 0;
		m_fdc_cmd_pos = 0;
		m_fdc_data_fifo_reads = 0;
		m_fdc_data_fifo_writes = 0;
		m_fdc_reset_sense_count = 4;
		m_fdc_reset_sense_pos = 0;
		m_fdc_sense_interrupt_pos = 0;
		m_fdc_reset_sense_active = false;
		m_fdc_sense_interrupt_active = false;
	}
}


u8 nakajies_fdc_state::fdc_msr_r()
{
	if (m_fdc_reset_sense_active)
		return 0xd0; // RQM | DIO | CB: result byte ready.

	return m_fdc->msr_r();
}


u8 nakajies_fdc_state::fdc_fifo_r()
{
	if (m_fdc_reset_sense_active)
	{
		const u8 data = m_fdc_reset_sense_data[m_fdc_reset_sense_pos & 1];

		if (!machine().side_effects_disabled())
		{
			m_fdc_reset_sense_pos++;
			if (m_fdc_reset_sense_pos == 2)
			{
				m_fdc_reset_sense_pos = 0;
				m_fdc_reset_sense_active = false;
			}
		}

		return data;
	}

	if (m_fdc_data_fifo_reads)
	{
		const u8 data = m_fdc->fifo_r();

		if (!machine().side_effects_disabled())
		{
			m_fdc_data_fifo_reads--;

			if (!m_fdc_data_fifo_reads)
			{
				m_fdc->tc_w(true);
				m_fdc->tc_w(false);
			}
		}

		return data;
	}

	u8 data = m_fdc->fifo_r();
	if (m_fdc_sense_interrupt_active)
	{
		if (!m_fdc_sense_interrupt_pos)
		{
			data &= ~0x04;
		}

		if (!machine().side_effects_disabled())
		{
			m_fdc_sense_interrupt_pos++;
			if (data == 0x80 || m_fdc_sense_interrupt_pos == 2)
			{
				m_fdc_sense_interrupt_active = false;
				m_fdc_sense_interrupt_pos = 0;
			}
		}
	}

	return data;
}


void nakajies_fdc_state::fdc_fifo_w(u8 data)
{
	if (data == 0x08 && m_fdc_reset_sense_count)
	{
		m_fdc->fifo_w(data);

		const u8 core_st0 = m_fdc->fifo_r();
		u8 core_pcn = 0x00;
		bool core_has_pcn = false;
		if (core_st0 != 0x80 && (m_fdc->msr_r() & 0xc0) == 0xc0)
		{
			core_pcn = m_fdc->fifo_r();
			core_has_pcn = true;
		}

		m_fdc_reset_sense_data[0] = core_st0 != 0x80 ? core_st0 : (0xc0 | (4 - m_fdc_reset_sense_count));
		m_fdc_reset_sense_data[1] = core_has_pcn ? core_pcn : 0x00;

		m_fdc_reset_sense_count--;
		if (!m_fdc_reset_sense_count && (m_fdc->msr_r() & 0xc0) == 0x80)
		{
			m_fdc->fifo_w(0x08);
			const u8 drain_st0 = m_fdc->fifo_r();
			if (drain_st0 != 0x80 && (m_fdc->msr_r() & 0xc0) == 0xc0)
				m_fdc->fifo_r();
		}
		m_fdc_reset_sense_pos = 0;
		m_fdc_reset_sense_active = true;
		return;
	}

	if (m_fdc_data_fifo_writes)
	{
		m_fdc->fifo_w(data);

		m_fdc_data_fifo_writes--;

		if (!m_fdc_data_fifo_writes)
		{
			m_fdc->tc_w(true);
			m_fdc->tc_w(false);
		}

		return;
	}

	if (!m_fdc_cmd_pos)
	{
		m_fdc_cmd_len = t200_fdc_command_length(data);
		m_fdc_sense_interrupt_active = data == 0x08;
		m_fdc_sense_interrupt_pos = 0;
	}

	m_fdc_cmd[m_fdc_cmd_pos++] = data;

	m_fdc->fifo_w(data);

	if (m_fdc_cmd_pos >= m_fdc_cmd_len)
	{
		switch (m_fdc_cmd[0] & 0x1f)
		{
		case 0x05:
			m_fdc_data_fifo_writes = t200_fdc_sector_size(m_fdc_cmd[5], m_fdc_cmd[8]);
			break;
		case 0x06:
			m_fdc_data_fifo_reads = t200_fdc_sector_size(m_fdc_cmd[5], m_fdc_cmd[8]);
			break;
		case 0x0d:
			m_fdc_data_fifo_writes = m_fdc_cmd[3] * 4;
			break;
		}

		m_fdc_cmd_len = 0;
		m_fdc_cmd_pos = 0;
	}
}


template<int Bank>
u8 nakajies_state::pccard_r(offs_t offset)
{
	if (!m_pccard)
		return 0xff;

	return m_pccard->read_memory_byte((Bank * 0x20000) + offset);
}


template<int Bank>
void nakajies_state::pccard_w(offs_t offset, u8 data)
{
	if (m_pccard)
		m_pccard->write_memory_byte((Bank * 0x20000) + offset, data);
}


void nakajies_state::nakajies_update_irqs()
{
	uint8_t irq = m_irq_active & ~bitswap<8>(m_irq_enabled, 0, 1, 2, 3, 4, 5, 6, 7);
	uint8_t vector = 0xFF;

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


void nakajies_state::set_irq(u8 data)
{
	m_irq_active |= data;
	nakajies_update_irqs();
}


void nakajies_state::sample_keyboard_rows()
{
	for (unsigned row = 0; row < std::size(m_port_row); row++)
		m_ram_base[0x6d06 + row] = m_port_row[row]->read();
}


/*
  System status:
  7------- PC Card present. 0 = present, 1 = no card present.
  -6------ PC Card write protected. 0 = not protected, 1 = write protected.
  --5----- unknown
  ---4---- PC Card battery status. 0 = ok, 1 = battery low/failed.
  ----3--- Battery pack ok. 0 = ok, 1 = low.
  -----2-- Lithium coin battery ok. 0 = ok, 1 = low.
  ------1- centronics busy. when set to 1 no parallel communication is performed.
  -------0 unknown
*/
u8 nakajies_state::sys_stat_r()
{
	return
		(m_pccard_card_detect ? 0x80 : 0x00) |
		(!m_pccard_card_detect && m_pccard_write_protect ? 0x40 : 0x00) |
		(m_pccard_battery_failed ? 0x10 : 0x00) |
		(m_port_status->read() & ~(0xd2)) |
		(m_centronics_busy ? 0x02 : 0x00);
}


void nakajies_state::banking_w(offs_t offset, u8 data)
{
	m_bank_select[offset] = data;

	const u8 internal_ram_pages = std::max<u8>(1, m_ram_size / 0x20000);
	m_rombank[offset]->set_entry((data & 0x0f) ^ 0xf);
	m_rambank[offset]->set_entry(((data & 0x0f) ^ 0xf) % internal_ram_pages);

	if (!BIT(data, 4))
	{
		// ROM or extra RAM
		if (BIT(data, 3))
		{
			if (m_bank_bit3_selects_ram)
			{
				m_rambank[offset]->set_entry(offset % internal_ram_pages);
				m_view[offset].select(VIEW_RAM);
			}
			else if (m_ram_size >= 256 * 1024)
			{
				m_rambank[offset]->set_entry(1);
				m_view[offset].select(VIEW_RAM);
			}
			else
			{
				m_view[offset].disable();
			}
		}
		else
		{
			m_view[offset].select(VIEW_ROM);
		}
	}
	else
	{
		if (!m_pccard.found() || !BIT(data, 3))
		{
			// Internal (S)RAM
			m_view[offset].select(VIEW_RAM);
		}
		else
		{
			// External PC Card space
			m_view[offset].select(VIEW_EXT + (0x0f - (data & 0x0f)));
		}
	}
}


u8 nakajies_state::keyboard_r()
{
	const u8 row = m_keyboard_row;
	return (row > 0) ? m_port_row[row - 1]->read() : 0;
}


void nakajies_state::keyboard_row_reset(u8 data)
{
	// Bit 0 appears to hold/release the external row-scan counter.  The
	// firmware pulses FE->FF when starting a scan and writes FE after repeated
	// empty scans, switching back to the scan-cycle IRQ source.
	const bool was_enabled = BIT(m_keyboard_row_reset, 0);
	const bool enabled = BIT(data, 0);

	m_keyboard_row_reset = data;

	if (enabled && !was_enabled)
		sample_keyboard_rows();

	if (!enabled || !was_enabled)
	{
		m_keyboard_row = 0;
		m_irq_active &= ~0x30;
		nakajies_update_irqs();
	}
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

	const bool reset_uart = BIT(m_uart_control, 3) && !BIT(data, 3);
	m_uart_control = data;

	m_centronics->write_strobe(BIT(data, 5));
	if (reset_uart)
		m_uart->reset();
	m_uart_clock->set_clock_scale(1.0 / double(1 << (data & 0x07)));
}


void nakajies_state::centronics_busy_w(int state)
{
	m_centronics_busy = state;
}


void nakajies_state::centronics_ack_w(int state)
{
	m_centronics_ack = state;

	if (state)
		set_irq(0x02); // IRQ vector 0xfe: ACK-driven printer byte feeder.
}


void nakajies_state::uart_txrdy_w(int state)
{
	if (!m_uart_txrdy && state)
		set_irq(0x04);

	m_uart_txrdy = state;
}


void nakajies_state::uart_rxrdy_w(int state)
{
	if (!m_uart_rxrdy && state)
		set_irq(0x08);

	m_uart_rxrdy = state;
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

static INPUT_PORTS_START(nakajies)
	PORT_START("status")
	PORT_CONFNAME(0x08, 0x00, "Battery pack failed")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x08, DEF_STR(Yes))
	PORT_CONFNAME(0x04, 0x00, "Coin battery Failed")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x04, DEF_STR(Yes))
	PORT_BIT(0x21, IP_ACTIVE_HIGH, IPT_UNKNOWN)

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
	if (!newval)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		return;
	}

	sample_keyboard_rows();

	if (m_lcd_on)
	{
		address_space &program = m_maincpu->space(AS_PROGRAM);
		const auto read_vector = [&program](offs_t address) -> std::pair<u16, u16>
		{
			return std::make_pair(
					program.read_byte(address) | (program.read_byte(address + 1) << 8),
					program.read_byte(address + 2) | (program.read_byte(address + 3) << 8));
		};
		const auto physical = [](u16 segment, u16 offset) -> offs_t { return (u32(segment) << 4) + offset; };

		auto const [nmi_offset, nmi_segment] = read_vector(0x0008);
		auto const [f8_offset, f8_segment] = read_vector(0x03e0);
		u16 f8_handler_offset = f8_offset;
		const offs_t f8_target = physical(f8_segment, f8_offset);

		if (f8_target <= 0xffffd && program.read_byte(f8_target) == 0xe9)
		{
			const s16 displacement = s16(u16(program.read_byte(f8_target + 1) | (program.read_byte(f8_target + 2) << 8)));
			f8_handler_offset = u16(f8_offset + 3 + displacement);
		}

		// The ES-210/Dator/325/T100/T400 v2.1-style ROMs do not route NMI to
		// the power path.  T400 v3.1, T450 and T200 install NMI to the same
		// save/suspend handler reached by IRQ F8.
		if (nmi_segment == f8_segment && nmi_offset == f8_handler_offset)
			m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		else
			set_irq(0x01); // IRQ vector 0xff: warm/power-management source.
	}
	else
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		machine().schedule_soft_reset();
	}
}


INPUT_CHANGED_MEMBER(nakajies_state::power_button_irq)
{
	if (!newval)
		return;

	sample_keyboard_rows();

	if (m_lcd_on)
		set_irq(0x01); // IRQ vector 0xff: warm/power-management source.
	else
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		machine().schedule_soft_reset();
	}
}


void nakajies_state::machine_start()
{
	u32 rom_size = m_rom_region->bytes();

	m_ram_base = make_unique_clear<uint8_t[]>(m_ram_size);
	m_nvram->set_base(m_ram_base.get(), m_ram_size);

	for (int i = 0; i < NUMBER_OF_AREAS; i++)
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
	save_item(NAME(m_lcd_on));
	save_item(NAME(m_keyboard_row));
	save_item(NAME(m_keyboard_row_reset));
	save_item(NAME(m_uart_control));
	save_item(NAME(m_buzzer_low));
	save_item(NAME(m_buzzer_high));
	save_item(NAME(m_bank_select));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_centronics_ack));
	save_item(NAME(m_uart_rxrdy));
	save_item(NAME(m_uart_txrdy));
	save_item(NAME(m_pccard_card_detect));
	save_item(NAME(m_pccard_write_protect));
	save_item(NAME(m_pccard_battery_failed));
}


void nakajies_state::machine_reset()
{
	m_irq_enabled = m_ram_base[0x6d4f];
	m_irq_active = 0;
	m_lcd_memory_start = 0;
	m_lcd_on = true;
	m_keyboard_row_reset = BIT(m_irq_enabled, 3) ? 0xfe : 0xff;
	m_keyboard_row = (BIT(m_keyboard_row_reset, 0) && (m_ram_base[0x6d29] <= 0x09)) ? m_ram_base[0x6d29] : 0;
	m_uart_control = 0;
	m_buzzer_low = 0;
	m_buzzer_high = 0;
	std::fill(std::begin(m_bank_select), std::end(m_bank_select), 0);
	m_centronics_busy = 0;
	m_uart_rxrdy = 0;
	m_uart_txrdy = 0;
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
	if (!m_lcd_on)
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

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
	if (!BIT(m_keyboard_row_reset, 0))
	{
		set_irq(0x20); // IRQ vector 0xfa: scan-cycle/reset source.
		return;
	}

	if (m_keyboard_row > 0x09)
	{
		// trigger reset of keyboard scan
		m_keyboard_row = 0;
		m_irq_active |= 0x20;
	}
	else
	{
		// trigger handling of keyboard row
		m_keyboard_row++;
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


static void pccard_devices(device_slot_interface &device)
{
	device.option_add("melcard_1m", PCCARD_SRAM_MITSUBISHI_1M);
	device.option_add("sram_1m", PCCARD_SRAM_CENTENNIAL_1M);
	device.option_add("sram_2m", PCCARD_SRAM_CENTENNIAL_2M);
	device.option_add("sram_4m", PCCARD_SRAM_CENTENNIAL_4M);
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
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 0).add_route(ALL_OUTPUTS, "mono", 0.05);

	/* rtc */
	RP5C01(config, m_rtc, XTAL(32'768));
	NVRAM(config, m_nvram);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(nakajies_state::centronics_busy_w));
	m_centronics->ack_handler().set(FUNC(nakajies_state::centronics_ack_w));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	// NEC uPD71051-compatible USART.
	I8251(config, m_uart, 0);
	m_uart->txd_handler().set(m_serial, FUNC(rs232_port_device::write_txd));
	m_uart->rts_handler().set(m_serial, FUNC(rs232_port_device::write_rts));
	m_uart->dtr_handler().set(m_serial, FUNC(rs232_port_device::write_dtr));
	m_uart->rxrdy_handler().set(FUNC(nakajies_state::uart_rxrdy_w));
	m_uart->txrdy_handler().set(FUNC(nakajies_state::uart_txrdy_w));

	CLOCK(config, m_uart_clock, 19200 * 16);
	m_uart_clock->signal_handler().set(m_uart, FUNC(i8251_device::write_rxc));
	m_uart_clock->signal_handler().append(m_uart, FUNC(i8251_device::write_txc));

	RS232_PORT(config, m_serial, default_rs232_devices, nullptr);
	m_serial->rxd_handler().set(m_uart, FUNC(i8251_device::write_rxd));
	m_serial->cts_handler().set(m_uart, FUNC(i8251_device::write_cts));
	m_serial->dsr_handler().set(m_uart, FUNC(i8251_device::write_dsr));

	TIMER(config, "kb_timer").configure_periodic(FUNC(nakajies_state::kb_timer), attotime::from_hz(X301 / 20480));

	m_ram_size = 128 * 1024;
}

void nakajies_state::nakajies210(machine_config &config)
{
	drwrt100(config);

	PCCARD_SLOT(config, m_pccard, pccard_devices, nullptr);
	m_pccard->cd1().set(FUNC(nakajies_state::pccard_card_detect_w));
	m_pccard->wp().set(FUNC(nakajies_state::pccard_write_protect_w));
	m_pccard->bvd2().set(FUNC(nakajies_state::pccard_battery_failed_w));
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
}

void nakajies_fdc_state::nakajies250(machine_config &config)
{
	nakajies220_256(config);
	m_maincpu->set_addrmap(AS_IO, &nakajies_fdc_state::nakajies_io_map_fdc);

	m_screen->set_size(80 * 6, 16 * 8);
	m_screen->set_visarea(0, 6 * 80 - 1, 0, 16 * 8 - 1);
	m_gfxdecode->set_info(gfx_drwrt200);

	N82077AA(config, m_fdc, 24_MHz_XTAL, n82077aa_device::mode_t::PS2);  // Actually Intel N82877SL
	FLOPPY_CONNECTOR(config, m_floppy, "35hd", FLOPPY_35_HD, true, floppy_image_device::default_pc_floppy_formats).enable_sound(false);
}

void nakajies_state::nakajies220_t100(machine_config &config)
{
	drwrt100(config);
	m_gfxdecode->set_info(gfx_drwrt400);
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


//    YEAR  NAME      PARENT    COMPAT  MACHINE          INPUT         CLASS           INIT        COMPANY     FULLNAME            FLAGS
COMP( 199?, wales210, 0,        0,      nakajies210,     nakajies_irq, nakajies_state, empty_init, "Walther",  "ES-210",           MACHINE_NOT_WORKING ) // German, 128KB RAM
COMP( 199?, dator3k,  wales210, 0,      dator3k,         nakajies_irq, nakajies_state, empty_init, "Dator",    "Dator 3000",       MACHINE_NOT_WORKING ) // Spanish, 128KB RAM
COMP( 199?, es210_es, wales210, 0,      nakajies210,     nakajies_irq, nakajies_state, empty_init, "Nakajima", "ES-210 (Spain)",   MACHINE_NOT_WORKING ) // Spanish, 128KB RAM
COMP( 199?, drwrt100, wales210, 0,      nakajies220_t100, nakajies_irq, nakajies_state, empty_init, "NTS",      "DreamWriter T100", MACHINE_NOT_WORKING ) // English, 128KB RAM
COMP( 1996, drwrt400, wales210, 0,      nakajies220_256, nakajies_nmi, nakajies_state, empty_init, "NTS",      "DreamWriter T400", 0 ) // English, 256KB RAM; built-in store formats as 160KB
COMP( 199?, drwrt450, wales210, 0,      nakajies220_t450, nakajies_nmi, nakajies_state, empty_init, "NTS",      "DreamWriter 450",  MACHINE_NOT_WORKING ) // English, 256KB? RAM
COMP( 199?, drwrt200, wales210, 0,      nakajies250,     nakajies_nmi, nakajies_fdc_state, empty_init, "NTS",      "DreamWriter T200", MACHINE_NOT_WORKING ) // English, 256KB? RAM
