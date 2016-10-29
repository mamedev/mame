// license:BSD-3-Clause
// copyright-holders:F. Ulivi
//
// *****************************
// Emulator for HP nanoprocessor
// *****************************
//
// http://www.hp9845.net/9845/downloads/manuals/Nanoprocessor.pdf
#ifndef _NANOPROCESSOR_H_
#define _NANOPROCESSOR_H_

#define HP_NANO_REGS	16	// Number of GP registers
#define HP_NANO_PC_MASK	0x7ff	// Mask of PC meaningful bits: 11 bits available
#define HP_NANO_DC_NO	8	// Number of direct control lines (DC7 is typically used as interrupt mask)
#define HP_NANO_IE_DC	7	// DC line used as interrupt enable/mask (DC7)

// DC changed callback
// The callback receives a 8-bit word holding the state of all DC lines.
// DC0 is in bit 0, DC1 in bit 1 and so on.
// Keep in mind that DC7 usually masks the interrupt signal.
#define MCFG_HP_NANO_DC_CHANGED(_devcb)									\
	hp_nanoprocessor_device::set_dc_changed_func(*device , DEVCB_##_devcb);

class hp_nanoprocessor_device : public cpu_device
{
public:
	hp_nanoprocessor_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_dc_changed_func(device_t &device, _Object object) { return downcast<hp_nanoprocessor_device &>(device).m_dc_changed_func.set_callback(object); }

	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 2; }
	// 3 cycles is for int. acknowledge + 1 instruction
	virtual uint32_t execute_max_cycles() const override { return 3; }
	virtual uint32_t execute_input_lines() const override { return 1; }
	virtual uint32_t execute_default_irq_vector() const override { return 0xff; }
	virtual void execute_run() override;
	virtual void execute_set_input(int linenum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum) const override {
		return (spacenum == AS_PROGRAM) ? &m_program_config : ((spacenum == AS_IO) ? &m_io_config : nullptr);
	}

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual uint32_t disasm_min_opcode_bytes() const override { return 1; }
	virtual uint32_t disasm_max_opcode_bytes() const override { return 2; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;

private:
	devcb_write8 m_dc_changed_func;
	int m_icount;

	// State of processor
	uint8_t  m_reg_A;	// Accumulator
	uint8_t  m_reg_R[ HP_NANO_REGS ];	// General purpose registers
	uint16_t m_reg_PA;	// Program counter ("Program Address" in HP doc)
	uint16_t m_reg_SSR;	// Subroutine stack register
	uint16_t m_reg_ISR;	// Interrupt stack register
	uint16_t m_flags;	// Flags: extend flag (E) & direct control lines (DC0-7)

	address_space_config m_program_config;
	address_space_config m_io_config;

	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_io;

	void execute_one(uint8_t opcode);
	uint16_t pa_offset(unsigned off) const;
	uint8_t fetch(void);
	void skip(void);
	void dc_update(void);
	void dc_set(unsigned bit_no);
	void dc_clr(unsigned bit_no);
};

extern const device_type HP_NANOPROCESSOR;

#endif /* _NANOPROCESSOR_H_ */
