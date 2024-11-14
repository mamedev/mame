// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2sam.c

    Implementation of the S.A.M. "Software Automated Mouth" card

*********************************************************************/

#include "emu.h"
#include "a2sam.h"

#include "sound/dac.h"

#include "speaker.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_sam_device:
		public device_t,
		public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_sam_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, A2BUS_SAM, tag, owner, clock),
		device_a2bus_card_interface(mconfig, *this),
		m_dac(*this, "dac")
	{
	}

protected:
	virtual void device_start() override { }
	virtual void device_reset() override { }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual void write_c0nx(uint8_t offset, uint8_t data) override { m_dac->write(data); }
	virtual bool take_c800() override { return false; }

	required_device<dac_byte_interface> m_dac;
};

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_sam_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_SAM, device_a2bus_card_interface, a2bus_sam_device, "a2sam", "Don't Ask Software S.A.M.")
