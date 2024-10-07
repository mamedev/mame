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

0000 - unknown
       0x00 written during boot sequence

0010 - unknown
       0x17 written during boot sequence

0011 - unknown
       0x1e written during boot sequence

0012 - unknown
       0x1f written during boot sequence

0013 - unknown
       0x1e written during boot sequence

0014 - unknown
       0x1d written during boot sequence

0015 - unknown
       0x1c written during boot sequence

0012-0015 - banking?
T200:
Time: 15 - 02; call ADxxx
Database: 15 - 02; call B3xxx
Spreadsheet: 15 - 02; call B9xxx

T450:
Regular boot: 12 - 1f; 13 - 1e; 14 - 1d; 15 - 1c
Typing game: 15 - 02; call B100:0;
Edit Text: 12 - 07; 13 - 06; 14 - 05; 15 - 04; call 4000:0000
c000:0000 - ffff:ffff = not banked?

T400, wales210:
Regular boot: 12 - 1f; 13 - 1e; 14 - 1d; 15 - 1c
Next step during boot: 12 - 17; 13 - 3; 14 - 2; jump to 3000:0000
writing 17 to port 12 maps rom offset 30000 to 3000:0000?

1f, 1e, 1d, 1c is banked RAM ??

banking possibility:
0010-0017 - control banking:
0010 - 00000 - 1ffff
0011 - 20000 - 3ffff
0012 - 40000 - 5ffff
0013 - 60000 - 7ffff
0014 - 80000 - 9ffff
0015 - a0000 - bffff
0016 - c0000 - dffff
0017 - e0000 - fffff

values 00-0f select a rom bank
      00 - selects last 20000h region of rom
      01 - 20000h region before last
      02 - etc

values 10-1f select a ram bank

on reset 0017 is set to 0, pointing to last 20000h bytes of ROM containing the boot setup code


0016 - unknown
       0x01 written during boot sequence

0017 - unknown
       0x00 written during boot sequence

0020 - unknown
       0x00 written during boot sequence

0030 - unknown
       Looking at code at C0769 bit 5 this seems to be used
       as some kind of clock for data that was written to
       I/O port 0040h.

0040 - unknown
       0xff written during boot sequence

Sounds related?
On boot: 50 = 98, 51 = 06, 52 = 7f
         52 = ff
         50 - 26, 51 = 01, 52 = 7f
         52 = ff
         (mem check)
         50 = 98, 51 = 06, 52 = 7f
         52 = ff
         50 = 74, 51 = 04, 52 = 7f
         52 = ff
         50 = 98, 51 = 06, 52 = 7f
         52 = ff
         (menu)
         (type 1 - medium frequency sound )
         50 = 5d, 51 = 01, 52 = 7f
         52 = ff
         52 = ff
         52 = ff
         50 = 00, 51 = 01, 52 = 7f
         52 = ff
         (type 2 - simple lower sound)
         50 = ba, 51 = 02, 52 = 7f
         52 = ff
         (type 3 - highest frequency sound)
         50 = 26, 51 = 01, 52 = 7f
         52 = ff
         50 = 06, 51 = 01, 52 = 7f
         52 = ff
         50 = e9, 51 = 00, 52 = 7f
         52 = ff
         50 = dc, 51 = 00, 52 = 7f
         52 = ff
         50 = c4, 51 = 00, 52 = 7f
         52 = ff
0050 - counter low?
0051 - counter high?
0052 - counter enable/disable?

0060 - Irq enable/disable (?)
       0xff written at start of boot sequence
       0x7e written just before enabling interrupts

0061 - unknown
       0xFE written in irq 0xFB handler

0070 - unknown 0x01 is written when going to terminal mode (enable rs232 receive?)

0090 - Interrupt source clear
       b7 - 1 = clear interrupt source for irq vector 0xf8
       b6 - 1 = clear interrupt source for irq vector 0xf9
       b5 - 1 = clear interrupt source for irq vector 0xfa
       b4 - 1 = clear interrupt source for irq vector 0xfb
       b3 - 1 = clear interrupt source for irq vector 0xfc
       b2 - 1 = clear interrupt source for irq vector 0xfd
       b1 - 1 = clear interrupt source for irq vector 0xfe
       b0 - 1 = clear interrupt source for irq vector 0xff

