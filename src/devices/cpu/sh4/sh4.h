// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*****************************************************************************
 *
 *   sh4->h
 *   Portable Hitachi SH-4 (SH7750 family) emulator interface
 *
 *   By R. Belmont, based on sh2.c by Juergen Buchmueller, Mariusz Wojcieszek,
 *      Olivier Galibert, Sylvain Glaize, and James Forshaw.
 *
 *****************************************************************************/

#ifndef __SH4_H__
#define __SH4_H__

// doesn't actually seem to improve performance at all
#define SH4_USE_FASTRAM_OPTIMIZATION 0
#define SH4_MAX_FASTRAM       3

#define SH4_INT_NONE    -1
enum
{
	SH4_IRL0=0, SH4_IRL1, SH4_IRL2, SH4_IRL3, SH4_IRLn
};

enum
{
	SH4_PC=1, SH4_SR, SH4_PR, SH4_GBR, SH4_VBR, SH4_DBR, SH4_MACH, SH4_MACL,
	SH4_R0, SH4_R1, SH4_R2, SH4_R3, SH4_R4, SH4_R5, SH4_R6, SH4_R7,
	SH4_R8, SH4_R9, SH4_R10, SH4_R11, SH4_R12, SH4_R13, SH4_R14, SH4_R15, SH4_EA,
	SH4_R0_BK0, SH4_R1_BK0, SH4_R2_BK0, SH4_R3_BK0, SH4_R4_BK0, SH4_R5_BK0, SH4_R6_BK0, SH4_R7_BK0,
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
	UINT32 length;
	UINT32 size;
	void *buffer;
	int channel;
};

struct sh4_ddt_dma
{
	UINT32 source;
	UINT32 length;
	UINT32 size;
	UINT32 destination;
	void *buffer;
	int direction;
	int channel;
	int mode;
};

typedef void (*sh4_ftcsr_callback)(UINT32);


#define MCFG_SH4_MD0(_md0) \
	sh34_base_device::set_md0(*device, _md0);

#define MCFG_SH4_MD1(_md1) \
	sh34_base_device::set_md1(*device, _md1);

#define MCFG_SH4_MD2(_md2) \
	sh34_base_device::set_md2(*device, _md2);

#define MCFG_SH4_MD3(_md3) \
	sh34_base_device::set_md3(*device, _md3);

#define MCFG_SH4_MD4(_md4) \
	sh34_base_device::set_md4(*device, _md4);

#define MCFG_SH4_MD5(_md5) \
	sh34_base_device::set_md5(*device, _md5);

#define MCFG_SH4_MD6(_md6) \
	sh34_base_device::set_md6(*device, _md6);

#define MCFG_SH4_MD7(_md7) \
	sh34_base_device::set_md7(*device, _md7);

#define MCFG_SH4_MD8(_md8) \
	sh34_base_device::set_md8(*device, _md8);

#define MCFG_SH4_CLOCK(_clock) \
	sh34_base_device::set_sh4_clock(*device, _clock);


class sh34_base_device : public cpu_device
{
public:
	// construction/destruction
	sh34_base_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, endianness_t endianness, address_map_constructor internal);

//#if SH4_USE_FASTRAM_OPTIMIZATION
	void add_fastram(offs_t start, offs_t end, UINT8 readonly, void *base);
