// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Tube Podule

    TODO:
    - attach BBC Micro as Tube Host.

**********************************************************************/

#include "emu.h"
#include "tube.h"
#include "machine/tube.h"


namespace {

class arc_tube_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_tube_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;
};


void arc_tube_device::ioc_map(address_map &map)
{
	map(0x0000, 0x3fff).rw("tube", FUNC(tube_device::parasite_r), FUNC(tube_device::parasite_w)).umask32(0x000000ff);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_tube_device::device_add_mconfig(machine_config &config)
{
	tube_device &tube(TUBE(config, "tube"));
	tube.pnmi_handler().set([this](int state) { set_pfiq(state); });
	tube.pirq_handler().set([this](int state) { set_pirq(state); });
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_tube_device - constructor
//-------------------------------------------------

arc_tube_device::arc_tube_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_TUBE, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_tube_device::device_start()
{
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_TUBE, device_archimedes_podule_interface, arc_tube_device, "arc_tube", "Acorn Tube Podule")
