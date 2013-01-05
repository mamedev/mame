/***************************************************************************

    dsp16.h

    WE|AT&T DSP16 series emulator.

***************************************************************************/

#pragma once

#ifndef __DSP16_H__
#define __DSP16_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

//#define MCFG_DSP16_CONFIG(_config)
//  dsp16_device::static_set_config(*device, _config);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dsp16_device

class dsp16_device : public cpu_device
{
public:
	// construction/destruction
	dsp16_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// public interfaces

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const;
	virtual UINT32 execute_max_cycles() const;
	virtual UINT32 execute_input_lines() const;
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, astring &string);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const;
	virtual UINT32 disasm_max_opcode_bytes() const;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	// address spaces
	const address_space_config m_program_config;

	// CPU registers
	// ROM Address Arithmetic Unit (XAAU)
	UINT16 m_i;		// 12 bits
	UINT16 m_pc;
	UINT16 m_pt;
	UINT16 m_pr;
	UINT16 m_pi;
	// RAM Address Arithmetic Unit (YAAU)
	UINT16 m_j;
	UINT16 m_k;
	UINT16 m_rb;
	UINT16 m_re;
	UINT16 m_r0;
	UINT16 m_r1;
	UINT16 m_r2;
	UINT16 m_r3;
	// Data Arithmetic Unit (DAU)
	UINT16 m_x;
	UINT32 m_y;
	UINT32 m_p;
	UINT64 m_a0;	// 36 bits
	UINT64 m_a1;	// 36 bits
	UINT8 m_auc;	// 6 bits
	UINT16 m_psw;
	UINT8 m_c0;
	UINT8 m_c1;
	UINT8 m_c2;
	// Serial and parallel interfaces
	UINT16 m_sioc;
	UINT16 m_pioc;

    // internal stuff
	UINT16 m_ppc;

	// memory access
	inline UINT32 program_read(UINT32 addr);
	inline void program_write(UINT32 addr, UINT32 data);
	inline UINT32 opcode_read(const UINT8 pcOffset=0);

	// address spaces
    address_space* m_program;
    direct_read_data* m_direct;

	// other internal states
    int m_icount;

	// operations
	void execute_one(const UINT16& op, UINT8& cycles, UINT8& pcAdvance);

	void* registerFromRTable(const UINT8& R);

	// helpers
	void* addressYL();
	//void writeYxRegister(const UINT16& value);
};


// device type definition
extern const device_type DSP16;


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	DSP16_I,		// ROM Address Arithmetic Unit (XAAU)
	DSP16_PC,
	DSP16_PT,
	DSP16_PR,
	DSP16_PI,
	DSP16_J,		// RAM Address Arithmetic Unit (YAAU)
	DSP16_K,
	DSP16_RB,
	DSP16_RE,
	DSP16_R0,
	DSP16_R1,
	DSP16_R2,
	DSP16_R3,
	DSP16_X,		// Data Arithmetic Unit (DAU)
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
	DSP16_PIOC
};


#endif /* __DSP16_H__ */
