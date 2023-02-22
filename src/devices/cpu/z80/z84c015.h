// license:BSD-3-Clause
/***************************************************************************

    Zilog Z84C015, MPUZ80/Z8400/84C00 Family
    Z80 CPU, SIO, CTC, CGC, PIO, WDT

***************************************************************************/

#ifndef MAME_CPU_Z80_Z84C015_H
#define MAME_CPU_Z80_Z84C015_H

#pragma once

#include "tmpz84c015.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class z84c015_device : public tmpz84c015_device
{
public:
	z84c015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);

    // As z80 doesn't implements A lines, addr must be supplied
    int cs0_r(u16 addr);
    int cs1_r(u16 addr);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void internal_io_map(address_map &map) override;

private:
	// system control registers
	u8 m_scrp;
	u8 m_wcr;
	u8 m_mwbr;
	u8 m_csbr;
	u8 m_mcr;

	u8 scrp_r() { return m_scrp; };
	void scrp_w(u8 data) { m_scrp = data; };
	u8 scdp_r();
	void scdp_w(u8 data);
};

// device type definition
DECLARE_DEVICE_TYPE(Z84C015, z84c015_device)


#endif // MAME_CPU_Z80_Z84C015_H
