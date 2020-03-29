// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_NES_VT_H
#define MAME_MACHINE_NES_VT_H

#pragma once

class nes_vt_soc_device : public device_t, public device_memory_interface
{
public:
	nes_vt_soc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void program_map(address_map &map);
protected:
	virtual void device_start() override;
	virtual space_config_vector memory_space_config() const override;

private:
	const address_space_config m_program_space_config;
};

DECLARE_DEVICE_TYPE(NES_VT_SOC, nes_vt_soc_device)

#endif // MAME_MACHINE_NES_VT_H
