// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    Mountain Hardware Apple Clock

    This early real time clock card for the Apple II was released
    in 1979 by Mountain Hardware, Inc. (which became Mountain
    Computer, Inc. the following year); it was later sold as
    The Clock and also distributed by Apple Computer (A2M0024).

    The card uses a chain of battery-backed CMOS 4518 decade counters
    counting microseconds (readable to the millisecond) and 4040
    ripple counters (extended by a 4013 flip-flop stage) counting
    seconds with a period of about 388 days. A 1 second interrupt is
    also provided. (Other rates can be selected by jumpering various
    counter outputs, but they are not supported here.)

    The manual recommends that this card be placed in slot #4,
    though software that supports it will usually recognize it in
    other slots as well.

    The write-protect switch should be kept on except when the date
    and time need to be set; it serves to prevent the clock from
    glitching when system power is switched off.

*********************************************************************/

#include "emu.h"
#include "a2clock.h"

#include "dirtc.h"
#include "timeconv.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2clock_device : public device_t, public device_a2bus_card_interface, public device_rtc_interface
{
public:
	// device type constructor
	a2clock_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_a2bus_card_interface implementation
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual u8 read_cnxx(u8 offset) override;
	virtual u8 read_c800(u16 offset) override;
	virtual bool take_c800() const override;
	virtual void reset_from_bus() override;

	// device_rtc_interface implementation
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:
	// timer callback
	TIMER_CALLBACK_MEMBER(advance_seconds);

	// internal controls
	void set_run(bool run);
	void set_irq(bool irq);
	void do_adv1();
	void do_adv2();

	// firmware ROM
	required_region_ptr<u8> m_firmware;

	// configuration port
	required_ioport m_switches;

	// the timer
	emu_timer *m_clock_timer;

	// internal state
	u32 m_seconds;
	u32 m_useconds;
	bool m_run_clock;
	bool m_int_enable;
	bool m_int_active;
};

//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

a2clock_device::a2clock_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, APPLE_CLOCK, tag, owner, clock)
	, device_a2bus_card_interface(mconfig, *this)
	, device_rtc_interface(mconfig, *this)
	, m_firmware(*this, "firmware")
	, m_switches(*this, "SWITCHES")
	, m_seconds(0)
	, m_useconds(0)
	, m_run_clock(true)
	, m_int_enable(false)
	, m_int_active(false)
{
}

void a2clock_device::device_start()
{
	m_clock_timer = timer_alloc(FUNC(a2clock_device::advance_seconds), this);
	m_clock_timer->adjust(attotime(1, 0));

	save_item(NAME(m_seconds));
	save_item(NAME(m_run_clock));
	save_item(NAME(m_int_enable));
	save_item(NAME(m_int_active));
}

void a2clock_device::device_reset()
{
	reset_from_bus();
}

TIMER_CALLBACK_MEMBER(a2clock_device::advance_seconds)
{
	m_seconds = (m_seconds + 1) & 0x1ffffff;
	m_useconds = 0;

	set_irq(m_int_enable);

	m_clock_timer->adjust(attotime(1, 0));
}

void a2clock_device::set_run(bool run)
{
	if (m_run_clock != run)
	{
		logerror("%s: Run flip-flop set to %s\n", machine().describe_context(), run ? "true" : "false");
		if (run)
			m_clock_timer->adjust(attotime::from_usec(std::max<u32>(999999 - m_useconds, 0)));
		else
		{
			m_useconds = std::max<u32>(999999 - m_clock_timer->remaining().as_ticks(1_MHz_XTAL), 0);
			m_clock_timer->enable(false);
		}
		m_run_clock = run;
	}
}

void a2clock_device::set_irq(bool irq)
{
	if (irq)
	{
		m_int_active = true;
		raise_slot_irq();
	}
	else
	{
		m_int_active = false;
		lower_slot_irq();
	}
}

void a2clock_device::do_adv1()
{
	if (!m_run_clock)
		m_seconds = (m_seconds + 1) & 0x1ffffff;

	if (m_int_active)
		logerror("%s: IRQ cleared\n", machine().describe_context());
	set_irq(false);
}

