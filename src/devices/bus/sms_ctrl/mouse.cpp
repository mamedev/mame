// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Sega Mega Mouse emulation

    The Sega Mouse sold in Japan and Europe has only two buttons but
    is otherwise compatible.

**********************************************************************/

#include "emu.h"
#include "mouse.h"

#include <algorithm>

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace {

INPUT_PORTS_START( sms_megamouse )
	PORT_START("BUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("%p A (Left Button)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("%p C (Right Button)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("%p B (Middle Button)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_START)

	PORT_START("X")
	PORT_BIT( 0x1fff, 0x0000, IPT_MOUSE_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(1)

	PORT_START("Y")
	PORT_BIT( 0x1fff, 0x0000, IPT_MOUSE_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(1) PORT_REVERSE
INPUT_PORTS_END


INPUT_PORTS_START( sms_segamouse )
	PORT_INCLUDE(sms_megamouse)

	PORT_MODIFY("BUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("%p Left Button")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("%p Right Button")
	PORT_BIT(0x0c, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END



class sms_megamouse_device : public device_t, public device_sms_control_interface
{
public:
	sms_megamouse_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual u8 in_r() override { return m_out; }
	virtual void out_w(u8 data, u8 mem_mask) override;

protected:
	sms_megamouse_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(sms_megamouse); }
	virtual void device_start() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(next_step);

	required_ioport m_buttons;
	required_ioport_array<2> m_axes;

	emu_timer *m_busy_timer;

	u16 m_base[2];
	s16 m_count[2];
	u8 m_th_in;
	u8 m_tr_in;
	u8 m_out;
	u8 m_phase;
};


sms_megamouse_device::sms_megamouse_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	sms_megamouse_device(mconfig, SMS_MEGAMOUSE, tag, owner, clock)
{
}


sms_megamouse_device::sms_megamouse_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_sms_control_interface(mconfig, *this),
	m_buttons(*this, "BUTTONS"),
	m_axes(*this, "%c", 'X'),
	m_busy_timer(nullptr),
	m_th_in(1),
	m_tr_in(1),
	m_out(0x30),
	m_phase(0)
{
}


void sms_megamouse_device::out_w(u8 data, u8 mem_mask)
{
	u8 const th = BIT(data, 6);
	u8 const tr = BIT(data, 5);
	if (th != m_th_in)
	{
		// seems to respond to TH pretty quickly
		if (th)
		{
			LOG("%s: TH rising, deselected\n", machine().describe_context());
			m_busy_timer->enable(false);
			m_out = 0x30;
			m_phase = 0;
		}
		else
		{
			LOG("%s: TH falling, selected\n", machine().describe_context());
			m_out = 0x3b;

			if (!tr)
			{
				LOG("%s: TR low, wait for data update\n", machine().describe_context());
				m_busy_timer->adjust(attotime::from_nsec(7200), 0);
			}
		}
		m_th_in = th;
		m_tr_in = tr;
	}
	else if (!m_th_in && (tr != m_tr_in))
	{
		if (m_busy_timer->enabled())
		{
			LOG(
					"%s: busy, deferring response to TH %s\n",
					machine().describe_context(),
					th ? "rising" : "falling");
		}
		else
		{
			LOG(
					"%s: TR %s, wait for data update\n",
					machine().describe_context(),
					tr ? "rising" : "falling");
			m_busy_timer->adjust(attotime::from_nsec(7200), tr);
		}
		m_tr_in = tr;
	}
}


void sms_megamouse_device::device_start()
{
	m_busy_timer = timer_alloc(FUNC(sms_megamouse_device::next_step), this);

	std::fill(std::begin(m_base), std::end(m_base), 0);
	std::fill(std::begin(m_count), std::end(m_count), 0);
	m_out = 0x30;
	m_phase = 0;

	save_item(NAME(m_base));
	save_item(NAME(m_count));
	save_item(NAME(m_th_in));
	save_item(NAME(m_tr_in));
	save_item(NAME(m_out));
	save_item(NAME(m_phase));
}


TIMER_CALLBACK_MEMBER(sms_megamouse_device::next_step)
{
	if (BIT(param, 1))
	{
		u8 const tr = BIT(param, 0);
		LOG("update TL = %u\n", tr);
		if (tr)
			m_out |= 0x10;
		else
			m_out &= ~0x10;

		assert(!m_th_in);
		if (tr != m_tr_in)
		{
			LOG("TR %s, wait for data update\n", tr ? "high" : "low");
			m_busy_timer->adjust(attotime::from_nsec(7200), tr);
		}
	}
	else
	{
		switch (m_phase)
		{
		case 0:
			if (!BIT(param, 0))
			{
				for (unsigned i = 0; m_axes.size() > i; ++i)
				{
					u16 const current = m_axes[i]->read();
					m_count[i] = s16(current) - s16(m_base[i]);
					m_base[i] = current;
					if (0x1000 < m_count[i])
						m_count[i] -= 0x2000;
					else if (-0x1000 > m_count[i])
						m_count[i] += 0x2000;
				}
				LOG("movement (%d %d)\n", m_count[0], m_count[1]);
			}
			m_out = (m_out & 0x30) | 0x0f;
			LOG("update data unused = 0x%X\n", m_out & 0x0f);
			break;
		case 1:
			if (BIT(param, 0))
			{
				m_out = (m_out & 0x30) | (m_buttons->read() & 0x0f);
				LOG("update data buttons = 0x%X\n", m_out & 0x0f);
			}
			else
			{
				m_out =
						(m_out & 0x30) |
						((util::sext(m_count[1], 9) != m_count[1]) ? 0x80 : 0x00) |
						((util::sext(m_count[0], 9) != m_count[0]) ? 0x40 : 0x00) |
						(BIT(m_count[1], 8) << 1) |
						(BIT(m_count[0], 8) << 0);
				LOG("update data flags = 0x%X\n", m_out & 0x0f);
			}
			break;
		case 2:
		case 3:
			m_out = (m_out & 0x30) | BIT(m_count[BIT(m_phase, 0)], BIT(param, 0) ? 0 : 4, 4);
			LOG(
					"update data %c %s = 0x%X\n",
					'X' + BIT(m_phase, 0),
					BIT(param, 0) ? "low" : "high",
					m_out & 0x0f);
			break;
		}
		if (BIT(param, 0))
			m_phase = (m_phase + 1) & 0x03;
		m_busy_timer->adjust(attotime::from_nsec(4800), param | 0x02);
	}
}



class sms_segamouse_device : public sms_megamouse_device
{
public:
	sms_segamouse_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(sms_segamouse); }
};


sms_segamouse_device::sms_segamouse_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	sms_megamouse_device(mconfig, SMS_SEGAMOUSE, tag, owner, clock)
{
}

} // anonymous namespace



DEFINE_DEVICE_TYPE_PRIVATE(SMS_MEGAMOUSE, device_sms_control_interface, sms_megamouse_device, "sms_megamouse", "Sega Mega Mouse (US)")
DEFINE_DEVICE_TYPE_PRIVATE(SMS_SEGAMOUSE, device_sms_control_interface, sms_segamouse_device, "sms_segamouse", "Sega Mega Drive Mouse (World)")
