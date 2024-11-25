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

#ifndef MAME_CPU_SH4_SH4_H
#define MAME_CPU_SH4_SH4_H

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
class sh4be_frontend;

class sh34_base_device : public sh_common_execution
{
public:

	void set_md(int bit, int md) { m_md[bit] = md; }
	void set_sh4_clock(int clock) { m_clock = clock; }
	void set_sh4_clock(const XTAL &xtal) { set_sh4_clock(xtal.value()); }

	void set_mmu_hacktype(int hacktype) { m_mmuhack = hacktype; }

	TIMER_CALLBACK_MEMBER(sh4_refresh_timer_callback);
	TIMER_CALLBACK_MEMBER(sh4_rtc_timer_callback);
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

	// sh4 internal
	uint32_t m_m[16384];

	// timer regs handled manually for reuse
	uint32_t m_SH4_TSTR;
	uint32_t m_SH4_TCNT0;
	uint32_t m_SH4_TCNT1;
	uint32_t m_SH4_TCNT2;
	uint32_t m_SH4_TCR0;
	uint32_t m_SH4_TCR1;
	uint32_t m_SH4_TCR2;
	uint32_t m_SH4_TCOR0;
	uint32_t m_SH4_TCOR1;
	uint32_t m_SH4_TCOR2;
	uint32_t m_SH4_TOCR;
	uint32_t m_SH4_TCPR2;

	// INTC regs
	uint32_t m_SH4_IPRA;

	uint32_t m_SH4_IPRC;

	// DMAC regs
	uint32_t m_SH4_SAR0;
	uint32_t m_SH4_SAR1;
	uint32_t m_SH4_SAR2;
	uint32_t m_SH4_SAR3;

	uint32_t m_SH4_DAR0;
	uint32_t m_SH4_DAR1;
	uint32_t m_SH4_DAR2;
	uint32_t m_SH4_DAR3;

	uint32_t m_SH4_CHCR0;
	uint32_t m_SH4_CHCR1;
	uint32_t m_SH4_CHCR2;
	uint32_t m_SH4_CHCR3;

	uint32_t m_SH4_DMATCR0;
	uint32_t m_SH4_DMATCR1;
	uint32_t m_SH4_DMATCR2;
	uint32_t m_SH4_DMATCR3;

	uint32_t m_SH4_DMAOR;

	int8_t    m_nmi_line_state;

	int     m_irln;
	int     m_internal_irq_level;
	int     m_internal_irq_vector;

	emu_timer *m_dma_timer[4];
	emu_timer *m_refresh_timer;
	emu_timer *m_rtc_timer;
	emu_timer *m_timer[3];
	uint32_t  m_refresh_timer_base;
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

	// sh3 internal
	uint32_t  m_sh3internal_upper[0x3000/4];
	uint32_t  m_sh3internal_lower[0x1000];

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
	uint32_t compute_ticks_refresh_timer(emu_timer *timer, int hertz, int base, int divisor);
	void sh4_refresh_timer_recompute();
	void increment_rtc_time(int mode);
	void sh4_dmac_nmi();
	void sh4_handler_ipra_w(uint32_t data, uint32_t mem_mask);
	virtual uint32_t get_remap(uint32_t address);
	virtual uint32_t sh4_getsqremap(uint32_t address);
	void sh4_parse_configuration();
	void sh4_timer_recompute(int which);
	uint32_t sh4_handle_tcnt0_addr_r(uint32_t mem_mask);
	uint32_t sh4_handle_tcnt1_addr_r(uint32_t mem_mask);
	uint32_t sh4_handle_tcnt2_addr_r(uint32_t mem_mask);
	uint32_t sh4_handle_tcor0_addr_r(uint32_t mem_mask);
	uint32_t sh4_handle_tcor1_addr_r(uint32_t mem_mask);
	uint32_t sh4_handle_tcor2_addr_r(uint32_t mem_mask);
	uint32_t sh4_handle_tcr0_addr_r(uint32_t mem_mask);
	uint32_t sh4_handle_tcr1_addr_r(uint32_t mem_mask);
	uint32_t sh4_handle_tcr2_addr_r(uint32_t mem_mask);
	uint32_t sh4_handle_tstr_addr_r(uint32_t mem_mask);
	uint32_t sh4_handle_tocr_addr_r(uint32_t mem_mask);
	uint32_t sh4_handle_tcpr2_addr_r(uint32_t mem_mask);
	void sh4_handle_tstr_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_tcr0_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_tcr1_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_tcr2_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_tcor0_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_tcor1_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_tcor2_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_tcnt0_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_tcnt1_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_tcnt2_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_tocr_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_tcpr2_addr_w(uint32_t data, uint32_t mem_mask);
	int sh4_dma_transfer(int channel, int timermode, uint32_t chcr, uint32_t *sar, uint32_t *dar, uint32_t *dmatcr);
	int sh4_dma_transfer_device(int channel, uint32_t chcr, uint32_t *sar, uint32_t *dar, uint32_t *dmatcr);
	void sh4_dmac_check(int channel);
	void sh4_handle_sar0_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_sar1_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_sar2_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_sar3_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_dar0_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_dar1_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_dar2_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_dar3_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_dmatcr0_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_dmatcr1_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_dmatcr2_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_dmatcr3_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_chcr0_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_chcr1_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_chcr2_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_chcr3_addr_w(uint32_t data, uint32_t mem_mask);
	void sh4_handle_dmaor_addr_w(uint32_t data, uint32_t mem_mask);
	uint32_t sh4_handle_sar0_addr_r(uint32_t mem_mask) { return m_SH4_SAR0; }
	uint32_t sh4_handle_sar1_addr_r(uint32_t mem_mask) { return m_SH4_SAR1; }
	uint32_t sh4_handle_sar2_addr_r(uint32_t mem_mask) { return m_SH4_SAR2; }
	uint32_t sh4_handle_sar3_addr_r(uint32_t mem_mask) { return m_SH4_SAR3; }
	uint32_t sh4_handle_dar0_addr_r(uint32_t mem_mask) { return m_SH4_DAR0; }
	uint32_t sh4_handle_dar1_addr_r(uint32_t mem_mask) { return m_SH4_DAR1; }
	uint32_t sh4_handle_dar2_addr_r(uint32_t mem_mask) { return m_SH4_DAR2; }
	uint32_t sh4_handle_dar3_addr_r(uint32_t mem_mask) { return m_SH4_DAR3; }
	uint32_t sh4_handle_dmatcr0_addr_r(uint32_t mem_mask) { return m_SH4_DMATCR0; }
	uint32_t sh4_handle_dmatcr1_addr_r(uint32_t mem_mask) { return m_SH4_DMATCR1; }
	uint32_t sh4_handle_dmatcr2_addr_r(uint32_t mem_mask) { return m_SH4_DMATCR2; }
	uint32_t sh4_handle_dmatcr3_addr_r(uint32_t mem_mask) { return m_SH4_DMATCR3; }
	uint32_t sh4_handle_chcr0_addr_r(uint32_t mem_mask) { return m_SH4_CHCR0; }
	uint32_t sh4_handle_chcr1_addr_r(uint32_t mem_mask) { return m_SH4_CHCR1; }
	uint32_t sh4_handle_chcr2_addr_r(uint32_t mem_mask) { return m_SH4_CHCR2; }
	uint32_t sh4_handle_chcr3_addr_r(uint32_t mem_mask) { return m_SH4_CHCR3; }
	uint32_t sh4_handle_dmaor_addr_r(uint32_t mem_mask) { return m_SH4_DMAOR; }

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
public:
	void sh3_internal_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sh3_internal_r(offs_t offset, uint32_t mem_mask = ~0);

