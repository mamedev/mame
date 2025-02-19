// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_VIDEO_SAA7110_H
#define MAME_VIDEO_SAA7110_H

#pragma once

#include "machine/i2chle.h"

class saa7110a_device :
	public device_t,
	public i2c_hle_interface,
	public device_memory_interface
{
public:
	saa7110a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
// saa7110a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;
	void regs_map(address_map &map) ATTR_COLD;
private:
	virtual u8 read_data(u16 offset) override;
	virtual void write_data(u16 offset, u8 data) override;

	address_space_config m_space_config;

	bool m_secs, m_sstb, m_hrmv, m_rtse, m_vtrc;
};

DECLARE_DEVICE_TYPE(SAA7110A, saa7110a_device)
//DECLARE_DEVICE_TYPE(SAA7110, saa7110_device)

#endif // MAME_VIDEO_SAA7110_H
