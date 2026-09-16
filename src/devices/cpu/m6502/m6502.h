// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6502.h

    MOS Technology 6502, original NMOS variant

***************************************************************************/

#ifndef MAME_CPU_M6502_M6502_H
#define MAME_CPU_M6502_M6502_H

#pragma once
//class nes_exrom_device;
class m6502_device : public cpu_device {
public:
typedef device_delegate<void ()> mmc5_reset_scanline_irq_delegate;
typedef device_delegate<void (uint8_t data)> mmc5_register_write_delegate;
	enum {
		IRQ_LINE = INPUT_LINE_IRQ0,
		APU_IRQ_LINE = INPUT_LINE_IRQ1,
		NMI_LINE = INPUT_LINE_NMI,
		V_LINE   = INPUT_LINE_IRQ0 + 16,
		RDY_LINE = INPUT_LINE_IRQ0 + 17
	};
	
	class memory_interface {
	public:
		memory_access<16, 0, 0, ENDIANNESS_LITTLE>::cache cprogram, csprogram;
		memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific program;
		memory_access<14, 0, 0, ENDIANNESS_LITTLE>::specific program14;
		memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_program, m_cprogram, m_csprogram;
		memory_access<14, 0, 0, ENDIANNESS_LITTLE>::specific m_program14, m_cprogram14, m_csprogram14;

		virtual ~memory_interface() = default;
		virtual uint8_t read(uint16_t adr) = 0;
		virtual uint8_t read_9(uint16_t adr);
		virtual uint8_t read_sync(uint16_t adr) = 0;
		virtual uint8_t read_arg(uint16_t adr) = 0;
		virtual void write(uint16_t adr, uint8_t val) = 0;
		virtual void write_9(uint16_t adr, uint8_t val);
	};

	m6502_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_address_width(int width, bool custom_interface) {
		program_config.m_addr_width = width;
		sprogram_config.m_addr_width = width;
		uses_custom_memory_interface = custom_interface;
	}

	void set_custom_memory_interface(std::unique_ptr<memory_interface> interface) {
		mintf = std::move(interface);
	}

	bool get_sync() const { return sync; }

	auto sync_cb() { return sync_w.bind(); }

	devcb_write_line sync_w;
	devcb_write_line &m_sync_w = sync_w;
	mmc5_reset_scanline_irq_delegate m_mmc5_reset_scanline_irq;
	mmc5_register_write_delegate m_mmc5_ppuctrl_write;
	mmc5_register_write_delegate m_mmc5_ppumask_write;
	
	//Added Functions
	bool get_apu_clk1_is_high(); 								//used in m6502.cpp
	bool get_cpu_is_reading(); 									//used in APU
	bool get_rmw_1();											//used in PPU
	bool is_branch_opcode(u8 op);								//used in m6502.cpp
	bool is_irq_oam_dma_window();								//used in m6502.cpp
	bool branch_delay_match_for_new_irq(int state);				//used in m6502.cpp
	bool branch_need_irq_match();								//used in m6502.cpp
	bool dma_window_interrupt_eligible();						//used in m6502.cpp
	uint8_t read_4016_4017(uint16_t adr);						//used in APU
	uint16_t get_adr_bus();										//used in APU
	uint8_t get_open_bus();										//used in APU
	uint8_t get_data_bus();
	void set_data_bus(uint8_t x);
	void set_open_bus(uint8_t x);								//used in APU
	void mark_oam_dma_halt_cycle();								//used in m6502.cpp
	void dmc_halt_next_read(bool a, bool b); 					//used in APU
	void oam_halt_next_read();									//used in APU
	void dmc_clear_halt();										//used in APU
	void oam_clear_halt();										//used in APU
	uint16_t get_prevReadAddress ();							//used in APU
	void handle_dma_rdy_stall();								//used in m6502.cpp
	void run_suspended_cpu_dma_cycle();							//used in APU
	void do_halt();												//Used in .hxx
	void queue_delayed_nmi(int cycles);							//used in PPU
	void cancel_delayed_nmi();									//used in PPU
	void queue_delayed_mapper_irq(int cycles);					//used in MMC3
	void cancel_delayed_mapper_irq();							//used in MMC3
	void queue_delayed_apu_irq(int cycles);						//used in APU
	void cancel_delayed_apu_irq();								//used in APU
	bool get_is_pal() { return is_pal; }						//used in APU
	void set_is_pal(bool x) { is_pal = x; }						//used in APU
	int64_t get_previous_cpu_write_cycle() { return m_previous_cpu_write_cycle; }	//mmc1.cpp
	int64_t	get_last_cpu_write_cycle() { return m_last_cpu_write_cycle; } 	//mmc1.cpp
	//MMC5 Mapper
	void set_m_exram_control(int x);							//used in mmc5.cpp
	//void set_is_mmc5 (bool x);									//used in mmc5.cpp
	template <typename... T>
	void set_mmc5_reset_scanline_irq(T &&... args) {
		m_mmc5_reset_scanline_irq.set(std::forward<T>(args)...);
		m_mmc5_reset_scanline_irq.resolve();
	}