//#endif

	static void set_md0(device_t &device, int md0) { downcast<sh34_base_device &>(device).c_md0 = md0; }
	static void set_md1(device_t &device, int md0) { downcast<sh34_base_device &>(device).c_md1 = md0; }
	static void set_md2(device_t &device, int md0) { downcast<sh34_base_device &>(device).c_md2 = md0; }
	static void set_md3(device_t &device, int md0) { downcast<sh34_base_device &>(device).c_md3 = md0; }
	static void set_md4(device_t &device, int md0) { downcast<sh34_base_device &>(device).c_md4 = md0; }
	static void set_md5(device_t &device, int md0) { downcast<sh34_base_device &>(device).c_md5 = md0; }
	static void set_md6(device_t &device, int md0) { downcast<sh34_base_device &>(device).c_md6 = md0; }
	static void set_md7(device_t &device, int md0) { downcast<sh34_base_device &>(device).c_md7 = md0; }
	static void set_md8(device_t &device, int md0) { downcast<sh34_base_device &>(device).c_md8 = md0; }
	static void set_sh4_clock(device_t &device, int clock) { downcast<sh34_base_device &>(device).c_clock = clock; }

	TIMER_CALLBACK_MEMBER( sh4_refresh_timer_callback );
	TIMER_CALLBACK_MEMBER( sh4_rtc_timer_callback );
	TIMER_CALLBACK_MEMBER( sh4_timer_callback );
	TIMER_CALLBACK_MEMBER( sh4_dmac_callback );

	void sh4_set_frt_input(int state);
	void sh4_set_irln_input(int value);
	void sh4_set_ftcsr_callback(sh4_ftcsr_callback callback);
	int sh4_dma_data(struct sh4_device_dma *s);
	void sh4_dma_ddt(struct sh4_ddt_dma *s);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 4; }
	virtual UINT32 execute_input_lines() const override { return 5; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : ((spacenum == AS_IO) ? &m_io_config : nullptr); }

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 2; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

