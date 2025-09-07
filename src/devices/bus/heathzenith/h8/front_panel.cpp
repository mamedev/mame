// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit H8 Front Panel card

****************************************************************************/

#include "emu.h"
#include "front_panel.h"

#include "machine/timer.h"
#include "sound/beep.h"
#include "speaker.h"

#include "h8_fp.lh"

#define LOG_PORT_W   (1U << 1)
#define LOG_PORT_R   (1U << 2)
#define LOG_SS       (1U << 3)

//#define VERBOSE      (LOG_SS)

#include "logmacro.h"

#define LOGPORTW(...)      LOGMASKED(LOG_PORT_W, __VA_ARGS__)
#define LOGPORTR(...)      LOGMASKED(LOG_PORT_R, __VA_ARGS__)
#define LOGSS(...)         LOGMASKED(LOG_SS, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


class front_panel_device : public device_t
						 , public device_h8bus_card_interface
						 , public device_p201_p1_card_interface
{
public:
	front_panel_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual void m1_w(int state) override;
	virtual void p201_inte_w(int state) override;

	DECLARE_INPUT_CHANGED_MEMBER(button_0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void map_io(address_space_installer & space) override ATTR_COLD;

	u8 portf0_r();
	void portf0_w(u8 data);
	void portf1_w(u8 data);

	TIMER_DEVICE_CALLBACK_MEMBER(h8_irq_pulse);
	TIMER_CALLBACK_MEMBER(turn_off_run_led_timer);

	u8   m_digit;
	u8   m_segment;
	u8   m_int_en;
	bool m_ic108a;
	bool m_ic108b;
	bool m_allow_int1;
	bool m_allow_int2;

	required_device<beep_device> m_beep;
	required_ioport_array<2>     m_io_keyboard;
	output_finder<16>            m_digits;
	output_finder<>              m_mon_led;
	output_finder<>              m_pwr_led;
	output_finder<>              m_ion_led;
	output_finder<>              m_run_led;
	emu_timer                   *m_run_led_timer;
};


front_panel_device::front_panel_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, H8BUS_FRONT_PANEL, tag, owner, 0)
	, device_h8bus_card_interface(mconfig, *this)
	, device_p201_p1_card_interface(*this, tag)
	, m_beep(*this, "beeper")
	, m_io_keyboard(*this, "X%u", 0U)
	, m_digits(*this, "digit%u", 0U)
	, m_mon_led(*this, "mon_led")
	, m_pwr_led(*this, "pwr_led")
	, m_ion_led(*this, "ion_led")
	, m_run_led(*this, "run_led")
{
}

void front_panel_device::m1_w(int state)
{
	// operate the RUN LED
	if (state)
	{
		m_run_led = 0;
		m_run_led_timer->adjust(attotime::from_nsec(4700));
	}

	// For Single Instruction(SI) mode, there are 2 D flipflops, ic108a and ic108b.  Both are held in
	// set mode while not in SI mode.  The data for ic108a is /INTE and ic108a Q is connected to data on
	// ic108b. Both use /M1 for the clock. When the system is in SI mode, an int20 will trigger after
	// after 2 M1 cycles, to pause the running program.
	if (m_allow_int2)
	{
		LOGSS("%s: m_int_en: %d, state: %d\n", FUNCNAME, m_int_en, state);

		if (state)
		{
			m_ic108b = !m_ic108a;
			m_ic108a = !m_int_en;

			if (m_ic108b)
			{
				m_p201_int2(ASSERT_LINE);
			}
		}
	}
}

void front_panel_device::p201_inte_w(int state)
{
	// Operate the ION LED
	m_ion_led = !state;

	m_int_en = bool(state);
}

u8 front_panel_device::portf0_r()
{
	u8 data = 0xff;

	// reads the keyboard
	u8 keyin = m_io_keyboard[0]->read() ^ 0xff;

	if (keyin)
	{
		for (int i = 1; i < 8; i++)
		{
			if (BIT(keyin, i))
			{
				data &= ~(i << 1);
			}
		}

		data &= 0xfe;
	}

	keyin = m_io_keyboard[1]->read() ^ 0xff;

	if (keyin)
	{
		for (int i = 1; i < 8; i++)
		{
			if (BIT(keyin, i))
			{
				data &= ~(i << 5);
			}
		}

		data &= 0xef;
	}

	LOGPORTR("%s: val: 0x%02x\n", FUNCNAME, data);

	return data;
}

void front_panel_device::portf0_w(u8 data)
{
	//  bit     description
	// ----------------------
	// d0-d3    digit selected
	//  d4      inhibit int20
	//  d5      mon led
	//  d6      allow int10
	//  d7      beeper on

	LOGPORTW("%s: val: 0x%02x\n", FUNCNAME, data);

	// writes to this port always turn off int10
	m_p201_int1(CLEAR_LINE);

	m_digit = data & 0xf;
	if (m_digit)
	{
		m_digits[m_digit] = m_segment;
	}

	m_mon_led = !BIT(data, 5);
	m_beep->set_state(!BIT(data, 7));

	m_allow_int1 = BIT(data, 6);

	// Setup Single Instruction interrupt
	bool old_allow_int2 = m_allow_int2;

	m_allow_int2 = bool(!BIT(data, 4));

	if (old_allow_int2 != m_allow_int2)
	{
		LOGSS("%s: value changed: oldval: %d, allow_int2: %d\n", FUNCNAME, old_allow_int2, m_allow_int2);

		if (!m_allow_int2)
		{
			m_ic108a = true;
			m_ic108b = false;
			m_p201_int2(CLEAR_LINE);
		}
	}
}

