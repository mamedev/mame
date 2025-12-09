// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*****************************************************************************
 *
 *   sh4.h
 *   Portable Hitachi SH-4 (SH7750 family) emulator interface
 *
 *   By R. Belmont, based on sh2.c by Juergen Buchmueller, Mariusz Wojcieszek,
 *      Olivier Galibert, Sylvain Glaize, and James Forshaw.
 *
 *****************************************************************************/

#ifndef MAME_CPU_SH_SH4_H
#define MAME_CPU_SH_SH4_H

#pragma once

#include "sh.h"

#define SH4_INT_NONE    -1
enum
{
	SH4_IRL0=0, SH4_IRL1, SH4_IRL2, SH4_IRL3, SH4_IRLn
};

enum
{
	SH4_R0_BK0 = SH4_EA+1, SH4_R1_BK0, SH4_R2_BK0, SH4_R3_BK0, SH4_R4_BK0, SH4_R5_BK0, SH4_R6_BK0, SH4_R7_BK0,
	SH4_R0_BK1, SH4_R1_BK1, SH4_R2_BK1, SH4_R3_BK1, SH4_R4_BK1, SH4_R5_BK1, SH4_R6_BK1, SH4_R7_BK1,
	SH4_SPC, SH4_SSR, SH4_SGR, SH4_FPSCR, SH4_FPUL, SH4_FR0, SH4_FR1, SH4_FR2, SH4_FR3, SH4_FR4, SH4_FR5,
	SH4_FR6, SH4_FR7, SH4_FR8, SH4_FR9, SH4_FR10, SH4_FR11, SH4_FR12, SH4_FR13, SH4_FR14, SH4_FR15,
	SH4_XF0, SH4_XF1, SH4_XF2, SH4_XF3, SH4_XF4, SH4_XF5, SH4_XF6, SH4_XF7,
	SH4_XF8, SH4_XF9, SH4_XF10, SH4_XF11, SH4_XF12, SH4_XF13, SH4_XF14, SH4_XF15
};

enum
{
	SH4_INTC_NMI=23,
	SH4_INTC_IRLn0,
	SH4_INTC_IRLn1,
	SH4_INTC_IRLn2,
	SH4_INTC_IRLn3,
	SH4_INTC_IRLn4,
	SH4_INTC_IRLn5,
	SH4_INTC_IRLn6,
	SH4_INTC_IRLn7,
	SH4_INTC_IRLn8,
	SH4_INTC_IRLn9,
	SH4_INTC_IRLnA,
	SH4_INTC_IRLnB,
	SH4_INTC_IRLnC,
	SH4_INTC_IRLnD,
	SH4_INTC_IRLnE,

	SH4_INTC_IRL0,
	SH4_INTC_IRL1,
	SH4_INTC_IRL2,
	SH4_INTC_IRL3,

	SH4_INTC_HUDI,
	SH4_INTC_GPOI,

	SH4_INTC_DMTE0,
	SH4_INTC_DMTE1,
	SH4_INTC_DMTE2,
	SH4_INTC_DMTE3,
	SH4_INTC_DMTE4,
	SH4_INTC_DMTE5,
	SH4_INTC_DMTE6,
	SH4_INTC_DMTE7,

	SH4_INTC_DMAE,

	SH4_INTC_TUNI3,
	SH4_INTC_TUNI4,
	SH4_INTC_TUNI0,
	SH4_INTC_TUNI1,
	SH4_INTC_TUNI2,
	SH4_INTC_TICPI2,
	SH4_INTC_ATI,
	SH4_INTC_PRI,
	SH4_INTC_CUI,
	SH4_INTC_SCI1ERI,
	SH4_INTC_SCI1RXI,

	SH4_INTC_SCI1TXI,
	SH4_INTC_SCI1TEI,
	SH4_INTC_SCIFERI,
	SH4_INTC_SCIFRXI,
	SH4_INTC_SCIFBRI,
	SH4_INTC_SCIFTXI,
	SH4_INTC_ITI,
	SH4_INTC_RCMI,
	SH4_INTC_ROVI
};

#define SH4_FPU_PZERO 0
#define SH4_FPU_NZERO 1
#define SH4_FPU_DENORM 2
#define SH4_FPU_NORM 3
#define SH4_FPU_PINF 4
#define SH4_FPU_NINF 5
#define SH4_FPU_qNaN 6
#define SH4_FPU_sNaN 7

enum
{
	SH4_IOPORT_16=8*0,
	SH4_IOPORT_4=8*1,
	SH4_IOPORT_DMA=8*2,
	// future use
	SH4_IOPORT_SCI=8*3,
	SH4_IOPORT_SCIF=8*4
};

struct sh4_device_dma
{
	uint32_t length;
	uint32_t size;
	void *buffer;
	int channel;
};

struct sh4_ddt_dma
{
	uint32_t source;
	uint32_t length;
	uint32_t size;
	uint32_t destination;
	void *buffer;
	int direction;
	int channel;
	int mode;
};


// ASID [7:0] | VPN [31:10] | V |    | PPN [28:10] | SZ[1:0] | SH | C | PR[1:0] | D | WT | SA[2:0] | TC

struct sh4_utlb
{
	uint8_t ASID;
	uint32_t VPN;
	uint8_t V;
	uint32_t PPN;
	uint8_t PSZ;
	uint8_t SH;
	uint8_t C;
	uint8_t PPR;
	uint8_t D;
	uint8_t WT;
	uint8_t SA;
	uint8_t TC;
};


typedef void (*sh4_ftcsr_callback)(uint32_t);

class sh4_frontend;

class sh34_base_device : public sh_common_execution
{
public:

	void set_md(int bit, int md) { m_md[bit] = md; }
	void set_sh4_clock(int clock) { m_clock = clock; }
	void set_sh4_clock(const XTAL &xtal) { set_sh4_clock(xtal.value()); }

	void set_mmu_hacktype(int hacktype) { m_mmuhack = hacktype; }

	TIMER_CALLBACK_MEMBER(sh4_timer_callback);
	TIMER_CALLBACK_MEMBER(sh4_dmac_callback);

	virtual void set_frt_input(int state) override;
	void sh4_set_irln_input(int value);
	//void sh4_set_ftcsr_callback(sh4_ftcsr_callback callback);
	int sh4_dma_data(struct sh4_device_dma *s);
	void sh4_dma_ddt(struct sh4_ddt_dma *s);

