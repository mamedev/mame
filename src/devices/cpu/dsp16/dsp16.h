// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/***************************************************************************

    dsp16.h

    WE|AT&T DSP16 series emulator.

***************************************************************************/

#pragma once

#ifndef __DSP16_H__
#define __DSP16_H__


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dsp16_device

class dsp16_device : public cpu_device
{
public:
	// construction/destruction
	dsp16_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// public interfaces

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const override { return (clocks + 2 - 1) / 2; } // internal /2 divider
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const override { return (cycles * 2); } // internal /2 divider
	virtual UINT32 execute_min_cycles() const override;
	virtual UINT32 execute_max_cycles() const override;
	virtual UINT32 execute_input_lines() const override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override;
	virtual UINT32 disasm_max_opcode_bytes() const override;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	// address spaces
	const address_space_config m_program_config;
	const address_space_config m_data_config;

	// CPU registers
	// ROM Address Arithmetic Unit (XAAU)  (page 2-4)
	UINT16 m_i;     // 12 bits
	UINT16 m_pc;
	UINT16 m_pt;
	UINT16 m_pr;
	UINT16 m_pi;

	// RAM Address Arithmetic Unit (YAAU)  (page 2-6)
	UINT16 m_j;     // Signed
	UINT16 m_k;     // Signed
	UINT16 m_rb;
	UINT16 m_re;
	UINT16 m_r0;
	UINT16 m_r1;
	UINT16 m_r2;
	UINT16 m_r3;

	// Data Arithmetic Unit (DAU)  (page 2-6)
	UINT16 m_x;
	UINT32 m_y;
	UINT32 m_p;
	UINT64 m_a0;    // 36 bits
	UINT64 m_a1;    // 36 bits
	UINT8 m_auc;    // 6 bits
	UINT16 m_psw;
	UINT8 m_c0;
	UINT8 m_c1;
	UINT8 m_c2;

	// Serial and parallel interfaces (TODO: More here  (page 2-13))
	UINT16 m_sioc;
	UINT16 m_srta;
	UINT16 m_sdx;
	UINT16 m_pioc;
	UINT16 m_pdx0;  // pdx0 & pdx1 refer to the same physical register (page 6-1)
	UINT16 m_pdx1;  // but we keep them separate for logic's sake.

	// internal stuff
	UINT16 m_ppc;

	// This CPU core handles the cache as more of a loop than 15 separate memory elements.
	// It's a bit of a hack, but it's easier this way (for now).
	UINT16 m_cacheStart;
	UINT16 m_cacheEnd;
	UINT16 m_cacheRedoNextPC;
	UINT16 m_cacheIterations;
	static const UINT16 CACHE_INVALID = 0xffff;

	// memory access
	inline UINT32 data_read(const UINT16& addr);
	inline void data_write(const UINT16& addr, const UINT16& data);
	inline UINT32 opcode_read(const UINT8 pcOffset=0);

	// address spaces
	address_space* m_program;
	address_space* m_data;
	direct_read_data* m_direct;

	// other internal states
	int m_icount;

	// operations
	void execute_one(const UINT16& op, UINT8& cycles, UINT8& pcAdvance);

	// table decoders
	void* registerFromRImmediateField(const UINT8& R);
	void* registerFromRTable(const UINT8& R);
	UINT16* registerFromYFieldUpper(const UINT8& Y);

	// execution
	void executeF1Field(const UINT8& F1, const UINT8& D, const UINT8& S);
	void executeYFieldPost(const UINT8& Y);
	void executeZFieldPartOne(const UINT8& Z, UINT16* rN);
	void executeZFieldPartTwo(const UINT8& Z, UINT16* rN);

	// helpers
	void* addressYL();
	void writeRegister(void* reg, const UINT16& value);
	bool conditionTest(const UINT8& CON);

	// flags
	bool lmi();
	bool leq();
	bool llv();
	bool lmv();
};


// device type definition
extern const device_type DSP16;


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	DSP16_I,        // ROM Address Arithmetic Unit (XAAU)
	DSP16_PC,
	DSP16_PT,
	DSP16_PR,
	DSP16_PI,
	DSP16_J,        // RAM Address Arithmetic Unit (YAAU)
	DSP16_K,
	DSP16_RB,
	DSP16_RE,
	DSP16_R0,
	DSP16_R1,
	DSP16_R2,
	DSP16_R3,
	DSP16_X,        // Data Arithmetic Unit (DAU)
	DSP16_Y,
	DSP16_P,
	DSP16_A0,
	DSP16_A1,
	DSP16_AUC,
	DSP16_PSW,
	DSP16_C0,
	DSP16_C1,
	DSP16_C2,
	DSP16_SIOC,
	DSP16_SRTA,
	DSP16_SDX,
	DSP16_PIOC,
	DSP16_PDX0,
	DSP16_PDX1
};


#endif /* __DSP16_H__ */
