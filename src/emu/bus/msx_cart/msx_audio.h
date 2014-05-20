#ifndef __MSX_CART_MSX_AUDIO_H
#define __MSX_CART_MSX_AUDIO_H

#include "bus/msx_cart/cartridge.h"
#include "sound/8950intf.h"
#include "machine/6850acia.h"


extern const device_type MSX_CART_MSX_AUDIO;


class msx_cart_msx_audio : public device_t
						, public msx_cart_interface
{
public:
	msx_cart_msx_audio(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual machine_config_constructor device_mconfig_additions() const;

	virtual void initialize_cartridge();

	virtual DECLARE_READ8_MEMBER(read_cart);

	DECLARE_WRITE8_MEMBER(write_y8950);
	DECLARE_READ8_MEMBER(read_y8950);

private:
	required_device<y8950_device> m_y8950;
	required_device<acia6850_device> m_acia6850;
};


#endif
