// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
#ifndef MAME_CPU_SSP1601_SSP1601_H
#define MAME_CPU_SSP1601_SSP1601_H

#pragma once


enum
{
	/* general registers */
	SSP_R0,   SSP_X,     SSP_Y,    SSP_A,
	SSP_ST,   SSP_STACK, SSP_PC,   SSP_P,
	/* hardware stack */
	SSP_STACK0, SSP_STACK1, SSP_STACK2, SSP_STACK3, SSP_STACK4, SSP_STACK5,
	/* pointer registers */
	SSP_PR0, SSP_PR1, SSP_PR2, SSP_PR3, SSP_PR4, SSP_PR5, SSP_PR6, SSP_PR7
};


class ssp1601_device : public cpu_device
{
public:
	// construction/destruction
	ssp1601_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 4; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;
	address_space_config m_io_config;

	PAIR m_gr[8];     /* general regs, some are 16bit, some 32bit */
	union
	{
		unsigned char m_r[8];             /* pointer registers, 4 for earch bank */
		struct {
			unsigned char m_r0[4];
			unsigned char m_r1[4];
		} regs;
	};
	union
	{
		unsigned short m_RAM[256*2];      /* 2 256-word internal RAM banks */
		struct {
			unsigned short m_RAM0[256];
			unsigned short m_RAM1[256];
		} mem;
	};
	uint16_t m_stack[6]; /* 6-level hardware stack */
	PAIR m_ppc;

	int m_g_cycles;

	memory_access<16, 1, -1, ENDIANNESS_BIG>::cache m_cache;
	memory_access<16, 1, -1, ENDIANNESS_BIG>::specific m_program;
	memory_access< 4, 1,  0, ENDIANNESS_BIG>::specific m_io;

	void update_P();
	uint32_t read_unknown(int reg);
	void write_unknown(int reg, uint32_t d);
	uint32_t read_ext(int reg);
	void write_ext(int reg, uint32_t d);
	void write_ST(int reg, uint32_t d);
	uint32_t read_STACK(int reg);
	void write_STACK(int reg, uint32_t d);
	uint32_t read_PC(int reg);
	void write_PC(int reg, uint32_t d);
	uint32_t read_P(int reg);
	uint32_t read_AL(int reg);
	void write_AL(int reg, uint32_t d);
	uint32_t ptr1_read_(int ri, int isj2, int modi3);
	void ptr1_write(int op, uint32_t d);
	uint32_t ptr2_read(int op);

	typedef uint32_t (ssp1601_device::*read_func_t)(int reg);
	typedef void (ssp1601_device::*write_func_t)(int reg, uint32_t d);

	static const read_func_t reg_read_handlers[16];
	static const write_func_t reg_write_handlers[16];

};


DECLARE_DEVICE_TYPE(SSP1601, ssp1601_device)

#endif // MAME_CPU_SSP1601_SSP1601_H
