// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_CART_BM_012_H
#define __MSX_CART_BM_012_H

#include "bus/msx_cart/cartridge.h"
#include "cpu/z80/tmpz84c015.h"
#include "bus/midi/midi.h"


extern const device_type MSX_CART_BM_012;


class msx_cart_bm_012 : public device_t
					, public msx_cart_interface
{
public:
	msx_cart_bm_012(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const rom_entry *device_rom_region() const override;
	virtual void device_start() override;

	DECLARE_WRITE_LINE_MEMBER(midi_in);

private:
	required_device<tmpz84c015_device> m_tmpz84c015af;
	required_device<z80pio_device> m_bm012_pio;
	required_device<midi_port_device> m_mdthru;
};


#endif
