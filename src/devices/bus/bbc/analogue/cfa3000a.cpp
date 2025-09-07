// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Henson CFA 3000 Analogue

**********************************************************************/

#include "emu.h"
#include "cfa3000a.h"


namespace {

class cfa3000_anlg_device : public device_t, public device_bbc_analogue_interface
{
public:
	cfa3000_anlg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, CFA3000_ANLG, tag, owner, clock),
		device_bbc_analogue_interface(mconfig, *this),
		m_channel(*this, "CHANNEL%u", 0),
		m_buttons(*this, "BUTTONS")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint16_t ch_r(offs_t channel) override
	{
		return m_channel[channel]->read() << 4;
	}

	virtual uint8_t pb_r() override
	{
		return m_buttons->read() & 0x30;
	}

private:
	required_ioport_array<4> m_channel;
	required_ioport m_buttons;
};


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( cfa3000a )
	PORT_START("CHANNEL0")
	PORT_BIT(0xfff, 0x800, IPT_PADDLE) PORT_NAME("Background Intensity") PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_CENTERDELTA(0)

	PORT_START("CHANNEL1")
	PORT_BIT(0xfff, 0xfff, IPT_UNKNOWN)

	PORT_START("CHANNEL2")
	PORT_BIT(0xfff, 0x800, IPT_PADDLE) PORT_NAME("Age") PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_CENTERDELTA(0)

	PORT_START("CHANNEL3")
	PORT_BIT(0xfff, 0xfff, IPT_UNKNOWN)

	PORT_START("BUTTONS")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN)
INPUT_PORTS_END

ioport_constructor cfa3000_anlg_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( cfa3000a );
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(CFA3000_ANLG, device_bbc_analogue_interface, cfa3000_anlg_device, "cfa3000a", "Henson CFA 3000 Analogue")