protected:
	address_space_config m_program_config;
	address_space_config m_io_config;

	int c_md2;
	int c_md1;
	int c_md0;
	int c_md6;
	int c_md4;
	int c_md3;
	int c_md5;
	int c_md7;
	int c_md8;
	int c_clock;

	UINT32  m_ppc;
	UINT32  m_pc;
	UINT32  m_spc;
	UINT32  m_pr;
	UINT32  m_sr;
	UINT32  m_ssr;
	UINT32  m_gbr;
	UINT32  m_vbr;
	UINT32  m_mach;
	UINT32  m_macl;
	UINT32  m_r[16];
	UINT32  m_rbnk[2][8];
	UINT32  m_sgr;
	UINT32  m_fr[16];
	UINT32  m_xf[16];
	UINT32  m_ea;
	UINT32  m_delay;
	UINT32  m_cpu_off;
	UINT32  m_pending_irq;
	UINT32  m_test_irq;
	UINT32  m_fpscr;
	UINT32  m_fpul;
	UINT32  m_dbr;

	UINT32  m_exception_priority[128];
	int     m_exception_requesting[128];

	INT8    m_irq_line_state[17];
	address_space *m_internal;
	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_io;

	// sh4 internal
	UINT32 m_m[16384];

	// timer regs handled manually for reuse
	UINT32 m_SH4_TSTR;
	UINT32 m_SH4_TCNT0;
	UINT32 m_SH4_TCNT1;
	UINT32 m_SH4_TCNT2;
	UINT32 m_SH4_TCR0;
	UINT32 m_SH4_TCR1;
	UINT32 m_SH4_TCR2;
	UINT32 m_SH4_TCOR0;
	UINT32 m_SH4_TCOR1;
	UINT32 m_SH4_TCOR2;
	UINT32 m_SH4_TOCR;
	UINT32 m_SH4_TCPR2;

	// INTC regs
	UINT32 m_SH4_IPRA;

	UINT32 m_SH4_IPRC;

	// DMAC regs
	UINT32 m_SH4_SAR0;
	UINT32 m_SH4_SAR1;
	UINT32 m_SH4_SAR2;
	UINT32 m_SH4_SAR3;

	UINT32 m_SH4_DAR0;
	UINT32 m_SH4_DAR1;
	UINT32 m_SH4_DAR2;
	UINT32 m_SH4_DAR3;

	UINT32 m_SH4_CHCR0;
	UINT32 m_SH4_CHCR1;
	UINT32 m_SH4_CHCR2;
	UINT32 m_SH4_CHCR3;

	UINT32 m_SH4_DMATCR0;
	UINT32 m_SH4_DMATCR1;
	UINT32 m_SH4_DMATCR2;
	UINT32 m_SH4_DMATCR3;

	UINT32 m_SH4_DMAOR;

	INT8    m_nmi_line_state;

	UINT8 m_sleep_mode;

	int     m_frt_input;
	int     m_irln;
	int     m_internal_irq_level;
	int     m_internal_irq_vector;

	emu_timer *m_dma_timer[4];
	emu_timer *m_refresh_timer;
	emu_timer *m_rtc_timer;
	emu_timer *m_timer[3];
	UINT32  m_refresh_timer_base;
	int     m_dma_timer_active[4];
	UINT32  m_dma_source[4];
	UINT32  m_dma_destination[4];
	UINT32  m_dma_count[4];
	int     m_dma_wordsize[4];
	int     m_dma_source_increment[4];
	int     m_dma_destination_increment[4];
	int     m_dma_mode[4];

	int     m_sh4_icount;
	int     m_is_slave;
	int     m_cpu_clock;
	int     m_bus_clock;
	int     m_pm_clock;
	int     m_fpu_sz;
	int     m_fpu_pr;
	int     m_ioport16_pullup;
	int     m_ioport16_direction;
	int     m_ioport4_pullup;
	int     m_ioport4_direction;

	void    (*m_ftcsr_read_callback)(UINT32 data);

	/* This MMU simulation is good for the simple remap used on Naomi GD-ROM SQ access *ONLY* */
	UINT32 m_sh4_tlb_address[64];
	UINT32 m_sh4_tlb_data[64];
	UINT8 m_sh4_mmu_enabled;

	int m_cpu_type;

	// sh3 internal
	UINT32  m_sh3internal_upper[0x3000/4];
	UINT32  m_sh3internal_lower[0x1000];

	UINT64 m_debugger_temp;


	void execute_one_0000(const UINT16 opcode);
	void execute_one_4000(const UINT16 opcode);
	void execute_one(const UINT16 opcode);
	inline void sh4_check_pending_irq(const char *message) // look for highest priority active exception and handle it
	{
		int a,irq,z;

		irq = 0;
		z = -1;
		for (a=0;a <= SH4_INTC_ROVI;a++)
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

	void TODO(const UINT16 opcode);
	void WB(offs_t A, UINT8 V);
	void WW(offs_t A, UINT16 V);
	void WL(offs_t A, UINT32 V);
	void ADD(const UINT16 opcode);
	void ADDI(const UINT16 opcode);
	void ADDC(const UINT16 opcode);
	void ADDV(const UINT16 opcode);
	void AND(const UINT16 opcode);
	void ANDI(const UINT16 opcode);
	void ANDM(const UINT16 opcode);
	void BF(const UINT16 opcode);
	void BFS(const UINT16 opcode);
	void BRA(const UINT16 opcode);
	void BRAF(const UINT16 opcode);
	void BSR(const UINT16 opcode);
	void BSRF(const UINT16 opcode);
	void BT(const UINT16 opcode);
	void BTS(const UINT16 opcode);
	void CLRMAC(const UINT16 opcode);
	void CLRT(const UINT16 opcode);
	void CMPEQ(const UINT16 opcode);
	void CMPGE(const UINT16 opcode);
	void CMPGT(const UINT16 opcode);
	void CMPHI(const UINT16 opcode);
	void CMPHS(const UINT16 opcode);
	void CMPPL(const UINT16 opcode);
	void CMPPZ(const UINT16 opcode);
	void CMPSTR(const UINT16 opcode);
	void CMPIM(const UINT16 opcode);
	void DIV0S(const UINT16 opcode);
	void DIV0U(const UINT16 opcode);
	void DIV1(const UINT16 opcode);
	void DMULS(const UINT16 opcode);
	void DMULU(const UINT16 opcode);
	void DT(const UINT16 opcode);
	void EXTSB(const UINT16 opcode);
	void EXTSW(const UINT16 opcode);
	void EXTUB(const UINT16 opcode);
	void EXTUW(const UINT16 opcode);
	void JMP(const UINT16 opcode);
	void JSR(const UINT16 opcode);
	void LDCSR(const UINT16 opcode);
	void LDCGBR(const UINT16 opcode);
	void LDCVBR(const UINT16 opcode);
	void LDCMSR(const UINT16 opcode);
	void LDCMGBR(const UINT16 opcode);
	void LDCMVBR(const UINT16 opcode);
	void LDSMACH(const UINT16 opcode);
	void LDSMACL(const UINT16 opcode);
	void LDSPR(const UINT16 opcode);
	void LDSMMACH(const UINT16 opcode);
	void LDSMMACL(const UINT16 opcode);
	void LDSMPR(const UINT16 opcode);
	void MAC_L(const UINT16 opcode);
	void MAC_W(const UINT16 opcode);
	void MOV(const UINT16 opcode);
	void MOVBS(const UINT16 opcode);
	void MOVWS(const UINT16 opcode);
	void MOVLS(const UINT16 opcode);
	void MOVBL(const UINT16 opcode);
	void MOVWL(const UINT16 opcode);
	void MOVLL(const UINT16 opcode);
	void MOVBM(const UINT16 opcode);
	void MOVWM(const UINT16 opcode);
	void MOVLM(const UINT16 opcode);
	void MOVBP(const UINT16 opcode);
	void MOVWP(const UINT16 opcode);
	void MOVLP(const UINT16 opcode);
	void MOVBS0(const UINT16 opcode);
	void MOVWS0(const UINT16 opcode);
	void MOVLS0(const UINT16 opcode);
	void MOVBL0(const UINT16 opcode);
	void MOVWL0(const UINT16 opcode);
	void MOVLL0(const UINT16 opcode);
	void MOVI(const UINT16 opcode);
	void MOVWI(const UINT16 opcode);
	void MOVLI(const UINT16 opcode);
	void MOVBLG(const UINT16 opcode);
	void MOVWLG(const UINT16 opcode);
	void MOVLLG(const UINT16 opcode);
	void MOVBSG(const UINT16 opcode);
	void MOVWSG(const UINT16 opcode);
	void MOVLSG(const UINT16 opcode);
	void MOVBS4(const UINT16 opcode);
	void MOVWS4(const UINT16 opcode);
	void MOVLS4(const UINT16 opcode);
	void MOVBL4(const UINT16 opcode);
	void MOVWL4(const UINT16 opcode);
	void MOVLL4(const UINT16 opcode);
	void MOVA(const UINT16 opcode);
	void MOVT(const UINT16 opcode);
	void MULL(const UINT16 opcode);
	void MULS(const UINT16 opcode);
	void MULU(const UINT16 opcode);
	void NEG(const UINT16 opcode);
	void NEGC(const UINT16 opcode);
	void NOP(const UINT16 opcode);
	void NOT(const UINT16 opcode);
	void OR(const UINT16 opcode);
	void ORI(const UINT16 opcode);
	void ORM(const UINT16 opcode);
	void ROTCL(const UINT16 opcode);
	void ROTCR(const UINT16 opcode);
	void ROTL(const UINT16 opcode);
	void ROTR(const UINT16 opcode);
	void RTE(const UINT16 opcode);
	void RTS(const UINT16 opcode);
	void SETT(const UINT16 opcode);
	void SHAL(const UINT16 opcode);
	void SHAR(const UINT16 opcode);
	void SHLL(const UINT16 opcode);
	void SHLL2(const UINT16 opcode);
	void SHLL8(const UINT16 opcode);
	void SHLL16(const UINT16 opcode);
	void SHLR(const UINT16 opcode);
	void SHLR2(const UINT16 opcode);
	void SHLR8(const UINT16 opcode);
	void SHLR16(const UINT16 opcode);
	void SLEEP(const UINT16 opcode);
	void STCSR(const UINT16 opcode);
	void STCGBR(const UINT16 opcode);
	void STCVBR(const UINT16 opcode);
	void STCMSR(const UINT16 opcode);
	void STCMGBR(const UINT16 opcode);
	void STCMVBR(const UINT16 opcode);
	void STSMACH(const UINT16 opcode);
	void STSMACL(const UINT16 opcode);
	void STSPR(const UINT16 opcode);
	void STSMMACH(const UINT16 opcode);
	void STSMMACL(const UINT16 opcode);
	void STSMPR(const UINT16 opcode);
	void SUB(const UINT16 opcode);
	void SUBC(const UINT16 opcode);
	void SUBV(const UINT16 opcode);
	void SWAPB(const UINT16 opcode);
	void SWAPW(const UINT16 opcode);
	void TAS(const UINT16 opcode);
	void TRAPA(const UINT16 opcode);
	void TST(const UINT16 opcode);
	void TSTI(const UINT16 opcode);
	void TSTM(const UINT16 opcode);
	void XOR(const UINT16 opcode);
	void XORI(const UINT16 opcode);
	void XORM(const UINT16 opcode);
	void XTRCT(const UINT16 opcode);
	void STCSSR(const UINT16 opcode);
	void STCSPC(const UINT16 opcode);
	void STCSGR(const UINT16 opcode);
	void STSFPUL(const UINT16 opcode);
	void STSFPSCR(const UINT16 opcode);
	void STCDBR(const UINT16 opcode);
	void STCRBANK(const UINT16 opcode);
	void STCMRBANK(const UINT16 opcode);
	void MOVCAL(const UINT16 opcode);
	void CLRS(const UINT16 opcode);
	void SETS(const UINT16 opcode);
	void STCMSGR(const UINT16 opcode);
	void STSMFPUL(const UINT16 opcode);
	void STSMFPSCR(const UINT16 opcode);
	void STCMDBR(const UINT16 opcode);
	void STCMSSR(const UINT16 opcode);
	void STCMSPC(const UINT16 opcode);
	void LDSMFPUL(const UINT16 opcode);
	void LDSMFPSCR(const UINT16 opcode);
	void LDCMDBR(const UINT16 opcode);
	void LDCMRBANK(const UINT16 opcode);
	void LDCMSSR(const UINT16 opcode);
	void LDCMSPC(const UINT16 opcode);
	void LDSFPUL(const UINT16 opcode);
	void LDSFPSCR(const UINT16 opcode);
	void LDCDBR(const UINT16 opcode);
	void SHAD(const UINT16 opcode);
	void SHLD(const UINT16 opcode);
	void LDCRBANK(const UINT16 opcode);
	void LDCSSR(const UINT16 opcode);
	void LDCSPC(const UINT16 opcode);
	void PREFM(const UINT16 opcode);
	void FMOVMRIFR(const UINT16 opcode);
	void FMOVFRMR(const UINT16 opcode);
	void FMOVFRMDR(const UINT16 opcode);
	void FMOVFRS0(const UINT16 opcode);
	void FMOVS0FR(const UINT16 opcode);
	void FMOVMRFR(const UINT16 opcode);
	void FMOVFR(const UINT16 opcode);
	void FLDI1(const UINT16 opcode);
	void FLDI0(const UINT16 opcode);
	void FLDS(const UINT16 opcode);
	void FSTS(const UINT16 opcode);
	void FRCHG();
	void FSCHG();
	void FTRC(const UINT16 opcode);
	void FLOAT(const UINT16 opcode);
	void FNEG(const UINT16 opcode);
	void FABS(const UINT16 opcode);
	void FCMP_EQ(const UINT16 opcode);
	void FCMP_GT(const UINT16 opcode);
	void FCNVDS(const UINT16 opcode);
	void FCNVSD(const UINT16 opcode);
	void FADD(const UINT16 opcode);
	void FSUB(const UINT16 opcode);
	void FMUL(const UINT16 opcode);
	void FDIV(const UINT16 opcode);
	void FMAC(const UINT16 opcode);
	void FSQRT(const UINT16 opcode);
	void FSRRA(const UINT16 opcode);
	void FSSCA(const UINT16 opcode);
	void FIPR(const UINT16 opcode);
	void FTRV(const UINT16 opcode);
	void op1111_0xf13(const UINT16 opcode);
	void dbreak(const UINT16 opcode);
	void op1111_0x13(UINT16 opcode);
	UINT8 RB(offs_t A);
	UINT16 RW(offs_t A);
	UINT32 RL(offs_t A);
	void sh4_change_register_bank(int to);
	void sh4_swap_fp_registers();
	void sh4_swap_fp_couples();
	void sh4_syncronize_register_bank(int to);
	void sh4_default_exception_priorities();
	void sh4_exception_recompute();
	void sh4_exception_request(int exception);
	void sh4_exception_unrequest(int exception);
	void sh4_exception_checkunrequest(int exception);
	void sh4_exception(const char *message, int exception);
	UINT32 compute_ticks_refresh_timer(emu_timer *timer, int hertz, int base, int divisor);
	void sh4_refresh_timer_recompute();
	void increment_rtc_time(int mode);
	void sh4_dmac_nmi();
	void sh4_handler_ipra_w(UINT32 data, UINT32 mem_mask);
	UINT32 sh4_getsqremap(UINT32 address);
	void sh4_parse_configuration();
	void sh4_timer_recompute(int which);
	UINT32 sh4_handle_tcnt0_addr_r(UINT32 mem_mask);
	UINT32 sh4_handle_tcnt1_addr_r(UINT32 mem_mask);
	UINT32 sh4_handle_tcnt2_addr_r(UINT32 mem_mask);
	UINT32 sh4_handle_tcor0_addr_r(UINT32 mem_mask);
	UINT32 sh4_handle_tcor1_addr_r(UINT32 mem_mask);
	UINT32 sh4_handle_tcor2_addr_r(UINT32 mem_mask);
	UINT32 sh4_handle_tcr0_addr_r(UINT32 mem_mask);
	UINT32 sh4_handle_tcr1_addr_r(UINT32 mem_mask);
	UINT32 sh4_handle_tcr2_addr_r(UINT32 mem_mask);
	UINT32 sh4_handle_tstr_addr_r(UINT32 mem_mask);
	UINT32 sh4_handle_tocr_addr_r(UINT32 mem_mask);
	UINT32 sh4_handle_tcpr2_addr_r(UINT32 mem_mask);
	void sh4_handle_tstr_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_tcr0_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_tcr1_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_tcr2_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_tcor0_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_tcor1_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_tcor2_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_tcnt0_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_tcnt1_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_tcnt2_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_tocr_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_tcpr2_addr_w(UINT32 data, UINT32 mem_mask);
	int sh4_dma_transfer(int channel, int timermode, UINT32 chcr, UINT32 *sar, UINT32 *dar, UINT32 *dmatcr);
	int sh4_dma_transfer_device(int channel, UINT32 chcr, UINT32 *sar, UINT32 *dar, UINT32 *dmatcr);
	void sh4_dmac_check(int channel);
	void sh4_handle_sar0_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_sar1_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_sar2_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_sar3_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_dar0_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_dar1_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_dar2_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_dar3_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_dmatcr0_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_dmatcr1_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_dmatcr2_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_dmatcr3_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_chcr0_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_chcr1_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_chcr2_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_chcr3_addr_w(UINT32 data, UINT32 mem_mask);
	void sh4_handle_dmaor_addr_w(UINT32 data, UINT32 mem_mask);
	UINT32 sh4_handle_sar0_addr_r(UINT32 mem_mask) { return m_SH4_SAR0; }
	UINT32 sh4_handle_sar1_addr_r(UINT32 mem_mask) { return m_SH4_SAR1; }
	UINT32 sh4_handle_sar2_addr_r(UINT32 mem_mask) { return m_SH4_SAR2; }
	UINT32 sh4_handle_sar3_addr_r(UINT32 mem_mask) { return m_SH4_SAR3; }
	UINT32 sh4_handle_dar0_addr_r(UINT32 mem_mask) { return m_SH4_DAR0; }
	UINT32 sh4_handle_dar1_addr_r(UINT32 mem_mask) { return m_SH4_DAR1; }
	UINT32 sh4_handle_dar2_addr_r(UINT32 mem_mask) { return m_SH4_DAR2; }
	UINT32 sh4_handle_dar3_addr_r(UINT32 mem_mask) { return m_SH4_DAR3; }
	UINT32 sh4_handle_dmatcr0_addr_r(UINT32 mem_mask) { return m_SH4_DMATCR0; }
	UINT32 sh4_handle_dmatcr1_addr_r(UINT32 mem_mask) { return m_SH4_DMATCR1; }
	UINT32 sh4_handle_dmatcr2_addr_r(UINT32 mem_mask) { return m_SH4_DMATCR2; }
	UINT32 sh4_handle_dmatcr3_addr_r(UINT32 mem_mask) { return m_SH4_DMATCR3; }
	UINT32 sh4_handle_chcr0_addr_r(UINT32 mem_mask) { return m_SH4_CHCR0; }
	UINT32 sh4_handle_chcr1_addr_r(UINT32 mem_mask) { return m_SH4_CHCR1; }
	UINT32 sh4_handle_chcr2_addr_r(UINT32 mem_mask) { return m_SH4_CHCR2; }
	UINT32 sh4_handle_chcr3_addr_r(UINT32 mem_mask) { return m_SH4_CHCR3; }
	UINT32 sh4_handle_dmaor_addr_r(UINT32 mem_mask) { return m_SH4_DMAOR; }

#if SH4_USE_FASTRAM_OPTIMIZATION
	/* fast RAM */
	bool            m_bigendian;
	UINT32          m_byte_xor;
	UINT32          m_word_xor;
	UINT32          m_dword_xor;
	UINT32              m_fastram_select;
	struct
	{
		offs_t              start;                      /* start of the RAM block */
		offs_t              end;                        /* end of the RAM block */
		UINT8               readonly;                   /* TRUE if read-only */
		void *              base;                       /* base in memory where the RAM lives */
	}       m_fastram[SH4_MAX_FASTRAM];
#endif
};


