// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_NTDEC_H
#define __NES_NTDEC_H

#include "nxrom.h"


// ======================> nes_ntdec_asder_device

class nes_ntdec_asder_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ntdec_asder_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();

private:
	UINT8 m_latch;
};


// ======================> nes_ntdec_fh_device

class nes_ntdec_fh_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ntdec_fh_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_m);

	virtual void pcb_reset();
};





// device type definition
extern const device_type NES_NTDEC_ASDER;
extern const device_type NES_NTDEC_FH;

#endif
