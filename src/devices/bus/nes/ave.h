// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_AVE_H
#define __NES_AVE_H

#include "nxrom.h"


// ======================> nes_nina001_device

class nes_nina001_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_nina001_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_m) override;

	virtual void pcb_reset() override;
};


// ======================> nes_nina006_device

class nes_nina006_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_nina006_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_l) override;

	virtual void pcb_reset() override;
};


// ======================> nes_maxi15_device

class nes_maxi15_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_maxi15_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(read_h) override;

	virtual void pcb_reset() override;

private:
	void update_banks();
	UINT8 m_reg, m_bank;
};





// device type definition
extern const device_type NES_NINA001;
extern const device_type NES_NINA006;
extern const device_type NES_MAXI15;

#endif
