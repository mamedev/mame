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
	n2a03_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	required_device<nesapu_device> m_apu;

	static const disasm_entry disasm_entries[0x100];

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

	READ8_MEMBER(psg1_4014_r);
	READ8_MEMBER(psg1_4015_r);
	WRITE8_MEMBER(psg1_4015_w);
	WRITE8_MEMBER(psg1_4017_w);

protected:
	class mi_2a03_normal : public memory_interface {
	public:
		virtual ~mi_2a03_normal() {}
		virtual UINT8 read(UINT16 adr) override;
		virtual UINT8 read_sync(UINT16 adr) override;
		virtual UINT8 read_arg(UINT16 adr) override;
		virtual void write(UINT16 adr, UINT8 val) override;
	};

	class mi_2a03_nd : public memory_interface {
	public:
		virtual ~mi_2a03_nd() {}
		virtual UINT8 read(UINT16 adr) override;
		virtual UINT8 read_sync(UINT16 adr) override;
		virtual UINT8 read_arg(UINT16 adr) override;
		virtual void write(UINT16 adr, UINT8 val) override;
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

#define N2A03_DEFAULTCLOCK (21477272.724 / 12)

enum {
	N2A03_IRQ_LINE = m6502_device::IRQ_LINE,
	N2A03_APU_IRQ_LINE = m6502_device::APU_IRQ_LINE,
	N2A03_NMI_LINE = m6502_device::NMI_LINE,
	N2A03_SET_OVERFLOW = m6502_device::V_LINE
};

extern const device_type N2A03;

#endif