void front_panel_device::portf1_w(u8 data)
{
	// bit   segment
	// --------------
	//  d7     dot
	//  d6      f
	//  d5      e
	//  d4      d
	//  d3      c
	//  d2      b
	//  d1      a
	//  d0      g

	LOGPORTW("%s: val: 0x%02x\n", FUNCNAME, data);

	m_segment = bitswap<8>(data, 7, 0, 6, 5, 4, 3, 2, 1) ^ 0xff;

	if (m_digit)
	{
		m_digits[m_digit] = m_segment;
	}
}

void front_panel_device::map_io(address_space_installer & space)
{
	space.install_readwrite_handler(0xf0, 0xf0,
		read8smo_delegate(*this, FUNC(front_panel_device::portf0_r)),
		write8smo_delegate(*this, FUNC(front_panel_device::portf0_w))
	);

	space.install_write_handler(0xf1, 0xf1,
		write8smo_delegate(*this, FUNC(front_panel_device::portf1_w))
	);
}

// Input ports
static INPUT_PORTS_START( h8 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("0")           PORT_CODE(KEYCODE_0)      PORT_CHAR('0') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(front_panel_device::button_0), 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("1 SP")        PORT_CODE(KEYCODE_1)      PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("2 AF")        PORT_CODE(KEYCODE_2)      PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("3 BC")        PORT_CODE(KEYCODE_3)      PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("4 DE GO")     PORT_CODE(KEYCODE_4)      PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("5 HL IN")     PORT_CODE(KEYCODE_5)      PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("6 PC OUT")    PORT_CODE(KEYCODE_6)      PORT_CHAR('6')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("7 SI")        PORT_CODE(KEYCODE_7)      PORT_CHAR('7')

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("8 LOAD")      PORT_CODE(KEYCODE_8)      PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("9 DUMP")      PORT_CODE(KEYCODE_9)      PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("+")           PORT_CODE(KEYCODE_UP)     PORT_CHAR('^')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("-")           PORT_CODE(KEYCODE_DOWN)   PORT_CHAR('V')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("* CANCEL")    PORT_CODE(KEYCODE_ESC)    PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("/ ALTER RST") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("# MEM RTM")   PORT_CODE(KEYCODE_MINUS)  PORT_CHAR('-')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(". REG")       PORT_CODE(KEYCODE_R)      PORT_CHAR('R')
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(front_panel_device::button_0)
{
	if (newval)
	{
		u8 data = m_io_keyboard[1]->read() ^ 0xff;

		if (BIT(data, 5))
		{
			m_p201_reset(ASSERT_LINE);
		}
		else if (BIT(data, 6))
		{
			m_p201_int1(ASSERT_LINE);
		}
		else
		{
			m_p201_reset(CLEAR_LINE);
			m_p201_int1(CLEAR_LINE);
		}
	}
}

void front_panel_device::device_start()
{
	m_digits.resolve();
	m_mon_led.resolve();
	m_pwr_led.resolve();
	m_ion_led.resolve();
	m_run_led.resolve();

	m_digit   = 0;
	m_segment = 0;

	save_item(NAME(m_digit));
	save_item(NAME(m_segment));
	save_item(NAME(m_ic108a));
	save_item(NAME(m_ic108b));
	save_item(NAME(m_int_en));
	save_item(NAME(m_allow_int1));
	save_item(NAME(m_allow_int2));

	m_run_led_timer = timer_alloc(FUNC(front_panel_device::turn_off_run_led_timer), this);
}

void front_panel_device::device_reset()
{
	m_allow_int1 = true;
	m_allow_int2 = false;
	m_int_en     = false;
	m_ic108a     = true;
	m_ic108b     = false;

	// Note: 0 means on and 1 means off
	m_pwr_led = 0;
	m_run_led = 1;
	m_ion_led = 1;
	m_mon_led = 1;
}

void front_panel_device::device_add_mconfig(machine_config &config)
{
	config.set_default_layout(layout_h8_fp);

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 1000).add_route(ALL_OUTPUTS, "mono", 1.00);

	TIMER(config, "h8_timer").configure_periodic(FUNC(front_panel_device::h8_irq_pulse), attotime::from_msec(2));
}

ioport_constructor front_panel_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(h8);
}

TIMER_DEVICE_CALLBACK_MEMBER(front_panel_device::h8_irq_pulse)
{
	if (m_allow_int1)
	{
		m_p201_int1(ASSERT_LINE);
	}
}

TIMER_CALLBACK_MEMBER(front_panel_device::turn_off_run_led_timer)
{
	m_run_led = 1;
}

DEFINE_DEVICE_TYPE_PRIVATE(H8BUS_FRONT_PANEL, device_h8bus_card_interface, front_panel_device, "h8_fp", "Heath H-8 Front Panel");
