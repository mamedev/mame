#ifndef __MSX_CART_YAMAHA_H
#define __MSX_CART_YAMAHA_H

#include "bus/msx_cart/cartridge.h"
#include "sound/2151intf.h"
#include "bus/msx_cart/msx_audio_kb.h"


extern const device_type MSX_CART_SFG01;
//extern const device_type MSX_CART_SFG05;


class msx_cart_sfg01 : public device_t
						, public msx_cart_interface
{
public:
	msx_cart_sfg01(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual const rom_entry *device_rom_region() const;
	virtual void device_start();

	virtual void initialize_cartridge();

	virtual DECLARE_READ8_MEMBER(read_cart);
	virtual DECLARE_WRITE8_MEMBER(write_cart);

	DECLARE_WRITE_LINE_MEMBER(ym2151_irq_w);
	DECLARE_WRITE_LINE_MEMBER(ym2148_irq_w);

private:
	required_memory_region m_region_sfg01;
	required_device<ym2151_device> m_ym2151;
	required_device<msx_audio_kbdc_port_device> m_kbdc;
	int m_ym2151_irq_state;
	int m_ym2148_irq_state;

	void check_irq();
};

#endif

