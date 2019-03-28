// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_ACT53_H
#define MAME_BUS_NES_ACT53_H

#include "nxrom.h"


// ======================> nes_action53_device

class nes_action53_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_action53_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void update_prg();
	void update_mirr();
	uint8_t m_sel;
	uint8_t m_reg[4];
};

// device type definition
DECLARE_DEVICE_TYPE(NES_ACTION53, nes_action53_device)

#endif // MAME_BUS_NES_ACT53_H
