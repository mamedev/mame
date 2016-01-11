// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    cubeqcpu.h
    Interface file for the Cube Quest CPUs
    Written by Phil Bennett

***************************************************************************/

#ifndef _CUBEQCPU_H
#define _CUBEQCPU_H


/***************************************************************************
    COMPILE-TIME DEFINITIONS
***************************************************************************/


/***************************************************************************
    GLOBAL CONSTANTS
***************************************************************************/


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	CQUESTSND_PC = 1,
	CQUESTSND_Q,
	CQUESTSND_RAM0,
	CQUESTSND_RAM1,
	CQUESTSND_RAM2,
	CQUESTSND_RAM3,
	CQUESTSND_RAM4,
	CQUESTSND_RAM5,
	CQUESTSND_RAM6,
	CQUESTSND_RAM7,
	CQUESTSND_RAM8,
	CQUESTSND_RAM9,
	CQUESTSND_RAMA,
	CQUESTSND_RAMB,
	CQUESTSND_RAMC,
	CQUESTSND_RAMD,
	CQUESTSND_RAME,
	CQUESTSND_RAMF,
	CQUESTSND_RTNLATCH,
	CQUESTSND_ADRCNTR,
	CQUESTSND_DINLATCH
};

enum
{
	CQUESTROT_PC = 1,
	CQUESTROT_Q,
	CQUESTROT_RAM0,
	CQUESTROT_RAM1,
	CQUESTROT_RAM2,
	CQUESTROT_RAM3,
	CQUESTROT_RAM4,
	CQUESTROT_RAM5,
	CQUESTROT_RAM6,
	CQUESTROT_RAM7,
	CQUESTROT_RAM8,
	CQUESTROT_RAM9,
	CQUESTROT_RAMA,
	CQUESTROT_RAMB,
	CQUESTROT_RAMC,
	CQUESTROT_RAMD,
	CQUESTROT_RAME,
	CQUESTROT_RAMF,
	CQUESTROT_SEQCNT,
	CQUESTROT_DYNADDR,
	CQUESTROT_DYNDATA,
	CQUESTROT_YRLATCH,
	CQUESTROT_YDLATCH,
	CQUESTROT_DINLATCH,
	CQUESTROT_DSRCLATCH,
	CQUESTROT_RSRCLATCH,
	CQUESTROT_LDADDR,
	CQUESTROT_LDDATA
};

enum
{
	CQUESTLIN_FGPC = 1,
	CQUESTLIN_BGPC,
	CQUESTLIN_Q,
	CQUESTLIN_RAM0,
	CQUESTLIN_RAM1,
	CQUESTLIN_RAM2,
	CQUESTLIN_RAM3,
	CQUESTLIN_RAM4,
	CQUESTLIN_RAM5,
	CQUESTLIN_RAM6,
	CQUESTLIN_RAM7,
	CQUESTLIN_RAM8,
	CQUESTLIN_RAM9,
	CQUESTLIN_RAMA,
	CQUESTLIN_RAMB,
	CQUESTLIN_RAMC,
	CQUESTLIN_RAMD,
	CQUESTLIN_RAME,
	CQUESTLIN_RAMF,
	CQUESTLIN_FADLATCH,
	CQUESTLIN_BADLATCH,
	CQUESTLIN_SREG,
	CQUESTLIN_XCNT,
	CQUESTLIN_YCNT,
	CQUESTLIN_CLATCH,
	CQUESTLIN_ZLATCH
};


/***************************************************************************
    CONFIGURATION STRUCTURE
***************************************************************************/


