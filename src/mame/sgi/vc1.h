// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_SGI_VC1_H
#define MAME_SGI_VC1_H

#pragma once

class sgi_vc1_device
	: public device_t
{
public:
	sgi_vc1_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u16 m_addr;
	u8 m_reg[128];
	u8 m_xmap[128];

	std::unique_ptr<u8[]> m_ram;
};

DECLARE_DEVICE_TYPE(SGI_VC1, sgi_vc1_device)

#endif // MAME_SGI_VC1_H
