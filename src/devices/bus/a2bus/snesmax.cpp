// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#include "emu.h"
#include "snesmax.h"

#include "bus/snes_ctrl/ctrl.h"

namespace {

class a2bus_snes_max_device : public device_t, public device_a2bus_card_interface
{
public:
	a2bus_snes_max_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock) :
		device_t(mconfig, A2BUS_SNES_MAX, tag, owner, clock),
		device_a2bus_card_interface(mconfig, *this),
		m_controllers(*this, "%u", 1U),
		m_latch_timer(nullptr),
		m_data(0xff)
	{
	}

	virtual u8 read_c0nx(u8 offset) override
	{
		return m_data;
	}

	virtual void write_c0nx(u8 offset, u8 data) override
	{
		if (BIT(offset, 0))
		{
			m_data =
					(m_controllers[0]->read_pin4() ? 0x00 : 0x80) |
					(m_controllers[1]->read_pin4() ? 0x00 : 0x40) |
					0x3f;
		}
		else
		{
			machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_snes_max_device::set_latch), this), 0);
		}
	}

	virtual bool take_c800() override
	{
		return false;
	}

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD
	{
		SNES_CONTROL_PORT(config, m_controllers[0], snes_control_port_devices, "joypad");
		SNES_CONTROL_PORT(config, m_controllers[1], snes_control_port_devices, "joypad");
	}

	virtual void device_start() override ATTR_COLD
	{
		m_latch_timer = timer_alloc(FUNC(a2bus_snes_max_device::reset_latch), this);

		m_data = 0xff;

		save_item(NAME(m_data));

		m_controllers[0]->write_strobe(0);
		m_controllers[1]->write_strobe(0);
	}

private:
	TIMER_CALLBACK_MEMBER(set_latch)
	{
		m_controllers[0]->write_strobe(1);
		m_controllers[1]->write_strobe(1);
		m_latch_timer->adjust(attotime::from_ticks(7, clock()));
	}

	TIMER_CALLBACK_MEMBER(reset_latch)
	{
		m_controllers[0]->write_strobe(0);
		m_controllers[1]->write_strobe(0);

		m_data =
				(m_controllers[0]->read_pin4() ? 0x00 : 0x80) |
				(m_controllers[1]->read_pin4() ? 0x00 : 0x40) |
				0x3f;
	}

	required_device_array<snes_control_port_device, 2> m_controllers;
	emu_timer *m_latch_timer;
	u8 m_data;
};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_SNES_MAX, device_a2bus_card_interface, a2bus_snes_max_device, "a2snesmax", "SNES MAX Game Controller Interface")
