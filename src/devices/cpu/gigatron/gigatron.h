// license:BSD-3-Clause
// copyright-holders:Sterophonick
/*****************************************************************************
 *
 * Skeleton Device for Gigatron CPU Core
 *
 *****************************************************************************/

#ifndef MAME_CPU_GTRON_H
#define MAME_CPU_GTRON_H

#pragma once

enum
{
	GTRON_R0=1, GTRON_R1, GTRON_R2, GTRON_R3,
	GTRON_R4, GTRON_R5, GTRON_R6, GTRON_R7
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


private:
	address_space_config m_program_config;

	uint8_t   m_pc;   /* registers */
	uint8_t   m_flags;  /* flags */
	address_space *m_program;
	address_space *m_data;
	int m_icount;

	void gigatron_illegal();

};


DECLARE_DEVICE_TYPE(GTRON, gigatron_cpu_device)



#endif // MAME_CPU_GTRON_H

