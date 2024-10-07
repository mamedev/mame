// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega SG-1000/Mark-III/SMS "Rapid Fire Unit" emulation


Release data from the Sega Retro project:

  Year: 1985    Country/region: JP    Model code: RF-150
  Year: 1987    Country/region: US    Model code: 3046
  Year: 1988    Country/region: EU    Model code: MK-3046-50
  Year: 1989    Country/region: BR    Model code: 011050

Notes:

  This emulated device is the version released by Sega. In Brazil,
  Tec Toy released a version that does not have any switch to turn
  on/off auto-repeat.

  Uses a separate oscillator consisting of a 27k resistor, a 1u
  capacitor, and two NOR gates from a 74HCTLS02 for each button.  In
  reality, the two oscillators will run at slightly different speeds
  and drift away from each other, so the buttons won't be
  synchronised.

  The rapid fire oscillators should start when the button is pressed,
  but this would require pushing TL and TR state from the controller
  to the port.

**********************************************************************/

#include "emu.h"
#include "rfu.h"

#include "controllers.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sms_rapid_fire_device : public device_t, public device_sms_control_interface
{
public:
	// construction/destruction
	sms_rapid_fire_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_sms_control_interface implementation
	virtual uint8_t in_r() override;
	virtual void out_w(uint8_t data, uint8_t mem_mask) override;

	void rapid_changed(int state);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<sms_control_port_device> m_subctrl_port;
	required_ioport m_rfire_sw;

	uint8_t m_in;
	uint8_t m_drive;
	uint8_t m_rapid;
};


INPUT_PORTS_START( sms_rapid_fire )
	PORT_START("rfu_sw")
	PORT_BIT( 0x00, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_TOGGLE PORT_NAME("Rapid Fire 1") PORT_WRITE_LINE_MEMBER(sms_rapid_fire_device, rapid_changed)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_TOGGLE PORT_NAME("Rapid Fire 2") PORT_WRITE_LINE_MEMBER(sms_rapid_fire_device, rapid_changed)
INPUT_PORTS_END


ioport_constructor sms_rapid_fire_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sms_rapid_fire);
}


//-------------------------------------------------
//  sms_rapid_fire_device - constructor
//-------------------------------------------------

sms_rapid_fire_device::sms_rapid_fire_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SMS_RAPID_FIRE, tag, owner, clock),
	device_sms_control_interface(mconfig, *this),
	m_subctrl_port(*this, "ctrl"),
	m_rfire_sw(*this, "rfu_sw"),
	m_in(0x03),
	m_drive(0x00),
	m_rapid(0x00)
{
}


void sms_rapid_fire_device::rapid_changed(int state)
{
	m_rapid = m_rfire_sw->read();
	m_subctrl_port->out_w(m_in | m_rapid, m_drive & ~m_rapid);
}


//-------------------------------------------------
//  sms_peripheral_r - rapid fire read
//-------------------------------------------------

uint8_t sms_rapid_fire_device::in_r()
{
	uint8_t const read_state = machine().time().as_ticks(10) & 1; // time interval not verified

	return m_subctrl_port->in_r() | (read_state ? m_rfire_sw->read() : 0U);
}


void sms_rapid_fire_device::out_w(uint8_t data, uint8_t mem_mask)
{
	// TR pulled up via a 4k7 resistor, connected directly to console pin when rapid fire disabled
	m_subctrl_port->out_w(data | m_rapid, mem_mask & ~m_rapid);
	m_in = data;
	m_drive = mem_mask;
}


void sms_rapid_fire_device::device_add_mconfig(machine_config &config)
{
	SMS_CONTROL_PORT(config, m_subctrl_port, sms_control_port_devices, SMS_CTRL_OPTION_JOYPAD);
	m_subctrl_port->th_handler().set(FUNC(sms_rapid_fire_device::th_w));
}


void sms_rapid_fire_device::device_config_complete()
{
	configure_screen(
			[this] (auto const &s)
			{ subdevice<sms_control_port_device>("ctrl")->set_screen(s); });
}


void sms_rapid_fire_device::device_start()
{
	save_item(NAME(m_in));
	save_item(NAME(m_drive));
	save_item(NAME(m_rapid));
}


void sms_rapid_fire_device::device_reset()
{
	m_rapid = m_rfire_sw->read();
}

} // anonymous namespace



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(SMS_RAPID_FIRE, device_sms_control_interface, sms_rapid_fire_device, "sms_rapid_fire", "Sega Master System Rapid Fire Unit")
