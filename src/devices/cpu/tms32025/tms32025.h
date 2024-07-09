// license:BSD-3-Clause
// copyright-holders:Tony La Porta
	/**************************************************************************\
	*                Texas Instruments TMS320x25 DSP Emulator                  *
	*                                                                          *
	*                 Copyright Tony La Porta                                  *
	*                      Written for the MAME project.                       *
	*                                                                          *
	*      Note :  This is a word based microcontroller, with addressing       *
	*              architecture based on the Harvard addressing scheme.        *
	*                                                                          *
	*  Three versions of the chip are available, and they are:                 *
	*  TMS320C25   Internal ROM one time programmed at TI                      *
	*  TMS320E25   Internal ROM programmable as a normal EPROM                 *
	*  TMS320P25   Internal ROM programmable once as a normal EPROM only       *
	*  These devices can also be used as a MicroController with external ROM   *
	*                                                                          *
	\***************************************************************************/

#ifndef MAME_TMS32025_TMS32025_H
#define MAME_TMS32025_TMS32025_H

#pragma once


/****************************************************************************
 *  Interrupt constants
 */

#define TMS32025_INT0             0         /* External INT0 */
#define TMS32025_INT1             1         /* External INT1 */
#define TMS32025_INT2             2         /* External INT2 */
#define TMS32025_TINT             3         /* Internal Timer interrupt */
#define TMS32025_RINT             4         /* Serial Port receive  interrupt */
#define TMS32025_XINT             5         /* Serial Port transmit interrupt */
#define TMS32025_TRAP             6         /* Trap instruction */
#define TMS32025_INT_NONE         -1

/* Non-irq line */
#define TMS32025_FSX              7         /* Frame synchronisation */

enum
{
	TMS32025_PC=1,
	TMS32025_PFC,  TMS32025_STR0, TMS32025_STR1, TMS32025_IFR,
	TMS32025_RPTC, TMS32025_ACC,  TMS32025_PREG, TMS32025_TREG,
	TMS32025_AR0,  TMS32025_AR1,  TMS32025_AR2,  TMS32025_AR3,
	TMS32025_AR4,  TMS32025_AR5,  TMS32025_AR6,  TMS32025_AR7,
	TMS32025_STK0, TMS32025_STK1, TMS32025_STK2, TMS32025_STK3,
	TMS32025_STK4, TMS32025_STK5, TMS32025_STK6, TMS32025_STK7,
	TMS32025_DRR,  TMS32025_DXR,  TMS32025_TIM,  TMS32025_PRD,
	TMS32025_IMR,  TMS32025_GREG
};


/****************************************************************************
 *  Public Functions
 */


class tms3202x_device : public cpu_device
{
public:
	// configuration helpers
	auto bio_in_cb() { return m_bio_in.bind(); }
	auto hold_in_cb() { return m_hold_in.bind(); }
	auto hold_ack_out_cb() { return m_hold_ack_out.bind(); }
	auto xf_out_cb() { return m_xf_out.bind(); }
	auto dr_in_cb() { return m_dr_in.bind(); }
	auto dx_out_cb() { return m_dx_out.bind(); }

