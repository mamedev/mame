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
	nes_action53_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
	virtual void pcb_start(running_machine &machine, u8 *ciram_ptr, bool cart_mounted) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void update_prg();
	void update_mirr();
	u8 m_sel;
	u8 m_reg[4];
};

// device type definition
DECLARE_DEVICE_TYPE(NES_ACTION53, nes_action53_device)

#endif // MAME_BUS_NES_ACT53_H
