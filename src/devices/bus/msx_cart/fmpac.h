// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_CART_FMPAC_H
#define __MSX_CART_FMPAC_H

#include "bus/msx_cart/cartridge.h"
#include "sound/2413intf.h"


extern const device_type MSX_CART_FMPAC;


class msx_cart_fmpac : public device_t
						, public msx_cart_interface
{
public:
	msx_cart_fmpac(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual void initialize_cartridge() override;

	virtual DECLARE_READ8_MEMBER(read_cart) override;
	virtual DECLARE_WRITE8_MEMBER(write_cart) override;

	void restore_banks();

	DECLARE_WRITE8_MEMBER(write_ym2413);

private:
	required_device<ym2413_device> m_ym2413;

	UINT8 m_selected_bank;
	UINT8 *m_bank_base;
	bool m_sram_active;
	bool m_opll_active;
	UINT8 m_1ffe;
	UINT8 m_1fff;
	UINT8 m_7ff6;
};


#endif
