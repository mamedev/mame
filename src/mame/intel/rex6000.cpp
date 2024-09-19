// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Xircom / Intel REX 6000

        Driver by Sandro Ronco

        Known REX 6000 banks:
        0x0000 - 0x00ff     first flash chip
        0x0600 - 0x06ff     second flash chip
        0x1000 - 0x1003     32KB RAM

        Known DataSlim 2 banks:
        0x0000 - 0x00ff     first flash chip
        0x0b00 - 0x0bff     second flash chip
        0x1000 - 0x1003     32KB RAM

        TODO:
        - deleting all Memos causes an error
        - alarm doesn't work
        - alarm sound and keyclick
        - NVRAM and warm start (without the pen calibration)
        - serial I/O

****************************************************************************/


#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/ins8250.h"
#include "machine/rp5c01.h"
#include "machine/bankdev.h"
#include "machine/ram.h"
#include "machine/intelfsh.h"
#include "machine/timer.h"
#include "imagedev/snapquik.h"
#include "sound/beep.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

#define MAKE_BANK(lo, hi)       ((lo) | ((hi)<<8))

//irq flag/mask bits
#define IRQ_FLAG_KEYCHANGE  0x01
#define IRQ_FLAG_ALARM      0x02
#define IRQ_FLAG_SERIAL     0x04
#define IRQ_FLAG_1HZ        0x10
#define IRQ_FLAG_IRQ2       0x20
#define IRQ_FLAG_IRQ1       0x40
#define IRQ_FLAG_EVENT      0x80

#define TC8521_TAG  "rtc"

class rex6000_state : public driver_device
{
public:
	rex6000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_beep(*this, "beeper")
		, m_uart(*this, "ns16550")
		, m_bankdev0(*this, "bank0")
		, m_bankdev1(*this, "bank1")
		, m_flash0a(*this, "flash0a")
		, m_flash0b(*this, "flash0b")
		, m_flash1a(*this, "flash1a")
		, m_flash1b(*this, "flash1b")
		, m_nvram(*this, "nvram")
		, m_battery(*this, "BATTERY")
		, m_pen_x(*this, "PENX")
		, m_pen_y(*this, "PENY")
	{ }

	void rex6000(machine_config &config);

	void rex6000_palettte(palette_device &palette) const;
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_rex6000);
	DECLARE_INPUT_CHANGED_MEMBER(trigger_irq);
	void serial_irq(int state);
	void alarm_irq(int state);

	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer1);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer2);
	TIMER_DEVICE_CALLBACK_MEMBER(sec_timer);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t bankswitch_r(offs_t offset);
	void bankswitch_w(offs_t offset, uint8_t data);
	uint8_t lcd_base_r(offs_t offset);
	void lcd_base_w(offs_t offset, uint8_t data);
	uint8_t beep_r(offs_t offset);
	void beep_w(offs_t offset, uint8_t data);
	uint8_t lcd_io_r(offs_t offset);
	void lcd_io_w(offs_t offset, uint8_t data);
	uint8_t irq_r(offs_t offset);
	void irq_w(offs_t offset, uint8_t data);
	uint8_t touchscreen_r(offs_t offset);
	void touchscreen_w(offs_t offset, uint8_t data);

	void rex6000_banked_map(address_map &map) ATTR_COLD;
	void rex6000_io(address_map &map) ATTR_COLD;
	void rex6000_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<beep_device> m_beep;
	required_device<ns16550_device> m_uart;
	required_device<address_map_bank_device> m_bankdev0;
	required_device<address_map_bank_device> m_bankdev1;
	optional_device<intelfsh8_device> m_flash0a;
	optional_device<intelfsh8_device> m_flash0b;
	optional_device<intelfsh8_device> m_flash1a;
	optional_device<intelfsh8_device> m_flash1b;
	required_shared_ptr<uint8_t> m_nvram;
	required_ioport m_battery;
	optional_ioport m_pen_x;
	optional_ioport m_pen_y;

	uint8_t m_bank[4];
	uint8_t m_beep_io[5];
	uint8_t m_lcd_base[2];
	uint8_t m_touchscreen[0x10];
	uint8_t m_lcd_enabled;
	uint8_t m_lcd_cmd;

	uint8_t m_irq_mask;
	uint8_t m_irq_flag;
	uint8_t m_port6;
	uint8_t m_beep_mode;
	uint8_t m_power_on;

};


class oz750_state : public rex6000_state
{
public:
	oz750_state(const machine_config &mconfig, device_type type, const char *tag)
		: rex6000_state(mconfig, type, tag)
		, m_keyboard(*this, "COL.%u", 0)
	{
	}

	optional_ioport_array<10> m_keyboard;

