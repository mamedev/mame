// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    This is the sound device described in and supported by the
    Integer BASIC music program "Apple Organ" from the Apple Software
    Bank (not to be confused with an Applesoft program of the same
    title). Made out of discrete components, it consists of a simple
    four-bit DAC with binary-weighted resistors attached to the
    annunciator outputs, plus a low pass filter capacitor.

*********************************************************************/

#include "emu.h"
#include "bus/a2gameio/organ.h"

#include "machine/rescap.h"
#include "sound/dac.h"
#include "sound/flt_rc.h"
#include "speaker.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> apple_organ_device

class apple_organ_device : public device_t, public device_a2gameio_interface
{
public:
	// construction/destruction
	apple_organ_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_a2gameio_interface overrides
	virtual void an0_w(int state) override;
	virtual void an1_w(int state) override;
	virtual void an2_w(int state) override;
	virtual void an3_w(int state) override;

private:
	// device finders
	required_device<dac_4bit_binary_weighted_device> m_dac;

	// internal state
	u8 m_output;
};


//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

void apple_organ_device::device_add_mconfig(machine_config &config)
{
	DAC_4BIT_BINARY_WEIGHTED(config, m_dac).add_route(0, "filter", 1.0);

	filter_rc_device &filter(FILTER_RC(config, "filter"));
	// "A high value capacitor works better, but attenuates the signal and needs more amplification from the audio system."
	filter.set_lowpass(RES_K(1), CAP_U(1));
	filter.add_route(0, "speaker", 1.0);

	// "Send the cable to an audio amplifier, or to the auxilliary input of your tape recorder."
	SPEAKER(config, "speaker").front_center();
}


//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

apple_organ_device::apple_organ_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, APPLE_ORGAN, tag, owner, clock)
	, device_a2gameio_interface(mconfig, *this)
	, m_dac(*this, "dac")
	, m_output(0)
{
}

void apple_organ_device::device_start()
{
	save_item(NAME(m_output));
}

void apple_organ_device::an0_w(int state)
{
	// AN0 connected to 5K resistor
	m_output = (m_output & 0b1110) | state;
	m_dac->write(m_output);
}

void apple_organ_device::an1_w(int state)
{
	// AN1 connected to 10K resistor
	m_output = (m_output & 0b1101) | (state << 1);
	m_dac->write(m_output);
}

void apple_organ_device::an2_w(int state)
{
	// AN2 connected to 20K resistor
	m_output = (m_output & 0b1011) | (state << 2);
	m_dac->write(m_output);
}

void apple_organ_device::an3_w(int state)
{
	// AN3 connected to 40K resistor
	m_output = (m_output & 0b0111) | (state << 3);
	m_dac->write(m_output);
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(APPLE_ORGAN, device_a2gameio_interface, apple_organ_device, "apple_organ", "Apple Organ")
