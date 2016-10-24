// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_CART_MOONSOUND_H
#define __MSX_CART_MOONSOUND_H

#include "bus/msx_cart/cartridge.h"
#include "sound/ymf278b.h"


extern const device_type MSX_CART_MOONSOUND;


class msx_cart_moonsound : public device_t
						, public msx_cart_interface
{
public:
	msx_cart_moonsound(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	void write_ymf278b_fm(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t read_ymf278b_fm(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write_ymf278b_pcm(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t read_ymf278b_pcm(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t read_c0(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void irq_w(int state);

private:
	required_device<ymf278b_device> m_ymf278b;
};


#endif
