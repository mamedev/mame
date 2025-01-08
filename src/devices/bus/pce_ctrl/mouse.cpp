// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***********************************************************************

    NEC PC Engine/TurboGrafx-16 Mouse emulation

***********************************************************************/

#include "emu.h"
#include "mouse.h"

#include <algorithm>
#include <iterator>

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace {

INPUT_PORTS_START( pce_mouse )
	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("%p Button I (right)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("%p Button II (left)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SELECT  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START   ) PORT_NAME("%p Run")

	PORT_START("X")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(1) PORT_REVERSE

	PORT_START("Y")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(1) PORT_REVERSE
INPUT_PORTS_END


class pce_mouse_device : public device_t, public device_pce_control_port_interface
{
public:
	pce_mouse_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual u8 peripheral_r() override;
	virtual void sel_w(int state) override { m_sel_in = state ? 1 : 0; }
	virtual void clr_w(int state) override;

protected:
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(pce_mouse); }
	virtual void device_start() override ATTR_COLD;

private:
	required_ioport m_buttons;
	required_ioport_array<2> m_axes;

	attotime m_squeak;
	u16 m_data;
	u8 m_base[2];
	u8 m_sel_in;
	u8 m_clr_in;
};


pce_mouse_device::pce_mouse_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, PCE_MOUSE, tag, owner, clock),
	device_pce_control_port_interface(mconfig, *this),
	m_buttons(*this, "BUTTONS"),
	m_axes(*this, "%c", 'X'),
	m_squeak(),
	m_data(0),
	m_sel_in(1),
	m_clr_in(1)
{
}


u8 pce_mouse_device::peripheral_r()
{
	if (m_clr_in)
		return 0x0f;
	else if (!m_sel_in)
		return m_buttons->read();
	else
		return BIT(m_data, 12, 4);
}


void pce_mouse_device::clr_w(int state)
{
	u8 const clr = state ? 1 : 0;
	if (clr != m_clr_in)
	{
		m_clr_in = clr;
		if (clr)
		{
			attotime const now = machine().time();
			if ((now - m_squeak) > attotime::from_usec(550))
			{
				LOG("%s: CLR rising, latch count\n", machine().describe_context());
				m_squeak = now;
				for (unsigned i = 0; m_axes.size() > i; ++i)
				{
					uint8_t const count = m_axes[i]->read();
					m_data <<= 8;
					m_data |= (count - m_base[i]) & 0x00ff;
					m_base[i] = count;
				}
			}
			else
			{
				LOG("%s: CLR rising, shift count\n", machine().describe_context());
				m_data <<= 4;
			}
		}
		else
		{
			LOG("%s: CLR falling\n", machine().describe_context());
		}
	}
}


void pce_mouse_device::device_start()
{
	m_squeak = machine().time();
	m_data = 0;
	std::fill(std::begin(m_base), std::end(m_base), 0);

	save_item(NAME(m_squeak));
	save_item(NAME(m_data));
	save_item(NAME(m_base));
	save_item(NAME(m_sel_in));
	save_item(NAME(m_clr_in));
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(PCE_MOUSE, device_pce_control_port_interface, pce_mouse_device, "pce_mouse", "NEC PC Engine Mouse")
