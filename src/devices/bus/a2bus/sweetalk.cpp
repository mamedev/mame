// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    Sweet Talker Phonetic Speech Synthesizer Interface
    (The Micromint Inc.)

**********************************************************************/

#include "emu.h"
#include "sweetalk.h"

#include "sound/votrax.h"
#include "speaker.h"

namespace {

class sweetalk_device : public device_t, public device_a2bus_card_interface
{
public:
	// device type constructor
	sweetalk_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_a2bus_card_interface implementation
	virtual u8 read_cnxx(u8 offset) override;
	virtual void write_cnxx(u8 offset, u8 data) override;

private:
	// object finders
	required_device<votrax_sc01_device> m_sc01;
};

sweetalk_device::sweetalk_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, A2BUS_SWEETALK, tag, owner, clock)
	, device_a2bus_card_interface(mconfig, *this)
	, m_sc01(*this, "sc01")
{
}

void sweetalk_device::device_start()
{
}

u8 sweetalk_device::read_cnxx(u8 offset)
{
	// lower 7 bits are actually open bus
	return m_sc01->request() ? 0x80 : 0;
}

void sweetalk_device::write_cnxx(u8 offset, u8 data)
{
	m_sc01->write(data);
	m_sc01->inflection_w(~data >> 6);
}

void sweetalk_device::device_add_mconfig(machine_config &config)
{
	// "The frequency is adjusted through potentiometer R8 and nominally set for 720 KHz." (owner's manual, p. 5)
	VOTRAX_SC01A(config, m_sc01, 720'000);
	m_sc01->add_route(ALL_OUTPUTS, "speaker", 0.8);

	SPEAKER(config, "speaker").front_center();
}

} // anonymous namespace

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_SWEETALK, device_a2bus_card_interface, sweetalk_device, "sweetalk", "Micromint Sweet Talker Phonetic Speech Synthesizer Interface")
