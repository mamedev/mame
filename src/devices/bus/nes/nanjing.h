// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_NANJING_H
#define __NES_NANJING_H

#include "nxrom.h"


// ======================> nes_nanjing_device

class nes_nanjing_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_nanjing_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(read_l) override;
	virtual DECLARE_WRITE8_MEMBER(write_l) override;

	virtual void hblank_irq(int scanline, int vblank, int blanked) override;
	virtual void pcb_reset() override;

private:
	UINT8 m_count;
	UINT8 m_reg[2];
	UINT8 m_latch1, m_latch2;
};





// device type definition
extern const device_type NES_NANJING;

#endif
