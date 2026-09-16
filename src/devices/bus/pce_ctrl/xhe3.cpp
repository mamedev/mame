// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***********************************************************************

    Micom Soft XHE-3 PC Joystick Adapter for PC Engine emuation

***********************************************************************/

#include "emu.h"
#include "xhe3.h"

#include "bus/msx/ctrl/ctrl.h"


namespace {

INPUT_PORTS_START( pce_xhe3 )
	PORT_START("BUTTONS") // using IPT_START/IPT_SELECT would steal a player index
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Run")
INPUT_PORTS_END



class pce_xhe3_device : public device_t, public device_pce_control_port_interface
{
public:
	pce_xhe3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual u8 peripheral_r() override;
	virtual void sel_w(int state) override;
	virtual void clr_w(int state) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(pce_xhe3); }
	virtual void device_start() override ATTR_COLD;

private:
	required_device<msx_general_purpose_port_device> m_port;
	required_ioport m_buttons;

	u8 m_sel_in;
};


pce_xhe3_device::pce_xhe3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, PCE_XHE3, tag, owner, clock),
	device_pce_control_port_interface(mconfig, *this),
	m_port(*this, "joy"),
	m_buttons(*this, "BUTTONS"),
	m_sel_in(1)
{
}


u8 pce_xhe3_device::peripheral_r()
{
	if (m_sel_in)
		return bitswap<4>(m_port->read(), 2, 1, 3, 0);
	else
		return bitswap<2>(m_port->read(), 5, 4) | (m_buttons->read() << 2);
}


void pce_xhe3_device::sel_w(int state)
{
	m_sel_in = state ? 1 : 0;
}


void pce_xhe3_device::clr_w(int state)
{
	m_port->pin_8_w(state);
}


void pce_xhe3_device::device_add_mconfig(machine_config &config)
{
	MSX_GENERAL_PURPOSE_PORT(config, m_port, msx_general_purpose_port_devices, "xe1ap");
}


void pce_xhe3_device::device_start()
{
	save_item(NAME(m_sel_in));
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(PCE_XHE3, device_pce_control_port_interface, pce_xhe3_device, "pce_xhe3", "Micom Soft XHE-3 PC Joystick Adapter for PC Engine")
