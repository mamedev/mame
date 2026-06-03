// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    rp2a03.h

    6502, NES variant

***************************************************************************/
#ifndef MAME_CPU_M6502_RP2A03_H
#define MAME_CPU_M6502_RP2A03_H

#pragma once

#include "m6502.h"
#include "sound/nes_apu.h"

class rp2a03_core_device : public m6502_device {
public:
	rp2a03_core_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

protected:
	rp2a03_core_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

#define O(o) void o ## _full(); void o ## _partial()

	// rp2a03 opcodes - same as 6502 with D disabled
	O(adc_nd_aba); O(adc_nd_abx); O(adc_nd_aby); O(adc_nd_idx); O(adc_nd_idy); O(adc_nd_imm); O(adc_nd_zpg); O(adc_nd_zpx);
	O(arr_nd_imm);
	O(isb_nd_aba); O(isb_nd_abx); O(isb_nd_aby); O(isb_nd_idx); O(isb_nd_idy); O(isb_nd_zpg); O(isb_nd_zpx);
	O(rra_nd_aba); O(rra_nd_abx); O(rra_nd_aby); O(rra_nd_idx); O(rra_nd_idy); O(rra_nd_zpg); O(rra_nd_zpx);
	O(sbc_nd_aba); O(sbc_nd_abx); O(sbc_nd_aby); O(sbc_nd_idx); O(sbc_nd_idy); O(sbc_nd_imm); O(sbc_nd_zpg); O(sbc_nd_zpx);

#undef O

private:
};

class rp2a03_device : public rp2a03_core_device, public device_mixer_interface {
public:
	rp2a03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void rp2a03_map(address_map &map) ATTR_COLD;

protected:
	rp2a03_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	required_device<nesapu_device> m_apu;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void apu_irq(int state);
	uint8_t apu_read_mem(offs_t offset);
};

class rp2a03g_device : public rp2a03_device
{
public:
	rp2a03g_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

/* These are the official XTAL values and clock rates used by Nintendo for
   manufacturing throughout the production of the 2A03. PALC_APU_CLOCK is
   the clock rate devised by UMC(?) for PAL Famicom clone hardware.        */

#define RP2A03_NTSC_XTAL           XTAL(21'477'272)
#define RP2A03_PAL_XTAL            XTAL(26'601'712)
#define NTSC_APU_CLOCK      (RP2A03_NTSC_XTAL/12) /* 1.7897726666... MHz */
#define PAL_APU_CLOCK       (RP2A03_PAL_XTAL/16) /* 1.662607 MHz */
#define PALC_APU_CLOCK      (RP2A03_PAL_XTAL/15) /* 1.77344746666... MHz */

enum {
	RP2A03_IRQ_LINE = m6502_device::IRQ_LINE,
	RP2A03_APU_IRQ_LINE = m6502_device::APU_IRQ_LINE,
	RP2A03_NMI_LINE = m6502_device::NMI_LINE,
	RP2A03_SET_OVERFLOW = m6502_device::V_LINE
};

DECLARE_DEVICE_TYPE(RP2A03_CORE, rp2a03_core_device)
DECLARE_DEVICE_TYPE(RP2A03,      rp2a03_device)
DECLARE_DEVICE_TYPE(RP2A03G,     rp2a03g_device)

#endif // MAME_CPU_M6502_RP2A03_H