	// DRC C-substitute ops
	void func_STCRBANK();
	void func_TRAPA();
	void func_LDCSR();
	void func_LDCMSR();
	void func_RTE();
	void func_SHAD();
	void func_SHLD();
	void func_CHECKIRQ();
	void func_LDCRBANK();
	void func_STCMSPC();
	void func_LDCMSPC();
	void func_STCMSSR();
	void func_LDCMSSR();
	void func_STCMRBANK();
	void func_LDCMRBANK();
	void func_PREFM();
	void func_FADD();
	void func_FADD_spre();
	void func_FADD_spost();
	void func_FSUB();
	void func_FMUL();
	void func_FDIV();
	void func_FCMP_EQ();
	void func_FCMP_GT();
	void func_LDSFPSCR();
	void func_LDCDBR();
	void func_FMOVMRIFR();
	void func_FRCHG();
	void func_FSCHG();
	void func_LDSMFPUL();
	void func_LDSMFPSCR();
	void func_FMOVFRMDR();
	void func_LDCSSR();
	void func_STSFPSCR();
	void func_FLDI0();
	void func_FLDI1();
	void func_FMOVFR();
	void func_FMOVFRS0();
	void func_FTRC();
	void func_FMOVMRFR();
	void func_FMOVS0FR();
	void func_STSFPUL();
	void func_FMOVFRMR();
	void func_LDSFPUL();
	void func_FLOAT();
	void func_STSMFPSCR();
	void func_STSMFPUL();
	void func_FNEG();
	void func_FMAC();
	void func_FABS();
	void func_FLDS();
	void func_FTRV();
	void func_FSTS();
	void func_FSSCA();
	void func_FCNVSD();
	void func_FIPR();
	void func_FSRRA();
	void func_FSQRT();
	void func_FCNVDS();
	void func_LDCMDBR();
	void func_STCMDBR();
	void func_LDCSPC();
	void func_STCMSGR();
	void func_STCDBR();
	void func_STCSGR();
	void func_SETS();
	void func_CLRS();
	void func_LDTLB();
	void func_MOVCAL();
	void func_STCSSR();
	void func_STCSPC();

protected:
	// construction/destruction
	sh34_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness, address_map_constructor internal);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 4; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI; }

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t& address, address_space*& target_space) override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	address_space_config m_program_config;
	address_space_config m_io_config;

	uml::parameter m_fs_regmap[16];
	uml::parameter m_fd_regmap[16];

	int m_md[9];
	int m_clock;

	// hack 1 = Naomi hack, hack 2 = WIP implementation
	int m_mmuhack;

	uint32_t  m_exception_priority[128];
	int     m_exception_requesting[128];

	int8_t    m_irq_line_state[17];
	address_space *m_internal;
	address_space *m_program;
	address_space *m_io;

	// CCN
	uint32_t m_pteh;
	uint32_t m_ptel;
	uint32_t m_ttb;
	uint32_t m_tea;
	uint32_t m_mmucr;
	uint8_t m_basra;
	uint8_t m_basrb;
	uint32_t m_ccr;
	uint32_t m_tra;
	uint32_t m_expevt;
	uint32_t m_intevt;

	// CCN 7709S
	uint32_t m_ccr2;

	// CCN 7091
	uint32_t m_ptea;
	uint32_t m_qacr0;
	uint32_t m_qacr1;

	// TMU
	uint8_t m_tocr;
	uint8_t m_tstr;
	uint32_t m_tcor0;
	uint32_t m_tcnt0;
	uint16_t m_tcr0;
	uint32_t m_tcor1;
	uint32_t m_tcnt1;
	uint16_t m_tcr1;
	uint32_t m_tcor2;
	uint32_t m_tcnt2;
	uint16_t m_tcr2;
	uint32_t m_tcpr2;

	// INTC
	uint16_t m_icr;
	uint16_t m_ipra;
	uint16_t m_iprc;

	// INTC 7709
	uint32_t m_intevt2;

	// DMAC
	uint32_t m_sar0;
	uint32_t m_dar0;
	uint32_t m_dmatcr0;
	uint32_t m_chcr0;
	uint32_t m_sar1;
	uint32_t m_dar1;
	uint32_t m_dmatcr1;
	uint32_t m_chcr1;
	uint32_t m_sar2;
	uint32_t m_dar2;
	uint32_t m_dmatcr2;
	uint32_t m_chcr2;
	uint32_t m_sar3;
	uint32_t m_dar3;
	uint32_t m_dmatcr3;
	uint32_t m_chcr3;
	uint32_t m_dmaor;

	int8_t    m_nmi_line_state;

	int     m_irln;
	int     m_internal_irq_level;
	int     m_internal_irq_vector;

	emu_timer *m_dma_timer[4];
	emu_timer *m_timer[3];
	int     m_dma_timer_active[4];
	uint32_t  m_dma_source[4];
	uint32_t  m_dma_destination[4];
	uint32_t  m_dma_count[4];
	int     m_dma_wordsize[4];
	int     m_dma_source_increment[4];
	int     m_dma_destination_increment[4];
	int     m_dma_mode[4];

	int     m_is_slave;
	int     m_cpu_clock;
	int     m_bus_clock;
	int     m_pm_clock;
	int     m_ioport16_pullup;
	int     m_ioport16_direction;
	int     m_ioport4_pullup;
	int     m_ioport4_direction;

	int     m_willjump;

	//void    (*m_ftcsr_read_callback)(uint32_t data);

	bool m_sh4_mmu_enabled;

	uint64_t m_debugger_temp;

	inline void sh4_check_pending_irq(const char *message) // look for highest priority active exception and handle it
	{
		m_willjump = 0; // for the DRC

		int irq = 0;
		int z = -1;
		for (int a = 0; a <= SH4_INTC_ROVI; a++)
		{
			if (m_exception_requesting[a])
			{
				if ((int)m_exception_priority[a] > z)
				{
					z = m_exception_priority[a];
					irq = a;
				}
			}
		}
		if (z >= 0)
		{
			sh4_exception(message, irq);
		}
	}

	void sh4_change_register_bank(int to);
	void sh4_swap_fp_registers();
	void sh4_swap_fp_couples();
	void sh4_syncronize_register_bank(int to);
	void sh4_default_exception_priorities();
	void sh4_exception_recompute();
	void sh4_exception_request(int exception);
	void sh4_exception_unrequest(int exception);
	void sh4_exception_checkunrequest(int exception);
	void sh4_exception_process(int exception, uint32_t vector);
	void sh4_exception(const char *message, int exception);
	void sh4_dmac_nmi();
	uint16_t ipra_r(offs_t offset, uint16_t mem_mask);
	void ipra_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	virtual uint32_t get_remap(uint32_t address);
	virtual uint32_t sh4_getsqremap(uint32_t address);
	void sh4_parse_configuration();
	void sh4_timer_recompute(int which);
	uint8_t tocr_r(offs_t offset, uint8_t mem_mask);
	void tocr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t tstr_r(offs_t offset, uint8_t mem_mask);
	void tstr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint32_t tcor0_r(offs_t offset, uint32_t mem_mask);
	void tcor0_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t tcnt0_r(offs_t offset, uint32_t mem_mask);
	void tcnt0_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint16_t tcr0_r(offs_t offset, uint16_t mem_mask);
	void tcr0_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint32_t tcor1_r(offs_t offset, uint32_t mem_mask);
	void tcor1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t tcnt1_r(offs_t offset, uint32_t mem_mask);
	void tcnt1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint16_t tcr1_r(offs_t offset, uint16_t mem_mask);
	void tcr1_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint32_t tcor2_r(offs_t offset, uint32_t mem_mask);
	void tcor2_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t tcnt2_r(offs_t offset, uint32_t mem_mask);
	void tcnt2_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint16_t tcr2_r(offs_t offset, uint16_t mem_mask);
	void tcr2_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint32_t tcpr2_r(offs_t offset, uint32_t mem_mask);
	void tcpr2_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	int sh4_dma_transfer(int channel, int timermode, uint32_t chcr, uint32_t *sar, uint32_t *dar, uint32_t *dmatcr);
	int sh4_dma_transfer_device(int channel, uint32_t chcr, uint32_t *sar, uint32_t *dar, uint32_t *dmatcr);
	void sh4_dmac_check(int channel);
	uint32_t sar0_r(offs_t offset, uint32_t mem_mask);
	void sar0_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t dar0_r(offs_t offset, uint32_t mem_mask);
	void dar0_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t dmatcr0_r(offs_t offset, uint32_t mem_mask);
	void dmatcr0_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t chcr0_r(offs_t offset, uint32_t mem_mask);
	void chcr0_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t sar1_r(offs_t offset, uint32_t mem_mask);
	void sar1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t dar1_r(offs_t offset, uint32_t mem_mask);
	void dar1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t dmatcr1_r(offs_t offset, uint32_t mem_mask);
	void dmatcr1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t chcr1_r(offs_t offset, uint32_t mem_mask);
	void chcr1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t sar2_r(offs_t offset, uint32_t mem_mask);
	void sar2_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t dar2_r(offs_t offset, uint32_t mem_mask);
	void dar2_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t dmatcr2_r(offs_t offset, uint32_t mem_mask);
	void dmatcr2_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t chcr2_r(offs_t offset, uint32_t mem_mask);
	void chcr2_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t sar3_r(offs_t offset, uint32_t mem_mask);
	void sar3_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t dar3_r(offs_t offset, uint32_t mem_mask);
	void dar3_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t dmatcr3_r(offs_t offset, uint32_t mem_mask);
	void dmatcr3_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t chcr3_r(offs_t offset, uint32_t mem_mask);
	void chcr3_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t dmaor_r(offs_t offset, uint32_t mem_mask);
	void dmaor_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	// memory handlers
	virtual uint8_t read_byte(offs_t offset) override;
	virtual uint16_t read_word(offs_t offset) override;
	virtual uint32_t read_long(offs_t offset) override;
	virtual uint16_t decrypted_read_word(offs_t offset) override;
	virtual void write_byte(offs_t offset, uint8_t data) override;
	virtual void write_word(offs_t offset, uint16_t data) override;
	virtual void write_long(offs_t offset, uint32_t data) override;

	// regular handlers for opcodes which need to differ on sh3/4 due to different interrupt / exception handling and register banking
	virtual void LDCSR(const uint16_t opcode) override;
	virtual void LDCMSR(const uint16_t opcode) override;
	virtual void RTE() override;
	virtual void TRAPA(uint32_t i) override;
	virtual void ILLEGAL() override;

	// regular handelrs for sh3/4 specific opcodes
	virtual void LDTLB(const uint16_t opcode);
	void TODO(const uint16_t opcode);
	void MOVCAL(const uint16_t opcode);
	void CLRS(const uint16_t opcode);
	void SETS(const uint16_t opcode);
	void STCRBANK(const uint16_t opcode);
	void STCMRBANK(const uint16_t opcode);
	void STCSSR(const uint16_t opcode);
	void STCSPC(const uint16_t opcode);
	void STCSGR(const uint16_t opcode);
	void STSFPUL(const uint16_t opcode);
	void STSFPSCR(const uint16_t opcode);
	void STCDBR(const uint16_t opcode);
	void STCMSGR(const uint16_t opcode);
	void STSMFPUL(const uint16_t opcode);
	void STSMFPSCR(const uint16_t opcode);
	void STCMDBR(const uint16_t opcode);
	void STCMSSR(const uint16_t opcode);
	void STCMSPC(const uint16_t opcode);
	void LDSMFPUL(const uint16_t opcode);
	void LDSMFPSCR(const uint16_t opcode);
	void LDCMDBR(const uint16_t opcode);
	void LDCMRBANK(const uint16_t opcode);
	void LDCMSSR(const uint16_t opcode);
	void LDCMSPC(const uint16_t opcode);
	void LDSFPUL(const uint16_t opcode);
	void LDSFPSCR(const uint16_t opcode);
	void LDCDBR(const uint16_t opcode);
	void SHAD(const uint16_t opcode);
	void SHLD(const uint16_t opcode);
	void LDCRBANK(const uint16_t opcode);
	void LDCSSR(const uint16_t opcode);
	void LDCSPC(const uint16_t opcode);
	void PREFM(const uint16_t opcode);
	void FMOVMRIFR(const uint16_t opcode);
	void FMOVFRMR(const uint16_t opcode);
	void FMOVFRMDR(const uint16_t opcode);
	void FMOVFRS0(const uint16_t opcode);
	void FMOVS0FR(const uint16_t opcode);
	void FMOVMRFR(const uint16_t opcode);
	void FMOVFR(const uint16_t opcode);
	void FLDI1(const uint16_t opcode);
	void FLDI0(const uint16_t opcode);
	void FLDS(const uint16_t opcode);
	void FSTS(const uint16_t opcode);
	void FRCHG();
	void FSCHG();
	void FTRC(const uint16_t opcode);
	void FLOAT(const uint16_t opcode);
	void FNEG(const uint16_t opcode);
	void FABS(const uint16_t opcode);
	void FCMP_EQ(const uint16_t opcode);
	void FCMP_GT(const uint16_t opcode);
	void FCNVDS(const uint16_t opcode);
	void FCNVSD(const uint16_t opcode);
	void FADD(const uint16_t opcode);
	void FSUB(const uint16_t opcode);
	void FMUL(const uint16_t opcode);
	void FDIV(const uint16_t opcode);
	void FMAC(const uint16_t opcode);
	void FSQRT(const uint16_t opcode);
	void FSRRA(const uint16_t opcode);
	void FSSCA(const uint16_t opcode);
	void FIPR(const uint16_t opcode);
	void FTRV(const uint16_t opcode);

	void op1111_0xf13(const uint16_t opcode);
	void dbreak(const uint16_t opcode);
	void op1111_0x13(uint16_t opcode);

	// group dispatchers to allow for new opcodes
	virtual void execute_one_0000(uint16_t opcode) override;
	virtual void execute_one_4000(uint16_t opcode) override;
	virtual void execute_one_f000(uint16_t opcode) override;

	// DRC related parts

	virtual void generate_update_cycles(drcuml_block &block, compiler_state &compiler, uml::parameter param, bool allow_exception) override;

	// code generators for sh3/4 specific opcodes
	bool generate_group_0_STCRBANK(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_0_STCSSR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_0_STCSPC(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_0_PREFM(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_0_MOVCAL(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_0_LDTLB(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_0_CLRS(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_0_SETS(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_0_STCSGR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_0_STSFPUL(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_0_STSFPSCR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_0_STCDBR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);

	bool generate_group_4_SHAD(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_4_SHLD(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_4_LDCRBANK(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_4_STCMRBANK(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_4_LDCMRBANK(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_4_STCMSGR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_4_STCMSSR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_4_LDCMSSR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_4_LDCSSR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_4_STCMSPC(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_4_LDCMSPC(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_4_LDCSPC(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_4_STSMFPUL(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_4_LDSMFPUL(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_4_LDSFPUL(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_4_STSMFPSCR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_4_LDSMFPSCR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_4_LDSFPSCR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_4_STCMDBR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_4_LDCMDBR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_4_LDCDBR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);

	bool generate_group_15_FADD(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_FSUB(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_FMUL(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_FDIV(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_FCMP_EQ(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_FCMP_GT(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_FMOVS0FR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_FMOVFRS0(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_FMOVMRFR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_FMOVMRIFR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_FMOVFRMR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_s_0xf13_FTRVlot, uint32_t ovrpc);
	bool generate_group_15_FMOVFRMDR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_FMOVFR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_op1111_0x13(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_FMAC(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);

	bool generate_group_15_op1111_0x13_FSTS(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_op1111_0x13_FLDS(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_op1111_0x13_FLOAT(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_op1111_0x13_FTRC(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_op1111_0x13_FNEG(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_op1111_0x13_FABS(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_op1111_0x13_FSQRT(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_op1111_0x13_FSRRA(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_op1111_0x13_FLDI0(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_op1111_0x13_FLDI1(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_op1111_0x13_FCNVSD(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_op1111_0x13_FCNVDS(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_op1111_0x13_FIPR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_op1111_0x13_op1111_0xf13(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);

	bool generate_group_15_op1111_0x13_op1111_0xf13_FSCHG(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_op1111_0x13_op1111_0xf13_FRCHG(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_op1111_0x13_op1111_0xf13_FTRV(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_15_op1111_0x13_op1111_0xf13_FSSCA(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);

	// code generators for opcodes which need to differ on sh3/4 due to different interrupt / exception handling and register banking
	virtual bool generate_group_0_RTE(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc) override;
	virtual bool generate_group_4_LDCSR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc) override;
	virtual bool generate_group_4_LDCMSR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc) override;
	virtual bool generate_group_12_TRAPA(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc) override;

	// group generators to allow for new opcodes
	virtual bool generate_group_0(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc) override;
	virtual bool generate_group_4(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc) override;
	virtual bool generate_group_15(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc) override;

	virtual void static_generate_entry_point() override;
	virtual void static_generate_memory_accessor(int size, int iswrite, const char *name, uml::code_handle *&handleptr) override;

private:
	bool            m_bigendian;
};


class sh3_base_device : public sh34_base_device
{
	friend class sh4_frontend;

protected:
	// construction/destruction
	sh3_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void sh3_internal_map(address_map &map) ATTR_COLD;
	virtual void sh3_register_map(address_map& map) ATTR_COLD {}

	void ccn_map(address_map& map) ATTR_COLD;
	void ccn_7709s_map(address_map& map) ATTR_COLD;
	void ubc_map(address_map& map) ATTR_COLD;
	void ubc_7709s_map(address_map& map) ATTR_COLD;
	void cpg_map(address_map& map) ATTR_COLD;
	void cpg_7709_map(address_map& map) ATTR_COLD;
	void bsc_map(address_map& map) ATTR_COLD;
	void bsc_7708_map(address_map& map) ATTR_COLD;
	void bsc_7709_map(address_map& map) ATTR_COLD;
	void bsc_7709s_map(address_map& map) ATTR_COLD;
	void rtc_map(address_map& map) ATTR_COLD;
	void intc_map(address_map& map) ATTR_COLD;
	void intc_7709_map(address_map& map) ATTR_COLD;
	void dmac_7709_map(address_map& map) ATTR_COLD;
	void tmu_map(address_map& map) ATTR_COLD;
	void sci_7708_map(address_map& map) ATTR_COLD;
	void sci_7709_map(address_map& map) ATTR_COLD;
	void cmt_7709_map(address_map& map) ATTR_COLD;
	void ad_7709_map(address_map& map) ATTR_COLD;
	void da_7709_map(address_map& map) ATTR_COLD;
	void port_7709_map(address_map& map) ATTR_COLD;
	void irda_7709_map(address_map& map) ATTR_COLD;
	void scif_7709_map(address_map& map) ATTR_COLD;
	void udi_7709s_map(address_map& map) ATTR_COLD;

	// CCN
	uint32_t pteh_r(offs_t offset, uint32_t mem_mask);
	void pteh_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t ptel_r(offs_t offset, uint32_t mem_mask);
	void ptel_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t ttb_r(offs_t offset, uint32_t mem_mask);
	void ttb_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t tea_r(offs_t offset, uint32_t mem_mask);
	void tea_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t mmucr_r(offs_t offset, uint32_t mem_mask);
	void mmucr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint8_t basra_r(offs_t offset, uint8_t mem_mask);
	void basra_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t basrb_r(offs_t offset, uint8_t mem_mask);
	void basrb_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint32_t ccr_r(offs_t offset, uint32_t mem_mask);
	void ccr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t tra_r(offs_t offset, uint32_t mem_mask);
	void tra_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t expevt_r(offs_t offset, uint32_t mem_mask);
	void expevt_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t intevt_r(offs_t offset, uint32_t mem_mask);
	void intevt_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	// CCN 7709S
	uint32_t ccr2_r(offs_t offset, uint32_t mem_mask);
	void ccr2_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	// UBC
	uint32_t bara_r(offs_t offset, uint32_t mem_mask);
	void bara_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint8_t bamra_r(offs_t offset, uint8_t mem_mask);
	void bamra_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint16_t bbra_r(offs_t offset, uint16_t mem_mask);
	void bbra_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint32_t barb_r(offs_t offset, uint32_t mem_mask);
	void barb_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint8_t bamrb_r(offs_t offset, uint8_t mem_mask);
	void bamrb_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint16_t bbrb_r(offs_t offset, uint16_t mem_mask);
	void bbrb_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint32_t bdrb_r(offs_t offset, uint32_t mem_mask);
	void bdrb_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t bdmrb_r(offs_t offset, uint32_t mem_mask);
	void bdmrb_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint16_t brcr_r(offs_t offset, uint16_t mem_mask);
	void brcr_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// UBC 7709S
	uint16_t betr_r(offs_t offset, uint16_t mem_mask);
	void betr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint32_t brsr_r(offs_t offset, uint32_t mem_mask);
	void brsr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t brdr_r(offs_t offset, uint32_t mem_mask);
	void brdr_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	// CPG
	uint16_t frqcr_r(offs_t offset, uint16_t mem_mask);
	void frqcr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t stbcr_r(offs_t offset, uint8_t mem_mask);
	void stbcr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t wtcnt_r(offs_t offset, uint8_t mem_mask);
	void wtcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t wtcsr_r(offs_t offset, uint8_t mem_mask);
	void wtcsr_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// CPG 7709
	uint8_t stbcr2_r(offs_t offset, uint8_t mem_mask);
	void stbcr2_w(offs_t offset, uint8_t data, uint8_t mem_mask);

	// BSC
	uint16_t bcr1_r(offs_t offset, uint16_t mem_mask);
	void bcr1_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t bcr2_r(offs_t offset, uint16_t mem_mask);
	void bcr2_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t wcr1_r(offs_t offset, uint16_t mem_mask);
	void wcr1_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t wcr2_r(offs_t offset, uint16_t mem_mask);
	void wcr2_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t mcr_r(offs_t offset, uint16_t mem_mask);
	void mcr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t pcr_r(offs_t offset, uint16_t mem_mask);
	void pcr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t rtcsr_r(offs_t offset, uint16_t mem_mask);
	void rtcsr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t rtcnt_r(offs_t offset, uint16_t mem_mask);
	void rtcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t rtcor_r(offs_t offset, uint16_t mem_mask);
	void rtcor_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t rfcr_r(offs_t offset, uint16_t mem_mask);
	void rfcr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t sdmr_r(offs_t offset, uint8_t mem_mask);
	void sdmr_w(offs_t offset, uint8_t data, uint8_t mem_mask);

	// BSC 7708
	uint16_t dcr_r(offs_t offset, uint16_t mem_mask);
	void dcr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t pctr_r(offs_t offset, uint16_t mem_mask);
	void pctr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t pdtr_r(offs_t offset, uint16_t mem_mask);
	void pdtr_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// BSC 7709
	uint16_t bcr3_r(offs_t offset, uint16_t mem_mask);
	void bcr3_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// BSC 7709S
	uint16_t mcscr0_r(offs_t offset, uint16_t mem_mask);
	void mcscr0_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t mcscr1_r(offs_t offset, uint16_t mem_mask);
	void mcscr1_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t mcscr2_r(offs_t offset, uint16_t mem_mask);
	void mcscr2_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t mcscr3_r(offs_t offset, uint16_t mem_mask);
	void mcscr3_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t mcscr4_r(offs_t offset, uint16_t mem_mask);
	void mcscr4_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t mcscr5_r(offs_t offset, uint16_t mem_mask);
	void mcscr5_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t mcscr6_r(offs_t offset, uint16_t mem_mask);
	void mcscr6_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t mcscr7_r(offs_t offset, uint16_t mem_mask);
	void mcscr7_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// RTC
	uint8_t r64cnt_r(offs_t offset, uint8_t mem_mask);
	uint8_t rseccnt_r(offs_t offset, uint8_t mem_mask);
	void rseccnt_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rmincnt_r(offs_t offset, uint8_t mem_mask);
	void rmincnt_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rhrcnt_r(offs_t offset, uint8_t mem_mask);
	void rhrcnt_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rwkcnt_r(offs_t offset, uint8_t mem_mask);
	void rwkcnt_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rdaycnt_r(offs_t offset, uint8_t mem_mask);
	void rdaycnt_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rmoncnt_r(offs_t offset, uint8_t mem_mask);
	void rmoncnt_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t ryrcnt_r(offs_t offset, uint8_t mem_mask);
	void ryrcnt_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rsecar_r(offs_t offset, uint8_t mem_mask);
	void rsecar_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rminar_r(offs_t offset, uint8_t mem_mask);
	void rminar_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rhrar_r(offs_t offset, uint8_t mem_mask);
	void rhrar_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rwkar_r(offs_t offset, uint8_t mem_mask);
	void rwkar_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rdayar_r(offs_t offset, uint8_t mem_mask);
	void rdayar_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rmonar_r(offs_t offset, uint8_t mem_mask);
	void rmonar_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rcr1_r(offs_t offset, uint8_t mem_mask);
	void rcr1_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rcr2_r(offs_t offset, uint8_t mem_mask);
	void rcr2_w(offs_t offset, uint8_t data, uint8_t mem_mask);

	// INTC
	uint16_t icr0_r(offs_t offset, uint16_t mem_mask);
	void icr0_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t iprb_r(offs_t offset, uint16_t mem_mask);
	void iprb_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// INTC 7709
	uint32_t intevt2_r(offs_t offset, uint32_t mem_mask);
	void intevt2_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint8_t irr0_r(offs_t offset, uint8_t mem_mask);
	void irr0_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t irr1_r(offs_t offset, uint8_t mem_mask);
	void irr1_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t irr2_r(offs_t offset, uint8_t mem_mask);
	void irr2_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint16_t icr1_r(offs_t offset, uint16_t mem_mask);
	void icr1_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t icr2_r(offs_t offset, uint16_t mem_mask);
	void icr2_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t pinter_r(offs_t offset, uint16_t mem_mask);
	void pinter_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t iprc_r(offs_t offset, uint16_t mem_mask);
	void iprc_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t iprd_r(offs_t offset, uint16_t mem_mask);
	void iprd_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t ipre_r(offs_t offset, uint16_t mem_mask);
	void ipre_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// SCI
	uint8_t scsmr_r(offs_t offset, uint8_t mem_mask);
	void scsmr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t scbrr_r(offs_t offset, uint8_t mem_mask);
	void scbrr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t scscr_r(offs_t offset, uint8_t mem_mask);
	void scscr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t sctdr_r(offs_t offset, uint8_t mem_mask);
	void sctdr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t scssr_r(offs_t offset, uint8_t mem_mask);
	void scssr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t scrdr_r(offs_t offset, uint8_t mem_mask);
	void scscmr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t scscmr_r(offs_t offset, uint8_t mem_mask);

	// SCI 7708
	uint8_t scsptr_r(offs_t offset, uint8_t mem_mask);
	void scsptr_w(offs_t offset, uint8_t data, uint8_t mem_mask);

	// CMT 7709
	uint16_t cmstr_r(offs_t offset, uint16_t mem_mask);
	void cmstr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t cmscr_r(offs_t offset, uint16_t mem_mask);
	void cmscr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t cmcnt_r(offs_t offset, uint16_t mem_mask);
	void cmcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t cmcor_r(offs_t offset, uint16_t mem_mask);
	void cmcor_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// AD 7709
	uint8_t addrah_r(offs_t offset, uint8_t mem_mask);
	void addrah_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t addral_r(offs_t offset, uint8_t mem_mask);
	void addral_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t addrbh_r(offs_t offset, uint8_t mem_mask);
	void addrbh_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t addrbl_r(offs_t offset, uint8_t mem_mask);
	void addrbl_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t addrch_r(offs_t offset, uint8_t mem_mask);
	void addrch_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t addrcl_r(offs_t offset, uint8_t mem_mask);
	void addrcl_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t addrdh_r(offs_t offset, uint8_t mem_mask);
	void addrdh_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t addrdl_r(offs_t offset, uint8_t mem_mask);
	void addrdl_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t adcsr_r(offs_t offset, uint8_t mem_mask);
	void adcsr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t adcr_r(offs_t offset, uint8_t mem_mask);
	void adcr_w(offs_t offset, uint8_t data, uint8_t mem_mask);

	// DA 7709
	uint8_t dadr0_r(offs_t offset, uint8_t mem_mask);
	void dadr0_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t dadr1_r(offs_t offset, uint8_t mem_mask);
	void dadr1_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t dadcr_r(offs_t offset, uint8_t mem_mask);
	void dadcr_w(offs_t offset, uint8_t data, uint8_t mem_mask);

	// PORT 7709
	uint16_t pacr_r(offs_t offset, uint16_t mem_mask);
	void pacr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t pbcr_r(offs_t offset, uint16_t mem_mask);
	void pbcr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t pccr_r(offs_t offset, uint16_t mem_mask);
	void pccr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t pdcr_r(offs_t offset, uint16_t mem_mask);
	void pdcr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t pecr_r(offs_t offset, uint16_t mem_mask);
	void pecr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t pfcr_r(offs_t offset, uint16_t mem_mask);
	void pfcr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t pgcr_r(offs_t offset, uint16_t mem_mask);
	void pgcr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t phcr_r(offs_t offset, uint16_t mem_mask);
	void phcr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t pjcr_r(offs_t offset, uint16_t mem_mask);
	void pjcr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t pkcr_r(offs_t offset, uint16_t mem_mask);
	void pkcr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t plcr_r(offs_t offset, uint16_t mem_mask);
	void plcr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t scpcr_r(offs_t offset, uint16_t mem_mask);
	void scpcr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t padr_r(offs_t offset, uint8_t mem_mask);
	void padr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t pbdr_r(offs_t offset, uint8_t mem_mask);
	void pbdr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t pcdr_r(offs_t offset, uint8_t mem_mask);
	void pcdr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t pddr_r(offs_t offset, uint8_t mem_mask);
	void pddr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t pedr_r(offs_t offset, uint8_t mem_mask);
	void pedr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t pfdr_r(offs_t offset, uint8_t mem_mask);
	void pfdr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t pgdr_r(offs_t offset, uint8_t mem_mask);
	void pgdr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t phdr_r(offs_t offset, uint8_t mem_mask);
	void phdr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t pjdr_r(offs_t offset, uint8_t mem_mask);
	void pjdr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t pkdr_r(offs_t offset, uint8_t mem_mask);
	void pkdr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t pldr_r(offs_t offset, uint8_t mem_mask);
	void pldr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t scpdr_r(offs_t offset, uint8_t mem_mask);
	void scpdr_w(offs_t offset, uint8_t data, uint8_t mem_mask);

	// IRDA 7709
	uint8_t scsmr1_r(offs_t offset, uint8_t mem_mask);
	void scsmr1_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t scbrr1_r(offs_t offset, uint8_t mem_mask);
	void scbrr1_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t scscr1_r(offs_t offset, uint8_t mem_mask);
	void scscr1_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t scftdr1_r(offs_t offset, uint8_t mem_mask);
	void scftdr1_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint16_t scssr1_r(offs_t offset, uint16_t mem_mask);
	void scssr1_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t scfrdr1_r(offs_t offset, uint8_t mem_mask);
	void scfrdr1_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t scfcr1_r(offs_t offset, uint8_t mem_mask);
	void scfcr1_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint16_t scfdr1_r(offs_t offset, uint16_t mem_mask);
	void scfdr1_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// SCIF 7709
	uint8_t scsmr2_r(offs_t offset, uint8_t mem_mask);
	void scsmr2_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t scbrr2_r(offs_t offset, uint8_t mem_mask);
	void scbrr2_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t scscr2_r(offs_t offset, uint8_t mem_mask);
	void scscr2_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t scftdr2_r(offs_t offset, uint8_t mem_mask);
	void scftdr2_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint16_t scssr2_r(offs_t offset, uint16_t mem_mask);
	void scssr2_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t scfrdr2_r(offs_t offset, uint8_t mem_mask);
	void scfrdr2_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t scfcr2_r(offs_t offset, uint8_t mem_mask);
	void scfcr2_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint16_t scfdr2_r(offs_t offset, uint16_t mem_mask);
	void scfdr2_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// UDI 7709S
	uint16_t sdir_r(offs_t offset, uint16_t mem_mask);
	void sdir_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	std::unique_ptr<sh4_frontend>      m_drcfe;                  /* pointer to the DRC front-end state */
	virtual const opcode_desc* get_desclist(offs_t pc) override;
	virtual void init_drc_frontend() override;

	// UBC
	uint32_t m_bara;
	uint8_t m_bamra;
	uint16_t m_bbra;
	uint32_t m_barb;
	uint8_t m_bamrb;
	uint16_t m_bbrb;
	uint32_t m_bdrb;
	uint32_t m_bdmrb;
	uint16_t m_brcr;

	// UBC 7709S
	uint16_t m_betr;
	uint32_t m_brsr;
	uint32_t m_brdr;

	// CPG
	uint16_t m_frqcr;
	uint8_t m_stbcr;
	uint16_t m_wtcnt;
	uint16_t m_wtcsr;

	// CPG 7709
	uint8_t m_stbcr2;

	// BSC
	uint16_t m_bcr1;
	uint16_t m_bcr2;
	uint16_t m_wcr1;
	uint16_t m_wcr2;
	uint16_t m_mcr;
	uint16_t m_pcr;
	uint16_t m_rtcsr;
	uint16_t m_rtcnt;
	uint16_t m_rtcor;
	uint16_t m_rfcr;

	// BSC 7708
	uint16_t m_dcr;
	uint16_t m_pctr;
	uint16_t m_pdtr;

	// BSC 7709
	uint16_t m_bcr3;

	// BSC 7709S
	uint16_t m_mcscr0;
	uint16_t m_mcscr1;
	uint16_t m_mcscr2;
	uint16_t m_mcscr3;
	uint16_t m_mcscr4;
	uint16_t m_mcscr5;
	uint16_t m_mcscr6;
	uint16_t m_mcscr7;

	// RTC
	uint8_t m_r64cnt;
	uint8_t m_rseccnt;
	uint8_t m_rmincnt;
	uint8_t m_rhrcnt;
	uint8_t m_rwkcnt;
	uint8_t m_rmoncnt;
	uint8_t m_rdaycnt;
	uint8_t m_ryrcnt;
	uint8_t m_rsecar;
	uint8_t m_rminar;
	uint8_t m_rhrar;
	uint8_t m_rwkar;
	uint8_t m_rdayar;
	uint8_t m_rmonar;
	uint8_t m_rcr1;
	uint8_t m_rcr2;

	// INTC
	uint16_t m_icr0;
	uint16_t m_iprb;

	// INTC 7709
	uint8_t m_irr0;
	uint8_t m_irr1;
	uint8_t m_irr2;
	uint16_t m_icr1;
	uint16_t m_icr2;
	uint16_t m_pinter;
	uint16_t m_iprd;
	uint16_t m_ipre;

	// SCI
	uint8_t m_scsmr;
	uint8_t m_scbrr;
	uint8_t m_scscr;
	uint8_t m_sctdr;
	uint8_t m_scssr;
	uint8_t m_scrdr;
	uint8_t m_scscmr;

	// SCI 7709
	uint8_t m_scsptr;

	// CMT 7709
	uint16_t m_cmstr;
	uint16_t m_cmscr;
	uint16_t m_cmcnt;
	uint16_t m_cmcor;

	// AD 7709
	uint8_t m_addrah;
	uint8_t m_addral;
	uint8_t m_addrbh;
	uint8_t m_addrbl;
	uint8_t m_addrch;
	uint8_t m_addrcl;
	uint8_t m_addrdh;
	uint8_t m_addrdl;
	uint8_t m_adcsr;
	uint8_t m_adcr;

	// DA 7709
	uint8_t m_dadr0;
	uint8_t m_dadr1;
	uint8_t m_dadcr;

	// PORT 7709
	uint16_t m_pacr;
	uint16_t m_pbcr;
	uint16_t m_pccr;
	uint16_t m_pdcr;
	uint16_t m_pecr;
	uint16_t m_pfcr;
	uint16_t m_pgcr;
	uint16_t m_phcr;
	uint16_t m_pjcr;
	uint16_t m_pkcr;
	uint16_t m_plcr;
	uint16_t m_scpcr;
	uint8_t m_padr;
	uint8_t m_pbdr;
	uint8_t m_pcdr;
	uint8_t m_pddr;
	uint8_t m_pedr;
	uint8_t m_pfdr;
	uint8_t m_pgdr;
	uint8_t m_phdr;
	uint8_t m_pjdr;
	uint8_t m_pkdr;
	uint8_t m_pldr;
	uint8_t m_scpdr;

	// IRDA 7709
	uint8_t m_scsmr1;
	uint8_t m_scbrr1;
	uint8_t m_scscr1;
	uint8_t m_scftdr1;
	uint16_t m_scssr1;
	uint8_t m_scfrdr1;
	uint8_t m_scfcr1;
	uint16_t m_scfdr1;

	// SCIF 7709
	uint8_t m_scsmr2;
	uint8_t m_scbrr2;
	uint8_t m_scscr2;
	uint8_t m_scftdr2;
	uint16_t m_scssr2;
	uint16_t m_scfrdr2;
	uint8_t m_scfcr2;
	uint8_t m_scfdr2;

	// UDI 7709S
	uint16_t m_sdir;
};


class sh4_base_device : public sh34_base_device
{
protected:
	friend class sh4_frontend;

	// construction/destruction
	sh4_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void init_drc_frontend() override;
	std::unique_ptr<sh4_frontend>      m_drcfe;                  /* pointer to the DRC front-end state */
	virtual const opcode_desc* get_desclist(offs_t pc) override;

	void sh4_internal_map(address_map& map) ATTR_COLD;
	virtual void sh4_register_map(address_map& map) ATTR_COLD {}

	void ccn_map(address_map &map) ATTR_COLD;
	void ubc_map(address_map& map) ATTR_COLD;
	void bsc_map(address_map& map) ATTR_COLD;
	void bsc_7750r_map(address_map& map) ATTR_COLD;
	void dmac_map(address_map& map) ATTR_COLD;
	void dmac_7750r_map(address_map& map) ATTR_COLD;
	void cpg_map(address_map& map) ATTR_COLD;
	void cpg_7750r_map(address_map& map) ATTR_COLD;
	void rtc_map(address_map& map) ATTR_COLD;
	void rtc_7750r_map(address_map& map) ATTR_COLD;
	void intc_map(address_map& map) ATTR_COLD;
	void intc_7750s_map(address_map& map) ATTR_COLD;
	void intc_7750r_map(address_map& map) ATTR_COLD;
	void tmu_map(address_map& map) ATTR_COLD;
	void tmu_7750r_map(address_map& map) ATTR_COLD;
	void sci_map(address_map &map) ATTR_COLD;
	void scif_map(address_map &map) ATTR_COLD;
	void hudi_map(address_map& map) ATTR_COLD;
	void hudi_7750r_map(address_map& map) ATTR_COLD;
	void pci_7751_map(address_map& map) ATTR_COLD;

	uint64_t sh4_utlb_address_array_r(offs_t offset);
	void sh4_utlb_address_array_w(offs_t offset, uint64_t data);
	uint64_t sh4_utlb_data_array1_r(offs_t offset);
	void sh4_utlb_data_array1_w(offs_t offset, uint64_t data);
	uint64_t sh4_utlb_data_array2_r(offs_t offset);
	void sh4_utlb_data_array2_w(offs_t offset, uint64_t data);

	// CCN
	uint32_t pteh_r(offs_t offset, uint32_t mem_mask);
	void pteh_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t ptel_r(offs_t offset, uint32_t mem_mask);
	void ptel_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t ttb_r(offs_t offset, uint32_t mem_mask);
	void ttb_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t tea_r(offs_t offset, uint32_t mem_mask);
	void tea_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t mmucr_r(offs_t offset, uint32_t mem_mask);
	void mmucr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint8_t basra_r(offs_t offset, uint8_t mem_mask);
	void basra_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t basrb_r(offs_t offset, uint8_t mem_mask);
	void basrb_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint32_t ccr_r(offs_t offset, uint32_t mem_mask);
	void ccr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t tra_r(offs_t offset, uint32_t mem_mask);
	void tra_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t expevt_r(offs_t offset, uint32_t mem_mask);
	void expevt_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t intevt_r(offs_t offset, uint32_t mem_mask);
	void intevt_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t ptea_r(offs_t offset, uint32_t mem_mask);
	void ptea_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t qacr0_r(offs_t offset, uint32_t mem_mask);
	void qacr0_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t qacr1_r(offs_t offset, uint32_t mem_mask);
	void qacr1_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	// UBC
	uint32_t bara_r(offs_t offset, uint32_t mem_mask);
	void bara_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint8_t bamra_r(offs_t offset, uint8_t mem_mask);
	void bamra_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint16_t bbra_r(offs_t offset, uint16_t mem_mask);
	void bbra_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint32_t barb_r(offs_t offset, uint32_t mem_mask);
	void barb_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint8_t bamrb_r(offs_t offset, uint8_t mem_mask);
	void bamrb_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint16_t bbrb_r(offs_t offset, uint16_t mem_mask);
	void bbrb_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint32_t bdrb_r(offs_t offset, uint32_t mem_mask);
	void bdrb_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t bdmrb_r(offs_t offset, uint32_t mem_mask);
	void bdmrb_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint16_t brcr_r(offs_t offset, uint16_t mem_mask);
	void brcr_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// BSC
	uint32_t bcr1_r(offs_t offset, uint32_t mem_mask);
	void bcr1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint16_t bcr2_r(offs_t offset, uint16_t mem_mask);
	void bcr2_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint32_t wcr1_r(offs_t offset, uint32_t mem_mask);
	void wcr1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t wcr2_r(offs_t offset, uint32_t mem_mask);
	void wcr2_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t wcr3_r(offs_t offset, uint32_t mem_mask);
	void wcr3_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t mcr_r(offs_t offset, uint32_t mem_mask);
	void mcr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint16_t pcr_r(offs_t offset, uint16_t mem_mask);
	void pcr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t rtcsr_r(offs_t offset, uint16_t mem_mask);
	void rtcsr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t rtcnt_r(offs_t offset, uint16_t mem_mask);
	void rtcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t rtcor_r(offs_t offset, uint16_t mem_mask);
	void rtcor_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t rfcr_r(offs_t offset, uint16_t mem_mask);
	void rfcr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint32_t pctra_r(offs_t offset, uint32_t mem_mask);
	void pctra_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint16_t pdtra_r(offs_t offset, uint16_t mem_mask);
	void pdtra_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint32_t pctrb_r(offs_t offset, uint32_t mem_mask);
	void pctrb_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint16_t pdtrb_r(offs_t offset, uint16_t mem_mask);
	void pdtrb_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t gpioic_r(offs_t offset, uint16_t mem_mask);
	void gpioic_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void sdmr2_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	void sdmr3_w(offs_t offset, uint8_t data, uint8_t mem_mask);

	// BSC 7750R
	uint16_t bcr3_r(offs_t offset, uint16_t mem_mask);
	void bcr3_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint32_t bcr4_r(offs_t offset, uint32_t mem_mask);
	void bcr4_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	// DMAC 7750R
	uint32_t sar4_r(offs_t offset, uint32_t mem_mask);
	void sar4_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t dar4_r(offs_t offset, uint32_t mem_mask);
	void dar4_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t dmatcr4_r(offs_t offset, uint32_t mem_mask);
	void dmatcr4_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t chcr4_r(offs_t offset, uint32_t mem_mask);
	void chcr4_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t sar5_r(offs_t offset, uint32_t mem_mask);
	void sar5_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t dar5_r(offs_t offset, uint32_t mem_mask);
	void dar5_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t dmatcr5_r(offs_t offset, uint32_t mem_mask);
	void dmatcr5_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t chcr5_r(offs_t offset, uint32_t mem_mask);
	void chcr5_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t sar6_r(offs_t offset, uint32_t mem_mask);
	void sar6_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t dar6_r(offs_t offset, uint32_t mem_mask);
	void dar6_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t dmatcr6_r(offs_t offset, uint32_t mem_mask);
	void dmatcr6_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t chcr6_r(offs_t offset, uint32_t mem_mask);
	void chcr6_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t sar7_r(offs_t offset, uint32_t mem_mask);
	void sar7_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t dar7_r(offs_t offset, uint32_t mem_mask);
	void dar7_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t dmatcr7_r(offs_t offset, uint32_t mem_mask);
	void dmatcr7_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t chcr7_r(offs_t offset, uint32_t mem_mask);
	void chcr7_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	// CPG
	uint16_t frqcr_r(offs_t offset, uint16_t mem_mask);
	void frqcr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t stbcr_r(offs_t offset, uint8_t mem_mask);
	void stbcr_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t wtcnt_r(offs_t offset, uint8_t mem_mask);
	void wtcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t wtcsr_r(offs_t offset, uint8_t mem_mask);
	void wtcsr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t stbcr2_r(offs_t offset, uint8_t mem_mask);
	void stbcr2_w(offs_t offset, uint8_t data, uint8_t mem_mask);

	// CPG 7750R
	uint32_t clkstp00_r(offs_t offset, uint32_t mem_mask);
	void clkstp00_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void clkstpclr_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	// RTC
	uint8_t r64cnt_r(offs_t offset, uint8_t mem_mask);
	uint8_t rseccnt_r(offs_t offset, uint8_t mem_mask);
	void rseccnt_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rmincnt_r(offs_t offset, uint8_t mem_mask);
	void rmincnt_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rhrcnt_r(offs_t offset, uint8_t mem_mask);
	void rhrcnt_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rwkcnt_r(offs_t offset, uint8_t mem_mask);
	void rwkcnt_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rdaycnt_r(offs_t offset, uint8_t mem_mask);
	void rdaycnt_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rmoncnt_r(offs_t offset, uint8_t mem_mask);
	void rmoncnt_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint16_t ryrcnt_r(offs_t offset, uint16_t mem_mask);
	void ryrcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t rsecar_r(offs_t offset, uint8_t mem_mask);
	void rsecar_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rminar_r(offs_t offset, uint8_t mem_mask);
	void rminar_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rhrar_r(offs_t offset, uint8_t mem_mask);
	void rhrar_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rwkar_r(offs_t offset, uint8_t mem_mask);
	void rwkar_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rdayar_r(offs_t offset, uint8_t mem_mask);
	void rdayar_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rmonar_r(offs_t offset, uint8_t mem_mask);
	void rmonar_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rcr1_r(offs_t offset, uint8_t mem_mask);
	void rcr1_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t rcr2_r(offs_t offset, uint8_t mem_mask);
	void rcr2_w(offs_t offset, uint8_t data, uint8_t mem_mask);

	// RTC 7750R
	uint8_t rcr3_r(offs_t offset, uint8_t mem_mask);
	void rcr3_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint16_t ryrar_r(offs_t offset, uint16_t mem_mask);
	void ryrar_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// INTC
	uint16_t icr_r(offs_t offset, uint16_t mem_mask);
	void icr_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t iprb_r(offs_t offset, uint16_t mem_mask);
	void iprb_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t iprc_r(offs_t offset, uint16_t mem_mask);
	void iprc_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// INTC 7750S
	uint16_t iprd_r(offs_t offset, uint16_t mem_mask);
	void iprd_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// INTC 7750R
	uint32_t intpri00_r(offs_t offset, uint32_t mem_mask);
	void intpri00_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t intreq00_r(offs_t offset, uint32_t mem_mask);
	void intreq00_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t intmsk00_r(offs_t offset, uint32_t mem_mask);
	void intmsk00_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void intmskclr_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	// TMU 7750R
	uint8_t tstr2_r(offs_t offset, uint8_t mem_mask);
	void tstr2_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint32_t tcor3_r(offs_t offset, uint32_t mem_mask);
	void tcor3_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t tcnt3_r(offs_t offset, uint32_t mem_mask);
	void tcnt3_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint16_t tcr3_r(offs_t offset, uint16_t mem_mask);
	void tcr3_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint32_t tcor4_r(offs_t offset, uint32_t mem_mask);
	void tcor4_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t tcnt4_r(offs_t offset, uint32_t mem_mask);
	void tcnt4_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint16_t tcr4_r(offs_t offset, uint16_t mem_mask);
	void tcr4_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// SCI
	uint8_t scsmr1_r(offs_t offset, uint8_t mem_mask);
	void scsmr1_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t scbrr1_r(offs_t offset, uint8_t mem_mask);
	void scbrr1_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t scscr1_r(offs_t offset, uint8_t mem_mask);
	void scscr1_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t sctdr1_r(offs_t offset, uint8_t mem_mask);
	void sctdr1_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t scssr1_r(offs_t offset, uint8_t mem_mask);
	void scssr1_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t scrdr1_r(offs_t offset, uint8_t mem_mask);
	void scscmr1_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t scscmr1_r(offs_t offset, uint8_t mem_mask);
	uint8_t scsptr1_r(offs_t offset, uint8_t mem_mask);
	void scsptr1_w(offs_t offset, uint8_t data, uint8_t mem_mask);

	// SCIF
	uint16_t scsmr2_r(offs_t offset, uint16_t mem_mask);
	void scsmr2_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t scbrr2_r(offs_t offset, uint8_t mem_mask);
	void scbrr2_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint16_t scscr2_r(offs_t offset, uint16_t mem_mask);
	void scscr2_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t scftdr2_r(offs_t offset, uint8_t mem_mask);
	void scftdr2_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint16_t scfsr2_r(offs_t offset, uint16_t mem_mask);
	void scfsr2_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t scfrdr2_r(offs_t offset, uint8_t mem_mask);
	uint16_t scfcr2_r(offs_t offset, uint16_t mem_mask);
	void scfcr2_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t scfdr2_r(offs_t offset, uint16_t mem_mask);
	uint16_t scsptr2_r(offs_t offset, uint16_t mem_mask);
	void scsptr2_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t sclsr2_r(offs_t offset, uint16_t mem_mask);
	void sclsr2_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// H-UDI
	uint16_t sdir_r(offs_t offset, uint16_t mem_mask);
	void sdir_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint32_t sddr_r(offs_t offset, uint32_t mem_mask);
	void sddr_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	// H-UDI 7750R
	uint16_t sdint_r(offs_t offset, uint16_t mem_mask);
	void sdint_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// PCI 7751
	uint32_t pciconf0_r(offs_t offset, uint32_t mem_mask);
	uint32_t pciconf1_r(offs_t offset, uint32_t mem_mask);
	void pciconf1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pciconf2_r(offs_t offset, uint32_t mem_mask);
	void pciconf2_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pciconf3_r(offs_t offset, uint32_t mem_mask);
	void pciconf3_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pciconf4_r(offs_t offset, uint32_t mem_mask);
	void pciconf4_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pciconf5_r(offs_t offset, uint32_t mem_mask);
	void pciconf5_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pciconf6_r(offs_t offset, uint32_t mem_mask);
	void pciconf6_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pciconf7_r(offs_t offset, uint32_t mem_mask);
	uint32_t pciconf8_r(offs_t offset, uint32_t mem_mask);
	uint32_t pciconf9_r(offs_t offset, uint32_t mem_mask);
	uint32_t pciconf10_r(offs_t offset, uint32_t mem_mask);
	uint32_t pciconf11_r(offs_t offset, uint32_t mem_mask);
	void pciconf11_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pciconf12_r(offs_t offset, uint32_t mem_mask);
	uint32_t pciconf13_r(offs_t offset, uint32_t mem_mask);
	uint32_t pciconf14_r(offs_t offset, uint32_t mem_mask);
	uint32_t pciconf15_r(offs_t offset, uint32_t mem_mask);
	void pciconf15_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pciconf16_r(offs_t offset, uint32_t mem_mask);
	void pciconf16_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pciconf17_r(offs_t offset, uint32_t mem_mask);
	void pciconf17_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcicr_r(offs_t offset, uint32_t mem_mask);
	void pcicr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcilsr0_r(offs_t offset, uint32_t mem_mask);
	void pcilsr0_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcilsr1_r(offs_t offset, uint32_t mem_mask);
	void pcilsr1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcilar0_r(offs_t offset, uint32_t mem_mask);
	void pcilar0_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcilar1_r(offs_t offset, uint32_t mem_mask);
	void pcilar1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pciint_r(offs_t offset, uint32_t mem_mask);
	void pciint_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pciintm_r(offs_t offset, uint32_t mem_mask);
	void pciintm_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcialr_r(offs_t offset, uint32_t mem_mask);
	void pcialr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pciclr_r(offs_t offset, uint32_t mem_mask);
	void pciclr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pciaint_r(offs_t offset, uint32_t mem_mask);
	void pciaint_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pciaintm_r(offs_t offset, uint32_t mem_mask);
	void pciaintm_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcibllr_r(offs_t offset, uint32_t mem_mask);
	void pcibllr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcidmabt_r(offs_t offset, uint32_t mem_mask);
	void pcidmabt_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcidpa0_r(offs_t offset, uint32_t mem_mask);
	void pcidpa0_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcidla0_r(offs_t offset, uint32_t mem_mask);
	void pcidla0_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcidtc0_r(offs_t offset, uint32_t mem_mask);
	void pcidtc0_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcidcr0_r(offs_t offset, uint32_t mem_mask);
	void pcidcr0_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcidpa1_r(offs_t offset, uint32_t mem_mask);
	void pcidpa1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcidla1_r(offs_t offset, uint32_t mem_mask);
	void pcidla1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcidtc1_r(offs_t offset, uint32_t mem_mask);
	void pcidtc1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcidcr1_r(offs_t offset, uint32_t mem_mask);
	void pcidcr1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcidpa2_r(offs_t offset, uint32_t mem_mask);
	void pcidpa2_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcidla2_r(offs_t offset, uint32_t mem_mask);
	void pcidla2_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcidtc2_r(offs_t offset, uint32_t mem_mask);
	void pcidtc2_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcidcr2_r(offs_t offset, uint32_t mem_mask);
	void pcidcr2_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcidpa3_r(offs_t offset, uint32_t mem_mask);
	void pcidpa3_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcidla3_r(offs_t offset, uint32_t mem_mask);
	void pcidla3_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcidtc3_r(offs_t offset, uint32_t mem_mask);
	void pcidtc3_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcidcr3_r(offs_t offset, uint32_t mem_mask);
	void pcidcr3_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcipar_r(offs_t offset, uint32_t mem_mask);
	void pcipar_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcimbr_r(offs_t offset, uint32_t mem_mask);
	void pcimbr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pciiobr_r(offs_t offset, uint32_t mem_mask);
	void pciiobr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcipint_r(offs_t offset, uint32_t mem_mask);
	void pcipint_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcipintm_r(offs_t offset, uint32_t mem_mask);
	void pcipintm_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pciclkr_r(offs_t offset, uint32_t mem_mask);
	void pciclkr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcibcr1_r(offs_t offset, uint32_t mem_mask);
	void pcibcr1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcibcr2_r(offs_t offset, uint32_t mem_mask);
	void pcibcr2_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcibcr3_r(offs_t offset, uint32_t mem_mask);
	void pcibcr3_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pciwcr1_r(offs_t offset, uint32_t mem_mask);
	void pciwcr1_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pciwcr2_r(offs_t offset, uint32_t mem_mask);
	void pciwcr2_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pciwcr3_r(offs_t offset, uint32_t mem_mask);
	void pciwcr3_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcimcr_r(offs_t offset, uint32_t mem_mask);
	void pcimcr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcipctr_r(offs_t offset, uint32_t mem_mask);
	void pcipctr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcipdtr_r(offs_t offset, uint32_t mem_mask);
	void pcipdtr_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t pcipdr_r(offs_t offset, uint32_t mem_mask);
	void pcipdr_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	TIMER_CALLBACK_MEMBER(sh4_rtc_timer_callback);
	TIMER_CALLBACK_MEMBER(sh4_refresh_timer_callback);
	void increment_rtc_time(int mode);
	uint32_t compute_ticks_refresh_timer(emu_timer* timer, int hertz, int base, int divisor);
	void sh4_refresh_timer_recompute();
	virtual void LDTLB(const uint16_t opcode) override;

	virtual uint32_t get_remap(uint32_t address) override;
	virtual uint32_t sh4_getsqremap(uint32_t address) override;

	emu_timer* m_rtc_timer;
	emu_timer* m_refresh_timer;
	uint32_t  m_refresh_timer_base;
	sh4_utlb m_utlb[64];

	// UBC
	uint32_t m_bara;
	uint8_t m_bamra;
	uint16_t m_bbra;
	uint32_t m_barb;
	uint8_t m_bamrb;
	uint16_t m_bbrb;
	uint32_t m_bdrb;
	uint32_t m_bdmrb;
	uint16_t m_brcr;

	// BSC
	uint32_t m_bcr1;
	uint16_t m_bcr2;
	uint32_t m_wcr1;
	uint32_t m_wcr2;
	uint32_t m_wcr3;
	uint32_t m_mcr;
	uint16_t m_pcr;
	uint16_t m_rtcsr;
	uint16_t m_rtcnt;
	uint16_t m_rtcor;
	uint16_t m_rfcr;
	uint32_t m_pctra;
	uint16_t m_pdtra;
	uint32_t m_pctrb;
	uint16_t m_pdtrb;
	uint16_t m_gpioic;

	// BSC 7750R
	uint32_t m_bcr3;
	uint32_t m_bcr4;

	// DMAC 7750R
	uint32_t m_sar4;
	uint32_t m_dar4;
	uint32_t m_dmatcr4;
	uint32_t m_chcr4;
	uint32_t m_sar5;
	uint32_t m_dar5;
	uint32_t m_dmatcr5;
	uint32_t m_chcr5;
	uint32_t m_sar6;
	uint32_t m_dar6;
	uint32_t m_dmatcr6;
	uint32_t m_chcr6;
	uint32_t m_sar7;
	uint32_t m_dar7;
	uint32_t m_dmatcr7;
	uint32_t m_chcr7;

	// CPG
	uint16_t m_frqcr;
	uint8_t m_stbcr;
	uint16_t m_wtcnt;
	uint16_t m_wtcsr;
	uint8_t m_stbcr2;

	// CPG 7750R
	uint32_t m_clkstp00;

	// RTC
	uint8_t m_r64cnt;
	uint8_t m_rseccnt;
	uint8_t m_rmincnt;
	uint8_t m_rhrcnt;
	uint8_t m_rwkcnt;
	uint8_t m_rmoncnt;
	uint8_t m_rdaycnt;
	uint16_t m_ryrcnt;
	uint8_t m_rsecar;
	uint8_t m_rminar;
	uint8_t m_rhrar;
	uint8_t m_rwkar;
	uint8_t m_rdayar;
	uint8_t m_rmonar;
	uint8_t m_rcr1;
	uint8_t m_rcr2;

	// RTC 7750R
	uint8_t m_rcr3;
	uint16_t m_ryrar;

	// INTC
	uint16_t m_iprb;

	// INTC 7750S
	uint16_t m_iprd;

	// INTC 7750R
	uint32_t m_intpri00;
	uint32_t m_intreq00;
	uint32_t m_intmsk00;

	// TMU 7750R
	uint8_t m_tstr2;
	uint32_t m_tcor3;
	uint32_t m_tcnt3;
	uint16_t m_tcr3;
	uint32_t m_tcor4;
	uint32_t m_tcnt4;
	uint16_t m_tcr4;

	// SCI
	uint8_t m_scsmr1;
	uint8_t m_scbrr1;
	uint8_t m_scscr1;
	uint8_t m_sctdr1;
	uint8_t m_scssr1;
	uint8_t m_scrdr1;
	uint8_t m_scscmr1;
	uint8_t m_scsptr1;

	// SCIF
	uint16_t m_scsmr2;
	uint8_t m_scbrr2;
	uint16_t m_scscr2;
	uint8_t m_scftdr2;
	uint16_t m_scfsr2;
	uint8_t m_scfrdr2;
	uint16_t m_scfcr2;
	uint16_t m_scfdr2;
	uint16_t m_scsptr2;
	uint16_t m_sclsr2;

	// H-UDI
	uint16_t m_sdir;
	uint32_t m_sddr;
	uint16_t m_sdint;

	// PCI 7751
	uint32_t m_pciconf0;
	uint32_t m_pciconf1;
	uint32_t m_pciconf2;
	uint32_t m_pciconf3;
	uint32_t m_pciconf4;
	uint32_t m_pciconf5;
	uint32_t m_pciconf6;
	uint32_t m_pciconf7;
	uint32_t m_pciconf8;
	uint32_t m_pciconf9;
	uint32_t m_pciconf10;
	uint32_t m_pciconf11;
	uint32_t m_pciconf12;
	uint32_t m_pciconf13;
	uint32_t m_pciconf14;
	uint32_t m_pciconf15;
	uint32_t m_pciconf16;
	uint32_t m_pciconf17;
	uint32_t m_pcicr;
	uint32_t m_pcilsr0;
	uint32_t m_pcilsr1;
	uint32_t m_pcilar0;
	uint32_t m_pcilar1;
	uint32_t m_pciint;
	uint32_t m_pciintm;
	uint32_t m_pcialr;
	uint32_t m_pciclr;
	uint32_t m_pciaint;
	uint32_t m_pciaintm;
	uint32_t m_pcibllr;
	uint32_t m_pcidmabt;
	uint32_t m_pcidpa0;
	uint32_t m_pcidla0;
	uint32_t m_pcidtc0;
	uint32_t m_pcidcr0;
	uint32_t m_pcidpa1;
	uint32_t m_pcidla1;
	uint32_t m_pcidtc1;
	uint32_t m_pcidcr1;
	uint32_t m_pcidpa2;
	uint32_t m_pcidla2;
	uint32_t m_pcidtc2;
	uint32_t m_pcidcr2;
	uint32_t m_pcidpa3;
	uint32_t m_pcidla3;
	uint32_t m_pcidtc3;
	uint32_t m_pcidcr3;
	uint32_t m_pcipar;
	uint32_t m_pcimbr;
	uint32_t m_pciiobr;
	uint32_t m_pcipint;
	uint32_t m_pcipintm;
	uint32_t m_pciclkr;
	uint32_t m_pcibcr1;
	uint32_t m_pcibcr2;
	uint32_t m_pcibcr3;
	uint32_t m_pciwcr1;
	uint32_t m_pciwcr2;
	uint32_t m_pciwcr3;
	uint32_t m_pcimcr;
	uint32_t m_pcipctr;
	uint32_t m_pcipdtr;
	uint32_t m_pcipdr;
};


class sh3_device : public sh3_base_device
{
public:
	sh3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness = ENDIANNESS_LITTLE);

	virtual void sh3_register_map(address_map& map) override ATTR_COLD;
};

class sh7708s_device : public sh3_base_device
{
public:
	sh7708s_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock, endianness_t endianness = ENDIANNESS_LITTLE);

	virtual void sh3_register_map(address_map& map) override ATTR_COLD;
};

class sh7709_device : public sh3_base_device
{
public:
	sh7709_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock, endianness_t endianness = ENDIANNESS_LITTLE);

	virtual void sh3_register_map(address_map& map) override ATTR_COLD;
};

class sh7709s_device : public sh3_base_device
{
public:
	sh7709s_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock, endianness_t endianness = ENDIANNESS_LITTLE);

	virtual void sh3_register_map(address_map& map) override ATTR_COLD;
};


class sh4_device : public sh4_base_device
{
public:
	sh4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness = ENDIANNESS_LITTLE);

protected:
	virtual void sh4_register_map(address_map& map) override ATTR_COLD;
};

class sh7091_device : public sh4_base_device
{
public:
	sh7091_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock, endianness_t endianness = ENDIANNESS_LITTLE);

protected:
	virtual void sh4_register_map(address_map& map) override ATTR_COLD;
};

class sh7750_device : public sh4_base_device
{
public:
	sh7750_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock, endianness_t endianness = ENDIANNESS_LITTLE);

protected:
	virtual void sh4_register_map(address_map& map) override ATTR_COLD;
};

class sh7750s_device : public sh4_base_device
{
public:
	sh7750s_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock, endianness_t endianness = ENDIANNESS_LITTLE);

protected:
	virtual void sh4_register_map(address_map& map) override ATTR_COLD;
};

class sh7750r_device : public sh4_base_device
{
public:
	sh7750r_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock, endianness_t endianness = ENDIANNESS_LITTLE);

protected:
	virtual void sh4_register_map(address_map& map) override ATTR_COLD;
};

class sh7751_device : public sh4_base_device
{
public:
	sh7751_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock, endianness_t endianness = ENDIANNESS_LITTLE);

protected:
	virtual void sh4_register_map(address_map& map) override ATTR_COLD;
};

class sh7751r_device : public sh4_base_device
{
public:
	sh7751r_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock, endianness_t endianness = ENDIANNESS_LITTLE);

protected:
	virtual void sh4_register_map(address_map& map) override ATTR_COLD;
};

class sh4_frontend : public sh_frontend
{
public:
	sh4_frontend(sh_common_execution *device, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);

protected:
	virtual uint16_t read_word(opcode_desc &desc) override;

private:
	virtual bool describe_group_0(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode) override;
	virtual bool describe_group_4(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode) override;
	virtual bool describe_group_15(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode) override;
	bool describe_op1111_0x13(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
	bool describe_op1111_0xf13(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
};

DECLARE_DEVICE_TYPE(SH3, sh3_device)
DECLARE_DEVICE_TYPE(SH7708S, sh7708s_device)
DECLARE_DEVICE_TYPE(SH7709, sh7709_device)
DECLARE_DEVICE_TYPE(SH7709S, sh7709s_device)
DECLARE_DEVICE_TYPE(SH4, sh4_device)
DECLARE_DEVICE_TYPE(SH7091, sh7091_device)
DECLARE_DEVICE_TYPE(SH7750, sh7750_device)
DECLARE_DEVICE_TYPE(SH7750R, sh7750r_device)
DECLARE_DEVICE_TYPE(SH7750S, sh7750s_device)
DECLARE_DEVICE_TYPE(SH7751, sh7751_device)
DECLARE_DEVICE_TYPE(SH7751R, sh7751r_device)

#endif // MAME_CPU_SH_SH4_H