00A0 - unknown
       Read during initial boot sequence, expects to have bit 3 set at least once during the boot sequence

00D0 - 00DC - Keyboard??

00DD - unknown
       0xf8 written during boot sequence

00DE - unknown
       0xf0 written during boot sequence


IRQ 0xF8:
The handler clears the irq active bit and copies a word from 6D79 to 6D85 and
from 6D7B to 6D87 then enters an endless loop (with interrupts disabled).
Perhaps power on/off related??


IRQ 0xF9: (T400: C049A)
Purpose unknown. IRQ handler clear bit 0 of 6DA9.


IRQ 0xFA: (T400: C04AE)
Purpose unknown. Expects 6D4F to be set up properly. Enables irq 0xFD. Reads
from input port 0xB0 and resets and sets bit 0 of output port 0x61.


IRQ 0xFB: (T400: C04D1)
Purpose unknown. Reads from input port 0xB0, sets bit 7 of 6D28 when
non-zero data was read.


IRQ 0xFC: (T400: C0550)
Purpose unknown. Reads from input port 0xC1 and 0xC0 and possibly outputs
something to output port 0xC1 depending on data read from 0xC1.


IRQ 0xFD: (T400: C0724)
Purpose unknown. Clears bit 3 of 70A5.


IRQ 0xFE: (T400: C0738)
Purpose unknown. Expects 6D4F to be set up properly. Enables irq 0xf9 or outputs
a byte to ports 0x40 and 0x30. No endless loop.


IRQ 0xFF:
This does some simple processing and then enters an endless loop (with interrupts
disabled). Perhaps power on/off related??

TODO:
- On boot, systems do not display version and copyright messages.
- Add PCMCIA slot

******************************************************************************/

#include "emu.h"

#include "cpu/nec/nec.h"
#include "machine/rp5c01.h"
#include "machine/timer.h"
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
		, m_view0(*this, "view0")
		, m_view1(*this, "view1")
		, m_view2(*this, "view2")
		, m_view3(*this, "view3")
		, m_view4(*this, "view4")
		, m_view5(*this, "view5")
		, m_view6(*this, "view6")
		, m_view7(*this, "view7")
		, m_port_row(*this, "ROW%u", 0U)
		, m_port_debug(*this, "debug")
		, m_rombank(*this, "rombank%u", 0U)
		, m_rambank(*this, "rambank%u", 0U)
		, m_rom_region(*this, "bios")
	{
	}

	void nakajies210(machine_config &config);
	void nakajies220(machine_config &config);
	void nakajies220_256(machine_config &config);
	void nakajies250(machine_config &config);
	void dator3k(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(trigger_irq);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void nakajies_update_irqs();
	u8 irq_clear_r();
	void irq_clear_w(u8 data);
	u8 irq_enable_r();
	void irq_enable_w(u8 data);
	u8 unk_a0_r();
	void lcd_memory_start_w(u8 data);
	u8 keyboard_r();
	void banking_w(offs_t offset, u8 data);

	void nakajies_palette(palette_device &palette) const;
	TIMER_DEVICE_CALLBACK_MEMBER(kb_timer);
	void nakajies_io_map(address_map &map) ATTR_COLD;
	void nakajies_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<rp5c01_device> m_rtc;
	memory_view m_view0;
	memory_view m_view1;
	memory_view m_view2;
	memory_view m_view3;
	memory_view m_view4;
	memory_view m_view5;
	memory_view m_view6;
	memory_view m_view7;
	required_ioport_array<10> m_port_row;
	required_ioport m_port_debug;
	memory_bank_array_creator<8> m_rombank;
	memory_bank_array_creator<8> m_rambank;
	required_memory_region m_rom_region;

	u8 m_irq_enabled = 0;
	u8 m_irq_active = 0;
	u8 m_lcd_memory_start = 0;
	u8 m_matrix = 0;
	std::unique_ptr<u8[]> m_ram_base;
	u32 m_ram_size = 0;
};