#define MCFG_CQUESTSND_CONFIG(_dac_w, _sound_tag) \
	cquestsnd_cpu_device::set_dac_w(*device, DEVCB_##_dac_w); \
	cquestsnd_cpu_device::set_sound_region(*device, _sound_tag);


#define MCFG_CQUESTROT_CONFIG(_linedata_w) \
	cquestrot_cpu_device::set_linedata_w(*device, DEVCB_##_linedata_w );


#define MCFG_CQUESTLIN_CONFIG(_linedata_r) \
	cquestlin_cpu_device::set_linedata_r(*device, DEVCB_##_linedata_r );


/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

class cquestsnd_cpu_device : public cpu_device
{
public:
	// construction/destruction
	cquestsnd_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_dac_w(device_t &device, _Object object) { return downcast<cquestsnd_cpu_device &>(device).m_dac_w.set_callback(object); }
	static void set_sound_region(device_t &device, const char *tag) { downcast<cquestsnd_cpu_device &>(device).m_sound_region_tag = tag; }

	DECLARE_WRITE16_MEMBER(sndram_w);
	DECLARE_READ16_MEMBER(sndram_r);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 1; }
	virtual UINT32 execute_input_lines() const override { return 0; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 8; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 8; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	address_space_config m_program_config;

	/* AM2901 internals */
	UINT16  m_ram[16];
	UINT16  m_q;
	UINT16  m_f;
	UINT16  m_y;
	UINT32  m_cflag;
	UINT32  m_vflag;

	UINT8   m_pc;         /* 2 x LS161 @ 6E, 6F */
	UINT16  m_platch;
	UINT8   m_rtnlatch;   /* LS374 @ 5F */
	UINT8   m_adrcntr;    /* 2 x LS161 */
	UINT16  m_adrlatch;
	UINT16  m_dinlatch;
	UINT16  m_ramwlatch;

	UINT16 m_sram[4096/2];

	int m_prev_ipram;
	int m_prev_ipwrt;

	devcb_write16 m_dac_w;
	const char *m_sound_region_tag;
	UINT16 *m_sound_data;

	address_space *m_program;
	direct_read_data *m_direct;
	int m_icount;

	int do_sndjmp(int jmp);
};


class cquestrot_cpu_device : public cpu_device
{
public:
	// construction/destruction
	cquestrot_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_linedata_w(device_t &device, _Object object) { return downcast<cquestrot_cpu_device &>(device).m_linedata_w.set_callback(object); }

	DECLARE_READ16_MEMBER(linedata_r);
	DECLARE_WRITE16_MEMBER(rotram_w);
	DECLARE_READ16_MEMBER(rotram_r);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 1; }
	virtual UINT32 execute_input_lines() const override { return 0; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 8; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 8; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	address_space_config m_program_config;
	devcb_write16 m_linedata_w;

	/* AM2901 internals */
	UINT16  m_ram[16];
	UINT16  m_q;
	UINT16  m_f;
	UINT16  m_y;
	UINT32  m_cflag;
	UINT32  m_vflag;

	UINT16  m_pc;         /* 12-bit, but only 9 used */
	UINT8   m_seqcnt;     /* 4-bit counter */

	UINT8   m_dsrclatch;
	UINT8   m_rsrclatch;
	UINT16  m_dynaddr;    /* LS374 at 2D, 8D  */
	UINT16  m_dyndata;    /* LS374 at 10B, 9B */
	UINT16  m_yrlatch;    /* LS374 at 9D, 10D */
	UINT16  m_ydlatch;    /* LS374 at 9C, 10C */
	UINT16  m_dinlatch;
	UINT8   m_divreg;     /* LS74 at ? */

	UINT16  m_linedata;
	UINT16  m_lineaddr;

	UINT16 m_dram[16384]; /* Shared with 68000 */
	UINT16 m_sram[2048];  /* Private */

	UINT8 m_prev_dred;
	UINT8 m_prev_dwrt;
	UINT8 m_wc;
	UINT8 m_rc;
	UINT8 m_clkcnt;

	address_space *m_program;
	direct_read_data *m_direct;
	int m_icount;

	// For the debugger
	UINT8 m_flags;

	int do_rotjmp(int jmp);
};


class cquestlin_cpu_device : public cpu_device
{
public:
	// construction/destruction
	cquestlin_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_linedata_r(device_t &device, _Object object) { return downcast<cquestlin_cpu_device &>(device).m_linedata_r.set_callback(object); }

	DECLARE_WRITE16_MEMBER( linedata_w );
	void cubeqcpu_swap_line_banks();
	void cubeqcpu_clear_stack();
	UINT8 cubeqcpu_get_ptr_ram_val(int i);
	UINT32* cubeqcpu_get_stack_ram();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 1; }
	virtual UINT32 execute_input_lines() const override { return 0; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 8; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 8; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	address_space_config m_program_config;
	devcb_read16 m_linedata_r;

	/* 12-bit AM2901 internals */
	UINT16  m_ram[16];
	UINT16  m_q;
	UINT16  m_f;
	UINT16  m_y;
	UINT32  m_cflag;
	UINT32  m_vflag;

	UINT8   m_pc[2];      /* Two program counters; one for FG, other for BG */

	UINT16  m_seqcnt;     /* 12-bit */
	UINT16  m_clatch;     /* LS374 at 9E and 1-bit FF */
	UINT8   m_zlatch;     /* LS374 at 4H */

	UINT16  m_xcnt;
	UINT16  m_ycnt;
	UINT8   m_sreg;

	UINT16  m_fadlatch;
	UINT16  m_badlatch;

	UINT16  m_sramdlatch;

	UINT8   m_fglatch;
	UINT8   m_bglatch;
	UINT8   m_gt0reg;
	UINT8   m_fdxreg;
	UINT32  m_field;

	UINT32  m_clkcnt;

	/* RAM */
	UINT16  m_sram[4096];       /* Shared with rotate CPU */
	UINT8   m_ptr_ram[1024];    /* Pointer RAM */
	UINT32  m_e_stack[32768];   /* Stack DRAM: 32kx20 */
	UINT32  m_o_stack[32768];   /* Stack DRAM: 32kx20 */

	address_space *m_program;
	direct_read_data *m_direct;
	int m_icount;

	// For the debugger
	UINT8 m_flags;
	UINT16 m_curpc;

	int do_linjmp(int jmp);
};


extern const device_type CQUESTSND;
extern const device_type CQUESTROT;
extern const device_type CQUESTLIN;


#endif /* _CUBEQCPU_H */