void a2clock_device::do_adv2()
{
	if (!m_run_clock)
		m_seconds = (m_seconds + 0x1000) & 0x1ffffff;
}

u8 a2clock_device::read_c0nx(u8 offset)
{
	switch (offset & 0x0f)
	{
	case 0:
		return (m_int_active ? 0 : 0x80) | (m_switches->read() & 0x40) | 0x20 | BIT(m_seconds, 20, 5);

	case 1:
		return BIT(m_seconds, 12, 8);

	case 2:
		return BIT(m_seconds, 4, 8);

	case 3:
		return BIT(m_seconds, 0, 4) << 4 | (m_run_clock ? std::max(9 - int(m_clock_timer->remaining().as_ticks(10)), 0) : m_useconds / 100000);

	case 4:
		return convert_to_bcd(99 - int(m_clock_timer->remaining().as_ticks(1000)) % 100);

	case 5:
		// Start clock
		if (!machine().side_effects_disabled())
			set_run(true);
		break;

	case 6:
		// Stop clock
		if (!machine().side_effects_disabled())
			set_run(!BIT(m_switches->read(), 7));
		break;

	case 7:
		// Advance one or clear IRQ
		if (!machine().side_effects_disabled())
			do_adv1();
		break;

	case 8:
		// Advance two or clear INT OUT
		if (!machine().side_effects_disabled())
			do_adv2();
		break;
	}

	return 0xff; // no input to 74LS367 buffers
}

void a2clock_device::write_c0nx(u8 offset, u8 data)
{
	switch (offset & 0x0f)
	{
	case 5:
		// Start clock
		set_run(true);
		break;

	case 6:
		// Stop clock
		set_run(!BIT(m_switches->read(), 7));
		break;

	case 7:
		// Advance one or clear IRQ
		do_adv1();
		break;

	case 8:
		// Advance one or clear INT OUT
		do_adv2();
		break;

	case 9:
		// Set interrupt
		m_int_enable = BIT(data, 0);
		break;
	}
}

u8 a2clock_device::read_cnxx(u8 offset)
{
	return m_firmware[offset];
}

u8 a2clock_device::read_c800(u16 offset)
{
	if (offset >= 0x780)
		return 0xff; // TODO: deselect card
	else
		return m_firmware[offset & 0x3ff];
}

bool a2clock_device::take_c800() const
{
	return true;
}

void a2clock_device::reset_from_bus()
{
	m_int_enable = false;
}

void a2clock_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	for (int i = 1; i < month; i++)
		day += gregorian_days_in_month(i, year);

	m_seconds = ((u32(day - 1) * 24 + hour) * 60 + minute) * 60 + second;

	// Set the LY switch so that the correct date will be returned for the given year
	for (ioport_field &field : m_switches->fields())
	{
		if (field.mask() == 0x40)
		{
			ioport_field::user_settings settings;
			field.get_user_settings(settings);
			settings.value = gregorian_is_leap_year(year) ? 0x40 : 0x00;
			field.set_user_settings(settings);
			break;
		}
	}
}

static INPUT_PORTS_START(a2clock)
	PORT_START("SWITCHES")
	PORT_DIPNAME(0x80, 0x00, "Write Protect") PORT_DIPLOCATION("S1:1")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x00, "Leap Year") PORT_DIPLOCATION("S1:2")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x40, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, DEF_STR(Unused)) PORT_DIPLOCATION("S1:3")
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, DEF_STR(Unused)) PORT_DIPLOCATION("S1:4")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
INPUT_PORTS_END

ioport_constructor a2clock_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(a2clock);
}

ROM_START(a2clock)
	ROM_REGION(0x400, "firmware", 0)
	ROM_LOAD("mountain computer apple clock rom.bin", 0x000, 0x400, CRC(c372ba00) SHA1(a852a519db87c205f4441c16f91954fdcd599a27))
ROM_END

const tiny_rom_entry *a2clock_device::device_rom_region() const
{
	return ROM_NAME(a2clock);
}

} // anonymous namespace

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************
//
// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(APPLE_CLOCK, device_a2bus_card_interface, a2clock_device, "a2clock", "Mountain Hardware Apple Clock")