	template <typename... T>
	void set_mmc5_ppuctrl_write(T &&... args) {
		m_mmc5_ppuctrl_write.set(std::forward<T>(args)...);
		m_mmc5_ppuctrl_write.resolve();
	}

	template <typename... T>
	void set_mmc5_ppumask_write(T &&... args) {
		m_mmc5_ppumask_write.set(std::forward<T>(args)...);
		m_mmc5_ppumask_write.resolve();
	}
	
	//MMC3 Clone Mappers
	uint8_t get_last_cpu_write_latch() const { return last_cpu_write_latch; }
	
	//Open Bus 
	void set_open_bus_ranges(const uint32_t *ranges, int count);	//nes_slot.cpp
	bool is_open_bus_address(uint16_t adr) const;				//nes_slot.cpp
	
protected:
	m6502_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	class mi_default : public memory_interface {
	public:
		virtual ~mi_default() = default;
		virtual uint8_t read(uint16_t adr) override;
		virtual uint8_t read_sync(uint16_t adr) override;
		virtual uint8_t read_arg(uint16_t adr) override;
		virtual void write(uint16_t adr, uint8_t val) override;
	};

	class mi_default14 : public mi_default {
	public:
		virtual ~mi_default14() = default;
		virtual uint8_t read(uint16_t adr) override;
		virtual void write(uint16_t adr, uint8_t val) override;
	};

	enum {
		STATE_RESET = 0xff00
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
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual bool cpu_is_interruptible() const override { return true; }
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	
	//void handle_mmc5_vector_read_side_effect();
	//bool handle_repeated_controller_read(uint16_t adr, uint8_t &result);
	//void cache_controller_read_value(uint16_t adr, uint8_t value);
	//void handle_mmc5_ppu_register_write_side_effect(uint16_t adr, uint8_t val);
	//bool handle_controller_write_suppression(uint16_t adr, uint8_t val);
	//bool handle_controller_write_9_suppression(uint16_t adr, uint8_t val);

	address_space_config program_config, sprogram_config;
	
	uint16_t	PPC;                    /* previous program counter */
	uint16_t  	NPC;                    /* next start-of-instruction program counter */
	uint16_t  	PC;                     /* program counter */
	uint16_t  	SP;                     /* stack pointer (always 100 - 1FF) */
	uint16_t  	TMP;                    /* temporary internal values */
	uint8_t   	TMP2;                   /* another temporary internal value, 8 bits this time */
	uint8_t   	A;                      /* Accumulator */
	uint8_t   	X;                      /* X index register */
	uint8_t   	Y;                      /* Y index register */
	uint8_t   	P;                      /* Processor status */
	uint8_t   	IR;                     /* Prefetched instruction register */
	uint16_t  	vec_addr;
	int       	inst_state_base;        /* Current instruction bank */
	
