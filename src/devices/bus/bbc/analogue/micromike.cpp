// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Micro Mike by Magpie Systems

**********************************************************************/

#include "emu.h"
#include "micromike.h"

#include "speaker.h"


namespace {

class bbc_micromike_device
	: public device_t
	, public device_sound_interface
	, public device_bbc_analogue_interface
{
public:
	bbc_micromike_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_MICROMIKE, tag, owner, clock)
		, device_sound_interface(mconfig, *this)
		, device_bbc_analogue_interface(mconfig, *this)
		, m_mic(*this, "mic")
		, m_buttons(*this, "BUTTONS")
		, m_stream(nullptr)
		, m_current_value(0)
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint16_t ch_r(offs_t channel) override
	{
		switch (channel)
		{
		case 0:
			m_stream->update();
			return m_current_value << 4;

		default:
			return 0x00;
		}
	}

	virtual uint8_t pb_r() override
	{
		return m_buttons->read() & 0x30;
	}

private:
	required_device<microphone_device> m_mic;
	required_ioport m_buttons;
	sound_stream *m_stream;
	uint16_t m_current_value;
};


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( micromike )
	PORT_START("BUTTONS")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Button")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor bbc_micromike_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( micromike );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_micromike_device::device_add_mconfig(machine_config &config)
{
	MICROPHONE(config, m_mic, 1).front_center();
	m_mic->add_route(0, DEVICE_SELF, 1.0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_micromike_device::device_start()
{
	m_stream = stream_alloc(1, 0, SAMPLE_RATE_INPUT_ADAPTIVE);

	save_item(NAME(m_current_value));
}


void bbc_micromike_device::sound_stream_update(sound_stream &stream)
{
	sound_stream::sample_t last_sample = stream.get(0, stream.samples()-1);
	m_current_value = std::abs(last_sample) * 0xfff;
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_MICROMIKE, device_bbc_analogue_interface, bbc_micromike_device, "micromike", "Micro Mike")
