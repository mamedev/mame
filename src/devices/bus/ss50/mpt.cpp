// license:BSD-3-Clause
// copyright-holders:68bit
/**********************************************************************

    SWTPC MP-T Timer / counter

    MC6821 PIA + MK5009 timer/counter

**********************************************************************/

#include "emu.h"
#include "mpt.h"
#include "machine/6821pia.h"
#include "machine/mc14411.h"


class ss50_mpt_device : public device_t, public ss50_card_interface
{
public:
	ss50_mpt_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, SS50_MPT, tag, owner, clock)
		, ss50_card_interface(mconfig, *this)
		, m_pia(*this, "pia")
		, m_irqa_jumper(*this, "IRQA")
		, m_irqb_jumper(*this, "IRQB")
	{
	}

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual u8 register_read(offs_t offset) override;
	virtual void register_write(offs_t offset, u8 data) override;

private:
	void pia_b_w(uint8_t data);
	void pia_irq_b(int state);
	TIMER_CALLBACK_MEMBER(mpt_timer_callback);
	void pia_irqa_w(int state);
	void pia_irqb_w(int state);

	required_device<pia6821_device> m_pia;
	required_ioport m_irqa_jumper;
	required_ioport m_irqb_jumper;

	emu_timer *m_mpt_timer;
	attotime m_mpt_duration;
	uint8_t m_mpt_timer_state;
};


static INPUT_PORTS_START( mpt )
	PORT_START("IRQA")
	PORT_DIPNAME(1, 0, "IRQ-A")
	PORT_DIPSETTING(0, DEF_STR(Off))
	PORT_DIPSETTING(1, DEF_STR(On))

	PORT_START("IRQB")
	PORT_DIPNAME(1, 1, "IRQ-B")
	PORT_DIPSETTING(0, DEF_STR(Off))
	PORT_DIPSETTING(1, DEF_STR(On))
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor ss50_mpt_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mpt);
}


//-------------------------------------------------
//  device_add_mconfig - add device-specific
//  machine configuration
//-------------------------------------------------

void ss50_mpt_device::device_add_mconfig(machine_config &config)
{
	PIA6821(config, m_pia);
	m_pia->writepb_handler().set(FUNC(ss50_mpt_device::pia_b_w));
	m_pia->cb1_w(0);
	m_pia->irqa_handler().set(FUNC(ss50_mpt_device::pia_irqa_w));
	m_pia->irqb_handler().set(FUNC(ss50_mpt_device::pia_irqb_w));
}

void ss50_mpt_device::device_start()
{
	m_mpt_timer = timer_alloc(FUNC(ss50_mpt_device::mpt_timer_callback), this);
	m_mpt_timer_state = 0;

	save_item(NAME(m_mpt_timer_state));
}

//-------------------------------------------------
//  register_read - read from a port register
//-------------------------------------------------

u8 ss50_mpt_device::register_read(offs_t offset)
{
	return m_pia->read(offset & 3);
}

//-------------------------------------------------
//  register_write - write to a port register
//-------------------------------------------------

void ss50_mpt_device::register_write(offs_t offset, u8 data)
{
	m_pia->write(offset & 3, data);
}


void ss50_mpt_device::pia_b_w(uint8_t data)
{
	if (data & 0x80)
	{
		// Reset state.
		m_mpt_timer->enable(false);
		return;
	}

	data &= 0x0f;

	if (data == 0x0c || data == 0x0d || data == 0x0f)
	{
		// No output for these input settings.
		m_mpt_timer->enable(false);
		return;
	}

	if (data == 0x00)
	{
		// Hack: could not keep up with this 1us period
		// anyway, and this is the initial state and it blocks
		// progress, so just give up on this setting.
		m_mpt_timer->enable(false);
		return;
	}

	attotime duration;
	// Set the duration for half the period, the time that the next
	// transition of state will occur.
	switch (data & 0x0f)
	{
		case 0: duration = attotime::from_nsec(500); break;
		case 1: duration = attotime::from_usec(5); break;
		case 2: duration = attotime::from_usec(50); break;
		case 3: duration = attotime::from_usec(500); break;
		case 4: duration = attotime::from_msec(5); break;
		case 5: duration = attotime::from_msec(50); break;
		case 6: duration = attotime::from_msec(500); break;
		case 7: duration = attotime::from_seconds(5); break;
		case 8: duration = attotime::from_seconds(50); break;
		case 9: duration = attotime::from_seconds(30); break;
		case 10: duration = attotime::from_seconds(30 * 60); break;
		case 11: duration = attotime::from_seconds(5 * 60); break;
		case 14: duration = attotime::from_msec(10); break;
	}

	if (duration != m_mpt_duration)
	{
		// TODO when would a change be latched?
		m_mpt_timer->adjust(duration);
		m_mpt_duration = duration;
	}
	m_mpt_timer->enable(true);
}

TIMER_CALLBACK_MEMBER(ss50_mpt_device::mpt_timer_callback)
{
	m_mpt_timer_state = !m_mpt_timer_state;
	m_pia->cb1_w(m_mpt_timer_state);
	m_mpt_timer->adjust(m_mpt_duration);
	m_mpt_timer->enable(true);
}

void ss50_mpt_device::pia_irqa_w(int state)
{
	if (m_irqa_jumper->read())
		write_irq(state);
}

void ss50_mpt_device::pia_irqb_w(int state)
{
	if (m_irqb_jumper->read())
		write_irq(state);
}


// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(SS50_MPT, ss50_card_interface, ss50_mpt_device, "ss50_mpt", "MP-T Timer / Counter")
