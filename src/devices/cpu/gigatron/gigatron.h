// license:BSD-3-Clause
// copyright-holders:<author_name>
/*****************************************************************************
 *
 * Skeleton Device for Gigatron CPU Core
 *
 *****************************************************************************/

#ifndef MAME_CPU_GIGATRON_GIGATRON_H
#define MAME_CPU_GIGATRON_GIGATRON_H

#pragma once

enum
{
	#if UNUSED
	GTRON_AC=0,GTRON_X,GTRON_Y
	#endif
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
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_disasm_interface overrides
	virtual uint32_t opcode_alignment() const override { return 4; }

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


#endif // MAME_CPU_GIGATRON_GIGATRON_H
