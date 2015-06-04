// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6502.h

    Mostek 6502, original NMOS variant

***************************************************************************/

#ifndef __M6502FAM_H__
#define __M6502FAM_H__

#define MCFG_M6502_DISABLE_DIRECT() \
	downcast<m6502_device *>(device)->disable_direct();

#define MCFG_M6502_SYNC_CALLBACK(_cb) \
	devcb = &m6502_device::set_sync_callback(*device, DEVCB_##_cb);

class m6502_device : public cpu_device {
public:
	enum {
		IRQ_LINE = INPUT_LINE_IRQ0,
		APU_IRQ_LINE = INPUT_LINE_IRQ1,
		NMI_LINE = INPUT_LINE_NMI,
		V_LINE   = INPUT_LINE_IRQ0 + 16
	};

	m6502_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	m6502_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	DECLARE_WRITE_LINE_MEMBER( irq_line );
	DECLARE_WRITE_LINE_MEMBER( nmi_line );

	bool get_sync() const { return sync; }
	void disable_direct() { direct_disabled = true; }

	template<class _Object> static devcb_base &set_sync_callback(device_t &device, _Object object) { return downcast<m6502_device &>(device).sync_w.set_callback(object); }

	devcb_write_line sync_w;

protected:
	class memory_interface {
	public:
		address_space *program, *sprogram;
		direct_read_data *direct, *sdirect;

		virtual ~memory_interface() {}
		virtual UINT8 read(UINT16 adr) = 0;
		virtual UINT8 read_9(UINT16 adr);
		virtual UINT8 read_sync(UINT16 adr) = 0;
		virtual UINT8 read_arg(UINT16 adr) = 0;
		virtual void write(UINT16 adr, UINT8 val) = 0;
		virtual void write_9(UINT16 adr, UINT8 val);
	};

	class mi_default_normal : public memory_interface {
	public:
		virtual ~mi_default_normal() {}
		virtual UINT8 read(UINT16 adr);
		virtual UINT8 read_sync(UINT16 adr);
		virtual UINT8 read_arg(UINT16 adr);
		virtual void write(UINT16 adr, UINT8 val);
	};

	class mi_default_nd : public mi_default_normal {
	public:
		virtual ~mi_default_nd() {}
		virtual UINT8 read_sync(UINT16 adr);
		virtual UINT8 read_arg(UINT16 adr);
	};

	struct disasm_entry {
		const char *opcode;
		int mode;
		offs_t flags;
	};

	enum {
		STATE_RESET = 0xff00,
	};

	enum {
		DASM_non,    /* no additional arguments */
		DASM_aba,    /* absolute */
		DASM_abx,    /* absolute + X */
		DASM_aby,    /* absolute + Y */
		DASM_acc,    /* accumulator */
		DASM_adr,    /* absolute address (jmp,jsr) */
		DASM_bzp,    /* zero page with bit selection */
		DASM_iax,    /* indirect + X (65c02 jmp) */
		DASM_idx,    /* zero page pre indexed */
		DASM_idy,    /* zero page post indexed */
		DASM_idz,    /* zero page post indexed (65ce02) */
		DASM_imm,    /* immediate */
		DASM_imp,    /* implicit */
		DASM_ind,    /* indirect (jmp) */
		DASM_isy,    /* zero page pre indexed sp and post indexed Y (65ce02) */
		DASM_iw2,    /* immediate word (65ce02) */
		DASM_iw3,    /* augment (65ce02) */
		DASM_rel,    /* relative */
		DASM_rw2,    /* relative word (65cs02, 65ce02) */
		DASM_zpb,    /* zero page and branch (65c02 bbr, bbs) */
		DASM_zpg,    /* zero page */
		DASM_zpi,    /* zero page indirect (65c02) */
		DASM_zpx,    /* zero page + X */
		DASM_zpy,    /* zero page + Y */
		DASM_imz,    /* load immediate byte, store to zero page address (M740) */
		DASM_spg,    /* "special page": implied FF00 OR immediate value (M740)*/
		DASM_biz,    /* bit, zero page (M740) */
		DASM_bzr,    /* bit, zero page, relative offset (M740) */
		DASM_bar,    /* bit, accumulator, relative offset (M740) */
		DASM_bac     /* bit, accumulator (M740) */
	};

	enum {
		F_N = 0x80,
		F_V = 0x40,
		F_E = 0x20, // 65ce02
		F_T = 0x20, // M740: replaces A with $00,X in some opcodes when set
		F_B = 0x10,
		F_D = 0x08,
		F_I = 0x04,
		F_Z = 0x02,
		F_C = 0x01
	};

	virtual void init();

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
	virtual void state_import(const device_state_entry &entry);
	virtual void state_export(const device_state_entry &entry);
	virtual void state_string_export(const device_state_entry &entry, std::string &str);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const;
	virtual UINT32 disasm_max_opcode_bytes() const;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	address_space_config program_config, sprogram_config;

	UINT16  PPC;                    /* previous program counter */
	UINT16  NPC;                    /* next start-of-instruction program counter */
	UINT16  PC;                     /* program counter */
	UINT16  SP;                     /* stack pointer (always 100 - 1FF) */
	UINT16  TMP;                    /* temporary internal values */
	UINT8   TMP2;                   /* another temporary internal value, 8 bits this time */
	UINT8   A;                      /* Accumulator */
	UINT8   X;                      /* X index register */
	UINT8   Y;                      /* Y index register */
	UINT8   P;                      /* Processor status */
	UINT8   IR;                     /* Prefetched instruction register */
	int     inst_state_base;        /* Current instruction bank */

	memory_interface *mintf;
	int inst_state, inst_substate;
	int icount;
	bool nmi_state, irq_state, apu_irq_state, v_state;
	bool irq_taken, sync, direct_disabled, inhibit_interrupts;

	static const disasm_entry disasm_entries[0x100];

	offs_t disassemble_generic(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options, const disasm_entry *table);
	UINT8 read(UINT16 adr) { return mintf->read(adr); }
	UINT8 read_9(UINT16 adr) { return mintf->read_9(adr); }
	void write(UINT16 adr, UINT8 val) { mintf->write(adr, val); }
	void write_9(UINT16 adr, UINT8 val) { mintf->write_9(adr, val); }
	UINT8 read_arg(UINT16 adr) { return mintf->read_arg(adr); }
	UINT8 read_pc() { return mintf->read_arg(PC++); }
	UINT8 read_pc_noinc() { return mintf->read_arg(PC); }
	void prefetch();
	void prefetch_noirq();
	void set_nz(UINT8 v);

	virtual void do_exec_full();
	virtual void do_exec_partial();

	// inline helpers
	static inline bool page_changing(UINT16 base, int delta) { return ((base + delta) ^ base) & 0xff00; }
	static inline UINT16 set_l(UINT16 base, UINT8 val) { return (base & 0xff00) | val; }
	static inline UINT16 set_h(UINT16 base, UINT8 val) { return (base & 0x00ff) | (val << 8); }

	inline void dec_SP() { SP = set_l(SP, SP-1); }
	inline void inc_SP() { SP = set_l(SP, SP+1); }

	void do_adc_d(UINT8 val);
	void do_adc_nd(UINT8 val);
	void do_sbc_d(UINT8 val);
	void do_sbc_nd(UINT8 val);
	void do_arr_d();
	void do_arr_nd();

	void do_adc(UINT8 val);
	void do_cmp(UINT8 val1, UINT8 val2);
	void do_sbc(UINT8 val);
	void do_bit(UINT8 val);
	void do_arr();
	UINT8 do_asl(UINT8 v);
	UINT8 do_lsr(UINT8 v);
	UINT8 do_ror(UINT8 v);
	UINT8 do_rol(UINT8 v);
	UINT8 do_asr(UINT8 v);

#define O(o) void o ## _full(); void o ## _partial()

	// NMOS 6502 opcodes
	//   documented opcodes
	O(adc_aba); O(adc_abx); O(adc_aby); O(adc_idx); O(adc_idy); O(adc_imm); O(adc_zpg); O(adc_zpx);
	O(and_aba); O(and_abx); O(and_aby); O(and_imm); O(and_idx); O(and_idy); O(and_zpg); O(and_zpx);
	O(asl_aba); O(asl_abx); O(asl_acc); O(asl_zpg); O(asl_zpx);
	O(bcc_rel);
	O(bcs_rel);
	O(beq_rel);
	O(bit_aba); O(bit_zpg);
	O(bmi_rel);
	O(bne_rel);
	O(bpl_rel);
	O(brk_imp);
	O(bvc_rel);
	O(bvs_rel);
	O(clc_imp);
	O(cld_imp);
	O(cli_imp);
	O(clv_imp);
	O(cmp_aba); O(cmp_abx); O(cmp_aby); O(cmp_idx); O(cmp_idy); O(cmp_imm); O(cmp_zpg); O(cmp_zpx);
	O(cpx_aba); O(cpx_imm); O(cpx_zpg);
	O(cpy_aba); O(cpy_imm); O(cpy_zpg);
	O(dec_aba); O(dec_abx); O(dec_zpg); O(dec_zpx);
	O(dex_imp);
	O(dey_imp);
	O(eor_aba); O(eor_abx); O(eor_aby); O(eor_idx); O(eor_idy); O(eor_imm); O(eor_zpg); O(eor_zpx);
	O(inc_aba); O(inc_abx); O(inc_zpg); O(inc_zpx);
	O(inx_imp);
	O(iny_imp);
	O(jmp_adr); O(jmp_ind);
	O(jsr_adr);
	O(lda_aba); O(lda_abx); O(lda_aby); O(lda_idx); O(lda_idy); O(lda_imm); O(lda_zpg); O(lda_zpx);
	O(ldx_aba); O(ldx_aby); O(ldx_imm); O(ldx_zpg); O(ldx_zpy);
	O(ldy_aba); O(ldy_abx); O(ldy_imm); O(ldy_zpg); O(ldy_zpx);
	O(lsr_aba); O(lsr_abx); O(lsr_acc); O(lsr_zpg); O(lsr_zpx);
	O(nop_imp);
	O(ora_aba); O(ora_abx); O(ora_aby); O(ora_imm); O(ora_idx); O(ora_idy); O(ora_zpg); O(ora_zpx);
	O(pha_imp);
	O(php_imp);
	O(pla_imp);
	O(plp_imp);
	O(rol_aba); O(rol_abx); O(rol_acc); O(rol_zpg); O(rol_zpx);
	O(ror_aba); O(ror_abx); O(ror_acc); O(ror_zpg); O(ror_zpx);
	O(rti_imp);
	O(rts_imp);
	O(sbc_aba); O(sbc_abx); O(sbc_aby); O(sbc_idx); O(sbc_idy); O(sbc_imm); O(sbc_zpg); O(sbc_zpx);
	O(sec_imp);
	O(sed_imp);
	O(sei_imp);
	O(sta_aba); O(sta_abx); O(sta_aby); O(sta_idx); O(sta_idy); O(sta_zpg); O(sta_zpx);
	O(stx_aba); O(stx_zpg); O(stx_zpy);
	O(sty_aba); O(sty_zpg); O(sty_zpx);
	O(tax_imp);
	O(tay_imp);
	O(tsx_imp);
	O(txa_imp);
	O(txs_imp);
	O(tya_imp);

	//   exceptions
	O(reset);

	//   undocumented reliable instructions
	O(dcp_aba); O(dcp_abx); O(dcp_aby); O(dcp_idx); O(dcp_idy); O(dcp_zpg); O(dcp_zpx);
	O(isb_aba); O(isb_abx); O(isb_aby); O(isb_idx); O(isb_idy); O(isb_zpg); O(isb_zpx);
	O(lax_aba); O(lax_aby); O(lax_idx); O(lax_idy); O(lax_zpg); O(lax_zpy);
	O(rla_aba); O(rla_abx); O(rla_aby); O(rla_idx); O(rla_idy); O(rla_zpg); O(rla_zpx);
	O(rra_aba); O(rra_abx); O(rra_aby); O(rra_idx); O(rra_idy); O(rra_zpg); O(rra_zpx);
	O(sax_aba); O(sax_idx); O(sax_zpg); O(sax_zpy);
	O(sbx_imm);
	O(sha_aby); O(sha_idy);
	O(shs_aby);
	O(shx_aby);
	O(shy_abx);
	O(slo_aba); O(slo_abx); O(slo_aby); O(slo_idx); O(slo_idy); O(slo_zpg); O(slo_zpx);
	O(sre_aba); O(sre_abx); O(sre_aby); O(sre_idx); O(sre_idy); O(sre_zpg); O(sre_zpx);

	//   undocumented unreliable instructions
	//     behaviour differs between visual6502 and online docs, which
	//     is a clear sign reliability is not to be expected
	//     implemented version follows visual6502
	O(anc_imm);
	O(ane_imm);
	O(arr_imm);
	O(asr_imm);
	O(las_aby);
	O(lxa_imm);

	//   nop variants
	O(nop_imm); O(nop_aba); O(nop_abx); O(nop_zpg); O(nop_zpx);

	//   system killers
	O(kil_non);

#undef O
};

enum {
	M6502_PC = 1,
	M6502_A,
	M6502_X,
	M6502_Y,
	M6502_P,
	M6502_S,
	M6502_IR
};

enum {
	M6502_IRQ_LINE = m6502_device::IRQ_LINE,
	M6502_NMI_LINE = m6502_device::NMI_LINE,
	M6502_SET_OVERFLOW = m6502_device::V_LINE,
};

extern const device_type M6502;

#endif
