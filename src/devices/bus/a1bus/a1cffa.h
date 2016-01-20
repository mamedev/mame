// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a1cffa.h

    Rich Dreher's Compact Flash for Apple I

*********************************************************************/

#ifndef __A1BUS_CFFA__
#define __A1BUS_CFFA__

#include "emu.h"
#include "a1bus.h"
#include "machine/ataintf.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a1bus_cffa_device:
	public device_t,
	public device_a1bus_card_interface
{
public:
	// construction/destruction
	a1bus_cffa_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	a1bus_cffa_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const rom_entry *device_rom_region() const override;

	required_device<ata_interface_device> m_ata;

	DECLARE_READ8_MEMBER(cffa_r);
	DECLARE_WRITE8_MEMBER(cffa_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	UINT8 *m_rom;
	UINT16 m_lastdata;
	bool m_writeprotect;
};

// device type definition
extern const device_type A1BUS_CFFA;

#endif  /* __A1BUS_CFFA__ */
