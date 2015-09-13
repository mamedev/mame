// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*** m6800: Portable 6800 class emulator *************************************/

#pragma once

#ifndef __M6800_H__
#define __M6800_H__


enum
{
	M6800_PC=1, M6800_S, M6800_A, M6800_B, M6800_X, M6800_CC,
	M6800_WAI_STATE
};

enum
{
	M6800_IRQ_LINE = 0,             /* IRQ line number */
	M6801_TIN_LINE,                 /* P20/Tin Input Capture line (eddge sense)     */
									/* Active eddge is selecrable by internal reg.  */
									/* raise eddge : CLEAR_LINE  -> ASSERT_LINE     */
									/* fall  eddge : ASSERT_LINE -> CLEAR_LINE      */
									/* it is usuali to use PULSE_LINE state         */
	M6801_SC1_LINE
};

enum
{
	M6801_MODE_0 = 0,
	M6801_MODE_1,
	M6801_MODE_2,
	M6801_MODE_3,
	M6801_MODE_4,
	M6801_MODE_5,
	M6801_MODE_6,
	M6801_MODE_7
};

enum
{
	M6801_PORT1 = 0x100,
	M6801_PORT2,
	M6801_PORT3,
	M6801_PORT4
};


#define MCFG_M6801_SC2(_devcb) \
	m6800_cpu_device::set_out_sc2_func(*device, DEVCB_##_devcb);
#define MCFG_M6801_SER_TX(_devcb) \
	m6800_cpu_device::set_out_sertx_func(*device, DEVCB_##_devcb);

class m6800_cpu_device :  public cpu_device
{
public:
	typedef void (m6800_cpu_device::*op_func)();

	// construction/destruction
	m6800_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	m6800_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, bool has_io, int clock_divider, const m6800_cpu_device::op_func *insn, const UINT8 *cycles, address_map_constructor internal = NULL);

	// static configuration helpers
	template<class _Object> static devcb_base &set_out_sc2_func(device_t &device, _Object object) { return downcast<m6800_cpu_device &>(device).m_out_sc2_func.set_callback(object); }
	template<class _Object> static devcb_base &set_out_sertx_func(device_t &device, _Object object) { return downcast<m6800_cpu_device &>(device).m_out_sertx_func.set_callback(object); }

	DECLARE_READ8_MEMBER( m6801_io_r );
	DECLARE_WRITE8_MEMBER( m6801_io_w );

	DECLARE_WRITE_LINE_MEMBER( irq_line );
	DECLARE_WRITE_LINE_MEMBER( nmi_line );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 12; }
	virtual UINT32 execute_input_lines() const { return 2; }
	virtual UINT32 execute_default_irq_vector() const { return 0; }
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, std::string &str);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	address_space_config m_program_config;
	address_space_config m_decrypted_opcodes_config;
	address_space_config m_io_config;
	bool m_has_io;

	devcb_write_line m_out_sc2_func;
	devcb_write_line m_out_sertx_func;

	PAIR    m_ppc;            /* Previous program counter */
	PAIR    m_pc;             /* Program counter */
	PAIR    m_s;              /* Stack pointer */
	PAIR    m_x;              /* Index register */
	PAIR    m_d;              /* Accumulators */
	UINT8   m_cc;             /* Condition codes */
	UINT8   m_wai_state;      /* WAI opcode state ,(or sleep opcode state) */
	UINT8   m_nmi_state;      /* NMI line state */
	UINT8   m_nmi_pending;    /* NMI pending */
	UINT8   m_irq_state[3];   /* IRQ line state [IRQ1,TIN,SC1] */
	UINT8   m_ic_eddge;       /* InputCapture eddge , b.0=fall,b.1=raise */
	int     m_sc1_state;

	/* Memory spaces */
	address_space *m_program, *m_decrypted_opcodes;
	direct_read_data *m_direct, *m_decrypted_opcodes_direct;
	address_space *m_io;

	const op_func *m_insn;
	const UINT8 *m_cycles;            /* clock cycle of instruction table */
	/* internal registers */
	UINT8   m_port1_ddr;
	UINT8   m_port2_ddr;
	UINT8   m_port3_ddr;
	UINT8   m_port4_ddr;
	UINT8   m_port1_data;
	UINT8   m_port2_data;
	UINT8   m_port3_data;
	UINT8   m_port4_data;
	UINT8   m_p3csr;          // Port 3 Control/Status Register
	UINT8   m_tcsr;           /* Timer Control and Status Register */
	UINT8   m_pending_tcsr;   /* pending IRQ flag for clear IRQflag process */
	UINT8   m_irq2;           /* IRQ2 flags */
	UINT8   m_ram_ctrl;
	PAIR    m_counter;        /* free running counter */
	PAIR    m_output_compare; /* output compare       */
	UINT16  m_input_capture;  /* input capture        */
	int     m_p3csr_is3_flag_read;
	int     m_port3_latched;

	int     m_clock_divider;
	UINT8   m_trcsr, m_rmcr, m_rdr, m_tdr, m_rsr, m_tsr;
	int     m_rxbits, m_txbits, m_txstate, m_trcsr_read_tdre, m_trcsr_read_orfe, m_trcsr_read_rdrf, m_tx, m_ext_serclock;
	bool    m_use_ext_serclock;
	int     m_port2_written;

	int     m_icount;
	int     m_latch09;

	PAIR    m_timer_over;
	emu_timer *m_sci_timer;
	PAIR m_ea;        /* effective address */

	static const UINT8 flags8i[256];
	static const UINT8 flags8d[256];
	static const UINT8 cycles_6800[256];
	static const UINT8 cycles_6803[256];
	static const UINT8 cycles_63701[256];
	static const UINT8 cycles_nsc8105[256];
	static const op_func m6800_insn[256];
	static const op_func m6803_insn[256];
	static const op_func hd63701_insn[256];
	static const op_func nsc8105_insn[256];

	UINT32 RM16(UINT32 Addr );
	void WM16(UINT32 Addr, PAIR *p );
	void enter_interrupt(const char *message,UINT16 irq_vector);
	void m6800_check_irq2();
	void CHECK_IRQ_LINES();
	void check_timer_event();
	void increment_counter(int amount);
	void set_rmcr(UINT8 data);
	void write_port2();
	int m6800_rx();
	void serial_transmit();
	void serial_receive();
	TIMER_CALLBACK_MEMBER( sci_tick );
	void set_os3(int state);

	void aba();
	void abx();
	void adca_di();
	void adca_ex();
	void adca_im();
	void adca_ix();
	void adcb_di();
	void adcb_ex();
	void adcb_im();
	void adcb_ix();
	void adcx_im();
	void adda_di();
	void adda_ex();
	void adda_im();
	void adda_ix();
	void addb_di();
	void addb_ex();
	void addb_im();
	void addb_ix();
	void addd_di();
	void addd_ex();
	void addx_ex();
	void addd_im();
	void addd_ix();
	void aim_di();
	void aim_ix();
	void anda_di();
	void anda_ex();
	void anda_im();
	void anda_ix();
	void andb_di();
	void andb_ex();
	void andb_im();
	void andb_ix();
	void asl_ex();
	void asl_ix();
	void asla();
	void aslb();
	void asld();
	void asr_ex();
	void asr_ix();
	void asra();
	void asrb();
	void bcc();
	void bcs();
	void beq();
	void bge();
	void bgt();
	void bhi();
	void bita_di();
	void bita_ex();
	void bita_im();
	void bita_ix();
	void bitb_di();
	void bitb_ex();
	void bitb_im();
	void bitb_ix();
	void ble();
	void bls();
	void blt();
	void bmi();
	void bne();
	void bpl();
	void bra();
	void brn();
	void bsr();
	void bvc();
	void bvs();
	void cba();
	void clc();
	void cli();
	void clr_ex();
	void clr_ix();
	void clra();
	void clrb();
	void clv();
	void cmpa_di();
	void cmpa_ex();
	void cmpa_im();
	void cmpa_ix();
	void cmpb_di();
	void cmpb_ex();
	void cmpb_im();
	void cmpb_ix();
	void cmpx_di();
	void cmpx_ex();
	void cmpx_im();
	void cmpx_ix();
	void com_ex();
	void com_ix();
	void coma();
	void comb();
	void daa();
	void dec_ex();
	void dec_ix();
	void deca();
	void decb();
	void des();
	void dex();
	void eim_di();
	void eim_ix();
	void eora_di();
	void eora_ex();
	void eora_im();
	void eora_ix();
	void eorb_di();
	void eorb_ex();
	void eorb_im();
	void eorb_ix();
	void illegal();
	void inc_ex();
	void inc_ix();
	void inca();
	void incb();
	void ins();
	void inx();
	void jmp_ex();
	void jmp_ix();
	void jsr_di();
	void jsr_ex();
	void jsr_ix();
	void lda_di();
	void lda_ex();
	void lda_im();
	void lda_ix();
	void ldb_di();
	void ldb_ex();
	void ldb_im();
	void ldb_ix();
	void ldd_di();
	void ldd_ex();
	void ldd_im();
	void ldd_ix();
	void lds_di();
	void lds_ex();
	void lds_im();
	void lds_ix();
	void ldx_di();
	void ldx_ex();
	void ldx_im();
	void ldx_ix();
	void lsr_ex();
	void lsr_ix();
	void lsra();
	void lsrb();
	void lsrd();
	void mul();
	void neg_ex();
	void neg_ix();
	void nega();
	void negb();
	void nop();
	void oim_di();
	void oim_ix();
	void ora_di();
	void ora_ex();
	void ora_im();
	void ora_ix();
	void orb_di();
	void orb_ex();
	void orb_im();
	void orb_ix();
	void psha();
	void pshb();
	void pshx();
	void pula();
	void pulb();
	void pulx();
	void rol_ex();
	void rol_ix();
	void rola();
	void rolb();
	void ror_ex();
	void ror_ix();
	void rora();
	void rorb();
	void rti();
	void rts();
	void sba();
	void sbca_di();
	void sbca_ex();
	void sbca_im();
	void sbca_ix();
	void sbcb_di();
	void sbcb_ex();
	void sbcb_im();
	void sbcb_ix();
	void sec();
	void sei();
	void sev();
	void slp();
	void sta_di();
	void sta_ex();
	void sta_im();
	void sta_ix();
	void stb_di();
	void stb_ex();
	void stb_im();
	void stb_ix();
	void std_di();
	void std_ex();
	void std_im();
	void std_ix();
	void sts_di();
	void sts_ex();
	void sts_im();
	void sts_ix();
	void stx_di();
	void stx_ex();
	void stx_im();
	void stx_ix();
	void suba_di();
	void suba_ex();
	void suba_im();
	void suba_ix();
	void subb_di();
	void subb_ex();
	void subb_im();
	void subb_ix();
	void subd_di();
	void subd_ex();
	void subd_im();
	void subd_ix();
	void swi();
	void tab();
	void tap();
	void tba();
	void tim_di();
	void tim_ix();
	void tpa();
	void tst_ex();
	void tst_ix();
	void tsta();
	void tstb();
	void tsx();
	void txs();
	void undoc1();
	void undoc2();
	void wai();
	void xgdx();
	void cpx_di();
	void cpx_ex();
	void cpx_im();
	void cpx_ix();
	void trap();
};


