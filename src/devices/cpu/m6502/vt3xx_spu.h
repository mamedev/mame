// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    vt3xx_spu.h

***************************************************************************/
#ifndef MAME_CPU_M6502_VT3XX_SPU_H
#define MAME_CPU_M6502_VT3XX_SPU_H

#pragma once

#include "m6502.h"

class vt3xx_spu_device : public m6502_device {
public:
	vt3xx_spu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

#define O(o) void o ## _full(); void o ## _partial()

	O(phx_vt_imp);
	O(phy_vt_imp);
	O(plx_vt_imp);
	O(ply_vt_imp);

	O(vtsetdbk_imp);
	O(vtgetdbk_imp);

	O(vtldabank_abx);	
	O(vtldabank_idy);
	O(vtadcx_aba);

#undef O

protected:
	uint8_t m_databank;

	vt3xx_spu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_extdata_config;
	address_space *m_extdata_space;

protected:
	void set_databank(uint8_t bank);
	uint8_t get_databank();
};

enum {
	VT3XX_SPU_DATABANK = M6502_IR+1,
};


DECLARE_DEVICE_TYPE(VT3XX_SPU, vt3xx_spu_device)

#endif // MAME_CPU_M6502_VT3XX_SPU_H
