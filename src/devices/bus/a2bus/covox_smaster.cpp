// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Covox Sound Master

    A very small AY-3-8913 interface card with minimal support logic
    and no firmware included.

**********************************************************************/

#include "emu.h"
#include "covox_smaster.h"

#include "sound/ay8910.h"
#include "speaker.h"


namespace {

class a2bus_smaster_device : public device_t, public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_smaster_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_a2bus_card_interface implementation
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual void reset_from_bus() override;

private:
	// object finders
	required_device<ay8913_device> m_psg;
};

a2bus_smaster_device::a2bus_smaster_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, A2BUS_SMASTER, tag, owner, clock)
	, device_a2bus_card_interface(mconfig, *this)
	, m_psg(*this, "psg")
{
}

void a2bus_smaster_device::device_start()
{
}

u8 a2bus_smaster_device::read_c0nx(u8 offset)
{
	switch (offset & 3)
	{
	case 2:
		return m_psg->data_r();

	default:
		return get_open_bus();
	}
}

void a2bus_smaster_device::write_c0nx(u8 offset, u8 data)
{
	// A0 and A1 are inverted and gated onto BC1 and BDIR using a SN74LS02N
	if (!BIT(offset, 1))
		m_psg->address_data_w(offset & 1, data);
}

void a2bus_smaster_device::reset_from_bus()
{
	m_psg->reset();
}

void a2bus_smaster_device::device_add_mconfig(machine_config &config)
{
	AY8913(config, m_psg, A2BUS_1M_CLOCK); // clocked from bus pin 38
	m_psg->add_route(ALL_OUTPUTS, "speaker", 0.5);

	SPEAKER(config, "speaker").front_center(); // amplified through LM386N-1
}

} // anonymous namespace


// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_SMASTER, device_a2bus_card_interface, a2bus_smaster_device, "a2bus_smaster", "Covox Sound Master")