	uint8_t kb_status_r();
	uint8_t kb_data_r();
	void kb_mask_w(offs_t offset, uint8_t data);
	DECLARE_INPUT_CHANGED_MEMBER(trigger_on_irq);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_oz750);

	virtual void machine_reset() override ATTR_COLD;
	uint32_t screen_update_oz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void oz750(machine_config &config);
	void oz750_banked_map(address_map &map) ATTR_COLD;
	void oz750_io(address_map &map) ATTR_COLD;
private:
	int oz_wzd_extract_tag(const std::vector<uint8_t> &data, const char *tag, char *dest_buf);

	uint16_t m_kb_mask = 0;
};


uint8_t rex6000_state::bankswitch_r(offs_t offset)
{
	return m_bank[offset];
}

void rex6000_state::bankswitch_w(offs_t offset, uint8_t data)
{
	m_bank[offset&3] = data;

	switch (offset)
	{
		case 0:     //bank 1 low
		case 1:     //bank 1 high
		{
			//bank 1 start at 0x8000
			m_bankdev0->set_bank(MAKE_BANK(m_bank[0], m_bank[1]) + 4);
			break;
		}
		case 2:     //bank 2 low
		case 3:     //bank 2 high
		{
			m_bankdev1->set_bank(MAKE_BANK(m_bank[2], m_bank[3]));
			break;
		}
	}
}

uint8_t rex6000_state::beep_r(offs_t offset)
{
	return m_beep_io[offset];
}

void rex6000_state::beep_w(offs_t offset, uint8_t data)
{
	m_beep_io[offset] = data;

	switch (offset)
	{
		case 0: // alarm mode control
			/*
			    ---- ---x   beep off
			    ---- --x-   continuous beep
			    ---- -x--   continuous alarm 1
			    ---- x---   continuous alarm 2
			    ---x ----   continuous alarm 3
			    --x- ----   single very short beep (keyclick)
			    -x-- ----   single alarm
			    x--- ----   single short beep
			*/
			// TODO: the beeper frequency and length in alarm mode need to be measured
			break;
		case 1: // tone mode control
			if (m_beep_mode)
			{
				m_beep->set_state(BIT(data, 0));

				// the beeper frequency is update only if the bit 1 is set
				if (BIT(data, 1))
				{
					uint16_t div = ((m_beep_io[2] | m_beep_io[3]<<8) & 0x0fff) + 2;
					m_beep->set_clock(16384 / div);
				}
			}
			break;
		case 4: // select alarm/tone mode
			if (m_beep_mode != BIT(data, 0))
				m_beep->set_state(0); // turned off when mode changes

			m_beep_mode = BIT(data, 0);
			break;
	}
}

uint8_t rex6000_state::lcd_base_r(offs_t offset)
{
	return m_lcd_base[offset];
}

void rex6000_state::lcd_base_w(offs_t offset, uint8_t data)
{
	m_lcd_base[offset&1] = data;
}

uint8_t rex6000_state::lcd_io_r(offs_t offset)
{
	return (offset == 0) ? m_lcd_enabled : m_lcd_cmd;
}

void rex6000_state::lcd_io_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_lcd_enabled = data;
			break;
		case 1:
			m_lcd_cmd = data;
			break;
	}
}

uint8_t rex6000_state::irq_r(offs_t offset)
{
	switch (offset)
	{
		case 0: //irq flag
			/*
			    ---- ---x   input port
			    ---- --x-   alarm
			    ---- -x--   serial
			    ---- x---   ??
			    ---x ----   1Hz timer
			    --x- ----   32Hz timer
			    -x-- ----   4096Hz timer
			    x--- ----   special events
			*/
			return m_irq_flag;

		case 1:
			return m_port6;

		case 2: //irq mask
			return m_irq_mask;

		default:
			return 0;
	}
}

void rex6000_state::irq_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0: //irq flag
			m_irq_flag = data;
			break;
		case 1: //clear irq flag
			m_port6 = data;
			m_irq_flag &= (~data);
			break;
		case 2: //irq mask
			m_irq_mask = data;
			break;
		case 3: //power control
			m_power_on = data & 1;
			break;
	}
}

uint8_t rex6000_state::touchscreen_r(offs_t offset)
{
	uint16_t x = m_pen_x->read();
	uint16_t y = m_pen_y->read();
	uint16_t battery = m_battery->read();

	switch (offset)
	{
		case 0x08:
			return ((ioport("INPUT")->read() & 0x40) ? 0x20 : 0x00) | 0x10;
		case 0x09:
			if (m_touchscreen[4] & 0x80)
				return (battery>>0) & 0xff;
			else
				return (y>>0) & 0xff;
		case 0x0a:
			if (m_touchscreen[4] & 0x80)
				return (battery>>8) & 0xff;
			else
				return (y>>8) & 0xff;
		case 0x0b:
			return (x>>0) & 0xff;
		case 0x0c:
			return (x>>8) & 0xff;
	}

	return m_touchscreen[offset&0x0f];
}

