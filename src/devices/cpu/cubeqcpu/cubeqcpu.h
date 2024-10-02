// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    cubeqcpu.h

    Interface file for the Cube Quest CPUs
    Written by Phil Bennett

***************************************************************************/

#ifndef MAME_CPU_CUBEQCPU_CUBEQCPU_H
#define MAME_CPU_CUBEQCPU_CUBEQCPU_H


/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

class cquestsnd_cpu_device : public cpu_device
{
public:
	// construction/destruction
	cquestsnd_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto dac_w() { return m_dac_w.bind(); }
	void set_sound_region(const char *tag) { m_sound_region_tag = tag; }

	void sndram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 sndram_r(offs_t offset);

protected:
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

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 1; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;

	/* AM2901 internals */
	u16  m_ram[16];
	u16  m_q;
	u16  m_f;
	u16  m_y;
	u32  m_cflag;
	u32  m_vflag;

	u8   m_pc;         /* 2 x LS161 @ 6E, 6F */
	u16  m_platch;
	u8   m_rtnlatch;   /* LS374 @ 5F */
	u8   m_adrcntr;    /* 2 x LS161 */
	u16  m_adrlatch;
	u16  m_dinlatch;
	u16  m_ramwlatch;

	u16 m_sram[4096/2];

	bool m_prev_ipram;
	bool m_prev_ipwrt;

	devcb_write16 m_dac_w;
	const char *m_sound_region_tag;
	u16 *m_sound_data;

	memory_access<9, 3, -3, ENDIANNESS_BIG>::cache m_cache;
	memory_access<9, 3, -3, ENDIANNESS_BIG>::specific m_program;

	int m_icount;

	bool do_sndjmp(u8 jmp);
};


class cquestrot_cpu_device : public cpu_device
{
public:
	// construction/destruction
	cquestrot_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto linedata_w() { return m_linedata_w.bind(); }

	u16 linedata_r();
	void rotram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 rotram_r(offs_t offset);

protected:
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

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 1; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;
	devcb_write16 m_linedata_w;

	/* AM2901 internals */
	u16  m_ram[16];
	u16  m_q;
	u16  m_f;
	u16  m_y;
	u32  m_cflag;
	u32  m_vflag;

	u16  m_pc;         /* 12-bit, but only 9 used */
	u8   m_seqcnt;     /* 4-bit counter */

	u8   m_dsrclatch;
	u8   m_rsrclatch;
	u16  m_dynaddr;    /* LS374 at 2D, 8D  */
	u16  m_dyndata;    /* LS374 at 10B, 9B */
	u16  m_yrlatch;    /* LS374 at 9D, 10D */
	u16  m_ydlatch;    /* LS374 at 9C, 10C */
	u16  m_dinlatch;
	u8   m_divreg;     /* LS74 at ? */

	u16  m_linedata;
	u16  m_lineaddr;

	u16 m_dram[16384]; /* Shared with 68000 */
	u16 m_sram[2048];  /* Private */

	u8 m_prev_dred;
	u8 m_prev_dwrt;
	u8 m_wc;
	u8 m_rc;
	u8 m_clkcnt;

	memory_access<9, 3, -3, ENDIANNESS_BIG>::cache m_cache;
	memory_access<9, 3, -3, ENDIANNESS_BIG>::specific m_program;
	int m_icount;

	// For the debugger
	u8 m_flags;

	int do_rotjmp(u8 jmp);
};


class cquestlin_cpu_device : public cpu_device
{
public:
	// construction/destruction
	cquestlin_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto linedata_r() { return m_linedata_r.bind(); }

	void linedata_w(offs_t offset, u16 data);
	void cubeqcpu_swap_line_banks();
	void cubeqcpu_clear_stack();
	u8 cubeqcpu_get_ptr_ram_val(int i);
	u32* cubeqcpu_get_stack_ram();

protected:
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

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 1; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;
	devcb_read16 m_linedata_r;

	/* 12-bit AM2901 internals */
	u16  m_ram[16];
	u16  m_q;
	u16  m_f;
	u16  m_y;
	u32  m_cflag;
	u32  m_vflag;

	u8   m_pc[2];      /* Two program counters; one for FG, other for BG */

	u16  m_seqcnt;     /* 12-bit */
	u16  m_clatch;     /* LS374 at 9E and 1-bit FF */
	u8   m_zlatch;     /* LS374 at 4H */

	u16  m_xcnt;
	u16  m_ycnt;
	u8   m_sreg;

	u16  m_fadlatch;
	u16  m_badlatch;

	u16  m_sramdlatch;

	u8   m_fglatch;
	u8   m_bglatch;
	u8   m_gt0reg;
	u8   m_fdxreg;
	u32  m_field;

	u32  m_clkcnt;

	/* RAM */
	u16  m_sram[4096];       /* Shared with rotate CPU */
	u8   m_ptr_ram[1024];    /* Pointer RAM */
	u32  m_e_stack[32768];   /* Stack DRAM: 32kx20 */
	u32  m_o_stack[32768];   /* Stack DRAM: 32kx20 */

	memory_access<9, 3, -3, ENDIANNESS_BIG>::cache m_cache;
	memory_access<9, 3, -3, ENDIANNESS_BIG>::specific m_program;
	int m_icount;

	// For the debugger
	u8 m_flags;
	u16 m_curpc;

	int do_linjmp(u8 jmp);
};


DECLARE_DEVICE_TYPE(CQUESTSND, cquestsnd_cpu_device)
DECLARE_DEVICE_TYPE(CQUESTROT, cquestrot_cpu_device)
DECLARE_DEVICE_TYPE(CQUESTLIN, cquestlin_cpu_device)


#endif // MAME_CPU_CUBEQCPU_CUBEQCPU_H
