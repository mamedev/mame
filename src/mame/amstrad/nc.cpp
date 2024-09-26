// license: GPL-2.0+
// copyright-holders: Wilbert Pol, Kevin Thacker, Dirk Best
// thanks-to: Cliff Lawson, Russell Marks, Tim Surtel
/***************************************************************************

    Amstrad NC100/NC150/N200 portable computer

    Hardware:
    - Z80 CPU, 4.606000 MHz
    - RAM: 64k (NC100) or 128k (NC150/200)
    - ROM: 256k (NC100) or 512k (NC150/200)
    - Custom ASIC integrating many components
    - LCD screen with 480x64 (NC100/150) or 480x128 (NC200) pixels
    - 2 channel sound (programmable frequency beeps)
    - I8251 compatible UART
    - RTC: TC8521 (NC100/150) or MC146818 (NC200)
    - PCMCIA slot (supports SRAM cards up to 1 MB)
    - 3.5" DD floppy drive, PC compatible FAT format (NC200)
    - 64 key keyboard
    - RS232 and Centronics port
    - Lithium and alkaline batteries

    TODO:
    - Investigate why the FDC IRQ needs to be delayed
    - The dw225 rom image is identical to the nc100 v1.06 rom with the
      last half filled with 0x00. Correct?
    - Computer can be turned off, but not on anymore
    - IRQ 6 is not connected
    - Disk change hardware (disk change is not detected)
    - Verify card status port
    - Artwork

    Notes:
    - Keymap: 'Function' is mapped to 'Left Alt', 'Symbol' is mapped to
      'Right Alt', 'Menu' is mapped to 'Right Control'
    - The system will complain and reset the clock if you don't turn it off
      properly using the 'on/off' key
    - NC100 self test: Turn off, hold Function+Symbol, reset
    - NC200 self test: Turn off, hold Function+Control+Symbol, reset

***************************************************************************/

#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "bus/pccard/sram.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/mc146818.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "machine/rp5c01.h"
#include "machine/timer.h"
#include "machine/upd765.h"
#include "sound/beep.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define LOG_DEBUG   (1U << 1)
#define LOG_IRQ     (1U << 2)

