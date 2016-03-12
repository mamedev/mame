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
#include "imagedev/snapquik.h"
#include "sound/beep.h"
#include "rendlay.h"

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
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_ram(*this, RAM_TAG),
			m_beep(*this, "beeper"),
			m_uart(*this, "ns16550"),
			m_bankdev0(*this, "bank0"),
			m_bankdev1(*this, "bank1"),
			m_flash0b(*this, "flash0b"),
			m_nvram(*this, "nvram"),
			m_battery(*this, "BATTERY"),
			m_pen_x(*this, "PENX"),
			m_pen_y(*this, "PENY")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<beep_device> m_beep;
	required_device<ns16550_device> m_uart;
	required_device<address_map_bank_device> m_bankdev0;
	required_device<address_map_bank_device> m_bankdev1;
	optional_device<intelfsh8_device> m_flash0b;
	required_shared_ptr<UINT8> m_nvram;
	required_ioport m_battery;
	optional_ioport m_pen_x;
	optional_ioport m_pen_y;

	UINT8 m_bank[4];
	UINT8 m_beep_io[5];
	UINT8 m_lcd_base[2];
	UINT8 m_touchscreen[0x10];
	UINT8 m_lcd_enabled;
	UINT8 m_lcd_cmd;

	UINT8 m_irq_mask;
	UINT8 m_irq_flag;
	UINT8 m_port6;
	UINT8 m_beep_mode;
	UINT8 m_power_on;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( bankswitch_r );
	DECLARE_WRITE8_MEMBER( bankswitch_w );
	DECLARE_READ8_MEMBER( lcd_base_r );
	DECLARE_WRITE8_MEMBER( lcd_base_w );
	DECLARE_READ8_MEMBER( beep_r );
	DECLARE_WRITE8_MEMBER( beep_w );
	DECLARE_READ8_MEMBER( lcd_io_r );
	DECLARE_WRITE8_MEMBER( lcd_io_w );
	DECLARE_READ8_MEMBER( irq_r );
	DECLARE_WRITE8_MEMBER( irq_w );
	DECLARE_READ8_MEMBER( touchscreen_r );
	DECLARE_WRITE8_MEMBER( touchscreen_w );
	DECLARE_WRITE_LINE_MEMBER( alarm_irq );
	DECLARE_WRITE_LINE_MEMBER( serial_irq );

	DECLARE_PALETTE_INIT(rex6000);
	DECLARE_INPUT_CHANGED_MEMBER(trigger_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer1);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer2);
	TIMER_DEVICE_CALLBACK_MEMBER(sec_timer);
	DECLARE_QUICKLOAD_LOAD_MEMBER(rex6000);
};


class oz750_state : public rex6000_state
{
public:
	oz750_state(const machine_config &mconfig, device_type type, const char *tag)
		: rex6000_state(mconfig, type, tag),
			m_keyboard(*this, "COL")
		{ }

	optional_ioport_array<10> m_keyboard;

	DECLARE_READ8_MEMBER( kb_status_r );
	DECLARE_READ8_MEMBER( kb_data_r );
	DECLARE_WRITE8_MEMBER( kb_mask_w );
	DECLARE_INPUT_CHANGED_MEMBER(trigger_on_irq);
	DECLARE_QUICKLOAD_LOAD_MEMBER(oz750);

	virtual void machine_reset() override;
	UINT32 screen_update_oz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	int oz_wzd_extract_tag(const dynamic_buffer &data, const char *tag, char *dest_buf);

	UINT16 m_kb_mask;
};


READ8_MEMBER( rex6000_state::bankswitch_r )
{
	return m_bank[offset];
}

WRITE8_MEMBER( rex6000_state::bankswitch_w )
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

READ8_MEMBER( rex6000_state::beep_r )
{
	return m_beep_io[offset];
}

WRITE8_MEMBER( rex6000_state::beep_w )
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
					UINT16 div = ((m_beep_io[2] | m_beep_io[3]<<8) & 0x0fff) + 2;
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

READ8_MEMBER( rex6000_state::lcd_base_r )
{
	return m_lcd_base[offset];
}

