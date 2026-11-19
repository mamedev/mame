// license:BSD-3-Clause
// copyright-holders:Luca Elia, Olivier Galibert

/***************************************************************************

    Astro Corp. CPU Code (protection key inside the V102PX custom CPU)

***************************************************************************/

#ifndef MAME_ASTROCORP_ASTRO_CPUCODE_H
#define MAME_ASTROCORP_ASTRO_CPUCODE_H

#pragma once


class astro_cpucode_device : public device_t
{
public:
	astro_cpucode_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	int do_read();              // DO: current key bit, no side effects
	void next_bit();            // to be called on each CPU read of the DO line
	void cs_write(int state);   // CS (active high): restarts from the MSB

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_region_ptr<u16> m_region_key;

	u8 m_cs_state;
	u8 m_bit_index;             // next key bit to emit (0 = bit 15)
};

DECLARE_DEVICE_TYPE(ASTRO_CPUCODE, astro_cpucode_device)

#endif // MAME_ASTROCORP_ASTRO_CPUCODE_H