class sh3_base_device : public sh34_base_device
{
public:
	// construction/destruction
	sh3_base_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, endianness_t endianness);

	DECLARE_WRITE32_MEMBER( sh3_internal_w );
	DECLARE_READ32_MEMBER( sh3_internal_r );

	DECLARE_WRITE32_MEMBER( sh3_internal_high_w );
	DECLARE_READ32_MEMBER( sh3_internal_high_r );

protected:
	virtual void device_reset() override;
};


class sh4_base_device : public sh34_base_device
{
public:
	// construction/destruction
	sh4_base_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, endianness_t endianness);

	DECLARE_WRITE32_MEMBER( sh4_internal_w );
	DECLARE_READ32_MEMBER( sh4_internal_r );

	DECLARE_READ64_MEMBER( sh4_tlb_r );
	DECLARE_WRITE64_MEMBER( sh4_tlb_w );

protected:
	virtual void device_reset() override;
};


class sh3_device : public sh3_base_device
{
public:
	sh3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class sh3be_device : public sh3_base_device
{
public:
	sh3be_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void execute_run() override;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;
};


class sh4_device : public sh4_base_device
{
public:
	sh4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class sh4be_device : public sh4_base_device
{
public:
	sh4be_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void execute_run() override;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;
};


extern const device_type SH3LE;
extern const device_type SH3BE;
extern const device_type SH4LE;
extern const device_type SH4BE;


/***************************************************************************
    COMPILER-SPECIFIC OPTIONS
***************************************************************************/

#define SH4DRC_STRICT_VERIFY    0x0001          /* verify all instructions */
#define SH4DRC_FLUSH_PC         0x0002          /* flush the PC value before each memory access */
#define SH4DRC_STRICT_PCREL     0x0004          /* do actual loads on MOVLI/MOVWI instead of collapsing to immediates */

#define SH4DRC_COMPATIBLE_OPTIONS   (SH4DRC_STRICT_VERIFY | SH4DRC_FLUSH_PC | SH4DRC_STRICT_PCREL)
#define SH4DRC_FASTEST_OPTIONS  (0)

void sh4drc_set_options(device_t *device, UINT32 options);
void sh4drc_add_pcflush(device_t *device, offs_t address);

#endif /* __SH4_H__ */
