// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    r65c02.h

    Rockwell 65c02, CMOS variant with bitwise instructions

***************************************************************************/

#ifndef MAME_CPU_M6502_R65C02_H
#define MAME_CPU_M6502_R65C02_H

#include "w65c02.h"

class r65c02_device : public w65c02_device {
public:
	r65c02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

protected:
	r65c02_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

class r65c102_device : public r65c02_device {
public:
	r65c102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 4 - 1) / 4; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 4); }
};

class r65c112_device : public r65c02_device {
public:
	r65c112_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

enum {
	R65C02_IRQ_LINE = m6502_device::IRQ_LINE,
	R65C02_NMI_LINE = m6502_device::NMI_LINE,
	R65C02_SET_OVERFLOW = m6502_device::V_LINE
};

DECLARE_DEVICE_TYPE(R65C02, r65c02_device)
DECLARE_DEVICE_TYPE(R65C102, r65c102_device)
DECLARE_DEVICE_TYPE(R65C112, r65c112_device)

#endif // MAME_CPU_M6502_R65C02_H
