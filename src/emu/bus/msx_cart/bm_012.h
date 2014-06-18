#ifndef __MSX_CART_BM_012_H
#define __MSX_CART_BM_012_H

#include "bus/msx_cart/cartridge.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "bus/midi/midi.h"


extern const device_type MSX_CART_BM_012;


class msx_cart_bm_012 : public device_t
					, public msx_cart_interface
{
public:
	msx_cart_bm_012(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual const rom_entry *device_rom_region() const;
	virtual void device_start();
	virtual void device_reset();

	DECLARE_WRITE8_MEMBER(tmpz84c015af_f4_w);
	DECLARE_WRITE_LINE_MEMBER(midi_in);

private:
	// TMPZ84C015AF related
	required_device<z80pio_device> m_tmpz84c015af_pio;
	required_device<z80ctc_device> m_tmpz84c015af_ctc;
	required_device<z80dart_device> m_tmpz84c015af_sio;
	UINT8 m_irq_priority;

	required_device<z80pio_device> m_bm012_pio;
	required_device<midi_port_device> m_mdthru;
};


#endif
