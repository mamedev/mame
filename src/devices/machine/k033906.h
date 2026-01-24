// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    Konami 033906

***************************************************************************/

#ifndef MAME_MACHINE_K033906_H
#define MAME_MACHINE_K033906_H

#pragma once

#include "video/voodoo.h"

class k033906_device :  public device_t
{
public:
	// construction/destruction
	template <typename T>
	k033906_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&voodoo_tag)
		: k033906_device(mconfig, tag, owner, clock)
	{
		m_voodoo.set_tag(std::forward<T>(voodoo_tag));
	}

	k033906_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_pciid(uint32_t pciid) { m_voodoo_pciid = pciid; }

	u32 read(offs_t offset);
	void write(offs_t offset, u32 data);
	void set_reg(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override { }
	virtual void device_post_load() override { }
	virtual void device_clock_changed() override { }

private:
	uint32_t reg_r(int reg);
	void reg_w(int reg, uint32_t data);

	/* i/o lines */

	int          m_reg_set; // 1 = access reg / 0 = access ram
	uint32_t     m_voodoo_pciid;

	required_device<generic_voodoo_device> m_voodoo;

	std::unique_ptr<u32[]> m_reg;
	std::unique_ptr<u32[]> m_ram;
};

DECLARE_DEVICE_TYPE(K033906, k033906_device)

#endif // MAME_MACHINE_K033906_H
