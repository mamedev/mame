// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_FDC37M707_H
#define MAME_MACHINE_FDC37M707_H

#pragma once

#include "bus/isa/isa.h"
#include "machine/8042kbdc.h"

class fdc37m707_device : public device_t, 
                         public device_isa16_card_interface,
						 public device_memory_interface
{
public:
	fdc37m707_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~fdc37m707_device() {}

	void remap(int space_id, offs_t start, offs_t end) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual space_config_vector memory_space_config() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	const address_space_config m_space_config;

	required_device<kbdc8042_device> m_kbdc;
	memory_view m_logical_view;

	uint8_t read(offs_t offset);
	void write(offs_t offset, u8 data);

	void config_map(address_map &map);

	u8 m_index = 0;
	u8 m_logical_index = 0;
	bool m_activate[9]{};
	void logical_device_select_w(u8 data);
	template <unsigned N> u8 activate_r();
	template <unsigned N> void activate_w(u8 data);

	u8 keybc_status_r();
	void keybc_command_w(u8 data);
};

DECLARE_DEVICE_TYPE(FDC37M707, fdc37m707_device);

#endif // MAME_MACHINE_FDC37M707_H
