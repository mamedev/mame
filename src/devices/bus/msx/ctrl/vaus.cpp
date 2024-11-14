// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    MSX Arkanoid Vaus controller emulation

**********************************************************************/

#include "emu.h"
#include "vaus.h"


namespace {

INPUT_PORTS_START(msx_vaus)
	PORT_START("BUTTON")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON1)

	PORT_START("POT")
	PORT_BIT(0x1ff, 0x100, IPT_PADDLE) PORT_MINMAX(110, 380) PORT_SENSITIVITY(30) PORT_KEYDELTA(20)
INPUT_PORTS_END


class msx_vaus_device : public device_t, public device_msx_general_purpose_port_interface
{
public:
	msx_vaus_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read() override;
	virtual void pin_6_w(int state) override;
	virtual void pin_8_w(int state) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(msx_vaus); }
	TIMER_CALLBACK_MEMBER(copy_counter);

private:
	required_ioport m_button;
	required_ioport m_pot;
	emu_timer *m_timer;
	u8 m_old_pin6;
	u8 m_old_pin8;
	u16 m_counter;
	u16 m_shift_reg;
};

msx_vaus_device::msx_vaus_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_VAUS, tag, owner, clock)
	, device_msx_general_purpose_port_interface(mconfig, *this)
	, m_button(*this, "BUTTON")
	, m_pot(*this, "POT")
	, m_timer(nullptr)
{
}

void msx_vaus_device::device_start()
{
	m_timer = timer_alloc(FUNC(msx_vaus_device::copy_counter), this);
	save_item(NAME(m_old_pin6));
	save_item(NAME(m_old_pin8));
	save_item(NAME(m_counter));
	save_item(NAME(m_shift_reg));
}

void msx_vaus_device::device_reset()
{
	m_old_pin8 = 0;
	m_counter = 0;
	m_shift_reg = 0;
}

u8 msx_vaus_device::read()
{
	// bit 0/pin 1 - shift register output
	// bit 1/pin 2 - button
	return (m_button->read() & 0x02) | (BIT(m_shift_reg, 8) ? 0x01 : 0x00) | 0xfc;
}

void msx_vaus_device::pin_6_w(int state)
{
	if (!m_old_pin6 && state)
	{
		m_shift_reg <<= 1;
	}
	m_old_pin6 = state;
}

void msx_vaus_device::pin_8_w(int state)
{
	if (!state)
	{
		m_counter = 0;
		m_timer->adjust(attotime::never);
	}
	else if (!m_old_pin8)
	{
		// start counting; the counter is incremented on a ~96k2 clock
		m_counter = m_pot->read();
		m_timer->adjust(attotime::from_ticks(m_counter, 96200));
	}
	m_old_pin8 = state;
}

TIMER_CALLBACK_MEMBER(msx_vaus_device::copy_counter)
{
	m_shift_reg = m_counter;
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MSX_VAUS, device_msx_general_purpose_port_interface, msx_vaus_device, "msx_vaus", "Taito Arkanoid Vaus Controller (MSX)")
