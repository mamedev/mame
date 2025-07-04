// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    The Barry-Box from BML Electronics

    Use with Barry-Box ROM.

**********************************************************************/

#include "emu.h"
#include "barrybox.h"

#include "sound/adc.h"
#include "sound/dac.h"

#include "speaker.h"


namespace {

class bbc_barrybox_device : public device_t, public device_bbc_1mhzbus_interface
{
public:
	bbc_barrybox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_BARRYBOX, tag, owner, clock)
		, device_bbc_1mhzbus_interface(mconfig, *this)
		, m_adc(*this, "adc")
		, m_dac(*this,"dac")
		, m_mic(*this, "mic")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD
	{
		SPEAKER(config, "speaker").front_center();

		ZN428E(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // audio can optionally be routed via 1MHz bus

		MICROPHONE(config, m_mic, 1).front_center();
		m_mic->add_route(0, m_adc, 1.0);

		ZN449(config, m_adc);
	}

	virtual uint8_t jim_r(offs_t offset) override
	{
		uint8_t data = 0x00;

		if (offset & 0x01)
		{
			// ADC busy?
		}

		if (offset & 0x04)
		{
			data = m_adc->read();
		}

		return data;
	}

	virtual void jim_w(offs_t offset, uint8_t data) override
	{
		if (offset & 0x02)
		{
			m_dac->write(data);
		}
	}

private:
	required_device<zn449_device> m_adc;
	required_device<zn428e_device> m_dac;
	required_device<microphone_device> m_mic;
};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_BARRYBOX, device_bbc_1mhzbus_interface, bbc_barrybox_device, "bbc_barrybox", "The Barry-Box")
