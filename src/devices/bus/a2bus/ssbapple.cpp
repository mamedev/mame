// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    ssbapple.c

    Implementation of the SSB Apple speech card
    Must be in slot 2 for the provided software to work!

*********************************************************************/

#include "emu.h"
#include "ssbapple.h"

#include "sound/tms5220.h"

#include "speaker.h"


namespace {

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define TMS_TAG         "tms5220"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_ssb_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_ssb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	required_device<tms5220_device> m_tms;

protected:
	a2bus_ssb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;
	virtual bool take_c800() override;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_ssb_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "ssbapple").front_center();
	TMS5220(config, m_tms, 640000); // guess - this gives 8 kHz output according to the datasheet
	m_tms->add_route(ALL_OUTPUTS, "ssbapple", 1.0);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_ssb_device::a2bus_ssb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_tms(*this, TMS_TAG)
{
}

a2bus_ssb_device::a2bus_ssb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_ssb_device(mconfig, A2BUS_SSBAPPLE, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_ssb_device::device_start()
{
	if (slotno() != 2)
		popmessage("SSB Card should be in slot 2!\n");
}

void a2bus_ssb_device::device_reset()
{
}

bool a2bus_ssb_device::take_c800()
{
	return false;
}

uint8_t a2bus_ssb_device::read_cnxx(uint8_t offset)
{
	return 0x1f | m_tms->status_r();
}

void a2bus_ssb_device::write_cnxx(uint8_t offset, uint8_t data)
{
	m_tms->data_w(data);
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_SSBAPPLE, device_a2bus_card_interface, a2bus_ssb_device, "a2ssbapl", "Multitech Industrial SSB Apple speech card")
