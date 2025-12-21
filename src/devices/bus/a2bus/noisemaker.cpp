// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    ADS (Ackerman Digital Systems) Noisemaker II

    This board provides a very simple interface to a AY-3-8910
    Programmable Sound Generator through two I/O addresses, and has
    a dedicated audio amplifier and speaker connector for the combined
    output of its three channels. The manual gives the option of using
    8T26 inverting buffers instead of 8T28 noninverting buffers, which
    changes the programming interface somewhat, though both schematics
    and actual boards have 8T28s.

    The PSG's two 8-bit parallel ports are not emulated here, though
    the actual PCB brings all 16 signals out to a generic DIP socket.

**********************************************************************/

#include "emu.h"
#include "noisemaker.h"

#include "sound/ay8910.h"
#include "speaker.h"


namespace {

class a2bus_noisemaker_device : public device_t, public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_noisemaker_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_a2bus_card_interface overrides
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual bool take_c800() override { return false; }

private:
	// object finders
	required_device<ay8910_device> m_psg;
};

a2bus_noisemaker_device::a2bus_noisemaker_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, A2BUS_NOISEMAKER, tag, owner, clock)
	, device_a2bus_card_interface(mconfig, *this)
	, m_psg(*this, "psg")
{
}

void a2bus_noisemaker_device::device_start()
{
}

u8 a2bus_noisemaker_device::read_c0nx(u8 offset)
{
	if (BIT(offset, 0))
		return m_psg->data_r();
	else
		return 0xff; // BDIR1 = BC1 = 0, so this should be open bus
}

void a2bus_noisemaker_device::write_c0nx(u8 offset, u8 data)
{
	m_psg->address_data_w(offset & 1, data);
}

void a2bus_noisemaker_device::device_add_mconfig(machine_config &config)
{
	AY8910(config, m_psg, 1021800); // CLK tied to ϕ1 signal from bus pin 38
	m_psg->add_route(ALL_OUTPUTS, "speaker", 0.5);

	SPEAKER(config, "speaker").front_center();
}

} // anonymous namespace


// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_NOISEMAKER, device_a2bus_card_interface, a2bus_noisemaker_device, "a2bus_noisemaker", "ADS Noisemaker II")