	void sh3_internal_high_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sh3_internal_high_r(offs_t offset, uint32_t mem_mask = ~0);

	void sh3_internal_map(address_map &map) ATTR_COLD;
protected:
	// construction/destruction
	sh3_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness);

	virtual void device_reset() override ATTR_COLD;
};


class sh4_base_device : public sh34_base_device
{
public:
	void sh4_internal_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sh4_internal_r(offs_t offset, uint32_t mem_mask = ~0);

	uint64_t sh4_utlb_address_array_r(offs_t offset);
	void sh4_utlb_address_array_w(offs_t offset, uint64_t data);
	uint64_t sh4_utlb_data_array1_r(offs_t offset);
	void sh4_utlb_data_array1_w(offs_t offset, uint64_t data);
	uint64_t sh4_utlb_data_array2_r(offs_t offset);
	void sh4_utlb_data_array2_w(offs_t offset, uint64_t data);

	virtual void LDTLB(const uint16_t opcode) override;

	virtual uint32_t get_remap(uint32_t address) override;
	virtual uint32_t sh4_getsqremap(uint32_t address) override;
	sh4_utlb m_utlb[64];

	void sh4_internal_map(address_map &map) ATTR_COLD;
protected:
	// construction/destruction
	sh4_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};


class sh3_device : public sh3_base_device
{
	friend class sh4_frontend;
public:
	sh3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	std::unique_ptr<sh4_frontend>      m_drcfe;                  /* pointer to the DRC front-end state */
	virtual const opcode_desc* get_desclist(offs_t pc) override;
	virtual void init_drc_frontend() override;
};


class sh3be_device : public sh3_base_device
{
	friend class sh4be_frontend;

public:
	sh3be_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	std::unique_ptr<sh4be_frontend>      m_drcfe;                  /* pointer to the DRC front-end state */
	virtual const opcode_desc* get_desclist(offs_t pc) override;
	virtual void init_drc_frontend() override;

protected:
	virtual void execute_run() override;
};


class sh4_device : public sh4_base_device
{
	friend class sh4_frontend;

public:
	sh4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	std::unique_ptr<sh4_frontend>      m_drcfe;                  /* pointer to the DRC front-end state */
	virtual const opcode_desc* get_desclist(offs_t pc) override;
	virtual void init_drc_frontend() override;

};


class sh4be_device : public sh4_base_device
{
	friend class sh4be_frontend;

public:
	sh4be_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	std::unique_ptr<sh4be_frontend>      m_drcfe;                  /* pointer to the DRC front-end state */
	virtual const opcode_desc* get_desclist(offs_t pc) override;
	virtual void init_drc_frontend() override;

protected:
	virtual void execute_run() override;
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

class sh4be_frontend : public sh4_frontend
{
public:
	sh4be_frontend(sh_common_execution *device, uint32_t window_start, uint32_t window_end, uint32_t max_sequence) :
		sh4_frontend(device, window_start, window_end, max_sequence)
	{}
protected:
	virtual uint16_t read_word(opcode_desc &desc) override;
};

DECLARE_DEVICE_TYPE(SH3LE, sh3_device)
DECLARE_DEVICE_TYPE(SH3BE, sh3be_device)
DECLARE_DEVICE_TYPE(SH4LE, sh4_device)
DECLARE_DEVICE_TYPE(SH4BE, sh4be_device)


#endif // MAME_CPU_SH4_SH4_H
