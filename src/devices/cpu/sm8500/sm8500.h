// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_CPU_SM8500_SM8500_H
#define MAME_CPU_SM8500_SM8500_H

#pragma once

enum
{
	/* "main" 16 bit register */
	SM8500_PC=1, SM8500_SP, SM8500_PS, SM8500_SYS16, SM8500_RR0, SM8500_RR2, SM8500_RR4, SM8500_RR6, SM8500_RR8, SM8500_RR10,
	SM8500_RR12, SM8500_RR14,
	/* additional internal 8 bit registers */
	SM8500_IE0, SM8500_IE1, SM8500_IR0, SM8500_IR1, SM8500_P0, SM8500_P1, SM8500_P2, SM8500_P3, SM8500_SYS, SM8500_CKC,
	SM8500_SPH, SM8500_SPL, SM8500_PS0, SM8500_PS1, SM8500_P0C, SM8500_P1C, SM8500_P2C, SM8500_P3C
};


class sm8500_cpu_device : public cpu_device
{
public:
	// construction/destruction
	sm8500_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, uint32_t _clock);

	// configuration helpers
	auto dma_cb() { return m_dma_func.bind(); }
	auto timer_cb() { return m_timer_func.bind(); }

	/* interrupts */
	static constexpr int ILL_INT  = 0;
	static constexpr int DMA_INT  = 1;
	static constexpr int TIM0_INT = 2;
	static constexpr int EXT_INT  = 3;
	static constexpr int UART_INT = 4;
	static constexpr int LCDC_INT = 5;
	static constexpr int TIM1_INT = 6;
	static constexpr int CK_INT   = 7;
	static constexpr int PIO_INT  = 8;
	static constexpr int WDT_INT  = 9;
	static constexpr int NMI_INT  = 10;

protected:
	// Flags
	static constexpr uint8_t FLAG_C = 0x80;
	static constexpr uint8_t FLAG_Z = 0x40;
	static constexpr uint8_t FLAG_S = 0x20;
	static constexpr uint8_t FLAG_V = 0x10;
	static constexpr uint8_t FLAG_D = 0x08;
	static constexpr uint8_t FLAG_H = 0x04;
	static constexpr uint8_t FLAG_B = 0x02;
	static constexpr uint8_t FLAG_I = 0x01;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 16; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	inline void get_sp();
	uint8_t mem_readbyte(uint32_t offset) const;
	void mem_writebyte(uint32_t offset, uint8_t data);
	inline uint16_t mem_readword(uint32_t address) const { return (mem_readbyte(address ) << 8) | (mem_readbyte(address+1)); }
	inline void mem_writeword(uint32_t address, uint16_t value) { mem_writebyte(address, value >> 8); mem_writebyte(address+1, value); }
	inline void take_interrupt(uint16_t vector);
	void process_interrupts();

	address_space_config m_program_config;

	devcb_write8 m_dma_func;
	devcb_write8 m_timer_func;

	uint16_t m_PC;
	uint8_t m_IE0;
	uint8_t m_IE1;
	uint8_t m_IR0;
	uint8_t m_IR1;
	uint8_t m_SYS;
	uint8_t m_CKC;
	uint8_t m_clock_changed;
	uint16_t m_SP;
	uint8_t m_PS0;
	uint8_t m_PS1;
	uint16_t m_IFLAGS;
	uint8_t m_CheckInterrupts;
	int m_halted;
	int m_icount;
	address_space *m_program;
	uint16_t m_oldpc;
	uint8_t m_register_ram[0x108];
};


DECLARE_DEVICE_TYPE(SM8500, sm8500_cpu_device)

#endif // MAME_CPU_SM8500_SM8500_H
