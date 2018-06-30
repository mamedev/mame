// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    r65c02.h

    Rockwell 65c02, CMOS variant with bitwise instructions

***************************************************************************/

#ifndef MAME_CPU_M6502_R65C02_H
#define MAME_CPU_M6502_R65C02_H

#include "m65c02.h"

class r65c02_device : public m65c02_device {
public:
	r65c02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

protected:
	r65c02_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

enum {
	R65C02_IRQ_LINE = m6502_device::IRQ_LINE,
	R65C02_NMI_LINE = m6502_device::NMI_LINE,
	R65C02_SET_OVERFLOW = m6502_device::V_LINE
};

DECLARE_DEVICE_TYPE(R65C02, r65c02_device)

#endif // MAME_CPU_M6502_R65C02_H
