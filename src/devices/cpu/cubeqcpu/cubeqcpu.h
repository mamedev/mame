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
    CONFIGURATION STRUCTURE
***************************************************************************/


#define MCFG_CQUESTSND_CONFIG(_dac_w, _sound_tag) \
	downcast<cquestsnd_cpu_device &>(*device).set_dac_w(DEVCB_##_dac_w); \
	downcast<cquestsnd_cpu_device &>(*device).set_sound_region(_sound_tag);


#define MCFG_CQUESTROT_CONFIG(_linedata_w) \
	downcast<cquestrot_cpu_device &>(*device).set_linedata_w(DEVCB_##_linedata_w );


#define MCFG_CQUESTLIN_CONFIG(_linedata_r) \
	downcast<cquestlin_cpu_device &>(*device).set_linedata_r(DEVCB_##_linedata_r );


/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

class cquestsnd_cpu_device : public cpu_device
{
public:
	// construction/destruction
	cquestsnd_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	template <class Object> devcb_base &set_dac_w(Object &&cb) { return m_dac_w.set_callback(std::forward<Object>(cb)); }
	void set_sound_region(const char *tag) { m_sound_region_tag = tag; }

	DECLARE_WRITE16_MEMBER(sndram_w);
	DECLARE_READ16_MEMBER(sndram_r);

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
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 1; }
	virtual uint32_t execute_max_cycles() const override { return 1; }
	virtual uint32_t execute_input_lines() const override { return 0; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;

	/* AM2901 internals */
	uint16_t  m_ram[16];
	uint16_t  m_q;
	uint16_t  m_f;
	uint16_t  m_y;
	uint32_t  m_cflag;
	uint32_t  m_vflag;

	uint8_t   m_pc;         /* 2 x LS161 @ 6E, 6F */
	uint16_t  m_platch;
	uint8_t   m_rtnlatch;   /* LS374 @ 5F */
	uint8_t   m_adrcntr;    /* 2 x LS161 */
	uint16_t  m_adrlatch;
	uint16_t  m_dinlatch;
	uint16_t  m_ramwlatch;

	uint16_t m_sram[4096/2];

	int m_prev_ipram;
	int m_prev_ipwrt;

	devcb_write16 m_dac_w;
	const char *m_sound_region_tag;
	uint16_t *m_sound_data;

	address_space *m_program;
	memory_access_cache<3, -3, ENDIANNESS_BIG> *m_cache;
	int m_icount;

	int do_sndjmp(int jmp);
};


class cquestrot_cpu_device : public cpu_device
{
public:
	// construction/destruction
	cquestrot_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	template <class Object> devcb_base &set_linedata_w(Object &&cb) { return m_linedata_w.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ16_MEMBER(linedata_r);
	DECLARE_WRITE16_MEMBER(rotram_w);
	DECLARE_READ16_MEMBER(rotram_r);

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
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 1; }
	virtual uint32_t execute_max_cycles() const override { return 1; }
	virtual uint32_t execute_input_lines() const override { return 0; }
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
	uint16_t  m_ram[16];
	uint16_t  m_q;
	uint16_t  m_f;
	uint16_t  m_y;
	uint32_t  m_cflag;
	uint32_t  m_vflag;

	uint16_t  m_pc;         /* 12-bit, but only 9 used */
	uint8_t   m_seqcnt;     /* 4-bit counter */

	uint8_t   m_dsrclatch;
	uint8_t   m_rsrclatch;
	uint16_t  m_dynaddr;    /* LS374 at 2D, 8D  */
	uint16_t  m_dyndata;    /* LS374 at 10B, 9B */
	uint16_t  m_yrlatch;    /* LS374 at 9D, 10D */
	uint16_t  m_ydlatch;    /* LS374 at 9C, 10C */
	uint16_t  m_dinlatch;
	uint8_t   m_divreg;     /* LS74 at ? */

	uint16_t  m_linedata;
	uint16_t  m_lineaddr;

	uint16_t m_dram[16384]; /* Shared with 68000 */
	uint16_t m_sram[2048];  /* Private */

	uint8_t m_prev_dred;
	uint8_t m_prev_dwrt;
	uint8_t m_wc;
	uint8_t m_rc;
	uint8_t m_clkcnt;

	address_space *m_program;
	memory_access_cache<3, -3, ENDIANNESS_BIG> *m_cache;
	int m_icount;

	// For the debugger
	uint8_t m_flags;

	int do_rotjmp(int jmp);
};


class cquestlin_cpu_device : public cpu_device
{
public:
	// construction/destruction
	cquestlin_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	template <class Object> devcb_base &set_linedata_r(Object &&cb) { return m_linedata_r.set_callback(std::forward<Object>(cb)); }

	DECLARE_WRITE16_MEMBER( linedata_w );
	void cubeqcpu_swap_line_banks();
	void cubeqcpu_clear_stack();
	uint8_t cubeqcpu_get_ptr_ram_val(int i);
	uint32_t* cubeqcpu_get_stack_ram();

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
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 1; }
	virtual uint32_t execute_max_cycles() const override { return 1; }
	virtual uint32_t execute_input_lines() const override { return 0; }
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
	uint16_t  m_ram[16];
	uint16_t  m_q;
	uint16_t  m_f;
	uint16_t  m_y;
	uint32_t  m_cflag;
	uint32_t  m_vflag;

	uint8_t   m_pc[2];      /* Two program counters; one for FG, other for BG */

	uint16_t  m_seqcnt;     /* 12-bit */
	uint16_t  m_clatch;     /* LS374 at 9E and 1-bit FF */
	uint8_t   m_zlatch;     /* LS374 at 4H */

	uint16_t  m_xcnt;
	uint16_t  m_ycnt;
	uint8_t   m_sreg;

	uint16_t  m_fadlatch;
	uint16_t  m_badlatch;

	uint16_t  m_sramdlatch;

	uint8_t   m_fglatch;
	uint8_t   m_bglatch;
	uint8_t   m_gt0reg;
	uint8_t   m_fdxreg;
	uint32_t  m_field;

	uint32_t  m_clkcnt;

	/* RAM */
	uint16_t  m_sram[4096];       /* Shared with rotate CPU */
	uint8_t   m_ptr_ram[1024];    /* Pointer RAM */
	uint32_t  m_e_stack[32768];   /* Stack DRAM: 32kx20 */
	uint32_t  m_o_stack[32768];   /* Stack DRAM: 32kx20 */

	address_space *m_program;
	memory_access_cache<3, -3, ENDIANNESS_BIG> *m_cache;
	int m_icount;

	// For the debugger
	uint8_t m_flags;
	uint16_t m_curpc;

	int do_linjmp(int jmp);
};


DECLARE_DEVICE_TYPE(CQUESTSND, cquestsnd_cpu_device)
DECLARE_DEVICE_TYPE(CQUESTROT, cquestrot_cpu_device)
DECLARE_DEVICE_TYPE(CQUESTLIN, cquestlin_cpu_device)


#endif // MAME_CPU_CUBEQCPU_CUBEQCPU_H