#define VERBOSE (LOG_GENERAL | LOG_DEBUG)
#include "logmacro.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class nc_state : public driver_device
{
public:
	nc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_rombank(*this, "rombank%u", 0U),
		m_rambank(*this, "rambank%u", 0U),
		m_screen(*this, "screen"),
		m_beeper1(*this, "beep.1"),
		m_beeper2(*this, "beep.2"),
		m_centronics(*this, "centronics"),
		m_uart(*this, "uart"),
		m_uart_clock(*this, "uart_clock"),
		m_nvram(*this, "nvram"),
		m_pcmcia(*this, "pcmcia"),
		m_mem_view0(*this, "block0"),
		m_mem_view1(*this, "block1"),
		m_mem_view2(*this, "block2"),
		m_mem_view3(*this, "block3"),
		m_keyboard(*this, "line%d", 0U),
		m_battery(*this, "battery"),
		m_pcmcia_card_detect(1),
		m_pcmcia_write_protect(1),
		m_pcmcia_battery_voltage_1(1),
		m_pcmcia_battery_voltage_2(1)
	{
	}

	int pcmcia_card_detect_r() { return m_pcmcia_card_detect; }
	int pcmcia_write_protect_r() { return m_pcmcia_write_protect; }
	int pcmcia_battery_voltage_r() { return m_pcmcia_battery_voltage_1 && m_pcmcia_battery_voltage_2; }

	void nc_base(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void update_interrupts();

	uint8_t memory_management_r(offs_t offset);
	void memory_management_w(offs_t offset, uint8_t data);
	virtual void uart_control_w(uint8_t data);
	void nc_sound_w(offs_t offset, uint8_t data);
	virtual void poweroff_control_w(uint8_t data);
	uint8_t irq_status_r();

	void centronics_busy_w(int state);

	template<int N> uint8_t pcmcia_r(offs_t offset);
	template<int N> void pcmcia_w(offs_t offset, uint8_t data);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_memory_bank_array<4> m_rombank;
	required_memory_bank_array<4> m_rambank;
	required_device<screen_device> m_screen;
	required_device<beep_device> m_beeper1;
	required_device<beep_device> m_beeper2;
	required_device<centronics_device> m_centronics;
	required_device<i8251_device> m_uart;
	required_device<clock_device> m_uart_clock;
	required_device<nvram_device> m_nvram;
	required_device<pccard_slot_device> m_pcmcia;
	memory_view m_mem_view0;
	memory_view m_mem_view1;
	memory_view m_mem_view2;
	memory_view m_mem_view3;
	required_ioport_array<10> m_keyboard;
	required_ioport m_battery;

	uint8_t m_rom_banks;
	uint8_t m_ram_banks;

	uint16_t m_display_memory_start = 0;
	uint8_t m_mmc[4];
	uint8_t m_uart_control = 0;

	int m_irq_mask = 0;
	int m_irq_status = 0;

	int m_uart_rxrdy;
	int m_uart_txrdy;
	int m_centronics_busy = 0;

	int m_pcmcia_card_detect;
	int m_pcmcia_write_protect;
	int m_pcmcia_battery_voltage_1;
	int m_pcmcia_battery_voltage_2;

	rgb_t m_fg_color;
	rgb_t m_bg_color;

private:
	TIMER_DEVICE_CALLBACK_MEMBER(keyscan_timer);
	void nc_sound_update(int channel);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void pcmcia_card_detect_w(int state) { m_pcmcia_card_detect = state; }
	void pcmcia_write_protect_w(int state) { m_pcmcia_write_protect = state; }
	void pcmcia_battery_voltage_1_w(int state) { m_pcmcia_battery_voltage_1 = state; }
	void pcmcia_battery_voltage_2_w(int state) { m_pcmcia_battery_voltage_2 = state; }

	int m_sound_channel_periods[2]{};
};

class nc100_state : public nc_state
{
public:
	nc100_state(const machine_config &mconfig, device_type type, const char *tag) :
		nc_state(mconfig, type, tag),
		m_centronics_ack(0)
	{
	}

	DECLARE_INPUT_CHANGED_MEMBER( power_button );

	int centronics_ack_r() { return m_centronics_ack; }
	int centronics_busy_r() { return m_centronics_busy; }

	void nc100(machine_config &config);
	void nc150(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void io_map(address_map &map) ATTR_COLD;

	void display_memory_start_w(uint8_t data);
	void card_wait_control_w(uint8_t data);
	void irq_mask_w(uint8_t data);
	void irq_status_w(uint8_t data);
	uint8_t keyboard_r(offs_t offset);

	void uart_txrdy_w(int state);
	void uart_rxrdy_w(int state);
	void centronics_ack_w(int state);

	int m_centronics_ack;
};

class nc200_state : public nc_state
{
public:
	nc200_state(const machine_config &mconfig, device_type type, const char *tag) :
		nc_state(mconfig, type, tag),
		m_rtc(*this, "rtc"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:0")
	{
	}

	DECLARE_INPUT_CHANGED_MEMBER( power_button );

	void nc200(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	required_device<mc146818_device> m_rtc;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_floppy;

	void io_map(address_map &map) ATTR_COLD;

	void display_memory_start_w(uint8_t data);
	void card_wait_control_w(uint8_t data);
	virtual void uart_control_w(uint8_t data) override;
	uint8_t centronics_busy_r();
	void irq_mask_w(uint8_t data);
	virtual void poweroff_control_w(uint8_t data) override;
	void irq_status_w(uint8_t data);
	uint8_t keyboard_r(offs_t offset);

	void fdc_int_w(int state);
	void uart_rxrdy_w(int state);
	void centronics_ack_w(int state);

	emu_timer *m_fdc_irq_timer;
	TIMER_CALLBACK_MEMBER(fdc_irq);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void nc_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).view(m_mem_view0);
	m_mem_view0[0](0x0000, 0x3fff).bankr(m_rombank[0]);
	m_mem_view0[1](0x0000, 0x3fff).bankrw(m_rambank[0]);
	m_mem_view0[2](0x0000, 0x3fff).rw(FUNC(nc_state::pcmcia_r<0>), FUNC(nc_state::pcmcia_w<0>));
	m_mem_view0[3](0x0000, 0x3fff).bankr(m_rombank[0]);
	map(0x4000, 0x7fff).view(m_mem_view1);
	m_mem_view1[0](0x4000, 0x7fff).bankr(m_rombank[1]);
	m_mem_view1[1](0x4000, 0x7fff).bankrw(m_rambank[1]);
	m_mem_view1[2](0x4000, 0x7fff).rw(FUNC(nc_state::pcmcia_r<1>), FUNC(nc_state::pcmcia_w<1>));
	m_mem_view1[3](0x4000, 0x7fff).bankr(m_rombank[1]);
	map(0x8000, 0xbfff).view(m_mem_view2);
	m_mem_view2[0](0x8000, 0xbfff).bankr(m_rombank[2]);
	m_mem_view2[1](0x8000, 0xbfff).bankrw(m_rambank[2]);
	m_mem_view2[2](0x8000, 0xbfff).rw(FUNC(nc_state::pcmcia_r<2>), FUNC(nc_state::pcmcia_w<2>));
	m_mem_view2[3](0x8000, 0xbfff).bankr(m_rombank[2]);
	map(0xc000, 0xffff).view(m_mem_view3);
	m_mem_view3[0](0xc000, 0xffff).bankr(m_rombank[3]);
	m_mem_view3[1](0xc000, 0xffff).bankrw(m_rambank[3]);
	m_mem_view3[2](0xc000, 0xffff).rw(FUNC(nc_state::pcmcia_r<3>), FUNC(nc_state::pcmcia_w<3>));
	m_mem_view3[3](0xc000, 0xffff).bankr(m_rombank[3]);
}

void nc100_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x0f).w(FUNC(nc100_state::display_memory_start_w));
	map(0x10, 0x13).rw(FUNC(nc100_state::memory_management_r), FUNC(nc100_state::memory_management_w));
	map(0x20, 0x20).w(FUNC(nc100_state::card_wait_control_w));
	map(0x30, 0x30).w(FUNC(nc100_state::uart_control_w));
	map(0x40, 0x40).w("cent_data_out", FUNC(output_latch_device::write));
	map(0x50, 0x53).w(FUNC(nc100_state::nc_sound_w));
	map(0x60, 0x60).w(FUNC(nc100_state::irq_mask_w));
	map(0x70, 0x70).w(FUNC(nc100_state::poweroff_control_w));
	map(0x90, 0x90).rw(FUNC(nc100_state::irq_status_r), FUNC(nc100_state::irq_status_w));
	map(0xa0, 0xa0).portr("battery");
	map(0xb0, 0xb9).r(FUNC(nc100_state::keyboard_r));
	map(0xc0, 0xc1).rw(m_uart, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xd0, 0xdf).rw("rtc", FUNC(tc8521_device::read), FUNC(tc8521_device::write));
}

void nc200_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x0f).w(FUNC(nc200_state::display_memory_start_w));
	map(0x10, 0x13).rw(FUNC(nc200_state::memory_management_r), FUNC(nc200_state::memory_management_w));
	map(0x20, 0x20).w(FUNC(nc200_state::card_wait_control_w));
	map(0x30, 0x30).w(FUNC(nc200_state::uart_control_w));
	map(0x40, 0x40).w("cent_data_out", FUNC(output_latch_device::write));
	map(0x50, 0x53).w(FUNC(nc200_state::nc_sound_w));
	map(0x60, 0x60).w(FUNC(nc200_state::irq_mask_w));
	map(0x70, 0x70).w(FUNC(nc200_state::poweroff_control_w));
	map(0x80, 0x80).r(FUNC(nc200_state::centronics_busy_r));
	map(0x90, 0x90).rw(FUNC(nc200_state::irq_status_r), FUNC(nc200_state::irq_status_w));
	map(0xa0, 0xa0).portr("battery");
	map(0xb0, 0xb9).r(FUNC(nc200_state::keyboard_r));
	map(0xc0, 0xc1).rw(m_uart, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xd0, 0xd0).w(m_rtc, FUNC(mc146818_device::address_w));
	map(0xd1, 0xd1).rw(m_rtc, FUNC(mc146818_device::data_r), FUNC(mc146818_device::data_w));
	map(0xe0, 0xe1).m(m_fdc, FUNC(upd765a_device::map));
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( nc100 )
	PORT_START("line0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)         PORT_NAME("Left Shift")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_MAMEKEY(RSHIFT)) PORT_NAME("Right Shift")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)   PORT_CHAR(UCHAR_MAMEKEY(LEFT))   PORT_NAME(u8"\u2190 Word (Red)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)  PORT_CHAR(13)                    PORT_NAME(u8"\u21b2")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("line1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)     PORT_CHAR(UCHAR_MAMEKEY(LALT))     PORT_NAME("Function (Yellow)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_NAME("Control")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)      PORT_CHAR(UCHAR_MAMEKEY(ESC))      PORT_NAME("Stop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)    PORT_CHAR(' ')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)        PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("line2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT)     PORT_CHAR(UCHAR_SHIFT_2)           PORT_NAME("Symbol")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)        PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)      PORT_CHAR('\t')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("line3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR(U'£')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')

	PORT_START("line4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')

	PORT_START("line5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_NAME("y  Y  M+ (Calc)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')

	PORT_START("line6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)         PORT_CHAR('6') PORT_CHAR('^')   PORT_NAME("6  ^  MRC (Calc)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)      PORT_CHAR(UCHAR_MAMEKEY(DOWN))  PORT_NAME(u8"\u2193 Diary (Blue)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)       PORT_CHAR(UCHAR_MAMEKEY(DEL))   PORT_NAME(u8"Del\u2192")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)     PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_NAME(u8"\u2192 Calc (Green)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('#') PORT_CHAR('~')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('/') PORT_CHAR('?')   PORT_NAME("/  ?  + (Calc)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)         PORT_CHAR('h') PORT_CHAR('H')   PORT_NAME("h  H  M- (Calc)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)         PORT_CHAR('n') PORT_CHAR('N')   PORT_NAME("n  N  CE/C (Calc)")

	PORT_START("line7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)      PORT_CHAR('=')  PORT_CHAR('+')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)           PORT_CHAR('7')  PORT_CHAR('&')      PORT_NAME("7  &  7 (Calc)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)          PORT_CHAR(UCHAR_MAMEKEY(UP))        PORT_NAME(u8"\u2191 (White)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL)    PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))  PORT_NAME("Menu  Secret")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)           PORT_CHAR('u')  PORT_CHAR('U')      PORT_NAME("u  U  4 (Calc)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)           PORT_CHAR('m')  PORT_CHAR('M')      PORT_NAME("m  M  0 (Calc)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)           PORT_CHAR('k')  PORT_CHAR('K')      PORT_NAME("k  K  2 (Calc)")

	PORT_START("line8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)           PORT_CHAR('8')  PORT_CHAR('*')  PORT_NAME("8  *  8 (Calc)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)       PORT_CHAR('-')  PORT_CHAR('_')  PORT_NAME(u8"-  _  ± (Calc)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)  PORT_CHAR(']')  PORT_CHAR('}')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR('[')  PORT_CHAR('{')  PORT_NAME("[  {  % (Calc)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR('\'') PORT_CHAR('@')  PORT_NAME(u8"'  @  \u221a (Calc)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)           PORT_CHAR('i')  PORT_CHAR('I')  PORT_NAME("i  I  5 (Calc)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)           PORT_CHAR('j')  PORT_CHAR('J')  PORT_NAME("j  J  1 (Calc)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)       PORT_CHAR(',')  PORT_CHAR('<')  PORT_NAME(",  <  = (Calc)")

	PORT_START("line9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)         PORT_CHAR('0') PORT_CHAR(')') PORT_NAME(u8"0  )  ÷ (Calc)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)         PORT_CHAR('9') PORT_CHAR('(') PORT_NAME("9  (  9 (Calc)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)                  PORT_NAME("Del\u2190")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)         PORT_CHAR('p') PORT_CHAR('P') PORT_NAME(u8"p  P  × (Calc)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(';') PORT_CHAR(':') PORT_NAME(";  :  - (Calc)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)         PORT_CHAR('l') PORT_CHAR('L') PORT_NAME("l  L  3 (Calc)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)         PORT_CHAR('o') PORT_CHAR('O') PORT_NAME("o  O  6 (Calc)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('>') PORT_NAME(".  >  . (Calc)")

	PORT_START("power")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Power On/Off") PORT_CODE(KEYCODE_END) PORT_CHANGED_MEMBER(DEVICE_SELF, nc100_state, power_button, 0)

	PORT_START("battery")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(nc100_state, centronics_ack_r)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(nc100_state, centronics_busy_r)
	PORT_CONFNAME(0x04, 0x00, "Lithium Battery")
	PORT_CONFSETTING(   0x00, "Good")
	PORT_CONFSETTING(   0x04, "Bad")
	PORT_CONFNAME(0x08, 0x00, "Main Battery")
	PORT_CONFSETTING(   0x00, "Good")
	PORT_CONFSETTING(   0x08, "Bad")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(nc100_state, pcmcia_battery_voltage_r)
	PORT_BIT(0x20, 0x00, IPT_UNKNOWN) // input voltage?
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(nc100_state, pcmcia_write_protect_r)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(nc100_state, pcmcia_card_detect_r)
INPUT_PORTS_END

static INPUT_PORTS_START( nc100de )
	PORT_INCLUDE(nc100)

	PORT_MODIFY("line0")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_NAME(u8"\u2190 Text (Red)")

	PORT_MODIFY("line1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)     PORT_CHAR(UCHAR_MAMEKEY(LALT))     PORT_NAME("Funktion (Yellow)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_NAME("Befehl")

	PORT_MODIFY("line3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR(U'§') PORT_CHAR(U'\u207f')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')  PORT_CHAR(U'²')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')  PORT_CHAR('@')

	PORT_MODIFY("line4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') PORT_CHAR(U'£')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('y') PORT_CHAR('Y')

	PORT_MODIFY("line5")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('z') PORT_CHAR('Z') PORT_NAME("z  Z  M+ (Calc)")

	PORT_MODIFY("line6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)         PORT_CHAR('6') PORT_CHAR('&') PORT_CHAR('^') PORT_NAME("6  &  ^  MRC (Calc)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)      PORT_CHAR(UCHAR_MAMEKEY(DOWN))               PORT_NAME(u8"\u2193 Kal/Uhr (Blue)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)       PORT_CHAR(UCHAR_MAMEKEY(DEL))                PORT_NAME(u8"Lösch\u2192")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)     PORT_CHAR(UCHAR_MAMEKEY(RIGHT))              PORT_NAME(u8"\u2192 Rechner (Green)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('#') PORT_CHAR('\'')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('-') PORT_CHAR('_')                PORT_NAME("-  _  + (Calc)")

	PORT_MODIFY("line7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR(U'´') PORT_CHAR('`')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')  PORT_CHAR('/') PORT_CHAR('{')  PORT_NAME("7  /  {  7 (Calc)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('<')  PORT_CHAR('>') PORT_CHAR('|')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))             PORT_NAME(u8"Menü  Geheim")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m')  PORT_CHAR('M') PORT_CHAR(U'μ') PORT_NAME(u8"m  M  μ  0 (Calc)")

	PORT_MODIFY("line8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('(')  PORT_CHAR('[')  PORT_NAME("8  (  [  8 (Calc)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR(U'ß') PORT_CHAR('?')  PORT_CHAR('\\') PORT_NAME(u8"ß  ?  \\  ± (Calc)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('+')  PORT_CHAR('*')  PORT_CHAR('~')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(U'ü') PORT_CHAR(U'Ü')                 PORT_NAME(u8"ü  Ü  % (Calc)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(U'ä') PORT_CHAR(U'Ä')                 PORT_NAME(u8"ä  Ä  \u221a (Calc)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR(';')                  PORT_NAME(",  ;  = (Calc)")

	PORT_MODIFY("line9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)         PORT_CHAR('0')  PORT_CHAR('=')  PORT_CHAR('}') PORT_NAME("0  =  }  ÷ (Calc)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)         PORT_CHAR('9')  PORT_CHAR(')')  PORT_CHAR(']') PORT_NAME("9  )  ]  9 (Calc)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)                                   PORT_NAME(u8"Lösch\u2190")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(U'ö') PORT_CHAR(U'Ö')                PORT_NAME(u8"ö  Ö  - (Calc)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.')  PORT_CHAR(':')                 PORT_NAME(".  :  . (Calc)")
INPUT_PORTS_END

static INPUT_PORTS_START( nc100dk )
	PORT_INCLUDE(nc100)

	PORT_MODIFY("line0")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_NAME(u8"\u2190 Ord (Red)")

	PORT_MODIFY("line1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)     PORT_CHAR(UCHAR_MAMEKEY(LALT))     PORT_NAME("Funktion (Yellow)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_NAME("Kontrol")

	PORT_MODIFY("line2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_NAME("Versaler")

	PORT_MODIFY("line3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') PORT_CHAR(U'£')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"') PORT_CHAR('@')

	PORT_MODIFY("line6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)         PORT_CHAR('6')  PORT_CHAR('&')  PORT_NAME("6  &  MRC (Calc)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)      PORT_CHAR(UCHAR_MAMEKEY(DOWN))  PORT_NAME(u8"\u2193 Kalend. (Blue)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)       PORT_CHAR(UCHAR_MAMEKEY(DEL))   PORT_NAME(u8"Slet\u2192")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)     PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_NAME(u8"\u2192 Regn. (Green)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\'') PORT_CHAR('*')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('-')  PORT_CHAR('_')  PORT_NAME("-  _  + (Calc)")

	PORT_MODIFY("line7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR(U'´') PORT_CHAR('`') PORT_CHAR('|')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')  PORT_CHAR('/') PORT_CHAR('{')  PORT_NAME("7  /  {  7 (Calc)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('<')  PORT_CHAR('>') PORT_CHAR('\\')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))             PORT_NAME(u8"Menu  Skjult")

	PORT_MODIFY("line8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('(')  PORT_CHAR('[') PORT_NAME("8  (  [  8 (Calc)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('+')  PORT_CHAR('?')                 PORT_NAME(u8"+  ?  ± (Calc)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(U'¨') PORT_CHAR('^')  PORT_CHAR('~')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(U'å') PORT_CHAR(U'Å')                PORT_NAME(u8"å  Å  % (Calc)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(U'ø') PORT_CHAR(U'Ø')                PORT_NAME(u8"ø  Ø  \u221a (Calc)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR(';')                 PORT_NAME(",  ;  = (Calc)")

	PORT_MODIFY("line9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)         PORT_CHAR('0')  PORT_CHAR('=')  PORT_CHAR('}') PORT_NAME("0  =  }  ÷ (Calc)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)         PORT_CHAR('9')  PORT_CHAR(')')  PORT_CHAR(']') PORT_NAME("9  )  ]  9 (Calc)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)                                   PORT_NAME(u8"Slet\u2190")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(U'æ') PORT_CHAR(U'Æ')                PORT_NAME(u8"æ  Æ  - (Calc)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.')  PORT_CHAR(':')                 PORT_NAME(".  :  . (Calc)")
INPUT_PORTS_END

static INPUT_PORTS_START( nc100sv )
	PORT_INCLUDE(nc100)

	PORT_MODIFY("line0")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_NAME(u8"\u2190 Text (Red)") // label on picture unclear

	PORT_MODIFY("line1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)     PORT_CHAR(UCHAR_MAMEKEY(LALT))     PORT_NAME("Funktion (Yellow)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_NAME("Kontroll")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)      PORT_CHAR(UCHAR_MAMEKEY(ESC))      PORT_NAME("Stopp")

	PORT_MODIFY("line2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_NAME(u8"Versallås")

	PORT_MODIFY("line3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') PORT_CHAR(U'£')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"') PORT_CHAR('@')

	PORT_MODIFY("line6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)         PORT_CHAR('6')  PORT_CHAR('&')  PORT_NAME("6  &  MRC (Calc)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)      PORT_CHAR(UCHAR_MAMEKEY(DOWN))  PORT_NAME(u8"\u2193 Kalend. (Blue)") // label on picture unclear
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)       PORT_CHAR(UCHAR_MAMEKEY(DEL))   PORT_NAME(u8"Rensa\u2192")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)     PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_NAME(u8"\u2192 Calc (Green)") // label on picture unclear
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\'') PORT_CHAR('*')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('-')  PORT_CHAR('_')  PORT_NAME("-  _  + (Calc)")

	PORT_MODIFY("line7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR(U'´') PORT_CHAR('`')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')  PORT_CHAR('/') PORT_CHAR('{') PORT_NAME("7  /  {  7 (Calc)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('<')  PORT_CHAR('>') PORT_CHAR('|')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))            PORT_NAME(u8"Menu  Hemlig") // label on picture unclear

	PORT_MODIFY("line8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('(')  PORT_CHAR('[') PORT_NAME("8  (  [  8 (Calc)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('+')  PORT_CHAR('?')                 PORT_NAME(u8"+  ?  ± (Calc)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(U'¨') PORT_CHAR('^')  PORT_CHAR('~')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(U'å') PORT_CHAR(U'Å')                PORT_NAME(u8"å  Å  % (Calc)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(U'ä') PORT_CHAR(U'Ä')                PORT_NAME(u8"ä  Ä  \u221a (Calc)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR(';')                 PORT_NAME(",  ;  = (Calc)")

	PORT_MODIFY("line9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)         PORT_CHAR('0')  PORT_CHAR('=') PORT_CHAR('}') PORT_NAME("0  =  }  ÷ (Calc)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)         PORT_CHAR('9')  PORT_CHAR(')') PORT_CHAR(']') PORT_NAME("9  )  ]  9 (Calc)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)                                  PORT_NAME(u8"Rensa\u2190")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(U'ö') PORT_CHAR(U'Ö')               PORT_NAME(u8"ö  Ö  - (Calc)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.')  PORT_CHAR(':')                PORT_NAME(".  :  . (Calc)")
INPUT_PORTS_END

static INPUT_PORTS_START( nc150fr )
	PORT_INCLUDE(nc100)

	PORT_MODIFY("line0")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_NAME(u8"\u2190 Texte (Red)")

	PORT_MODIFY("line1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))               PORT_NAME("Fonction (Yellow)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)    PORT_CHAR('(') PORT_CHAR('5') PORT_CHAR('[')

	PORT_MODIFY("line2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_NAME("Maj/Min")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)        PORT_CHAR('&') PORT_CHAR('1')

	PORT_MODIFY("line3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('"')  PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR(U'é') PORT_CHAR('2') PORT_CHAR('~')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('a')  PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('z')  PORT_CHAR('Z')

	PORT_MODIFY("line4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR(U'´') PORT_CHAR('4') PORT_CHAR('{')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('w')  PORT_CHAR('W')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('q')  PORT_CHAR('Q')

	PORT_MODIFY("line6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)         PORT_CHAR('-') PORT_CHAR('6')  PORT_CHAR('|') PORT_NAME("-  6  |  MRC (Calc)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)      PORT_CHAR(UCHAR_MAMEKEY(DOWN))                PORT_NAME(u8"\u2193 Agenda (Blue)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)       PORT_CHAR(UCHAR_MAMEKEY(DEL))                 PORT_NAME(u8"Eff\u2192")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('*') PORT_CHAR(U'μ')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('!') PORT_CHAR(U'§')                PORT_NAME(u8"!  §  + (Calc)")

	PORT_MODIFY("line7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=')  PORT_CHAR('+') PORT_CHAR('}')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR(U'è') PORT_CHAR('7') PORT_CHAR('`') PORT_NAME(u8"è  7  `  7 (Calc)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('<')  PORT_CHAR('>')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))                  PORT_NAME(u8"\u2191 Table (White)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR(',')  PORT_CHAR('?')                PORT_NAME(",  ?  0 (Calc)")

	PORT_MODIFY("line8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('_')  PORT_CHAR('8')  PORT_CHAR('\\') PORT_NAME("_  8  \\  8 (Calc)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR(')')  PORT_CHAR(U'°') PORT_CHAR(']')  PORT_NAME(u8")  °  ]  ± (Calc)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('$')  PORT_CHAR(U'£')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('^')  PORT_CHAR(U'¨')                 PORT_NAME(u8"^  ¨  % (Calc)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(U'ù') PORT_CHAR('%')                  PORT_NAME(u8"ù  %  \u221a (Calc)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(':')  PORT_CHAR('/')                  PORT_NAME(":  /  = (Calc)")

	PORT_MODIFY("line9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)         PORT_CHAR(U'à') PORT_CHAR('0') PORT_CHAR('@') PORT_NAME(u8"à  0  @  ÷ (Calc)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)         PORT_CHAR(U'ç') PORT_CHAR('9') PORT_CHAR('^') PORT_NAME(u8"ç  9  ^  9 (Calc)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)                                  PORT_NAME(u8"Eff\u2190")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)     PORT_CHAR('m')  PORT_CHAR('M')                PORT_NAME(u8"m  M  - (Calc)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR(';')  PORT_CHAR('.')                PORT_NAME(";  .  . (Calc)")
INPUT_PORTS_END

static INPUT_PORTS_START( nc150it )
	PORT_INCLUDE(nc100)

	PORT_MODIFY("line0")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_NAME(u8"\u2190 Testi (Red)")

	PORT_MODIFY("line1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)     PORT_CHAR(UCHAR_MAMEKEY(LALT))     PORT_NAME("Funzione (Yellow)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_NAME("CTRL")

	PORT_MODIFY("line2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_SHIFT_2) PORT_NAME("Simboli")

	PORT_MODIFY("line6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)         PORT_CHAR('6')  PORT_CHAR('&')  PORT_NAME("6  &  MRC (Calc)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)      PORT_CHAR(UCHAR_MAMEKEY(DOWN))  PORT_NAME(u8"\u2193 Agenda (Blue)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)       PORT_CHAR(UCHAR_MAMEKEY(DEL))   PORT_NAME(u8"Canc\u2192")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(U'ù') PORT_CHAR(U'§')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('-')  PORT_CHAR('_')  PORT_NAME("-  _  + (Calc)")

	PORT_MODIFY("line7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR(U'ì') PORT_CHAR('^')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')  PORT_CHAR('/') PORT_NAME("7  /  7 (Calc)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('<')  PORT_CHAR('>')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))   PORT_NAME(u8"\u2191 Tabella. El. (White)")

	PORT_MODIFY("line8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('(')                 PORT_NAME("8  (  8 (Calc)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('\'') PORT_CHAR('?')                 PORT_NAME(u8"'  ?  ± (Calc)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('+')  PORT_CHAR('*')  PORT_CHAR(']')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(U'è') PORT_CHAR(U'é') PORT_CHAR('[') PORT_NAME("è  é  [  % (Calc)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(U'à') PORT_CHAR(U'°') PORT_CHAR('#') PORT_NAME(u8"à  °  #  \u221a (Calc)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR(';')                 PORT_NAME(",  ;  = (Calc)")

	PORT_MODIFY("line9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)         PORT_CHAR('0')  PORT_CHAR('=')  PORT_NAME(u8"0  =  ÷ (Calc)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)         PORT_CHAR('9')  PORT_CHAR(')')  PORT_NAME("9  )  9 (Calc)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)                    PORT_NAME("Canc\u2190")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(U'ò') PORT_CHAR(U'ç') PORT_NAME(u8"ò  ç  - (Calc)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.')  PORT_CHAR(':')  PORT_NAME(".  :  . (Calc)")
INPUT_PORTS_END

static INPUT_PORTS_START( nc200 )
	PORT_INCLUDE(nc100)

	PORT_MODIFY("line0")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')

	PORT_MODIFY("line1")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(') PORT_NAME("9  (  9 (Calc)")

	PORT_MODIFY("line2")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^') PORT_NAME("6  ^  MRC (Calc)")

	PORT_MODIFY("line4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*') PORT_NAME("8  *  8 (Calc)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&') PORT_NAME("7  &  7 (Calc)")

	PORT_MODIFY("line6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("line7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')

	PORT_MODIFY("line8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("line9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')') PORT_NAME(u8"0  )  ÷ (Calc)")

	PORT_MODIFY("power")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Power On/Off") PORT_CODE(KEYCODE_END) PORT_CHANGED_MEMBER(DEVICE_SELF, nc200_state, power_button, 0)

	PORT_MODIFY("battery")
	PORT_CONFNAME(0x01, 0x00, "Battery for Floppy Drive")
	PORT_CONFSETTING(   0x00, "Good")
	PORT_CONFSETTING(   0x01, "Bad")
	PORT_BIT(0x02, 0x00, IPT_UNKNOWN)
	PORT_CONFNAME(0x04, 0x00, "Main Battery")
	PORT_CONFSETTING(   0x00, "Good")
	PORT_CONFSETTING(   0x04, "Bad")
	PORT_BIT(0x08, 0x00, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(nc200_state, pcmcia_battery_voltage_r)
	PORT_CONFNAME(0x20, 0x00, "Lithium Battery")
	PORT_CONFSETTING(   0x00, "Good")
	PORT_CONFSETTING(   0x20, "Bad")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(nc200_state, pcmcia_write_protect_r)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(nc200_state, pcmcia_card_detect_r)
INPUT_PORTS_END


//**************************************************************************
//  INPUT HANDLING
//**************************************************************************

INPUT_CHANGED_MEMBER( nc100_state::power_button )
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? ASSERT_LINE : CLEAR_LINE);
}

INPUT_CHANGED_MEMBER( nc200_state::power_button )
{
	m_irq_status |= 0x10;
	update_interrupts();
}

uint8_t nc100_state::keyboard_r(offs_t offset)
{
	if (offset == 9)
	{
		m_irq_status &= ~0x08;
		update_interrupts();
	}

	return m_keyboard[offset]->read();
}

uint8_t nc200_state::keyboard_r(offs_t offset)
{
	return m_keyboard[offset]->read();
}

TIMER_DEVICE_CALLBACK_MEMBER( nc_state::keyscan_timer )
{
	m_irq_status |= 0x08;
	update_interrupts();
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void nc100_state::display_memory_start_w(uint8_t data)
{
	// 7654----  a15 to a12 of display memory start address
	// ----3210  not used

	LOG("display_memory_start_w: %04x\n", data);

	m_display_memory_start = BIT(data, 4, 4) << 12;
}

void nc200_state::display_memory_start_w(uint8_t data)
{
	// 765-----  a15 to a13 of display memory start address
	// ---43210  not used

	LOG("display_memory_start_w: %04x\n", data);

	m_display_memory_start = BIT(data, 5, 3) << 13;
}

uint32_t nc_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y <= cliprect.max_y; y++)
	{
		// 64 bytes/line
		uint8_t const *line = reinterpret_cast<uint8_t const *>(m_ram->pointer()) + m_display_memory_start + (y << 6);

		for (int x = 0; x <= cliprect.max_x; x+= 8)
		{
			for (int i = 0; i < 8; i++)
				bitmap.pix(y, x + i) = BIT(*line, 7 - i) ? m_fg_color : m_bg_color;

			line++;
		}
	}

	return 0;
}


//**************************************************************************
//  AUDIO EMULATION
//**************************************************************************

void nc_state::nc_sound_update(int channel)
{
	channel &= 1;
	beep_device *beeper_device = channel ? m_beeper2 : m_beeper1;

	int period = m_sound_channel_periods[channel];

	/* if top bit is 0, sound is on */
	int on = ((period & (1<<15))==0);

	/* calculate frequency from period */
	int frequency = (int)(1000000.0f/((float)((period & 0x07fff)<<1) * 1.6276f));

	/* set state */
	beeper_device->set_state(on);
	/* set frequency */
	beeper_device->set_clock(frequency);
}

void nc_state::nc_sound_w(offs_t offset, uint8_t data)
{
	LOG("sound w: %04x %02x\n", offset, data);

	switch (offset)
	{
		case 0x0:
		{
			/* update period value */
			m_sound_channel_periods[0]  =
				(m_sound_channel_periods[0] & 0x0ff00) | (data & 0x0ff);

			nc_sound_update(0);
		}
		break;

		case 0x01:
		{
			m_sound_channel_periods[0] =
				(m_sound_channel_periods[0] & 0x0ff) | ((data & 0x0ff)<<8);

			nc_sound_update(0);
		}
		break;

		case 0x02:
		{
			/* update period value */
			m_sound_channel_periods[1]  =
				(m_sound_channel_periods[1] & 0x0ff00) | (data & 0x0ff);

			nc_sound_update(1);
		}
		break;

		case 0x03:
		{
			m_sound_channel_periods[1] =
				(m_sound_channel_periods[1] & 0x0ff) | ((data & 0x0ff)<<8);

			nc_sound_update(1);
		}
		break;

		default:
			break;
	}
}


//**************************************************************************
//  PCMCIA
//**************************************************************************

template<int N>
uint8_t nc_state::pcmcia_r(offs_t offset)
{
	if (BIT(m_uart_control, 7) == 1)
		return m_pcmcia->read_memory_byte(((m_mmc[N] & 0x3f) << 14) | offset);
	else
		return m_pcmcia->read_reg_byte(((m_mmc[N] & 0x3f) << 14) | offset);
}

template<int N>
void nc_state::pcmcia_w(offs_t offset, uint8_t data)
{
	if (BIT(m_uart_control, 7) == 1)
		m_pcmcia->write_memory_byte(((m_mmc[N] & 0x3f) << 14) | offset, data);
	else
		m_pcmcia->write_reg_byte(((m_mmc[N] & 0x3f) << 14) | offset, data);
}

void nc100_state::card_wait_control_w(uint8_t data)
{
	// 7-------  enable memory wait states for card
	// -6543210  not used

	LOG("card_wait_control_w: %02x\n", data);
}

void nc200_state::card_wait_control_w(uint8_t data)
{
	// 7-------  enable memory wait states for card
	// -6543---  not used
	// -----2--  floppy motor
	// ------1-  unknown (floppy related?)
	// -------0  fdc terminal count

	LOG("card_wait_control_w: %02x\n", data);

	floppy_image_device *floppy = m_floppy->get_device();

	if (floppy)
		floppy->mon_w(BIT(data, 2));

	m_fdc->tc_w(BIT(data, 0));
}


//**************************************************************************
//  CENTRONICS
//**************************************************************************

void nc_state::centronics_busy_w(int state)
{
	m_centronics_busy = state;
}

uint8_t nc200_state::centronics_busy_r()
{
	// 7654321-  not used
	// -------0  centronics busy

	return m_centronics_busy;
}

void nc100_state::centronics_ack_w(int state)
{
	LOGMASKED(LOG_IRQ, "centronics_ack_w: %02x\n", state);

	m_centronics_ack = state;

	if (state)
		m_irq_status |= 0x04;

	update_interrupts();
}

void nc200_state::centronics_ack_w(int state)
{
	LOGMASKED(LOG_IRQ, "centronics_ack_w: %02x\n", state);

	if (state)
		m_irq_status |= 0x01;

	update_interrupts();
}


//**************************************************************************
//  UART
//**************************************************************************

void nc_state::uart_control_w(uint8_t data)
{
	// 7-------  card register space
	// -6------  centronics strobe
	// --5-----  not used
	// ---4----  line driver on/off
	// ----3---  uart clock/reset
	// -----210  baud rate: 150, 300, 600, 1200, 2400, 4800, 9600, 19200

	LOG("uart_control_w: %02x\n", data);

	m_centronics->write_strobe(BIT(data, 6));

	if (BIT(m_uart_control, 3) == 1 && BIT(data, 3) == 0)
		m_uart->reset();

	m_uart_clock->set_clock_scale(1 << (data & 0x07));

	m_uart_control = data;
}

void nc200_state::uart_control_w(uint8_t data)
{
	// 76------  see above
	// --5-----  floppy related?
	// ---43210  see above

	LOG("uart_control_w: %02x\n", data);

	nc_state::uart_control_w(data);
}

void nc100_state::uart_txrdy_w(int state)
{
	LOGMASKED(LOG_IRQ, "uart_txrdy_w: %02x\n", state);

	if (m_uart_txrdy == 0 && state == 1)
		m_irq_status |= 0x02;

	update_interrupts();

	m_uart_txrdy = state;
}

void nc100_state::uart_rxrdy_w(int state)
{
	LOGMASKED(LOG_IRQ, "uart_rxrdy_w: %02x\n", state);

	if (m_uart_rxrdy == 0 && state == 1)
		m_irq_status |= 0x01;

	update_interrupts();

	m_uart_rxrdy = state;
}

void nc200_state::uart_rxrdy_w(int state)
{
	LOGMASKED(LOG_IRQ, "uart_rxrdy_w: %02x\n", state);

	if (m_uart_rxrdy == 0 && state == 1)
		m_irq_status |= 0x04;

	update_interrupts();

	m_uart_rxrdy = state;
}


//**************************************************************************
//  FLOPPY
//**************************************************************************

void nc200_state::fdc_int_w(int state)
{
	LOGMASKED(LOG_IRQ, "fdc_int_w: %02x\n", state);

	if (state)
		m_fdc_irq_timer->adjust(attotime::from_usec(100), 0);
}

TIMER_CALLBACK_MEMBER( nc200_state::fdc_irq )
{
	LOGMASKED(LOG_IRQ, "fdc_irq assert\n");

	m_irq_status |= 0x20;
	update_interrupts();
}


//**************************************************************************
//  INTERRUPTS
//**************************************************************************

void nc_state::update_interrupts()
{
	LOGMASKED(LOG_IRQ, "update_interrupts: %02x & %02x\n", m_irq_status, m_irq_mask);

	if (m_irq_status & m_irq_mask)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	else
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

uint8_t nc_state::irq_status_r()
{
	return m_irq_status ^ 0xff;
}

void nc100_state::irq_status_w(uint8_t data)
{
	// 7654----  not used
	// ----3---  key scan interrupt
	// -----2--  ack from centronics
	// ------1-  tx ready from uart
	// -------0  rx ready from uart

	LOGMASKED(LOG_IRQ, "irq_status_w: %02x\n", data);

	m_irq_status &= data;
}

void nc200_state::irq_status_w(uint8_t data)
{
	// 7-------  not used
	// -6------  ?
	// --5-----  fdc interrupt
	// ---4----  power off interrupt
	// ----3---  key scan interrupt
	// -----2--  rx ready from uart
	// ------1-  not used
	// -------0  ack from centronics

	LOGMASKED(LOG_IRQ, "irq_status_w: %02x\n", data);

	m_irq_status &= data;
	update_interrupts();
}

void nc100_state::irq_mask_w(uint8_t data)
{
	// see irq_status_w for bit assignment

	LOGMASKED(LOG_IRQ, "irq_mask_w: %02x\n", data);

	m_irq_mask = data;
	update_interrupts();
}

void nc200_state::irq_mask_w(uint8_t data)
{
	// see irq_status_w for bit assignment

	LOGMASKED(LOG_IRQ, "irq_mask_w: %02x\n", data);

	m_irq_mask = data;
	update_interrupts();
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void nc_state::poweroff_control_w(uint8_t data)
{
	// 7654321-  not used
	// -------0  power off

	LOG("poweroff_control_w: %02x\n", data);
}

void nc200_state::poweroff_control_w(uint8_t data)
{
	// 76543---  not used
	// -----2--  backlight
	// ------1-  floppy?
	// -------0  power off

	LOG("poweroff_control_w: %02x\n", data);

	nc_state::poweroff_control_w(data);

	m_fg_color = BIT(data, 2) ? rgb_t(0x2b, 0x42, 0x66) : rgb_t(0x0c, 0x54, 0x9f);
	m_bg_color = BIT(data, 2) ? rgb_t(0xae, 0xa0, 0x66) : rgb_t(0xbe, 0xb7, 0x94);
}

uint8_t nc_state::memory_management_r(offs_t offset)
{
	return m_mmc[offset];
}

void nc_state::memory_management_w(offs_t offset, uint8_t data)
{
	// 76------  memory mode select
	// --543210  address lines 19 to 14

	m_mmc[offset] = data;

	memory_view *const mem_view[4] = { &m_mem_view0, &m_mem_view1, &m_mem_view2, &m_mem_view3 };
	mem_view[offset]->select(BIT(m_mmc[offset], 6, 2));
	m_rombank[offset]->set_entry(m_mmc[offset] & 0x3f & (m_rom_banks - 1));
	m_rambank[offset]->set_entry(m_mmc[offset] & 0x3f & (m_ram_banks - 1));
}

void nc_state::machine_start()
{
	m_rom_banks = (memregion("maincpu")->bytes() / 0x4000);
	m_ram_banks = (m_ram->size() / 0x4000);

	for (int i = 0; i < 4; i++)
	{
		m_rombank[i]->configure_entries(0, m_rom_banks, memregion("maincpu")->base(), 0x4000);
		m_rambank[i]->configure_entries(0, m_ram_banks, m_ram->pointer(), 0x4000);
	}

	m_nvram->set_base(m_ram->pointer(), m_ram->size());

	// setup lcd colors
	m_fg_color = rgb_t(0x38, 0x40, 0x63);
	m_bg_color = rgb_t(0xad, 0xa0, 0x71);

	// register for save states
	save_item(NAME(m_display_memory_start));
	save_item(NAME(m_mmc));
	save_item(NAME(m_uart_control));
	save_item(NAME(m_irq_mask));
	save_item(NAME(m_irq_status));
	save_item(NAME(m_uart_rxrdy));
	save_item(NAME(m_uart_txrdy));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_pcmcia_card_detect));
	save_item(NAME(m_pcmcia_write_protect));
	save_item(NAME(m_pcmcia_battery_voltage_1));
	save_item(NAME(m_pcmcia_battery_voltage_2));
}

void nc100_state::machine_start()
{
	nc_state::machine_start();

	// register for save states
	save_item(NAME(m_centronics_ack));
}

void nc200_state::machine_start()
{
	nc_state::machine_start();

	// allocate a timer to delay the fdc interrupt
	m_fdc_irq_timer = timer_alloc(FUNC(nc200_state::fdc_irq), this);
}

void nc_state::machine_reset()
{
	// display memory start is set to 0 on reset
	m_display_memory_start = 0;

	// memory management control is set to 0 on reset
	memory_management_w(0, 0x00);
	memory_management_w(1, 0x00);
	memory_management_w(2, 0x00);
	memory_management_w(3, 0x00);

	// all interrupts masked and none active
	m_irq_mask = 0;
	m_irq_status = 0;
	update_interrupts();

	// set to 0x01 on reset
	poweroff_control_w(0x01);

	// set to 0xff on reset
	uart_control_w(0xff);

	/* setup reset state */
	m_sound_channel_periods[0] = (m_sound_channel_periods[1] = 0x0ffff);
}

void nc200_state::machine_reset()
{
	nc_state::machine_reset();

	m_fdc_irq_timer->adjust(attotime::never);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

static void pcmcia_devices(device_slot_interface &device)
{
	device.option_add("melcard_1m", PCCARD_SRAM_MITSUBISHI_1M);
	device.option_add("sram_1m", PCCARD_SRAM_CENTENNIAL_1M);
}

void nc_state::nc_base(machine_config &config)
{
	Z80(config, m_maincpu, /*6000000*/ 4606000);        /* Russell Marks says this is more accurate */
	m_maincpu->set_addrmap(AS_PROGRAM, &nc_state::mem_map);
	config.set_maximum_quantum(attotime::from_hz(60));

	// ram
	RAM(config, m_ram).set_default_size("64K");
	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	TIMER(config, "keyscan_timer").configure_periodic(FUNC(nc_state::keyscan_timer), attotime::from_msec(10));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	m_screen->set_screen_update(FUNC(nc_state::screen_update));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper1, 0).add_route(ALL_OUTPUTS, "mono", 0.50);
	BEEP(config, m_beeper2, 0).add_route(ALL_OUTPUTS, "mono", 0.50);

	// centronics
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(nc_state::centronics_busy_w));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	// uart
	I8251(config, m_uart, 0);
	m_uart->txd_handler().set("serial", FUNC(rs232_port_device::write_txd));
	m_uart->rts_handler().set("serial", FUNC(rs232_port_device::write_rts));
	m_uart->dtr_handler().set("serial", FUNC(rs232_port_device::write_dtr));

	clock_device &uart_clock(CLOCK(config, "uart_clock", 150 * 16));
	uart_clock.signal_handler().set(m_uart, FUNC(i8251_device::write_rxc));
	uart_clock.signal_handler().append(m_uart, FUNC(i8251_device::write_txc));

	rs232_port_device &rs232(RS232_PORT(config, "serial", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_uart, FUNC(i8251_device::write_rxd));
	rs232.cts_handler().set(m_uart, FUNC(i8251_device::write_cts));
	rs232.dsr_handler().set(m_uart, FUNC(i8251_device::write_dsr));

	PCCARD_SLOT(config, m_pcmcia, pcmcia_devices, nullptr);
	m_pcmcia->cd1().set(FUNC(nc_state::pcmcia_card_detect_w));
	m_pcmcia->wp().set(FUNC(nc_state::pcmcia_write_protect_w));
	m_pcmcia->bvd1().set(FUNC(nc_state::pcmcia_battery_voltage_1_w));
	m_pcmcia->bvd2().set(FUNC(nc_state::pcmcia_battery_voltage_2_w));
}

void nc100_state::nc100(machine_config &config)
{
	nc_base(config);

	m_maincpu->set_addrmap(AS_IO, &nc100_state::io_map);

	// video hardware
	m_screen->set_size(480, 64);
	m_screen->set_visarea_full();

	// centronics
	m_centronics->ack_handler().set(FUNC(nc100_state::centronics_ack_w));

	// uart
	m_uart->rxrdy_handler().set(FUNC(nc100_state::uart_rxrdy_w));
	m_uart->txrdy_handler().set(FUNC(nc100_state::uart_txrdy_w));

	// rtc
	TC8521(config, "rtc", XTAL(32'768));
}

void nc100_state::nc150(machine_config &config)
{
	nc100(config);

	m_ram->set_default_size("128K");
}

void nc200_state::nc200(machine_config &config)
{
	nc_base(config);

	m_maincpu->set_addrmap(AS_IO, &nc200_state::io_map);

	// ram
	m_ram->set_default_size("128K");

	// video hardware
	m_screen->set_size(480, 128);
	m_screen->set_visarea_full();

	// centronics
	m_centronics->ack_handler().set(FUNC(nc200_state::centronics_ack_w));

	// uart
	m_uart->rxrdy_handler().set(FUNC(nc200_state::uart_rxrdy_w));

	// floppy
	UPD765A(config, m_fdc, 4'000'000, true, true);
	m_fdc->intrq_wr_callback().set(FUNC(nc200_state::fdc_int_w));

	FLOPPY_CONNECTOR(config, "fdc:0", "fdd", FLOPPY_35_DD, true, floppy_image_device::default_pc_floppy_formats);

	// rtc
	MC146818(config, m_rtc, 4.194304_MHz_XTAL);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( nc100 )
	ROM_REGION(0x40000, "maincpu", 0)
	ROM_DEFAULT_BIOS("106")
	ROM_SYSTEM_BIOS(0, "100", "ROM v1.00")
	ROMX_LOAD("nc100.rom",  0x00000, 0x40000, CRC(a699eca3) SHA1(ce217d5a298b959ccc3d7bc5c93b1dba043f1339), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "106", "ROM v1.06")
	ROMX_LOAD("nc100a.rom", 0x00000, 0x40000, CRC(849884f9) SHA1(ff030dd334ca867d620ee4a94b142ef0d93b69b6), ROM_BIOS(1))
ROM_END

ROM_START( nc100de )
	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD("nc100_de_a1.rom", 0x00000, 0x40000, CRC(bd9ce223) SHA1(2efb26911832bf1456d76d2508e24c0733dc216d))
ROM_END

ROM_START( nc100dk )
	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD("nc100_dk_a1.rom", 0x00000, 0x40000, CRC(ebb54923) SHA1(30321011384c5e10204b9a837430c36fc63580d2))
ROM_END

ROM_START( nc100sv )
	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD("nc100_sv_a1.rom", 0x00000, 0x40000, CRC(86e24cca) SHA1(1ca716c46400e2acfc4665c8e12acc3762f1e401))
ROM_END

ROM_START( dw225 )
	ROM_REGION(0x80000, "maincpu", 0)
	// identical to nc100a.rom with the last half filled with 0x00
	ROM_LOAD("dr,1.06.ic303", 0x00000, 0x80000, CRC(fcf2f7bd) SHA1(a69951618b24e97154cb4284d215cbf4aa9fb34f))
ROM_END

ROM_START( nc150fr )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("nc150_fr_b2.rom", 0x00000, 0x80000, CRC(be442d14) SHA1(f141d409dc72dc1e6662c21a147231c4df3be6b8))
ROM_END

ROM_START( nc150it )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("nc150_it_b1.rom", 0x00000, 0x80000, CRC(1b2fe2fd) SHA1(67eb6bce0b0d4668401d9c8f5a900dc6bd135c21))
ROM_END

ROM_START( nc200 )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("nc200.rom", 0x00000, 0x80000, CRC(bb8180e7) SHA1(fb5c93b0a3e199202c6a12548d2617f7a09bae47))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS        INIT        COMPANY                 FULLNAME           FLAGS
COMP( 1992, nc100,   0,      0,      nc100,   nc100,   nc100_state, empty_init, "Amstrad plc",          "NC100",           MACHINE_SUPPORTS_SAVE )
COMP( 1992, nc100de, nc100,  0,      nc100,   nc100de, nc100_state, empty_init, "Amstrad plc",          "NC100 (Germany)", MACHINE_SUPPORTS_SAVE )
COMP( 1992, nc100dk, nc100,  0,      nc100,   nc100dk, nc100_state, empty_init, "Amstrad plc",          "NC100 (Denmark)", MACHINE_SUPPORTS_SAVE )
COMP( 1992, nc100sv, nc100,  0,      nc100,   nc100sv, nc100_state, empty_init, "Amstrad plc",          "NC100 (Sweden)",  MACHINE_SUPPORTS_SAVE )
COMP( 1992, dw225,   nc100,  0,      nc100,   nc100,   nc100_state, empty_init, "NTS Computer Systems", "DreamWriter 225", MACHINE_SUPPORTS_SAVE )
COMP( 1992, nc150fr, nc100,  0,      nc150,   nc150fr, nc100_state, empty_init, "Amstrad plc",          "NC150 (France)",  MACHINE_SUPPORTS_SAVE )
COMP( 1992, nc150it, nc100,  0,      nc150,   nc150it, nc100_state, empty_init, "Amstrad plc",          "NC150 (Italy)",   MACHINE_SUPPORTS_SAVE )
COMP( 1993, nc200,   0,      0,      nc200,   nc200,   nc200_state, empty_init, "Amstrad plc",          "NC200",           MACHINE_SUPPORTS_SAVE )