void nakajies_state::nakajies_map(address_map &map)
{
	map(0x00000, 0x1ffff).view(m_view0);
	m_view0[0](0x00000, 0x1ffff).bankr(m_rombank[0]);
	m_view0[1](0x00000, 0x1ffff).bankrw(m_rambank[0]);
	map(0x20000, 0x3ffff).view(m_view1);
	m_view1[0](0x20000, 0x3ffff).bankr(m_rombank[1]);
	m_view1[1](0x20000, 0x3ffff).bankrw(m_rambank[1]);
	map(0x40000, 0x5ffff).view(m_view2);
	m_view2[0](0x40000, 0x5ffff).bankr(m_rombank[2]);
	m_view2[1](0x40000, 0x5ffff).bankrw(m_rambank[2]);
	map(0x60000, 0x7ffff).view(m_view3);
	m_view3[0](0x60000, 0x7ffff).bankr(m_rombank[3]);
	m_view3[1](0x60000, 0x7ffff).bankrw(m_rambank[3]);
	map(0x80000, 0x9ffff).view(m_view4);
	m_view4[0](0x80000, 0x9ffff).bankr(m_rombank[4]);
	m_view4[1](0x80000, 0x9ffff).bankrw(m_rambank[4]);
	map(0xa0000, 0xbffff).view(m_view5);
	m_view5[0](0xa0000, 0xbffff).bankr(m_rombank[5]);
	m_view5[1](0xa0000, 0xbffff).bankrw(m_rambank[5]);
	map(0xc0000, 0xdffff).view(m_view6);
	m_view6[0](0xc0000, 0xdffff).bankr(m_rombank[6]);
	m_view6[1](0xc0000, 0xdffff).bankrw(m_rambank[6]);
	map(0xe0000, 0xfffff).view(m_view7);
	m_view7[0](0xe0000, 0xfffff).bankr(m_rombank[7]);
	m_view7[1](0xe0000, 0xfffff).bankrw(m_rambank[7]);
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
  bit 7-4 - unknown
  bit 3   - battery low (when set)
  bit 2-0 - unknown
*/
u8 nakajies_state::unk_a0_r()
{
	return 0xf7;
}

void nakajies_state::lcd_memory_start_w(u8 data)
{
	m_lcd_memory_start = data;
}


void nakajies_state::banking_w(offs_t offset, u8 data)
{
	m_rombank[offset]->set_entry((data & 0x0f) ^ 0xf);
	m_rambank[offset]->set_entry((data & 0x0f) ^ 0xf);

	switch (offset)
	{
	case 0: m_view0.select(BIT(data, 4)); break;
	case 1: m_view1.select(BIT(data, 4)); break;
	case 2: m_view2.select(BIT(data, 4)); break;
	case 3: m_view3.select(BIT(data, 4)); break;
	case 4: m_view4.select(BIT(data, 4)); break;
	case 5: m_view5.select(BIT(data, 4)); break;
	case 6: m_view6.select(BIT(data, 4)); break;
	case 7: m_view7.select(BIT(data, 4)); break;
	}
}


u8 nakajies_state::keyboard_r()
{
	return (m_matrix > 0x00) ? m_port_row[m_matrix - 1]->read() : 0;
}


void nakajies_state::nakajies_io_map(address_map &map)
{
	map(0x0000, 0x0000).w(FUNC(nakajies_state::lcd_memory_start_w));
	map(0x0010, 0x0017).w(FUNC(nakajies_state::banking_w));
	map(0x0060, 0x0060).rw(FUNC(nakajies_state::irq_enable_r), FUNC(nakajies_state::irq_enable_w));
	map(0x0090, 0x0090).rw(FUNC(nakajies_state::irq_clear_r), FUNC(nakajies_state::irq_clear_w));
	map(0x00a0, 0x00a0).r(FUNC(nakajies_state::unk_a0_r));
	map(0x00b0, 0x00b0).r(FUNC(nakajies_state::keyboard_r));
	map(0x00d0, 0x00df).rw(m_rtc, FUNC(rp5c01_device::read), FUNC(rp5c01_device::write));
}


INPUT_CHANGED_MEMBER(nakajies_state::trigger_irq)
{
	m_irq_active |= m_port_debug->read();
	nakajies_update_irqs();
}


static INPUT_PORTS_START(nakajies)
	PORT_START("debug")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_NAME("irq 0xff") PORT_CHANGED_MEMBER(DEVICE_SELF, nakajies_state, trigger_irq, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_NAME("irq 0xfe") PORT_CHANGED_MEMBER(DEVICE_SELF, nakajies_state, trigger_irq, 0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_NAME("irq 0xfd") PORT_CHANGED_MEMBER(DEVICE_SELF, nakajies_state, trigger_irq, 0)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_NAME("irq 0xfc") PORT_CHANGED_MEMBER(DEVICE_SELF, nakajies_state, trigger_irq, 0)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_NAME("irq 0xfb") PORT_CHANGED_MEMBER(DEVICE_SELF, nakajies_state, trigger_irq, 0)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_NAME("irq 0xfa") PORT_CHANGED_MEMBER(DEVICE_SELF, nakajies_state, trigger_irq, 0)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_NAME("irq 0xf9") PORT_CHANGED_MEMBER(DEVICE_SELF, nakajies_state, trigger_irq, 0)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_NAME("irq 0xf8") PORT_CHANGED_MEMBER(DEVICE_SELF, nakajies_state, trigger_irq, 0)

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


void nakajies_state::machine_start()
{
	u32 rom_size = m_rom_region->bytes();

	m_ram_base = make_unique_clear<uint8_t[]>(m_ram_size);

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

	save_item(NAME(m_irq_enabled));
	save_item(NAME(m_irq_active));
	save_item(NAME(m_lcd_memory_start));
	save_item(NAME(m_matrix));
}


void nakajies_state::machine_reset()
{
	m_irq_enabled = 0;
	m_irq_active = 0;
	m_lcd_memory_start = 0;
	m_matrix = 0;

	m_view0.select(0);
	m_view1.select(0);
	m_view2.select(0);
	m_view3.select(0);
	m_view4.select(0);
	m_view5.select(0);
	m_view6.select(0);
	m_view7.select(0);
	for (int i = 0; i < 8; i++)
	{
		m_rombank[i]->set_entry(0x0f);
		m_rambank[i]->set_entry(0x0f);
	}
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
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 1.00);

	/* rtc */
	RP5C01(config, m_rtc, XTAL(32'768));

	TIMER(config, "kb_timer").configure_periodic(FUNC(nakajies_state::kb_timer), attotime::from_hz(250));

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

void nakajies_state::nakajies220_256(machine_config &config)
{
	nakajies220(config);
	m_ram_size = 256 * 1024;
}

void nakajies_state::nakajies250(machine_config &config)
{
	nakajies210(config);
	subdevice<screen_device>("screen")->set_size(80 * 6, 16 * 8);
	subdevice<screen_device>("screen")->set_visarea(0, 6 * 80 - 1, 0, 16 * 8 - 1);
	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_drwrt200);
	m_ram_size = 256 * 1024;
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
COMP( 199?, wales210, 0,        0,      nakajies210, nakajies, nakajies_state, empty_init, "Walther",  "ES-210",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // German, 128KB RAM
COMP( 199?, dator3k,  wales210, 0,      dator3k,     nakajies, nakajies_state, empty_init, "Dator",    "Dator 3000",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // Spanish, 128KB RAM
COMP( 199?, es210_es, wales210, 0,      nakajies210, nakajies, nakajies_state, empty_init, "Nakajima", "ES-210 (Spain)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // Spanish, 128KB RAM
COMP( 199?, drwrt100, wales210, 0,      nakajies220, nakajies, nakajies_state, empty_init, "NTS",      "DreamWriter T100", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // English, 128KB RAM
COMP( 1996, drwrt400, wales210, 0,      nakajies220_256, nakajies, nakajies_state, empty_init, "NTS",      "DreamWriter T400", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // English, 256KB RAM, also found a machine with 160KB RAM
COMP( 199?, drwrt450, wales210, 0,      nakajies220, nakajies, nakajies_state, empty_init, "NTS",      "DreamWriter 450",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // English, 128KB RAM
COMP( 199?, drwrt200, wales210, 0,      nakajies250, nakajies, nakajies_state, empty_init, "NTS",      "DreamWriter T200", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // English, 256KB? RAM
