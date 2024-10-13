// license:BSD-3-Clause
/***************************************************************************

    Zilog Z84C015, MPUZ80/Z8400/84C00 Family
    Z80 CPU, SIO, CTC, CGC, PIO, WDT

***************************************************************************/

#ifndef MAME_CPU_Z80_Z84C015_H
#define MAME_CPU_Z80_Z84C015_H

#pragma once

#include "tmpz84c015.h"


enum
{
	Z84_WCR = Z80_WZ + 1, Z84_MWBR, Z84_CSBR, Z84_MCR
};

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class z84c015_device : public tmpz84c015_device
{
public:
	z84c015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u8 csbr_r() { return m_csbr; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	const address_space_config m_program_space_config;
	const address_space_config m_opcodes_space_config;

	void internal_io_map(address_map &map) const;
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;
	virtual u32 translate_memory_address(u16 address) override;

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
