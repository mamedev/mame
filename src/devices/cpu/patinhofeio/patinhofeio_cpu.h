// license:GPL-2.0+
// copyright-holders:Felipe Sanches
#pragma once

#ifndef __PATINHOFEIO_H__
#define __PATINHOFEIO_H__

#define MCFG_PATINHO_RC_READ_CB(_devcb) \
	devcb = &patinho_feio_cpu_device::set_rc_read_callback(*device, DEVCB_##_devcb);
#define MCFG_PATINHO_BUTTONS_READ_CB(_devcb) \
	devcb = &patinho_feio_cpu_device::set_buttons_read_callback(*device, DEVCB_##_devcb);
#define MCFG_PATINHO_IODEV_READ_CB(devnumber, _devcb) \
	devcb = &patinho_feio_cpu_device::set_iodev_read_callback(*device, devnumber, DEVCB_##_devcb);
#define MCFG_PATINHO_IODEV_WRITE_CB(devnumber, _devcb) \
	devcb = &patinho_feio_cpu_device::set_iodev_write_callback(*device, devnumber, DEVCB_##_devcb);

/* register IDs */
enum
{
	PATINHO_FEIO_CI=1, PATINHO_FEIO_ACC, PATINHO_FEIO_EXT, PATINHO_FEIO_IDX, PATINHO_FEIO_RC
};

enum
{
	NORMAL_MODE,
	CYCLE_STEP_MODE,
	INSTRUCTION_STEP_MODE,
	ADDRESSING_MODE,
	DATA_STORE_MODE,
	DATA_VIEW_MODE
};

#define IODEV_READY true
#define IODEV_BUSY false
#define REQUEST true
#define NO_REQUEST false

#define BUTTON_NORMAL                (1 << 0)  /* normal CPU execution */
#define BUTTON_CICLO_UNICO           (1 << 1)  /* single-cycle step */
#define BUTTON_INSTRUCAO_UNICA       (1 << 2)  /* single-instruction step */
#define BUTTON_ENDERECAMENTO         (1 << 3)  /* addressing action */
#define BUTTON_ARMAZENAMENTO         (1 << 4)  /* storage action */
#define BUTTON_EXPOSICAO             (1 << 5)  /* memory viewing action */
#define BUTTON_ESPERA                (1 << 6)  /* wait */
#define BUTTON_INTERRUPCAO           (1 << 7)  /* interrupt */
#define BUTTON_PARTIDA               (1 << 8)  /* startup */
#define BUTTON_PREPARACAO            (1 << 9)  /* reset */
#define BUTTON_TIPO_DE_ENDERECAMENTO (1 << 10) /* Addressing mode (0: Fixed / 1: Sequential) */
#define BUTTON_PROTECAO_DE_MEMORIA   (1 << 11) /* Memory protection (in the address range 0xF80-0xFFF (1: write-only / 0: read-write) */

class patinho_feio_cpu_device : public cpu_device {
public:
	// construction/destruction
	patinho_feio_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, UINT32 _clock);

	template<class _Object> static devcb_base &set_rc_read_callback(device_t &device, _Object object) { return downcast<patinho_feio_cpu_device &>(device).m_rc_read_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_buttons_read_callback(device_t &device, _Object object) { return downcast<patinho_feio_cpu_device &>(device).m_buttons_read_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_iodev_read_callback(device_t &device, int devnumber, _Object object) { return downcast<patinho_feio_cpu_device &>(device).m_iodev_read_cb[devnumber].set_callback(object); }
	template<class _Object> static devcb_base &set_iodev_write_callback(device_t &device, int devnumber, _Object object) { return downcast<patinho_feio_cpu_device &>(device).m_iodev_write_cb[devnumber].set_callback(object); }
	template<class _Object> static devcb_base &set_iodev_status_callback(device_t &device, int devnumber, _Object object) { return downcast<patinho_feio_cpu_device &>(device).m_iodev_status_cb[devnumber].set_callback(object); }

	void transfer_byte_from_external_device(UINT8 channel, UINT8 data);
	void set_iodev_status(UINT8 channel, bool status) {
		m_iodev_status[channel] = status;
	}
protected:

	virtual void execute_run() override;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	address_space_config m_program_config;

	offs_t m_addr;
	unsigned char m_opcode;

	/* processor registers */
	unsigned char m_acc; /* accumulator (8 bits) */
	unsigned int m_pc;   /* program counter (12 bits)
                          * Actual register name is CI, which
                          * stands for "Contador de Instrucao"
                          * or "instructions counter".
                          */
	unsigned int m_rc; /* RC = "Registrador de Chaves" (Keys Register)
                        * It represents the 12 bits of input data
                        * from toggle switches in the computer panel
                        */
	unsigned char m_idx; /* IDX = Index Register */
	unsigned char m_ext; /* EXT = Accumulator Extension Register */

	/* processor state flip-flops */
	bool m_run; /* processor is running */
	bool m_wait_for_interrupt;
	bool m_interrupts_enabled;
	bool m_scheduled_IND_bit_reset;
	bool m_indirect_addressing;
	bool m_iodev_control[16];
	bool m_iodev_status[16];

	/* 8-bit registers for receiving data from peripherals */
	UINT8 m_iodev_incoming_byte[16];

	/* 8-bit registers for sending data to peripherals */
	UINT8 m_iodev_outgoing_byte[16];

	int m_flags;
	// V = "Vai um" (Carry flag)
	// T = "Transbordo" (Overflow flag)

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
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 2; }

private:
	void execute_instruction();
	void compute_effective_address(unsigned int addr);
	void set_flag(UINT8 flag, bool state);
	UINT16 read_panel_keys_register();
	devcb_read16 m_rc_read_cb;
	devcb_read16 m_buttons_read_cb;
	devcb_read8 m_iodev_read_cb[16];
	devcb_write8 m_iodev_write_cb[16];
	devcb_read8 m_iodev_status_cb[16];
	UINT8 m_mode;
};

extern const device_type PATINHO_FEIO;

#endif /* __PATINHOFEIO_H__ */
