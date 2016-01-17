// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_CART_YAMAHA_H
#define __MSX_CART_YAMAHA_H

#include "bus/msx_cart/cartridge.h"
#include "sound/2151intf.h"
#include "bus/msx_cart/msx_audio_kb.h"
#include "machine/ym2148.h"


extern const device_type MSX_CART_SFG01;
extern const device_type MSX_CART_SFG05;


class msx_cart_sfg : public device_t
					, public msx_cart_interface
{
public:
	msx_cart_sfg(const machine_config &mconfig, const device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;

	virtual DECLARE_READ8_MEMBER(read_cart) override;
	virtual DECLARE_WRITE8_MEMBER(write_cart) override;

	DECLARE_WRITE_LINE_MEMBER(ym2151_irq_w);
	DECLARE_WRITE_LINE_MEMBER(ym2148_irq_w);

	IRQ_CALLBACK_MEMBER(irq_callback);

private:
	required_memory_region m_region_sfg;
	required_device<ym2151_device> m_ym2151;
	required_device<msx_audio_kbdc_port_device> m_kbdc;
	required_device<ym2148_device> m_ym2148;
	int m_ym2151_irq_state;
	int m_ym2148_irq_state;
	UINT32 m_rom_mask;

	void check_irq();
};


class msx_cart_sfg01 : public msx_cart_sfg
{
public:
	msx_cart_sfg01(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual const rom_entry *device_rom_region() const override;
};


class msx_cart_sfg05 : public msx_cart_sfg
{
public:
	msx_cart_sfg05(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual const rom_entry *device_rom_region() const override;
};

#endif
