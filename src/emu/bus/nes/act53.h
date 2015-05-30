// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_ACTION53_H
#define __NES_ACTION53_H

#include "nxrom.h"


// ======================> nes_racermate_device

class nes_action53_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_action53_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();

private:
	void update_prg();
	void update_mirr();
	UINT8 m_sel;
	UINT8 m_reg[4];
};





// device type definition
extern const device_type NES_ACTION53;

#endif