	uint8_t	  	prev_IR;				//track branch delays IRQ
	uint8_t	  	next_IR;				//track branch delays IRQ
	bool 	  	cpu_is_reading;			//APU
	uint8_t 	cpu_data_bus;       	//internal CPU data bus / last CPU-visible value
	uint8_t 	cpu_external_bus;   	//external/open bus latch
	uint16_t 	adr_bus;				//APU
	int 	  	delay;					//CPU to track NMI
	bool 	  	nmi_pending_1;			//PPU sets to tell CPU of an NMI
	bool	  	irq_delay;				//track branch delays IRQ
	bool	  	apu_irq_delay;			//track branch delays IRQ
	bool	  	nmi_delay;				//track branch delays IRQ
	bool	  	branched;				//track branch delays IRQ
	bool 	  	paged;					//track branch delays IRQ
	int64_t   	oam_dma_halt_cycle;		//Track IRQ interrupt DMA
	bool 		rmw_1;					//Track if we are running a RWM OpCode
	bool 		apu_irq_branch_delay;	//track branch delays IRQ
	bool 		irq_branch_delay;		//track branch delays IRQ
	bool 		nmi_branch_delay;		//track branch delays IRQ
	bool 		apu_clk1_is_high;		//Track CPU "get and "put" Cycles
	bool		dmc_halt;				//Need to suspend CPU for DMA
	bool		oam_halt;				//Need to suspend CPU for OAM
	bool		dmc_dma_explicit_stop;	//is this an explicit stop request?
	int 		write_cycles_since_dma_halt_request;			//used to help stop the CPU for DMA
	bool		dmc_dma_reload;			//is this a DMC reload?
	int64_t 	prev_4016_write;		//keep track of strobing the controller and when
	int64_t 	prev_4017_write;		//keep track of strobing the controller and when
	int64_t 	prev_4016_read;			//last controller read cycle
	int64_t 	prev_4017_read;			//last controller read cycle
	uint8_t 	last_4016_val;			//last returned controller value
	uint8_t 	last_4017_val;			//last returned controller value
	bool		inst_halted;			//used in .hxx
	bool		next_read;				//used in .hxx
	bool		prev_next_read; 		//keep next_read from previous cycle
	bool		need_irq;				//used in m6502.cpp
	uint16_t 	prevReadAddress;		//used in m6502.h
	int 		m_exram_control;		//used in m6502.h
	bool		is_mmc5;				//used in m6502.h
	int64_t nmi_cpu_cycle;				//NMI stuff
	bool nmi_overlap_brk_irq;			//NMI stuff
	bool m_real_brk;					//NMI stuff
	uint8_t last_cpu_write_latch;		//mmc3_clone.cpp
	int64_t m_previous_cpu_write_cycle; //mmc1.cpp
	int64_t m_last_cpu_write_cycle;		//mmc1.cpp
	bool mapper_irq;
	int mapper_irq_delay;
	int64_t mapper_irq_cpu_cycle;
	bool apu_irq;
	int apu_irq_delay_apu;
	int64_t apu_irq_cpu_cycle;
	bool pending_apu_irq_cancel;
	static constexpr int MAX_OPEN_BUS_RANGES = 16;		//OpenBus nes_slot.cpp
    uint32_t m_open_bus_ranges[MAX_OPEN_BUS_RANGES];	//OpenBus nes_slot.cpp
    int m_ob_count;										//OpenBus nes_slot.cpp
	bool is_pal;
		
	std::unique_ptr<memory_interface> mintf;
	
	int inst_state, inst_substate;
	int icount, bcount, count_before_instruction_step;
	bool nmi_state, irq_state, apu_irq_state, v_state;
	bool m_rdy_state = true, m_irq_sampled = false, m_nmi_sampled = false;
	bool nmi_pending, irq_taken, sync, inhibit_interrupts;
	bool uses_custom_memory_interface;

