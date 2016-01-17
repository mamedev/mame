// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_2A03PUR_H
#define __NES_2A03PUR_H

#include "nxrom.h"


// ======================> nes_racermate_device

class nes_2a03pur_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_2a03pur_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(read_h) override;
	virtual DECLARE_WRITE8_MEMBER(write_l) override;

	virtual void pcb_reset() override;

private:
	UINT8 m_reg[8];
};



// device type definition
extern const device_type NES_2A03PURITANS;

#endif
