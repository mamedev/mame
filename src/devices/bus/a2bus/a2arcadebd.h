// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2arcadebd.h

    Third Millenium Engineering Arcade Board

*********************************************************************/

#ifndef __A2BUS_ARCADEBOARD__
#define __A2BUS_ARCADEBOARD__

#include "a2bus.h"
#include "video/tms9928a.h"
#include "sound/ay8910.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_arcboard_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_arcboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	a2bus_arcboard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	DECLARE_WRITE_LINE_MEMBER( tms_irq_w );

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(address_space &space, uint8_t offset) override;
	virtual void write_c0nx(address_space &space, uint8_t offset, uint8_t data) override;

	required_device<tms9918a_device> m_tms;
	required_device<ay8910_device> m_ay;
};

// device type definition
extern const device_type A2BUS_ARCADEBOARD;

#endif  /* __A2BUS_ARCADEBOARD__ */
