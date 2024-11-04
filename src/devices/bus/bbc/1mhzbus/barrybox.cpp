// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    The Barry-Box from BML Electronics

    Use with Barry-Box ROM.

**********************************************************************/

#include "emu.h"
#include "barrybox.h"

#include "sound/dac.h"
#include "speaker.h"


namespace {

class bbc_barrybox_device : public device_t, public device_bbc_1mhzbus_interface
{
public:
	static constexpr feature_type unemulated_features() { return feature::MICROPHONE; }

	bbc_barrybox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_BARRYBOX, tag, owner, clock)
		, device_bbc_1mhzbus_interface(mconfig, *this)
		, m_dac(*this,"dac")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t jim_r(offs_t offset) override;
	virtual void jim_w(offs_t offset, uint8_t data) override;

private:
	required_device<dac_byte_interface> m_dac;
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_barrybox_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker").front_center();
	ZN428E(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5);

	// TODO: ZN449E ADC
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_barrybox_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_barrybox_device::jim_r(offs_t offset)
{
	uint8_t data = 0x00;

	if (offset & 0x01)
	{
		// ADC busy?
	}

	if (offset & 0x04)
	{
		//data = m_adc->read();
	}

	return data;
}

void bbc_barrybox_device::jim_w(offs_t offset, uint8_t data)
{
	if (offset & 0x02)
	{
		m_dac->write(data);
	}
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_BARRYBOX, device_bbc_1mhzbus_interface, bbc_barrybox_device, "bbc_barrybox", "The Barry-Box");
