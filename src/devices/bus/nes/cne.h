// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_CNE_H
#define __NES_CNE_H

#include "nxrom.h"


// ======================> nes_cne_decathl_device

class nes_cne_decathl_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_cne_decathl_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset() override;
};


// ======================> nes_cne_fsb_device

class nes_cne_fsb_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_cne_fsb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(read_m);
	virtual DECLARE_WRITE8_MEMBER(write_m);

	virtual void pcb_reset() override;
};


// ======================> nes_cne_shlz_device

class nes_cne_shlz_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_cne_shlz_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_l);

	virtual void pcb_reset() override;
};





// device type definition
extern const device_type NES_CNE_DECATHL;
extern const device_type NES_CNE_FSB;
extern const device_type NES_CNE_SHLZ;

#endif