WRITE8_MEMBER( rex6000_state::lcd_base_w )
{
	m_lcd_base[offset&1] = data;
}

READ8_MEMBER( rex6000_state::lcd_io_r )
{
	return (offset == 0) ? m_lcd_enabled : m_lcd_cmd;
}

WRITE8_MEMBER( rex6000_state::lcd_io_w )
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

READ8_MEMBER( rex6000_state::irq_r )
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

WRITE8_MEMBER( rex6000_state::irq_w )
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

READ8_MEMBER( rex6000_state::touchscreen_r )
{
	UINT16 x = m_pen_x->read();
	UINT16 y = m_pen_y->read();
	UINT16 battery = m_battery->read();

	switch (offset)
	{
		case 0x08:
			return ((ioport("INPUT")->read() & 0x40) ? 0x20 : 0x00) | 0X10;
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

WRITE8_MEMBER( rex6000_state::touchscreen_w )
{
	m_touchscreen[offset&0x0f] = data;
}

READ8_MEMBER( oz750_state::kb_status_r )
{
	UINT8 data = 0x6b;
	if (m_battery->read() & 0x01)   data |= 0x80;

	return data;
}

READ8_MEMBER( oz750_state::kb_data_r )
{
	UINT8 data = 0;
	for(int i=0; i<10; i++)
	{
		if (m_kb_mask & (1<<i))
			data |= m_keyboard[i]->read();
	}

	return data;
}
WRITE8_MEMBER( oz750_state::kb_mask_w )
{
	if (offset)
		m_kb_mask = (m_kb_mask & 0x00ff) | (data << 8);
	else
		m_kb_mask = (m_kb_mask & 0xff00) | data;
}

static ADDRESS_MAP_START(rex6000_banked_map, AS_PROGRAM, 8, rex6000_state)
	AM_RANGE( 0x0000000, 0x00fffff ) AM_DEVREADWRITE("flash0a", intelfsh8_device, read, write)
	AM_RANGE( 0x0100000, 0x01fffff ) AM_DEVREADWRITE("flash0b", intelfsh8_device, read, write)
	AM_RANGE( 0x0c00000, 0x0cfffff ) AM_DEVREADWRITE("flash1a", intelfsh8_device, read, write)
	AM_RANGE( 0x0d00000, 0x0dfffff ) AM_DEVREADWRITE("flash1b", intelfsh8_device, read, write)
	AM_RANGE( 0x1600000, 0x16fffff ) AM_DEVREADWRITE("flash1a", intelfsh8_device, read, write)
	AM_RANGE( 0x1700000, 0x17fffff ) AM_DEVREADWRITE("flash1b", intelfsh8_device, read, write)
	AM_RANGE( 0x2000000, 0x2007fff ) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START(oz750_banked_map, AS_PROGRAM, 8, oz750_state)
	AM_RANGE( 0x0000000, 0x01fffff )  AM_DEVREADWRITE("flash0a", intelfsh8_device, read, write)
	AM_RANGE( 0x0200000, 0x02fffff )  AM_MIRROR(0x100000) AM_DEVREADWRITE("flash1a", intelfsh8_device, read, write)
	AM_RANGE( 0x0600000, 0x07fffff )  AM_READWRITE(lcd_io_r, lcd_io_w)
	AM_RANGE( 0x0800000, 0x083ffff )  AM_MIRROR(0x1c0000) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x0a00000, 0x0a3ffff )  AM_MIRROR(0x1c0000) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START(rex6000_mem, AS_PROGRAM, 8, rex6000_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x7fff ) AM_DEVREADWRITE("flash0a", intelfsh8_device, read, write)
	AM_RANGE( 0x8000, 0x9fff ) AM_DEVREADWRITE("bank0", address_map_bank_device, read8, write8)
	AM_RANGE( 0xa000, 0xbfff ) AM_DEVREADWRITE("bank1", address_map_bank_device, read8, write8)
	AM_RANGE( 0xc000, 0xffff ) AM_RAMBANK("ram")            //system RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( rex6000_io, AS_IO, 8, rex6000_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x04 ) AM_READWRITE(bankswitch_r, bankswitch_w)
	AM_RANGE( 0x05, 0x07 ) AM_READWRITE(irq_r, irq_w)
	AM_RANGE( 0x10, 0x10 ) AM_READ_PORT("INPUT")
	AM_RANGE( 0x15, 0x19 ) AM_READWRITE(beep_r, beep_w)
	AM_RANGE( 0x22, 0x23 ) AM_READWRITE(lcd_base_r, lcd_base_w)
	AM_RANGE( 0x30, 0x3f ) AM_DEVREADWRITE(TC8521_TAG, rp5c01_device, read, write)
	AM_RANGE( 0x40, 0x47 ) AM_MIRROR(0x08)  AM_DEVREADWRITE("ns16550", ns16550_device, ins8250_r, ins8250_w )
	AM_RANGE( 0x50, 0x51 ) AM_READWRITE(lcd_io_r, lcd_io_w)
	AM_RANGE( 0x60, 0x6f ) AM_READWRITE(touchscreen_r, touchscreen_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( oz750_io, AS_IO, 8, oz750_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x04 ) AM_READWRITE(bankswitch_r, bankswitch_w)
	AM_RANGE( 0x05, 0x08 ) AM_READWRITE(irq_r, irq_w)
	AM_RANGE( 0x10, 0x10 ) AM_READ(kb_data_r)
	AM_RANGE( 0x11, 0x12 ) AM_READWRITE(kb_status_r, kb_mask_w)
	AM_RANGE( 0x15, 0x19 ) AM_READWRITE(beep_r, beep_w)
	AM_RANGE( 0x22, 0x23 ) AM_READWRITE(lcd_base_r, lcd_base_w)
	AM_RANGE( 0x30, 0x3f ) AM_DEVREADWRITE(TC8521_TAG, rp5c01_device, read, write)
	AM_RANGE( 0x40, 0x47 ) AM_MIRROR(0x08)  AM_DEVREADWRITE("ns16550", ns16550_device, ins8250_r, ins8250_w )
ADDRESS_MAP_END

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
	membank("ram")->set_base((UINT8*)m_nvram + 0x4000);
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

UINT32 rex6000_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 lcd_bank = MAKE_BANK(m_lcd_base[0], m_lcd_base[1]);

	if (m_lcd_enabled)
	{
		for (int y=0; y<120; y++)
			for (int x=0; x<30; x++)
			{
				UINT8 data = m_bankdev0->space(AS_PROGRAM).read_byte((lcd_bank << 13) + y*30 + x);

				for (int b=0; b<8; b++)
				{
					bitmap.pix16(y, (x * 8) + b) = BIT(data, 7);
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

UINT32 oz750_state::screen_update_oz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 lcd_bank = MAKE_BANK(m_lcd_base[0], m_lcd_base[1]);

	if (m_lcd_enabled && m_power_on)
	{
		for (int y=0; y<=cliprect.max_y; y++)
			for (int x=0; x<30; x++)
			{
				UINT8 data = m_bankdev0->space(AS_PROGRAM).read_byte((lcd_bank << 13) + y*30 + x);

				for (int b=0; b<8; b++)
				{
					bitmap.pix16(y, (x * 8) + b) = BIT(data, 0);
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

WRITE_LINE_MEMBER( rex6000_state::alarm_irq )
{
	if (!(m_irq_mask & IRQ_FLAG_ALARM) && state)
	{
		m_irq_flag |= IRQ_FLAG_ALARM;
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}

WRITE_LINE_MEMBER( rex6000_state::serial_irq )
{
	if (!(m_irq_mask & IRQ_FLAG_SERIAL))
	{
		m_irq_flag |= IRQ_FLAG_SERIAL;
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}

PALETTE_INIT_MEMBER(rex6000_state, rex6000)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

QUICKLOAD_LOAD_MEMBER( rex6000_state,rex6000)
{
	static const char magic[] = "ApplicationName:Addin";
	address_space& flash = m_flash0b->space(0);
	UINT32 img_start = 0;

	dynamic_buffer data(image.length());
	image.fread(&data[0], image.length());

	if(strncmp((const char*)&data[0], magic, 21))
		return IMAGE_INIT_FAIL;

	img_start = strlen((const char*)&data[0]) + 5;
	img_start += 0xa0;  //skip the icon (40x32 pixel)

	for (UINT32 i=0; i<image.length() - img_start ;i++)
		flash.write_byte(i, data[img_start + i]);

	return IMAGE_INIT_PASS;
}

int oz750_state::oz_wzd_extract_tag(const dynamic_buffer &data, const char *tag, char *dest_buf)
{
	int tag_len = strlen(tag);
	UINT32 img_start = 0;
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
		UINT32 i;
		for (i=0; data[img_start + i] != 0 && data[img_start + i] != '\n' && data[img_start + i] != '\r'; i++)
			dest_buf[i] = data[img_start + i];

		dest_buf[i] = '\0';
	}

	return img_start;
}

QUICKLOAD_LOAD_MEMBER(oz750_state,oz750)
{
	address_space* flash = &machine().device("flash0a")->memory().space(0);
	dynamic_buffer data(image.length());
	image.fread(&data[0], image.length());

	const char *fs_type = "BSIC";
	char data_type[0x100];
	char app_name[0x100];
	char file_name[0x100];

	oz_wzd_extract_tag(data, "<DATA TYPE>", data_type);
	if (strcmp(data_type, "MY PROGRAMS"))
		return IMAGE_INIT_FAIL;

	oz_wzd_extract_tag(data, "<TITLE>", app_name);
	oz_wzd_extract_tag(data, "<DATA>", file_name);
	if (!strncmp(file_name, "PFILE:", 6))
		strcpy(file_name, file_name + 6);

	UINT32 img_start = oz_wzd_extract_tag(data, "<BIN>", NULL);

	if (img_start == 0)
		return IMAGE_INIT_FAIL;

	UINT16 icon_size = data[img_start++];

	UINT32 pos = 0xc0000;
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

	UINT16 size = (UINT16)image.length() - img_start;
	flash->write_byte(pos++, size);                             // data size LSB
	flash->write_byte(pos++, size >> 8);                        // data size MSB

	for (int i=img_start; i<image.length(); i++)
		flash->write_byte(pos++, data[i]);                      // data

	return IMAGE_INIT_PASS;
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

static GFXDECODE_START( rex6000 )
	GFXDECODE_ENTRY( "flash0a", 0x0f0000, rex6000_bold_charlayout,  0, 0 )  //normal
	GFXDECODE_ENTRY( "flash0a", 0x0f2000, rex6000_bold_charlayout,  0, 0 )  //bold
	GFXDECODE_ENTRY( "flash0a", 0x0f4000, rex6000_tiny_charlayout,  0, 0 )  //tiny
	GFXDECODE_ENTRY( "flash0a", 0x0f6000, rex6000_graph_charlayout, 0, 0 )  //graphic
GFXDECODE_END


static MACHINE_CONFIG_START( rex6000, rex6000_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz) //Toshiba microprocessor Z80 compatible at 4.3MHz
	MCFG_CPU_PROGRAM_MAP(rex6000_mem)
	MCFG_CPU_IO_MAP(rex6000_io)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("sec_timer", rex6000_state, sec_timer, attotime::from_hz(1))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_timer1", rex6000_state, irq_timer1, attotime::from_hz(32))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_timer2", rex6000_state, irq_timer2, attotime::from_hz(4096))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(rex6000_state, screen_update)
	MCFG_SCREEN_SIZE(240, 120)
	MCFG_SCREEN_VISIBLE_AREA(0, 240-1, 0, 120-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_lcd)
	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(rex6000_state, rex6000)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", rex6000)

	MCFG_DEVICE_ADD("bank0", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(rex6000_banked_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x2000)

	MCFG_DEVICE_ADD("bank1", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(rex6000_banked_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x2000)

	MCFG_DEVICE_ADD( "ns16550", NS16550, XTAL_1_8432MHz )
	MCFG_INS8250_OUT_TX_CB(DEVWRITELINE("serport", rs232_port_device, write_txd))
	MCFG_INS8250_OUT_DTR_CB(DEVWRITELINE("serport", rs232_port_device, write_dtr))
	MCFG_INS8250_OUT_RTS_CB(DEVWRITELINE("serport", rs232_port_device, write_rts))
	MCFG_INS8250_OUT_INT_CB(WRITELINE(rex6000_state, serial_irq))

	MCFG_RS232_PORT_ADD( "serport", default_rs232_devices, nullptr )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ns16550", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ns16550", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ns16550", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ns16550", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ns16550", ins8250_uart_device, cts_w))

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", rex6000_state, rex6000, "rex,ds2", 0)

	MCFG_DEVICE_ADD(TC8521_TAG, RP5C01, XTAL_32_768kHz)
	MCFG_RP5C01_OUT_ALARM_CB(WRITELINE(rex6000_state, alarm_irq))

	/*
	Fujitsu 29DL16X have feature which is capability of reading data from one
	bank of memory while a program or erase operation is in progress in the
	other bank of memory (simultaneous operation). This is not supported yet
	by the flash emulation and I have splitted every bank into a separate
	device for have a similar behavior.
	*/
	MCFG_FUJITSU_29DL16X_ADD("flash0a") //bank 0 of first flash
	MCFG_FUJITSU_29DL16X_ADD("flash0b") //bank 1 of first flash
	MCFG_FUJITSU_29DL16X_ADD("flash1a") //bank 0 of second flash
	MCFG_FUJITSU_29DL16X_ADD("flash1b") //bank 1 of second flash

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD( "beeper", BEEP, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.00 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( oz750, oz750_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_9_8304MHz) //Toshiba microprocessor Z80 compatible at 9.8MHz
	MCFG_CPU_PROGRAM_MAP(rex6000_mem)
	MCFG_CPU_IO_MAP(oz750_io)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("sec_timer", rex6000_state, sec_timer, attotime::from_hz(1))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_timer1", rex6000_state, irq_timer1, attotime::from_hz(64))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_timer2", rex6000_state, irq_timer2, attotime::from_hz(8192))

	MCFG_DEVICE_ADD( "ns16550", NS16550, XTAL_9_8304MHz / 4 )
	MCFG_INS8250_OUT_TX_CB(DEVWRITELINE("serport", rs232_port_device, write_txd))
	MCFG_INS8250_OUT_DTR_CB(DEVWRITELINE("serport", rs232_port_device, write_dtr))
	MCFG_INS8250_OUT_RTS_CB(DEVWRITELINE("serport", rs232_port_device, write_rts))
	MCFG_INS8250_OUT_INT_CB(WRITELINE(rex6000_state, serial_irq))

	MCFG_RS232_PORT_ADD( "serport", default_rs232_devices, NULL )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ns16550", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ns16550", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ns16550", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ns16550", ins8250_uart_device, ri_w))
	//MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ns16550", ins8250_uart_device, cts_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(oz750_state, screen_update_oz)
	MCFG_SCREEN_SIZE(240, 80)
	MCFG_SCREEN_VISIBLE_AREA(0, 240-1, 0, 80-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_lcd)
	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(rex6000_state, rex6000)

	MCFG_DEVICE_ADD("bank0", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(oz750_banked_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x2000)

	MCFG_DEVICE_ADD("bank1", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(oz750_banked_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x2000)

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", oz750_state, oz750, "wzd", 0)

	MCFG_DEVICE_ADD(TC8521_TAG, RP5C01, XTAL_32_768kHz)
	MCFG_RP5C01_OUT_ALARM_CB(WRITELINE(rex6000_state, alarm_irq))

	MCFG_SHARP_LH28F016S_ADD("flash0a")
	MCFG_SHARP_LH28F016S_ADD("flash1a")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("512K")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD( "beeper", BEEP, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.00 )
MACHINE_CONFIG_END

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

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY   FULLNAME       FLAGS */
COMP( 199?, oz750,    0,       0,   oz750  ,    oz750,   driver_device,  0,   "Sharp",            "Wizard OZ-750",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 2000, rex6000,  0,       0,   rex6000,    rex6000, driver_device,  0,   "Xircom / Intel",   "REX 6000",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 2000, ds2,      rex6000, 0,   rex6000,    rex6000, driver_device,  0,   "Citizen",          "DataSlim 2",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
