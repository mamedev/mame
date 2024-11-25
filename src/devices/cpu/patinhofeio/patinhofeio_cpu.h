// license:GPL-2.0+
// copyright-holders:Felipe Sanches
#ifndef MAME_DEVICES_CPU_PATINHOFEIO_CPU_H
#define MAME_DEVICES_CPU_PATINHOFEIO_CPU_H

#pragma once

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
	using update_panel_cb = device_delegate<void (uint8_t ACC, uint8_t opcode, uint8_t mem_data, uint16_t mem_addr, uint16_t PC, uint8_t FLAGS, uint16_t RC, uint8_t mode)>;

	// construction/destruction
	patinho_feio_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, uint32_t _clock);

	auto rc_read() { return m_rc_read_cb.bind(); }
	auto buttons_read() { return m_buttons_read_cb.bind(); }
	template <std::size_t DevNumber> auto iodev_read() { return m_iodev_read_cb[DevNumber].bind(); }
	template <std::size_t DevNumber> auto iodev_write() { return m_iodev_write_cb[DevNumber].bind(); }
	template <std::size_t DevNumber> auto iodev_status() { return m_iodev_status_cb[DevNumber].bind(); }
	template <typename... T> void set_update_panel_cb(T &&... args) { m_update_panel_cb.set(std::forward<T>(args)...); }

	void transfer_byte_from_external_device(uint8_t channel, uint8_t data);
	void set_iodev_status(uint8_t channel, bool status) { m_iodev_status[channel] = status; }

	void prog_8bit(address_map &map) ATTR_COLD;
protected:

	virtual void execute_run() override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	address_space_config m_program_config;
	update_panel_cb m_update_panel_cb;

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
	uint8_t m_iodev_incoming_byte[16];

	/* 8-bit registers for sending data to peripherals */
	uint8_t m_iodev_outgoing_byte[16];

	int m_flags;
	// V = "Vai um" (Carry flag)
	// T = "Transbordo" (Overflow flag)

	int m_address_mask;        /* address mask */
	int m_icount;

	address_space *m_program;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 2; }

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	void execute_instruction();
	void compute_effective_address(unsigned int addr);
	void set_flag(uint8_t flag, bool state);
	uint16_t read_panel_keys_register();
	devcb_read16 m_rc_read_cb;
	devcb_read16 m_buttons_read_cb;
	devcb_read8::array<16> m_iodev_read_cb;
	devcb_write8::array<16> m_iodev_write_cb;
	devcb_read8::array<16> m_iodev_status_cb;
	uint8_t m_mode;
};

DECLARE_DEVICE_TYPE(PATO_FEIO_CPU, patinho_feio_cpu_device)

#endif // MAME_DEVICES_CPU_PATINHOFEIO_CPU_H