	// Compatibility names used by current MAME 6502-derived devices.
	address_space_config &m_program_config = program_config;
	address_space_config &m_sprogram_config = sprogram_config;
	uint16_t &m_PPC = PPC, &m_NPC = NPC, &m_PC = PC, &m_SP = SP, &m_TMP = TMP;
	uint8_t &m_TMP2 = TMP2, &m_A = A, &m_X = X, &m_Y = Y, &m_P = P, &m_IR = IR;
	int &m_inst_state_base = inst_state_base;
	std::unique_ptr<memory_interface> &m_mintf = mintf;
	int &m_inst_state = inst_state, &m_inst_substate = inst_substate;
	int &m_icount = icount, &m_bcount = bcount, &m_count_before_instruction_step = count_before_instruction_step;
	bool &m_nmi_state = nmi_state, &m_irq_state = irq_state, &m_apu_irq_state = apu_irq_state, &m_v_state = v_state;
	bool &m_nmi_pending = nmi_pending, &m_irq_taken = irq_taken, &m_sync = sync, &m_inhibit_interrupts = inhibit_interrupts;
	bool &m_uses_custom_memory_interface = uses_custom_memory_interface;
	
	uint8_t read(uint16_t adr);
	uint8_t read_9(uint16_t adr);
	void write(uint16_t adr, uint8_t val);
	void write_1(uint16_t adr, uint8_t val);
	void write_9(uint16_t adr, uint8_t val);
	uint8_t read_arg(uint16_t adr);
	uint8_t read_pc();
	uint8_t read_pc_noirq() { return read_arg(PC); }
	uint8_t read_arg_noirq(uint16_t adr) { return read_arg(adr); }
	uint8_t read_sync(uint16_t adr);
	void set_var_read();

	void prefetch_start();
	virtual void prefetch_end();
	void prefetch_end_noirq();
	void set_nz(uint8_t v);
	void sample_interrupt() { m_irq_sampled = irq_state || apu_irq_state; m_nmi_sampled = nmi_pending; }

	u32 XPC;
	u32 &m_XPC = XPC;
	virtual offs_t pc_to_external(u16 pc); // For paged PCs
	virtual void do_exec_full();
	virtual void do_exec_partial();

	// inline helpers
	static inline bool page_changing(uint16_t base, int delta) { return ((base + delta) ^ base) & 0xff00; }
	static inline uint16_t set_l(uint16_t base, uint8_t val) { return (base & 0xff00) | val; }
	static inline uint16_t set_h(uint16_t base, uint8_t val) { return (base & 0x00ff) | (val << 8); }

	inline void dec_SP() { SP = set_l(SP, SP-1); }
	inline void inc_SP() { SP = set_l(SP, SP+1); }

	void do_adc_d(uint8_t val);
	void do_adc_nd(uint8_t val);
	void do_sbc_d(uint8_t val);
	void do_sbc_nd(uint8_t val);
	void do_arr_d();
	void do_arr_nd();

	void do_adc(uint8_t val);
	void do_cmp(uint8_t val1, uint8_t val2);
	void do_sbc(uint8_t val);
	void do_bit(uint8_t val);
	void do_arr();
	uint8_t do_asl(uint8_t v);
	uint8_t do_lsr(uint8_t v);
	uint8_t do_ror(uint8_t v);
	uint8_t do_rol(uint8_t v);
	uint8_t do_asr(uint8_t v);

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

class m6512_device : public m6502_device {
public:
	m6512_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
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
	M6502_SET_OVERFLOW = m6502_device::V_LINE
};

DECLARE_DEVICE_TYPE(M6502, m6502_device)
DECLARE_DEVICE_TYPE(M6512, m6512_device)

#endif // MAME_CPU_M6502_M6502_H
