// license:BSD-3-Clause
// copyright-holders:Philip Bennett, Carl

// SoftFloat 2 lacks an include guard
#ifndef softfloat2_h
#define softfloat2_h 1
#include "softfloat/milieu.h"
#include "softfloat/softfloat.h"
#endif

DECLARE_DEVICE_TYPE(I8087, i8087_device)

class i8087_device : public device_t
{
public:
	i8087_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <class Object> void set_space_88(Object &&tag, int spacenum) { m_space.set_tag(std::forward<Object>(tag), spacenum); m_space.set_data_width(8); }
	template <class Object> void set_space_86(Object &&tag, int spacenum) { m_space.set_tag(std::forward<Object>(tag), spacenum); m_space.set_data_width(16); }
	auto irq() { return m_int_handler.bind(); }
	auto busy() { return m_busy_handler.bind(); }

	void insn_w(uint32_t data); // the real 8087 sniffs the bus watching for esc, can't do that here so provide a poke spot
	void addr_w(uint32_t data);

protected:
	i8087_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(release_busy);

private:
	address_space &space() { return *m_space; }

	typedef void (i8087_device::*x87_func)(u8 modrm);
	required_address_space m_space;
	devcb_write_line m_int_handler;
	devcb_write_line m_busy_handler;
	emu_timer *m_timer;
	floatx80 m_reg[8];

	u32 m_ea;
	u32 m_pc;
	u32 m_ppc;
	u16 m_opcode;
	u16 m_cw;
	u16 m_sw;
	u16 m_tw;
	int m_icount;

	x87_func m_opcode_table_d8[256];
	x87_func m_opcode_table_d9[256];
	x87_func m_opcode_table_da[256];
	x87_func m_opcode_table_db[256];
	x87_func m_opcode_table_dc[256];
	x87_func m_opcode_table_dd[256];
	x87_func m_opcode_table_de[256];
	x87_func m_opcode_table_df[256];

