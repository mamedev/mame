#ifndef __NES_KARASTUDIO_H
#define __NES_KARASTUDIO_H

#include "machine/nes_nxrom.h"


// ======================> nes_karaokestudio_device

class nes_karaokestudio_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_karaokestudio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual ioport_constructor device_input_ports() const;
	virtual DECLARE_WRITE8_MEMBER(write_h);
	virtual DECLARE_READ8_MEMBER(read_m);

	virtual void pcb_reset();

	required_ioport m_mic_ipt;
};


// device type definition
extern const device_type NES_KARAOKESTUDIO;

#endif