	//void tms32025_program(address_map &map);
	void tms3202x_data(address_map &map);
	void tms32026_data(address_map &map);

protected:
	// construction/destruction
	tms3202x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, unsigned stack_depth, address_map_constructor prgmap, address_map_constructor datamap);

	// device_t implementation
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface implementation
	virtual uint32_t execute_min_cycles() const noexcept override { return 4; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 20; }
	virtual uint32_t execute_input_lines() const noexcept override { return 6; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface implementation
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void common_reset();

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_io_config;

	required_shared_ptr<uint16_t> m_b0;
	required_shared_ptr<uint16_t> m_b1;
	required_shared_ptr<uint16_t> m_b2;
	optional_shared_ptr<uint16_t> m_b3;

	memory_access<16, 1, -1, ENDIANNESS_BIG>::cache m_cache;
	memory_access<16, 1, -1, ENDIANNESS_BIG>::specific m_program;
	memory_access<16, 1, -1, ENDIANNESS_BIG>::specific m_data;
	memory_access<16, 1, -1, ENDIANNESS_BIG>::specific m_io;

	typedef void ( tms3202x_device::*opcode_func ) ();
	struct tms32025_opcode
	{
		uint8_t       cycles;
		opcode_func function;
	};
	static const tms32025_opcode s_opcode_main[256];
	static const tms32025_opcode s_opcode_CE_subset[256];
	static const tms32025_opcode s_opcode_Dx_subset[8];

	devcb_read16 m_bio_in;
	devcb_read16 m_hold_in;
	devcb_write16 m_hold_ack_out;
	devcb_write16 m_xf_out;
	devcb_read16 m_dr_in;
	devcb_write16 m_dx_out;

	unsigned const m_stack_limit;


	/******************** CPU Internal Registers *******************/
	uint16_t  m_PREVPC;     /* previous program counter */
	uint16_t  m_PC;
	uint16_t  m_PFC;
	uint16_t  m_STR0, m_STR1;
	uint8_t   m_IFR;
	uint8_t   m_RPTC;
	PAIR    m_ACC;
	PAIR    m_Preg;
	uint16_t  m_Treg;
	uint16_t  m_AR[8]; // 5 for TMS32020, 8 for rest
	uint16_t  m_STACK[8]; // 4 level for TMS32020, 8 level for rest
	PAIR    m_ALU;
	uint16_t  m_drr, m_dxr, m_tim, m_prd, m_imr, m_greg;

	uint16_t m_fixed_STR1;

	uint8_t   m_timerover;

	/********************** Status data ****************************/
	PAIR    m_opcode;
	int     m_idle;
	int     m_hold;
	int     m_external_mem_access;    /** required for hold mode. Implement it ! */
	int     m_init_load_addr;         /* 0=No, 1=Yes, 2=Once for repeat mode */
	int     m_tms32025_irq_cycles;
	int     m_tms32025_dec_cycles;

	PAIR    m_oldacc;
	uint32_t  m_memaccess;
	int     m_icount;
	int     m_mHackIgnoreARP;          /* special handling for lst, lst1 instructions */
	int     m_waiting_for_serial_frame;

	uint16_t drr_r();
	void drr_w(uint16_t data);
	uint16_t dxr_r();
	void dxr_w(uint16_t data);
	uint16_t tim_r();
	void tim_w(uint16_t data);
	uint16_t prd_r();
	void prd_w(uint16_t data);
	uint16_t imr_r();
	void imr_w(uint16_t data);
	uint16_t greg_r();
	void greg_w(uint16_t data);

	void CLR0(uint16_t flag);
	void SET0(uint16_t flag);
	void CLR1(uint16_t flag);
	void SET1(uint16_t flag);
	void MODIFY_DP(int data);
	void MODIFY_PM(int data);
	void MODIFY_ARP(int data);
	uint16_t reverse_carry_add(uint16_t arg0, uint16_t arg1 );
	void MODIFY_AR_ARP();
	void CALCULATE_ADD_CARRY();
	void CALCULATE_SUB_CARRY();
	void CALCULATE_ADD_OVERFLOW(int32_t addval);
	void CALCULATE_SUB_OVERFLOW(int32_t subval);
	uint16_t POP_STACK();
	void PUSH_STACK(uint16_t data);
	void SHIFT_Preg_TO_ALU();
	void GETDATA(int shift,int signext);
	void PUTDATA(uint16_t data);
	void PUTDATA_SST(uint16_t data);
	void opcodes_CE();
	void opcodes_Dx();
	void illegal();
	void abst();
	void add();
	void addc();
	void addh();
	void addk();
	void adds();
	void addt();
	void adlk();
	void adrk();
	void and_();
	void andk();
	void apac();
	void br();
	void bacc();
	void banz();
	void bbnz();
	void bbz();
	void bc();
	void bgez();
	void bgz();
	void bioz();
	void bit();
	void bitt();
	void blez();
	void blkd();
	void blkp();
	void blz();
	void bnc();
	void bnv();
	void bnz();
	void bv();
	void bz();
	void cala();
	void call();
	void cmpl();
	void cmpr();
	virtual void cnfd();
	virtual void cnfp();
	virtual void conf();
	void dint();
	void dmov();
	void eint();
	void fort();
	void idle();
	void in();
	void lac();
	void lack();
	void lact();
	void lalk();
	void lar_ar0();
	void lar_ar1();
	void lar_ar2();
	void lar_ar3();
	void lar_ar4();
	void lar_ar5();
	void lar_ar6();
	void lar_ar7();
	void lark_ar0();
	void lark_ar1();
	void lark_ar2();
	void lark_ar3();
	void lark_ar4();
	void lark_ar5();
	void lark_ar6();
	void lark_ar7();
	void ldp();
	void ldpk();
	void lph();
	void lrlk();
	void lst();
	void lst1();
	void lt();
	void lta();
	void ltd();
	void ltp();
	void lts();
	void mac();
	void macd();
	void mar();
	void mpy();
	void mpya();
	void mpyk();
	void mpys();
	void mpyu();
	void neg();
	void norm();
	void or_();
	void ork();
	void out();
	void pac();
	void pop();
	void popd();
	void pshd();
	void push();
	void rc();
	void ret();
	void rfsm();
	void rhm();
	void rol();
	void ror();
	void rovm();
	void rpt();
	void rptk();
	void rsxm();
	void rtc();
	void rtxm();
	void rxf();
	void sach();
	void sacl();
	void sar_ar0();
	void sar_ar1();
	void sar_ar2();
	void sar_ar3();
	void sar_ar4();
	void sar_ar5();
	void sar_ar6();
	void sar_ar7();
	void sblk();
	void sbrk_ar();
	void sc();
	void sfl();
	void sfr();
	void sfsm();
	void shm();
	void sovm();
	void spac();
	void sph();
	void spl();
	void spm();
	void sqra();
	void sqrs();
	void sst();
	void sst1();
	void ssxm();
	void stc();
	void stxm();
	void sub();
	void subb();
	void subc();
	void subh();
	void subk();
	void subs();
	void subt();
	void sxf();
	void tblr();
	void tblw();
	void trap();
	void xor_();
	void xork();
	void zalh();
	void zalr();
	void zals();
	int process_IRQs();
	void process_timer(int clocks);
};


class tms32020_device : public tms3202x_device
{
public:
	// construction/destruction
	tms32020_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class tms32025_device : public tms3202x_device
{
public:
	// construction/destruction
	tms32025_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	void set_mp_mc(bool state) { m_mp_mc = state; }

protected:
	// construction/destruction
	tms32025_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	tms32025_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor prgmap, address_map_constructor datamap);

	// device_t implementation
	virtual void device_start() override;

	virtual const tiny_rom_entry *device_rom_region() const override;

	bool    m_mp_mc;
};

class tms32026_device : public tms32025_device
{
public:
	// construction/destruction
	tms32026_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_reset() override;
	virtual void cnfd() override;
	virtual void cnfp() override;
	virtual void conf() override;
};

DECLARE_DEVICE_TYPE(TMS32020, tms32020_device)
DECLARE_DEVICE_TYPE(TMS32025, tms32025_device)
DECLARE_DEVICE_TYPE(TMS32026, tms32026_device)

#endif // MAME_TMS32025_TMS32025_H