class m6801_cpu_device : public m6800_cpu_device
{
public:
	m6801_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	m6801_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, const m6800_cpu_device::op_func *insn, const UINT8 *cycles, address_map_constructor internal = NULL);

	void m6801_clock_serial();

protected:
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const { return (clocks + 4 - 1) / 4; }
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const { return (cycles * 4); }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
};


class m6802_cpu_device : public m6800_cpu_device
{
public:
	m6802_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	m6802_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, const m6800_cpu_device::op_func *insn, const UINT8 *cycles);

protected:
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const { return (clocks + 4 - 1) / 4; }
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const { return (cycles * 4); }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
};


class m6803_cpu_device : public m6801_cpu_device
{
public:
	m6803_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
};


class m6808_cpu_device : public m6802_cpu_device
{
public:
	m6808_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
};


class hd6301_cpu_device : public m6801_cpu_device
{
public:
	hd6301_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	hd6301_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

protected:
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
};


class hd63701_cpu_device : public m6801_cpu_device
{
public:
	hd63701_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
};


class nsc8105_cpu_device : public m6802_cpu_device
{
public:
	nsc8105_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
};


// DP-40 package: HD6303RP,  HD63A03RP,  HD63B03RP,
// FP-54 package: HD6303RF,  HD63A03RF,  HD63B03RF,
// CG-40 package: HD6303RCG, HD63A03RCG, HD63B03RCG,
// Not fully emulated yet
class hd6303r_cpu_device : public hd6301_cpu_device
{
public:
	hd6303r_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// DP-64S package: HD6303YP,  HD63A03YP,  HD63B03YP,  HD63C03YP
// FP-64  package: HD6303YF,  HD63A03YF,  HD63B03YF,  HD63C03YF
// FP-64A package: HD6303YH,  HD63A03YH,  HD63B03YH,  HD63C03YH
// CP-68  package: HD6303YCP, HD63A03YCP, HD63B03YCP, HD63C03YCP
// Not fully emulated yet
class hd6303y_cpu_device : public hd6301_cpu_device
{
public:
	hd6303y_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


extern const device_type M6800;
extern const device_type M6801;
extern const device_type M6802;
extern const device_type M6803;
extern const device_type M6808;
extern const device_type HD6301;
extern const device_type HD63701;
extern const device_type NSC8105;
extern const device_type HD6303R;
extern const device_type HD6303Y;

#endif /* __M6800_H__ */
