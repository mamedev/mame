// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#pragma once

#ifndef __SM8500_H__
#define __SM8500_H__

#define MCFG_SM8500_DMA_CB(_devcb) \
	sm8500_cpu_device::set_dma_cb(*device, DEVCB_##_devcb);

#define MCFG_SM8500_TIMER_CB(_devcb) \
	sm8500_cpu_device::set_timer_cb(*device, DEVCB_##_devcb);

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
	sm8500_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, UINT32 _clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_dma_cb(device_t &device, _Object object) { return downcast<sm8500_cpu_device &>(device).m_dma_func.set_callback(object); }
	template<class _Object> static devcb_base &set_timer_cb(device_t &device, _Object object) { return downcast<sm8500_cpu_device &>(device).m_timer_func.set_callback(object); }

	/* interrupts */
	static const int ILL_INT  = 0;
	static const int DMA_INT  = 1;
	static const int TIM0_INT = 2;
	static const int EXT_INT  = 3;
	static const int UART_INT = 4;
	static const int LCDC_INT = 5;
	static const int TIM1_INT = 6;
	static const int CK_INT   = 7;
	static const int PIO_INT  = 8;
	static const int WDT_INT  = 9;
	static const int NMI_INT  = 10;

protected:
	// Flags
	static const UINT8 FLAG_C = 0x80;
	static const UINT8 FLAG_Z = 0x40;
	static const UINT8 FLAG_S = 0x20;
	static const UINT8 FLAG_V = 0x10;
	static const UINT8 FLAG_D = 0x08;
	static const UINT8 FLAG_H = 0x04;
	static const UINT8 FLAG_B = 0x02;
	static const UINT8 FLAG_I = 0x01;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 16; }
	virtual UINT32 execute_input_lines() const override { return 11; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 5; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	inline void get_sp();
	UINT8 mem_readbyte(UINT32 offset) const;
	void mem_writebyte(UINT32 offset, UINT8 data);
	inline UINT16 mem_readword(UINT32 address) const { return (mem_readbyte(address ) << 8) | (mem_readbyte(address+1)); }
	inline void mem_writeword(UINT32 address, UINT16 value) { mem_writebyte(address, value >> 8); mem_writebyte(address+1, value); }
	inline void take_interrupt(UINT16 vector);
	void process_interrupts();

	address_space_config m_program_config;

	devcb_write8 m_dma_func;
	devcb_write8 m_timer_func;

	UINT16 m_PC;
	UINT8 m_IE0;
	UINT8 m_IE1;
	UINT8 m_IR0;
	UINT8 m_IR1;
	UINT8 m_SYS;
	UINT8 m_CKC;
	UINT8 m_clock_changed;
	UINT16 m_SP;
	UINT8 m_PS0;
	UINT8 m_PS1;
	UINT16 m_IFLAGS;
	UINT8 m_CheckInterrupts;
	int m_halted;
	int m_icount;
	address_space *m_program;
	UINT16 m_oldpc;
	UINT8 m_register_ram[0x108];
};


extern const device_type SM8500;


#endif /* __SM8500_H__ */
