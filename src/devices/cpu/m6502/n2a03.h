// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    n2a03.h

    6502, NES variant

***************************************************************************/

#ifndef __N2A03_H__
#define __N2A03_H__

#include "m6502.h"
#include "sound/nes_apu.h"

class n2a03_device : public m6502_device {
public:
	n2a03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	required_device<nesapu_device> m_apu;

	static const disasm_entry disasm_entries[0x100];

	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;
	virtual void device_clock_changed() override;

	READ8_MEMBER(psg1_4014_r);
	READ8_MEMBER(psg1_4015_r);
	WRITE8_MEMBER(psg1_4015_w);
	WRITE8_MEMBER(psg1_4017_w);

protected:
	class mi_2a03_normal : public memory_interface {
	public:
		virtual ~mi_2a03_normal() {}
		virtual uint8_t read(uint16_t adr) override;
		virtual uint8_t read_sync(uint16_t adr) override;
		virtual uint8_t read_arg(uint16_t adr) override;
		virtual void write(uint16_t adr, uint8_t val) override;
	};

	class mi_2a03_nd : public memory_interface {
	public:
		virtual ~mi_2a03_nd() {}
		virtual uint8_t read(uint16_t adr) override;
		virtual uint8_t read_sync(uint16_t adr) override;
		virtual uint8_t read_arg(uint16_t adr) override;
		virtual void write(uint16_t adr, uint8_t val) override;
	};

	virtual void device_start() override;

#define O(o) void o ## _full(); void o ## _partial()

	// n2a03 opcodes - same as 6502 with D disabled
	O(adc_nd_aba); O(adc_nd_abx); O(adc_nd_aby); O(adc_nd_idx); O(adc_nd_idy); O(adc_nd_imm); O(adc_nd_zpg); O(adc_nd_zpx);
	O(arr_nd_imm);
	O(isb_nd_aba); O(isb_nd_abx); O(isb_nd_aby); O(isb_nd_idx); O(isb_nd_idy); O(isb_nd_zpg); O(isb_nd_zpx);
	O(rra_nd_aba); O(rra_nd_abx); O(rra_nd_aby); O(rra_nd_idx); O(rra_nd_idy); O(rra_nd_zpg); O(rra_nd_zpx);
	O(sbc_nd_aba); O(sbc_nd_abx); O(sbc_nd_aby); O(sbc_nd_idx); O(sbc_nd_idy); O(sbc_nd_imm); O(sbc_nd_zpg); O(sbc_nd_zpx);

#undef O

	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

private:
	address_space_config m_program_config;

};

/* These are the official XTAL values and clock rates used by Nintendo for
   manufacturing throughout the production of the 2A03. PALC_APU_CLOCK is
   the clock rate devised by UMC(?) for PAL Famicom clone hardware.        */   

#define N2A03_NTSC_XTAL           XTAL_21_4772MHz
#define N2A03_PAL_XTAL            XTAL_26_601712MHz
#define NTSC_APU_CLOCK      (N2A03_NTSC_XTAL/12) /* 1.7897726666... MHz */
#define PAL_APU_CLOCK       (N2A03_PAL_XTAL/16) /* 1.662607 MHz */
#define PALC_APU_CLOCK      (N2A03_PAL_XTAL/15) /* 1.77344746666... MHz */

enum {
	N2A03_IRQ_LINE = m6502_device::IRQ_LINE,
	N2A03_APU_IRQ_LINE = m6502_device::APU_IRQ_LINE,
	N2A03_NMI_LINE = m6502_device::NMI_LINE,
	N2A03_SET_OVERFLOW = m6502_device::V_LINE
};

extern const device_type N2A03;

#endif
