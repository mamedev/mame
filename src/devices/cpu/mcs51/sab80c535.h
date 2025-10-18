// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Manuel Abadia, Couriersud

#ifndef MAME_CPU_MCS51_SAB80C535_H
#define MAME_CPU_MCS51_SAB80C535_H

#include "i80c52.h"

class sab80c535_device : public i80c52_device
{
public:
	// construction/destruction
	sab80c535_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

DECLARE_DEVICE_TYPE(SAB80C535, sab80c535_device)

#endif
