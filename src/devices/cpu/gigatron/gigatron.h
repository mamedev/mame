// license:BSD-2-Clause
// copyright-holders:Sterophonick, Phil Thomas
/*****************************************************************************
 *
 * Skeleton device for Gigatron CPU Core
 *
 *****************************************************************************/

 //https://github.com/PhilThomas/gigatron/blob/master/src/gigatron.js

#ifndef MAME_CPU_GTRON_H
#define MAME_CPU_GTRON_H

#pragma once

enum
{
	GTRON_PC, GTRON_NPC,
	GTRON_AC, GTRON_X, GTRON_Y
};


class gigatron_cpu_device :  public cpu_device
{
public:
	// construction/destruction
	gigatron_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 7; }
	virtual uint32_t execute_input_lines() const noexcept override { return 0; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	void branchOp(uint8_t op, uint8_t mode, uint8_t bus, uint8_t d);
	void aluOp(uint8_t op, uint8_t mode, uint8_t bus, uint8_t d);
	void storeOp(uint8_t op, uint8_t mode, uint8_t bus, uint8_t d);
	uint8_t offset(uint8_t bus, uint8_t d);
	uint16_t addr(uint8_t mode, uint8_t d);

	uint8_t m_ac;
	uint8_t m_x;
	uint8_t m_y;
	uint16_t m_npc;
	uint16_t m_ppc;
	uint8_t m_inReg;
	uint16_t m_ramMask;
	uint16_t m_out;
	uint16_t m_outx;

	virtual void init();

private:
	address_space_config m_program_config;
	address_space_config m_data_config;

	uint16_t  m_pc;   /* registers */
	uint8_t   m_flags;  /* flags */
	address_space *m_program;
	address_space *m_data;
	int m_icount;

	void gigatron_illegal();
};


DECLARE_DEVICE_TYPE(GTRON, gigatron_cpu_device)



#endif // MAME_CPU_GTRON_H

