// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2dx1.c

    Implementation of the Decillionix DX-1 sampler card

*********************************************************************/

#include "emu.h"
#include "a2dx1.h"

#include "sound/dac.h"

#include "speaker.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_dx1_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_dx1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_dx1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual bool take_c800() override { return false; }

	required_device<dac_byte_interface> m_dac;
	required_device<dac_byte_interface> m_dacvol;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_dx1_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC
	DAC_8BIT_R2R(config, m_dacvol, 0)
		.set_output_range(0, 1)
		.add_route(0, "dac", 1.0, DAC_INPUT_RANGE_HI)
		.add_route(0, "dac", -1.0, DAC_INPUT_RANGE_LO); // unknown DAC
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_dx1_device::a2bus_dx1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_dac(*this, "dac"),
	m_dacvol(*this, "dacvol")
{
}

a2bus_dx1_device::a2bus_dx1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_dx1_device(mconfig, A2BUS_DX1, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_dx1_device::device_start()
{
}

uint8_t a2bus_dx1_device::read_c0nx(uint8_t offset)
{
	switch (offset)
	{
		case 1: // ADC input
			return 0;

		case 3: // busy flag
			return 0x80;    // indicate not busy

		case 7: // 1-bit ADC input (bit 7 of c0n1, probably)
			return 0;
	}

	return 0xff;
}

void a2bus_dx1_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 5:
			m_dacvol->write(data);
			break;

		case 6:
			m_dac->write(data);
			break;
	}
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_DX1, device_a2bus_card_interface, a2bus_dx1_device, "a2dx1", "Decillonix DX-1")
