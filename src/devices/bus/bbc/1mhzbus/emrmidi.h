// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    EMR BBC MIDI Interface

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/EMR_BBCMIDI.html

**********************************************************************/


#ifndef MAME_BUS_BBC_1MHZBUS_EMRMIDI_H
#define MAME_BUS_BBC_1MHZBUS_EMRMIDI_H

#include "1mhzbus.h"
#include "machine/6850acia.h"
#include "machine/clock.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_emrmidi_device :
	public device_t,
	public device_bbc_1mhzbus_interface
{
public:
	// construction/destruction
	bbc_emrmidi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t fred_r(offs_t offset) override;
	virtual void fred_w(offs_t offset, uint8_t data) override;

private:
	void write_acia_clock(int state);

	required_device<acia6850_device> m_acia;
	required_device<clock_device> m_acia_clock;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_EMRMIDI, bbc_emrmidi_device);


#endif /* MAME_BUS_BBC_1MHZBUS_EMRMIDI_H */