void rex6000_state::touchscreen_w(offs_t offset, uint8_t data)
{
	m_touchscreen[offset&0x0f] = data;
}

uint8_t oz750_state::kb_status_r()
{
	uint8_t data = 0x6b;
	if (m_battery->read() & 0x01)   data |= 0x80;

	return data;
}

uint8_t oz750_state::kb_data_r()
{
	uint8_t data = 0;
	for(int i=0; i<10; i++)
	{
		if (m_kb_mask & (1<<i))
			data |= m_keyboard[i]->read();
	}

	return data;
}
void oz750_state::kb_mask_w(offs_t offset, uint8_t data)
{
	if (offset)
		m_kb_mask = (m_kb_mask & 0x00ff) | (data << 8);
	else
		m_kb_mask = (m_kb_mask & 0xff00) | data;
}

void rex6000_state::rex6000_banked_map(address_map &map)
{
	map(0x0000000, 0x00fffff).rw(m_flash0a, FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
	map(0x0100000, 0x01fffff).rw(m_flash0b, FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
	map(0x0c00000, 0x0cfffff).rw(m_flash1a, FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
	map(0x0d00000, 0x0dfffff).rw(m_flash1b, FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
	map(0x1600000, 0x16fffff).rw(m_flash1a, FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
	map(0x1700000, 0x17fffff).rw(m_flash1b, FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
	map(0x2000000, 0x2007fff).ram().share("nvram");
}

void oz750_state::oz750_banked_map(address_map &map)
{
	map(0x0000000, 0x01fffff).rw(m_flash0a, FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
	map(0x0200000, 0x02fffff).mirror(0x100000).rw(m_flash1a, FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
	map(0x0600000, 0x07fffff).rw(FUNC(oz750_state::lcd_io_r), FUNC(oz750_state::lcd_io_w));
	map(0x0800000, 0x083ffff).mirror(0x1c0000).ram().share("nvram");
	map(0x0a00000, 0x0a3ffff).mirror(0x1c0000).ram();
}


void rex6000_state::rex6000_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rw(m_flash0a, FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
	map(0x8000, 0x9fff).rw(m_bankdev0, FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
	map(0xa000, 0xbfff).rw(m_bankdev1, FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
	map(0xc000, 0xffff).bankrw("ram");            //system RAM
}

void rex6000_state::rex6000_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x01, 0x04).rw(FUNC(rex6000_state::bankswitch_r), FUNC(rex6000_state::bankswitch_w));
	map(0x05, 0x07).rw(FUNC(rex6000_state::irq_r), FUNC(rex6000_state::irq_w));
	map(0x10, 0x10).portr("INPUT");
	map(0x15, 0x19).rw(FUNC(rex6000_state::beep_r), FUNC(rex6000_state::beep_w));
	map(0x22, 0x23).rw(FUNC(rex6000_state::lcd_base_r), FUNC(rex6000_state::lcd_base_w));
	map(0x30, 0x3f).rw(TC8521_TAG, FUNC(tc8521_device::read), FUNC(tc8521_device::write));
	map(0x40, 0x47).mirror(0x08).rw(m_uart, FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
	map(0x50, 0x51).rw(FUNC(rex6000_state::lcd_io_r), FUNC(rex6000_state::lcd_io_w));
	map(0x60, 0x6f).rw(FUNC(rex6000_state::touchscreen_r), FUNC(rex6000_state::touchscreen_w));
}

void oz750_state::oz750_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x01, 0x04).rw(FUNC(oz750_state::bankswitch_r), FUNC(oz750_state::bankswitch_w));
	map(0x05, 0x08).rw(FUNC(oz750_state::irq_r), FUNC(oz750_state::irq_w));
	map(0x10, 0x10).r(FUNC(oz750_state::kb_data_r));
	map(0x11, 0x12).rw(FUNC(oz750_state::kb_status_r), FUNC(oz750_state::kb_mask_w));
	map(0x15, 0x19).rw(FUNC(oz750_state::beep_r), FUNC(oz750_state::beep_w));
	map(0x22, 0x23).rw(FUNC(oz750_state::lcd_base_r), FUNC(oz750_state::lcd_base_w));
	map(0x30, 0x3f).rw(TC8521_TAG, FUNC(tc8521_device::read), FUNC(tc8521_device::write));
	map(0x40, 0x47).mirror(0x08).rw(m_uart, FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
}

INPUT_CHANGED_MEMBER(rex6000_state::trigger_irq)
{
	if (!(m_irq_mask & IRQ_FLAG_KEYCHANGE))
	{
		m_irq_flag |= IRQ_FLAG_KEYCHANGE;

		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}

INPUT_CHANGED_MEMBER(oz750_state::trigger_on_irq)
{
	m_uart->cts_w(!newval);

	if (!(m_irq_mask & IRQ_FLAG_EVENT))
	{
		m_irq_flag |= IRQ_FLAG_EVENT;

		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}

/* Input ports */
INPUT_PORTS_START( rex6000 )
	PORT_START("BATTERY")
	PORT_CONFNAME( 0x03ff, 0x03ff, "Battery Status" )
	PORT_CONFSETTING( 0x03ff, "Good" )
	PORT_CONFSETTING( 0x0000, "Poor" )

	PORT_START("INPUT")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Home")   PORT_CODE(KEYCODE_HOME)         PORT_CHANGED_MEMBER(DEVICE_SELF, rex6000_state, trigger_irq, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Back")   PORT_CODE(KEYCODE_BACKSPACE)    PORT_CHANGED_MEMBER(DEVICE_SELF, rex6000_state, trigger_irq, 0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Select")     PORT_CODE(KEYCODE_SPACE)    PORT_CHANGED_MEMBER(DEVICE_SELF, rex6000_state, trigger_irq, 0)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Up")     PORT_CODE(KEYCODE_PGUP)         PORT_CHANGED_MEMBER(DEVICE_SELF, rex6000_state, trigger_irq, 0)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Down")   PORT_CODE(KEYCODE_PGDN)         PORT_CHANGED_MEMBER(DEVICE_SELF, rex6000_state, trigger_irq, 0)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Pen")    PORT_CODE(KEYCODE_ENTER) PORT_CODE(MOUSECODE_BUTTON1)   PORT_CHANGED_MEMBER(DEVICE_SELF, rex6000_state, trigger_irq, 0)

	PORT_START("PENX")
	PORT_BIT(0x3ff, 0x00, IPT_LIGHTGUN_X) PORT_NAME("Pen X") PORT_CROSSHAIR(X, 1, 0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_INVERT

	PORT_START("PENY")
	PORT_BIT(0x3ff, 0x00, IPT_LIGHTGUN_Y) PORT_NAME("Pen Y") PORT_CROSSHAIR(Y, 1, 0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_INVERT
INPUT_PORTS_END

INPUT_PORTS_START( oz750 )
	PORT_START("BATTERY")
	PORT_CONFNAME( 0x01, 0x01, "Battery Status" )
	PORT_CONFSETTING( 0x01, "Normal operating mode" )
	PORT_CONFSETTING( 0x00, "Replace battery mode" )

	PORT_START("ON")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_0_PAD)  PORT_NAME("ON")                         PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_on_irq, 0)

	PORT_START("COL.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)         PORT_CHAR(UCHAR_MAMEKEY(ESC))       PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)           PORT_CHAR('Q')                      PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)           PORT_CHAR('G')                      PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)           PORT_CHAR('A')                      PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)      PORT_CHAR(UCHAR_MAMEKEY(LSHIFT))    PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)    PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))  PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)      PORT_CHAR('1')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)      PORT_CHAR('V')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)      PORT_CHAR('Y')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)      PORT_CHAR('H')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)      PORT_CHAR('S')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)      PORT_CHAR('Z')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)     PORT_NAME("MENU")                        PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)      PORT_CHAR('2')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)      PORT_CHAR('E')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)      PORT_CHAR('U')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)      PORT_CHAR('J')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)      PORT_CHAR('D')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)      PORT_CHAR('X')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)     PORT_NAME("NEW")                         PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)      PORT_CHAR('3')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)      PORT_CHAR('R')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)      PORT_CHAR('I')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)      PORT_CHAR('K')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)      PORT_CHAR('F')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)      PORT_CHAR('C')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)     PORT_CHAR(UCHAR_MAMEKEY(F3))             PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)      PORT_CHAR('4')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)      PORT_CHAR('T')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)      PORT_CHAR('O')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)      PORT_CHAR('L')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)   PORT_CHAR(UCHAR_MAMEKEY(LEFT))           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)      PORT_CHAR('V')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)      PORT_CHAR('5')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)      PORT_CHAR('8')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)      PORT_CHAR('P')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)   PORT_NAME("RETURN")                 PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)   PORT_CHAR(UCHAR_MAMEKEY(DOWN))           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)      PORT_CHAR('B')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)  PORT_CHAR('-')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)      PORT_CHAR('5')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)      PORT_CHAR('8')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)    PORT_CHAR(UCHAR_MAMEKEY(DEL))            PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))         PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)  PORT_CHAR(UCHAR_MAMEKEY(RIGHT))          PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)      PORT_CHAR('N')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)  PORT_CHAR(13)                            PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)      PORT_CHAR('7')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)      PORT_CHAR('0')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)     PORT_CHAR(UCHAR_MAMEKEY(UP))             PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)      PORT_CHAR('M')                           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)     PORT_NAME("MENU Screen")                 PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)     PORT_NAME("ESC Screen")                  PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)     PORT_NAME("ENTER Screen")                PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)   PORT_CHAR(UCHAR_MAMEKEY(PGUP))           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)   PORT_CHAR(UCHAR_MAMEKEY(PGDN))           PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL.9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)     PORT_NAME("MAIN")                        PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)     PORT_NAME("TEL")                         PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)     PORT_NAME("SCHEDULE")                    PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)    PORT_NAME("MEMO")                        PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)    PORT_NAME("My Programs")                 PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12)    PORT_NAME("Backlight")                   PORT_CHANGED_MEMBER(DEVICE_SELF, oz750_state, trigger_irq, 0)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

void rex6000_state::machine_start()
{
	membank("ram")->set_base((uint8_t*)m_nvram + 0x4000);
}

void rex6000_state::machine_reset()
{
	memset(m_bank, 0, sizeof(m_bank));
	memset(m_beep_io, 0, sizeof(m_beep_io));
	memset(m_lcd_base, 0, sizeof(m_lcd_base));
	memset(m_touchscreen, 0, sizeof(m_touchscreen));
	m_lcd_enabled = 0;
	m_lcd_cmd = 0;
	m_irq_mask = 0;
	m_irq_flag = 0;
	m_port6 = 0;
	m_beep_mode = 0;
	m_power_on = 0;
}

void oz750_state::machine_reset()
{
	rex6000_state::machine_reset();
	m_kb_mask = 0;
}

uint32_t rex6000_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t const lcd_bank = MAKE_BANK(m_lcd_base[0], m_lcd_base[1]);

	if (m_lcd_enabled)
	{
		for (int y=0; y<120; y++)
			for (int x=0; x<30; x++)
			{
				uint8_t data = m_bankdev0->space(AS_PROGRAM).read_byte((lcd_bank << 13) + y*30 + x);

				for (int b=0; b<8; b++)
				{
					bitmap.pix(y, (x * 8) + b) = BIT(data, 7);
					data <<= 1;
				}
			}
	}
	else
	{
		bitmap.fill(0, cliprect);
	}

	return 0;
}

uint32_t oz750_state::screen_update_oz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t const lcd_bank = MAKE_BANK(m_lcd_base[0], m_lcd_base[1]);

	if (m_lcd_enabled && m_power_on)
	{
		for (int y=0; y<=cliprect.max_y; y++)
			for (int x=0; x<30; x++)
			{
				uint8_t data = m_bankdev0->space(AS_PROGRAM).read_byte((lcd_bank << 13) + y*30 + x);

				for (int b=0; b<8; b++)
				{
					bitmap.pix(y, (x * 8) + b) = BIT(data, 0);
					data >>= 1;
				}
			}
	}
	else
	{
		bitmap.fill(0, cliprect);
	}

	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(rex6000_state::irq_timer1)
{
	if (!(m_irq_mask & IRQ_FLAG_IRQ2))
	{
		m_irq_flag |= IRQ_FLAG_IRQ2;

		m_maincpu->set_input_line(0, HOLD_LINE);
	}

}

TIMER_DEVICE_CALLBACK_MEMBER(rex6000_state::irq_timer2)
{
	if (!(m_irq_mask & IRQ_FLAG_IRQ1))
	{
		m_irq_flag |= IRQ_FLAG_IRQ1;

		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(rex6000_state::sec_timer)
{
	if (!(m_irq_mask & IRQ_FLAG_1HZ))
	{
		m_irq_flag |= IRQ_FLAG_1HZ;

		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}

void rex6000_state::alarm_irq(int state)
{
	if (!(m_irq_mask & IRQ_FLAG_ALARM) && state)
	{
		m_irq_flag |= IRQ_FLAG_ALARM;
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}

void rex6000_state::serial_irq(int state)
{
	if (!(m_irq_mask & IRQ_FLAG_SERIAL))
	{
		m_irq_flag |= IRQ_FLAG_SERIAL;
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}

void rex6000_state::rex6000_palettte(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

QUICKLOAD_LOAD_MEMBER(rex6000_state::quickload_rex6000)
{
	// FIXME: this suffers buffer overruns on excessively short images and various kinds of invalid images
	static const char magic[] = "ApplicationName:Addin";
	uint32_t img_start = 0;

	std::vector<uint8_t> data(image.length());
	image.fread(&data[0], image.length());

	if(strncmp((const char*)&data[0], magic, 21))
		return std::make_pair(image_error::INVALIDIMAGE, std::string());

	img_start = strlen((const char*)&data[0]) + 5;
	img_start += 0xa0;  //skip the icon (40x32 pixel)

	for (uint32_t i=0; i<image.length() - img_start ;i++)
		m_flash0b->write_raw(i, data[img_start + i]);

	return std::make_pair(std::error_condition(), std::string());
}

int oz750_state::oz_wzd_extract_tag(const std::vector<uint8_t> &data, const char *tag, char *dest_buf)
{
	int tag_len = strlen(tag);
	uint32_t img_start = 0;
	for (img_start=0; img_start < data.size() - tag_len; img_start++)
		if (data[img_start] && !memcmp(&data[img_start], tag, tag_len))
			break;

	if (img_start >= data.size() - tag_len)
	{
		if (dest_buf)
			strcpy(dest_buf, "NONE");
		return 0;
	}

	img_start += tag_len;

	while(data[img_start] == '\r' || data[img_start] == '\n')
		img_start++;

	if (dest_buf)
	{
		uint32_t i;
		for (i=0; data[img_start + i] != 0 && data[img_start + i] != '\n' && data[img_start + i] != '\r'; i++)
			dest_buf[i] = data[img_start + i];

		dest_buf[i] = '\0';
	}

	return img_start;
}

QUICKLOAD_LOAD_MEMBER(oz750_state::quickload_oz750)
{
	address_space* flash = &m_flash0a->memory().space(0);
	std::vector<uint8_t> data(image.length());
	image.fread(&data[0], image.length());

	const char *fs_type = "BSIC";
	char data_type[0x100];
	char app_name[0x100];
	char file_name[0x100];

	oz_wzd_extract_tag(data, "<DATA TYPE>", data_type);
	if (strcmp(data_type, "MY PROGRAMS"))
		return std::make_pair(image_error::INVALIDIMAGE, std::string());

	oz_wzd_extract_tag(data, "<TITLE>", app_name);
	oz_wzd_extract_tag(data, "<DATA>", file_name);
	if (!strncmp(file_name, "PFILE:", 6))
		memmove(file_name, file_name + 6, strlen(file_name + 6) + 1);

	uint32_t img_start = oz_wzd_extract_tag(data, "<BIN>", nullptr);

	if (img_start == 0)
		return std::make_pair(image_error::INVALIDIMAGE, std::string());

	uint16_t icon_size = data[img_start++];

	uint32_t pos = 0xc0000;
	flash->write_byte(pos++, 0x4f);

	for (int i=0; fs_type[i]; i++)
		flash->write_byte(pos++, fs_type[i]);                   // file type

	flash->write_byte(pos++, 0xff);
	flash->write_byte(pos++, 0xff);
	flash->write_byte(pos++, 0xff);
	flash->write_byte(pos++, 0x10 + icon_size);                                         // filename offset LSB
	flash->write_byte(pos++, 0x00);                                                     // filename offset MSB
	flash->write_byte(pos++, 0x11 + icon_size + strlen(file_name));                     // title offset LSB
	flash->write_byte(pos++, 0x00);                                                     // title offset MSB
	flash->write_byte(pos++, 0x12 + icon_size + strlen(file_name) + strlen(app_name));  // data offset LSB
	flash->write_byte(pos++, 0x00);                                                     // data offset MSB
	flash->write_byte(pos++, 1);
	flash->write_byte(pos++, 1);

	flash->write_byte(pos++, icon_size);                        // icon size

	for (int i=0; i<icon_size; i++)
		flash->write_byte(pos++, data[img_start++]);            // icon data

	for (int i=0, slen = strlen(file_name); i <= slen; i++)
		flash->write_byte(pos++, file_name[i]);                 // filename

	for (int i=0, slen = strlen(app_name); i <= slen; i++)
		flash->write_byte(pos++, app_name[i]);                  // title

	uint16_t size = (uint16_t)image.length() - img_start;
	flash->write_byte(pos++, size);                             // data size LSB
	flash->write_byte(pos++, size >> 8);                        // data size MSB

	for (int i=img_start; i<image.length(); i++)
		flash->write_byte(pos++, data[i]);                      // data

	return std::make_pair(std::error_condition(), std::string());
}


/* F4 Character Displayer */
static const gfx_layout rex6000_bold_charlayout =
{
	16, 11,                 /* 16 x 11 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
	/* y offsets */
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16, 9*16, 10*16 },
	8*24            /* every char takes 24 bytes, first 2 bytes are used for the char size */
};

static const gfx_layout rex6000_tiny_charlayout =
{
	16, 9,                  /* 16 x 9 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
	/* y offsets */
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16 },
	8*20            /* every char takes 20 bytes, first 2 bytes ared use for the char size */
};

static const gfx_layout rex6000_graph_charlayout =
{
	16, 13,                 /* 16 x 13 characters */
	48,                     /* 48 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
	/* y offsets */
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16, 9*16, 10*16 , 11*16 , 12*16 },
	8*28            /* every char takes 28 bytes, first 2 bytes are used for the char size */
};

static GFXDECODE_START( gfx_rex6000 )
	GFXDECODE_ENTRY( "flash0a", 0x0f0000, rex6000_bold_charlayout,  0, 0 )  //normal
	GFXDECODE_ENTRY( "flash0a", 0x0f2000, rex6000_bold_charlayout,  0, 0 )  //bold
	GFXDECODE_ENTRY( "flash0a", 0x0f4000, rex6000_tiny_charlayout,  0, 0 )  //tiny
	GFXDECODE_ENTRY( "flash0a", 0x0f6000, rex6000_graph_charlayout, 0, 0 )  //graphic
GFXDECODE_END


void rex6000_state::rex6000(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000)); //Toshiba microprocessor Z80 compatible at 4.3MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &rex6000_state::rex6000_mem);
	m_maincpu->set_addrmap(AS_IO, &rex6000_state::rex6000_io);

	TIMER(config, "sec_timer").configure_periodic(FUNC(rex6000_state::sec_timer), attotime::from_hz(1));
	TIMER(config, "irq_timer1").configure_periodic(FUNC(rex6000_state::irq_timer1), attotime::from_hz(32));
	TIMER(config, "irq_timer2").configure_periodic(FUNC(rex6000_state::irq_timer2), attotime::from_hz(4096));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(rex6000_state::screen_update));
	screen.set_size(240, 120);
	screen.set_visarea(0, 240-1, 0, 120-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(rex6000_state::rex6000_palettte), 2);
	GFXDECODE(config, "gfxdecode", "palette", gfx_rex6000);

	ADDRESS_MAP_BANK(config, "bank0").set_map(&rex6000_state::rex6000_banked_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x2000);
	ADDRESS_MAP_BANK(config, "bank1").set_map(&rex6000_state::rex6000_banked_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x2000);

	NS16550(config, m_uart, XTAL(1'843'200));
	m_uart->out_tx_callback().set("serport", FUNC(rs232_port_device::write_txd));
	m_uart->out_dtr_callback().set("serport", FUNC(rs232_port_device::write_dtr));
	m_uart->out_rts_callback().set("serport", FUNC(rs232_port_device::write_rts));
	m_uart->out_int_callback().set(FUNC(rex6000_state::serial_irq));

	rs232_port_device &serport(RS232_PORT(config, "serport", default_rs232_devices, nullptr));
	serport.rxd_handler().set(m_uart, FUNC(ins8250_uart_device::rx_w));
	serport.dcd_handler().set(m_uart, FUNC(ins8250_uart_device::dcd_w));
	serport.dsr_handler().set(m_uart, FUNC(ins8250_uart_device::dsr_w));
	serport.ri_handler().set(m_uart, FUNC(ins8250_uart_device::ri_w));
	serport.cts_handler().set(m_uart, FUNC(ins8250_uart_device::cts_w));

	/* quickload */
	QUICKLOAD(config, "quickload", "rex,ds2").set_load_callback(FUNC(rex6000_state::quickload_rex6000));

	tc8521_device &rtc(TC8521(config, TC8521_TAG, XTAL(32'768)));
	rtc.out_alarm_callback().set(FUNC(rex6000_state::alarm_irq));

	/*
	Fujitsu 29DL16X has a feature which is capable of reading data from one
	bank of memory while a program or erase operation is in progress in the
	other bank of memory (simultaneous operation). This is not supported yet
	by the flash emulation and I have split every bank into a separate
	device in order to have similar behavior.
	*/
	FUJITSU_29DL164BD(config, m_flash0a); //bank 0 of first flash
	FUJITSU_29DL164BD(config, m_flash0b); //bank 1 of first flash
	FUJITSU_29DL164BD(config, m_flash1a); //bank 0 of second flash
	FUJITSU_29DL164BD(config, m_flash1b); //bank 1 of second flash

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("32K");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 0).add_route(ALL_OUTPUTS, "mono", 1.00);
}

void oz750_state::oz750(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(9'830'400)); //Toshiba microprocessor Z80 compatible at 9.8MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &oz750_state::rex6000_mem);
	m_maincpu->set_addrmap(AS_IO, &oz750_state::oz750_io);

	TIMER(config, "sec_timer").configure_periodic(FUNC(rex6000_state::sec_timer), attotime::from_hz(1));
	TIMER(config, "irq_timer1").configure_periodic(FUNC(rex6000_state::irq_timer1), attotime::from_hz(64));
	TIMER(config, "irq_timer2").configure_periodic(FUNC(rex6000_state::irq_timer2), attotime::from_hz(8192));

	NS16550(config, m_uart, XTAL(9'830'400) / 4);
	m_uart->out_tx_callback().set("serport", FUNC(rs232_port_device::write_txd));
	m_uart->out_dtr_callback().set("serport", FUNC(rs232_port_device::write_dtr));
	m_uart->out_rts_callback().set("serport", FUNC(rs232_port_device::write_rts));
	m_uart->out_int_callback().set(FUNC(rex6000_state::serial_irq));

	rs232_port_device &serport(RS232_PORT(config, "serport", default_rs232_devices, nullptr));
	serport.rxd_handler().set(m_uart, FUNC(ins8250_uart_device::rx_w));
	serport.dcd_handler().set(m_uart, FUNC(ins8250_uart_device::dcd_w));
	serport.dsr_handler().set(m_uart, FUNC(ins8250_uart_device::dsr_w));
	serport.ri_handler().set(m_uart, FUNC(ins8250_uart_device::ri_w));
	//serport.cts_handler().set(m_uart, FUNC(ins8250_uart_device::cts_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(oz750_state::screen_update_oz));
	screen.set_size(240, 80);
	screen.set_visarea(0, 240-1, 0, 80-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(rex6000_state::rex6000_palettte), 2);

	ADDRESS_MAP_BANK(config, "bank0").set_map(&oz750_state::oz750_banked_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x2000);
	ADDRESS_MAP_BANK(config, "bank1").set_map(&oz750_state::oz750_banked_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x2000);

	/* quickload */
	QUICKLOAD(config, "quickload", "wzd").set_load_callback(FUNC(oz750_state::quickload_oz750));

	tc8521_device &rtc(TC8521(config, TC8521_TAG, XTAL(32'768)));
	rtc.out_alarm_callback().set(FUNC(rex6000_state::alarm_irq));

	SHARP_LH28F016S(config, m_flash0a);
	SHARP_LH28F016S(config, m_flash1a);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("512K");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 0).add_route(ALL_OUTPUTS, "mono", 1.00);
}

/* ROM definition */
ROM_START( rex6000 )
	ROM_REGION( 0x200000, "flash0a", ROMREGION_ERASE )
	ROM_LOAD( "rex6000_0.dat", 0x0000, 0x100000, BAD_DUMP CRC(e7a7324f) SHA1(01adcd469c93984438cbd4bf6e5a46e74da500d0))

	ROM_REGION( 0x200000, "flash0b", ROMREGION_ERASE )
	ROM_LOAD( "rex6000_1.dat", 0x0000, 0x100000, BAD_DUMP CRC(cc4b51db) SHA1(73b655e21ece30e8dc60f2f09719bc446492cc0f))

	ROM_REGION( 0x200000, "flash1a", ROMREGION_ERASE )
	ROM_LOAD( "rex6000_2.dat", 0x0000, 0x100000, NO_DUMP)

	ROM_REGION( 0x200000, "flash1b", ROMREGION_ERASE )
	ROM_LOAD( "rex6000_3.dat", 0x0000, 0x100000, NO_DUMP)
ROM_END

ROM_START( ds2 )
	ROM_REGION( 0x200000, "flash0a", ROMREGION_ERASE )
	ROM_LOAD( "ds2_0.dat", 0x0000, 0x100000, BAD_DUMP CRC(3197bed3) SHA1(9f826fd6d23ced797daae666d88ced14d24b64c3))

	ROM_REGION( 0x200000, "flash0b", ROMREGION_ERASE )
	ROM_LOAD( "ds2_1.dat", 0x0000, 0x100000, BAD_DUMP CRC(a909b755) SHA1(4db8a44539b9f94d418c33bbd794afb9bacbbadd))

	ROM_REGION( 0x200000, "flash1a", ROMREGION_ERASE )
	ROM_LOAD( "ds2_2.dat", 0x0000, 0x100000, BAD_DUMP CRC(a738ea1c) SHA1(3b71f43ff30f4b15b5cd85dd9e95ebc7e84eb5a3))

	ROM_REGION( 0x200000, "flash1b", ROMREGION_ERASE )
	ROM_LOAD( "ds2_3.dat", 0x0000, 0x100000, BAD_DUMP CRC(64f7c189) SHA1(a47d3413ddb879083c3aec03a42fe357d3a3c10a))
ROM_END

ROM_START( oz750 )
	ROM_REGION( 0x200000, "flash0a", ROMREGION_ERASEFF )
	ROM_LOAD( "oz750_0.bin", 0x0000, 0x200000, CRC(bf321bab) SHA1(2fb44a870b26aebf4b949b5de84fb39cd9205191))

	ROM_REGION( 0x200000, "flash1a", ROMREGION_ERASEFF )
	ROM_LOAD( "oz750_1.bin", 0x0000, 0x100000, CRC(d74e97d7) SHA1(19d17393a9af85e07773feaf1aed5e2cfa80f7cc))
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY           FULLNAME         FLAGS */
COMP( 199?, oz750,   0,       0,      oz750,   oz750,   oz750_state,   empty_init, "Sharp",          "Wizard OZ-750", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 2000, rex6000, 0,       0,      rex6000, rex6000, rex6000_state, empty_init, "Xircom / Intel", "REX 6000",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 2000, ds2,     rex6000, 0,      rex6000, rex6000, rex6000_state, empty_init, "Citizen",        "DataSlim 2",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
