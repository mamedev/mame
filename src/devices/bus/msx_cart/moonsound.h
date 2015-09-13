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
	msx_cart_moonsound(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual const rom_entry *device_rom_region() const;

	DECLARE_WRITE8_MEMBER(write_ymf278b_fm);
	DECLARE_READ8_MEMBER(read_ymf278b_fm);
	DECLARE_WRITE8_MEMBER(write_ymf278b_pcm);
	DECLARE_READ8_MEMBER(read_ymf278b_pcm);
	DECLARE_READ8_MEMBER(read_c0);
	DECLARE_WRITE_LINE_MEMBER(irq_w);

private:
	required_device<ymf278b_device> m_ymf278b;
};


#endif
