// license:GPL-2.0+
// copyright-holders:Felipe Sanches
#pragma once

#ifndef __PATINHOFEIO_H__
#define __PATINHOFEIO_H__

#define MCFG_PATINHO_RC_READ_CB(_devcb) \
	devcb = &patinho_feio_cpu_device::set_rc_read_callback(*device, DEVCB_##_devcb);

/* register IDs */
enum
{
	PATINHO_FEIO_CI=1, PATINHO_FEIO_ACC, PATINHO_FEIO_IDX, PATINHO_FEIO_RC
};

enum {
	DEVICE_BUSY=0,
	DEVICE_READY=1
};

class patinho_feio_peripheral
{
public:
	patinho_feio_peripheral()
	: io_status(DEVICE_READY)
	, device_is_ok(true)
	, IRQ_request(false)
	{ };

	int io_status;
	bool device_is_ok;
	bool IRQ_request;
};

class patinho_feio_cpu_device : public cpu_device
{
public:
	// construction/destruction
	patinho_feio_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, UINT32 _clock);

	template<class _Object> static devcb_base &set_rc_read_callback(device_t &device, _Object object) { return downcast<patinho_feio_cpu_device &>(device).m_rc_read_cb.set_callback(object); }

protected:

	virtual void execute_run() override;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	address_space_config m_program_config;

	/* processor registers */
	unsigned char m_acc; /* accumulator (8 bits) */
	unsigned int m_pc;         /* program counter (12 bits)
                                *  Actual register name is CI, which
                                *  stands for "Contador de Instrucao"
                                *  or "instructions counter".
                                */
	unsigned int m_rc; /* RC = "Registrador de Chaves" (Keys Register)
                        *       It represents the 12 bits of input data
                        *       from toggle switches in the computer panel
                        */
	unsigned char m_idx;

	/* processor state flip-flops */
	bool m_run;                /* processor is running */
	bool m_wait_for_interrupt;
	bool m_interrupts_enabled;
	bool m_scheduled_IND_bit_reset;
	bool m_indirect_addressing;

	int m_flags;
	// V = "Vai um" (Carry flag)
	// T = "Transbordo" (Overflow flag)

	patinho_feio_peripheral m_peripherals[16];

	int m_address_mask;        /* address mask */
	int m_icount;

	address_space *m_program;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 2; }

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : NULL; }

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 2; }

private:
	void execute_instruction();
	unsigned int compute_effective_address(unsigned int addr);
	UINT16 read_panel_keys_register();
	devcb_read16 m_rc_read_cb;
};


extern const device_type PATINHO_FEIO;

#endif /* __PATINHOFEIO_H__ */
