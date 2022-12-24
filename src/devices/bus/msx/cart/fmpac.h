// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_FMPAC_H
#define MAME_BUS_MSX_CART_FMPAC_H

#pragma once

#include "bus/msx/slot/cartridge.h"
#include "sound/ymopl.h"


DECLARE_DEVICE_TYPE(MSX_CART_FMPAC, msx_cart_fmpac_device)


class msx_cart_fmpac_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_fmpac_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;

private:
	void write_ym2413(offs_t offset, u8 data);
	void sram_unlock(offs_t offset, u8 data);
	u8 control_r();
	void control_w(u8 data);
	u8 bank_r();
	void bank_w(u8 data);

	required_device<ym2413_device> m_ym2413;
	memory_bank_creator m_rombank;
	memory_view m_view;

	bool m_sram_active;
	bool m_opll_active;
	u8 m_sram_unlock[2];
	u8 m_control;
};


#endif // MAME_BUS_MSX_CART_FMPAC_H
