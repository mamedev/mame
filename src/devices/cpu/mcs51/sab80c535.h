// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Manuel Abadia, Couriersud

#ifndef MAME_CPU_MCS51_SAB80C535_H
#define MAME_CPU_MCS51_SAB80C535_H

#include "i80c52.h"

class sab80c535_device : public i8052_device
{
public:
	// construction/destruction
	sab80c535_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void sfr_map(address_map &map) override ATTR_COLD;

private:
	u8 m_p4, m_p5;

	void p4_w(u8 data);
	u8 p4_r();
	void p5_w(u8 data);
	u8 p5_r();
};

DECLARE_DEVICE_TYPE(SAB80C535, sab80c535_device)

#endif