	void execute();
	void CYCLES(int cycles) {m_icount = cycles;}
	u8 FETCH();
	u8 READ8(offs_t ea);
	void WRITE8(offs_t ea, u8 data);
	u16 READ16(offs_t ea);
	void WRITE16(offs_t ea, u16 data);
	u32 READ32(offs_t ea);
	void WRITE32(offs_t ea, u32 data);
	u64 READ64(offs_t ea);
	void WRITE64(offs_t ea, u64 data);
	floatx80 READ80(offs_t);
	void WRITE80(offs_t ea, floatx80 t);
	void set_stack_top(int top);
	void set_tag(int reg, int tag);
	void write_stack(int i, floatx80 value, bool update_tag);
	void set_stack_underflow();
	void set_stack_overflow();
	int inc_stack();
	int dec_stack();
	int check_exceptions(bool store = false);
	void write_cw(u16 cw);
	floatx80 add(floatx80 a, floatx80 b);
	floatx80 sub(floatx80 a, floatx80 b);
	floatx80 mul(floatx80 a, floatx80 b);
	floatx80 div(floatx80 a, floatx80 b);
	void fadd_m32real(u8 modrm);
	void fadd_m64real(u8 modrm);
	void fadd_st_sti(u8 modrm);
	void fadd_sti_st(u8 modrm);
	void faddp(u8 modrm);
	void fiadd_m32int(u8 modrm);
	void fiadd_m16int(u8 modrm);
	void fsub_m32real(u8 modrm);
	void fsub_m64real(u8 modrm);
	void fsub_st_sti(u8 modrm);
	void fsub_sti_st(u8 modrm);
	void fsubp(u8 modrm);
	void fisub_m32int(u8 modrm);
	void fisub_m16int(u8 modrm);
	void fsubr_m32real(u8 modrm);
	void fsubr_m64real(u8 modrm);
	void fsubr_st_sti(u8 modrm);
	void fsubr_sti_st(u8 modrm);
	void fsubrp(u8 modrm);
	void fisubr_m32int(u8 modrm);
	void fisubr_m16int(u8 modrm);
	void fdiv_m32real(u8 modrm);
	void fdiv_m64real(u8 modrm);
	void fdiv_st_sti(u8 modrm);
	void fdiv_sti_st(u8 modrm);
	void fdivp(u8 modrm);
	void fidiv_m32int(u8 modrm);
	void fidiv_m16int(u8 modrm);
	void fdivr_m32real(u8 modrm);
	void fdivr_m64real(u8 modrm);
	void fdivr_st_sti(u8 modrm);
	void fdivr_sti_st(u8 modrm);
	void fdivrp(u8 modrm);
	void fidivr_m32int(u8 modrm);
	void fidivr_m16int(u8 modrm);
	void fmul_m32real(u8 modrm);
	void fmul_m64real(u8 modrm);
	void fmul_st_sti(u8 modrm);
	void fmul_sti_st(u8 modrm);
	void fmulp(u8 modrm);
	void fimul_m32int(u8 modrm);
	void fimul_m16int(u8 modrm);
	void fprem(u8 modrm);
	void fprem1(u8 modrm);
	void fsqrt(u8 modrm);
	void f2xm1(u8 modrm);
	void fyl2x(u8 modrm);
	void fyl2xp1(u8 modrm);
	void fptan(u8 modrm);
	void fpatan(u8 modrm);
	void fsin(u8 modrm);
	void fcos(u8 modrm);
	void fsincos(u8 modrm);
	void fld_m32real(u8 modrm);
	void fld_m64real(u8 modrm);
	void fld_m80real(u8 modrm);
	void fld_sti(u8 modrm);
	void fild_m16int(u8 modrm);
	void fild_m32int(u8 modrm);
	void fild_m64int(u8 modrm);
	void fbld(u8 modrm);
	void fst_m32real(u8 modrm);
	void fst_m64real(u8 modrm);
	void fst_sti(u8 modrm);
	void fstp_m32real(u8 modrm);
	void fstp_m64real(u8 modrm);
	void fstp_m80real(u8 modrm);
	void fstp_sti(u8 modrm);
	void fist_m16int(u8 modrm);
	void fist_m32int(u8 modrm);
	void fistp_m16int(u8 modrm);
	void fistp_m32int(u8 modrm);
	void fistp_m64int(u8 modrm);
	void fbstp(u8 modrm);
	void fld1(u8 modrm);
	void fldl2t(u8 modrm);
	void fldl2e(u8 modrm);
	void fldpi(u8 modrm);
	void fldlg2(u8 modrm);
	void fldln2(u8 modrm);
	void fldz(u8 modrm);
	void fnop(u8 modrm);
	void fchs(u8 modrm);
	void fabs(u8 modrm);
	void fscale(u8 modrm);
	void frndint(u8 modrm);
	void fxtract(u8 modrm);
	void ftst(u8 modrm);
	void fxam(u8 modrm);
	void ficom_m16int(u8 modrm);
	void ficom_m32int(u8 modrm);
	void ficomp_m16int(u8 modrm);
	void ficomp_m32int(u8 modrm);
	void fcom_m32real(u8 modrm);
	void fcom_m64real(u8 modrm);
	void fcom_sti(u8 modrm);
	void fcomp_m32real(u8 modrm);
	void fcomp_m64real(u8 modrm);
	void fcomp_sti(u8 modrm);
	void fcompp(u8 modrm);
	void fucom_sti(u8 modrm);
	void fucomp_sti(u8 modrm);
	void fucompp(u8 modrm);
	void fdecstp(u8 modrm);
	void fincstp(u8 modrm);
	void fclex(u8 modrm);
	void ffree(u8 modrm);
	void fdisi(u8 modrm);
	void feni(u8 modrm);
	void finit(u8 modrm);
	void fldcw(u8 modrm);
	void fstcw(u8 modrm);
	void fldenv(u8 modrm);
	void fstenv(u8 modrm);
	void fsave(u8 modrm);
	void frstor(u8 modrm);
	void fxch(u8 modrm);
	void fxch_sti(u8 modrm);
	void fstsw_ax(u8 modrm);
	void fstsw_m2byte(u8 modrm);
	void invalid(u8 modrm);
	void group_d8(u8 modrm);
	void group_d9(u8 modrm);
	void group_da(u8 modrm);
	void group_db(u8 modrm);
	void group_dc(u8 modrm);
	void group_dd(u8 modrm);
	void group_de(u8 modrm);
	void group_df(u8 modrm);
	void build_opcode_table_d8();
	void build_opcode_table_d9();
	void build_opcode_table_da();
	void build_opcode_table_db();
	void build_opcode_table_dc();
	void build_opcode_table_dd();
	void build_opcode_table_de();
	void build_opcode_table_df();
	void build_opcode_table();

};
